// SPDX-License-Identifier: GPL-3.0-or-later
#include "model.h"
#include "dressform.h"
#include "clothsolver.h"
#include <QFile>
#include <QLineF>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMatrix4x4>
#include <QSaveFile>
#include <QSet>
#include <QMap>
#include <algorithm>
#include <cmath>

namespace Seamly3D
{
namespace
{
double cross(const QPointF &a, const QPointF &b, const QPointF &c)
{
    return (b.x()-a.x())*(c.y()-a.y()) - (b.y()-a.y())*(c.x()-a.x());
}
bool number(const QJsonValue &v, double limit)
{
    return v.isDouble() && std::isfinite(v.toDouble()) && std::abs(v.toDouble()) <= limit;
}
bool vector3(const QJsonValue &v, QVector3D &result, double limit)
{
    const auto a = v.toArray();
    if (a.size() != 3 || !number(a[0], limit) || !number(a[1], limit) || !number(a[2], limit)) return false;
    result = QVector3D(float(a[0].toDouble()), float(a[1].toDouble()), float(a[2].toDouble()));
    return true;
}
QJsonArray jsonVector(const QVector3D &v) { return {v.x(), v.y(), v.z()}; }
bool onSegment(const QPointF &a, const QPointF &b, const QPointF &p)
{
    return std::abs(cross(a,b,p)) < 1e-7 && p.x() >= qMin(a.x(),b.x())-1e-7 &&
           p.x() <= qMax(a.x(),b.x())+1e-7 && p.y() >= qMin(a.y(),b.y())-1e-7 &&
           p.y() <= qMax(a.y(),b.y())+1e-7;
}
bool intersects(const QPointF &a, const QPointF &b, const QPointF &c, const QPointF &d)
{
    const double ab1=cross(a,b,c), ab2=cross(a,b,d), cd1=cross(c,d,a), cd2=cross(c,d,b);
    return (((ab1>0 && ab2<0) || (ab1<0 && ab2>0)) &&
            ((cd1>0 && cd2<0) || (cd1<0 && cd2>0))) ||
           onSegment(a,b,c) || onSegment(a,b,d) || onSegment(c,d,a) || onSegment(c,d,b);
}
bool simple(const QVector<QPointF> &p)
{
    double area=0;
    for (int i=0; i<p.size(); ++i)
    {
        const int next=(i+1)%p.size();
        if (QLineF(p[i],p[next]).length()<1e-6) return false;
        area += p[i].x()*p[next].y()-p[next].x()*p[i].y();
        for (int j=i+1; j<p.size(); ++j)
        {
            if (j==next || (j+1)%p.size()==i) continue;
            if (intersects(p[i],p[next],p[j],p[(j+1)%p.size()])) return false;
        }
    }
    return std::abs(area)>1e-4;
}
bool triangulate(const QVector<QPointF> &p, QVector<std::array<int,3>> &triangles)
{
    QVector<int> ring;
    double area=0;
    for (int i=0; i<p.size(); ++i) { ring << i; area += cross(QPointF(),p[i],p[(i+1)%p.size()]); }
    if (area<0) std::reverse(ring.begin(),ring.end());
    while (ring.size()>3)
    {
        int best=-1; double quality=-1;
        for (int i=0; i<ring.size(); ++i)
        {
            const int a=ring[(i+ring.size()-1)%ring.size()], b=ring[i], c=ring[(i+1)%ring.size()];
            const double area2=cross(p[a],p[b],p[c]);
            if (area2<1e-8) continue;
            bool contains=false;
            for (int k : ring)
            {
                if (k==a || k==b || k==c) continue;
                if (cross(p[a],p[b],p[k])>=-1e-8 && cross(p[b],p[c],p[k])>=-1e-8 &&
                    cross(p[c],p[a],p[k])>=-1e-8) { contains=true; break; }
            }
            if (contains) continue;
            const double ab=QLineF(p[a],p[b]).length(),bc=QLineF(p[b],p[c]).length(),ca=QLineF(p[c],p[a]).length();
            const double score=area2/(ab*ab+bc*bc+ca*ca);
            if (score>quality) { best=i; quality=score; }
        }
        if (best<0) return false;
        triangles.append(std::array<int,3>{{ring[(best+ring.size()-1)%ring.size()],ring[best],ring[(best+1)%ring.size()]}});
        ring.removeAt(best);
    }
    if (cross(p[ring[0]],p[ring[1]],p[ring[2]])<=1e-8) return false;
    triangles.append(std::array<int,3>{{ring[0],ring[1],ring[2]}});
    return true;
}
QPair<int,int> edgeKey(int a, int b) { return qMakePair(qMin(a,b),qMax(a,b)); }
}

double perimeter(const QVector<QPointF> &p)
{
    double length=0;
    for (int i=0; i<p.size(); ++i) length += QLineF(p[i],p[(i+1)%p.size()]).length();
    return length;
}
QVector<QPointF> resample(const QVector<QPointF> &p, int count)
{
    QVector<QPointF> result;
    if (p.size()<3 || count<3) return result;
    const double total=perimeter(p);
    if (total<1e-6) return result;
    int edge=0; double before=0;
    for (int i=0; i<count; ++i)
    {
        const double distance=total*i/count;
        double length=QLineF(p[edge],p[(edge+1)%p.size()]).length();
        while (edge<p.size()-1 && before+length<distance)
        {
            before+=length; ++edge;
            length=QLineF(p[edge],p[(edge+1)%p.size()]).length();
        }
        const double t=length>1e-10 ? (distance-before)/length : 0;
        result << p[edge]*(1-t)+p[(edge+1)%p.size()]*t;
    }
    return result;
}
double arcSteps(double start, double end)
{
    double n=std::fmod(end-start+BoundarySamples,BoundarySamples);
    return std::abs(n)<1e-10 ? BoundarySamples : n;
}
QPointF contourPoint(const QVector<QPointF> &outline,double fraction)
{
    if (outline.isEmpty()) return {};
    const double distance=(fraction-std::floor(fraction))*perimeter(outline);
    double before=0;
    for (int i=0;i<outline.size();++i)
    {
        const auto a=outline[i],b=outline[(i+1)%outline.size()]; const double length=QLineF(a,b).length();
        if (before+length>=distance && length>1e-10) return a+(b-a)*((distance-before)/length);
        before+=length;
    }
    return outline.first();
}
double seamLength(const Piece &piece, double start, double end)
{
    return perimeter(piece.outline)*arcSteps(start,end)/BoundarySamples;
}
bool decode(const QJsonObject &json, Project &project, QString &error)
{
    error.clear();
    auto fail=[&error](const QString &message) { error=message; return false; };
    if (json.value("format").toString()!=QStringLiteral("seamly3d") || (json.value("version").toDouble()!=1 && json.value("version").toDouble()!=2 && json.value("version").toDouble()!=3 && json.value("version").toDouble()!=4) ||
        json.value("units").toString()!=QStringLiteral("mm")) return fail(QStringLiteral("Unsupported Seamly3D format, version or units."));
    if (!json.value("pieces").isArray() || !json.value("seams").isArray()) return fail(QStringLiteral("Missing pieces or seams array."));
    Project candidate; candidate.source=json.value("source").toString();
    if (json.value("version").toInt()>=2)
    {
        if (!json.value("dressForm").isObject()) return fail(QStringLiteral("Missing dress form settings."));
        const auto f=json.value("dressForm").toObject(); auto &form=candidate.dressForm;
        if (!f.value("enabled").isBool() || !f.value("contact").isBool() || !f.value("stand").isBool()) return fail(QStringLiteral("Invalid dress form flags."));
        form.enabled=f.value("enabled").toBool(); form.contact=f.value("contact").toBool(); form.stand=f.value("stand").toBool();
        double *targets[]={&form.bust,&form.waist,&form.hips,&form.neck,&form.shoulders,&form.height,&form.clearance};
        const char *keys[]={"bust","waist","hips","neck","shoulders","height","clearance"};
        for (int i=0; i<7; ++i)
        {
            const auto value=f.value(QLatin1String(keys[i]));
            if (!number(value,100000)) return fail(QStringLiteral("Invalid dress form measurement."));
            *targets[i]=value.toDouble();
        }
        if (!validateDressForm(form,error)) return false;
    }
    if (json.value("version").toInt()>=4)
    {
        if (!number(json.value("meshSize"),100) || json.value("meshSize").toDouble()<5) return fail(QStringLiteral("Mesh size must be between 5 and 100 mm."));
        candidate.meshSize=json.value("meshSize").toDouble();
        const auto settings=json.value("simulation").toObject();
        if (!settings.value("physical").isBool()) return fail(QStringLiteral("Missing simulation mode."));
        auto &sim=candidate.simulation; sim.physical=settings.value("physical").toBool();
        double *values[]={&sim.density,&sim.stretchCompliance,&sim.bendCompliance,&sim.damping,&sim.friction};
        const char *keys[]={"density","stretchCompliance","bendCompliance","damping","friction"};
        const double low[]={10,0,0,0,0},high[]={2000,0.01,10,20,1};
        for (int i=0;i<5;++i) { const auto v=settings.value(keys[i]); if (!number(v,high[i]) || v.toDouble()<low[i]) return fail(QStringLiteral("Invalid cloth parameter: %1").arg(keys[i])); *values[i]=v.toDouble(); }
    }
    const auto pieces=json.value("pieces").toArray();
    if ((pieces.isEmpty() && !candidate.dressForm.enabled) || pieces.size()>32) return fail(QStringLiteral("A project needs 1 to 32 pieces or an enabled dress form."));
    QSet<QString> ids;
    for (const auto &value : pieces)
    {
        const auto o=value.toObject(); Piece piece;
        piece.id=o.value("id").toString(); piece.name=o.value("name").toString();
        if (piece.id.isEmpty() || piece.id.size()>128 || ids.contains(piece.id)) return fail(QStringLiteral("Empty or duplicate piece ID."));
        ids.insert(piece.id);
        if (json.value("version").toInt()>=4)
        { if (!o.value("pinTop").isBool()) return fail(QStringLiteral("Invalid pin flag.")); piece.pinTop=o.value("pinTop").toBool(); }
        if (!vector3(o.value("translation"),piece.translation,100000) || !vector3(o.value("rotation"),piece.rotation,360))
            return fail(QStringLiteral("Invalid piece position or rotation."));
        const auto points=o.value("outline").toArray();
        if (points.size()<3 || points.size()>2048) return fail(QStringLiteral("A contour must contain 3 to 2048 points."));
        for (const auto &point : points)
        {
            const auto xy=point.toArray();
            if (xy.size()!=2 || !number(xy[0],100000) || !number(xy[1],100000)) return fail(QStringLiteral("Invalid contour coordinate."));
            piece.outline << QPointF(xy[0].toDouble(),xy[1].toDouble());
        }
        if (!simple(piece.outline)) return fail(QStringLiteral("Invalid or intersecting sewing contour: %1").arg(piece.name));
        candidate.pieces << piece;
    }
    const auto seams=json.value("seams").toArray();
    if (seams.size()>256) return fail(QStringLiteral("Too many seams (maximum 256)."));
    QSet<QString> seamIds;
    for (const auto &value : seams)
    {
        const auto o=value.toObject(); Seam seam;
        if (json.value("version").toInt()>=3)
        {
            seam.id=o.value("id").toString(); seam.name=o.value("name").toString();
            if (seam.id.isEmpty() || seam.id.size()>128 || seamIds.contains(seam.id)) return fail(QStringLiteral("Empty or duplicate seam ID."));
            if (seam.name.trimmed().isEmpty() || seam.name.size()>128 || !o.value("enabled").isBool()) return fail(QStringLiteral("Invalid seam name or enabled flag."));
            seam.enabled=o.value("enabled").toBool();
        }
        else
        {
            seam.id=QStringLiteral("legacy-seam-%1").arg(candidate.seams.size()+1);
            seam.name=QStringLiteral("Seam %1").arg(candidate.seams.size()+1);
        }
        seamIds.insert(seam.id);
        seam.a=o.value("a").toString(); seam.b=o.value("b").toString();
        if (!ids.contains(seam.a) || !ids.contains(seam.b)) return fail(QStringLiteral("A seam references a missing piece."));
        double *targets[]={&seam.startA,&seam.endA,&seam.startB,&seam.endB};
        const char *keys[]={"startA","endA","startB","endB"};
        for (int i=0; i<4; ++i)
        {
            const auto v=o.value(QLatin1String(keys[i]));
            const bool continuous=json.value("version").toInt()>=4;
            if (!number(v,continuous ? 1 : BoundarySamples-1) || v.toDouble()<0 ||
                (continuous ? v.toDouble()>=1 : std::floor(v.toDouble())!=v.toDouble()))
                return fail(QStringLiteral("Invalid seam endpoint."));
            *targets[i]=v.toDouble()*(continuous ? BoundarySamples : 1);
        }
        if (!o.value("reverse").isBool()) return fail(QStringLiteral("Invalid seam direction."));
        seam.reverse=o.value("reverse").toBool();
        // A point on a piece cannot be sewn to itself.
        if (seam.a==seam.b)
        {
            const double aLength=arcSteps(seam.startA,seam.endA),bLength=arcSteps(seam.startB,seam.endB);
            const auto inside=[](double point,double start,double length) { const double d=std::fmod(point-start+BoundarySamples,BoundarySamples); return d<=length+1e-9; };
            if (inside(seam.startA,seam.startB,bLength) || inside(seam.startB,seam.startA,aLength))
                return fail(QStringLiteral("Self-sewing arcs must be disjoint."));
        }
        candidate.seams << seam;
    }
    project=candidate;
    return true;
}
QJsonObject encode(const Project &project)
{
    QJsonArray pieces,seams;
    for (const auto &p : project.pieces)
    {
        QJsonArray outline;
        for (const auto &point : p.outline) outline.append(QJsonArray{point.x(),point.y()});
        pieces.append(QJsonObject{{"id",p.id},{"name",p.name},{"outline",outline},
                                  {"translation",jsonVector(p.translation)},{"rotation",jsonVector(p.rotation)},{"pinTop",p.pinTop}});
    }
    for (const auto &s : project.seams)
        seams.append(QJsonObject{{"id",s.id.isEmpty() ? QStringLiteral("seam-%1").arg(seams.size()+1) : s.id},
                                 {"name",s.name.isEmpty() && s.id.isEmpty() ? QStringLiteral("Seam %1").arg(seams.size()+1) : s.name},{"enabled",s.enabled},
                                 {"a",s.a},{"b",s.b},{"startA",s.startA/BoundarySamples},{"endA",s.endA/BoundarySamples},
                                 {"startB",s.startB/BoundarySamples},{"endB",s.endB/BoundarySamples},{"reverse",s.reverse}});
    const auto &f=project.dressForm;
    const QJsonObject form{{"enabled",f.enabled},{"contact",f.contact},{"stand",f.stand},{"bust",f.bust},{"waist",f.waist},
                           {"hips",f.hips},{"neck",f.neck},{"shoulders",f.shoulders},{"height",f.height},{"clearance",f.clearance}};
    const auto &sim=project.simulation;
    const QJsonObject simulation{{"physical",sim.physical},{"density",sim.density},{"stretchCompliance",sim.stretchCompliance},
        {"bendCompliance",sim.bendCompliance},{"damping",sim.damping},{"friction",sim.friction}};
    return {{"meshSize",project.meshSize},{"simulation",simulation},{"format","seamly3d"},{"version",4},{"units","mm"},{"source",project.source},{"pieces",pieces},{"seams",seams},{"dressForm",form}};
}
bool load(const QString &path, Project &project, QString &error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) { error=file.errorString(); return false; }
    if (file.size()>8*1024*1024) { error=QStringLiteral("Project exceeds 8 MB."); return false; }
    QJsonParseError parseError;
    const auto document=QJsonDocument::fromJson(file.readAll(),&parseError);
    if (parseError.error!=QJsonParseError::NoError || !document.isObject()) { error=QStringLiteral("Invalid JSON project: %1").arg(parseError.errorString()); return false; }
    return decode(document.object(),project,error);
}
bool save(const QString &path, const Project &project, QString &error)
{
    Project validated;
    if (!decode(encode(project),validated,error)) return false;
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) { error=file.errorString(); return false; }
    const auto bytes=QJsonDocument(encode(project)).toJson();
    if (file.write(bytes)!=bytes.size() || !file.commit()) { error=file.errorString(); return false; }
    return true;
}

bool Assembly::reset(const Project &project, QString &error)
{
    Project validated;
    if (!decode(encode(project),validated,error)) return false;
    QVector<Mesh> meshes;
    for (const auto &piece : project.pieces)
    {
        Mesh mesh; mesh.id=piece.id;
        // Original corners and authored seam endpoints are independent of mesh density.
        QVector<double> parameters{0}; const double length=perimeter(piece.outline); double distance=0;
        for (int i=0;i<piece.outline.size()-1;++i) { distance+=QLineF(piece.outline[i],piece.outline[i+1]).length(); parameters<<distance/length; }
        if (piece.pinTop)
        {
            double top=-1e9; for (const auto &p : piece.outline) top=qMax(top,p.y());
            double before=0;
            for (int i=0;i<piece.outline.size();++i)
            {
                const auto a=piece.outline[i],b=piece.outline[(i+1)%piece.outline.size()]; const double edge=QLineF(a,b).length();
                if (std::abs(a.y()-top)<0.01 && std::abs(b.y()-top)<0.01) parameters<<(before+edge*0.5)/length;
                before+=edge;
            }
        }
        auto addEndpoint=[&](double u) {
            // A nearby corner already supports the exact interpolated anchor.
            // Do not create microscopic triangles after percentage-field rounding.
            for (double existing : parameters) { const double d=std::abs(existing-u); if (qMin(d,1-d)*length<0.001) return; }
            parameters<<u;
        };
        for (const auto &seam : project.seams)
        {
            if (seam.a==piece.id) { addEndpoint(seam.startA/BoundarySamples); addEndpoint(seam.endA/BoundarySamples); }
            if (seam.b==piece.id) { addEndpoint(seam.startB/BoundarySamples); addEndpoint(seam.endB/BoundarySamples); }
        }
        std::sort(parameters.begin(),parameters.end());
        parameters.erase(std::unique(parameters.begin(),parameters.end(),[](double a,double b) { return std::abs(a-b)<1e-10; }),parameters.end());
        if (parameters.size()>512) { error=QStringLiteral("Contour %1 exceeds 512 meshing corners/anchors. Simplify the source contour.").arg(piece.name); return false; }
        QVector<QPointF> boundary; for (double u : parameters) boundary<<contourPoint(piece.outline,u);
        if (!triangulate(boundary,mesh.triangles))
        { error=QStringLiteral("Cannot triangulate %1 without losing contour details.").arg(piece.name); return false; }
        // Retain the legacy placement origin across densities and file migrations.
        QPointF center; const auto reference=resample(piece.outline);
        for (const auto &p : reference) center+=p; center/=reference.size();
        for (int i=0;i<boundary.size();++i)
        { mesh.rest<<QVector3D(float(boundary[i].x()-center.x()),float(boundary[i].y()-center.y()),0); mesh.boundary<<i; }
        mesh.parameters=parameters;
        bool complete=false;
        for (int level=0;level<20;++level)
        {
            QMap<QPair<int,int>,int> midpoints;
            for (const auto &t : mesh.triangles) for (int k=0;k<3;++k)
            {
                const auto key=edgeKey(t[k],t[(k+1)%3]);
                if (!midpoints.contains(key) && (mesh.rest[key.first]-mesh.rest[key.second]).length()>project.meshSize*1.00001)
                { midpoints.insert(key,int(mesh.rest.size())); mesh.rest<<(mesh.rest[key.first]+mesh.rest[key.second])*0.5f; }
            }
            if (midpoints.isEmpty()) { complete=true; break; }
            if (mesh.rest.size()>12000 || mesh.triangles.size()>24000)
            { error=QStringLiteral("Mesh budget exceeded for %1. Increase edge size or simplify the contour.").arg(piece.name); return false; }
            QVector<std::array<int,3>> refined;
            auto add=[&](int a,int b,int c) { refined.append(std::array<int,3>{{a,b,c}}); };
            for (const auto &t : mesh.triangles)
            {
                const int a=t[0],b=t[1],c=t[2];
                const int ab=midpoints.value(edgeKey(a,b),-1),bc=midpoints.value(edgeKey(b,c),-1),ca=midpoints.value(edgeKey(c,a),-1);
                const int mask=(ab>=0 ? 1 : 0)|(bc>=0 ? 2 : 0)|(ca>=0 ? 4 : 0);
                switch(mask)
                {
                case 0: add(a,b,c); break;
                case 1: add(a,ab,c); add(ab,b,c); break;
                case 2: add(b,bc,a); add(bc,c,a); break;
                case 4: add(c,ca,b); add(ca,a,b); break;
                case 3: add(b,bc,ab); add(a,ab,c); add(ab,bc,c); break;
                case 6: add(c,ca,bc); add(b,bc,a); add(bc,ca,a); break;
                case 5: add(a,ab,ca); add(c,ca,b); add(ca,ab,b); break;
                case 7: add(a,ab,ca); add(ab,b,bc); add(ca,bc,c); add(ab,bc,ca); break;
                }
            }
            QVector<int> indices; QVector<double> fractions;
            for (int i=0;i<mesh.boundary.size();++i)
            {
                const int j=(i+1)%mesh.boundary.size(); indices<<mesh.boundary[i]; fractions<<mesh.parameters[i];
                const int midpoint=midpoints.value(edgeKey(mesh.boundary[i],mesh.boundary[j]),-1);
                if (midpoint>=0) { indices<<midpoint; fractions<<(mesh.parameters[i]+(j ? mesh.parameters[j] : 1))*0.5; }
            }
            mesh.boundary=indices; mesh.parameters=fractions; mesh.triangles=refined;
        }
        if (!complete) { error=QStringLiteral("Mesh refinement did not reach the requested edge size."); return false; }
        QSet<QPair<int,int>> edges;
        for (const auto &t : mesh.triangles)
            for (int k=0; k<3; ++k) edges.insert(edgeKey(t[k],t[(k+1)%3]));
        // Sort constraints for reproducible previews across runs/platforms.
        auto keys=edges.values(); std::sort(keys.begin(),keys.end());
        for (const auto &e : keys) mesh.edges << Mesh::Edge{e.first,e.second,(mesh.rest[e.first]-mesh.rest[e.second]).length()};
        QMatrix4x4 transform;
        transform.translate(piece.translation);
        transform.rotate(piece.rotation.z(),0,0,1); transform.rotate(piece.rotation.y(),0,1,0); transform.rotate(piece.rotation.x(),1,0,0);
        for (const auto &v : mesh.rest) mesh.vertices << transform.map(v);
        int vertexCount=int(mesh.vertices.size()),triangleCount=int(mesh.triangles.size());
        for (const auto &existing : meshes) { vertexCount+=existing.vertices.size(); triangleCount+=existing.triangles.size(); }
        if (vertexCount>40000 || triangleCount>80000) { error=QStringLiteral("Project mesh budget exceeded. Increase edge size."); return false; }
        meshes << mesh;
    }
    QVector<Stitch> stitches;
    for (const auto &s : project.seams)
    {
        if (!s.enabled) continue;
        int a=-1,b=-1;
        for (int i=0; i<meshes.size(); ++i) { if (meshes[i].id==s.a) a=i; if (meshes[i].id==s.b) b=i; }
        const int count=qMax(1,int(std::ceil(qMax(seamLength(project.pieces[a],s.startA,s.endA),seamLength(project.pieces[b],s.startB,s.endB))/project.meshSize)));
        if (count>4096 || stitches.size()+count>32768) { error=QStringLiteral("Too many stitch constraints. Increase mesh edge size."); return false; }
        for (int i=0; i<=count; ++i)
        {
            if (i==count && s.startA==s.endA && s.startB==s.endB) continue;
            const double t=double(i)/count;
            stitches<<Stitch{a,b,boundaryAnchor(meshes[a],s.startA+t*arcSteps(s.startA,s.endA)),
                                  boundaryAnchor(meshes[b],s.startB+(s.reverse ? 1-t : t)*arcSteps(s.startB,s.endB))};
        }
    }
    m_meshes=meshes; m_stitches=stitches; m_dressForm=project.dressForm;
    m_formProfile=m_dressForm.enabled && m_dressForm.contact ? dressFormProfile(m_dressForm) : QVector<QVector3D>();
    m_cloth=project.simulation.physical ? std::make_shared<ClothSolver>(m_meshes,project) : nullptr;
    return true;
}
BoundaryAnchor boundaryAnchor(const Mesh &mesh,double position)
{
    const double u=position/BoundarySamples-std::floor(position/BoundarySamples);
    const auto it=std::upper_bound(mesh.parameters.cbegin(),mesh.parameters.cend(),u);
    const int next=int(it-mesh.parameters.cbegin()),first=qMax(0,next-1),second=next%mesh.boundary.size();
    const double end=next==mesh.parameters.size() ? 1 : mesh.parameters[next];
    return {mesh.boundary[first],mesh.boundary[second],float((u-mesh.parameters[first])/(end-mesh.parameters[first]))};
}
namespace
{
QVector3D location(const Mesh &m,const BoundaryAnchor &p) { return m.vertices[p.first]*(1-p.t)+m.vertices[p.second]*p.t; }
}
QVector3D boundaryPoint(const Mesh &mesh,double position) { return location(mesh,boundaryAnchor(mesh,position)); }
QVector<QVector3D> seamPath(const Mesh &mesh,double start,double end)
{
    QVector<QVector3D> path{boundaryPoint(mesh,start)};
    QVector<double> positions;
    const double length=arcSteps(start,end);
    for (double u : mesh.parameters)
    { const double d=std::fmod(u*BoundarySamples-start+BoundarySamples,BoundarySamples); if (d>1e-8 && d<length-1e-8) positions<<d; }
    std::sort(positions.begin(),positions.end());
    for (double d : positions) path<<boundaryPoint(mesh,start+d);
    path<<boundaryPoint(mesh,start+length); return path;
}
void Assembly::step(int iterations)
{
    if (m_cloth) { for (int i=0;i<qBound(0,iterations,100) && !m_cloth->stopped();++i) m_cloth->step(m_meshes,m_stitches); return; }
    for (int iteration=0; iteration<qBound(0,iterations,100); ++iteration)
    {
        for (auto &m : m_meshes)
            for (const auto &e : m.edges)
            {
                const auto delta=m.vertices[e.b]-m.vertices[e.a];
                const float distance=delta.length();
                if (distance<1e-8f) continue;
                const auto correction=delta*((distance-e.length)/distance*0.45f);
                m.vertices[e.a]+=correction; m.vertices[e.b]-=correction;
            }
        for (const auto &s : m_stitches)
        {
            auto &a=m_meshes[s.a]; auto &b=m_meshes[s.b];
            const auto p=s.u,q=s.v;
            const auto delta=location(b,q)-location(a,p);
            const float weight=(1-p.t)*(1-p.t)+p.t*p.t+(1-q.t)*(1-q.t)+q.t*q.t;
            const auto correction=delta*(0.65f/weight);
            a.vertices[p.first]+=correction*(1-p.t); a.vertices[p.second]+=correction*p.t;
            b.vertices[q.first]-=correction*(1-q.t); b.vertices[q.second]-=correction*q.t;
        }
        if (!m_formProfile.isEmpty())
            for (auto &mesh : m_meshes)
            {
                // Sample interiors as well as vertices: long triangles can cut
                // through a curved form even when all three corners are outside.
                const float weights[][3]={{1.f/3,1.f/3,1.f/3},{0.5f,0.5f,0},{0,0.5f,0.5f},{0.5f,0,0.5f}};
                for (const auto &triangle : mesh.triangles) for (const auto &w : weights)
                {
                    const auto p=mesh.vertices[triangle[0]]*w[0]+mesh.vertices[triangle[1]]*w[1]+mesh.vertices[triangle[2]]*w[2];
                    const auto delta=outsideDressForm(m_formProfile,p,float(m_dressForm.clearance))-p;
                    if (delta.lengthSquared()<1e-8f) continue;
                    const float denominator=w[0]*w[0]+w[1]*w[1]+w[2]*w[2];
                    for (int k=0; k<3; ++k) mesh.vertices[triangle[k]]+=delta*(w[k]/denominator);
                }
                for (auto &point : mesh.vertices)
                    point=outsideDressForm(m_formProfile,point,float(m_dressForm.clearance));
            }
    }
}
double Assembly::seamError() const
{
    if (m_stitches.isEmpty()) return 0;
    double total=0;
    for (const auto &s : m_stitches) total+=(location(m_meshes[s.a],s.u)-location(m_meshes[s.b],s.v)).length();
    return total/m_stitches.size();
}
double Assembly::meanStrain() const
{
    double strain=0; int count=0;
    for (const auto &mesh : m_meshes) for (const auto &edge : mesh.edges)
    {
        if (edge.length<1e-6f) continue;
        strain+=std::abs((mesh.vertices[edge.a]-mesh.vertices[edge.b]).length()/edge.length-1);
        ++count;
    }
    return count ? strain/count : 0;
}
double Assembly::simulationTime() const { return m_cloth ? m_cloth->time() : 0; }
double Assembly::maximumSpeed() const { return m_cloth ? m_cloth->speed() : 0; }
QString Assembly::simulationStatus() const { return m_cloth ? m_cloth->status() : QStringLiteral("Geometric assembly"); }
bool Assembly::simulationStopped() const { return m_cloth && m_cloth->stopped(); }
Project mergeGeometry(const Project &current, const Project &incoming)
{
    Project candidate=incoming; candidate.dressForm=current.dressForm; candidate.meshSize=current.meshSize; candidate.simulation=current.simulation; candidate.seams.clear(); QSet<QString> unchanged;
    if (current.source!=incoming.source) return candidate;
    for (auto &p : candidate.pieces)
        for (const auto &old : current.pieces)
            if (old.id==p.id)
            {
                p.translation=old.translation; p.rotation=old.rotation; p.pinTop=old.pinTop;
                if (old.outline==p.outline) unchanged.insert(p.id);
                break;
            }
    for (const auto &s : current.seams) if (unchanged.contains(s.a) && unchanged.contains(s.b)) candidate.seams << s;
    return candidate;
}
Project demoProject()
{
    Project project; project.source=QStringLiteral("Built-in assembly example");
    Piece a; a.id=QStringLiteral("front"); a.name=QStringLiteral("Front");
    // An octagonal pouch with an open top, intentionally simple and editable.
    a.outline={{-45,50},{45,50},{55,30},{55,-35},{35,-55},{-35,-55},{-55,-35},{-55,30}};
    a.translation={0,0,35};
    Piece b=a; b.id=QStringLiteral("back"); b.name=QStringLiteral("Back"); b.translation={0,0,-35};
    project.pieces={a,b};
    Seam seam; seam.a=a.id; seam.b=b.id; seam.startA=16; seam.endA=0;
    seam.id=QStringLiteral("pouch-seam"); seam.name=QStringLiteral("Pouch sides");
    seam.startB=16; seam.endB=0; seam.reverse=false;
    project.seams << seam;
    return project;
}
}

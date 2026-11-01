// SPDX-License-Identifier: GPL-3.0-or-later
#include "dressform.h"
#include <QLineF>
#include <cmath>
#include <tuple>

namespace Seamly3D
{
namespace
{
constexpr double Pi=3.14159265358979323846;
QVector2D ellipse(double circumference, double ratio)
{
    // Ramanujan perimeter approximation; scale both axes by the same amount.
    const double a=ratio,b=1,h=(a-b)*(a-b)/((a+b)*(a+b));
    const double perimeter=Pi*(a+b)*(1+3*h/(10+std::sqrt(4-3*h)));
    return QVector2D(float(circumference*a/perimeter),float(circumference*b/perimeter));
}
}
bool DressForm::operator==(const DressForm &o) const
{
    return std::tie(enabled,contact,stand,bust,waist,hips,neck,shoulders,height,clearance)==
           std::tie(o.enabled,o.contact,o.stand,o.bust,o.waist,o.hips,o.neck,o.shoulders,o.height,o.clearance);
}
bool validateDressForm(const DressForm &f, QString &error)
{
    const double values[]={f.bust,f.waist,f.hips,f.neck,f.shoulders,f.height,f.clearance};
    for (double value : values) if (!std::isfinite(value)) { error=QStringLiteral("Invalid dress form measurement."); return false; }
    if (f.bust<500 || f.bust>1600 || f.waist<400 || f.waist>1600 || f.hips<500 || f.hips>1800 ||
        f.neck<250 || f.neck>600 || f.shoulders<250 || f.shoulders>650 || f.height<400 || f.height>1000 ||
        f.clearance<0 || f.clearance>30)
    { error=QStringLiteral("Dress form measurements are outside the supported ranges."); return false; }
    return true;
}
double dressFormBottom(const DressForm &f) { return -0.42*f.height; }
double dressFormTop(const DressForm &f) { return 0.58*f.height; }
double dressFormFloor(const DressForm &f) { return dressFormBottom(f)-(f.stand ? 560 : 0); }
QVector2D dressFormRadii(const DressForm &f, double y)
{
    const auto bust=ellipse(f.bust,1.38),waist=ellipse(f.waist,1.35),hips=ellipse(f.hips,1.48),neck=ellipse(f.neck,1.14);
    struct Station { double t; QVector2D radius; };
    const Station stations[]={
        {0,hips*0.91f},{0.12,hips*0.98f},{0.22,hips},{0.32,(hips+waist)*0.5f},
        {0.42,waist},{0.54,(waist+bust)*0.5f},{0.67,bust},{0.75,QVector2D(bust.x()*0.98f,bust.y()*0.89f)},
        {0.85,QVector2D(float(f.shoulders*0.49),bust.y()*0.72f)},
        {0.88,QVector2D(float(f.shoulders*0.50),bust.y()*0.67f)},
        {0.93,QVector2D(float(f.shoulders*0.35),neck.y()*1.22f)},
        {0.97,neck},{1,neck*0.98f}};
    const double t=qBound(0.0,(y-dressFormBottom(f))/f.height,1.0);
    for (int i=1; i<int(sizeof(stations)/sizeof(stations[0])); ++i)
        if (t<=stations[i].t)
        {
            const int count=int(sizeof(stations)/sizeof(stations[0]));
            auto slope=[&](int k,int axis) {
                if (k==0) return double(stations[1].radius[axis]-stations[0].radius[axis])/(stations[1].t-stations[0].t);
                if (k==count-1) return double(stations[k].radius[axis]-stations[k-1].radius[axis])/(stations[k].t-stations[k-1].t);
                const double h0=stations[k].t-stations[k-1].t,h1=stations[k+1].t-stations[k].t;
                const double d0=(stations[k].radius[axis]-stations[k-1].radius[axis])/h0;
                const double d1=(stations[k+1].radius[axis]-stations[k].radius[axis])/h1;
                if (d0*d1<=0) return 0.0;
                const double w0=2*h1+h0,w1=h1+2*h0;
                return (w0+w1)/(w0/d0+w1/d1);
            };
            const double h=stations[i].t-stations[i-1].t,u=(t-stations[i-1].t)/h;
            QVector2D result;
            for (int axis=0; axis<2; ++axis)
                result[axis]=float((2*u*u*u-3*u*u+1)*stations[i-1].radius[axis]+(u*u*u-2*u*u+u)*h*slope(i-1,axis)+
                                   (-2*u*u*u+3*u*u)*stations[i].radius[axis]+(u*u*u-u*u)*h*slope(i,axis));
            return result;
        }
    return stations[12].radius;
}
QVector<QVector3D> dressFormProfile(const DressForm &f)
{
    QVector<QVector3D> result;
    for (int i=0; i<=80; ++i)
    {
        const double y=dressFormBottom(f)+f.height*i/80;
        const auto radius=dressFormRadii(f,y);
        result << QVector3D(float(y),radius.x(),radius.y());
    }
    return result;
}
Mesh dressFormMesh(const DressForm &f)
{
    Mesh mesh; mesh.id=QStringLiteral("__dress_form");
    const auto profile=dressFormProfile(f); constexpr int sides=64;
    for (const auto &ring : profile)
        for (int j=0; j<sides; ++j)
        {
            const double angle=2*Pi*j/sides;
            mesh.vertices << QVector3D(float(ring.y()*std::cos(angle)),ring.x(),float(ring.z()*std::sin(angle)));
        }
    for (int i=0; i<profile.size()-1; ++i)
        for (int j=0; j<sides; ++j)
        {
            const int a=i*sides+j,b=(i+1)*sides+j,c=i*sides+(j+1)%sides,d=(i+1)*sides+(j+1)%sides;
            mesh.triangles.append(std::array<int,3>{{a,b,c}}); mesh.triangles.append(std::array<int,3>{{c,b,d}});
        }
    const int bottom=int(mesh.vertices.size()),top=bottom+1,last=int(profile.size()-1)*sides;
    mesh.vertices << QVector3D(0,profile.first().x(),0) << QVector3D(0,profile.last().x(),0);
    for (int j=0; j<sides; ++j)
    {
        mesh.triangles.append(std::array<int,3>{{bottom,j,(j+1)%sides}});
        mesh.triangles.append(std::array<int,3>{{top,last+(j+1)%sides,last+j}});
    }
    mesh.rest=mesh.vertices; return mesh;
}
QVector3D outsideDressForm(const QVector<QVector3D> &profile, const QVector3D &p, float clearance)
{
    if (profile.size()<2 || p.y()<profile.first().x()-clearance || p.y()>profile.last().x()+clearance) return p;
    const float height=profile.last().x()-profile.first().x();
    const float position=qBound(0.f,(p.y()-profile.first().x())/height,1.f)*(profile.size()-1);
    const int index=qMin(int(position),int(profile.size()-2));
    const float t=position-index;
    const float rx=profile[index].y()*(1-t)+profile[index+1].y()*t+clearance;
    const float rz=profile[index].z()*(1-t)+profile[index+1].z()*t+clearance;
    const float q=std::sqrt(p.x()*p.x()/(rx*rx)+p.z()*p.z()/(rz*rz));
    if (q>=1) return p;
    QVector3D projected=q>1e-8f ? QVector3D(p.x()/q,p.y(),p.z()/q) : QVector3D(0,p.y(),rz);
    const float lower=p.y()-(profile.first().x()-clearance),upper=profile.last().x()+clearance-p.y();
    if (qMin(lower,upper)<(projected-p).length())
        projected=QVector3D(p.x(),lower<upper ? profile.first().x()-clearance : profile.last().x()+clearance,p.z());
    return projected;
}
void placeAroundDressForm(Piece &piece, const DressForm &form, double angleDegrees)
{
    const auto boundary=resample(piece.outline);
    if (boundary.isEmpty()) return;
    QPointF center; double top=-1e9;
    for (const auto &p : boundary) { center+=p; top=qMax(top,p.y()); }
    center/=boundary.size();
    float rx=0,rz=0;
    for (const auto &ring : dressFormProfile(form)) { rx=qMax(rx,ring.y()); rz=qMax(rz,ring.z()); }
    const double angle=angleDegrees*Pi/180;
    piece.rotation=QVector3D(0,float(angleDegrees),0);
    piece.translation=QVector3D(float((rx+60)*std::sin(angle)),float(0.44*form.height-(top-center.y())),float((rz+60)*std::cos(angle)));
}
Project dressFormDemoProject()
{
    Project result; result.source=QStringLiteral("Dress form assembly example"); result.dressForm.enabled=true;
    Piece front; front.id=QStringLiteral("front"); front.name=QStringLiteral("Front panel");
    front.outline={{-225,300},{225,300},{225,-300},{-225,-300}};
    Piece back=front; back.id=QStringLiteral("back"); back.name=QStringLiteral("Back panel");
    placeAroundDressForm(front,result.dressForm,0); placeAroundDressForm(back,result.dressForm,180);
    result.pieces={front,back};
    // Join sides only. Open top and hem; an editable demonstration, not a fitted garment.
    auto closest=[&](const QPointF &p) { double distance=0; for (int i=0;i<front.outline.size();++i) { if (front.outline[i]==p) return distance/perimeter(front.outline)*BoundarySamples; distance+=QLineF(front.outline[i],front.outline[(i+1)%front.outline.size()]).length(); } return 0.0; };
    Seam right; right.a=front.id; right.b=back.id; right.startA=closest(front.outline[1]); right.endA=closest(front.outline[2]);
    right.id=QStringLiteral("right-seam"); right.name=QStringLiteral("Right side");
    right.startB=closest(back.outline[3]); right.endB=closest(back.outline[0]); right.reverse=true;
    Seam left=right; left.startA=right.startB; left.endA=right.endB; left.startB=right.startA; left.endB=right.endA;
    left.id=QStringLiteral("left-seam"); left.name=QStringLiteral("Left side");
    result.seams={right,left}; return result;
}
}

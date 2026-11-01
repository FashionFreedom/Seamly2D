// SPDX-License-Identifier: GPL-3.0-or-later
#include "scenegeometry.h"
#include <algorithm>
#include <cmath>

namespace Seamly3D
{
namespace
{
struct Vertex { float x,y,z,nx,ny,nz; };
Vertex vertex(const QVector3D &p, const QVector3D &n) { return {p.x(),p.y(),p.z(),n.x(),n.y(),n.z()}; }
template<class T> QByteArray bytes(const QVector<T> &values)
{
    return QByteArray(reinterpret_cast<const char *>(values.constData()),int(values.size()*sizeof(T)));
}
void upload(QQuick3DGeometry *geometry, const QVector<Vertex> &vertices, const QVector<quint32> &indices,
            QQuick3DGeometry::PrimitiveType primitive)
{
    geometry->clear();
    geometry->setPrimitiveType(primitive); geometry->setStride(sizeof(Vertex));
    geometry->addAttribute(QQuick3DGeometry::Attribute::PositionSemantic,0,QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::NormalSemantic,3*sizeof(float),QQuick3DGeometry::Attribute::F32Type);
    geometry->addAttribute(QQuick3DGeometry::Attribute::IndexSemantic,0,QQuick3DGeometry::Attribute::U32Type);
    geometry->setVertexData(bytes(vertices)); geometry->setIndexData(bytes(indices));
    QVector3D minimum,maximum;
    if (!vertices.isEmpty())
    {
        minimum=maximum=QVector3D(vertices.first().x,vertices.first().y,vertices.first().z);
        for (const auto &v : vertices)
        {
            const QVector3D p(v.x,v.y,v.z);
            for (int k=0; k<3; ++k) { minimum[k]=qMin(minimum[k],p[k]); maximum[k]=qMax(maximum[k],p[k]); }
        }
    }
    geometry->setBounds(minimum,maximum); geometry->update();
}
}
ScenePiece::ScenePiece(const QString &id, QObject *parent) : QObject(parent),m_id(id)
{
    m_surface=new QQuick3DGeometry; m_surface->setParent(this);
    m_wire=new QQuick3DGeometry; m_wire->setParent(this);
    m_sewing=new QQuick3DGeometry; m_sewing->setParent(this);
}
void ScenePiece::synchronize(const Mesh &mesh, const Piece &piece, const QVector<Seam> &seams, int colorIndex)
{
    QVector<QVector3D> normals(mesh.vertices.size());
    QVector<quint32> triangles;
    for (const auto &t : mesh.triangles)
    {
        const auto normal=QVector3D::crossProduct(mesh.vertices[t[1]]-mesh.vertices[t[0]],mesh.vertices[t[2]]-mesh.vertices[t[0]]);
        for (int index : t) { normals[index]+=normal; triangles << quint32(index); }
    }
    QVector<Vertex> vertices;
    for (int i=0; i<mesh.vertices.size(); ++i)
        vertices << vertex(mesh.vertices[i],normals[i].lengthSquared()>1e-16f ? normals[i].normalized() : QVector3D(0,0,1));
    upload(m_surface,vertices,triangles,QQuick3DGeometry::PrimitiveType::Triangles);
    QVector<quint32> lines;
    for (const auto &edge : mesh.edges) lines << quint32(edge.a) << quint32(edge.b);
    upload(m_wire,vertices,lines,QQuick3DGeometry::PrimitiveType::Lines);

    // Small tubes remain legible with Metal, whose native line width is one pixel.
    const float radius=qMax(0.15f,(m_surface->boundsMax()-m_surface->boundsMin()).length()*0.0012f);
    QVector<Vertex> sewingVertices; QVector<quint32> sewingTriangles;
    for (const auto &seam : seams)
        for (int side=0; side<2; ++side)
        {
            if (!seam.enabled || (side==0 ? seam.a : seam.b)!=m_id) continue;
            const double start=side==0 ? seam.startA : seam.startB,end=side==0 ? seam.endA : seam.endB;
            const auto path=seamPath(mesh,start,end);
            for (int step=0; step+1<path.size(); ++step)
            {
                const auto a=path[step],b=path[step+1];
                const auto direction=(b-a).normalized(); if (direction.isNull()) continue;
                const auto helper=std::abs(direction.y())<0.9f ? QVector3D(0,1,0) : QVector3D(1,0,0);
                const auto u=QVector3D::crossProduct(direction,helper).normalized(),v=QVector3D::crossProduct(direction,u);
                const QVector3D offsets[]={u,v,-u,-v};
                const quint32 base=quint32(sewingVertices.size());
                for (const auto &point : {a,b}) for (const auto &normal : offsets) sewingVertices << vertex(point+normal*radius,normal);
                for (quint32 k=0; k<4; ++k)
                {
                    const quint32 next=(k+1)%4;
                    sewingTriangles << base+k << base+next << base+4+k << base+next << base+4+next << base+4+k;
                }
            }
        }
    upload(m_sewing,sewingVertices,sewingTriangles,QQuick3DGeometry::PrimitiveType::Triangles);
    const QString name=piece.name;
    const QColor color=QColor::fromHsv((colorIndex*71+185)%360,130,215);
    const bool hasSewing=!m_sewing->vertexData().isEmpty();
    if (name!=m_name || color!=m_color || hasSewing!=m_hasSewing)
    {
        m_name=name; m_color=color; m_hasSewing=hasSewing; emit appearanceChanged();
    }
}
}

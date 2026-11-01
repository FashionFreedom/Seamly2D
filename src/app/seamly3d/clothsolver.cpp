// SPDX-License-Identifier: GPL-3.0-or-later
#include "clothsolver.h"
#include "dressform.h"
#include <QMap>
#include <cmath>
#include <algorithm>
namespace Seamly3D
{
ClothSolver::ClothSolver(QVector<Mesh> &meshes,const Project &project) : m_settings(project.simulation)
{
    m_floor=project.dressForm.enabled ? dressFormFloor(project.dressForm)*0.001 : -0.5;
    m_clearance=float(project.dressForm.clearance);
    if (project.dressForm.enabled && project.dressForm.contact) m_profile=dressFormProfile(project.dressForm);
    for (int n=0;n<meshes.size();++n)
    {
        auto &mesh=meshes[n]; mesh.initial=mesh.vertices;
        QVector<double> mass(mesh.rest.size(),0); QMap<QPair<int,int>,int> opposites;
        for (const auto &t : mesh.triangles)
        {
            const double area=QVector3D::crossProduct(mesh.rest[t[1]]-mesh.rest[t[0]],mesh.rest[t[2]]-mesh.rest[t[0]]).length()*0.5e-6;
            for (int i : t) mass[i]+=area*(m_settings.density*0.001)/3;
            for (int k=0;k<3;++k)
            {
                const int a=t[k],b=t[(k+1)%3],opposite=t[(k+2)%3]; const auto key=qMakePair(qMin(a,b),qMax(a,b));
                if (opposites.contains(key)) { const int other=opposites[key]; mesh.bends<<Mesh::Edge{other,opposite,(mesh.rest[other]-mesh.rest[opposite]).length()}; }
                else opposites.insert(key,opposite);
            }
        }
        float top=-1e9; for (const auto &p : mesh.rest) top=qMax(top,p.y());
        float left=1e9,right=-1e9;
        for (const auto &p : mesh.rest) if (top-p.y()<0.01f) { left=qMin(left,p.x()); right=qMax(right,p.x()); }
        const float middle=(left+right)*0.5f,halfWidth=(right-left)*0.25f;
        for (int i=0;i<mass.size();++i)
            mesh.inverseMass<<((project.pieces[n].pinTop && top-mesh.rest[i].y()<0.01f && std::abs(mesh.rest[i].x()-middle)<=halfWidth+0.01f) ? 0.f : float(1/qMax(1e-10,mass[i])));
        m_velocity<<QVector<QVector3D>(mesh.rest.size());
    }
}
void ClothSolver::step(QVector<Mesh> &meshes,const QVector<Stitch> &stitches)
{
    if (m_stopped) return;
    constexpr float dt=1.f/240; constexpr int substeps=4,iterations=8;
    QVector<QVector<QVector3D>> frameStart; for (const auto &mesh : meshes) frameStart<<mesh.vertices;
    m_speed=0;
    for (int substep=0;substep<substeps;++substep)
    {
        QVector<QVector<QVector3D>> old,x;
        QVector<QVector<float>> edgeLambda,bendLambda;
        for (int n=0;n<meshes.size();++n)
        {
            const auto &mesh=meshes[n]; QVector<QVector3D> positions;
            for (const auto &p : mesh.vertices) positions<<p*0.001f;
            old<<positions;
            for (int i=0;i<positions.size();++i) if (mesh.inverseMass[i]>0)
            { m_velocity[n][i]+=QVector3D(0,-9.81f,0)*dt; positions[i]+=m_velocity[n][i]*dt; }
            x<<positions; edgeLambda<<QVector<float>(mesh.edges.size(),0); bendLambda<<QVector<float>(mesh.bends.size(),0);
        }
        QVector<float> seamLambda(stitches.size(),0);
        auto distanceConstraint=[&](QVector<QVector3D> &positions,const QVector<float> &mass,const Mesh::Edge &e,double compliance,float &lambda) {
            const auto delta=positions[e.b]-positions[e.a]; const float length=delta.length(),weight=mass[e.a]+mass[e.b];
            if (length<1e-10f || weight==0) return;
            const float alpha=float(compliance)/(dt*dt),dl=(-(length-e.length*0.001f)-alpha*lambda)/(weight+alpha);
            const auto correction=delta*(dl/length); lambda+=dl;
            positions[e.a]-=correction*mass[e.a]; positions[e.b]+=correction*mass[e.b];
        };
        for (int iteration=0;iteration<iterations;++iteration)
        {
            for (int n=0;n<meshes.size();++n)
            {
                const auto &mesh=meshes[n];
                for (int i=0;i<mesh.edges.size();++i) distanceConstraint(x[n],mesh.inverseMass,mesh.edges[i],m_settings.stretchCompliance,edgeLambda[n][i]);
                for (int i=0;i<mesh.bends.size();++i) distanceConstraint(x[n],mesh.inverseMass,mesh.bends[i],m_settings.bendCompliance,bendLambda[n][i]);
            }
            for (int i=0;i<stitches.size();++i)
            {
                const auto &s=stitches[i]; const auto p=s.u,q=s.v;
                const auto delta=x[s.b][q.first]*(1-q.t)+x[s.b][q.second]*q.t-x[s.a][p.first]*(1-p.t)-x[s.a][p.second]*p.t;
                const float length=delta.length(); if (length<1e-10f) continue;
                const auto &ma=meshes[s.a].inverseMass,&mb=meshes[s.b].inverseMass;
                const float w=ma[p.first]*(1-p.t)*(1-p.t)+ma[p.second]*p.t*p.t+mb[q.first]*(1-q.t)*(1-q.t)+mb[q.second]*q.t*q.t;
                if (w==0) continue;
                const float alpha=1e-8f/(dt*dt),dl=(-length-alpha*seamLambda[i])/(w+alpha); seamLambda[i]+=dl;
                const auto correction=delta*(dl/length);
                x[s.a][p.first]-=correction*(ma[p.first]*(1-p.t)); x[s.a][p.second]-=correction*(ma[p.second]*p.t);
                x[s.b][q.first]+=correction*(mb[q.first]*(1-q.t)); x[s.b][q.second]+=correction*(mb[q.second]*q.t);
            }
            for (int n=0;n<meshes.size();++n)
            {
                const auto &mesh=meshes[n];
                auto project=[&](const QVector3D &point) {
                    auto result=m_profile.isEmpty() ? point : outsideDressForm(m_profile,point*1000,m_clearance)*0.001f;
                    result.setY(qMax(float(m_floor+0.001),result.y())); return result;
                };
                if (!m_profile.isEmpty())
                {
                    const float samples[][3]={{1.f/3,1.f/3,1.f/3},{0.5f,0.5f,0},{0,0.5f,0.5f},{0.5f,0,0.5f}};
                    for (const auto &t : mesh.triangles) for (const auto &w : samples)
                    {
                        const auto point=x[n][t[0]]*w[0]+x[n][t[1]]*w[1]+x[n][t[2]]*w[2]; const auto delta=project(point)-point;
                        if (delta.lengthSquared()<1e-14f) continue;
                        float denominator=0; for (int k=0;k<3;++k) denominator+=mesh.inverseMass[t[k]]*w[k]*w[k];
                        if (denominator>0) for (int k=0;k<3;++k) x[n][t[k]]+=delta*(mesh.inverseMass[t[k]]*w[k]/denominator);
                    }
                }
                for (int i=0;i<x[n].size();++i) if (mesh.inverseMass[i]>0) x[n][i]=project(x[n][i]);
            }
        }
        for (int n=0;n<meshes.size();++n) for (int i=0;i<x[n].size();++i)
        {
            auto velocity=(x[n][i]-old[n][i])/dt;
            if (meshes[n].inverseMass[i]==0) velocity={};
            // Friction damps tangent motion on sampled body/floor contact.
            QVector3D normal;
            if (x[n][i].y()<=m_floor+0.00101) normal={0,1,0};
            if (!m_profile.isEmpty())
            { const auto near=outsideDressForm(m_profile,x[n][i]*1000,m_clearance+0.1f)*0.001f-x[n][i]; if (near.lengthSquared()>1e-14f) normal=near.normalized(); }
            if (!normal.isNull()) { const auto tangent=velocity-normal*QVector3D::dotProduct(velocity,normal); velocity-=tangent*float(m_settings.friction); }
            velocity*=float(std::exp(-m_settings.damping*dt));
            if (!std::isfinite(velocity.lengthSquared()) || velocity.length()>100 || !std::isfinite(x[n][i].lengthSquared()))
            {
                for (int k=0;k<meshes.size();++k) meshes[k].vertices=frameStart[k];
                m_stopped=true; m_status=QStringLiteral("Stopped: unstable constraints. Reset or improve placement."); return;
            }
            m_velocity[n][i]=velocity; meshes[n].vertices[i]=x[n][i]*1000; m_speed=qMax(m_speed,double(velocity.length()));
        }
    }
    m_time+=1.0/60;
    // Rest is based on actual velocity over a full second, never on the iteration cap.
    if (m_speed<0.002) ++m_quietFrames; else m_quietFrames=0;
    if (m_quietFrames>=60) { m_stopped=true; m_status=QStringLiteral("At rest (motion threshold)"); }
    else if (m_time>=20) { m_stopped=true; m_status=QStringLiteral("Time limit reached (20 s), not convergence"); }
}
}

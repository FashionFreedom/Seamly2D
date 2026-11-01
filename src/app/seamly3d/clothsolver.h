// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_CLOTHSOLVER_H
#define SEAMLY3D_CLOTHSOLVER_H
#include "model.h"
namespace Seamly3D
{
// Fixed-step XPBD prototype. Positions exposed by Mesh are mm; integration uses SI.
// Opposite-vertex distances approximate bending. No continuous/self collision.
class ClothSolver
{
public:
    ClothSolver(QVector<Mesh> &meshes,const Project &project);
    void step(QVector<Mesh> &meshes,const QVector<Stitch> &stitches);
    double time() const { return m_time; }
    double speed() const { return m_speed*1000; }
    bool stopped() const { return m_stopped; }
    QString status() const { return m_status; }
private:
    SimulationSettings m_settings;
    QVector<QVector<QVector3D>> m_velocity;
    QVector<QVector3D> m_profile;
    double m_time=0,m_speed=0,m_floor;
    float m_clearance;
    int m_quietFrames=0;
    bool m_stopped=false;
    QString m_status=QStringLiteral("Draping");
};
}
#endif

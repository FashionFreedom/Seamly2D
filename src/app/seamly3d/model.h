// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_MODEL_H
#define SEAMLY3D_MODEL_H

#include <QJsonObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QVector3D>
#include <array>
#include <memory>

namespace Seamly3D
{
constexpr int BoundarySamples = 64;
struct Piece
{
    QString id;
    QString name;
    QVector<QPointF> outline; // Evaluated sewing contour, millimetres, y up.
    QVector3D translation;
    QVector3D rotation; // Degrees about x, y, z.
    bool pinTop=false; // Fix the middle half of the highest original edge during cloth simulation.
};
struct Seam
{
    QString a, b;
    double startA = 0, endA = 16, startB = 0, endB = 16; // Continuous arc position in legacy 1/64 units.
    bool reverse = true;
    QString id;
    QString name;
    bool enabled = true;
};
struct DressForm
{
    bool enabled=false;
    bool contact=true;
    bool stand=true;
    double bust=920,waist=720,hips=960,neck=360,shoulders=380,height=760,clearance=3;
    bool operator==(const DressForm &other) const;
    bool operator!=(const DressForm &other) const { return !(*this==other); }
};
struct SimulationSettings
{
    bool physical=false;
    double density=200; // g/m²
    double stretchCompliance=0.000001, bendCompliance=0.01; // XPBD, SI units
    double damping=2, friction=0.4; // 1/s, dimensionless
};
struct Project
{
    QString source;
    QVector<Piece> pieces;
    QVector<Seam> seams;
    DressForm dressForm;
    double meshSize=25; // Maximum triangle edge length in mm.
    SimulationSettings simulation;
};
// All reads are transactional: invalid input leaves the destination untouched.
bool decode(const QJsonObject &json, Project &project, QString &error);
QJsonObject encode(const Project &project);
bool load(const QString &path, Project &project, QString &error);
bool save(const QString &path, const Project &project, QString &error);
QVector<QPointF> resample(const QVector<QPointF> &outline, int count = BoundarySamples);
double perimeter(const QVector<QPointF> &outline);
// Equal endpoints mean a full loop; otherwise traverse forward, wrapping through zero.
double arcSteps(double start, double end);
double seamLength(const Piece &piece, double start, double end);
QPointF contourPoint(const QVector<QPointF> &outline,double fraction);
Project demoProject();
Project dressFormDemoProject();
Project mergeGeometry(const Project &current, const Project &incoming);

struct Mesh
{
    QString id;
    QVector<QVector3D> rest;
    QVector<QVector3D> vertices;
    QVector<std::array<int, 3>> triangles;
    struct Edge { int a, b; float length; };
    QVector<Edge> edges;
    QVector<int> boundary;
    QVector<double> parameters; // Normalized original-outline arc positions, sorted.
    QVector<float> inverseMass;
    QVector<QVector3D> initial;
    QVector<Edge> bends; // Opposite-vertex distance across adjacent triangles.
};
struct BoundaryAnchor { int first=0,second=0; float t=0; };
BoundaryAnchor boundaryAnchor(const Mesh &mesh,double position);
QVector3D boundaryPoint(const Mesh &mesh,double position);
QVector<QVector3D> seamPath(const Mesh &mesh,double start,double end);
struct Stitch { int a,b; BoundaryAnchor u,v; };
class ClothSolver;
class Assembly
{
public:
    bool reset(const Project &project, QString &error);
    void step(int iterations = 6);
    double seamError() const;
    double meanStrain() const;
    const QVector<Mesh> &meshes() const { return m_meshes; }
    double simulationTime() const;
    double maximumSpeed() const;
    QString simulationStatus() const;
    bool simulationStopped() const;
private:
    std::shared_ptr<ClothSolver> m_cloth;
    QVector<Mesh> m_meshes;
    QVector<Stitch> m_stitches;
    DressForm m_dressForm;
    QVector<QVector3D> m_formProfile;
};
}
#endif

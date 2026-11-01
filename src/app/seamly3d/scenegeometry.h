// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY3D_SCENEGEOMETRY_H
#define SEAMLY3D_SCENEGEOMETRY_H
#include "model.h"
#include <QColor>
#include <QQuick3DGeometry>

namespace Seamly3D
{
// Rendering owns copies of accepted solver positions, never the simulation state.
class ScenePiece : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString pieceId READ pieceId CONSTANT)
    Q_PROPERTY(QString name READ name NOTIFY appearanceChanged)
    Q_PROPERTY(QColor color READ color NOTIFY appearanceChanged)
    Q_PROPERTY(QQuick3DGeometry *surface READ surface CONSTANT)
    Q_PROPERTY(QQuick3DGeometry *wire READ wire CONSTANT)
    Q_PROPERTY(QQuick3DGeometry *sewing READ sewing CONSTANT)
    Q_PROPERTY(bool hasSewing READ hasSewing NOTIFY appearanceChanged)
    Q_PROPERTY(bool shown READ shown NOTIFY inspectionChanged)
    Q_PROPERTY(bool ghosted READ ghosted NOTIFY inspectionChanged)
public:
    explicit ScenePiece(const QString &id, QObject *parent=nullptr);
    void synchronize(const Mesh &mesh, const Piece &piece, const QVector<Seam> &seams, int colorIndex);
    QString pieceId() const { return m_id; }
    QString name() const { return m_name; }
    QColor color() const { return m_color; }
    QQuick3DGeometry *surface() const { return m_surface; }
    QQuick3DGeometry *wire() const { return m_wire; }
    QQuick3DGeometry *sewing() const { return m_sewing; }
    bool hasSewing() const { return m_hasSewing; }
    bool shown() const { return m_shown; }
    bool ghosted() const { return m_ghosted; }
    void setInspection(bool shown,bool ghosted) {
        if (shown==m_shown && ghosted==m_ghosted) return;
        m_shown=shown; m_ghosted=ghosted; emit inspectionChanged();
    }
signals:
    void appearanceChanged();
    void inspectionChanged();
private:
    QString m_id,m_name;
    QColor m_color;
    QQuick3DGeometry *m_surface,*m_wire,*m_sewing;
    bool m_hasSewing=false;
    bool m_shown=true,m_ghosted=false;
};
}
#endif

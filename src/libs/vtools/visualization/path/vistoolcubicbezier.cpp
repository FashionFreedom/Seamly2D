/***************************************************************************
 **  @file   vistoolcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  Dual-mode visualization for VToolCubicBezier ("Kurve Fixiert"):
 **
 **  Creation mode (ChosenObject flow, object IDs set):
 **    Shows live feedback as the user clicks P1 → P2 → P3 → P4.
 **    P2/P3 are tracked as actual canvas points.
 **
 **  Show mode (after creation or editing, SetVisualization called):
 **    Uses precomputed angle/length values from the stored VCubicBezier.
 **    This correctly reflects States 2/3/4 where P2'/P3' differ from canvas.
 **************************************************************************/

#include "vistoolcubicbezier.h"

#include <QLineF>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vmisc/vabstractapplication.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCubicBezier::VisToolCubicBezier(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , object2Id(NULL_ID)
    , object3Id(NULL_ID)
    , object4Id(NULL_ID)
    , m_angle1(QString())
    , m_angle2(QString())
    , m_c1Length(QString())
    , m_c2Length(-1.0)
    , point1(nullptr)
    , point4(nullptr)
    , point2(nullptr)
    , point3(nullptr)
    , helpLine1(nullptr)
    , helpLine2(nullptr)
{
    helpLine1 = InitItem<VScaledLine>(mainColor, this);
    helpLine2 = InitItem<VScaledLine>(mainColor, this);
    point1    = InitPoint(supportColor, this);
    point4    = InitPoint(supportColor, this);
    point2    = InitPoint(mainColor,    this);
    point3    = InitPoint(Qt::gray,     this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezier::RefreshGeometry()
{
    if (object1Id <= NULL_ID)
        return;

    const auto p1 = Visualization::data->GeometricObject<VPointF>(object1Id);
    DrawPoint(point1, static_cast<QPointF>(*p1), supportColor);

    // -----------------------------------------------------------------------
    // SHOW MODE — object4Id is set AND angle1/c1Length are provided.
    // Uses precomputed geometry from SetVisualization() for States 2/3/4.
    // -----------------------------------------------------------------------
    if (object4Id != NULL_ID && !m_angle1.isEmpty() && !m_c1Length.isEmpty())
    {
        const auto p4 = Visualization::data->GeometricObject<VPointF>(object4Id);
        DrawPoint(point4, static_cast<QPointF>(*p4), supportColor);

        bool ok1, ok2, ok3;
        const qreal angle1 = m_angle1.toDouble(&ok1);
        const qreal c1Len  = qApp->toPixel(m_c1Length.toDouble(&ok2));
        const qreal angle2 = m_angle2.isEmpty() ? 0.0 : m_angle2.toDouble(&ok3);

        if (ok1 && ok2 && c1Len > 0.0)
        {
            QLineF h1(static_cast<QPointF>(*p1), static_cast<QPointF>(*p1) + QPointF(c1Len, 0));
            h1.setAngle(angle1);
            const VPointF p2(h1.p2());

            const qreal c2Len = (m_c2Length >= 0.0) ? m_c2Length : c1Len;
            QLineF h2(static_cast<QPointF>(*p4), static_cast<QPointF>(*p4) + QPointF(c2Len, 0));
            h2.setAngle(angle2);
            const VPointF p3(h2.p2());

            DrawLine(helpLine1, QLineF(static_cast<QPointF>(*p1), static_cast<QPointF>(p2)),
                     mainColor, lineWeight, Qt::DashLine);
            DrawLine(helpLine2, QLineF(static_cast<QPointF>(*p4), static_cast<QPointF>(p3)),
                     Qt::gray,  lineWeight, Qt::DashLine);
            DrawPoint(point2, static_cast<QPointF>(p2), mainColor);
            DrawPoint(point3, static_cast<QPointF>(p3), Qt::gray);

            VCubicBezier spline(*p1, p2, p3, *p4);
            DrawPath(this, spline.GetPath(), mainColor, lineStyle, lineWeight, Qt::RoundCap);
        }
        return;
    }

    // -----------------------------------------------------------------------
    // CREATION MODE — object IDs used for live 4-click feedback.
    // -----------------------------------------------------------------------
    if (object2Id <= NULL_ID)
    {
        // Only P1 selected: draw line to cursor
        DrawLine(helpLine1, QLineF(static_cast<QPointF>(*p1), Visualization::scenePos),
                 mainColor, lineWeight, Qt::DashLine);
        helpLine2->setVisible(false);
        point2->setVisible(false);
        point3->setVisible(false);
        point4->setVisible(false);
        return;
    }

    const auto p2pt = Visualization::data->GeometricObject<VPointF>(object2Id);
    DrawPoint(point2, static_cast<QPointF>(*p2pt), supportColor);
    DrawLine(helpLine1, QLineF(static_cast<QPointF>(*p1), static_cast<QPointF>(*p2pt)),
             mainColor, lineWeight, Qt::DashLine);

    if (object3Id <= NULL_ID)
    {
        // P1, P2 selected: show partial curve toward cursor
        VCubicBezier spline(*p1, *p2pt, VPointF(Visualization::scenePos),
                             VPointF(Visualization::scenePos));
        DrawPath(this, spline.GetPath(), mainColor, lineStyle, lineWeight, Qt::RoundCap);
        helpLine2->setVisible(false);
        point3->setVisible(false);
        point4->setVisible(false);
        return;
    }

    const auto p3pt = Visualization::data->GeometricObject<VPointF>(object3Id);
    DrawPoint(point3, static_cast<QPointF>(*p3pt), Qt::gray);

    if (object4Id <= NULL_ID)
    {
        // P1, P2, P3 selected: curve toward cursor for P4
        VCubicBezier spline(*p1, *p2pt, *p3pt, VPointF(Visualization::scenePos));
        DrawPath(this, spline.GetPath(), mainColor, lineStyle, lineWeight, Qt::RoundCap);
        DrawLine(helpLine2, QLineF(static_cast<QPointF>(*p3pt), Visualization::scenePos),
                 mainColor, lineWeight, Qt::DashLine);
        point4->setVisible(false);
        return;
    }

    // All 4 points selected
    const auto p4pt = Visualization::data->GeometricObject<VPointF>(object4Id);
    DrawPoint(point4, static_cast<QPointF>(*p4pt), supportColor);
    DrawLine(helpLine2, QLineF(static_cast<QPointF>(*p4pt), static_cast<QPointF>(*p3pt)),
             mainColor, lineWeight, Qt::DashLine);
    VCubicBezier spline(*p1, *p2pt, *p3pt, *p4pt);
    DrawPath(this, spline.GetPath(), spline.DirectionArrows(), mainColor, lineStyle,
             lineWeight, Qt::RoundCap);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezier::setObject2Id(const quint32 &value) { object2Id = value; }
void VisToolCubicBezier::setObject3Id(const quint32 &value) { object3Id = value; }
void VisToolCubicBezier::setObject4Id(const quint32 &value) { object4Id = value; }
void VisToolCubicBezier::setAngle1(const QString &value)    { m_angle1   = value; }
void VisToolCubicBezier::setAngle2(const QString &value)    { m_angle2   = value; }
void VisToolCubicBezier::setC1Length(const QString &value)  { m_c1Length = value; }
void VisToolCubicBezier::setC2Length(qreal value)           { m_c2Length = value; }

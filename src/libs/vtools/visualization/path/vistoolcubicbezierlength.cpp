/***************************************************************************
 **  @file   vistoolcubicbezierlength.cpp
 **  @author Seamly2D contributors
 **
 **  @brief  Visualization for the cubic Bézier curve with matched length tool.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#include "vistoolcubicbezierlength.h"

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
VisToolCubicBezierLength::VisToolCubicBezierLength(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , object4Id(NULL_ID)
    , m_angle1(QString())
    , m_angle2(QString())
    , m_c1Length(QString())
    , m_targetLength(QString())
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
void VisToolCubicBezierLength::RefreshGeometry()
{
    if (object1Id <= NULL_ID)
        return;

    const auto p1 = Visualization::data->GeometricObject<VPointF>(object1Id);
    DrawPoint(point1, static_cast<QPointF>(*p1), supportColor);

    if (object4Id <= NULL_ID)
    {
        // Still selecting end point: draw a helper line to the cursor
        DrawLine(helpLine1, QLineF(static_cast<QPointF>(*p1), Visualization::scenePos),
                 mainColor, lineWeight, Qt::DashLine);
        return;
    }

    const auto p4 = Visualization::data->GeometricObject<VPointF>(object4Id);
    DrawPoint(point4, static_cast<QPointF>(*p4), supportColor);

    // Evaluate parameters if available
    bool ok1 = false, ok2 = false, ok3 = false;
    const qreal angle1   = m_angle1.isEmpty()      ? 0.0 : m_angle1.toDouble(&ok1);
    const qreal c1Len    = m_c1Length.isEmpty()    ? 0.0 : qApp->toPixel(m_c1Length.toDouble(&ok2));
    const qreal angle2   = m_angle2.isEmpty()      ? 0.0 : m_angle2.toDouble(&ok3);

    if (!ok1 || !ok2 || !ok3 || c1Len <= 0.0)
    {
        // Parameters not yet set — just show a line from P1 to P4
        DrawLine(helpLine1, QLineF(static_cast<QPointF>(*p1), static_cast<QPointF>(*p4)),
                 mainColor, lineWeight, Qt::DashLine);
        point2->setVisible(false);
        point3->setVisible(false);
        helpLine2->setVisible(false);
        return;
    }

    // Build P2 from P1 + handle1
    QLineF h1(static_cast<QPointF>(*p1), static_cast<QPointF>(*p1) + QPointF(c1Len, 0));
    h1.setAngle(angle1);
    const VPointF p2(h1.p2());

    // Build P3 from P4 + handle2.
    // In Show mode use the actual computed c2Length; during placement use c1Len as approximation.
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

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setObject4Id(const quint32 &value)
{
    object4Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setAngle1(const QString &value)
{
    m_angle1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setAngle2(const QString &value)
{
    m_angle2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setC1Length(const QString &value)
{
    m_c1Length = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setTargetLength(const QString &value)
{
    m_targetLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezierLength::setC2Length(qreal value)
{
    m_c2Length = value;
}

//---------------------------------------------------------------------------------------------------------------------

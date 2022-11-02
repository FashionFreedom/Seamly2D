/**************************************************************************
 **
 **  @file   point_intersectxy_visual.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 8, 2014
 **
 **  @author Douglas S Caskey
 **  @date   7.21.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "point_intersectxy_visual.h"

#include "visline.h"
#include "../visualization.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QLine>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

//---------------------------------------------------------------------------------------------------------------------
PointIntersectXYVisual::PointIntersectXYVisual(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
    , point2Id(NULL_ID)
    , point(nullptr)
    , axisP1(nullptr)
    , axisP2(nullptr)
    , axis2(nullptr)
{
    axisP1 = InitPoint(supportColor, this);
    axisP2 = InitPoint(supportColor, this); //-V656
    axis2  = InitItem<VScaledLine>(supportColor, this);
    point  = InitPoint(mainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYVisual::RefreshGeometry()
{
    QLineF axisL1;
    if (object1Id <= NULL_ID)
    {
        axisL1 = Axis(Visualization::scenePos, 90);
        DrawLine(this, axisL1, supportColor, Qt::DashLine);
    }
    else
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(axisP1, static_cast<QPointF>(*first), supportColor);

        axisL1 = Axis(static_cast<QPointF>(*first), 90);
        DrawLine(this, axisL1, supportColor, Qt::DashLine);

        QLineF axisL2;
        if (point2Id <= NULL_ID)
        {
            axisL2 = Axis(Visualization::scenePos, 180);
            showIntersection(axisL1, axisL2, supportColor);
        }
        else
        {
            const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(point2Id);
            DrawPoint(axisP2, static_cast<QPointF>(*second), supportColor);
            axisL2 = Axis(static_cast<QPointF>(*second), 180);
            showIntersection(axisL1, axisL2, mainColor);
        }
        DrawLine(axis2, axisL2, supportColor, Qt::DashLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYVisual::setPoint1Id(const quint32 &value)
{
    object1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYVisual::setPoint2Id(const quint32 &value)
{
    point2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYVisual::showIntersection(const QLineF &axis1, const QLineF &axis2, const QColor &color)
{
    QPointF p;
    QLineF::IntersectType intersect = axis1.intersects(axis2, &p);
    if (intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection)
    {
        point->setVisible(true);
        DrawPoint(point, p, color);
    }
    else
    {
        point->setVisible(false);
    }
}

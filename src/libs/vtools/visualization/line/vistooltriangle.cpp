/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   vistooltriangle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#include "vistooltriangle.h"

#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolTriangle::VisToolTriangle(const VContainer *data, QGraphicsItem *parent)
    :VisLine(data, parent), object2Id(NULL_ID), hypotenuseP1Id(NULL_ID), hypotenuseP2Id(NULL_ID), point(nullptr),
      axisP1(nullptr), axisP2(nullptr), axis(nullptr), hypotenuseP1(nullptr), hypotenuseP2(nullptr), foot1(nullptr),
      foot2(nullptr)
{
    axisP1 = InitPoint(supportColor, this);
    axisP2 = InitPoint(supportColor, this);
    axis = InitItem<ArrowedLineItem>(supportColor, this);
    hypotenuseP1 = InitPoint(supportColor, this);
    hypotenuseP2 = InitPoint(supportColor, this);
    foot1 = InitItem<VScaledLine>(supportColor, this);
    foot2 = InitItem<VScaledLine>(supportColor, this); //-V656

    point = InitPoint(mainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(axisP1, static_cast<QPointF>(*first), supportColor);

        if (object2Id <= NULL_ID)
        {
            drawArrowedLine(axis, QLineF(static_cast<QPointF>(*first), Visualization::scenePos), supportColor);
        }
        else
        {
            const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(object2Id);
            DrawPoint(axisP2, static_cast<QPointF>(*second), supportColor);

            drawArrowedLine(axis, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)), supportColor);

            if (hypotenuseP1Id <= NULL_ID)
            {
                return;
            }
            else
            {
                const QSharedPointer<VPointF> third = Visualization::data->GeometricObject<VPointF>(hypotenuseP1Id);
                DrawPoint(hypotenuseP1, static_cast<QPointF>(*third), supportColor);

                if (hypotenuseP2Id <= NULL_ID)
                {
                    DrawLine(this, QLineF(static_cast<QPointF>(*third), Visualization::scenePos), supportColor,
                             Qt::DashLine);

                    QPointF trPoint = VToolTriangle::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               static_cast<QPointF>(*third), Visualization::scenePos);
                    DrawPoint(point, trPoint, mainColor);

                    DrawLine(foot1, QLineF(static_cast<QPointF>(*third), trPoint), supportColor, Qt::DashLine);
                    DrawLine(foot2, QLineF(Visualization::scenePos, trPoint), supportColor, Qt::DashLine);
                }
                else
                {
                    const QSharedPointer<VPointF> forth = Visualization::data->GeometricObject<VPointF>(hypotenuseP2Id);
                    DrawPoint(hypotenuseP2, static_cast<QPointF>(*forth), supportColor);

                    DrawLine(this, QLineF(static_cast<QPointF>(*third), static_cast<QPointF>(*forth)), supportColor,
                                          Qt::DashLine);

                    QPointF trPoint = VToolTriangle::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               static_cast<QPointF>(*third),
                                                               static_cast<QPointF>(*forth));
                    DrawPoint(point, trPoint, mainColor);

                    DrawLine(foot1, QLineF(static_cast<QPointF>(*third), trPoint), supportColor, Qt::DashLine);
                    DrawLine(foot2, QLineF(static_cast<QPointF>(*forth), trPoint), supportColor, Qt::DashLine);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::setObject2Id(const quint32 &value)
{
    object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::setHypotenuseP1Id(const quint32 &value)
{
    hypotenuseP1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::setHypotenuseP2Id(const quint32 &value)
{
    hypotenuseP2Id = value;
}

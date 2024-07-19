/**************************************************************************
 **
 **  @file   intersect_circletangent_visual.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2015
 **
 **  @author Douglas S. Caskey
 **  @date   7.16.2022
 **
 **  @copyright
 **  Copyright (C) 2013-2022 Seamly2D project.
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published
 **  by the Free Software Foundation, either version 3 of the License,
 **  or (at your option) any later version.
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

#include "intersect_circletangent_visual.h"

#include "visline.h"
#include "../ifc/ifcdef.h"
#include "../visualization.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../../tools/drawTools/toolpoint/toolsinglepoint/intersect_circletangent_tool.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QSharedPointer>
#include <Qt>
#include <new>

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentVisual::IntersectCircleTangentVisual(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
    , object2Id(NULL_ID)
    , cRadius(0)
    , crossPoint(CrossCirclesPoint::FirstPoint)
    , point(nullptr)
    , tangent(nullptr)
    , cCenter(nullptr)
    , cPath(nullptr)
    , tangent2(nullptr)
    , m_secondarySupportColor(QColor(qApp->Settings()->getSecondarySupportColor()))

{
    cPath    = InitItem<QGraphicsEllipseItem>(m_secondarySupportColor, this);
    point    = InitPoint(mainColor, this);
    tangent  = InitPoint(supportColor, this);
    cCenter  = InitPoint(supportColor, this); //-V656
    tangent2 = InitItem<VScaledLine>(supportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentVisual::RefreshGeometry()
{
    if (object1Id > NULL_ID)// tangent point
    {
        const QSharedPointer<VPointF> tan = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(tangent, static_cast<QPointF>(*tan), supportColor);

        if (object2Id > NULL_ID)// circle center
        {
            const QSharedPointer<VPointF> center = Visualization::data->GeometricObject<VPointF>(object2Id);
            DrawPoint(cCenter, static_cast<QPointF>(*center), supportColor);

            if (cRadius > 0)
            {
                cPath->setRect(pointRect(cRadius));
                DrawPoint(cPath, static_cast<QPointF>(*center), m_secondarySupportColor, Qt::DashLine);

                FindRays(static_cast<QPointF>(*tan), static_cast<QPointF>(*center), cRadius);

                const QPointF fPoint = IntersectCircleTangentTool::FindPoint(static_cast<QPointF>(*tan),
                                                                                 static_cast<QPointF>(*center),
                                                                                 cRadius, crossPoint);
                DrawPoint(point, fPoint, mainColor);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentVisual::setObject2Id(const quint32 &value)
{
    object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentVisual::setCRadius(const QString &value)
{
    cRadius = FindLength(value, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentVisual::setCrossPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentVisual::FindRays(const QPointF &p, const QPointF &center, qreal radius)
{
    QPointF p1, p2;
    const int res = VGObject::ContactPoints (p, center, radius, p1, p2);

    switch(res)
    {
        case 2:
            DrawRay(this, p, p1, supportColor, Qt::DashLine);
            DrawRay(tangent2, p, p2, supportColor, Qt::DashLine);
            break;
        case 1:
            DrawRay(this, p, p1, supportColor, Qt::DashLine);
            tangent2->setVisible(false);
            break;
        default:
            this->setVisible(false);
            tangent2->setVisible(false);
            break;
    }
}

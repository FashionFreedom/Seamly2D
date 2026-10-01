//---------------------------------------------------------------------------------------------------------------------
//  @file   intersect_arc_line_visual.cpp
//  @author Douglas S Caskey
//  @date   25 Srp, 2026
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vistoolpointofcontact.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   14 Aug, 2014
//
//  @copyright
//  Copyright (C) 2014 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "intersect_arc_line_visual.h"

#include <QGraphicsEllipseItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/intersect_arc_line_tool.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
IntersectArcLineVisual::IntersectArcLineVisual(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
    , m_line_point2_id(NULL_ID)
    , m_radius_id(NULL_ID)
    , m_point(nullptr)
    , m_line_point1(nullptr)
    , m_line_point2(nullptr)
    , m_arc_point(nullptr)
    , m_circle(nullptr)
    , m_radius(0)
{
    m_arc_point   = InitPoint(supportColor, this);
    m_line_point1 = InitPoint(supportColor, this);
    m_line_point2 = InitPoint(supportColor, this);
    m_circle      = InitItem<QGraphicsEllipseItem>(supportColor, this);
    m_point       = InitPoint(mainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineVisual::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(m_line_point1, static_cast<QPointF>(*first), supportColor);

        if (m_line_point2_id <= NULL_ID)
        {
            DrawLine(this, QLineF(static_cast<QPointF>(*first), Visualization::scenePos), supportColor, lineWeight);
        }
        else
        {
            const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(m_line_point2_id);
            DrawPoint(m_line_point2, static_cast<QPointF>(*second), supportColor);
            DrawLine(this, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)),
                                  supportColor, lineWeight);

            if (m_radius_id > NULL_ID)
            {
                const QSharedPointer<VPointF> third = Visualization::data->GeometricObject<VPointF>(m_radius_id);
                DrawPoint(m_arc_point, static_cast<QPointF>(*third), supportColor);

                if (!qFuzzyIsNull(m_radius))
                {
                    QPointF found_point = IntersectArcLineTool::FindPoint(m_radius, static_cast<QPointF>(*third),
                                                                         static_cast<QPointF>(*first),
                                                                         static_cast<QPointF>(*second));
                    DrawPoint(m_point, found_point, mainColor);

                    m_circle->setRect(pointRect(m_radius));
                    DrawPoint(m_circle, static_cast<QPointF>(*third), supportColor, Qt::DashLine);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineVisual::setLineP2Id(const quint32 &value)
{
    m_line_point2_id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineVisual::setRadiusId(const quint32 &value)
{
    m_radius_id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineVisual::setRadius(const QString &expression)
{
    m_radius = FindLength(expression, Visualization::data->DataVariables());
}

/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "vistoolmirrorbyaxis.h"
#include "../vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolMirrorByAxis::VisToolMirrorByAxis(const VContainer *data, QGraphicsItem *parent)
    : VisOperation(data, parent)
    , m_axisType(AxisType::VerticalAxis)
    , point1(nullptr)
{
    point1 = InitPoint(supportColor2, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMirrorByAxis::RefreshGeometry()
{
    if (objects.isEmpty())
    {
        return;
    }

    QPointF firstPoint;
    QPointF secondPoint;

    if (object1Id != NULL_ID)
    {
        firstPoint = static_cast<QPointF>(*Visualization::data->GeometricObject<VPointF>(object1Id));
        DrawPoint(point1, firstPoint, supportColor2);

        if (m_axisType == AxisType::VerticalAxis)
        {
            secondPoint = QPointF(firstPoint.x(), firstPoint.y() + 100);
        }
        else
        {
            secondPoint = QPointF(firstPoint.x() + 100, firstPoint.y());
        }

        DrawLine(this, Axis(firstPoint, secondPoint), supportColor2, Qt::DashLine);
    }

    refreshMirroredObjects(firstPoint, secondPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMirrorByAxis::setOriginPointId(quint32 value)
{
    object1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMirrorByAxis::setAxisType(AxisType value)
{
    m_axisType = value;
}

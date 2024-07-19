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
 **  @file   vistoolcutsplinepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 9, 2014
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

#include "vistoolcutsplinepath.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutSplinePath::VisToolCutSplinePath(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
    , point(nullptr)
    , splPath1(nullptr)
    , splPath2(nullptr)
    , m_length(0)
    , m_direction("forward")
{
    splPath1 = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    splPath1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    splPath2 = InitItem<VCurvePathItem>(Qt::darkRed, this);
    splPath2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    point = InitPoint(mainColor, this);
    point->setZValue(2);
    point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSplinePath::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const auto splPath = Visualization::data->GeometricObject<VAbstractCubicBezierPath>(object1Id);
        DrawPath(this, splPath->GetPath(), splPath->DirectionArrows(), supportColor, lineStyle,
                 lineWeight, Qt::RoundCap);

        if (m_direction == "backward")
        {
            m_length = splPath->GetLength() - m_length;
        }

        if (!qFuzzyIsNull(m_length))
        {
            VSplinePath *spPath1 = nullptr;
            VSplinePath *spPath2 = nullptr;
            VPointF *p = VToolCutSplinePath::CutSplinePath(m_length, splPath, "X", &spPath1, &spPath2);
            SCASSERT(p != nullptr)
            SCASSERT(spPath1 != nullptr)
            SCASSERT(spPath2 != nullptr)

            DrawPoint(point, static_cast<QPointF>(*p), mainColor);
            delete p;

            DrawPath(splPath1, spPath1->GetPath(), spPath1->DirectionArrows(), Qt::darkGreen, lineStyle,
                     lineWeight, Qt::RoundCap);
            DrawPath(splPath2, spPath2->GetPath(), spPath2->DirectionArrows(), Qt::darkRed, lineStyle,
                     lineWeight, Qt::RoundCap);

            delete spPath1;
            delete spPath2;
        }
    }
}

void VisToolCutSplinePath::setDirection(const QString &direction)
{
    m_direction = direction;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSplinePath::setLength(const QString &expression)
{
    m_length = FindLength(expression, Visualization::data->DataVariables());
}

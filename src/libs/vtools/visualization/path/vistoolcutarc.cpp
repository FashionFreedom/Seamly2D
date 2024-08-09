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
 **  @file   vistoolcutarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#include "vistoolcutarc.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutArc::VisToolCutArc(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , point(nullptr)
    , arc1(nullptr)
    , arc2(nullptr)
    , m_length(0)
    , m_direction("forward")
{
    arc1 = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    arc1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    arc2 = InitItem<VCurvePathItem>(Qt::darkRed, this);
    arc2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    point = InitPoint(mainColor, this);
    point->setZValue(2);
    point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VArc> arc = Visualization::data->GeometricObject<VArc>(object1Id);
        DrawPath(this, arc->GetPath(), arc->DirectionArrows(), supportColor, lineStyle, lineWeight, Qt::RoundCap);

        if (m_direction == "backward")
        {
            m_length = arc->GetLength() - m_length;
        }

        if (!qFuzzyIsNull(m_length))
        {
            VArc ar1;
            VArc ar2;
            QPointF p = arc->CutArc(m_length, ar1, ar2);
            DrawPoint(point, p, mainColor);

            DrawPath(arc1, ar1.GetPath(), ar1.DirectionArrows(), Qt::darkGreen, lineStyle, lineWeight, Qt::RoundCap);
            DrawPath(arc2, ar2.GetPath(), ar2.DirectionArrows(), Qt::darkRed, lineStyle, lineWeight, Qt::RoundCap);
        }
    }
}

void VisToolCutArc::setDirection(const QString &direction)
{
    m_direction = direction;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::setLength(const QString &expression)
{
    m_length = FindLength(expression, Visualization::data->DataVariables());
}

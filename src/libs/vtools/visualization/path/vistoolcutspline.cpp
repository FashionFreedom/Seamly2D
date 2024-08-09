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
 **  @file   vistoolcutspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 9, 2014
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

#include "vistoolcutspline.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutSpline::VisToolCutSpline(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , point(nullptr)
    , spl1(nullptr)
    , spl2(nullptr)
    , m_length(0)
    , m_direction("")
{
    spl1 = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    spl1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    spl2 = InitItem<VCurvePathItem>(Qt::darkRed, this);
    spl2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    point = InitPoint(mainColor, this);
    point->setZValue(2);
    point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSpline::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const auto spl = Visualization::data->GeometricObject<VAbstractCubicBezier>(object1Id);
        DrawPath(this, spl->GetPath(), spl->DirectionArrows(), supportColor, lineStyle, lineWeight, Qt::RoundCap);
        qreal length = m_length;
        if (m_direction == "backward")
        {
            length = spl->GetLength() - m_length;
        }

        if (!qFuzzyIsNull(length))
        {
            QPointF spl1p2;
            QPointF spl1p3;
            QPointF spl2p2;
            QPointF spl2p3;
            const QPointF p = spl->CutSpline(length, spl1p2, spl1p3, spl2p2, spl2p3);

            const VSpline sp1 = VSpline(spl->GetP1(), spl1p2, spl1p3, VPointF(p));
            const VSpline sp2 = VSpline(VPointF(p), spl2p2, spl2p3, spl->GetP4());

            DrawPoint(point, p, mainColor);

            DrawPath(spl1, sp1.GetPath(), sp1.DirectionArrows(), Qt::darkGreen, lineStyle, lineWeight, Qt::RoundCap);
            DrawPath(spl2, sp2.GetPath(), sp2.DirectionArrows(), Qt::darkRed, lineStyle, lineWeight, Qt::RoundCap);
        }
    }
}

void VisToolCutSpline::setDirection(const QString &direction)
{
    m_direction = direction;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSpline::setLength(const QString &expression)
{
    m_length = FindLength(expression, Visualization::data->DataVariables());
}

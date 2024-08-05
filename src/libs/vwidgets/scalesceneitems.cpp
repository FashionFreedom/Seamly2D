//  @file   scalesceneitems.cpp
//  @author Douglas S Caskey
//  @date   22 Jun, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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

/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#include "scalesceneitems.h"
#include "vcurvepathitem.h"
#include "global.h"

#include <QtCore/qmath.h>
#include <QPen>

//---------------------------------------------------------------------------------------------------------------------
VScaledLine::VScaledLine(QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
    , m_defaultWidth(widthMainLine)
{}

//---------------------------------------------------------------------------------------------------------------------
VScaledLine::VScaledLine(const QLineF &line, QGraphicsItem *parent)
    : QGraphicsLineItem(line, parent)
    , m_defaultWidth(widthMainLine)
{}

//---------------------------------------------------------------------------------------------------------------------
void VScaledLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen linePen = pen();
    linePen.setWidthF(scaleWidth(m_defaultWidth, sceneScale(scene())));
    setPen(linePen);

    QGraphicsLineItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VScaledLine::getDefaultWidth() const
{
    return m_defaultWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VScaledLine::setDefaultWidth(const qreal &value)
{
    m_defaultWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
ArrowedLineItem::ArrowedLineItem(QGraphicsItem *parent)
    : QGraphicsLineItem(parent)
    , m_arrows(new VCurvePathItem(this))
{}

//---------------------------------------------------------------------------------------------------------------------
ArrowedLineItem::ArrowedLineItem(const QLineF &line, QGraphicsItem *parent)
    : QGraphicsLineItem(line, parent)
    , m_arrows(new VCurvePathItem(this))
{}

//---------------------------------------------------------------------------------------------------------------------
void ArrowedLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen linePen = pen();
    linePen.setWidthF(scaleWidth(widthMainLine, sceneScale(scene())));
    setPen(linePen);
    m_arrows->setPen(linePen);

    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());

    qreal arrow_step = 60;
    qreal arrow_size = 10;

    if (line().length() < arrow_step)
    {
        drawArrow(line(), path, arrow_size);
    }

    QLineF axis;
    axis.setP1(line().p1());
    axis.setAngle(line().angle());
    axis.setLength(arrow_step);

    int steps = qFloor(line().length()/arrow_step);
    for (int i=0; i<steps; ++i)
    {
        drawArrow(axis, path, arrow_size);
        axis.setLength(axis.length()+arrow_step);
    }
    m_arrows->setPath(path);

    QGraphicsLineItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void ArrowedLineItem::drawArrow(const QLineF &axis, QPainterPath &path, const qreal &arrow_size)
{
    QLineF arrowPart1;
    arrowPart1.setP1(axis.p2());
    arrowPart1.setLength(arrow_size);
    arrowPart1.setAngle(axis.angle()+180+35);

    path.moveTo(arrowPart1.p1());
    path.lineTo(arrowPart1.p2());

    QLineF arrowPart2;
    arrowPart2.setP1(axis.p2());
    arrowPart2.setLength(arrow_size);
    arrowPart2.setAngle(axis.angle()+180-35);

    path.moveTo(arrowPart2.p1());
    path.lineTo(arrowPart2.p2());
}

//---------------------------------------------------------------------------------------------------------------------
VScaledEllipse::VScaledEllipse(QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
{}

//---------------------------------------------------------------------------------------------------------------------
void VScaledEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal scale = sceneScale(scene());
    const qreal width = scaleWidth(widthMainLine, scale);

    QPen visPen = pen();
    visPen.setWidthF(width);

    setPen(visPen);
    scaleCircleSize(this, scale);

    QGraphicsEllipseItem::paint(painter, option, widget);
}

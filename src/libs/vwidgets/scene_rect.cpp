// @file   scene_rect.cpp
// @author DSCaskey
// @date   3.30.2021
//
// @copyright
// Copyright (C) 2017 - 2024 Seamly, LLC
// https://github.com/fashionfreedom/seamly2d
//
// @brief
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

#include "scene_rect.h"

#include "global.h"
#include "scalesceneitems.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vgeometry/vpointf.h"

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QtDebug>

//---------------------------------------------------------------------------------------------------------------------
SceneRect::SceneRect(const QColor &lineColor, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_rectColor(QColor(correctColor(this,lineColor)))
    , m_onlyPoint(false)
    , m_isHovered(false)
    , m_showPointName(true)
{
    setZValue(100);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal scale = sceneScale(scene());

    setRectPen(scale);
    scaleRectSize(this, scale * .75);

    QGraphicsRectItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::refreshPointGeometry(const VPointF &point)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setPos(static_cast<QPointF>(point));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::setOnlyPoint(bool value)
{
    m_onlyPoint = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool SceneRect::isOnlyPoint() const
{
    return m_onlyPoint;
}

void SceneRect::setPointColor(const QString &value)
{
    m_rectColor = QColor(value);
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    QGraphicsRectItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    QGraphicsRectItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void SceneRect::setRectPen(qreal scale)
{
    const qreal width = scaleWidth(m_isHovered ? widthMainLine : widthHairLine, scale);

    QColor brushColor = m_rectColor;
    brushColor.setAlpha(100);

    setPen(QPen(m_rectColor, width));
    if (!qApp->Settings()->isWireframe())
    {
       setBrush(QBrush(brushColor, Qt::SolidPattern));
    }
    else
    {
       setBrush(QBrush(brushColor, Qt::NoBrush));
    }
}

/***************************************************************************
 *                                                                         *
 *   @file   scene_rect.cpp                                                *
 *   @author DSCaskey                                                      *
 *   @date   3.30.2021                                                     *
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
 **************************************************************************/

#include "scene_rect.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vgeometry/vpointf.h"
#include "global.h"
#include "scalesceneitems.h"

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QtDebug>

//---------------------------------------------------------------------------------------------------------------------
SceneRect::SceneRect(const QColor &lineColor, QGraphicsItem *parent)
    : QGraphicsRectItem(parent)
    , m_pointLeader(new VScaledLine(this))
    , m_pointColor(QColor(correctColor(this,lineColor)))
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

    setPointPen(scale);
    scaleCircleSize(this, scale * .75);

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
    m_pointColor = QColor(value);
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
void SceneRect::setPointPen(qreal scale)
{
    const qreal width = scaleWidth(m_isHovered ? widthMainLine : widthHairLine, scale);

    if (qApp->Settings()->getUseToolColor() || isOnlyPoint())
    {
        setPen(QPen(correctColor(this, m_pointColor), width));
        if (!m_onlyPoint)
        {
            setBrush(QBrush(correctColor(this, m_pointColor),Qt::SolidPattern));
        }
        else
        {
            setBrush(QBrush(correctColor(this, m_pointColor),Qt::NoBrush));
        }
    }
    else
    {
        setPen(QPen(correctColor(this, QColor(qApp->Settings()->getPointNameColor())), width));
        setBrush(QBrush(correctColor(this, QColor(qApp->Settings()->getPointNameColor())),Qt::SolidPattern));
    }
}

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
 **  @date   14 6, 2017
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

#include "vscenepoint.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vgeometry/vpointf.h"
#include "global.h"
#include "vgraphicssimpletextitem.h"
#include "scalesceneitems.h"

#include <QBrush>
#include <QFont>
#include <QPen>
#include <QColor>
#include <QtDebug>

//---------------------------------------------------------------------------------------------------------------------
VScenePoint::VScenePoint(const QColor &lineColor, QGraphicsItem *parent)
    : QGraphicsEllipseItem(parent)
    , m_pointName(new VGraphicsSimpleTextItem(m_pointColor, this))
    , m_pointLeader(new VScaledLine(this))
    , m_pointColor(QColor(correctColor(this,lineColor)))
    , m_onlyPoint(false)
    , m_isHovered(false)
    , m_showPointName(true)
{
    m_pointLeader->setBasicWidth(widthHairLine);
    m_pointLeader->setLine(QLineF(0, 0, 1, 0));
    m_pointLeader->setVisible(false);

    setZValue(100);
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal scale = sceneScale(scene());

    setPointPen(scale);
    scaleCircleSize(this, scale * .75);

    if (qApp->Settings()->getPointNameSize()*scale < 6 || !qApp->Settings()->getHidePointNames())
    {
        m_pointName->setVisible(false);
        m_pointLeader->setVisible(false);
    }
    else
    {
        if (!m_onlyPoint)
        {
            m_pointName->setVisible(m_showPointName);

            QPen leaderPen = m_pointLeader->pen();
            if (qApp->Settings()->getUseToolColor())
            {
                QColor leaderColor = correctColor(m_pointLeader, m_pointColor);
                leaderColor.setAlpha(128);
                leaderPen.setColor(leaderColor);
            }
            else
            {
                QColor leaderColor = correctColor(m_pointLeader, QColor(qApp->Settings()->getPointNameColor()));
                leaderColor.setAlpha(128);
                leaderPen.setColor(leaderColor);
            }
            m_pointLeader->setPen(leaderPen);

            refreshLeader();
        }
    }

    QGraphicsEllipseItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::refreshPointGeometry(const VPointF &point)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setPos(static_cast<QPointF>(point));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    m_showPointName = point.isShowPointName();

    m_pointName->blockSignals(true);
    m_pointName->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    m_pointName->setPosition(QPointF(point.mx(), point.my()));
    m_pointName->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    m_pointName->blockSignals(false);
    m_pointName->setText(point.name());
    m_pointName->setTextColor(m_pointColor);
    m_pointName->setVisible(m_showPointName);

    refreshLeader();
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::setOnlyPoint(bool value)
{
    m_onlyPoint = value;
    m_pointName->setVisible(!m_onlyPoint);
    m_pointLeader->setVisible(!m_onlyPoint);
}

//---------------------------------------------------------------------------------------------------------------------
bool VScenePoint::isOnlyPoint() const
{
    return m_onlyPoint;
}

void VScenePoint::setPointColor(const QString &value)
{
    m_pointColor = QColor(value);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::refreshLeader()
{
    QRectF nRect = m_pointName->sceneBoundingRect();
    nRect.translate(- scenePos());
    if (not rect().intersects(nRect))
    {
        const QRectF nameRect = m_pointName->sceneBoundingRect();
        QPointF p1;
        QPointF p2;
        VGObject::LineIntersectCircle(QPointF(), scaledRadius(sceneScale(scene())),
                                      QLineF(QPointF(), nameRect.center() - scenePos()), p1, p2);
        const QPointF pointRect = VGObject::LineIntersectRect(nameRect, QLineF(scenePos(), nameRect.center()));

        if (QLineF(p1, pointRect - scenePos()).length() <= ToPixel(4/qMax(1.0, sceneScale(scene())), Unit::Mm))
        {
            m_pointLeader->setVisible(false);
        }
        else
        {
            m_pointLeader->setLine(QLineF(p1, pointRect - scenePos()));
            m_pointLeader->setVisible(m_showPointName);
        }
    }
    else
    {
        m_pointLeader->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::setPointPen(qreal scale)
{
    const qreal width = scaleWidth(m_isHovered ? widthMainLine : widthHairLine, scale);

    if (qApp->Settings()->getUseToolColor() || isOnlyPoint())
    {
        setPen(QPen(correctColor(this, m_pointColor), width));
        if (!m_onlyPoint)
        {
            if (!qApp->Settings()->isWireframe())
            {
               setBrush(QBrush(correctColor(this, m_pointColor), Qt::SolidPattern));
            }
            else
            {
               setBrush(QBrush(correctColor(this, m_pointColor), Qt::NoBrush));
            }
        }
        else
        {
            setBrush(QBrush(correctColor(this, m_pointColor),Qt::NoBrush));
        }
    }
    else
    {
        setPen(QPen(correctColor(this, QColor(qApp->Settings()->getPointNameColor())), width));

        if (!qApp->Settings()->isWireframe())
        {
           setBrush(QBrush(correctColor(this, QColor(qApp->Settings()->getPointNameColor())),Qt::SolidPattern));
        }
        else
        {
           setBrush(QBrush(correctColor(this, QColor(qApp->Settings()->getPointNameColor())),Qt::NoBrush));
        }

    }
}

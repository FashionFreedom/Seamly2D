//  @file   vcontrolpointspline.cpp
//  @author Douglas S Caskey
//  @date   2 Jun, 2024
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
 **  @file   vcontrolpointspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vcontrolpointspline.h"

#include <QBrush>
#include <QEvent>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QLineF>
#include <QList>
#include <QMessageLogger>
#include <QPen>
#include <QPoint>
#include <QPolygonF>
#include <QRectF>
#include <Qt>

#include "../vwidgets/global.h"
#include "../vmisc/vabstractapplication.h"
#include "../vgeometry/vgobject.h"
#include "vmaingraphicsscene.h"
#include "vmaingraphicsview.h"
#include "vgraphicssimpletextitem.h"
#include "scalesceneitems.h"

// @brief VControlPointSpline constructor.
// @param splineIndex index spline in list.
// @param position position point in spline.
// @param parent parent object.
VControlPointSpline::VControlPointSpline(const qint32 &splineIndex, SplinePointPosition position, QGraphicsItem *parent)
    : SceneRect(QColor(Qt::red), parent)
    , m_controlLine(nullptr)
    , m_splineIndex(splineIndex)
    , m_position(position)
    , m_freeAngle(true)
    , m_freeLength(true)
{
    init();
    setVisible(false);
}

// @brief VControlPointSpline constructor.
// @param splineIndex index spline in list.
// @param position position point in spline.
// @param controlPoint control point.
// @param splinePoint spline point.
// @param parent parent object.
VControlPointSpline::VControlPointSpline(const qint32 &splineIndex, SplinePointPosition position,
                                         const QPointF &controlPoint, bool freeAngle,
                                         bool freeLength, QGraphicsItem *parent)
    : SceneRect(QColor(Qt::red), parent)
    , m_controlLine(nullptr)
    , m_splineIndex(splineIndex)
    , m_position(position)
    , m_freeAngle(freeAngle)
    , m_freeLength(freeLength)
{
    init();

    this->setPos(controlPoint);
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setAcceptHoverEvents(true);
}

// @brief Paint paints the spline control handles. Paints the line from the spline to the control point first,
//        then calls the SceneRect widget paint to paint the control point.
// @param painter Qpainter.
// @param option QStyleOptionGraphicsItem.
// @param widget parent QWidget.
void VControlPointSpline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen lPen = m_controlLine->pen();
    lPen.setColor(correctColor(m_controlLine, Qt::black));

    QPointF p1, p2;
    VGObject::LineIntersectCircle(QPointF(), scaledRadius(sceneScale(scene())),
                                  QLineF(QPointF(), m_controlLine->line().p1()), p1, p2);
    QLineF const line(m_controlLine->line().p1(), p1);

    m_controlLine->setPen(lPen);
    m_controlLine->setLine(line);

    SceneRect::paint(painter, option, widget);
}

// @brief hoverEnterEvent handle hover enter events.
// @param event hover move event.
void VControlPointSpline::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_freeAngle || m_freeLength)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    SceneRect::hoverEnterEvent(event);
}

// @brief hoverLeaveEvent handle hover leave events.
// @param event hover move event.
void VControlPointSpline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_freeAngle || m_freeLength)
    {
        setCursor(QCursor());
    }
    SceneRect::hoverLeaveEvent(event);
}

// @brief itemChange handle item change.
// @param change change.
// @param value value.
// @return value.
QVariant VControlPointSpline::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    switch (change)
    {
        case ItemPositionChange:
        {
            if (!m_freeAngle || !m_freeLength)
            {
                const QPointF splPoint = m_controlLine->line().p1() + pos();

                QLineF newLine(splPoint, value.toPointF()); // value - new position.
                QLineF oldLine(splPoint, pos());            // pos() - old position.

                if (!m_freeAngle)
                {
                    newLine.setAngle(oldLine.angle());
                }

                if (!m_freeLength)
                {
                    newLine.setLength(oldLine.length());
                }

                return newLine.p2();
            }

            break;
        }
        case ItemPositionHasChanged:
        {
            // Each time we move something we call recalculation scene rect. In some cases this can cause moving
            // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last
            // move.
            static bool changeFinished = true;
            if (changeFinished)
            {
                changeFinished = false;
                // value - new position.
                emit controlPointPositionChanged(m_splineIndex, m_position, value.toPointF());
                if (scene())
                {
                    const QList<QGraphicsView *> viewList = scene()->views();
                    if (!viewList.isEmpty())
                    {
                        if (QGraphicsView *view = viewList.at(0))
                        {
                            const int xmargin = 50;
                            const int ymargin = 50;

                            const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(view);
                            const QRectF itemRect = mapToScene(boundingRect()).boundingRect();

                            // If item's rect is bigger than view's rect ensureVisible works very unstable.
                            if (itemRect.height() + 2*ymargin < viewRect.height() &&
                                itemRect.width() + 2*xmargin < viewRect.width())
                            {
                                 view->ensureVisible(itemRect, xmargin, ymargin);
                            }
                            else
                            {
                                // Ensure visible only small rect around a cursor
                                VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                                SCASSERT(currentScene)
                                const QPointF cursorPosition = currentScene->getScenePos();
                                view->ensureVisible(QRectF(cursorPosition.x()-5, cursorPosition.y()-5, 10, 10));
                            }
                        }
                    }
                }
                changeFinished = true;
            }
            break;
        }
        case QGraphicsItem::ItemSelectedHasChanged:
            emit selected(value.toBool());
            break;
        default:
            break;
    }
    return SceneRect::itemChange(change, value);
}

/// @brief mousePressEvent  handle mouse press events.
// @param event mouse release event.
void VControlPointSpline::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        if (m_freeAngle || m_freeLength)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            event->accept();
        }

        if (!(flags() & ItemIsSelectable) && scene())
        {
            scene()->clearSelection();
        }
    }
    SceneRect::mousePressEvent(event);
}

// @brief mouseReleaseEvent  handle mouse release events.
// @param event mouse release event.
void VControlPointSpline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        if (m_freeAngle || m_freeLength)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }
    SceneRect::mouseReleaseEvent(event);
}

// @brief contextMenuEvent  handle right mouse context menu events.
// @param context menu event.
void VControlPointSpline::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit showContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VControlPointSpline::init()
{
    setOnlyPoint(true);
    this->setBrush(QBrush(Qt::NoBrush));
    this->setZValue(100);

    m_controlLine = new VScaledLine(this);
    m_controlLine->setDefaultWidth(widthHairLine);
    m_controlLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
}

// @brief setCtrlLine set the geometry of the control handle line based on the control and spline points.
// @param controlPoint end of control line.
// @param splinePoint start point of the control line.
void VControlPointSpline::setCtrlLine(const QPointF &controlPoint, const QPointF &splinePoint)
{
    QPointF p1, p2;
    VGObject::LineIntersectCircle(QPointF(), scaledRadius(sceneScale(scene())),
                                  QLineF(QPointF(), splinePoint - controlPoint), p1, p2);

    QLineF const line(splinePoint - controlPoint, p1);
    m_controlLine->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false); //prevent paint recursion
    m_controlLine->setLine(QLineF(splinePoint - controlPoint, p1));
    m_controlLine->setVisible(!line.isNull());
    m_controlLine->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

// @brief refreshCtrlPoint refresh the control point.
// @param splineIndex index spline in list.
// @param pos position point in spline.
// @param controlPoint control point.
// @param splinePoint spline point.
void VControlPointSpline::refreshCtrlPoint(const qint32 &splineIndex, SplinePointPosition pos,
                                           const QPointF &controlPoint, const QPointF &splinePoint, bool freeAngle,
                                           bool freeLength)
{
    if (this->m_splineIndex == splineIndex && this->m_position == pos)
    {
        this->m_freeAngle = freeAngle;
        this->m_freeLength = freeLength;
        this->setPos(controlPoint);
        setCtrlLine(controlPoint, splinePoint);
    }
}

// @brief setEnabledPoint disable or enable control point.
// @param enable true - enable.
void VControlPointSpline::setEnabledPoint(bool enable)
{
    setEnabled(enable);
    m_controlLine->setEnabled(enable);
}

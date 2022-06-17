/******************************************************************************
 **  @file   resize_handle.cpp
 **  @author DS Caskey
 **  @date   Jul 2, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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
 *****************************************************************************/

#include "resize_handle.h"

#include "../vmisc/def.h"

#include <QMimeData>
#include <QGraphicsItem>
#include <QPainter>
#include <QCursor>
#include <QGraphicsScene>
#include <QGuiApplication>

/**
 * ResizeHandlesItem Constructor.
 */
ResizeHandlesItem::ResizeHandlesItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , m_handleItems()
    , m_parentRect()
{
    if (parentItem())
    {
        m_parentRect = parentItem()->boundingRect();
    }

    m_handleItems.append(new HandleItem(Position::TopLeft, this));
    m_handleItems.append(new HandleItem(Position::Top, this));
    m_handleItems.append(new HandleItem(Position::TopRight, this));
    m_handleItems.append(new HandleItem(Position::Right, this));
    m_handleItems.append(new HandleItem(Position::BottomRight, this));
    m_handleItems.append(new HandleItem(Position::Bottom, this));
    m_handleItems.append(new HandleItem(Position::BottomLeft, this));
    m_handleItems.append(new HandleItem(Position::Left, this));

    updateHandlePositions();
}


/**
 * @brief boundingRect get item's boundingrect.
 * @return QRectF.
 */
QRectF ResizeHandlesItem::boundingRect() const
{
    //return m_parentRect;
    return QRectF(0, 0, HANDLE_SIZE, HANDLE_SIZE);
}

/**
 * @brief paint handle item painting.
 * @param QPainter painter.
 * @param QStyleOptionGraphicsItem option.
 * @return void.
 */
void ResizeHandlesItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

#define SET_POSITION(POS, VALUE)                 \
void ResizeHandlesItem::set ## POS (VALUE value) \
{                                                \
    m_parentRect.set ## POS (value);             \
    updateHandlePositions();                     \
}                                                \

SET_POSITION(Top, qreal)
SET_POSITION(Right, qreal)
SET_POSITION(Bottom, qreal)
SET_POSITION(Left, qreal)
SET_POSITION(TopLeft, const QPointF&)
SET_POSITION(TopRight, const QPointF&)
SET_POSITION(BottomRight, const QPointF&)
SET_POSITION(BottomLeft, const QPointF&)

/**
 * @brief updateHandlePositions update handle positions on new parent bounding rect.
 * @return void.
 */
void ResizeHandlesItem::updateHandlePositions()
{
    foreach (HandleItem *item, m_handleItems)
    {
        item->setFlag(ItemSendsGeometryChanges, false);

        switch (item->position())
        {
            case Position::TopLeft:
                item->setPos(m_parentRect.topLeft());
                break;
            case Position::Top:
                item->setPos(m_parentRect.left() + m_parentRect.width() / 2 - 1,
                             m_parentRect.top());
                break;
            case Position::TopRight:
                item->setPos(m_parentRect.topRight());
                break;
            case Position::Right:
                item->setPos(m_parentRect.right(),
                             m_parentRect.top() + m_parentRect.height() / 2 - 1);
                break;
            case Position::BottomRight:
                item->setPos(m_parentRect.bottomRight());
                break;
            case Position::Bottom:
                item->setPos(m_parentRect.left() + m_parentRect.width() / 2 - 1,
                             m_parentRect.bottom());
                break;
            case Position::BottomLeft:
                item->setPos(m_parentRect.bottomLeft());
                break;
            case Position::Left:
                item->setPos(m_parentRect.left(),
                             m_parentRect.top() + m_parentRect.height() / 2 - 1);
                break;
            case Position::Center:
                break;
        }

        item->setFlag(ItemSendsGeometryChanges, true);
    }

    emit sizeChanged(m_parentRect);
}

/**
 * HandleItem Constructor.
 */
ResizeHandlesItem::HandleItem::HandleItem(Position position, ResizeHandlesItem* parent)
    : QGraphicsRectItem(-HANDLE_SIZE/2, -HANDLE_SIZE/2, HANDLE_SIZE, HANDLE_SIZE, parent)
    , m_parent(parent)
    , m_handlePosition(position)
    , m_handleCursors()
    , m_isHovered(false)
{
    this->setBrush(QBrush(Qt::lightGray));
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    setAcceptHoverEvents(true);

    m_handleCursors[0] = Qt::SizeFDiagCursor;   //TopLeft
    m_handleCursors[1] = Qt::SizeVerCursor;     //Top
    m_handleCursors[2] = Qt::SizeBDiagCursor;   //TopRight
    m_handleCursors[3] = Qt::SizeHorCursor;     //Right
    m_handleCursors[4] = Qt::SizeFDiagCursor;   //BottomRight
    m_handleCursors[5] = Qt::SizeVerCursor;     //Bottom
    m_handleCursors[6] = Qt::SizeBDiagCursor;   //Bottomleft
    m_handleCursors[7] = Qt::SizeHorCursor;     //Left
    m_handleCursors[8] = Qt::ArrowCursor;       //Center
}

/**
 * @brief position get handle position on the parent boundingrect.
 * @return Position
 */
Position ResizeHandlesItem::HandleItem::position() const
{
    return m_handlePosition;
}

/**
 * @brief paint handle item painting.
 * @param QPainter painter.
 * @param QStyleOptionGraphicsItem option.
 * @return void.
 */
void ResizeHandlesItem::HandleItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
    painter->setBrush(m_isHovered ? QColor(Qt::red) : QColor(Qt::lightGray));
    painter->drawRect(boundingRect());
}

/**
 * @brief itemChange handle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
QVariant ResizeHandlesItem::HandleItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant returnValue = value;

    if (change == ItemPositionChange)
    {
        returnValue = limitPosition(value.toPointF());
        return returnValue;
    }
    else if (change == ItemPositionHasChanged)
    {
        QPointF pos = value.toPointF();

        switch (m_handlePosition)
        {
            case Position::TopLeft:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal xdiff = m_parent->m_parentRect.left() - pos.x();
                    qreal ydiff = m_parent->m_parentRect.top() - pos.y();

                    m_parent->setTopLeft(pos);
                    m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                    m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                }
                else
                {
                    m_parent->setTopLeft(pos);
                }
                break;

            case Position::Top:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal diff = m_parent->m_parentRect.top() - pos.y();
                    m_parent->setTop(pos.y());
                    m_parent->setBottom(m_parent->m_parentRect.bottom() + diff);
                }
                else
                {
                    m_parent->setTop(pos.y());
                }
                break;

            case Position::TopRight:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal xdiff = pos.x() - m_parent->m_parentRect.right();
                    qreal ydiff = m_parent->m_parentRect.top() - pos.y();

                    m_parent->setTopRight(pos);
                    m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                    m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                }
                else
                {
                    m_parent->setTopRight(pos);
                }
                break;

            case Position::Right:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal diff = pos.x() - m_parent->m_parentRect.right();
                    m_parent->setRight(pos.x());
                    m_parent->setLeft(m_parent->m_parentRect.left() - diff);
                }
                else
                {
                    m_parent->setRight(pos.x());
                }
                break;

            case Position::BottomRight:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal xdiff = pos.x() - m_parent->m_parentRect.right();
                    qreal ydiff = pos.y() - m_parent->m_parentRect.bottom();

                    m_parent->setBottomRight(pos);
                    m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                }
                else
                {
                    m_parent->setBottomRight(pos);
                }
                break;

            case Position::Bottom:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal diff = pos.y() - m_parent->m_parentRect.bottom();
                    m_parent->setBottom(pos.y());
                    m_parent->setTop(m_parent->m_parentRect.top() - diff);
                }
                else
                {
                    m_parent->setBottom(pos.y());
                }
                break;

            case Position::BottomLeft:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal xdiff = m_parent->m_parentRect.left() - pos.x();
                    qreal ydiff = pos.y() - m_parent->m_parentRect.bottom();

                    m_parent->setBottomLeft(pos);
                    m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                }
                else
                {
                    m_parent->setBottomLeft(pos);
                }
                break;

            case Position::Left:
                if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    qreal diff = m_parent->m_parentRect.left() - pos.x();
                    m_parent->setLeft(pos.x());
                    m_parent->setRight(m_parent->m_parentRect.right() + diff);
                }
                else
                {
                    m_parent->setLeft(pos.x());
                }
                break;

            case Position::Center:
                break;
        }
    }
    return QGraphicsItem::itemChange(change, value);
}

/**
 * @brief limitPosition limits the bounds of the minimum size of the bounding rect.
 * @param QPointF newPos new position.
 * @return QPointF limited new position.
 */
QPointF ResizeHandlesItem::HandleItem::limitPosition(const QPointF& newPos)
{
    QPointF point = pos();
    if (m_handlePosition == Position::TopLeft      ||
        m_handlePosition == Position::BottomRight  ||
        m_handlePosition == Position::TopRight     ||
        m_handlePosition == Position::BottomLeft)
    {
        point.setX(newPos.x());
        point.setY(newPos.y());
    }

    if (m_handlePosition == Position::Top || m_handlePosition == Position::Bottom)
    {
        point.setY(newPos.y());
    }

    if (m_handlePosition == Position::Left || m_handlePosition == Position::Right)
    {
        point.setX(newPos.x());
    }

    if ((m_handlePosition == Position::Top ||
         m_handlePosition == Position::TopLeft ||
         m_handlePosition == Position::TopRight) &&
         point.y() > m_parent->m_parentRect.bottom() - HANDLE_SIZE * 2)
    {
        point.setY(m_parent->m_parentRect.bottom() - HANDLE_SIZE * 2);
    }
    else if ((m_handlePosition == Position::Bottom ||
              m_handlePosition == Position::BottomLeft ||
              m_handlePosition == Position::BottomRight) &&
              point.y() < m_parent->m_parentRect.top() + HANDLE_SIZE * 2)
    {
        point.setY(m_parent->m_parentRect.top() + HANDLE_SIZE * 2);
    }

    if ((m_handlePosition == Position::Left ||
        m_handlePosition == Position::TopLeft ||
        m_handlePosition == Position::BottomLeft) &&
        point.x() > m_parent->m_parentRect.right() - HANDLE_SIZE * 2)
    {
        point.setX(m_parent->m_parentRect.right() - HANDLE_SIZE * 2);
    }
    else if ((m_handlePosition == Position::Right ||
              m_handlePosition == Position::TopRight ||
              m_handlePosition == Position::BottomRight) &&
              point.x() < m_parent->m_parentRect.left() + HANDLE_SIZE * 2)
    {
        point.setX(m_parent->m_parentRect.left() + HANDLE_SIZE * 2);
    }

    return point;
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void ResizeHandlesItem::HandleItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;

    QGraphicsItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void ResizeHandlesItem::HandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;

    QGraphicsItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePressEvent handle mouse press events.
 * @param event mouse press event.
 */
//---------------------------------------------------------------------------------------------------------------------
void ResizeHandlesItem::HandleItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //if (isSelected())
    //{
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            setCursor(QCursor(m_handleCursors[static_cast<int>(m_handlePosition)]));
        }

        //emit handleSelected(m_handlePosition, true);
    //}

    QGraphicsItem::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseMoveEvent handle mouse move events.
 * @param event mouse move event.
 */
//---------------------------------------------------------------------------------------------------------------------
void ResizeHandlesItem::HandleItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent handle mouse release events.
 * @param event mouse release event.
 */
//---------------------------------------------------------------------------------------------------------------------
void ResizeHandlesItem::HandleItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        setCursor(QCursor(Qt::ArrowCursor));
    }
    //emit handleSelected(m_handlePosition, false);

    QGraphicsItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key release events.
 * @param event key release event.
 */
//---------------------------------------------------------------------------------------------------------------------
void ResizeHandlesItem::HandleItem::keyReleaseEvent(QKeyEvent *event)
{
    QGraphicsItem::keyReleaseEvent(event);
}

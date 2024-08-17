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
#include <QDebug>

/**
 * ResizeHandlesItem Constructor.
 */
ResizeHandlesItem::ResizeHandlesItem(QGraphicsItem *parent, qreal minDimension, qreal maxDimension)
    : QGraphicsItem(parent)
    , m_handleItems()
    , m_parentRect()
    , m_minDimension(minDimension)
    , m_maxDimension(maxDimension)
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

//------------------------------------------------------------------------------
void ResizeHandlesItem::setParentRect(const QRectF & rect)
{
    m_parentRect = rect;
    m_sizeChangedExternally = true;
    updateHandlePositions();
}


//------------------------------------------------------------------------------
void ResizeHandlesItem::setLockAspectRatio(bool lock)
{
    m_lockAspectRatio = lock;
}

//------------------------------------------------------------------------------
void ResizeHandlesItem::parentIsLocked(bool lock)
{
    m_parentIsLocked = lock;
    qreal lock_size = qMax(m_parentRect.width(), m_parentRect.height()) / 30;

    foreach (HandleItem *handleItem, m_handleItems)
    {
        handleItem->setFlag(QGraphicsItem::ItemIgnoresTransformations, !lock);
        handleItem->setAcceptedMouseButtons(lock ? Qt::NoButton : Qt::AllButtons);
        handleItem->setRect(lock ? QRect(-lock_size / 2, -lock_size / 2, lock_size, lock_size) : QRect(-HANDLE_SIZE/2, -HANDLE_SIZE/2, HANDLE_SIZE, HANDLE_SIZE));
    }
}


//------------------------------------------------------------------------------
void ResizeHandlesItem::setParentRotation(qreal rotation)
{
    m_parentRotation = rotation;
}


//------------------------------------------------------------------------------
void ResizeHandlesItem::setLimitDimensions(qreal min, qreal max)
{
    m_minDimension = min;
    m_maxDimension = max;
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

    if(!m_sizeChangedExternally)
    {
        emit sizeChangedFromHandles(m_parentRect);
    }
    else
    {
        m_sizeChangedExternally = false;
    }
}

/**
 * HandleItem Constructor.
 */
ResizeHandlesItem::HandleItem::HandleItem(Position position, ResizeHandlesItem* parent)
    : QGraphicsRectItem(-HANDLE_SIZE/2, -HANDLE_SIZE/2, HANDLE_SIZE, HANDLE_SIZE, parent)
    , m_parent(parent)
    , m_handlePosition(position)
    , m_handleIsHighlighted(false)
    , m_minDimension(parent->m_minDimension)
    , m_maxDimension(parent->m_maxDimension)
{
    this->setBrush(QBrush(Qt::lightGray));
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
    this->setFlag(QGraphicsItem::ItemIgnoresTransformations, !m_parent->m_parentIsLocked);

    setAcceptHoverEvents(true);
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

    if (!m_parent->m_parentIsLocked)
    {
        painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
        painter->setBrush(m_handleIsHighlighted ? QColor(Qt::red) : QColor(Qt::darkGray));
        painter->drawEllipse(boundingRect());
    }
    else
    {
        QPixmap pixmap("://icon/32x32/lock_off.png");
        painter->drawPixmap(boundingRect().toRect(), pixmap);
    }
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
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = m_parent->m_parentRect.left() - pos.x();
                    qreal ydiff = m_parent->m_parentRect.top() - pos.y();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        xdiff = ((m_parent->m_parentRect.bottom() + ydiff - pos.y()) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        pos.setX(m_parent->m_parentRect.left() - xdiff);
                    }

                    m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                    m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.right() - (m_parent->m_parentRect.bottom() - pos.y()) * m_scalingFactor);
                }

                m_parent->setTopLeft(pos);

                break;

            case Position::Top:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal ydiff = m_parent->m_parentRect.top() - pos.y();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        qreal xdiff = ((m_parent->m_parentRect.bottom() + ydiff - pos.y()) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                        m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                    }

                    m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.left() + (m_parent->m_parentRect.bottom() - pos.y()) * m_scalingFactor);
                    m_parent->setRight(pos.x());
                }

                m_parent->setTop(pos.y());

                break;

            case Position::TopRight:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = pos.x() - m_parent->m_parentRect.right();
                    qreal ydiff = m_parent->m_parentRect.top() - pos.y();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        xdiff = ((m_parent->m_parentRect.bottom() + ydiff - pos.y()) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        pos.setX(m_parent->m_parentRect.right() + xdiff);
                    }

                    m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                    m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.left() + (m_parent->m_parentRect.bottom() - pos.y()) * m_scalingFactor);
                }

                m_parent->setTopRight(pos);

                break;

            case Position::Right:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = pos.x() - m_parent->m_parentRect.right();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        qreal ydiff = ((pos.x() - m_parent->m_parentRect.left() + xdiff) / m_scalingFactor - m_parent->m_parentRect.height()) / 2;
                        m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                        m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    }

                    m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setY(m_parent->m_parentRect.top() + (pos.x() - m_parent->m_parentRect.left()) / m_scalingFactor);
                    m_parent->setBottom(pos.y());
                }

                m_parent->setRight(pos.x());

                break;

            case Position::BottomRight:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = pos.x() - m_parent->m_parentRect.right();
                    qreal ydiff = pos.y() - m_parent->m_parentRect.bottom();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        xdiff = ((pos.y() - m_parent->m_parentRect.top() + ydiff) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        pos.setX(m_parent->m_parentRect.right() + xdiff);
                    }

                    m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.left() + (pos.y() - m_parent->m_parentRect.top()) * m_scalingFactor);
                }

                m_parent->setBottomRight(pos);

                break;

            case Position::Bottom:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal ydiff = pos.y() - m_parent->m_parentRect.bottom();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        qreal xdiff = ((pos.y() - m_parent->m_parentRect.top() + ydiff) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                        m_parent->setLeft(m_parent->m_parentRect.left() - xdiff);
                    }

                    m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.left() + (pos.y() - m_parent->m_parentRect.top()) * m_scalingFactor);
                    m_parent->setRight(pos.x());
                }

                m_parent->setBottom(pos.y());

                break;

            case Position::BottomLeft:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = m_parent->m_parentRect.left() - pos.x();
                    qreal ydiff = pos.y() - m_parent->m_parentRect.bottom();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        xdiff = ((pos.y() - m_parent->m_parentRect.top() + ydiff) * m_scalingFactor - m_parent->m_parentRect.width()) / 2;
                        pos.setX(m_parent->m_parentRect.left() - xdiff);
                    }

                    m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setX(m_parent->m_parentRect.right() - (pos.y() - m_parent->m_parentRect.top()) * m_scalingFactor);
                }

                m_parent->setBottomLeft(pos);

                break;

            case Position::Left:
                if (QGuiApplication::keyboardModifiers() & Qt::ControlModifier)
                {
                    qreal xdiff = m_parent->m_parentRect.left() - pos.x();

                    if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                    {
                        qreal ydiff = ((m_parent->m_parentRect.right() + xdiff - pos.x()) / m_scalingFactor - m_parent->m_parentRect.height()) / 2;
                        m_parent->setBottom(m_parent->m_parentRect.bottom() + ydiff);
                        m_parent->setTop(m_parent->m_parentRect.top() - ydiff);
                    }

                    m_parent->setRight(m_parent->m_parentRect.right() + xdiff);
                }
                else if (m_parent->m_lockAspectRatio || QGuiApplication::keyboardModifiers() & Qt::ShiftModifier)
                {
                    pos.setY(m_parent->m_parentRect.top() + (m_parent->m_parentRect.right() - pos.x()) / m_scalingFactor);
                    m_parent->setBottom(pos.y());
                }

                m_parent->setLeft(pos.x());

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

    bool isMovingTop = m_handlePosition == Position::Top ||
                      m_handlePosition == Position::TopLeft ||
                      m_handlePosition == Position::TopRight;
    bool isMovingBottom = m_handlePosition == Position::Bottom ||
                         m_handlePosition == Position::BottomLeft ||
                         m_handlePosition == Position::BottomRight;
    bool isMovingLeft = m_handlePosition == Position::Left ||
                       m_handlePosition == Position::TopLeft ||
                       m_handlePosition == Position::BottomLeft;
    bool isMovingRight = m_handlePosition == Position::Right ||
                        m_handlePosition == Position::TopRight ||
                        m_handlePosition == Position::BottomRight;

    qreal height;
    qreal width;

    if (isMovingTop)
    {
        height = m_parent->m_parentRect.bottom() - point.y();

        if (height < m_minDimension)
        {
            point.setY(m_parent->m_parentRect.bottom() - m_minDimension);
        }
        else if (height > m_maxDimension)
        {
            point.setY(m_parent->m_parentRect.bottom() - m_maxDimension);
        }

        if (m_parent->m_lockAspectRatio && m_scalingFactor < 1 && height * m_scalingFactor < m_minDimension)
        {
            point.setY(m_parent->m_parentRect.bottom() - m_minDimension / m_scalingFactor);
        }
        else if (m_parent->m_lockAspectRatio && m_scalingFactor > 1 && height * m_scalingFactor > m_maxDimension)
        {
            point.setY(m_parent->m_parentRect.bottom() - m_maxDimension / m_scalingFactor);
        }
    }
    else if (isMovingBottom)
    {
        height = point.y() - m_parent->m_parentRect.top();

        if (height < m_minDimension)
        {
            point.setY(m_parent->m_parentRect.top() + m_minDimension);
        }
        else if (height > m_maxDimension)
        {
            point.setY(m_parent->m_parentRect.top() + m_maxDimension);
        }

        if (m_parent->m_lockAspectRatio && m_scalingFactor < 1 && height * m_scalingFactor < m_minDimension)
        {
            point.setY(m_parent->m_parentRect.top() + m_minDimension / m_scalingFactor);
        }
        else if (m_parent->m_lockAspectRatio && m_scalingFactor > 1 && height * m_scalingFactor > m_maxDimension)
        {
            point.setY(m_parent->m_parentRect.top() + m_maxDimension / m_scalingFactor);
        }
    }

    if (isMovingLeft)
    {
        width = m_parent->m_parentRect.right() - point.x();

        if (width < m_minDimension)
        {
            point.setX(m_parent->m_parentRect.right() - m_minDimension);
        }
        else if (width > m_maxDimension)
        {
            point.setX(m_parent->m_parentRect.right() - m_maxDimension);
        }

        if (m_parent->m_lockAspectRatio && m_scalingFactor > 1 && width / m_scalingFactor < m_minDimension)
        {
            point.setX(m_parent->m_parentRect.right() - m_minDimension * m_scalingFactor);
        }
        else if (m_parent->m_lockAspectRatio && m_scalingFactor < 1 && width / m_scalingFactor > m_maxDimension)
        {
            point.setX(m_parent->m_parentRect.right() - m_maxDimension * m_scalingFactor);
        }
    }
    else if (isMovingRight)
    {
        width = point.x() - m_parent->m_parentRect.left();

        if (width < m_minDimension)
        {
            point.setX(m_parent->m_parentRect.left() + m_minDimension);
        }
        else if (width > m_maxDimension)
        {
            point.setX(m_parent->m_parentRect.left() + m_maxDimension);
        }

        if (m_parent->m_lockAspectRatio && m_scalingFactor > 1 && width / m_scalingFactor < m_minDimension)
        {
            point.setX(m_parent->m_parentRect.left() + m_minDimension * m_scalingFactor);
        }
        else if (m_parent->m_lockAspectRatio && m_scalingFactor < 1 && width / m_scalingFactor > m_maxDimension)
        {
            point.setX(m_parent->m_parentRect.left() + m_maxDimension * m_scalingFactor);
        }
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
    m_handleIsHighlighted = true;

    if (!m_parent->m_parentIsLocked)
    {
        QPixmap pixmap(cursorResizeArrow);
        QTransform transform;
        transform.rotate(m_parent->m_parentRotation + (static_cast<int>(m_handlePosition) - 1) * 45);
        pixmap = pixmap.transformed(transform);
        QPointF offset = pixmap.rect().center();
        setCursor(QCursor(pixmap, offset.x(), offset.y()));

        QString message;
        if(m_parent->m_lockAspectRatio)
        {
            message = tr("Press <b>CTRL</b> to scale around the center - <b> Aspect ratio locked </b>");
        }
        else
        {
            message = tr("Press <b>CTRL</b> to scale around the center,"
                                 " <b>SHIFT</b> to scale uniformly.");
        }

        emit m_parent->setStatusMessage(message);
    }

    QGraphicsItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void ResizeHandlesItem::HandleItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_handleIsHighlighted = false;

    if (!m_parent->m_parentIsLocked)
    {
        setCursor(QCursor(Qt::ArrowCursor));
        emit m_parent->setStatusMessage("");
    }

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
    if (m_parent->m_parentIsLocked)
    {
        return;
    }

    m_scalingFactor = m_parent->m_parentRect.width() / m_parent->m_parentRect.height();
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
    if (m_parent->m_parentIsLocked)
    {
        return;
    }

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
    if (m_parent->m_parentIsLocked)
    {
        return;
    }

    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
    }
    //emit handleSelected(m_handlePosition, false);

    m_handleIsHighlighted = false;
    emit m_parent->imageNeedsSave();
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
    if (m_parent->m_parentIsLocked)
    {
        return;
    }

    QGraphicsItem::keyReleaseEvent(event);
}

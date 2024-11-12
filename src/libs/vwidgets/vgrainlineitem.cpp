//---------------------------------------------------------------------------------------------------------------------
//  @file   vgrainlineitem.cpp
//  @author Douglas S Caskey
//  @date   11 Nov, 2024
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
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vgrainlineitem.cpp
//  @author Bojan Kverh
//  @date   September 10, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include <math.h>

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QDebug>
#include <QGraphicsScene>
#include <QGraphicsView>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vabstractapplication.h"

#include "vgrainlineitem.h"

#define ARROW_ANGLE                     M_PI/9
#define RECT_WIDTH                      30
#define RESIZE_RECT_SIZE                10
#define ROTATE_CIRC_R                   7
#define ACTIVE_Z                        10
#define LINE_PEN_WIDTH                  3

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::VGrainlineItem constructor
/// @param parent pointer to the parent item
//---------------------------------------------------------------------------------------------------------------------
VGrainlineItem::VGrainlineItem(QGraphicsItem* parent)
    : VPieceItem(parent)
    , m_rotation(0)
    , m_rotationStart(0)
    , m_length(0)
    , m_boundingPoly()
    , m_startPos()
    , m_movePos()
    , m_resizePolygon()
    , m_startLength(0)
    , m_startPoint()
    , m_finishPoint()
    , m_centerPoint()
    , m_angle(0)
    , m_arrowType(ArrowType::Both)
    , m_penWidth(LINE_PEN_WIDTH)
{
    setAcceptHoverEvents(true);
    m_inactiveZ = 5;
    reset();
    updateRectangle();
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VGrainlineItem::shape() const
{
    if (m_mode == Mode::Normal)
    {
        return mainShape();
    }
    else
    {
        QPainterPath path;
        path.addPolygon(m_boundingPoly);
        return path;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::paint paints the item content
/// @param painter pointer to the painter object
/// @param option not used
/// @param widget not used
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    QColor color  = QColor(qApp->Settings()->getDefaultGrainlineColor());
    m_penWidth = ToPixel(qApp->Settings()->getDefaultGrainlineLineweight(), Unit::Mm) * 3;
    painter->setPen(QPen(color, m_penWidth, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

    painter->setRenderHints(QPainter::Antialiasing);
    // line
    const QLineF line = mainLine();
    painter->drawLine(line.p1(), line.p2());

    painter->setBrush(color);

    if (m_arrowType != ArrowType::Bottom)
    {
        // first arrow
        painter->drawPolygon(firstArrow());
    }
    if (m_arrowType != ArrowType::Top)
    {
        // second arrow
        painter->drawPolygon(secondArrow());
    }

    if (m_mode != Mode::Normal)
    {
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        // bounding polygon
        painter->drawPolygon(m_boundingPoly);

        if (m_mode != Mode::Rotate)
        {
            painter->setPen(QPen(Qt::black, 3));
            painter->setBrush(Qt::black);
            updatePolyResize();
            painter->drawPolygon(m_resizePolygon);
        }

        painter->setBrush(Qt::NoBrush);
        if (m_mode == Mode::Resize)
        {
            painter->setPen(Qt::black);
            painter->drawLine(m_boundingPoly.at(0), m_boundingPoly.at(2));
            painter->drawLine(m_boundingPoly.at(1), m_boundingPoly.at(3));
        }

        if (m_mode == Mode::Rotate)
        {
            QPointF ptC = (m_boundingPoly.at(0) + m_boundingPoly.at(2))/2;
            qreal dRad =  ROTATE_CIRC_R;
            painter->setBrush(Qt::black);
            painter->drawEllipse(ptC, dRad, dRad);

            painter->setBrush(Qt::NoBrush);
            painter->save();
            painter->translate(ptC);
            painter->rotate(qRadiansToDegrees(-m_rotation));
            int iX = int(qRound(m_length/2 - 0.5*dRad));
            int iY = int(qRound(RECT_WIDTH - 0.5*dRad));
            int iR = int(qRound(dRad*3));
            painter->drawArc(iX - iR, iY - iR, iR, iR, 0*16, -90*16);
            painter->drawArc(-iX, iY - iR, iR, iR, 270*16, -90*16);
            painter->drawArc(-iX, -iY, iR, iR, 180*16, -90*16);
            painter->drawArc(iX - iR, -iY, iR, iR, 90*16, -90*16);
            painter->restore();
        }
    }
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::updateGeometry updates the item with grainline parameters.
/// @param position position of one grainline's end.
/// @param rotation rotation of the grainline in [degrees].
/// @param length length of the grainline in user's units.
/// @param type type of arrowhead.
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::updateGeometry(const QPointF& position, qreal rotation, qreal length, ArrowType type)
{
    m_rotation = qDegreesToRadians(rotation);
    m_length = length;

    qreal dX;
    qreal dY;
    QPointF point = position;
    if (isContained(point, m_rotation, dX, dY) == false)
    {
        point.setX(point.x() + dX);
        point.setY(point.y() + dY);
    }
    setPos(point);
    m_arrowType = type;

    updateRectangle();
    updateItem();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::isContained checks, if both ends of the grainline, starting at point, are contained in
/// parent widget.
/// @param point starting point of the grainline.
/// @param dRot rotation of the grainline in [rad]
/// @param dX horizontal translation needed to put the arrow inside parent item
/// @param dY vertical translation needed to put the arrow inside parent item
/// @return true, if both ends of the grainline, starting at point, are contained in the parent widget and
/// false otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VGrainlineItem::isContained(const QPointF& point, qreal dRot, qreal &dX, qreal &dY) const
{
    dX = 0;
    dY = 0;
    QPointF apoint[2];
    apoint[0] = point;
    apoint[1].setX(point.x() + m_length * cos(dRot));
    apoint[1].setY(point.y() - m_length * sin(dRot));
    // single point differences
    qreal dPtX;
    qreal dPtY;
    bool bInside = true;

    QRectF rectParent = parentItem()->boundingRect();
    for (int i = 0; i < 2; ++i)
    {
        dPtX = 0;
        dPtY = 0;
        if (rectParent.contains(apoint[i]) == false)
        {
            if (apoint[i].x() < rectParent.left())
            {
                dPtX = rectParent.left() - apoint[i].x();
            }
            else if (apoint[i].x() > rectParent.right())
            {
                dPtX = rectParent.right() - apoint[i].x();
            }
            if (apoint[i].y() < rectParent.top())
            {
                dPtY = rectParent.top() - apoint[i].y();
            }
            else if (apoint[i].y() > rectParent.bottom())
            {
                dPtY = rectParent.bottom() - apoint[i].y();
            }

            if (fabs(dPtX) > fabs(dX))
            {
                dX = dPtX;
            }
            if (fabs(dPtY) > fabs(dY))
            {
                dY = dPtY;
            }

            bInside = false;
        }
    }
    return bInside;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::mousePressEvent handles left button mouse press events
/// @param event pointer to QGraphicsSceneMouseEvent object
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick
        && (flags() & QGraphicsItem::ItemIsMovable))
    {
        if (m_moveType == NotMovable)
        {
            event->ignore();
            return;
        }

        m_startPos = pos();
        m_movePos = event->scenePos();
        m_startLength = m_length;
        m_rotationStart = m_rotation;
        m_angle = GetAngle(mapToParent(event->pos()));
        m_rotationCenter = m_centerPoint;

        if ((m_moveType & AllModifications ) == AllModifications)
        {
            allUserModifications(event->pos());
            setZValue(ACTIVE_Z);
            updateItem();
        }
        else if (m_moveType & IsRotatable)
        {
            if (m_moveType & IsResizable)
            {
                allUserModifications(event->pos());
            }
            else if (m_moveType & IsMovable)
            {
                userRotateAndMove();
            }
            else
            {
                m_mode = Mode::Rotate;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }
            setZValue(ACTIVE_Z);
            updateItem();
        }
        else if (m_moveType & IsResizable)
        {
            if (m_moveType & IsRotatable)
            {
                allUserModifications(event->pos());
            }
            else if (m_moveType & IsMovable)
            {
                userMoveAndResize(event->pos());
            }
            setZValue(ACTIVE_Z);
            updateItem();
        }
        else if (m_moveType & IsMovable)
        {
            if (m_moveType & IsRotatable)
            {
                userRotateAndMove();
            }
            else if (m_moveType & IsResizable)
            {
                userMoveAndResize(event->pos());
            }
            else
            {
                m_mode = Mode::Move;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }

            setZValue(ACTIVE_Z);
            updateItem();
        }
        else
        {
            event->ignore();
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::mouseMoveEvent handles mouse move events, making sure that the item is moved properly
/// @param event pointer to QGraphicsSceneMouseEvent object
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QPointF delta = event->scenePos() - m_movePos;
    qreal dX;
    qreal dY;
    if (m_mode == Mode::Move && m_moveType & IsMovable)
    {
        QPointF point = m_startPos + delta;
        if (isContained(point, m_rotation, dX, dY) == false)
        {
            point.setX(point.x() + dX);
            point.setY(point.y() + dY);
        }
        setPos(point);
        updateItem();
    }
    else if (m_mode == Mode::Resize && m_moveType & IsResizable)
    {
        qreal dLen = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
        qreal dAng = qAtan2(-delta.y(), delta.x());
        dLen = -dLen*qCos(dAng - m_rotation);
        qreal dPrevLen = m_length;
        // try with new length
        if (!(m_moveType & IsMovable))
        {
            dLen *= 2;
        }
        m_length = m_startLength + dLen;

        QPointF pos;

        if (m_moveType & IsMovable)
        {
            QLineF grainline(this->pos().x(), this->pos().y(),
                             this->pos().x() + dPrevLen, this->pos().y());
            grainline.setAngle(qRadiansToDegrees(m_rotation));
            grainline = QLineF(grainline.p2(), grainline.p1());
            grainline.setLength(m_length);
            pos = grainline.p2();
        }
        else
        {
            QLineF grainline(m_centerPoint.x(), m_centerPoint.y(),
                             m_centerPoint.x() + m_length / 2.0, m_centerPoint.y());

            grainline.setAngle(qRadiansToDegrees(m_rotation));
            grainline = QLineF(grainline.p2(), grainline.p1());
            grainline.setLength(m_length);

            pos = grainline.p2();
        }

        qreal dX;
        qreal dY;
        if (isContained(pos, m_rotation, dX, dY) == false)
        {
            m_length = dPrevLen;
        }
        else
        {
            setPos(pos);
        }

        updateRectangle();
        updateItem();
    }
    else if (m_mode == Mode::Rotate && m_moveType & IsRotatable)
    {
        // prevent strange angle changes due to singularities
        qreal dLen = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
        if (dLen < 2)
        {
            return;
        }

        if (fabs(m_angle) < 0.01)
        {
            m_angle = GetAngle(mapToParent(event->pos()));
            return;
        }

        qreal dAng = GetAngle(mapToParent(event->pos())) - m_angle;
        QPointF ptNewPos = rotate(m_startPos, m_rotationCenter, dAng);
        if (isContained(ptNewPos, m_rotationStart + dAng, dX, dY) == true)
        {
            setPos(ptNewPos);
            m_rotation = m_rotationStart + dAng;
            updateRectangle();
            updateItem();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::mouseReleaseEvent handles mouse release events and emits the proper signal if the item was
/// moved
/// @param event pointer to QGraphicsSceneMouseEvent object
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        if ((m_mode == Mode::Move || m_mode == Mode::Rotate || m_mode == Mode::Resize) && (flags() & QGraphicsItem::ItemIsMovable))
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }

        QPointF delta = event->scenePos() - m_movePos;
        qreal dLen = qSqrt(delta.x()*delta.x() + delta.y()*delta.y());
        bool bShort = (dLen < 2);

        if (m_mode == Mode::Move || m_mode == Mode::Resize)
        {
            if (bShort == true)
            {
                if (m_isReleased == true && m_moveType & IsRotatable)
                {
                    m_mode = Mode::Rotate;
                    updateItem();
                }
            }
            else
            {
                if (m_mode == Mode::Move && m_moveType & IsMovable)
                {
                    emit itemMoved(pos());
                }
                else if (m_moveType & IsResizable)
                {
                    emit itemResized(m_length);
                }
                updateItem();
            }
        }
        else
        {
            if (bShort == true)
            {
                m_mode = Mode::Move;
            }
            else if (m_moveType & IsRotatable)
            {
                emit itemRotated(m_rotation, m_startPoint);
            }
            updateItem();
        }
        m_isReleased = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_penWidth = m_penWidth + 1;
    VPieceItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_penWidth = m_penWidth - 1;
    VPieceItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::updateItem updates the item
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::updateItem()
{
    update(m_boundingRect);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::updateRectangle updates the polygon for the box around active item
/// and the bounding rectangle
//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::updateRectangle()
{
    QPointF point1(0, 0);
    QPointF point2(point1.x() + m_length * cos(m_rotation), point1.y() - m_length * sin(m_rotation));

    m_startPoint  = mapToParent(point1);
    m_finishPoint = mapToParent(point2);
    m_centerPoint = (m_startPoint + m_finishPoint)/2;

    m_boundingPoly.clear();
    m_boundingPoly << QPointF(point1.x() + RECT_WIDTH*cos(m_rotation + M_PI/2),
                              point1.y() - RECT_WIDTH*sin(m_rotation + M_PI/2));
    m_boundingPoly << QPointF(point1.x() + RECT_WIDTH*cos(m_rotation - M_PI/2),
                              point1.y() - RECT_WIDTH*sin(m_rotation - M_PI/2));
    m_boundingPoly << QPointF(point2.x() + RECT_WIDTH*cos(m_rotation - M_PI/2),
                              point2.y() - RECT_WIDTH*sin(m_rotation - M_PI/2));
    m_boundingPoly << QPointF(point2.x() + RECT_WIDTH*cos(m_rotation + M_PI/2),
                              point2.y() - RECT_WIDTH*sin(m_rotation + M_PI/2));
    m_boundingRect = m_boundingPoly.boundingRect().adjusted(-2, -2, 2, 2);
    setTransformOriginPoint(m_boundingRect.center());

    updatePolyResize();
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
double VGrainlineItem::GetAngle(const QPointF &point) const
{
    return -VPieceItem::GetAngle(point);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::rotate rotates point point around ptCenter by angle dAng [rad]
/// and returns the resulting point
/// @param point point to rotate
/// @param ptCenter center of rotation
/// @param dAng angle of rotation
/// @return point, which is a result of rotating point around ptCenter by angle dAng
//---------------------------------------------------------------------------------------------------------------------
QPointF VGrainlineItem::rotate(const QPointF& point, const QPointF& ptCenter, qreal dAng) const
{
    QPointF ptRel = point - ptCenter;
    QPointF ptFinal;
    ptFinal.setX(ptRel.x()*qCos(dAng) + ptRel.y()*qSin(dAng));
    ptFinal.setY(-ptRel.x()*qSin(dAng) + ptRel.y()*qCos(dAng));
    ptFinal += ptCenter;
    return ptFinal;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VGrainlineItem::getInsideCorner calculates a point inside the bounding polygon,
/// distance away of i-th point in each direction
/// @param i index of corner
/// @param distance distance
/// @return resulting point
//---------------------------------------------------------------------------------------------------------------------
QPointF VGrainlineItem::getInsideCorner(int i, qreal distance) const
{
    QPointF point1 = m_boundingPoly.at((i + 1) % m_boundingPoly.count()) - m_boundingPoly.at(i);

    QPointF point2 = m_boundingPoly.at((i + m_boundingPoly.count() - 1) %
                     m_boundingPoly.count()) - m_boundingPoly.at(i);

    point1 = distance * point1/qSqrt(point1.x()*point1.x() + point1.y()*point1.y());
    point2 = distance * point2/qSqrt(point2.x()*point2.x() + point2.y()*point2.y());

    return m_boundingPoly.at(i) + point1 + point2;
}

//---------------------------------------------------------------------------------------------------------------------
QLineF VGrainlineItem::mainLine() const
{
    QPointF point1;
    QPointF point2(point1.x() + m_length * cos(m_rotation), point1.y() - m_length * sin(m_rotation));
    return QLineF(point1, point2);
}

//---------------------------------------------------------------------------------------------------------------------
QPolygonF VGrainlineItem::firstArrow() const
{
    qreal arrowLength = qApp->Settings()->getDefaultArrowLength();
    QPointF point2 = mainLine().p2();
    QPolygonF polygon;
    polygon << point2;
    polygon << QPointF(point2.x() + arrowLength * cos(M_PI + m_rotation + ARROW_ANGLE),
                       point2.y() - arrowLength * sin(M_PI + m_rotation + ARROW_ANGLE));
    polygon << QPointF(point2.x() + arrowLength * cos(M_PI + m_rotation - ARROW_ANGLE),
                       point2.y() - arrowLength * sin(M_PI + m_rotation - ARROW_ANGLE));
    return polygon;
}

//---------------------------------------------------------------------------------------------------------------------
QPolygonF VGrainlineItem::secondArrow() const
{
    qreal arrowLength = qApp->Settings()->getDefaultArrowLength();
    QPointF point1 = mainLine().p1();
    QPolygonF polygon;
    polygon << point1;
    polygon << QPointF(point1.x() + arrowLength * cos(m_rotation + ARROW_ANGLE),
                       point1.y() - arrowLength * sin(m_rotation + ARROW_ANGLE));
    polygon << QPointF(point1.x() + arrowLength * cos(m_rotation - ARROW_ANGLE),
                       point1.y() - arrowLength * sin(m_rotation - ARROW_ANGLE));
    return polygon;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VGrainlineItem::mainShape() const
{
    QPainterPath path;
    const QLineF line = mainLine();

    QPainterPath linePath;
    linePath.moveTo(line.p1());
    linePath.lineTo(line.p2());
    linePath.closeSubpath();

    QPainterPathStroker stroker;
    stroker.setWidth(m_penWidth);
    path.addPath((stroker.createStroke(linePath) + linePath).simplified());
    path.closeSubpath();

    if (m_arrowType != ArrowType::Bottom)
    {
        // first arrow
        QPainterPath polyPath;
        polyPath.addPolygon(firstArrow());
        path.addPath((stroker.createStroke(polyPath) + polyPath).simplified());
        path.closeSubpath();
    }

    if (m_arrowType != ArrowType::Top)
    {
        // second arrow
        QPainterPath polyPath;
        polyPath.addPolygon(secondArrow());
        path.addPath((stroker.createStroke(polyPath) + polyPath).simplified());
        path.closeSubpath();
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::allUserModifications(const QPointF &pos)
{
    if (m_mode != Mode::Rotate)
    {
        userMoveAndResize(pos);
    }
    else
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::userRotateAndMove()
{
    if (m_mode != Mode::Rotate)
    {
        m_mode = Mode::Move;
    }
    SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::userMoveAndResize(const QPointF &pos)
{
    if (m_resizePolygon.containsPoint(pos, Qt::OddEvenFill) == true)
    {
        m_mode = Mode::Resize;
        setCursor(Qt::SizeFDiagCursor);
    }
    else
    {
        m_mode = Mode::Move; // block later if need
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineItem::updatePolyResize()
{
    m_resizePolygon.clear();
    QPointF point = m_boundingPoly.at(1);
    m_resizePolygon << point;

    point.setX(point.x() - RESIZE_RECT_SIZE * cos(m_rotation - M_PI/2));
    point.setY(point.y() + RESIZE_RECT_SIZE * sin(m_rotation - M_PI/2));
    m_resizePolygon << point;

    point.setX(point.x() + RESIZE_RECT_SIZE * cos(m_rotation));
    point.setY(point.y() - RESIZE_RECT_SIZE * sin(m_rotation));
    m_resizePolygon << point;

    point.setX(point.x() - RESIZE_RECT_SIZE * cos(m_rotation + M_PI/2));
    point.setY(point.y() + RESIZE_RECT_SIZE * sin(m_rotation + M_PI/2));
    m_resizePolygon << point;
}

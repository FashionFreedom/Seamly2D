/************************************************************************
 **
 **  @file   vtextgraphicsitem.cpp
 **  @author Bojan Kverh
 **  @date   June 16, 2016
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

#include <QApplication>
#include <QColor>
#include <QFlags>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPen>
#include <QPoint>
#include <QStyleOptionGraphicsItem>
#include <Qt>
#include <QDebug>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vabstractapplication.h"
#include "vtextgraphicsitem.h"

const qreal resizeSquare = (3./*mm*/ / 25.4) * PrintDPI;
const qreal rotateCircle = (2./*mm*/ / 25.4) * PrintDPI;
#define ROTATE_RECT                 60
#define ROTATE_ARC                  50
const qreal minW = (4./*mm*/ / 25.4) * PrintDPI + resizeSquare;
const qreal minH = (4./*mm*/ / 25.4) * PrintDPI + resizeSquare;
#define ACTIVE_Z                    10

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBoundingRect calculates the bounding box around rectBB rectangle, rotated around its center by rotation degrees
 * @param rectBB rectangle of interest
 * @param rotation rectangle rotation
 * @return bounding box around rectBB rotated by rotation
 */
QRectF GetBoundingRect(QRectF rectBB, qreal rotation)
{
    QPointF apt[4] = { rectBB.topLeft(), rectBB.topRight(), rectBB.bottomLeft(), rectBB.bottomRight() };
    QPointF ptCenter = rectBB.center();

    qreal xPos1 = 0;
    qreal xPos2 = 0;
    qreal yPos1 = 0;
    qreal yPos2 = 0;

    double angle = qDegreesToRadians(rotation);
    for (int i = 0; i < 4; ++i)
    {
        QPointF pt = apt[i] - ptCenter;
        qreal xPos = pt.x()*cos(angle) + pt.y()*sin(angle);
        qreal yPos = -pt.x()*sin(angle) + pt.y()*cos(angle);

        if (i == 0)
        {
            xPos1 = xPos2 = xPos;
            yPos1 = yPos2 = yPos;
        }
        else
        {
            if (xPos < xPos1)
            {
                xPos1 = xPos;
            }
            else if (xPos > xPos2)
            {
                xPos2 = xPos;
            }
            if (yPos < yPos1)
            {
                yPos1 = yPos;
            }
            else if (yPos > yPos2)
            {
                yPos2 = yPos;
            }
        }
    }
    QRectF rect;
    rect.setTopLeft(ptCenter + QPointF(xPos1, yPos1));
    rect.setWidth(xPos2 - xPos1);
    rect.setHeight(yPos2 - yPos1);
    return rect;
}
}//static functions

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::VTextGraphicsItem constructor
 * @param parent pointer to the parent item
 */
VTextGraphicsItem::VTextGraphicsItem(QGraphicsItem *parent)
    : VPieceItem(parent)
    , m_startPos()
    , m_start()
    , m_startSize()
    , m_rotation(0)
    , m_angle(0)
    , m_rectResize()
    , m_textMananger()
{
    m_inactiveZ = 2;
    setSize(minW, minH);
    setZValue(m_inactiveZ);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::SetFont sets the item font
 * @param font font to be used in item
 */
void VTextGraphicsItem::setFont(const QFont &font)
{
    m_textMananger.setFont(font);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::paint redraws the item content
 * @param painter pointer to the QPainter in use
 * @param option pointer to the object containing the actual label rectangle
 * @param widget not used
 */
void VTextGraphicsItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget)
    Q_UNUSED(option)

    QColor color  = QColor(qApp->Settings()->getDefaultLabelColor());

    painter->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    painter->setPen(QPen(color));

    QFont font = m_textMananger.GetFont();
    int width = qFloor(boundingRect().width());
    // draw text lines
    int yPos = 0;
    for (int i = 0; i < m_textMananger.GetSourceLinesCount(); ++i)
    {
        const TextLine& textLine = m_textMananger.GetSourceLine(i);

        font.setPixelSize(m_textMananger.GetFont().pixelSize() + textLine.m_iFontSize);
        font.setBold(textLine.bold);
        font.setItalic(textLine.italic);

        QString text = textLine.m_text;
        QFontMetrics fm(font);

        // check if the next line will go out of bounds
        if (yPos + fm.height() > boundingRect().height())
        {
            break;
        }

        if (fm.horizontalAdvance(text) > width)
        {
            text = fm.elidedText(text, Qt::ElideMiddle, width);
        }

        painter->setFont(font);
        painter->drawText(0, yPos, width, fm.height(), static_cast<int>(textLine.m_eAlign), text);
        yPos += fm.height() + m_textMananger.GetSpacing();
    }

    // now draw the features specific to non-normal modes
    if (m_eMode != mNormal)
    {
        // outextLineine the rectangle
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->drawRect(boundingRect().adjusted(1, 1, -1, -1));

        if (m_eMode != mRotate)
        {
            // draw the resize square
            painter->setPen(Qt::black);
            painter->setBrush(Qt::black);
            painter->drawRect(m_rectResize.adjusted(-1, -1, -1, -1));

            if (m_eMode == mResize)
            {
                // draw the resize diagonal lines
                painter->drawLine(1, 1, qFloor(m_rectBoundingBox.width())-1, qFloor(m_rectBoundingBox.height())-1);
                painter->drawLine(1, qFloor(m_rectBoundingBox.height())-1, qFloor(m_rectBoundingBox.width())-1, 1);
            }
        }
        else
        {
            // in rotate mode, draw the circle in the middle
            painter->setPen(Qt::black);
            painter->setBrush(Qt::black);
            painter->drawEllipse(
                        QPointF(m_rectBoundingBox.width()/2, m_rectBoundingBox.height()/2),
                        rotateCircle,
                        rotateCircle
                        );
            if (m_rectBoundingBox.width() > minW*3 && m_rectBoundingBox.height() > minH*3)
            {
                painter->setPen(QPen(Qt::black, 3));
                painter->setBrush(Qt::NoBrush);
                // and then draw the arc in each of the corners
                int top = ROTATE_RECT - ROTATE_ARC;
                int left = ROTATE_RECT - ROTATE_ARC;
                int right = qRound(m_rectBoundingBox.width()) - ROTATE_RECT;
                int bottom = qRound(m_rectBoundingBox.height()) - ROTATE_RECT;
                painter->drawArc(left, top, ROTATE_ARC, ROTATE_ARC, 180*16, -90*16);
                painter->drawArc(right, top, ROTATE_ARC, ROTATE_ARC, 90*16, -90*16);
                painter->drawArc(left, bottom, ROTATE_ARC, ROTATE_ARC, 270*16, -90*16);
                painter->drawArc(right, bottom, ROTATE_ARC, ROTATE_ARC, 0*16, -90*16);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::setSize Tries to set the label size to (width, height).
 * If either of those is too small, the labelsize does not change.
 * @param width label width
 * @param height label height
 */
void VTextGraphicsItem::setSize(qreal width, qreal height)
{
    // Take into account the rotation of the bounding rectangle
    QTransform transform = QTransform().rotate(-rotation());
    QRectF bbRect = transform.map(parentItem()->boundingRect()).boundingRect();

    // don't allow resize under specific size
    if (width > bbRect.width())
    {
        width = bbRect.width();
        qDebug() << "Setting label width to parent item width" << width;
    }
    if (width < minW)
    {
        width = minW;
        qDebug() << "Setting label width to min width" << width;
    }
    if (height > bbRect.height())
    {
        height = bbRect.height();
        qDebug() << "Setting label height to parent item height" << width;
    }
    if (height < minH)
    {
        height = minH;
        qDebug() << "Setting label height to min item height" << width;
    }

    prepareGeometryChange();
    m_rectBoundingBox.setTopLeft(QPointF(0, 0));
    m_rectBoundingBox.setWidth(width);
    m_rectBoundingBox.setHeight(height);
    m_rectResize.setTopLeft(QPointF(width - resizeSquare, height - resizeSquare));
    m_rectResize.setWidth(resizeSquare);
    m_rectResize.setHeight(resizeSquare);
    setTransformOriginPoint(m_rectBoundingBox.center());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::Update sets the correct size and font size and redraws the label
 */
void VTextGraphicsItem::Update()
{
    correctLabel();
    //UpdateBox();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::isContained checks if the bounding box around rotated rectBB is contained in
 * the parent. If that is not the case, it calculates the amount of movement needed to put it inside the parent
 * and write it into xPos, yPos
 * @param rectBB bounding box in question
 * @param rotation bounding box rotation in degrees
 * @param xPos horizontal translation needed to put the box inside parent item
 * @param yPos vertical translation needed to put the box inside parent item
 * @return true, if rectBB is contained in parent item and false otherwise
 */
bool VTextGraphicsItem::isContained(QRectF rectBB, qreal rotation, qreal &xPos, qreal &yPos) const
{
    QRectF rectParent = parentItem()->boundingRect();
    rectBB = GetBoundingRect(rectBB, rotation);
    xPos = 0;
    yPos = 0;

    if (rectParent.contains(rectBB) == false)
    {
        if (rectParent.left() - rectBB.left() > fabs(xPos))
        {
            xPos = rectParent.left() - rectBB.left();
        }
        else if (rectBB.right() - rectParent.right() > fabs(xPos))
        {
            xPos = rectParent.right() - rectBB.right();
        }

        if (rectParent.top() - rectBB.top() > fabs(yPos))
        {
            yPos = rectParent.top() - rectBB.top();
        }
        else if (rectBB.bottom() - rectParent.bottom() > fabs(yPos))
        {
            yPos = rectParent.bottom() - rectBB.bottom();
        }

        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::updateData Updates the detail label
 * @param name name of detail
 * @param data reference to VPatternPieceData
 */
void VTextGraphicsItem::updateData(const QString &name, const VPieceLabelData &data)
{
    m_textMananger.Update(name, data);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::updateData Updates the pattern label
 * @param doc pointer to the pattern object
 */
void VTextGraphicsItem::updateData(VAbstractPattern *doc)
{
    m_textMananger.Update(doc);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::getTextLines returns the number of lines of text to show
 * @return number of lines of text
 */
int VTextGraphicsItem::getTextLines() const
{
    return m_textMananger.GetSourceLinesCount();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::getFontSize returns the currentextLiney used text base font size
 * @return current text base font size
 */
int VTextGraphicsItem::getFontSize() const
{
    return m_textMananger.GetFont().pixelSize();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mousePressEvent handles left button mouse press events
 * @param event pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    qDebug() << "VTextGraphicsItem::mousePressEvent\n";
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick
        && (flags() & QGraphicsItem::ItemIsMovable))
    {
        if (m_moveType == NotMovable)
        {
            event->ignore();
            return;
        }

        // record the parameters of the mouse press. Specially record the position
        // of the press as the origin for the following operations
        m_startPos = pos();
       qDebug() << " start position" << m_startPos;
        m_start = event->scenePos();
        m_startSize = m_rectBoundingBox.size();
        m_ptRotCenter = mapToScene(m_rectBoundingBox.center());
        qDebug() << " center position" << m_ptRotCenter;
        m_angle = GetAngle(event->scenePos());
        m_rotation = rotation();
        // in rotation mode, do not do any changes here, because user might want to
        // rotate the label more.

        if ((m_moveType & AllModifications ) == AllModifications)
        {
            allUserModifications(event->pos());
            setZValue(ACTIVE_Z);
            Update();
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
                m_eMode = mRotate;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }
            setZValue(ACTIVE_Z);
            Update();
        }
        else if (m_moveType & IsResizable)
        {
            qDebug() << " Item is Movable\n";
            if (m_moveType & IsRotatable)
            {
                allUserModifications(event->pos());
            }
            else if (m_moveType & IsMovable)
            {
                userMoveAndResize(event->pos());
            }
            setZValue(ACTIVE_Z);
            Update();
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
                m_eMode = mMove;
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }

            setZValue(ACTIVE_Z);
            Update();
        }
        else
        {
            event->ignore();
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mouseMoveEvent handles mouse move events
 * @param event  pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event )
{
    qDebug() << "VTextGraphicsItem::mouseMoveEvent\n";
    qreal xPos;
    qreal yPos;
    QRectF rectBB;
    const QPointF ptDiff = event ->scenePos() - m_start;
    if (m_eMode == mMove && m_moveType & IsMovable)
    {
        qDebug() << " Item is Movable\n";
        // in move mode move the label along the mouse move from the origin
        QPointF pt = m_startPos + ptDiff;
        rectBB.setTopLeft(pt);
        rectBB.setWidth(m_rectBoundingBox.width());
        rectBB.setHeight(m_rectBoundingBox.height());
        // before moving label to a new position, check if it will still be inside the parent item
        if (isContained(rectBB, rotation(), xPos, yPos) == false)
        {
            qDebug() << " Item is contained\n";
            pt.setX(pt.x() + xPos);
            pt.setY(pt.y() + yPos);
        }
        setPos(pt);
        qDebug() << " Item position" << pt;
        UpdateBox();
    }
    else if (m_eMode == mResize && m_moveType & IsResizable)
    {
        qDebug() << " Item is Resizable\n";
        // in resize mode, resize the label along the mouse move from the origin
        QPointF pt;

        if (m_moveType & IsMovable)
        {
            pt = m_startPos;
        }
        else
        {
            pt = m_ptRotCenter - QRectF(0, 0, m_startSize.width() + ptDiff.x(),
                                        m_startSize.height() + ptDiff.y()).center();
        }

        rectBB.setTopLeft(pt);

        // Apply rotation to ptDiff in order to resize based on current rotation
        QTransform transform = QTransform().rotate(-rotation());
        QPointF ptDiff2 = transform.map(ptDiff);

        QSizeF size(m_startSize.width() + ptDiff2.x(), m_startSize.height() + ptDiff2.y());
        rectBB.setSize(size);
        // before resizing the label to a new size, check if it will still be inside the parent item
        if (isContained(rectBB, rotation(), xPos, yPos) == false)
        {
            size = QSizeF(size.width()+xPos, size.height()+yPos);
        }
        else
        {
            if (not (m_moveType & IsMovable))
            {
                setPos(pt);
            }
        }

        setSize(size.width(), size.height());
        qDebug() << " Item size" << size.width() << size.height();
        Update();
        emit textShrink();
    }
    else if (m_eMode == mRotate && m_moveType & IsRotatable)
    {
        qDebug() << " Item is Rotatable\n";
        // if the angle from the original position is small (0.5 degrees), just remeber the new angle
        // new angle will be the starting angle for rotation
        if (fabs(m_angle) < 0.01)
        {
            m_angle = GetAngle(event ->scenePos());
            return;
        }
        // calculate the angle difference from the starting angle
        double angle =  qRadiansToDegrees(GetAngle(event ->scenePos()) - m_angle);
        rectBB.setTopLeft(m_startPos);
        rectBB.setWidth(m_rectBoundingBox.width());
        rectBB.setHeight(m_rectBoundingBox.height());
        // check if the rotated label will be inside the parent item and then rotate it
        if (isContained(rectBB, m_rotation + angle, xPos, yPos) == true)
        {
            setRotation(m_rotation + angle);
            Update();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::mouseReleaseEvent handles left button mouse release events
 * @param event  pointer to QGraphicsSceneMouseEvent object
 */
void VTextGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event )
{
    if (event ->button() == Qt::LeftButton)
    {
        // restore the cursor
        if ((m_eMode == mMove || m_eMode == mRotate || m_eMode == mResize) && (flags() & QGraphicsItem::ItemIsMovable))
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
        double distance = fabs(event ->scenePos().x() - m_start.x()) + fabs(event ->scenePos().y() - m_start.y());
        // determine if this was just press/release (isShort) or user did some operation between press and release
        bool isShort = (distance < 2);

        if (m_eMode == mMove || m_eMode == mResize)
        {   // if user just pressed and released the button, we must switch the mode to rotate
            // but if user did some operation (move/resize), emit the proper signal and update the label
            if (isShort)
            {
                if (m_bReleased  && m_moveType & IsRotatable)
                {
                    m_eMode = mRotate;
                    UpdateBox();
                }
            }
            else if (m_eMode == mMove && m_moveType & IsMovable)
            {
                emit itemMoved(pos());
                UpdateBox();
            }
            else if (m_moveType & IsResizable)
            {
                emit itemResized(m_rectBoundingBox.width(), m_textMananger.GetFont().pixelSize());
                Update();
            }
        }
        else
        {   // in rotate mode, if user did just press/release, switch to move mode
            if (isShort && (m_moveType & IsMovable || m_moveType & IsResizable))
            {
                m_eMode = mMove;
            }
            else if (m_moveType & IsRotatable)
            {
                // if user rotated the item, emit proper signal and update the label
                emit itemRotated(rotation());
            }
            UpdateBox();
        }
        m_bReleased = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::hoverMoveEvent checks if cursor has to be changed
 * @param event pointer to the scene hover event
 */
void VTextGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_eMode == mResize && m_moveType & IsResizable)
    {
        if (m_rectResize.contains(event->pos()))
        {
            setCursor(Qt::SizeFDiagCursor);
        }
        else
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }
    VPieceItem::hoverMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::hoverLeaveEvent tries to restore normal mouse cursor
 * @param event pointer to the scene hover event
 */
void VTextGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    setCursor(QCursor());
    VPieceItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateBox redraws the label content
 */
void VTextGraphicsItem::UpdateBox()
{
    update(m_rectBoundingBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VTextGraphicsItem::UpdateFont sets the text font size, so that the entire text will
 *  just fit into the label bounding box
 */
void VTextGraphicsItem::correctLabel()
{
    qreal xPos;
    qreal yPos;
    QRectF rectBB;
    rectBB.setTopLeft(pos());
    rectBB.setSize(m_rectBoundingBox.size());
    if (isContained(rectBB, rotation(), xPos, yPos) == false)
    {
        // put the label inside the pattern
        setPos(pos().x() + xPos, pos().y() + yPos);
    }
    m_textMananger.FitFontSize(m_rectBoundingBox.width(), m_rectBoundingBox.height());
    UpdateBox();
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::allUserModifications(const QPointF &pos)
{
    if (m_eMode != mRotate)
    {
        userMoveAndResize(pos);
    }
    else
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::userRotateAndMove()
{
    if (m_eMode != mRotate)
    {
        m_eMode = mMove;
    }
    SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
}

//---------------------------------------------------------------------------------------------------------------------
void VTextGraphicsItem::userMoveAndResize(const QPointF &pos)
{
    if (m_rectResize.contains(pos) == true)
    {
        m_eMode = mResize;
        setCursor(Qt::SizeAllCursor);
    }
    else
    {
        m_eMode = mMove; // block later if need
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
    }
}

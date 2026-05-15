/******************************************************************************
 **  @file   image_item.cpp
 **  @author DS Caskey
 **  @date   May 29, 2022
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

#include "image_item.h"
#include "image_dialog.h"

#include "vmaingraphicsscene.h"
#include "vmaingraphicsview.h"
#include "global.h"
#include "../vmisc/vcommonsettings.h"
#include "../vwidgets/resize_handle.h"

#include <QColor>
#include <QEvent>
#include <QImageReader>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QPoint>
#include <QPointF>
#include <QRectF>
#include <QPixmap>
#include <QCursor>
#include <QStyleOptionGraphicsItem>
#include <Qt>
#include <QGraphicsItem>


constexpr qreal OriginMarkerRadius = 6.0;
constexpr qreal RotationHandleWidth = 12.0;
constexpr qreal HandleHitRadius = 10.0;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ImageItem default constructor.
 * @param parent parent object.
 */
ImageItem::ImageItem(QObject *parent, VAbstractPattern *doc, DraftImage image)
    : QObject(parent)
    , m_doc(doc)
    , m_offset(QPointF(0.0, 0.0))
    , m_boundingRect(QRectF())
    , m_handleRect(QRectF())
    , m_actualRect(QRectF())
    , m_resizeHandles()
    , m_resizePosition()
    , m_rotateLine()
    , m_angleHandle()
    , m_angle()
    , m_mousePressed(false)
    , m_draggingOrigin(false)
    , m_draggingRotation(false)
    , m_initialRotation(0.0)
    , m_isHovered(false)
    , m_showHandles(false)
    , m_selectionType(SelectionType::ByMouseRelease)
    , m_transformationMode(Qt::SmoothTransformation)
    , m_image(image)
    , m_pixmapWidth()
    , m_pixmapHeight()
    , m_selectable(true)
    , m_minDimension(16)
    , m_maxDimension(60000)
    , m_selectNewOrigin(false)
{
    initializeItem();

    QImageReader imageReader(m_image.filename);
    m_pixmap = QPixmap::fromImageReader(&imageReader);
    m_pixmapWidth = m_pixmap.width();
    m_pixmapHeight = m_pixmap.height();

    if (m_image.width == 0 || m_image.height == 0)
    {
        m_image.width  = m_pixmapWidth;
        m_image.height = m_pixmapHeight;
    }

    m_boundingRect = QRectF(m_image.xPos - m_image.xOrigin, m_image.yPos - m_image.yOrigin, m_image.width, m_image.height);
    m_handleRect   = m_boundingRect.adjusted(HANDLE_SIZE/2, HANDLE_SIZE/2, -HANDLE_SIZE/2, -HANDLE_SIZE/2);
    m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
    m_rotationHandleDistance = qMin(m_image.width, m_image.height)/2;

    if (m_image.order == 0)
    {
        qint32   minZValue = maxImageZvalue+1;
        foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
        {
            minZValue = qMin(minZValue, item->m_image.order);
        }
        m_image.order = minZValue-1;
        moveToTop();
    }

    updateImage();

    m_resizeHandles = new ResizeHandlesItem(this, m_minDimension, m_maxDimension);
    m_resizeHandles->setLockAspectRatio(m_image.aspectLocked);
    m_resizeHandles->setParentRotation(m_image.rotation);
    m_resizeHandles->parentIsLocked(m_image.locked);
    m_resizeHandles->setVisible(m_image.locked);
    connect(m_resizeHandles, &ResizeHandlesItem::imageNeedsSave, this, [this]() {emit imageNeedsSave();});
    connect(m_resizeHandles, &ResizeHandlesItem::sizeChangedFromHandles, this, &ImageItem::updateFromHandles);
    connect(m_resizeHandles, &ResizeHandlesItem::setStatusMessage, this, [this](QString message) {emit setStatusMessage(message);});
}

//---------------------------------------------------------------------------------------------------------------------
QVariant ImageItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneChange || change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (scene())
        {
            connect(scene(), SIGNAL(ItemClicked(QGraphicsItem*)), this, SLOT(onSceneItemClicked(QGraphicsItem*)), Qt::UniqueConnection);
        }
    }

    return QGraphicsItem::itemChange(change, value);
}


//---------------------------------------------------------------------------------------------------------------------
QRectF ImageItem::boundingRect() const
{
    return m_boundingRect;
}


void ImageItem::setPixmap(const QPixmap &pixmap)
{
    prepareGeometryChange();

    m_pixmap = pixmap;

    m_pixmapWidth  = pixmap.width();
    m_pixmapHeight = pixmap.height();
    m_image.width  = pixmap.width();
    m_image.height = pixmap.height();

    m_boundingRect = QRectF(m_image.xPos, m_image.yPos, m_image.xPos + m_pixmapWidth, m_image.yPos + m_pixmapHeight);

    m_handleRect   = m_boundingRect.adjusted(HANDLE_SIZE/2, HANDLE_SIZE/2, -HANDLE_SIZE/2, -HANDLE_SIZE/2);
}

DraftImage ImageItem::getImage()
{
    return m_image;
}

void ImageItem::setImage(DraftImage image)
{
    m_image = image;
}


void ImageItem::setOrigin(qreal xOrigin, qreal yOrigin)
{
    //m_image.xOrigin / m_image.yOrigin  is the distance between the top left corner of the image and the image origin
    //m_origin is the distance between the real origin of the QGraphicsItem and the image origin

    m_image.xOrigin = xOrigin;
    m_image.yOrigin = yOrigin;

    m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
}


void  ImageItem::updateImage()
{
    prepareGeometryChange();

    setTransformOriginPoint(m_origin);
    setRotation(m_image.rotation);

    setPos(m_image.xPos - m_origin.x(), m_image.yPos - m_origin.y());

    m_boundingRect.setSize(QSizeF(m_image.width, m_image.height));

    setVisible(m_image.visible);
    setOpacity(m_image.opacity/100);

    setLock(m_image.locked);

    setZValue(m_image.order);
}

void ImageItem::setLock(bool checked)
{
    m_image.locked = checked;
    if (m_image.locked)
    {
        setAcceptedMouseButtons(Qt::RightButton);
        emit setStatusMessage("");
    }
    else
    {
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    }
    setFlag(QGraphicsItem::ItemIsMovable, !m_image.locked);
    //enableSelection(!m_image.locked);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (isSelected())
    {
        painter->save();
        painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
        painter->drawRect(m_boundingRect);
        painter->restore();
    }

    if (!m_image.locked && m_showHandles)
    {
        painter->save();
        QColor color = QColor(qApp->Settings()->getTertiarySupportColor());
        color.setAlpha(20);
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setBrush(color);
        painter->drawRect(m_boundingRect);
        painter->restore();
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, (m_transformationMode == Qt::SmoothTransformation));
    painter->drawPixmap(m_boundingRect.x(), m_boundingRect.y(), m_image.width, m_image.height, m_pixmap);

    if (!m_image.locked && m_showHandles)
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
        const QPointF rotationHandlePosition = m_origin + QPointF(0.0, -m_rotationHandleDistance);
        painter->drawLine(QLineF(m_origin, rotationHandlePosition));

        painter->setBrush(Qt::darkGray);
        painter->drawRect(rotationHandlePosition.x()-RotationHandleWidth/2, rotationHandlePosition.y()-RotationHandleWidth/2, RotationHandleWidth, RotationHandleWidth);

        painter->setBrush(QBrush(Qt::transparent));
        painter->drawEllipse(m_origin, OriginMarkerRadius, OriginMarkerRadius);
        painter->drawLine(QLineF(m_origin.x()-OriginMarkerRadius, m_origin.y(), m_origin.x()+OriginMarkerRadius, m_origin.y()));
        painter->drawLine(QLineF(m_origin.x(), m_origin.y()-OriginMarkerRadius, m_origin.x(), m_origin.y()+OriginMarkerRadius));
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::enableSelection(bool enable)
{
    m_selectable = enable;
    setFlag(QGraphicsItem::ItemIsSelectable, enable);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageItem::enableHovering(bool enable)
{
    Q_UNUSED(enable);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(qApp->getSceneView()->viewport()->cursor());
    }

    //override cursor if in new origin selection mode
    if (m_selectNewOrigin)
    {
        SetItemOverrideCursor(this, cursorImageOrigin, 16, 16);
    }

    if (!m_image.locked)
    {
        if (!m_selectNewOrigin)
        {
            // m_resizeHandles->show(); // Removed: handles now show on click
        }
        showImageStatusMessage();
    }

    QGraphicsItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }

    if(!m_image.locked)
    {
        emit setStatusMessage("");
        // m_resizeHandles->hide(); // Removed: handles now hide on click outside
    }

    QGraphicsItem::hoverLeaveEvent(event);
}


void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_image.locked)
    {
        const QPointF mousePos = event->pos();
        const QPointF rotationHandleCenter = m_origin + QPointF(0.0, -m_rotationHandleDistance);
        const qreal originDistance = QLineF(mousePos, m_origin).length();
        const qreal rotationDistance = QLineF(mousePos, rotationHandleCenter).length();

        if (rotationDistance <= RotationHandleWidth + 2.0)
        {
            SetItemOverrideCursor(this, cursorResizeArrow, 16, 16);
        }
        else if (originDistance <= OriginMarkerRadius + 2.0)
        {
            SetItemOverrideCursor(this, cursorImageOrigin, 16, 16);
        }
        else
        {
            showImageStatusMessage();
        }
    }
    QGraphicsItem::hoverMoveEvent(event);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void ImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!m_selectable)
    {
        return;
    }

    QMenu menu;
    QAction *actionProperties = menu.addAction(QIcon::fromTheme("preferences-other"), tr("Properties"));
    actionProperties->setEnabled(!m_selectNewOrigin);

    QAction *actionLock = menu.addAction(tr("Lock"));
    if (m_image.locked){
        actionLock->setIcon(QIcon("://icon/32x32/lock_on.png"));
    }
    else{
        actionLock->setIcon(QIcon("://icon/32x32/lock_off.png"));
    }
    actionLock->setCheckable(true);
    actionLock->setChecked(m_image.locked);
    actionLock->setEnabled(!m_selectNewOrigin);

    // QAction *actionShow = menu.addAction(QIcon("://icon/32x32/visible_on.png"), tr("Show"));
    // actionShow->setCheckable(true);
    // actionShow->setChecked(m_image.visible);
    // actionShow->setEnabled(!m_image.locked);

    QAction *actionOrigin = menu.addAction(tr("Move Origin"));
    actionOrigin->setIcon(QIcon(cursorImageOrigin));
    actionOrigin->setEnabled(!m_image.locked && !m_selectNewOrigin);

    QAction *actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    menu.addAction(actionSeparator);

    QMenu *orderMenu;
    orderMenu = menu.addMenu(tr("Order"));
    QAction *actionMoveTop = orderMenu->addAction(tr("Bring to top"));
    QAction *actionMoveUp = orderMenu->addAction(tr("Move up"));
    QAction *actionMoveDn = orderMenu->addAction(tr("Move down"));
    QAction *actionMoveBottom = orderMenu->addAction(tr("Send to bottom"));

    orderMenu->setEnabled(!m_image.locked && !m_selectNewOrigin);
    actionMoveTop->setEnabled(!m_image.locked && !m_selectNewOrigin);
    actionMoveUp->setEnabled(!m_image.locked && !m_selectNewOrigin);
    actionMoveDn->setEnabled(!m_image.locked && !m_selectNewOrigin);
    actionMoveBottom->setEnabled(!m_image.locked && !m_selectNewOrigin);

    //actionMoveTop->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Home));
    //actionMoveUp->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageUp));
    //actionMoveDn->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageDown));
    //actionMoveBottom->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_End));

    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    menu.addAction(actionSeparator);

    QAction *actionDelete = menu.addAction(QIcon("://icon/32x32/trashcan.png"), tr("Delete"));
    actionDelete->setEnabled(!m_image.locked && !m_selectNewOrigin);

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == actionProperties)
    {
        ImageDialog *dialog = new ImageDialog(m_image, m_minDimension, m_maxDimension, m_pixmapWidth, m_pixmapHeight, qApp->getMainWindow());
        connect(dialog, &ImageDialog::applyClicked, this, &ImageItem::updateImageAndHandles);

        if (dialog->exec() == QDialog::Accepted)
        {
            updateImageAndHandles(dialog->getImage());
            emit imageNeedsSave();
        }
    }
    else if (selectedAction == actionLock)
    {
        m_image.locked = !m_image.locked;
        updateImageAndHandles(m_image);
        emit imageNeedsSave();
    }
    // else if (selectedAction == actionShow)
    // {
    //     SetVisible(selectedAction->isChecked());
    // }
    else if (selectedAction == actionOrigin)
    {
        if (!m_image.locked)
        {
            m_selectNewOrigin = true;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resizeHandles->hide();
        }
    }
    else if (selectedAction == actionDelete)
    {
        if (!m_image.locked)
        {
            emit deleteImage(m_image.id);
        }
    }

    else if (selectedAction == actionMoveTop)
    {
        moveToTop();
        emit imageNeedsSave();
    }
    else if (selectedAction == actionMoveUp)
    {
        moveUp();
        emit imageNeedsSave();
    }
    else if (selectedAction == actionMoveDn)
    {
        moveDown();
        emit imageNeedsSave();
    }
    else if (selectedAction == actionMoveBottom)
    {
        moveToBottom();
        emit imageNeedsSave();
    }

    emit showContextMenu(event);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_selectable)
    {
        event->ignore();
        return;
    }

    const QPointF mousePos = event->pos();
    const QPointF rotationHandleCenter = m_origin + QPointF(0.0, -RotationHandleDistance);
    const qreal originDistance = QLineF(mousePos, m_origin).length();
    const qreal rotationDistance = QLineF(mousePos, rotationHandleCenter).length();

    if (event->button() == Qt::LeftButton && !m_image.locked)
    {
        if (rotationDistance <= RotationHandleRadius + 2.0)
        {
            m_draggingRotation = true;
            m_initialRotation = m_image.rotation;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resizeHandles->hide();
            event->accept();
            return;
        }

        if (originDistance <= OriginMarkerWidth + 2.0)
        {
            m_selectNewOrigin = true;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resizeHandles->hide();
            m_image.xOrigin = event->pos().x() - m_boundingRect.topLeft().x();
            m_image.yOrigin = event->pos().y() - m_boundingRect.topLeft().y();
            m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
            m_image.xPos = mapToScene(event->pos()).x();
            m_image.yPos = mapToScene(event->pos()).y();
            updateImage();
            scene()->update();
            event->accept();
            return;
        }
    }

    const QPointF mousePos = event->pos();
    const QPointF rotationHandleCenter = m_origin + QPointF(0.0, -m_rotationHandleDistance);
    const qreal originDistance = QLineF(mousePos, m_origin).length();
    const qreal rotationDistance = QLineF(mousePos, rotationHandleCenter).length();

    if (event->button() == Qt::LeftButton && !m_image.locked)
    {
        if (rotationDistance <= RotationHandleWidth + 2.0)
        {
            m_draggingRotation = true;
            m_initialRotation = m_image.rotation;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resizeHandles->hide();
            event->accept();
            return;
        }

        if (originDistance <= OriginMarkerRadius + 2.0)
        {
            m_selectNewOrigin = true;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resizeHandles->hide();
            m_image.xOrigin = event->pos().x() - m_boundingRect.topLeft().x();
            m_image.yOrigin = event->pos().y() - m_boundingRect.topLeft().y();
            m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
            m_image.xPos = mapToScene(event->pos()).x();
            m_image.yPos = mapToScene(event->pos()).y();
            updateImage();
            scene()->update();
            event->accept();
            return;
        }
    }

    // Special for not selectable item first need to call standard mousePressEvent then accept event
    //QGraphicsItem::mousePressEvent(event);

    // Somehow clicking on non selectable object does not clear previous selections.
    if (not (flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }
    m_mousePressed = true;

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            m_offset = event->pos() - m_origin;
        }
    }
    if (m_selectionType == SelectionType::ByMouseRelease)
    {
        event->accept(); // This help for non selectable items still receive mouseReleaseEvent events
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            emit imageSelected(m_image.id);
            event->accept();
        }
    }

    if(m_selectNewOrigin)
    {
        m_image.xOrigin = event->pos().x() - m_boundingRect.topLeft().x();
        m_image.yOrigin = event->pos().y() - m_boundingRect.topLeft().y();

        m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);

        m_image.xPos = mapToScene(event->pos()).x();
        m_image.yPos = mapToScene(event->pos()).y();
    }
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    prepareGeometryChange();

    if (m_draggingRotation && event->buttons() & Qt::LeftButton)
    {
        const QPointF delta = event->pos() - m_origin;
        if (!qFuzzyIsNull(delta.x()) || !qFuzzyIsNull(delta.y()))
        {
            qreal angle = qAtan2(delta.y(), delta.x()) * 180.0 / M_PI;
            qreal newRotation = angle + 90.0;
            if (newRotation > 180.0)
            {
                newRotation -= 360.0;
            }
            else if (newRotation <= -180.0)
            {
                newRotation += 360.0;
            }
            m_image.rotation = newRotation;
            updateImage();
            scene()->update();
        }
        event->accept();
        return;
    }

    if (m_selectNewOrigin && event->buttons() & Qt::LeftButton)
    {
        m_image.xOrigin = event->pos().x() - m_boundingRect.topLeft().x();
        m_image.yOrigin = event->pos().y() - m_boundingRect.topLeft().y();
        m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
        m_image.xPos = mapToScene(event->pos()).x();
        m_image.yPos = mapToScene(event->pos()).y();

        showImageStatusMessage();
        updateImage();
        scene()->update();
        event->accept();
        return;
    }

    if (flags() & QGraphicsItem::ItemIsMovable && event->buttons() & Qt::LeftButton)
    {
        m_imageWasMoved = true;

        m_image.xPos = mapToScene(event->pos() - m_offset).x();
        m_image.yPos = mapToScene(event->pos() - m_offset).y();

        showImageStatusMessage();
        updateImage();
        scene()->update();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(m_imageWasMoved){
        m_imageWasMoved = false;
        emit imageNeedsSave();
    }

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }

    if (m_selectionType == SelectionType::ByMouseRelease)
    {
        emit imageSelected(m_image.id);
    }

    if (m_draggingRotation)
    {
        m_draggingRotation = false;
        setFlag(QGraphicsItem::ItemIsMovable, true);
        if (!m_image.locked)
        {
            m_showHandles = true;
            m_resizeHandles->show();
        }
        emit imageNeedsSave();
    }

    if(m_selectNewOrigin)
    {
        m_selectNewOrigin = false;
        setFlag(QGraphicsItem::ItemIsMovable, true);
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        m_showHandles = true;
        m_resizeHandles->show();
        emit imageNeedsSave();
    }

    m_mousePressed = false;
    QGraphicsItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            if (!m_image.locked && isSelected())
            {
                emit deleteImage(m_image.id);
            }
        case Qt::Key_Escape:
            if (m_selectNewOrigin)
            {
                m_selectNewOrigin = false;
                setFlag(QGraphicsItem::ItemIsMovable, true);
                SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
                m_resizeHandles->show();
            }
        default:
            break;
    }
    QGraphicsItem::keyReleaseEvent (event);
}

void ImageItem::initializeItem()
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setAcceptHoverEvents(true);
    //enableSelection(false);
}

void ImageItem::updateFromHandles(QRectF rect)
{
    prepareGeometryChange();

    //The image origin is moved so that it stays at the same place on the image
    //The image origin is different from the QGraphicsItem origin, see below
    m_image.xOrigin = m_image.xOrigin / m_image.width * rect.width();
    m_image.yOrigin = m_image.yOrigin / m_image.height * rect.height();

    //m_image.xOrigin / m_image.yOrigin  is the distance between the top left corner of the image and the image origin
    //m_origin is the distance between the real origin of the QGraphicsItem and the image origin
    m_origin = rect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);

    m_image.xPos = mapToScene(m_origin).x();
    m_image.yPos = mapToScene(m_origin).y();
    m_image.width = rect.width();
    m_image.height = rect.height();

    m_boundingRect.setTopLeft(rect.topLeft());

    updateImage();
    scene()->update();
}


void ImageItem::updateImageAndHandles(DraftImage image)
{
    m_image = image;
    m_origin = m_boundingRect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
    updateImage();
    m_resizeHandles->setParentRect(m_boundingRect);
    m_resizeHandles->setLockAspectRatio(m_image.aspectLocked);
    m_resizeHandles->setParentRotation(m_image.rotation);
    m_resizeHandles->parentIsLocked(m_image.locked);
    m_resizeHandles->setVisible(m_image.locked);
}

void ImageItem::deleteImageItem()
{
    moveToBottom(); //so that there is no gap in zValue
    scene()->removeItem(this);
    deleteLater();
}

void ImageItem::moveToBottom()
{
    qint32   minZValue = m_image.order;
    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item != this && item->m_image.order < m_image.order)
        {
            minZValue = qMin(minZValue, item->m_image.order);
            item->m_image.order++;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order = minZValue;
    updateImage();
}


void ImageItem::moveToTop()
{
    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item != this && item->m_image.order > m_image.order)
        {
            item->m_image.order--;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order = maxImageZvalue;
    updateImage();
}

void ImageItem::moveUp()
{
    if (m_image.order == maxImageZvalue)
    {
        return;
    }

    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item->m_image.order == m_image.order + 1)
        {
            item->m_image.order--;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order ++;
    updateImage();
}


void ImageItem::moveDown()
{
    if (m_image.order == maxImageZvalue-m_doc->getBackgroundImageMap().values().size()+1)
    {
        return;
    }

    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item->m_image.order == m_image.order - 1)
        {
            item->m_image.order++;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order --;
    updateImage();
}


void ImageItem::showImageStatusMessage()
{
    QString width;
    QString height;
    QString posX;
    QString posY;

    if (m_image.units == Unit::Px)
    {
        width = QString::number(m_image.width);
        height = QString::number(m_image.height);
        posX = QString::number(m_image.xPos);
        posY = QString::number(m_image.yPos);
    }
    else
    {
        width = QString::number(qApp->fromPixel(m_image.width));
        height = QString::number(qApp->fromPixel(m_image.height));
        posX = QString::number(qApp->fromPixel(m_image.xPos));
        posY = QString::number(qApp->fromPixel(m_image.yPos));
    }

    QString message = QString(tr("<b>Image (%7)</b>: Size(%2%1, %3%1); Pos(%4%1, %5%1); Rot(%6°)%8"))
                          .arg(UnitsToStr(m_image.units))
                          .arg(width)
                          .arg(height)
                          .arg(posX)
                          .arg(posY)
                          .arg(m_image.rotation)
                          .arg(m_image.name)
                          .arg(!m_image.aspectLocked ? "" : tr(" - <b>Aspect ratio locked</b>"));

    emit setStatusMessage(message);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::hideHandles()
{
    m_showHandles = false;
    m_resizeHandles->hide();
    update(); // to repaint without handles
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::onSceneItemClicked(QGraphicsItem *item)
{
    // Hide handles only when the clicked item is not this image or one of its children.
    QGraphicsItem *clickedItem = item;
    while (clickedItem)
    {
        if (clickedItem == this)
        {
            return;
        }
        clickedItem = clickedItem->parentItem();
    }

    hideHandles();
}
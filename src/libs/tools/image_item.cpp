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
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vwidgets/resize_handle.h"

#include <QColor>
#include <QEvent>
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
#include <QStyleOptionGraphicsItem>
#include <Qt>
#include <QGraphicsItem>


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ImageItem default constructor.
 * @param parent parent object.
 */
ImageItem::ImageItem(DraftImage image, QGraphicsItem *parent)
    : QGraphicsItem(parent)
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
    , m_isResizing (false)
    , m_isHovered(false)
    , m_selectionType(SelectionType::ByMouseRelease)
    , m_transformationMode(Qt::SmoothTransformation)
    , m_hasShape(false)
    , m_image(image)
    , m_pixmapWidth()
    , m_pixmapHeight()
    , m_selectable(true)
{
    initializeItem();
    setPixmap(m_image.pixmap);

    m_resizeHandles = new ResizeHandlesItem(this);
    connect(m_resizeHandles, &ResizeHandlesItem::sizeChanged, this, &ImageItem::updateFromHandles);

    updateImage();
}


//---------------------------------------------------------------------------------------------------------------------
QRectF ImageItem::boundingRect() const
{
    return m_boundingRect;
}


void ImageItem::setPixmap(const QPixmap &pixmap)
{
    m_image.pixmap = pixmap;
    m_pixmapWidth  = pixmap.width();
    m_pixmapHeight = pixmap.height();
    m_image.width  = pixmap.width();
    m_image.height = pixmap.height();
    m_image.xScale = m_image.width / m_pixmapWidth * 100;
    m_image.yScale = m_image.height / m_pixmapHeight * 100;
    m_hasShape     = false;
    m_boundingRect = QRectF(m_image.xPos, m_image.yPos, m_image.xPos + m_pixmapWidth, m_image.yPos + m_pixmapHeight);
    m_handleRect   = m_boundingRect.adjusted(HANDLE_SIZE/2, HANDLE_SIZE/2, -HANDLE_SIZE/2, -HANDLE_SIZE/2);

    prepareGeometryChange();
}

DraftImage ImageItem::getImage()
{
    return m_image;
}

void ImageItem::setImage(DraftImage image)
{
    m_image = image;
}

void  ImageItem::updateImage()
{
    setPos(m_image.xPos - m_boundingRect.topLeft().x(), m_image.yPos - m_boundingRect.topLeft().y());
    m_boundingRect.setSize(QSizeF(m_image.width, m_image.height));

    // QTransform transform;
    // transform.translate(m_boundingRect.center().x(), m_boundingRect.center().y());
    // transform.scale(m_image.width / m_pixmapWidth, m_image.height / m_pixmapHeight);
    // transform.rotate(m_image.rotation);
    // transform.translate(-m_boundingRect.center().x(), -m_boundingRect.center().y());
    // setTransform(transform);

    setVisible(m_image.visible);
    setOpacity(m_image.opacity/100);

    emit imageUpdated(m_image);

    prepareGeometryChange();
}

void ImageItem::setLock(bool checked)
{
    m_image.locked = checked;
}


//---------------------------------------------------------------------------------------------------------------------
void ImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_isHovered)
    {
        //highlight and draw border
        painter->save();
        QColor color = QColor(qApp->Settings()->getTertiarySupportColor());
        color.setAlpha(20);
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setBrush(color);
        painter->drawRect(m_boundingRect);
        painter->restore();
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, (m_transformationMode == Qt::SmoothTransformation));
    painter->drawPixmap(m_boundingRect.x(), m_boundingRect.y(), m_image.width, m_image.height, m_image.pixmap);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::enableSelection(bool enable)
{
    m_selectable = enable;
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, m_selectable);
    setFlag(QGraphicsItem::ItemIsFocusable, m_selectable);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    if (m_selectable && flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(qApp->getSceneView()->viewport()->cursor());
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

    QGraphicsItem::hoverLeaveEvent(event);
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
    QAction *actionProperties = menu.addAction(QIcon::fromTheme("preferences-other"), tr("Properties") + "\tP");

    QAction *actionLock = menu.addAction(QIcon("://icon/32x32/lock_on.png"), tr("Lock"));
    actionLock->setCheckable(true);
    actionLock->setChecked(m_image.locked);

    QAction *actionShow = menu.addAction(QIcon("://icon/32x32/visible_on.png"), tr("Show"));
    actionShow->setCheckable(true);
    actionShow->setChecked(m_image.visible);
    actionShow->setEnabled(!m_image.locked);

    QAction *actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    menu.addAction(actionSeparator);

    QMenu *orderMenu;
    orderMenu = menu.addMenu(tr("Order"));
    QAction *actionMoveTop = orderMenu->addAction(tr("Bring to top"));
    QAction *actionMoveUp = orderMenu->addAction(tr("Move up"));
    QAction *actionMoveDn = orderMenu->addAction(tr("Move down"));
    QAction *actionMoveBottom = orderMenu->addAction(tr("Send to bottom"));

    actionMoveTop->setEnabled(!m_image.locked);
    actionMoveUp->setEnabled(!m_image.locked);
    actionMoveDn->setEnabled(!m_image.locked);
    actionMoveBottom->setEnabled(!m_image.locked);

    actionMoveTop->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Home));
    actionMoveUp->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageUp));
    actionMoveDn->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageDown));
    actionMoveBottom->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_End));

    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    menu.addAction(actionSeparator);

    QAction *actionDelete = menu.addAction(QIcon("://icon/32x32/trashcan.png"), tr("Delete") + "\tDel");
    actionDelete->setEnabled(!m_image.locked);

    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == actionProperties)
    {
        ImageDialog *dialog = new ImageDialog(m_image);

        if (dialog->exec() == QDialog::Accepted)
        {
            m_image = dialog->getImage();
            updateImage();
            m_resizeHandles->setParentRect(m_boundingRect);
            emit imageUpdated(m_image);
        }
    }
    else if (selectedAction == actionLock)
    {
        m_image.locked = !m_image.locked;
        this->setFlag(QGraphicsItem::ItemIsMovable, !m_image.locked);
        emit imageUpdated(m_image);
    }
    else if (selectedAction == actionShow)
    {
        //SetVisible(selectedAction->isChecked());
    }
    else if (selectedAction == actionDelete)
    {
        if (!m_image.locked)
        {
            //emit deleteImage(m_image.id);
        }
    }

    else if (selectedAction == actionMoveTop)
    {

    }
    else if (selectedAction == actionMoveUp)
    {

    }
    else if (selectedAction == actionMoveDn)
    {

    }
    else if (selectedAction == actionMoveBottom)
    {

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

    // Special for not selectable item first need to call standard mousePressEvent then accept event
    //QGraphicsItem::mousePressEvent(event);

    // Somehow clicking on non selectable object does not clear previous selections.
    if (not (flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }
    m_mousePressed = true;
    if (m_isResizing)
    {
      m_actualRect = m_boundingRect;
    }

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            m_offset = event->pos();
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
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        qDebug() << "Item moved";
        qDebug() << "mapToScene(event->pos())" << mapToScene(event->pos());
        qDebug() << "m_offset" << m_offset;

        m_image.xPos = mapToScene(event->pos() - m_offset).x() + m_boundingRect.topLeft().x();
        m_image.yPos = mapToScene(event->pos() - m_offset).y() + m_boundingRect.topLeft().y();

        updateImage();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
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

    m_mousePressed = false;
    m_isResizing = false;
    /*if (m_actualRect != m_boundingRect)
    { // Rotating won't trigger this, only resizing.
      auto oldScenePos = scenePos();
      setTransformOriginPoint(m_boundingRect.center());
      auto newScenePos = scenePos();
      auto oldPos = pos();
      setPos(oldPos.x() + (oldScenePos.x() - newScenePos.x()), oldPos.y() + (oldScenePos.y() - newScenePos.y()));
  }

    m_image.xPos = mapToScene(m_boundingRect.topLeft()).x();
    m_image.yPos = mapToScene(m_boundingRect.topLeft()).y();
    m_image.width  = m_boundingRect.width();
    m_image.height = m_boundingRect.height();
    updateGeometry(m_boundingRect);*/

    QGraphicsItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageItem::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
        if (!m_image.locked)
        {
            //emit deleteImage(m_image.id);
        }
        default:
            break;
    }
    QGraphicsItem::keyReleaseEvent (event);
}

void ImageItem::initializeItem()
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    //this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus
    this->setAcceptHoverEvents(true);
}

void ImageItem::updateFromHandles(QRectF rect)
{
    m_image.xPos = mapToScene(rect.topLeft()).x();
    m_image.yPos = mapToScene(rect.topLeft()).y();
    m_image.width = rect.width();
    m_image.height = rect.height();

    m_boundingRect.setTopLeft(rect.topLeft());

    updateImage();
}

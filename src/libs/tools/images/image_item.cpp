//-----------------------------------------------------------------------------
//  @file   image_item.cpp
//  @author Douglas S Caskey
//  @date   29 May, 2022
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

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
#include <QStyleOptionGraphicsItem>
#include <Qt>
#include <QGraphicsItem>

//---------------------------------------------------------------------------------------------------------------------
///@brief ImageItem default constructor.
///@param parent parent object.
//---------------------------------------------------------------------------------------------------------------------
ImageItem::ImageItem(QObject *parent, VAbstractPattern *doc, DraftImage image)
    : QObject(parent)
    , m_doc(doc)
    , m_offset(QPointF(0.0, 0.0))
    , m_bounding_rect(QRectF())
    , m_handle_rect(QRectF())
    , m_actual_rect(QRectF())
    , m_resize_handles()
    , m_resize_position()
    , m_rotate_line()
    , m_handle_angle()
    , m_angle()
    , m_mouse_pressed(false)
    , m_is_hovered(false)
    , m_selection_type(SelectionType::ByMouseRelease)
    , m_transformation_mode(Qt::SmoothTransformation)
    , m_image(image)
    , m_pixmap_width()
    , m_pixmap_height()
    , m_selectable(true)
    , m_min_dimension(16)
    , m_max_dimension(60000)
    , m_select_new_origin(false)
    , m_image_was_moved(false)
{
    initializeItem();

    QImageReader image_reader(m_image.filename);

    if (image_reader.format().toLower() == "svg")
    {
        QSize native_size = image_reader.size();
        if (native_size.isValid())
        {
            // Use fallback DPI that Qt defaults to internally
            qreal fallback_dpi = 90.0;

            // Calculate the precision correction scalar to reach exactly 96 DPI
            qreal scale_factor = 96.0 / fallback_dpi;

            // Force QImageReader to scale the vector layout before it rasterizes
            QSize target_size(qRound(native_size.width() * scale_factor),
                              qRound(native_size.height() * scale_factor));

            image_reader.setScaledSize(target_size);
        }
    }

    m_pixmap = QPixmap::fromImageReader(&image_reader);
    m_pixmap_width = m_pixmap.width();
    m_pixmap_height = m_pixmap.height();

    if (m_image.width == 0 || m_image.height == 0)
    {
        m_image.width  = m_pixmap_width;
        m_image.height = m_pixmap_height;
    }

    m_bounding_rect = QRectF(m_image.xPos - m_image.xOrigin,
                             m_image.yPos - m_image.yOrigin,
                             m_image.width,
                             m_image.height);
    m_handle_rect   = m_bounding_rect.adjusted(HANDLE_SIZE/2, HANDLE_SIZE/2, -HANDLE_SIZE/2, -HANDLE_SIZE/2);
    m_origin        = m_bounding_rect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);

    if (m_image.order == 0)
    {
        qint32   min_z_value = max_z_value+1;
        foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
        {
            min_z_value = qMin(min_z_value, item->m_image.order);
        }
        m_image.order = min_z_value-1;
        moveToTop();
    }

    updateImage();

    m_resize_handles = new ResizeHandlesItem(this, m_min_dimension, m_max_dimension);
    m_resize_handles->setLockAspectRatio(m_image.aspectLocked);
    m_resize_handles->setParentRotation(m_image.rotation);
    m_resize_handles->parentIsLocked(m_image.locked);
    m_resize_handles->setVisible(m_image.locked);

    connect(m_resize_handles, &ResizeHandlesItem::imageNeedsSave,
            this, [this]() {emit imageNeedsSave();});

    connect(m_resize_handles, &ResizeHandlesItem::sizeChangedFromHandles,
            this, &ImageItem::updateFromHandles);

    connect(m_resize_handles, &ResizeHandlesItem::setStatusMessage,
            this, [this](QString message) {emit setStatusMessage(message);});
}


//---------------------------------------------------------------------------------------------------------------------
QRectF ImageItem::boundingRect() const
{
    return m_bounding_rect;
}


void ImageItem::setPixmap(const QPixmap &pixmap)
{
    prepareGeometryChange();

    m_pixmap = pixmap;

    m_pixmap_width  = pixmap.width();
    m_pixmap_height = pixmap.height();
    m_image.width   = pixmap.width();
    m_image.height  = pixmap.height();

    m_bounding_rect = QRectF(m_image.xPos, m_image.yPos, m_image.xPos + m_pixmap_width, m_image.yPos + m_pixmap_height);

    m_handle_rect   = m_bounding_rect.adjusted(HANDLE_SIZE/2, HANDLE_SIZE/2, -HANDLE_SIZE/2, -HANDLE_SIZE/2);
}

DraftImage ImageItem::getImage()
{
    return m_image;
}

void ImageItem::setImage(DraftImage image)
{
    m_image = image;
}


void ImageItem::setOrigin(qreal x_origin, qreal y_origin)
{
    //m_image.xOrigin / m_image.yOrigin  is the distance between the top left corner of the image and the image origin
    //m_origin is the distance between the real origin of the QGraphicsItem and the image origin

    m_image.xOrigin = x_origin;
    m_image.yOrigin = y_origin;

    m_origin = m_bounding_rect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
}


void  ImageItem::updateImage()
{
    prepareGeometryChange();

    setTransformOriginPoint(m_origin);
    setRotation(m_image.rotation);

    setPos(m_image.xPos - m_origin.x(), m_image.yPos - m_origin.y());

    m_bounding_rect.setSize(QSizeF(m_image.width, m_image.height));

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
        painter->drawRect(m_bounding_rect);
        painter->restore();
    }

    if (!m_image.locked && m_is_hovered)
    {
        painter->save();
        QColor color = QColor(qApp->Settings()->getTertiarySupportColor());
        color.setAlpha(20);
        painter->setPen(QPen(Qt::black, 2, Qt::DashLine));
        painter->setBrush(color);
        painter->drawRect(m_bounding_rect);
        painter->restore();
    }

    painter->setRenderHint(QPainter::SmoothPixmapTransform, (m_transformation_mode == Qt::SmoothTransformation));
    painter->drawPixmap(m_bounding_rect.x(), m_bounding_rect.y(), m_image.width, m_image.height, m_pixmap);

    if (m_origin != m_bounding_rect.topLeft())
    {
        painter->save();
        painter->setPen(QPen(Qt::blue, 2, Qt::SolidLine));
        qreal x1 = qMax(m_origin.x() - 8, m_bounding_rect.x());
        qreal y1 = qMax(m_origin.y() - 8, m_bounding_rect.y());
        qreal x2 = qMin(m_origin.x() + 8, m_bounding_rect.x() + m_bounding_rect.width());
        qreal y2 = qMin(m_origin.y() + 8, m_bounding_rect.y() + m_bounding_rect.height());
        painter->drawLine(QLineF(x1, m_origin.y(), x2, m_origin.y()));
        painter->drawLine(QLineF(m_origin.x(), y1, m_origin.x(), y2));
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
/// @brief hoverEnterEvent handle hover enter events.
/// @param event hover enter event.
//---------------------------------------------------------------------------------------------------------------------
void ImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_is_hovered = true;

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(qApp->getSceneView()->viewport()->cursor());
    }

    //override cursor if in new origin selection mode
    if (m_select_new_origin)
    {
        SetItemOverrideCursor(this, cursorImageOrigin, 16, 16);
    }

    if (!m_image.locked)
    {
        if (!m_select_new_origin)
        {
            m_resize_handles->show();
        }
        showImageStatusMessage();
    }

    QGraphicsItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief hoverLeaveEvent handle hover leave events.
/// @param event hover leave event.
//---------------------------------------------------------------------------------------------------------------------
void ImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_is_hovered = false;

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }

    if(!m_image.locked)
    {
        emit setStatusMessage("");
        m_resize_handles->hide();
    }

    QGraphicsItem::hoverLeaveEvent(event);
}


void ImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (!m_image.locked)
    {
        showImageStatusMessage();
    }
    QGraphicsItem::hoverMoveEvent(event);
}


//---------------------------------------------------------------------------------------------------------------------
/// @brief contextMenuEvent handle context menu events.
/// @param event context menu event.
//---------------------------------------------------------------------------------------------------------------------
void ImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (!m_selectable)
    {
        return;
    }

    QMenu menu;
    QAction *action_properties = menu.addAction(QIcon::fromTheme("preferences-other"), tr("Properties"));
    action_properties->setEnabled(!m_select_new_origin);

    QAction *actionLock = menu.addAction(tr("Lock"));
    if (m_image.locked)
    {
        actionLock->setIcon(QIcon("://icon/32x32/lock_on.png"));
    }
    else
    {
        actionLock->setIcon(QIcon("://icon/32x32/lock_off.png"));
    }
    actionLock->setCheckable(true);
    actionLock->setChecked(m_image.locked);
    actionLock->setEnabled(!m_select_new_origin);

    // QAction *actionShow = menu.addAction(QIcon("://icon/32x32/visible_on.png"), tr("Show"));
    // actionShow->setCheckable(true);
    // actionShow->setChecked(m_image.visible);
    // actionShow->setEnabled(!m_image.locked);

    QAction *origin_action = menu.addAction(tr("Move Origin"));
    origin_action->setIcon(QIcon(cursorImageOrigin));
    origin_action->setEnabled(!m_image.locked && !m_select_new_origin);

    QAction *action_separator = new QAction(this);
    action_separator->setSeparator(true);
    menu.addAction(action_separator);

    QMenu *order_menu;
    order_menu = menu.addMenu(tr("Order"));
    QAction *move_top_action    = order_menu->addAction(tr("Bring to top"));
    QAction *move_up_action     = order_menu->addAction(tr("Move up"));
    QAction *move_down_action   = order_menu->addAction(tr("Move down"));
    QAction *move_bottom_action = order_menu->addAction(tr("Send to bottom"));

    order_menu->setEnabled(!m_image.locked && !m_select_new_origin);
    move_top_action->setEnabled(!m_image.locked && !m_select_new_origin);
    move_up_action->setEnabled(!m_image.locked && !m_select_new_origin);
    move_down_action->setEnabled(!m_image.locked && !m_select_new_origin);
    move_bottom_action->setEnabled(!m_image.locked && !m_select_new_origin);

    //move_top_action->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_Home));
    //move_up_action->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageUp));
    //move_down_action->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_PageDown));
    //move_bottom_action->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_End));

    action_separator = new QAction(this);
    action_separator->setSeparator(true);
    menu.addAction(action_separator);

    QAction *delete_action = menu.addAction(QIcon("://icon/32x32/trashcan.png"), tr("Delete"));
    delete_action->setEnabled(!m_image.locked && !m_select_new_origin);

    QAction *selected_action = menu.exec(event->screenPos());

    if (selected_action == action_properties)
    {
        ImageDialog *dialog = new ImageDialog(m_image,
                                              m_min_dimension,
                                              m_max_dimension,
                                              m_pixmap_width,
                                              m_pixmap_height,
                                              qApp->getMainWindow());

        connect(dialog, &ImageDialog::applyClicked, this, &ImageItem::updateImageAndHandles);

        if (dialog->exec() == QDialog::Accepted)
        {
            updateImageAndHandles(dialog->getImage());
            emit imageNeedsSave();
        }
    }
    else if (selected_action == actionLock)
    {
        m_image.locked = !m_image.locked;
        updateImageAndHandles(m_image);
        emit imageNeedsSave();
    }
    else if (selected_action == origin_action)
    {
        if (!m_image.locked)
        {
            m_select_new_origin = true;
            setFlag(QGraphicsItem::ItemIsMovable, false);
            m_resize_handles->hide();
        }
    }
    else if (selected_action == delete_action)
    {
        if (!m_image.locked)
        {
            emit deleteImage(m_image.id);
        }
    }
    else if (selected_action == move_top_action)
    {
        moveToTop();
        emit imageNeedsSave();
    }
    else if (selected_action == move_up_action)
    {
        moveUp();
        emit imageNeedsSave();
    }
    else if (selected_action == move_down_action)
    {
        moveDown();
        emit imageNeedsSave();
    }
    else if (selected_action == move_bottom_action)
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

    // Special for not selectable item first need to call standard mousePressEvent then accept event
    //QGraphicsItem::mousePressEvent(event);

    // Somehow clicking on non selectable object does not clear previous selections.
    if (!(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }
    m_mouse_pressed = true;

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            m_offset = event->pos() - m_origin;
        }
    }
    if (m_selection_type == SelectionType::ByMouseRelease)
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

    if(m_select_new_origin)
    {
        m_image.xOrigin = event->pos().x() - m_bounding_rect.topLeft().x();
        m_image.yOrigin = event->pos().y() - m_bounding_rect.topLeft().y();

        m_origin = m_bounding_rect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);

        m_image.xPos = mapToScene(event->pos()).x();
        m_image.yPos = mapToScene(event->pos()).y();
    }
}

void ImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    prepareGeometryChange();

    if (flags() & QGraphicsItem::ItemIsMovable && event->buttons() & Qt::LeftButton)
    {
        m_image_was_moved = true;

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
    if(m_image_was_moved)
    {
        m_image_was_moved = false;
        emit imageNeedsSave();
    }

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }

    if (m_selection_type == SelectionType::ByMouseRelease)
    {
        emit imageSelected(m_image.id);
    }

    if(m_select_new_origin)
    {
        m_select_new_origin = false;
        setFlag(QGraphicsItem::ItemIsMovable, true);
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        m_resize_handles->show();
        emit imageNeedsSave();
    }

    m_mouse_pressed = false;
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
            if (m_select_new_origin)
            {
                m_select_new_origin = false;
                setFlag(QGraphicsItem::ItemIsMovable, true);
                SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
                m_resize_handles->show();
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

    m_bounding_rect.setTopLeft(rect.topLeft());

    updateImage();
    scene()->update();
}


void ImageItem::updateImageAndHandles(DraftImage image)
{
    m_image = image;
    m_origin = m_bounding_rect.topLeft() + QPointF(m_image.xOrigin, m_image.yOrigin);
    updateImage();
    m_resize_handles->setParentRect(m_bounding_rect);
    m_resize_handles->setLockAspectRatio(m_image.aspectLocked);
    m_resize_handles->setParentRotation(m_image.rotation);
    m_resize_handles->parentIsLocked(m_image.locked);
    m_resize_handles->setVisible(m_image.locked);
}

void ImageItem::deleteImageItem()
{
    moveToBottom(); //so that there is no gap in zValue
    scene()->removeItem(this);
    deleteLater();
}

void ImageItem::moveToBottom()
{
    qint32   min_z_value = m_image.order;
    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item != this && item->m_image.order < m_image.order)
        {
            min_z_value = qMin(min_z_value, item->m_image.order);
            item->m_image.order ++;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order = min_z_value;
    updateImage();
}


void ImageItem::moveToTop()
{
    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item != this && item->m_image.order > m_image.order)
        {
            item->m_image.order --;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order = max_z_value;
    updateImage();
}

void ImageItem::moveUp()
{
    if (m_image.order == max_z_value)
    {
        return;
    }

    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item->m_image.order == m_image.order + 1)
        {
            item->m_image.order --;
            item->updateImage();
            emit item->imageNeedsSave();
        }
    }
    m_image.order ++;
    updateImage();
}


void ImageItem::moveDown()
{
    if (m_image.order == max_z_value-m_doc->getBackgroundImageMap().values().size()+1)
    {
        return;
    }

    foreach (ImageItem *item, m_doc->getBackgroundImageMap().values())
    {
        if (item->m_image.order == m_image.order - 1)
        {
            item->m_image.order ++;
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
    QString x_pos;
    QString y_pos;

    if (m_image.units == Unit::Px)
    {
        width  = QString::number(m_image.width);
        height = QString::number(m_image.height);
        x_pos  = QString::number(m_image.xPos);
        y_pos  = QString::number(m_image.yPos);
    }
    else
    {
        width  = QString::number(qApp->fromPixel(m_image.width));
        height = QString::number(qApp->fromPixel(m_image.height));
        x_pos  = QString::number(qApp->fromPixel(m_image.xPos));
        y_pos  = QString::number(qApp->fromPixel(m_image.yPos));
    }

    QString message = QString(tr("<b>Image (%7)</b>: Size(%2%1, %3%1); Pos(%4%1, %5%1); Rot(%6°)%8"))
                          .arg(UnitsToStr(m_image.units))
                          .arg(width)
                          .arg(height)
                          .arg(x_pos)
                          .arg(y_pos)
                          .arg(m_image.rotation)
                          .arg(m_image.name)
                          .arg(!m_image.aspectLocked ? "" : tr(" - <b>Aspect ratio locked</b>"));

    emit setStatusMessage(message);
}

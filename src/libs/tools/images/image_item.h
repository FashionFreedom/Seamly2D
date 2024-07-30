/******************************************************************************
 **  @file   image_item.h
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

#ifndef IMAGE_ITEM_H
#define IMAGE_ITEM_H

#include <qcompilerdetection.h>

#include "../vmisc/def.h"
#include "../vwidgets/resize_handle.h"
#include "../vmisc/vabstractapplication.h"

#include <QMetaObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtCore/qglobal.h>
#include <QGraphicsItem>

class ResizeHandlesItem;

/**
 * @brief The ImageItem class pointer label.
 */
class ImageItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    explicit         ImageItem(QObject *parent, VAbstractPattern *doc, DraftImage image);
    virtual         ~ImageItem() = default;

    virtual int      type() const override {return Type;}
    enum             {Type = UserType + static_cast<int>(Tool::BackgroundImage)};

    virtual QRectF   boundingRect() const override;

    virtual void     paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget = nullptr) override;

    static constexpr qint32      maxImageZvalue = -100;

    void             moveToBottom();
    void             moveToTop();
    void             moveUp();
    void             moveDown();


    DraftImage       getImage();
    void             setImage(DraftImage image);
    void             setOrigin(qreal xOrigin, qreal yOrigin);
    void             updateImage();
    void             updateImageAndHandles(DraftImage image);

    void             setLock(bool checked);

    void             enableSelection(bool enable);
    void             enableHovering(bool enable);

    void             deleteImageItem();

signals:
    void             imageNeedsSave();
    void             showContextMenu(QGraphicsSceneContextMenuEvent *event);
    void             deleteImage(quint32 id);
    void             imageSelected(quint32 id);
    void             setStatusMessage(QString message);

protected:
    virtual void     hoverEnterEvent (QGraphicsSceneHoverEvent *event) override;
    virtual void     hoverMoveEvent (QGraphicsSceneHoverEvent *event) override;
    virtual void     hoverLeaveEvent (QGraphicsSceneHoverEvent *event) override;
    virtual void     contextMenuEvent (QGraphicsSceneContextMenuEvent *event) override;
    virtual void     mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void     mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void     mouseReleaseEvent (QGraphicsSceneMouseEvent *event) override;
    virtual void     keyReleaseEvent (QKeyEvent *event) override;

private:
    VAbstractPattern  *m_doc;
    QPointF            m_offset;
    QPointF            m_origin;
    QRectF             m_boundingRect;
    QRectF             m_handleRect;
    QRectF             m_actualRect;
    ResizeHandlesItem *m_resizeHandles;
    Position           m_resizePosition;
    QLineF             m_rotateLine;
    QPolygonF          m_angleHandle;
    qreal              m_angle;
    bool               m_mousePressed;
    bool               m_isHovered;
    SelectionType      m_selectionType;
    bool               m_transformationMode;
    DraftImage         m_image;
    QPixmap            m_pixmap;
    qreal              m_pixmapWidth;
    qreal              m_pixmapHeight;
    bool               m_selectable;
    qreal              m_minDimension;
    qreal              m_maxDimension;
    bool               m_selectNewOrigin;
    bool               m_imageWasMoved;

    void               initializeItem();
    void               updateFromHandles(QRectF rect);
    void               setPixmap(const QPixmap &pixmap);
    void               showImageStatusMessage();
};

#endif // IMAGE_ITEM_H

/************************************************************************
 **
 **  @file   vtextgraphicsitem.h
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

#ifndef VTEXTGRAPHICSITEM_H
#define VTEXTGRAPHICSITEM_H

#include <QFont>
#include <QGraphicsObject>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QtGlobal>

#include "vpieceitem.h"
#include "../vlayout/vtextmanager.h"

/**
 * @brief The VTextGraphicsItem class. This class implements text graphics item,
 * which can be dragged around, resized and rotated within the parent item. The text font
 * size will be automatically updated, so that the entire text will fit into the item.
 */
class VTextGraphicsItem : public VPieceItem
{
    Q_OBJECT
public:
    explicit VTextGraphicsItem(QGraphicsItem* pParent = nullptr);
    virtual ~VTextGraphicsItem() Q_DECL_EQ_DEFAULT;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) Q_DECL_OVERRIDE;
    virtual void updateItem() Q_DECL_OVERRIDE;

    virtual int  type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::TextGraphicsItem)};

    void setFont(const QFont &font);
    int  getFontSize() const;
    void setSize(qreal width, qreal height);
    bool isContained(QRectF rectBB, qreal rotation, qreal &xPos, qreal &yPos) const;
    void updateData(const QString &name, const VPieceLabelData &data);
    void updateData(VAbstractPattern *doc);
    int  getTextLines() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;

    void UpdateBox();
    void correctLabel();

signals:
    void itemResized(qreal width, int fontSize);
    void itemRotated(qreal angle);
    void textShrink();

private:
    Q_DISABLE_COPY(VTextGraphicsItem)
    QPointF      m_startPos;
    QPointF      m_start;
    QSizeF       m_startSize;
    double       m_rotation;
    double       m_angle;
    QRectF       m_rectResize;
    VTextManager m_textMananger;

    void allUserModifications(const QPointF &pos);
    void userRotateAndMove();
    void userMoveAndResize(const QPointF &pos);
};

#endif // VTEXTGRAPHICSITEM_H

//---------------------------------------------------------------------------------------------------------------------
//  @file   vtextgraphicsitem.h
//  @author Douglas S Caskey
//  @date   20 May, 2026
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//  @file   vtextgraphicsitem.h
//  @author Bojan Kverh
//  @date   16 Jun, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2016 Valentina project
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

//---------------------------------------------------------------------------------------------------------------------
// @brief The VTextGraphicsItem class. This class implements text graphics item,
// which can be dragged around, resized and rotated within the parent item. The text font
// size will be automatically updated, so that the entire text will fit into the item.
//---------------------------------------------------------------------------------------------------------------------
class VTextGraphicsItem : public VPieceItem
{
    Q_OBJECT
public:
    explicit VTextGraphicsItem(QGraphicsItem* pParent = nullptr);
    virtual ~VTextGraphicsItem() Q_DECL_EQ_DEFAULT;

    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual void updateItem() override;

    virtual int  type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::TextGraphicsItem)};

    void setFont(const QFont &font);
    int  getFontSize() const;
    void setSize(qreal width, qreal height);
    bool isContained(QRectF rectBB, qreal rotation, qreal &xPos, qreal &yPos) const;
    void updateData(const QString &name, const VPieceLabelData &data);
    void updateData(VAbstractPattern *doc);
    int  getTextLines() const;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

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

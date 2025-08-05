//---------------------------------------------------------------------------------------------------------------------
//  @file   foldline.h
//  @author Evans PERRET
//  @date   01 Aug, 2025
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

#ifndef VFOLDLINEITEM_H
#define VFOLDLINEITEM_H

#include "vpieceitem.h"
#include "../vmisc/def.h"
#include "../vpatterndb/floatItemData/floatitemdef.h"

class VFoldlineItem : public VPieceItem
{
    Q_OBJECT
public:
    explicit             VFoldlineItem(QGraphicsItem* parent = nullptr);
    virtual             ~VFoldlineItem() Q_DECL_EQ_DEFAULT;

    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    virtual void         paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) Q_DECL_OVERRIDE;
    void                 updateGeometry(const QPointF& pos, qreal rotation, qreal length, ArrowType type);


    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                 {Type = UserType + static_cast<int>(Vis::GrainlineItem)};

    bool                 isContained(const QPointF &pt, qreal dRot, qreal &dX, qreal &dY) const;

signals:
    void                 itemResized(qreal dLength);
    void                 itemRotated(qreal dRot, const QPointF& ptNewPos);

protected:
    virtual void         mousePressEvent(QGraphicsSceneMouseEvent* pME) Q_DECL_OVERRIDE;
    virtual void         mouseMoveEvent(QGraphicsSceneMouseEvent* pME) Q_DECL_OVERRIDE;
    virtual void         mouseReleaseEvent(QGraphicsSceneMouseEvent* pME) Q_DECL_OVERRIDE;
    virtual void         hoverEnterEvent(QGraphicsSceneHoverEvent* pME) Q_DECL_OVERRIDE;
    virtual void         hoverLeaveEvent(QGraphicsSceneHoverEvent* pME) Q_DECL_OVERRIDE;
    virtual void         updateItem() Q_DECL_OVERRIDE;
    void                 updateRectangle();

    virtual double       GetAngle(const QPointF &pt) const Q_DECL_OVERRIDE;

    QPointF              rotate(const QPointF& pt, const QPointF& ptCenter, qreal dAng) const;
    QPointF              getInsideCorner(int i, qreal dDist) const;

private:
    Q_DISABLE_COPY(VFoldlineItem)
    qreal                m_rotation;
    qreal                m_rotationStart;
    qreal                m_length;
    qreal                m_width;
    QPolygonF            m_boundingPoly;
    QPointF              m_startPos;
    QPointF              m_movePos;
    QPolygonF            m_resizeHandle;
    qreal                m_startLength;
    QPointF              m_startPoint;
    QPointF              m_finishPoint;
    QPointF              m_centerPoint;
    qreal                m_angle;
    ArrowType            m_arrowType;
    qreal                m_penWidth;

    struct
    {
        QLineF           mainline;
        QLineF           branch1;
        QLineF           branch2;
        QPolygonF        arrow1;
        QPolygonF        arrow2;
    } m_foldline;

    void                 updateFoldline();

    QPainterPath         mainShape() const;

    void                 allUserModifications(const QPointF &pos);
    void                 userRotateAndMove();
    void                 userMoveAndResize(const QPointF &pos);

    void                 updateResizeHandle();
};

#endif // VFOLDLINEITEM_H

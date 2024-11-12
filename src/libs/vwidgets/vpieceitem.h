//---------------------------------------------------------------------------------------------------------------------
//  @file   vpieceitem.h
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
//  @file   vpieceitem.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   18 1, 2017
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

#ifndef VPIECEITEM_H
#define VPIECEITEM_H

#include <QtCore/qglobal.h>
#include <QGraphicsObject>

#include "../vmisc/def.h"

class VPieceItem : public QGraphicsObject
{
    Q_OBJECT
public:
    enum MoveType
    {
        NotMovable  = 0x0, // 0000
        IsRotatable = 0x1, // 0001
        IsResizable = 0x2, // 0010
        IsMovable   = 0x4, // 0100
        Error       = 0x8,  // 1000
        AllModifications = IsRotatable | IsResizable | IsMovable // 0111
    };
    Q_DECLARE_FLAGS(MoveTypes, MoveType)

    explicit              VPieceItem(QGraphicsItem* pParent = nullptr);
    virtual              ~VPieceItem();

    virtual QRectF        boundingRect() const Q_DECL_OVERRIDE;

    virtual void          updateItem() =0;

    void                  reset();
    bool                  isIdle() const;

    VPieceItem::MoveTypes getMoveType() const;
    void                  setMoveType(const VPieceItem::MoveTypes &moveType);

    virtual int           type() const Q_DECL_OVERRIDE {return Type;}
    enum                  {Type = UserType + static_cast<int>(Vis::PieceItem)};

signals:
    void                  itemMoved(const QPointF &ptPos);

protected:
    enum class Mode       {Normal, Move, Resize, Rotate};
    QRectF                m_boundingRect;
    Mode                  m_mode;
    bool                  m_isReleased;
    QPointF               m_rotationCenter;
    VPieceItem::MoveTypes m_moveType;
    qreal                 m_inactiveZ;

    virtual double        GetAngle(const QPointF &pt) const;

private:
                          Q_DISABLE_COPY(VPieceItem)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(VPieceItem::MoveTypes)

#endif // VPIECEITEM_H

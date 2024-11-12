//---------------------------------------------------------------------------------------------------------------------
//  @file   vpieceitem.cpp
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
//  @file   vpieceitem.cpp
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

#include "vpieceitem.h"
#include "../vmisc/vmath.h"

#include <QGraphicsScene>

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::VPieceItem(QGraphicsItem *pParent)
    : QGraphicsObject(pParent)
    , m_boundingRect()
    , m_mode(Mode::Normal)
    , m_isReleased(false)
    , m_rotationCenter()
    , m_moveType(AllModifications)
    , m_inactiveZ(1)
{
    m_boundingRect.setTopLeft(QPointF(0, 0));
    setAcceptHoverEvents(true);
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::~VPieceItem()
{
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief boundingRect returns the item bounding box
/// @return item bounding box
//---------------------------------------------------------------------------------------------------------------------
QRectF VPieceItem::boundingRect() const
{
    return m_boundingRect;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief reset resets the item, putting the mode and z coordinate to normal and redraws it
//---------------------------------------------------------------------------------------------------------------------
void VPieceItem::reset()
{
    if (QGraphicsScene *toolScene = scene())
    {
        toolScene->clearSelection();
    }
    m_mode = Mode::Normal;
    m_isReleased = false;
    updateItem();
    setZValue(m_inactiveZ);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief isIdle returns the idle flag.
/// @return true, if item mode is normal and false otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VPieceItem::isIdle() const
{
    return m_mode == Mode::Normal;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetAngle calculates the angle between the line, which goes from rotation center to point and x axis
/// @param point point of interest
/// @return the angle between line from rotation center and point of interest and x axis
//---------------------------------------------------------------------------------------------------------------------
double VPieceItem::GetAngle(const QPointF &point) const
{
    const double dX = point.x() - m_rotationCenter.x();
    const double dY = point.y() - m_rotationCenter.y();

    if (fabs(dX) < 1 && fabs(dY) < 1)
    {
        return 0;
    }
    else
    {
        return qAtan2(dY, dX);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::MoveTypes VPieceItem::getMoveType() const
{
    return m_moveType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceItem::setMoveType(const VPieceItem::MoveTypes &moveType)
{
    m_moveType = moveType;
    setAcceptHoverEvents(m_moveType != NotMovable);
}

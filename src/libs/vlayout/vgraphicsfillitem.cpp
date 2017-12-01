/************************************************************************
 **
 **  @file   vgraphicsfillitem.cpp
 **  @author Bojan Kverh
 **  @date   October 16, 2016
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

#include "vgraphicsfillitem.h"

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFillItem::VGraphicsFillItem(QGraphicsItem *parent)
    :QGraphicsPathItem(parent)
{}

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFillItem::~VGraphicsFillItem()
{}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsFillItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->save();
    painter->setBrush(painter->pen().color());
    painter->drawPath(path());
    painter->restore();
}

//---------------------------------------------------------------------------------------------------------------------


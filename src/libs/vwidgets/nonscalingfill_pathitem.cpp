/***************************************************************************
 **  @file   nonscalingfill_pathitem.cpp
 **  @author Douglas S Caskey
 **  @date   Jan 29, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vnobrushscalepathitem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 1, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "nonscalingfill_pathitem.h"

#include <QBrush>
#include <QTransform>
#include <QPainter>

//---------------------------------------------------------------------------------------------------------------------
NonScalingFillPathItem::NonScalingFillPathItem(QGraphicsItem *parent)
    : QGraphicsPathItem(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void NonScalingFillPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    /*
     * How to avoid transformation of QBrush fill
     * http://www.qtforum.org/article/23942/how-to-avoid-transformation-of-qbrush-texture.html
     * ?s=b4ba78dd6758da78fe395d8f6bb7512511a0833e#post84983
     *
     * Non Scaling QBrush fill style for qgraphicspathitem
     * http://www.qtcentre.org/archive/index.php/t-13950.html
     *
     * You'll have to scale the brush down. The QStyleOptionGraphicsItem option passed in paint() will give you the
     * transform being used, and you can set a transform on a QBrush. Put the two together and you can scale the brush
     * inversely of the item.
     */
    QBrush brush = this->brush();
    brush.setTransform(painter->combinedTransform().inverted());
    this->setBrush(brush);
    QGraphicsPathItem::paint(painter, option, widget);
}

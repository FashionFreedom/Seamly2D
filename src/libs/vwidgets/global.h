//  @file   global.h
//  @author Douglas S Caskey
//  @date   22 Jun, 2024
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

/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#ifndef GLOBAL_H
#define GLOBAL_H

#include <QtGlobal>

extern const qreal defPointRadiusPixel;
extern const qreal widthMainLine;
extern const qreal widthHairLine;

class QGraphicsScene;
class QGraphicsItem;
class QGraphicsEllipseItem;
class QGraphicsRectItem;
class QGraphicsLineItem;
class QColor;
class QRectF;
class QPainterPath;
class QPen;

qreal        sceneScale(QGraphicsScene *scene);
QColor       correctColor(const QGraphicsItem *item, const QColor &color);
QRectF       pointRect(qreal radius);
qreal        scaledRadius(qreal scale);
void         scaleCircleSize(QGraphicsEllipseItem *item, qreal scale);
void         scaleRectSize(QGraphicsRectItem *item, qreal scale);
qreal        scaleWidth(qreal width, qreal scale);
QPainterPath itemShapeFromPath(const QPainterPath &path, const QPen &pen);

#endif // GLOBAL_H

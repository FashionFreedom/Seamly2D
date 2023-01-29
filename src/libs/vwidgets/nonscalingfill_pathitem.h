/***************************************************************************
 **  @file   nonscalingfill_pathitem.h
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
 **  @file   vnobrushscalepathitem.h
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

#ifndef  NONSCALINGFILL_PATHITEM_H
#define  NONSCALINGFILL_PATHITEM_H

#include <qcompilerdetection.h>
#include <QGraphicsPathItem>
#include <QtGlobal>

#include "../vmisc/def.h"

class NonScalingFillPathItem : public QGraphicsPathItem
{
public:
    explicit     NonScalingFillPathItem(QGraphicsItem *parent = nullptr);

protected:
    virtual void paint(QPainter * painter, const QStyleOptionGraphicsItem * option,
                       QWidget * widget = nullptr) Q_DECL_OVERRIDE;

    virtual int  type() const Q_DECL_OVERRIDE {return Type;}
    enum         {Type = UserType + static_cast<int>(Vis::NoBrush)};

private:
    Q_DISABLE_COPY(NonScalingFillPathItem)
};

#endif // NONSCALINGFILL_PATHITEM_H

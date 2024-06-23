//  @file   vcurvepathitem.h
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
 **  @date   16 5, 2017
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

#ifndef VCURVEPATHITEM_H
#define VCURVEPATHITEM_H

#include <QGraphicsPathItem>
#include <QtGlobal>

#include "../vmisc/def.h"

class VCurvePathItem : public QGraphicsPathItem
{
public:
    explicit             VCurvePathItem(QGraphicsItem *parent = nullptr);
    virtual             ~VCurvePathItem() = default;

    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

    virtual void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                 {Type = UserType + static_cast<int>(Vis::CurvePathItem)};

    void                 SetDirectionArrows(const QVector<QPair<QLineF, QLineF>> &arrows);
    void                 SetPoints(const QVector<QPointF> &points);

protected:
    virtual void         ScalePenWidth();

private:
    Q_DISABLE_COPY(VCurvePathItem)

    QVector<QPair<QLineF, QLineF>> m_directionArrows;
    QVector<QPointF>               m_points;
};

#endif // VCURVEPATHITEM_H

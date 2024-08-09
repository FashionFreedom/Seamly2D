//  @file   scalesceneitems.h
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
 **  @date   21 6, 2017
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

#ifndef SCALESCENEITEMS_H
#define SCALESCENEITEMS_H

#include <QGraphicsLineItem>

#include "../vmisc/def.h"
#include "vcurvepathitem.h"

class VCurvePathItem;

class VScaledLine : public QGraphicsLineItem
{
public:
    explicit        VScaledLine(QGraphicsItem * parent = nullptr);
                    VScaledLine(const QLineF &line, QGraphicsItem * parent = nullptr);
    virtual        ~VScaledLine() = default;

    virtual int     type() const Q_DECL_OVERRIDE {return Type;}
    enum            {Type = UserType + static_cast<int>(Vis::ScaledLine)};

    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    qreal           getDefaultWidth() const;
    void            setDefaultWidth(const qreal &value);

private:
    Q_DISABLE_COPY(VScaledLine)

    qreal           m_defaultWidth;
};

class ArrowedLineItem : public QGraphicsLineItem
{
public:
    explicit        ArrowedLineItem(QGraphicsItem * parent = nullptr);
                    ArrowedLineItem(const QLineF &line, QGraphicsItem * parent = nullptr);
    virtual        ~ArrowedLineItem() = default;

    virtual int     type() const Q_DECL_OVERRIDE {return Type;}
    enum            {Type = UserType + static_cast<int>(Vis::ArrowedLineItem)};

    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget = nullptr) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(ArrowedLineItem)
    void            drawArrow(const QLineF &axis, QPainterPath &path, const qreal &arrow_size);
    VCurvePathItem *m_arrows;
};

class VScaledEllipse : public QGraphicsEllipseItem
{
public:
    explicit        VScaledEllipse(QGraphicsItem * parent = nullptr);
    virtual        ~VScaledEllipse() = default;

    virtual int     type() const Q_DECL_OVERRIDE {return Type;}
    enum            {Type = UserType + static_cast<int>(Vis::ScaledEllipse)};

    virtual void    paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                          QWidget *widget = nullptr) Q_DECL_OVERRIDE;
private:
    Q_DISABLE_COPY(VScaledEllipse)
};

#endif // SCALESCENEITEMS_H

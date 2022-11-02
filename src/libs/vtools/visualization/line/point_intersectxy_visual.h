/**************************************************************************
 **
 **  @file   point_intersectxy_visual.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date  13 8, 2014
 **
 **  @author Douglas S Caskey
 **  @date   7.21.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#ifndef POINT_INTERSECTXY_VISUAL_H
#define POINT_INTERSECTXY_VISUAL_H

#include "visline.h"
#include "../vmisc/def.h"

#include <qcompilerdetection.h>
#include <QColor>
#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

class PointIntersectXYVisual : public VisLine
{
    Q_OBJECT

public:
    explicit        PointIntersectXYVisual(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual        ~PointIntersectXYVisual() = default;

    virtual void    RefreshGeometry() Q_DECL_OVERRIDE;

    void            setPoint2Id(const quint32 &value);
    void            setPoint1Id(const quint32 &value);
    virtual int     type() const Q_DECL_OVERRIDE {return Type;}
    enum            {Type = UserType + static_cast<int>(Vis::ToolPointOfIntersection)};

private:
                    Q_DISABLE_COPY(PointIntersectXYVisual)
    quint32         point2Id;
    VScaledEllipse *point;
    VScaledEllipse *axisP1;
    VScaledEllipse *axisP2;
    VScaledLine    *axis2;

    void            showIntersection(const QLineF &axis1, const QLineF &axis2, const QColor &color);
};

#endif // POINT_INTERSECTXY_VISUAL_H

/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   vpointf_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
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

#ifndef VPOINTF_P_H
#define VPOINTF_P_H

#include <QSharedData>
#include "vgeometrydef.h"
#include "../vmisc/diagnostic.h"
#include <QPointF>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPointFData : public QSharedData
{
public:

    VPointFData()
        : _mx(0)
        , _my(0)
        , _x(0)
        , _y(0)
        , m_showPointName(true)
    {}

    VPointFData(const VPointFData &point)
        : QSharedData(point)
        , _mx(point._mx)
        , _my(point._my)
        , _x(point._x)
        , _y(point._y)
        , m_showPointName(point.m_showPointName)
    {}

    explicit VPointFData(const QPointF &point)
        : _mx(0)
        , _my(0)
        , _x(point.x())
        , _y(point.y())
        , m_showPointName(true)
    {}

    VPointFData(qreal x, qreal y, qreal mx, qreal my)
        : _mx(mx)
        , _my(my)
        , _x(x)
        , _y(y)
        , m_showPointName(true)
    {}

    VPointFData(const QPointF &point, qreal mx, qreal my)
        : _mx(mx)
        , _my(my)
        , _x(point.x())
        , _y(point.y())
        , m_showPointName(true)
    {}

    virtual ~VPointFData();

    /** @brief _mx offset name respect to x */
    qreal   _mx;

    /** @brief _my offset name respect to y */
    qreal   _my;

    /** @brief _x x coordinate */
    qreal   _x;

    /** @brief _y y coordinate */
    qreal   _y;

    /** @brief m_showPointName hide or show name for this point */
    bool    m_showPointName;

private:
    VPointFData &operator=(const VPointFData &) Q_DECL_EQ_DELETE;
};

VPointFData::~VPointFData()
{}

QT_WARNING_POP

#endif // VPOINTF_P_H

/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 - 2022 Seamly2D project
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

#ifndef VGRAINLINEDATA_P_H
#define VGRAINLINEDATA_P_H

#include <QPointF>
#include <QSharedData>

#include "../vmisc/diagnostic.h"
#include "floatitemdef.h"
#include "../ifc/ifcdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VGrainlineDataPrivate : public QSharedData
{
public:
    VGrainlineDataPrivate()
        : m_qsLength()
        , m_dRotation()
        , m_eArrowType(ArrowType::atBoth)
        , m_centerAnchorPoint(NULL_ID)
        , m_topAnchorPoint(NULL_ID)
        , m_bottomAnchorPoint(NULL_ID)
    {}

    VGrainlineDataPrivate(const VGrainlineDataPrivate &data)
        : QSharedData(data)
        , m_qsLength(data.m_qsLength)
        , m_dRotation(data.m_dRotation)
        , m_eArrowType(data.m_eArrowType)
        , m_centerAnchorPoint(data.m_centerAnchorPoint)
        , m_topAnchorPoint(data.m_topAnchorPoint)
        , m_bottomAnchorPoint(data.m_bottomAnchorPoint)
    {}

    ~VGrainlineDataPrivate() Q_DECL_EQ_DEFAULT;

    QString   m_qsLength;          /** @brief m_dLength formula to calculate the length of grainline */
    QString   m_dRotation;         /** @brief m_dRotation formula to calculate the rotation of grainline in [degrees] */
    ArrowType m_eArrowType;        /** @brief m_eArrowType type of arrow on the grainline */
    quint32   m_centerAnchorPoint; /** @brief m_centerAnchorPoint center anchor point id */
    quint32   m_topAnchorPoint;    /** @brief m_topAnchorPoint top anchor point id */
    quint32   m_bottomAnchorPoint; /** @brief m_bottomAnchorPoint bottom anchor point id */

private:
    VGrainlineDataPrivate &operator=(const VGrainlineDataPrivate &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VGRAINLINEDATA_P_H

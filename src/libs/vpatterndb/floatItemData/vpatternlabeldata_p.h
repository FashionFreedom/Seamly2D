/************************************************************************
 **
 **  @file   vpatternlabeldata_p.h
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

#ifndef VPATTERNLABELDATA_P_H
#define VPATTERNLABELDATA_P_H

#include <QPointF>
#include <QSharedData>

#include "../vmisc/diagnostic.h"
#include "../ifc/ifcdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPatternLabelDataPrivate : public QSharedData
{
public:
    VPatternLabelDataPrivate()
        : m_pieceLabelWidth()
        , m_pieceLabelHeight()
        , m_pieceLabelAngle()
        , m_iFontSize(0)
        , m_centerAnchorPoint(NULL_ID)
        , m_topLeftAnchorPoint(NULL_ID)
        , m_bottomRightAnchorPoint(NULL_ID)
    {}

    VPatternLabelDataPrivate(const VPatternLabelDataPrivate &data)
        : QSharedData(data)
        , m_pieceLabelWidth(data.m_pieceLabelWidth)
        , m_pieceLabelHeight(data.m_pieceLabelHeight)
        , m_pieceLabelAngle(data.m_pieceLabelAngle)
        , m_iFontSize(data.m_iFontSize)
        , m_centerAnchorPoint(data.m_centerAnchorPoint)
        , m_topLeftAnchorPoint(data.m_topLeftAnchorPoint)
        , m_bottomRightAnchorPoint(data.m_bottomRightAnchorPoint)
    {}

    ~VPatternLabelDataPrivate() Q_DECL_EQ_DEFAULT;

    QString m_pieceLabelWidth;            /** @brief m_pieceLabelWidth formula to calculate the width of label */
    QString m_pieceLabelHeight;           /** @brief m_pieceLabelHeight formula to calculate the height of label */
    QString m_pieceLabelAngle;            /** @brief m_pieceLabelAngle formula to calculate the rotation angle of label */
    int     m_iFontSize;              /** @brief m_iFontSize label text base font size */
    quint32 m_centerAnchorPoint;      /** @brief m_centerAnchorPoint center pin id */
    quint32 m_topLeftAnchorPoint;     /** @brief m_topLeftAnchorPoint top left corner pin id */
    quint32 m_bottomRightAnchorPoint; /** @brief m_bottomRightAnchorPoint bottom right corner pin id */

private:
    VPatternLabelDataPrivate &operator=(const VPatternLabelDataPrivate &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VPATTERNLABELDATA_P_H

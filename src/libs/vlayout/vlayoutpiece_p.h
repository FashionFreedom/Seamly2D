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
 **  @file   vlayoutdetail_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 1, 2015
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

#ifndef VLAYOUTDETAIL_P_H
#define VLAYOUTDETAIL_P_H

#include <QSharedData>
#include <QPointF>
#include <QVector>
#include <QTransform>

#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vmisc/diagnostic.h"
#include "vlayoutpiecepath.h"

#include "vtextmanager.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPieceData : public QSharedData
{
public:
    VLayoutPieceData()
        : contour(),
          seamAllowance(),
          layoutAllowance(),
          notches(),
          m_internalPaths(),
          transform(),
          layoutWidth(0),
          mirror(false),
          detailLabel(),
          patternInfo(),
          grainlinePoints(),
          m_tmDetail(),
          m_tmPattern()
    {}

    VLayoutPieceData(const VLayoutPieceData &detail)
        : QSharedData(detail),
          contour(detail.contour),
          seamAllowance(detail.seamAllowance),
          layoutAllowance(detail.layoutAllowance),
          notches(detail.notches),
          m_internalPaths(detail.m_internalPaths),
          transform(detail.transform),
          layoutWidth(detail.layoutWidth),
          mirror(detail.mirror),
          detailLabel(detail.detailLabel),
          patternInfo(detail.patternInfo),
          grainlinePoints(detail.grainlinePoints),
          m_tmDetail(detail.m_tmDetail),
          m_tmPattern(detail.m_tmPattern)
    {}

    ~VLayoutPieceData() {}

    QVector<QPointF>           contour;            //! @brief contour list of contour points.
    QVector<QPointF>           seamAllowance;      //! @brief seamAllowance list of seam allowance points.
    QVector<QPointF>           layoutAllowance;    //! @brief layoutAllowance list of layout allowance points.
    QVector<QLineF>            notches;            //! @brief notches list of notches.
    QVector<VLayoutPiecePath>  m_internalPaths;    //! @brief m_internalPaths list of internal paths.
    QTransform                 transform;          //! @brief transform transformation transform
    qreal                      layoutWidth;        //! @brief layoutWidth value layout allowance width in pixels.
    bool                       mirror;
    QVector<QPointF>           detailLabel;        //! @brief detailLabel detail label rectangle
    QVector<QPointF>           patternInfo;        //! @brief patternInfo pattern info rectangle
    QVector<QPointF>           grainlinePoints;    //! @brief grainlineInfo line
    VTextManager               m_tmDetail;         //! @brief m_tmDetail text manager for laying out detail info
    VTextManager               m_tmPattern;        //! @brief m_tmPattern text manager for laying out pattern info */

private:
    VLayoutPieceData &operator=(const VLayoutPieceData &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VLAYOUTDETAIL_P_H

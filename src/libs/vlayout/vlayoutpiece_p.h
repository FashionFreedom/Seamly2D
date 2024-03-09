/***************************************************************************
 **  @file   vlayoutdetail_p.h
 **  @author Douglas S Caskey
 **  @date   Dec 27, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
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
 **  @file   vlayoutdetail_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
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
          m_cutoutPaths(),
          transform(),
          layoutWidth(0),
          mirror(false),
          pieceLabel(),
          patternInfo(),
          grainlinePoints(),
          m_tmPiece(),
          m_tmPattern()
    {}

    VLayoutPieceData(const VLayoutPieceData &piece)
        : QSharedData(piece),
          contour(piece.contour),
          seamAllowance(piece.seamAllowance),
          layoutAllowance(piece.layoutAllowance),
          notches(piece.notches),
          m_internalPaths(piece.m_internalPaths),
          m_cutoutPaths(piece.m_cutoutPaths),
          transform(piece.transform),
          layoutWidth(piece.layoutWidth),
          mirror(piece.mirror),
          pieceLabel(piece.pieceLabel),
          patternInfo(piece.patternInfo),
          grainlinePoints(piece.grainlinePoints),
          m_tmPiece(piece.m_tmPiece),
          m_tmPattern(piece.m_tmPattern)
    {}

    ~VLayoutPieceData() {}

    QVector<QPointF>           contour;            //! @brief contour list of contour points.
    QVector<QPointF>           seamAllowance;      //! @brief seamAllowance list of seam allowance points.
    QVector<QPointF>           layoutAllowance;    //! @brief layoutAllowance list of layout allowance points.
    QVector<QLineF>            notches;            //! @brief notches list of notches.
    QVector<VLayoutPiecePath>  m_internalPaths;    //! @brief m_internalPaths list of internal paths.
    QVector<VLayoutPiecePath>  m_cutoutPaths;      //! @brief m_cutoutPaths list of internal cutout paths.
    QTransform                 transform;          //! @brief transform transformation transform
    qreal                      layoutWidth;        //! @brief layoutWidth value layout allowance width in pixels.
    bool                       mirror;
    QVector<QPointF>           pieceLabel;         //! @brief pieceLabel piece label rectangle
    QVector<QPointF>           patternInfo;        //! @brief patternInfo pattern info rectangle
    QVector<QPointF>           grainlinePoints;    //! @brief grainlineInfo line
    VTextManager               m_tmPiece;          //! @brief m_tmPiece text manager for laying out piece info
    VTextManager               m_tmPattern;        //! @brief m_tmPattern text manager for laying out pattern info */

private:
    VLayoutPieceData &operator=(const VLayoutPieceData &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VLAYOUTDETAIL_P_H

//---------------------------------------------------------------------------------------------------------------------
//   @file   vposition_p.h
//   @author Douglas S Caskey
//   @date   Dec 27, 2022
//
//   @copyright
//   Copyright (C) 2017 - 2022 Seamly, LLC
//   https://github.com/fashionfreedom/seamly2d
//
//   @brief
//   Seamly2D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Seamly2D is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//
//   @file   vposition_p.h
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   8 1, 2015
//
//   @brief
//   @copyright
//   This source code is part of the Valentina project, a pattern making
//   program, whose allow create and modeling patterns of clothing.
//   Copyright (C) 2013-2015 Valentina project
//   <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//   Valentina is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Valentina is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef VLAYOUTPAPER_P_H
#define VLAYOUTPAPER_P_H

#include <QSharedData>
#include <QVector>
#include <QPointF>

#include "vlayoutpiece.h"
#include "vcontour.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPaperData : public QSharedData
{
public:
    VLayoutPaperData()
        : pieces(QVector<VLayoutPiece>())
        , globalContour(VContour())
        , paperIndex(0)
        , layoutWidth(0)
        , globalRotate(true)
        , localRotate(true)
        , globalRotationIncrease(180)
        , localRotationIncrease(180)
        , saveLength(false)
    {}

    VLayoutPaperData(int height, int width)
        : pieces(QVector<VLayoutPiece>())
        , globalContour(VContour(height, width))
        , paperIndex(0)
        , layoutWidth(0)
        , globalRotate(true)
        , localRotate(true)
        , globalRotationIncrease(180)
        , localRotationIncrease(180)
        , saveLength(false)
    {}

    VLayoutPaperData(const VLayoutPaperData &paper)
        : QSharedData(paper)
        , pieces(paper.pieces)
        , globalContour(paper.globalContour)
        , paperIndex(paper.paperIndex)
        , layoutWidth(paper.layoutWidth)
        , globalRotate(paper.globalRotate)
        , localRotate(paper.localRotate)
        , globalRotationIncrease(paper.globalRotationIncrease)
        , localRotationIncrease(paper.localRotationIncrease)
        , saveLength(paper.saveLength)
    {}

    ~VLayoutPaperData() {}

    QVector<VLayoutPiece> pieces;        /// @brief pieces list of arranged pieces.
    VContour              globalContour; /// @brief globalContour list of global points contour.
    quint32               paperIndex;
    qreal                 layoutWidth;
    bool                  globalRotate;
    bool                  localRotate;
    int                   globalRotationIncrease;
    int                   localRotationIncrease;
    bool                  saveLength;

private:
    VLayoutPaperData& operator=(const VLayoutPaperData&) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VLAYOUTPAPER_P_H

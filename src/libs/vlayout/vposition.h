//---------------------------------------------------------------------------------------------------------------------
//   @file   vposition.h
//   @author Douglas S Caskey
//   @date   Dec 11, 2022
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
//   @file   vposition.h
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   20 1, 2015
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

#ifndef VPOSITION_H
#define VPOSITION_H

#include <qcompilerdetection.h>
#include <QRunnable>
#include <QVector>
#include <QtGlobal>
#include <atomic>

#include "vbestsquare.h"
#include "vcontour.h"
#include "vlayoutdef.h"
#include "vlayoutpiece.h"

class VPosition : public QRunnable
{
public:
                 VPosition(const VContour &sheet, int sheetEdgeNum, const VLayoutPiece &piece, int pieceEdgeNum,
                           std::atomic_bool *stop, bool rotate, int rotationIncrement, bool saveLength);
    virtual     ~VPosition() Q_DECL_OVERRIDE{}
    virtual void run() Q_DECL_OVERRIDE;

    VBestSquare  getBestResult() const;

private:
    Q_DISABLE_COPY(VPosition)
    VBestSquare           m_bestResult;
    const VContour        m_sheet;
    int                   m_sheetEdgeNum;
    const VLayoutPiece    m_piece;
    int                   m_pieceEdgeNum;
    std::atomic_bool     *m_stop;
    bool                  m_rotate;
    int                   m_rotationIncrement;
    qreal                 m_angleBetween; /// keep angle between global edge and piece edge. Need for optimizing rotation.

    enum class IntersectionType : char
    {
        NoIntersection = 0,
        Intersection   = 1,
    };

    void                saveCandidate(VBestSquare &bestResult, const VLayoutPiece &piece,
                                      int pieceEdgeNum, int sheetEdgeNum,  BestFrom type);

    bool                edgesIntersect(VLayoutPiece &piece, int &pieceEdgeNum, int sheetEdgeNum);
    bool                rotatedEdgeIntersects(VLayoutPiece &piece, int pieceEdgeNum, int sheetEdgeNum, int angle) const;

    IntersectionType    intersectionType(const VLayoutPiece &piece) const;
    bool                sheetContains(const QRectF &rect) const;

    void                combineEdges(VLayoutPiece &piece, const int &pieceEdgeNum, const QLineF &sheetEdge);
    void                rotateEdges(VLayoutPiece &piece, int pieceEdgeNum, const QLineF &sheetEdge, int angle) const;

    void                rotate(int increment);
};

#endif // VPOSITION_H

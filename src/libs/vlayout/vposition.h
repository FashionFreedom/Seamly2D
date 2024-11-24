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
                 VPosition(const VContour &gContour, int j, const VLayoutPiece &piece, int i,
                           std::atomic_bool *stop, bool rotate, int rotationIncrease, bool saveLength);
    virtual     ~VPosition() Q_DECL_OVERRIDE{}
    virtual void run() Q_DECL_OVERRIDE;

    quint32      getPaperIndex() const;
    void         setPaperIndex(const quint32 &value);

    quint32      getFrame() const;
    void         setFrame(const quint32 &value);

    quint32      getPieceCount() const;
    void         setPieceCount(const quint32 &value);

    void         setPieces(const QVector<VLayoutPiece> &pieces);

    VBestSquare  getBestResult() const;

private:
    Q_DISABLE_COPY(VPosition)
    VBestSquare           bestResult;
    const VContour        gContour;
    const VLayoutPiece    piece;
    int                   i;
    int                   j;
    quint32               paperIndex;
    quint32               frame;
    quint32               piecesCount;
    QVector<VLayoutPiece> pieces;
    std::atomic_bool     *stop;
    bool                  rotate;
    int                   rotationIncrease;

    /// @brief angle_between keep angle between global edge and piece edge. Need for optimization rotation.
    qreal                 angle_between;

    enum class CrossingType : char
    {
        NoIntersection = 0,
        Intersection = 1,
        EdgeError = 2
    };

    enum class InsideType : char
    {
        Outside = 0,
        Inside = 1,
        EdgeError = 2
    };

    void                SaveCandidate(VBestSquare &bestResult, const VLayoutPiece &piece, int globalI, int detJ, BestFrom type);

    bool                CheckCombineEdges(VLayoutPiece &piece, int j, int &dEdge);
    bool                CheckRotationEdges(VLayoutPiece &piece, int j, int dEdge, int angle) const;

    CrossingType        Crossing(const VLayoutPiece &piece) const;
    bool                SheetContains(const QRectF &rect) const;

    void                CombineEdges(VLayoutPiece &piece, const QLineF &globalEdge, const int &dEdge);
    void                RotateEdges(VLayoutPiece &piece, const QLineF &globalEdge, int dEdge, int angle) const;

    static QPainterPath ShowDirection(const QLineF &edge);
    static QPainterPath DrawContour(const QVector<QPointF> &points);
    static QPainterPath drawPieces(const QVector<VLayoutPiece> &pieces);

    void                Rotate(int increase);
};

#endif // VPOSITION_H

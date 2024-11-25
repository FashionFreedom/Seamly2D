//---------------------------------------------------------------------------------------------------------------------
//  @file   vposition.cpp
//  @author Douglas S Caskey
//  @date   Dec 11, 2022
//
//  @copyright
//  Copyright (C) 2017 - 2022 Seamly, LLC
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
//  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//
//  @file   vposition.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   20 1, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "vposition.h"

#include <QLineF>
#include <QPainterPath>
#include <QPointF>
#include <QPolygonF>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QStaticStringData>
#include <QString>
#include <QStringData>
#include <QStringDataPtr>
#include <Qt>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"

//---------------------------------------------------------------------------------------------------------------------
/// @brief VPosition constructor
///
/// This consrtucts an instance of VPosition.
/// @param sheet
/// @param sheetEdgeNum
/// @param piece
/// @param pieceEdgeNum
/// @param stop
/// @param rotate
/// @param rotationIncrement
/// @param saveLength
//---------------------------------------------------------------------------------------------------------------------
VPosition::VPosition(const VContour &sheet, int sheetEdgeNum, const VLayoutPiece &piece, int pieceEdgeNum,
                     std::atomic_bool *stop, bool rotate, int rotationIncrement, bool saveLength)
    : QRunnable()
    , m_bestResult(VBestSquare(sheet.GetSize(), saveLength))
    , m_sheet(sheet)
    , m_sheetEdgeNum(sheetEdgeNum)
    , m_piece(piece)
    , m_pieceEdgeNum(pieceEdgeNum)
    , m_stop(stop)
    , m_rotate(rotate)
    , m_rotationIncrement(rotationIncrement)
    , m_angleBetween(0)
{
    if ((m_rotationIncrement >= 1 && m_rotationIncrement <= 180 && 360 % m_rotationIncrement == 0) == false)
    {
        this->m_rotationIncrement = 180;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief run run the thread to position piece.
///
/// This method runs the thread that positions the piece near to the sheet edge.
//---------------------------------------------------------------------------------------------------------------------
void VPosition::run()
{
    if (m_stop->load())
    {
        return;
    }

    // We should use copy of the piece.
    VLayoutPiece workpiece = m_piece;

    int pieceEdgeNum = m_pieceEdgeNum; // For flipped piece edge will be different
    if (edgesIntersect(workpiece, pieceEdgeNum, m_sheetEdgeNum))
    {
        saveCandidate(m_bestResult, workpiece, pieceEdgeNum, m_sheetEdgeNum, BestFrom::Combine);
    }

    if (m_rotate)
    {
        rotate(m_rotationIncrement);
    }
    else
    {
        if (m_sheet.GetContour().isEmpty())
        {
            rotate(m_rotationIncrement);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getBestResult getter for m_bestResult.
///
/// This method gets and returns m_bestResult.
///
/// @returns VBestSquare m_bestResult.
//---------------------------------------------------------------------------------------------------------------------
VBestSquare VPosition::getBestResult() const
{
    return m_bestResult;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief saveCandidate saves the best square.
///
/// This method saves the resulting new size after adding the piece to the sheet.
///
/// @param bestResult new sheet size saved here.
/// @param piece pattern piece being added to sheet.
/// @param pieceEdgeNum edge number of piece to be combined with the sheet edge.
/// @param sheetEdgeNum edge of sheet piece is being comibed with.
/// @param type type of best sqaure being saved.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VPosition::saveCandidate(VBestSquare &bestResult, const VLayoutPiece &piece, int pieceEdgeNum, int sheetEdgeNum,
                              BestFrom type)
{
    QVector<QPointF> newContour = m_sheet.UniteWithContour(piece, sheetEdgeNum, pieceEdgeNum, type);
    newContour.append(newContour.first());
    const QSizeF size = QPolygonF(newContour).boundingRect().size();
    bestResult.NewResult(size, sheetEdgeNum, pieceEdgeNum, piece.getTransform(), piece.isMirror(), type);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief edgesIntersect check for intersection between piece and sheet
///
/// This method checks if the pattern piece edge intersects with the sheet edge.
///
/// @param piece pattern piece being added to sheet.
/// @param pieceEdgeNum edge number of piece to be combined with the sheet edge.
/// @param sheetEdgeNum edge of sheet piece is being comibed with.
/// @returns bool.
///     - true if there no intersection.
///     - false if it has an intersection.
//---------------------------------------------------------------------------------------------------------------------
bool VPosition::edgesIntersect(VLayoutPiece &piece, int &pieceEdgeNum, int sheetEdgeNum)
{
    const QLineF sheetEdge = m_sheet.GlobalEdge(sheetEdgeNum);
    bool isFlipped = false;
    bool hasNoIntersectiom = false;

    combineEdges(piece, pieceEdgeNum, sheetEdge);

    IntersectionType type = IntersectionType::Intersection;
    if (sheetContains(piece.pieceBoundingRect()))
    {
        if (!m_sheet.GetContour().isEmpty())
        {
            type = intersectionType(piece);
        }
        else
        {
            type = IntersectionType::NoIntersection;
        }
    }

    switch (type)
    {
        case IntersectionType::Intersection:
            piece.Mirror(sheetEdge);
            isFlipped = true;
            break;
        case IntersectionType::NoIntersection:
            hasNoIntersectiom = true;
            break;
        default:
            break;
    }

    if (isFlipped && !piece.IsForbidFlipping())
    {
        if (m_sheet.GetContour().isEmpty())
        {
            pieceEdgeNum = piece.pieceEdgeByPoint(sheetEdge.p2());
        }
        else
        {
            pieceEdgeNum = piece.LayoutEdgeByPoint(sheetEdge.p2());
        }

        if (pieceEdgeNum <= 0)
        {
            return false;
        }

        IntersectionType type = IntersectionType::Intersection;
        if (sheetContains(piece.pieceBoundingRect()))
        {
            type = intersectionType(piece);
        }

        switch (type)
        {
            case IntersectionType::Intersection:
                hasNoIntersectiom = false;
                break;
            case IntersectionType::NoIntersection:
                hasNoIntersectiom = true;
                break;
            default:
                break;
        }
    }
    return hasNoIntersectiom;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief rotatedEdgeIntersects check for intersection between rotated piece and sheet
///
/// This method checks if rotated pattern piece edge intersects with the sheet edge.
///
/// @param piece pattern piece being added to sheet.
/// @param pieceEdgeNum edge number of piece to be combined with the sheet edge.
/// @param sheetEdgeNum edge of sheet piece is being comibed with.
/// @returns bool.
///     - true if there no intersection.
///     - false if it has an intersection.
//---------------------------------------------------------------------------------------------------------------------
bool VPosition::rotatedEdgeIntersects(VLayoutPiece &piece, int pieceEdgeNum, int sheetEdgeNum, int angle) const
{
    const QLineF sheetEdge = m_sheet.GlobalEdge(sheetEdgeNum);
    bool hasNoIntersectiom = false;

    rotateEdges(piece, pieceEdgeNum, sheetEdge, angle);

    IntersectionType type = IntersectionType::Intersection;
    if (sheetContains(piece.pieceBoundingRect()))
    {
        type = intersectionType(piece);
    }

    switch (type)
    {
        case IntersectionType::Intersection:
            hasNoIntersectiom = false;
            break;
        case IntersectionType::NoIntersection:
            hasNoIntersectiom = true;
            break;
        default:
            break;
    }
    return hasNoIntersectiom;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief intersectionType get the intersection type between sheet and piece.
///
/// This method checks if the bounding box of a piece intersects with sheet bounding box.
///
/// @param piece layput piece.
/// @returns IntersectionType intersection type
///     - NoIntersection = 0,
///     - Intersection   = 1,
//---------------------------------------------------------------------------------------------------------------------
VPosition::IntersectionType VPosition::intersectionType(const VLayoutPiece &piece) const
{
    const QRectF globalRect = m_sheet.BoundingRect();
    if (!globalRect.intersects(piece.LayoutBoundingRect()) && !globalRect.contains(piece.pieceBoundingRect()))
    {
        // This we can determine efficiently.
        return IntersectionType::NoIntersection;
    }

    const QPainterPath globalPath = m_sheet.ContourPath();
    if (!globalPath.intersects(piece.LayoutAllowancePath()) && !globalPath.contains(piece.createMainPath()))
    {
        return IntersectionType::NoIntersection;
    }
    else
    {
        return IntersectionType::Intersection;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief sheetContains check if sheet contains piece.
///
/// This method checks if the bounding box of a piece is contained in the sheet bounding box.
///
/// @param rect bounding rectangle of the piece.
/// @returns bool
///     - true if sheet contains piece.
///     - false if sheet does not contain piece.
//---------------------------------------------------------------------------------------------------------------------
bool VPosition::sheetContains(const QRectF &rect) const
{
    const QRectF sheetRect(0, 0, m_sheet.GetWidth(), m_sheet.GetHeight());
    return sheetRect.contains(rect);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief combineEdges rotate and move piece to contour edge.
///
/// This method rotates and moves piece near to the sheet edge.
///
/// @param piece layout piece.
/// @param pieceEdgeNum piece edge number.
/// @param sheetEdge line that represents the sheet edge.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VPosition::combineEdges(VLayoutPiece &piece, const int &pieceEdgeNum, const QLineF &sheetEdge)
{
    QLineF pieceEdge;
    if (m_sheet.GetContour().isEmpty())
    {
        pieceEdge = piece.pieceEdge(pieceEdgeNum);
    }
    else
    {
        pieceEdge = piece.LayoutEdge(pieceEdgeNum);
    }

    // Find distance between sheet and piece edges.
    const qreal dx = sheetEdge.x2() - pieceEdge.x2();
    const qreal dy = sheetEdge.y2() - pieceEdge.y2();

    pieceEdge.translate(dx, dy); // Use values for translate piece edge.

    m_angleBetween = sheetEdge.angleTo(pieceEdge); // Seek angle between two edges.

    // Move piece to position near to sheet edge.
    piece.Translate(dx, dy);
    if (!qFuzzyIsNull(m_angleBetween) || !qFuzzyCompare(m_angleBetween, 360))
    {
        piece.Rotate(pieceEdge.p2(), -m_angleBetween);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief rotateEdges rotate and move piece to sheet edge.
///
/// This method rotates and moves piece near to the sheet edge.
///
/// @param piece layout piece.
/// @param sheetEdge line that represents the sheet edge.
/// @param pieceEdgeNum piece edge number.
/// @param angle piece rotation angle.
/// @return void.
//---------------------------------------------------------------------------------------------------------------------
void VPosition::rotateEdges(VLayoutPiece &piece, int pieceEdgeNum, const QLineF &sheetEdge, int angle) const
{
    QLineF pieceEdge;
    if (m_sheet.GetContour().isEmpty())
    {
        pieceEdge = piece.pieceEdge(pieceEdgeNum);
    }
    else
    {
        pieceEdge = piece.LayoutEdge(pieceEdgeNum);
    }

    // Find distance between sheet and piece edges.
    const qreal dx = sheetEdge.x2() - pieceEdge.x2();
    const qreal dy = sheetEdge.y2() - pieceEdge.y2();

    // Move and rotate piece to position near to sheet edge.
    piece.Translate(dx, dy);
    piece.Rotate(sheetEdge.p2(), angle);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief rotate rotate the pattern piece.
///
/// This method rotates the pattern piece by the number of degrees in incrmentuntil it
/// intersects with the sheet. The best square result is saved.
///
/// @param increment pattern piece rotation angle.
/// @return void.
//---------------------------------------------------------------------------------------------------------------------
void VPosition::rotate(int increment)
{
    int startAngle = 0;
    if (VFuzzyComparePossibleNulls(m_angleBetween, 360))
    {
        startAngle = increment;
    }
    for (int angle = startAngle; angle < 360; angle = angle + increment)
    {
        if (m_stop->load())
        {
            return;
        }

        // We should use copy of the piece.
        VLayoutPiece workpiece = m_piece;

        if (rotatedEdgeIntersects(workpiece, m_pieceEdgeNum, m_sheetEdgeNum, angle))
        {
            saveCandidate(m_bestResult, workpiece, m_pieceEdgeNum, m_sheetEdgeNum, BestFrom::Rotation);
        }
    }
}

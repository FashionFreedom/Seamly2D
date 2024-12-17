//---------------------------------------------------------------------------------------------------------------------
//   @file   vposition.cpp
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
//   @file   vposition.cpp
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   7 1, 2015
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

#include "vlayoutpaper.h"

#include <QBrush>
#include <QCoreApplication>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QList>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QThread>
#include <QThreadPool>
#include <QVector>
#include <Qt>
#include <QtAlgorithms>

#include "vbestsquare.h"
#include "vcontour.h"
#include "vlayoutpiece.h"
#include "vlayoutpaper_p.h"
#include "vposition.h"

#ifdef Q_COMPILER_RVALUE_REFS
VLayoutPaper &VLayoutPaper::operator=(VLayoutPaper &&paper) Q_DECL_NOTHROW
{
    Swap(paper);
    return *this;
}
#endif

void VLayoutPaper::Swap(VLayoutPaper &paper) Q_DECL_NOTHROW
{ std::swap(d, paper.d); }

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper()
    : d(new VLayoutPaperData)
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper(int height, int width)
    : d(new VLayoutPaperData(height, width))
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper(const VLayoutPaper &paper)
    : d(paper.d)
{}

//---------------------------------------------------------------------------------------------------------------------
/// Assigns paper to this paper and returns a reference to this.
//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper &VLayoutPaper::operator=(const VLayoutPaper &paper)
{
    if (&paper == this)
    {
        return *this;
    }
    d = paper.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/// Destroys this VLayoutPaper.
//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::~VLayoutPaper()
{}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetHeight gets the height of the paper.
///
/// This method gets the height of the paper.
///
/// @returns int height of paper.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutPaper::GetHeight() const
{
    return d->globalContour.GetHeight();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setHeight sets the height of the paper.
///
/// This method sets the height of the paper.
///
/// @param height of paper.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::setHeight(int height)
{
    d->globalContour.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetHeight sets the height of the paper.
///
/// This method sets the height of the paper.
///
/// @returns int height of paper.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutPaper::GetWidth() const
{
    return d->globalContour.GetWidth();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setHeight sets the width of the paper.
///
/// This method sets the width of the paper.
///
/// @param width of paper.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetWidth(int width)
{
    d->globalContour.SetWidth(width);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getLayoutGap gets the gap width of the layout.
///
/// This method gets the ga width of the layout.
///
/// @returns qreal width of gap.
//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutPaper::getLayoutGap() const
{
    return d->layoutWidth;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLayoutGap sets the gap width of the layout.
///
/// This method sets the gap width of the layout.
///
/// @param width of gap.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::setLayoutGap(qreal width)
{
    if (width >= 0)
    {
        d->layoutWidth = width;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetShift gets the shift of the paper.
///
/// This method gets the shift length of the paper as set in the layout settings dialog.
///
/// @returns quint32 shift length.
//---------------------------------------------------------------------------------------------------------------------
quint32 VLayoutPaper::GetShift() const
{
    return d->globalContour.GetShift();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetShift sets the shift of the paper.
///
/// This method sets the shift of the paper.
///
/// @param shift of the paper.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetShift(quint32 shift)
{
    d->globalContour.SetShift(shift);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetRotate gets the paper rotation.
///
/// This method gets whether the paper is rotated i.e. it's orientation.
///
/// @returns bool.
///     - true paper is rotated. (landscape)
///     - false paper is not rotated. (portrait)
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPaper::GetRotate() const
{
    return d->globalRotate;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRotate sets the paper is rotated.
///
/// This method sets whether the paper is rotated i.e. the orientation.
///
/// @param value
///     - true paper is rotated. (landscape)
///     - false paper is not rotated. (portrait)
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetRotate(bool value)
{
    d->globalRotate = value;
    d->localRotate = d->globalRotate;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetRotationIncrease gets the rotation increment.
///
/// This method gets the rotation increment.
///
/// @returns int rotation increment.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutPaper::GetRotationIncrease() const
{
    return d->globalRotationIncrease;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRotationIncrease sets the rotation increment.
///
/// This method sets the rotation increment.
///
/// @param value rotation increment.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetRotationIncrease(int value)
{
    d->globalRotationIncrease = value;

    if ((d->globalRotationIncrease >= 1
         && d->globalRotationIncrease <= 180
         && 360 % d->globalRotationIncrease == 0) == false)
    {
        d->globalRotationIncrease = 180;
    }

    d->localRotationIncrease = d->globalRotationIncrease;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief IsSaveLength gets savelength of the layout.
///
/// This method gets whether to save the length of the layout.
///
/// @returns bool.
///     - true save length.
///     - false don't save length.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPaper::IsSaveLength() const
{
    return d->saveLength;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetSaveLength sets savelength of the layout.
///
/// This method sets whether to save the length of the layout.
///
/// @param value.
///     - true save length.
///     - false don't save length.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetSaveLength(bool value)
{
    d->saveLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setPaperIndex sets the paper index.
///
/// This method sets the paper index.
///
/// @param index paper index.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::setPaperIndex(quint32 index)
{
    d->paperIndex = index;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPaper::arrangePiece(const VLayoutPiece &piece, std::atomic_bool &stop)
{
    // First need set size of paper
    if (d->globalContour.GetHeight() <= 0 || d->globalContour.GetWidth() <= 0)
    {
        return false;
    }

    if (piece.LayoutEdgesCount() < 3 || piece.pieceEdgesCount() < 3)
    {
        return false;//Not enough edges
    }

    if (piece.IsForbidFlipping() && !d->globalRotate)
    { // Compensate forbidden flipping by rotating. 180 degree will be enough.
        d->localRotate = true;
        d->localRotationIncrease = 180;
    }
    else
    { // Return to global values if was changed
        d->localRotate = d->globalRotate;
        d->localRotationIncrease = d->globalRotationIncrease;
    }

    return addToSheet(piece, stop);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Count returns the number of piece.
///
/// This method returns number of pieces used in the layout.
///
/// @returns int count.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutPaper::Count() const
{
    return d->pieces.count();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief addToSheet add piece to layout page.
///
/// This method adds a piece to a layout page.
///
/// @param piece piece being added to page.
/// @param stop shared bool between threads.
/// @returns bool true if valid result, false if invalid result.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPaper::addToSheet(const VLayoutPiece &piece, std::atomic_bool &stop)
{
    VBestSquare bestResult(d->globalContour.GetSize(), d->saveLength);
    QThreadPool *thread_pool = QThreadPool::globalInstance();
    thread_pool->setExpiryTimeout(1000);
    QVector<VPosition *> threads;

    int pieceEdgesCount = 0;

    if (d->globalContour.GetContour().isEmpty())
    {
        pieceEdgesCount = piece.pieceEdgesCount();
    }
    else
    {
        pieceEdgesCount = piece.LayoutEdgesCount();
    }

    for (int j=1; j <= d->globalContour.GlobalEdgesCount(); ++j)
    {
        for (int i=1; i<= pieceEdgesCount; ++i)
        {
            VPosition *thread = new VPosition(d->globalContour, j, piece, i, &stop, d->localRotate,
                                              d->localRotationIncrease,
                                              d->saveLength);
            thread->setAutoDelete(false);
            threads.append(thread);
            thread_pool->start(thread);
        }
    }

    // Wait for done
    do
    {
        QCoreApplication::processEvents();
        QThread::msleep(250);
    }
    while(thread_pool->activeThreadCount() > 0 && !stop.load());

    if (stop.load())
    {
        qDeleteAll(threads.begin(), threads.end());
        threads.clear();
        return false;
    }

    for (int i=0; i < threads.size(); ++i)
    {
        bestResult.NewResult(threads.at(i)->getBestResult());
    }

    qDeleteAll(threads.begin(), threads.end());
    threads.clear();

    return saveResult(bestResult, piece);
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPaper::saveResult(const VBestSquare &bestResult, const VLayoutPiece &piece)
{
    if (bestResult.ValidResult())
    {
        VLayoutPiece workDetail = piece;
        workDetail.setTransform(bestResult.Transform());// Don't forget set transform
        workDetail.SetMirror(bestResult.isMirror());
        const QVector<QPointF> newGContour = d->globalContour.UniteWithContour(workDetail, bestResult.GContourEdge(),
                                                                               bestResult.pieceEdge(),
                                                                               bestResult.Type());
        if (newGContour.isEmpty())
        {
            return false;
        }
        d->pieces.append(workDetail);
        d->globalContour.SetContour(newGContour);
    }

    return bestResult.ValidResult(); // Do we have the best result?
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getPaperItem returns paper an item.
///
/// This method returns a paper (page) as a QGraphicsRectItem.
///
/// @param autoCrop If true autocrop the page, if false otherwise.
/// @param textAsPaths If true append text as a path, if false otherwise.
/// @returns paper.
//---------------------------------------------------------------------------------------------------------------------
QGraphicsRectItem *VLayoutPaper::getPaperItem(bool autoCrop, bool textAsPaths) const
{
    QGraphicsRectItem *paper;
    if (autoCrop)
    {
        QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());
        QList<QGraphicsItem *> list = getPieceItems(textAsPaths);
        for (int i=0; i < list.size(); ++i)
        {
            scene->addItem(list.at(i));
        }
        const int height = scene->itemsBoundingRect().toRect().height();
        if (d->globalContour.GetHeight() > height) //-V807
        {
            paper = new QGraphicsRectItem(QRectF(0, 0, d->globalContour.GetWidth(), height));
        }
        else
        {
            paper = new QGraphicsRectItem(QRectF(0, 0, d->globalContour.GetWidth(), d->globalContour.GetHeight()));
        }
    }
    else
    {
        paper = new QGraphicsRectItem(QRectF(0, 0, d->globalContour.GetWidth(), d->globalContour.GetHeight()));
    }
    paper->setPen(QPen(Qt::black, 1));
    paper->setBrush(QBrush(Qt::white));
    return paper;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getPieceItems returns the piece items in the layout.
///
/// This method returns list of pieces used in the layout as graphicitems.
///
/// @param textAsPaths if true append text as path, if false otherwise.
/// @returns QList<QGraphicsItem.
//---------------------------------------------------------------------------------------------------------------------
QList<QGraphicsItem *> VLayoutPaper::getPieceItems(bool textAsPaths) const
{
    QList<QGraphicsItem *> list;
    for (int i=0; i < d->pieces.count(); ++i)
    {
        list.append(d->pieces.at(i).GetItem(textAsPaths));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getPieces returns the pieces in the layout.
///
/// This method returns a vector of pieces used in the layout.
///
/// @returns QVector<VLayoutPiece>.
//---------------------------------------------------------------------------------------------------------------------
QVector<VLayoutPiece> VLayoutPaper::getPieces() const
{
    return d->pieces;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setPieces sets the pieces in the layout.
///
/// This method converts and sets a vector from list of pieces used in the layout.
///
/// @param pieces list of layout pieces.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::setPieces(const QList<VLayoutPiece> &pieces)
{
    d->pieces = pieces.toVector();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief piecesBoundingRect returns the bounding rect of pieces.
///
/// This method returns bounding rect of all the pieces used in the layout.
///
/// @returns QRectF bounding rect of pieces.
//---------------------------------------------------------------------------------------------------------------------
QRectF VLayoutPaper::piecesBoundingRect() const
{
    QRectF rect;
    for (int i=0; i < d->pieces.count(); ++i)
    {
        rect = rect.united(d->pieces.at(i).pieceBoundingRect());
    }

    return rect;
}

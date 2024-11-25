//---------------------------------------------------------------------------------------------------------------------
//   @file   vposition.h
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
//   @file   vposition.h
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

#ifndef VLAYOUTPAPER_H
#define VLAYOUTPAPER_H

#include <qcompilerdetection.h>
#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>
#include <atomic>

#include "vlayoutdef.h"

class VBestSquare;
class VLayoutPaperData;
class VLayoutPiece;
class QGraphicsRectItem;
class QRectF;
class QGraphicsItem;
template <typename T> class QList;
template <typename T> class QVector;

class VLayoutPaper
{
public:
    VLayoutPaper();
    VLayoutPaper(int height, int width);
    VLayoutPaper(const VLayoutPaper &paper);

    ~VLayoutPaper();

    VLayoutPaper &operator=(const VLayoutPaper &paper);
    
//check if the current compiler supports C++11's rvalue references
#ifdef Q_COMPILER_RVALUE_REFS
	VLayoutPaper &operator=(VLayoutPaper &&paper) Q_DECL_NOTHROW;
#endif

	void    Swap(VLayoutPaper &paper) Q_DECL_NOTHROW;

    int     GetHeight() const;
    void    setHeight(int height);

    int     GetWidth() const;
    void    SetWidth(int width);

    qreal   getLayoutGap() const;
    void    setLayoutGap(qreal width);

    quint32 GetShift() const;
    void    SetShift(quint32 shift);

    bool    GetRotate() const;
    void    SetRotate(bool value);

    int     GetRotationIncrease() const;
    void    SetRotationIncrease(int value);

    bool    IsSaveLength() const;
    void    SetSaveLength(bool value);

    void    setPaperIndex(quint32 index);

    bool    arrangePiece(const VLayoutPiece &piece, std::atomic_bool &stop);
    int     Count() const;
    Q_REQUIRED_RESULT QGraphicsRectItem     *getPaperItem(bool autoCrop, bool textAsPaths) const;
    Q_REQUIRED_RESULT QList<QGraphicsItem *> getPieceItems(bool textAsPaths) const;

    QVector<VLayoutPiece> getPieces() const;
    void                  setPieces(const QList<VLayoutPiece>& pieces);

    QRectF                piecesBoundingRect() const;

private:
    QSharedDataPointer<VLayoutPaperData> d;
    bool addToSheet(const VLayoutPiece &piece, std::atomic_bool &stop);
    bool saveResult(const VBestSquare &bestResult, const VLayoutPiece &piece);
};

Q_DECLARE_TYPEINFO(VLayoutPaper, Q_MOVABLE_TYPE);

#endif // VLAYOUTPAPER_H

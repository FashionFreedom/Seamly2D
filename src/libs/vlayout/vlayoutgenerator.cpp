/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   vlayoutgenerator.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 1, 2015
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

#include "vlayoutgenerator.h"

#include <QGraphicsRectItem>
#include <QRectF>
#include <QThreadPool>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "vlayoutpiece.h"
#include "vlayoutpaper.h"

//---------------------------------------------------------------------------------------------------------------------
VLayoutGenerator::VLayoutGenerator(QObject *parent)
    : QObject(parent),
      papers(),
      bank(new VBank()),
      paperHeight(0),
      paperWidth(0),
      margins(),
      usePrinterFields(true),
#ifdef Q_CC_MSVC
      // See https://stackoverflow.com/questions/15750917/initializing-stdatomic-bool
      stopGeneration(ATOMIC_VAR_INIT(false)),
#else
      stopGeneration(false),
#endif
      state(LayoutErrors::NoError),
      shift(0),
      rotate(true),
      rotationIncrease(180),
      autoCrop(false),
      saveLength(false),
      unitePages(false),
      stripOptimizationEnabled(false),
      multiplier(1),
      stripOptimization(false),
      textAsPaths(false)
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutGenerator::~VLayoutGenerator()
{
    delete bank;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setPieces(const QVector<VLayoutPiece> &pieces)
{
    bank->setPieces(pieces);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetLayoutWidth(qreal width)
{
    bank->SetLayoutWidth(width);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetCaseType(Cases caseType)
{
    bank->SetCaseType(caseType);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
int VLayoutGenerator::PieceCount()
{
    return bank->allPieceCount();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Generate()
{
    stopGeneration.store(false);
    papers.clear();
    state = LayoutErrors::NoError;

    emit Start();

    if (bank->Prepare())
    {
        const int width = PageWidth();
        int height = PageHeight();

        if (stripOptimization)
        {
            const qreal b = bank->GetBiggestDiagonal() * multiplier + bank->GetLayoutWidth();

            if (height >= b*2)
            {
                stripOptimizationEnabled = true;
                height = qFloor(height / qFloor(height/b));
            }
        }

        while (bank->allPieceCount() > 0)
        {
            if (stopGeneration.load())
            {
                break;
            }

            VLayoutPaper paper(height, width);
            paper.SetShift(shift);
            paper.SetLayoutWidth(bank->GetLayoutWidth());
            paper.SetPaperIndex(static_cast<quint32>(papers.count()));
            paper.SetRotate(rotate);
            paper.SetRotationIncrease(rotationIncrease);
            paper.SetSaveLength(saveLength);
            do
            {
                const int index = bank->GetTiket();
                if (paper.arrangePiece(bank->getPiece(index), stopGeneration))
                {
                    bank->Arranged(index);
                    emit Arranged(bank->ArrangedCount());
                }
                else
                {
                    bank->NotArranged(index);
                }

                if (stopGeneration.load())
                {
                    break;
                }
            } while(bank->LeftArrange() > 0);

            if (stopGeneration.load())
            {
                break;
            }

            if (paper.Count() > 0)
            {
                papers.append(paper);
            }
            else
            {
                state = LayoutErrors::EmptyPaperError;
                emit Error(state);
                return;
            }
        }
    }
    else
    {
        state = LayoutErrors::PrepareLayoutError;
        emit Error(state);
        return;
    }

    if (stripOptimizationEnabled)
    {
        GatherPages();
    }

    if (IsUnitePages())
    {
        UnitePages();
    }

    emit Finished();
}

//---------------------------------------------------------------------------------------------------------------------
LayoutErrors VLayoutGenerator::State() const
{
    return state;
}

//---------------------------------------------------------------------------------------------------------------------
QList<QGraphicsItem *> VLayoutGenerator::GetPapersItems() const
{
    QList<QGraphicsItem *> list;
    for (int i=0; i < papers.count(); ++i)
    {
        list.append(papers.at(i).GetPaperItem(autoCrop, IsTestAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QList<QList<QGraphicsItem *> > VLayoutGenerator::getAllPieceItems() const
{
    QList<QList<QGraphicsItem *> > list;
    for (int i=0; i < papers.count(); ++i)
    {
        list.append(papers.at(i).getPieceItems(IsTestAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QVector<VLayoutPiece> > VLayoutGenerator::getAllPieces() const
{
    QVector<QVector<VLayoutPiece> > list;
    for (int i=0; i < papers.count(); ++i)
    {
        list.append(papers.at(i).getPieces());
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Abort()
{
    stopGeneration.store(true);
    state = LayoutErrors::ProcessStoped;
    QThreadPool::globalInstance()->clear();
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsStripOptimization() const
{
    return stripOptimization;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetStripOptimization(bool value)
{
    stripOptimization = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsTestAsPaths() const
{
    return textAsPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetTestAsPaths(bool value)
{
    textAsPaths = value;
}

//---------------------------------------------------------------------------------------------------------------------
quint8 VLayoutGenerator::GetMultiplier() const
{
    return multiplier;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetMultiplier(const quint8 &value)
{
    if (value > 10)
    {
        multiplier = 10;
    }
    else if (value == 0)
    {
        multiplier = 1;
    }
    else
    {
        multiplier = value;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutGenerator::PageHeight() const
{
    return static_cast<int>(paperHeight - (margins.top() + margins.bottom()));
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutGenerator::PageWidth() const
{
    return static_cast<int>(paperWidth - (margins.left() + margins.right()));
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::GatherPages()
{
    if (papers.size() < 2)
    {
        return;
    }

    QList<QList<VLayoutPiece>> pieces;
    qreal length = 0;
    int j = 0; // papers count

    for (int i = 0; i < papers.size(); ++i)
    {
        int paperHeight = qRound(papers.at(i).piecesBoundingRect().height());

        if (i != papers.size()-1)
        {
            paperHeight += qRound(bank->GetLayoutWidth()*2);
        }

        if (length + paperHeight <= PageHeight())
        {
            unitePieces(j, pieces, length, i);
            length += paperHeight;
        }
        else
        {
            length = 0; // Start new paper
            ++j;// New paper
            unitePieces(j, pieces, length, i);
            length += paperHeight;
        }
    }

    QVector<VLayoutPaper> nPapers;
    for (int i = 0; i < pieces.size(); ++i)
    {
        VLayoutPaper paper(PageHeight(), PageWidth());
        paper.SetShift(shift);
        paper.SetLayoutWidth(bank->GetLayoutWidth());
        paper.SetPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(rotate);
        paper.SetRotationIncrease(rotationIncrease);
        paper.SetSaveLength(saveLength);
        paper.setPieces(pieces.at(i));

        nPapers.append(paper);
    }

    papers.clear();
    papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::UnitePages()
{
    if (papers.size() < 2)
    {
        return;
    }

    QList<qreal> papersLength;
    QList<QList<VLayoutPiece> > pieces;
    qreal length = 0;
    int j = 0; // papers count

    for (int i = 0; i < papers.size(); ++i)
    {
        int paperHeight = 0;
        if (autoCrop)
        {
            paperHeight = qRound(papers.at(i).piecesBoundingRect().height());
        }
        else
        {
            paperHeight = papers.at(i).GetHeight();
        }

        if (i != papers.size()-1)
        {
            paperHeight = qRound(paperHeight + bank->GetLayoutWidth()*2);
        }

        if (length + paperHeight <= QIMAGE_MAX)
        {
            unitePieces(j, pieces, length, i);
            length += paperHeight;
            UnitePapers(j, papersLength, length);
        }
        else
        {
            length = 0; // Start new paper
            ++j;// New paper
            unitePieces(j, pieces, length, i);
            length += paperHeight;
            UnitePapers(j, papersLength, length);
        }
    }

    QVector<VLayoutPaper> nPapers;
    for (int i = 0; i < pieces.size(); ++i)
    {
        VLayoutPaper paper(qFloor(papersLength.at(i)), PageWidth());
        paper.SetShift(shift);
        paper.SetLayoutWidth(bank->GetLayoutWidth());
        paper.SetPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(rotate);
        paper.SetRotationIncrease(rotationIncrease);
        paper.SetSaveLength(saveLength);
        paper.setPieces(pieces.at(i));

        nPapers.append(paper);
    }

    papers.clear();
    papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::unitePieces(int j, QList<QList<VLayoutPiece> > &pieces, qreal length, int i)
{
    if ((j == 0 && pieces.isEmpty()) || j >= pieces.size())
    {//First or new pieces in paper
        pieces.insert(j, movePieces(length, papers.at(i).getPieces()));
    }
    else
    {
        pieces[j].append(movePieces(length, papers.at(i).getPieces()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::UnitePapers(int j, QList<qreal> &papersLength, qreal length)
{
    if ((j == 0 && papersLength.isEmpty()) || j >= papersLength.size())
    {
        papersLength.insert(j, length);
    }
    else
    {
        papersLength[j] = length;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QList<VLayoutPiece> VLayoutGenerator::movePieces(qreal length, const QVector<VLayoutPiece> &pieces)
{
    if (qFuzzyIsNull(length))
    {
        return pieces.toList();
    }

    QList<VLayoutPiece> newPieces;
    for (int i = 0; i < pieces.size(); ++i)
    {
        VLayoutPiece d = pieces.at(i);
        d.Translate(0, length);
        newPieces.append(d);
    }

    return newPieces;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsUnitePages() const
{
    return unitePages;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetUnitePages(bool value)
{
    unitePages = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsSaveLength() const
{
    return saveLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetSaveLength(bool value)
{
    saveLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::GetAutoCrop() const
{
    return autoCrop;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetAutoCrop(bool value)
{
    autoCrop = value;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
int VLayoutGenerator::GetRotationIncrease() const
{
    return rotationIncrease;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotationIncrease(int value)
{
    rotationIncrease = value;

    if ((rotationIncrease >= 1 && rotationIncrease <= 180 && 360 % rotationIncrease == 0) == false)
    {
        rotationIncrease = 180;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::GetRotate() const
{
    return rotate;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotate(bool value)
{
    rotate = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutGenerator::GetPaperWidth() const
{
    return paperWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperWidth(qreal value)
{
    paperWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsUsePrinterFields() const
{
    return usePrinterFields;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VLayoutGenerator::GetPrinterFields() const
{
    return margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPrinterFields(bool usePrinterFields, const QMarginsF &value)
{
    this->usePrinterFields = usePrinterFields;
    margins = value;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VLayoutGenerator::GetShift() const
{
    return shift;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetShift(quint32 shift)
{
    this->shift = shift;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutGenerator::GetPaperHeight() const
{
    return paperHeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperHeight(qreal value)
{
    paperHeight = value;
}

//---------------------------------------------------------------------------------------------------------------------
//   @file   vlayoutgenerator.cpp
//   @author Douglas S Caskey
//   @date   Nov 28, 2024
//
//   @copyright
//   Copyright (C) 2017 - 2024 Seamly, LLC
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
//   @file   vlayoutgenerator.cpp
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   2 1, 2015
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
    : QObject(parent)
    , m_papers()
    , m_bank(new VBank())
    , m_paperHeight(0)
    , m_paperWidth(0)
    , m_margins()
    , m_usePrinterFields(true)
    , m_stopGeneration(false)
    , m_state(LayoutErrors::NoError)
    , m_shift(0)
    , m_rotate(true)
    , m_rotationIncrement(180)
    , m_autoCrop(false)
    , m_saveLength(false)
    , m_unitePages(false)
    , m_stripOptimized(false)
    , m_multiplier(1)
    , m_useStripOptimization(false)
    , m_textAsPaths(false)
{}

//---------------------------------------------------------------------------------------------------------------------
/// Destroys this VLayoutGenerator.
//---------------------------------------------------------------------------------------------------------------------
VLayoutGenerator::~VLayoutGenerator()
{
    delete m_bank;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setPieces sets the bank pieces.
///
/// This method sets the bank to the vector of layout pieces.
///
/// @param pieces vector of layout pieces.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setPieces(const QVector<VLayoutPiece> &pieces)
{
    m_bank->setPieces(pieces);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLayoutGap sets layout gap width.
///
/// This method sets the width of the gap layout.
///
/// @param width width of gap.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setLayoutGap(qreal width)
{
    m_bank->setLayoutGap(width);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setCaseType sets the case type.
///
/// This method sets the case type for choosing the next workpiece
/// as set in the layout settings dialog.
///
/// @param caseType
///     - Three groups: big, middle, small.
///     - Two groups: big, small.
///     - Descending area.
/// @returns void
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setCaseType(Cases caseType)
{
    m_bank->setCaseType(caseType);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
int VLayoutGenerator::PieceCount()
{
    return m_bank->allPieceCount();
}
//---------------------------------------------------------------------------------------------------------------------
/// @brief Generate generate layout.
///
/// This method generates the pattern layout.
///
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Generate()
{
    m_stopGeneration.store(false);
    m_papers.clear();
    m_state = LayoutErrors::NoError;

    emit Start();

    if (m_bank->Prepare())
    {
        const int width = pageWidth();
        int height = pageHeight();

        if (m_useStripOptimization)
        {
            const qreal b = m_bank->GetBiggestDiagonal() * m_multiplier + m_bank->getLayoutGap();

            if (height >= b*2)
            {
                m_stripOptimized = true;
                height = qFloor(height / qFloor(height/b));
            }
        }

        while (m_bank->allPieceCount() > 0)
        {
            if (m_stopGeneration.load())
            {
                break;
            }

            VLayoutPaper paper(height, width);
            paper.SetShift(m_shift);
            paper.setLayoutGap(m_bank->getLayoutGap());
            paper.setPaperIndex(static_cast<quint32>(m_papers.count()));
            paper.SetRotate(m_rotate);
            paper.SetRotationIncrease(m_rotationIncrement);
            paper.SetSaveLength(m_saveLength);
            do
            {
                const int index = m_bank->GetTiket();
                if (paper.arrangePiece(m_bank->getPiece(index), m_stopGeneration))
                {
                    m_bank->Arranged(index);
                    emit Arranged(m_bank->ArrangedCount());
                }
                else
                {
                    m_bank->NotArranged(index);
                }

                if (m_stopGeneration.load())
                {
                    break;
                }
            } while(m_bank->LeftArrange() > 0);

            if (m_stopGeneration.load())
            {
                break;
            }

            if (paper.Count() > 0)
            {
                m_papers.append(paper);
            }
            else
            {
                m_state = LayoutErrors::EmptyPaperError;
                emit Error(m_state);
                return;
            }
        }
    }
    else
    {
        m_state = LayoutErrors::PrepareLayoutError;
        emit Error(m_state);
        return;
    }

    if (m_stripOptimized)
    {
        gatherPages();
    }

    if (IsUnitePages())
    {
        unitePages();
    }

    emit Finished();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief State gets the error state.
///
/// This method returns any layout generator errors.
///
/// @returns m_state Could be one one the following:
///     - NoError
///     - PrepareLayoutError
///     - ProcessStoped
///     - EmptyPaperError
//---------------------------------------------------------------------------------------------------------------------
LayoutErrors VLayoutGenerator::State() const
{
    return m_state;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetPapersItems gets a list of paper items.
///
/// This method converts a vector of layout papers (pages) and returns a list of QGraphicItems.
///
/// @returns list.
//---------------------------------------------------------------------------------------------------------------------
QList<QGraphicsItem *> VLayoutGenerator::GetPapersItems() const
{
    QList<QGraphicsItem *> list;
    for (int i=0; i < m_papers.count(); ++i)
    {
        list.append(m_papers.at(i).getPaperItem(m_autoCrop, isTextAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getAllPieceItems gets a list of all piece items.
///
/// This method get all the piece item on the layout papers (pages).
///
/// @Details If the item is a text item and isTextAsPaths is true,
/// text item is convetd to a path.
///
/// @returns list QGraphicsItem of layout pieces.
//---------------------------------------------------------------------------------------------------------------------
QList<QList<QGraphicsItem *> > VLayoutGenerator::getAllPieceItems() const
{
    QList<QList<QGraphicsItem *> > list;
    for (int i=0; i < m_papers.count(); ++i)
    {
        list.append(m_papers.at(i).getPieceItems(isTextAsPaths()));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getAllPieces gets a list of all pieces.
///
/// This method get all the pieces on the layout papers (pages).
///
/// @returns list Qvector of layout pieces.
//---------------------------------------------------------------------------------------------------------------------
QVector<QVector<VLayoutPiece> > VLayoutGenerator::getAllPieces() const
{
    QVector<QVector<VLayoutPiece> > list;
    for (int i=0; i < m_papers.count(); ++i)
    {
        list.append(m_papers.at(i).getPieces());
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Abort abort layout generation.
///
/// This method aborts the layout generation.
///
/// @returns void
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::Abort()
{
    m_stopGeneration.store(true);
    m_state = LayoutErrors::ProcessStoped;
    QThreadPool::globalInstance()->clear();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief useStripOptimization gets whether to use strip optimization.
///
/// This method gets whether to use strip optimization when uniting pages.
///
/// @returns m_useStripOptimization
///     - true if optimization should be used when uniting pages.
///     - false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::useStripOptimization() const
{
    return m_useStripOptimization;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setStripOptimization sets whether to use strip optimization.
///
/// This method sets whether to use strip optimization when uniting pages.
///
/// @param bool value
///     - true if optimization should be used when uniting pages.
///     - false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setStripOptimization(bool value)
{
    m_useStripOptimization = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief isTextAsPaths gets whether to render text as path.
///
/// This method gets whether text is rendered as a path.
///
/// @returns m_unitePages true if text should be rendered as a path false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::isTextAsPaths() const
{
    return m_textAsPaths;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setTextAsPaths sets whether to render text as path.
///
/// This method gets whether text is rendered as a path.
///
/// @returns m_unitePages true if text should be rendered as a path false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::setTextAsPaths(bool value)
{
    m_textAsPaths = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetMultiplier gets the multipler.
///
/// This method gets thew multipler used in the strip optimization.
///
/// @returns m_multiplier.
//---------------------------------------------------------------------------------------------------------------------
quint8 VLayoutGenerator::GetMultiplier() const
{
    return m_multiplier;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetMultiplier sets the multipler.
///
/// This method sets the multipler used in the strip optimization from the layout setting dialog.
///
/// @param value from 1 to 10.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetMultiplier(const quint8 &value)
{
    if (value > 10)
    {
        m_multiplier = 10;
    }
    else if (value == 0)
    {
        m_multiplier = 1;
    }
    else
    {
        m_multiplier = value;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief pageHeight gets the page height.
///
/// This method gets the height of the page which equals the paper height minus
/// the top and bottom margins.
///
/// @returns int page height.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutGenerator::pageHeight() const
{
    return static_cast<int>(m_paperHeight - (m_margins.top() + m_margins.bottom()));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief pageWidth gets the page width.
///
/// This method gets the width of the page which equals the paper width minus
/// the left and right margins.
///
/// @returns int paper width.
//---------------------------------------------------------------------------------------------------------------------
int VLayoutGenerator::pageWidth() const
{
    return static_cast<int>(m_paperWidth - (m_margins.left() + m_margins.right()));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief gatherPages Unite layout pages with optimization.
///
/// This method unites the layout pages if there is more than 1 page with strip optimization.
///
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::gatherPages()
{
    if (m_papers.size() < 2)
    {
        return;
    }

    QList<QList<VLayoutPiece>> pieces;
    qreal length = 0;
    int j = 0; // m_papers count

    for (int i = 0; i < m_papers.size(); ++i)
    {
        int m_paperHeight = qRound(m_papers.at(i).piecesBoundingRect().height());

        if (i != m_papers.size()-1)
        {
            m_paperHeight += qRound(m_bank->getLayoutGap()*2);
        }

        if (length + m_paperHeight <= pageHeight())
        {
            unitePieces(j, pieces, length, i);
            length += m_paperHeight;
        }
        else
        {
            length = 0; // Start new paper
            ++j;// New paper
            unitePieces(j, pieces, length, i);
            length += m_paperHeight;
        }
    }

    QVector<VLayoutPaper> nPapers;
    for (int i = 0; i < pieces.size(); ++i)
    {
        VLayoutPaper paper(pageHeight(), pageWidth());
        paper.SetShift(m_shift);
        paper.setLayoutGap(m_bank->getLayoutGap());
        paper.setPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(m_rotate);
        paper.SetRotationIncrease(m_rotationIncrement);
        paper.SetSaveLength(m_saveLength);
        paper.setPieces(pieces.at(i));

        nPapers.append(paper);
    }

    m_papers.clear();
    m_papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief unitePages Unite layout pages.
///
/// This method unites the layout pages if there is more than 1 page. It first crops any page if
/// the autocrop is true, and then appends them 1 at a time into 1 page.
///
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::unitePages()
{
    if (m_papers.size() < 2)
    {
        return;
    }

    QList<qreal> papersLength;
    QList<QList<VLayoutPiece> > pieces;
    qreal length = 0;
    int j = 0; // m_papers count

    for (int i = 0; i < m_papers.size(); ++i)
    {
        int paperHeight = 0;
        if (m_autoCrop)
        {
            paperHeight = qRound(m_papers.at(i).piecesBoundingRect().height());
        }
        else
        {
            paperHeight = m_papers.at(i).GetHeight();
        }

        if (i != m_papers.size()-1)
        {
            paperHeight = qRound(paperHeight + m_bank->getLayoutGap()*2);
        }

        if (length + paperHeight <= QIMAGE_MAX)
        {
            unitePieces(j, pieces, length, i);
            length += paperHeight;
            unitePapers(j, papersLength, length);
        }
        else
        {
            length = 0; // Start new paper
            ++j;// New paper
            unitePieces(j, pieces, length, i);
            length += paperHeight;
            unitePapers(j, papersLength, length);
        }
    }

    QVector<VLayoutPaper> nPapers;
    for (int i = 0; i < pieces.size(); ++i)
    {
        VLayoutPaper paper(qFloor(papersLength.at(i)), pageWidth());
        paper.SetShift(m_shift);
        paper.setLayoutGap(m_bank->getLayoutGap());
        paper.setPaperIndex(static_cast<quint32>(i));
        paper.SetRotate(m_rotate);
        paper.SetRotationIncrease(m_rotationIncrement);
        paper.SetSaveLength(m_saveLength);
        paper.setPieces(pieces.at(i));

        nPapers.append(paper);
    }

    m_papers.clear();
    m_papers = nPapers;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief unitePieces combines layout pieces.
///
/// This method combines the layout pieces into one list .
///
/// @param pieces list of layout pieces.
/// @param length length of page.
/// @param i number of page.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::unitePieces(int j, QList<QList<VLayoutPiece> > &pieces, qreal length, int i)
{
    if ((j == 0 && pieces.isEmpty()) || j >= pieces.size())
    {//First or new pieces in paper
        pieces.insert(j, movePieces(length, m_papers.at(i).getPieces()));
    }
    else
    {
        pieces[j].append(movePieces(length, m_papers.at(i).getPieces()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief unitePapers combines page lengths into one.
///
/// This method combines the page lengths into one list .
///
/// @param papersLength list of page lengths.
/// @param length length of page.
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::unitePapers(int j, QList<qreal> &papersLength, qreal length)
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
/// @brief movePieces move pieces.
///
/// This method combines the layout pieces into one list .
///
/// @param length of page.
/// @param pieces vector of all the layout pieces.
/// @returns newPieces. Combined list of layout pieces from all the pages.
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
/// @brief IsUnitePages gets whether to unite pages.
///
/// This method gets whether the pages (if possible) should e united into 1 page.
///
/// @returns m_unitePages true if pages should be united false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsUnitePages() const
{
    return m_unitePages;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetUnitePages sets whether to unite pages.
///
/// This method sets whether the pages (if possible) should e united into 1 page..
///
/// @returns m_unitePages true if pages should be united false if otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetUnitePages(bool value)
{
    m_unitePages = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsSaveLength() const
{
    return m_saveLength;
}


//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetSaveLength(bool value)
{
    m_saveLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetAutoCrop gets the paper width.
///
/// This method gets whether to crop the unused portion of paper.
///
/// @returns true if the paper should be cropped, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::GetAutoCrop() const
{
    return m_autoCrop;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetAutoCrop sets whether to crop paper.
///
/// This method sets whether to crop the unused portion of paper.
///
/// @param value true if the paper should be cropped, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetAutoCrop(bool value)
{
    m_autoCrop = value;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
int VLayoutGenerator::GetRotationIncrease() const
{
    return m_rotationIncrement;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRotationIncrease sets sets the rotation increment.
///
/// This method sets rotation increment of the paper.
///
/// @param value rotation increment.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotationIncrease(int value)
{
    m_rotationIncrement = value;

    if ((m_rotationIncrement >= 1 && m_rotationIncrement <= 180 && 360 % m_rotationIncrement == 0) == false)
    {
        m_rotationIncrement = 180;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetRotate gets whether the paper should rotate.
///
/// This method gets whether to rotate the paper.
///
/// @returns true if the paper should be rotated, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::GetRotate() const
{
    return m_rotate;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRotate sets whether the paper should rotate.
///
/// This method sets whether to rotate the paper.
///
/// @param value true if the paper should be rotated, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetRotate(bool value)
{
    m_rotate = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetPaperWidth gets the paper width.
///
/// This method gets the width of the paper.
///
/// @returns qreal paper width.
//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutGenerator::GetPaperWidth() const
{
    return m_paperWidth;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetPaperWidth sets layout width.
///
/// This method sets the width of the paper.
///
/// @param value width of paper.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperWidth(qreal value)
{
    m_paperWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief IsUsePrinterFields gets whether to use the margins.
///
/// This method gets whether to use the paper margins set in the layout dialog.
///
/// @returns true if the layout paper should use the margins, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
bool VLayoutGenerator::IsUsePrinterFields() const
{
    return m_usePrinterFields;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetPrinterFields gets the margins.
///
/// This method gets the paper margins set in the layout dialog.
///
/// @returns margins.
//---------------------------------------------------------------------------------------------------------------------
QMarginsF VLayoutGenerator::GetPrinterFields() const
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetPrinterFields sets the paper marguins.
///
/// This method sets whether to use the paper margins.
/// Also sets the paper margins set in the layout dialog.
///
/// @param value paper margins.
/// @param usePrinterFields true if the layout paper should use the margins, false otherwise.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPrinterFields(bool usePrinterFields, const QMarginsF &value)
{
    m_usePrinterFields = usePrinterFields;
    m_margins = value;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetShift gets the shift.
///
/// This method gets the shift offset length that is set in the layout dialog.
///
/// @returns shift.
//---------------------------------------------------------------------------------------------------------------------
quint32 VLayoutGenerator::GetShift() const
{
    return m_shift;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetShift sets the shift.
///
/// This method sets the shift offset length that is set in the layout dialog.
///
/// @param shift
/// @returns void.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetShift(quint32 shift)
{
    m_shift = shift;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetPaperHeight gets the paper height.
///
/// This method gets the height of the paper.
///
/// @returns qreal paper height.
//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutGenerator::GetPaperHeight() const
{
    return m_paperHeight;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetPaperHeight sets layout height.
///
/// This method sets the height of the paper.
///
/// @param value height of paper.
//---------------------------------------------------------------------------------------------------------------------
void VLayoutGenerator::SetPaperHeight(qreal value)
{
    m_paperHeight = value;
}

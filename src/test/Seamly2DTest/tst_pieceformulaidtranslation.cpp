/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
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
 **************************************************************************/

#include "tst_pieceformulaidtranslation.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_PieceFormulaIdTranslation::TST_PieceFormulaIdTranslation(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A piece's seam allowance width formula (<piece width="..."/>) is translated and read back
 * through the exact same FormulaIdTranslator/PatternFormulaTokens calls
 * PatternPieceTool::addAttributes/VPattern::parsePieceElement use at the XML boundary - proving the
 * formula survives a rename of the point it references, the same way a draft tool's formula does.
 */
void TST_PieceFormulaIdTranslation::TestSeamAllowanceWidthFormulaSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 2001;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1/2").arg(id1));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("Saum_vorne"), 5, 5));

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Saum_vorne/2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A per-node seam allowance before/after override (<node saBefore="..." saAfter="..."/>) is
 * translated the same way VAbstractTool::AddSANode/VAbstractPattern::ParseSANode do.
 */
void TST_PieceFormulaIdTranslation::TestNodeSAOverrideFormulaSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 2002;
    const quint32 id2 = 2003;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->AddLine(id1, id2, id2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2/3"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1/3").arg(id2));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    after->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("Armloch"), 5, 5));
    after->AddLine(id1, id2, id2);

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Line_A1_Armloch/3"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A grainline length formula (<grainline length="..."/>) is translated the same way
 * PatternPieceTool::addGrainline/VPattern::ParsePieceGrainline do.
 */
void TST_PieceFormulaIdTranslation::TestGrainlineFormulaSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 2004;
    const quint32 id2 = 2005;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->AddLine(id1, id2, id2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1").arg(id2));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("Schulter"), 5, 5));
    after->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->AddLine(id1, id2, id2);

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Line_Schulter_A2"));
}

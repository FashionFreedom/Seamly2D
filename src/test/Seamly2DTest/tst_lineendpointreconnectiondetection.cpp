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

#include "tst_lineendpointreconnectiondetection.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/calculator.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/qmuparser/qmuparsererror.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_LineEndpointReconnectionDetection::TST_LineEndpointReconnectionDetection(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reconnecting a line tool's second point (e.g. from A2 to A3, via the property editor's
 * VToolLine::SetSecondPoint or the tool's dialog) is NOT a rename - the composite variable
 * "Line_A1_A2" stops existing altogether, and a different one ("Line_A1_A3") takes its place. A
 * formula elsewhere that stored the id-token for the old pairing must NOT be silently resolved to
 * the new line, and must NOT be silently left displaying stale text either - it has to come back
 * from translation still in id-token form, because that's what makes CheckFormula() (called on
 * every parse, see VAbstractTool::CheckFormula) notice the break and offer the "repair formula"
 * dialog. This is the opposite of TestNodeSAOverrideFormulaSurvivesRename in
 * tst_pieceformulaidtranslation.cpp, which covers the true rename case (same points, new name).
 */
void TST_LineEndpointReconnectionDetection::TestReconnectedLineLeavesDependentFormulaUnresolved()
{
    const Unit unit = Unit::Cm;
    const quint32 idA1 = 4001;
    const quint32 idA2 = 4002;
    const quint32 idA3 = 4003;

    // Before: the line tool connects A1 to A2, and a dependent formula references that line.
    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    before->AddLine(idA1, idA2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1_id%2/2").arg(idA1).arg(idA2));

    // After: the user edited the line tool's second point in the property editor/dialog, so it now
    // connects A1 to A3 instead. "Line_A1_A2" (and its id-token pairing) no longer exists at all -
    // a new composite variable "Line_A1_A3" exists in its place.
    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    after->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    after->AddLine(idA1, idA3);

    // The old id-token pairing must be left untranslated - it must NOT resolve to "Line_A1_A3"
    // (that would silently point the dependent formula at the wrong line) and must NOT resolve to
    // stale text like "Line_A1_A2" either (that would hide the break until the next full parse).
    const QString translated = formulaIdsToNames(stored, idTokenToNameMap(after.data()));
    QCOMPARE(translated, stored);
    QVERIFY(translated != QStringLiteral("Line_A1_A3/2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief The untranslated id-token text left behind by a reconnection is not just cosmetically
 * different - it genuinely fails to evaluate against the reconnected container, the same
 * qmu::QmuParserError that VAbstractTool::CheckFormula() catches on every full/lite parse to pop
 * the "repair formula" dialog. This is what proves editing a line tool's endpoint surfaces the
 * break to the user immediately (via the existing full-reparse-on-reference-change path, see
 * SaveToolOptions::referencesChanged() / issue #1521) instead of only failing silently on the next
 * file reload.
 */
void TST_LineEndpointReconnectionDetection::TestReconnectedLineFormulaFailsEvaluation()
{
    const Unit unit = Unit::Cm;
    const quint32 idA1 = 4004;
    const quint32 idA2 = 4005;
    const quint32 idA3 = 4006;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    before->AddLine(idA1, idA2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2/2"), nameToIdTokenMap(before.data()));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    after->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    after->AddLine(idA1, idA3);

    const QString translated = formulaIdsToNames(stored, idTokenToNameMap(after.data()));

    bool threw = false;
    try
    {
        QScopedPointer<Calculator> cal(new Calculator());
        cal->EvalFormula(after->DataVariables(), translated);
    }
    catch (const qmu::QmuParserError &)
    {
        threw = true;
    }
    QVERIFY2(threw, "expected the dependent formula to fail evaluation, exactly like "
                     "VAbstractTool::CheckFormula() detects and offers to repair on every parse");
}

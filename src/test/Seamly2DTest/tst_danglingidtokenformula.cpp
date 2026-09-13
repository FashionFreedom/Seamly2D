/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
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

#include "tst_danglingidtokenformula.h"

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
TST_DanglingIdTokenFormula::TST_DanglingIdTokenFormula(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A formula is stored referencing two points by id. One of the two points is then
 * deleted (simulated the same way TST_PatternFormulaTokens::TestFormulaReferencingBothTranslatesAndSurvivesRename
 * simulates a rename: rebuild the container fresh, the way a full reparse after an edit
 * actually does - only this time the deleted point's id is simply never added again).
 *
 * Reproduces exactly the sequence HistoryDialog::formulaValue() runs
 * (FormulaIdTranslator::formulaIdsToNames(formula, PatternFormulaTokens::idTokenToNameMap(data)))
 * against a formula that now contains one live and one dangling id token.
 *
 * Expected per formulaIdsToNames' own docstring ("unknown tokens are left untouched"): the
 * surviving point's token resolves to its current name, the deleted point's token is left as
 * the raw "id<N>" text. This test proves that documented behaviour actually holds for a
 * genuinely dangling token, and that translation itself never throws or crashes.
 */
void TST_DanglingIdTokenFormula::TestDanglingIdTokenLeftUntouchedByTranslation()
{
    const Unit unit = Unit::Cm;
    const quint32 id_surviving = 6001;
    const quint32 id_deleted   = 6002;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id_surviving, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(id_deleted,   new VPointF(10, 0, QStringLiteral("A2"), 5, 5));

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1+A2*2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1+id%2*2").arg(id_surviving).arg(id_deleted));

    // A2 is deleted: rebuild the container without it, the way a full reparse after the
    // deletion would - id_deleted is never added back.
    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id_surviving, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    const QString displayed = formulaIdsToNames(stored, idTokenToNameMap(after.data()));
    QCOMPARE(displayed, QStringLiteral("A1+id%1*2").arg(id_deleted));

    // The stored formula itself must stay untouched by the translation attempt.
    QCOMPARE(stored, QStringLiteral("id%1+id%2*2").arg(id_surviving).arg(id_deleted));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Once a dangling id token survives translation as raw text (previous test), what
 * happens if that text actually reaches the formula evaluator - e.g. a tool dialog trying to
 * recompute the value of a formula that still references a since-deleted point?
 *
 * Calculator::EvalFormula() is fed the untranslated "id<N>" text directly against
 * VContainer::DataVariables(), which is keyed by display name, never by id token - so the
 * dangling token can never resolve there either. Confirms the concrete, observed failure mode:
 * a normal, catchable qmu::QmuParserError (ecUNASSIGNABLE_TOKEN) naming the exact dangling
 * token, not a crash, not an unhandled exception, not a silently-wrong numeric result.
 *
 * This is also exactly what VFormula::Eval() (src/libs/vpatterndb/vformula.cpp) already
 * wraps in a try/catch(qmu::QmuParserError&) around its own Calculator::EvalFormula() call,
 * turning this into a plain "Error" shown to the user - this test isolates and proves the
 * underlying exception that catch block relies on is in fact what gets thrown, and that it
 * does not crash the process on its way there.
 */
void TST_DanglingIdTokenFormula::TestDanglingIdTokenFailsEvaluationGracefullyInsteadOfCrashing()
{
    const Unit unit = Unit::Cm;
    const quint32 id_deleted = 6003;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id_deleted, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1*2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1*2").arg(id_deleted));

    // A1 deleted: fresh, empty container - the id is gone entirely.
    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));

    const QString displayed = formulaIdsToNames(stored, idTokenToNameMap(after.data()));
    QCOMPARE(displayed, stored); // still raw "id6003*2" - nothing to translate it to

    bool threw = false;
    qmu::EErrorCodes caught_code = qmu::ecUNDEFINED;
    QString caught_token;
    qreal result = -1;
    try
    {
        Calculator cal;
        result = cal.EvalFormula(after->DataVariables(), displayed);
    }
    catch (qmu::QmuParserError &error)
    {
        threw = true;
        caught_code = error.GetCode();
        caught_token = error.GetToken();
    }

    QVERIFY2(threw, "Expected Calculator::EvalFormula to throw qmu::QmuParserError for a "
                     "dangling id token instead of crashing or silently returning a value");
    QCOMPARE(caught_code, qmu::ecUNASSIGNABLE_TOKEN);
    QCOMPARE(caught_token, QStringLiteral("id%1").arg(id_deleted));
    Q_UNUSED(result)
}

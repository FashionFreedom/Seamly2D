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

#include "tst_variableformulaidtranslation.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/calculator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/variables/custom_variable.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/qmuparser/qmuparsererror.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_VariableFormulaIdTranslation::TST_VariableFormulaIdTranslation(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A custom variable/increment formula (<variable formula="..."/>) is translated and read
 * back through the exact same FormulaIdTranslator/PatternFormulaTokens calls
 * VPattern::setVariableFormula/VPattern::parseVariablesElement use at the XML boundary - proving a
 * variable formula referencing a point survives a rename of that point, the same way a draft
 * tool's formula does. Before the fix, setVariableFormula/parseVariablesElement never called these
 * functions at all, so a renamed point left the stored formula as dead name-form text.
 */
void TST_VariableFormulaIdTranslation::TestCustomVariableFormulaSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3001;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1*2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1*2").arg(id1));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("Bundweite"), 5, 5));

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Bundweite*2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("id%1*2").arg(id1));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same proof, but referencing a composite/derived variable (line length) instead of a plain
 * point name - the most realistic shape for a real increment formula, since a bare point name isn't
 * a valid scalar on its own.
 */
void TST_VariableFormulaIdTranslation::TestCustomVariableFormulaReferencingLineLengthSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3002;
    const quint32 id2 = 3003;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("C5"), 5, 5));
    before->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("D1"), 5, 5));
    before->AddLine(id1, id2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_C5_D1/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1_id%2/2").arg(id1).arg(id2));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("C5qqq"), 5, 5));
    after->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("D1"), 5, 5));
    after->AddLine(id1, id2);

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Line_C5qqq_D1/2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("Line_id%1_id%2/2").arg(id1).arg(id2));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief A custom variable's formula can legitimately reference draft geometry by id-token - a line
 * length between two points, the same way TestCustomVariableFormulaReferencingLineLengthSurvivesRename
 * above proves for a rename (see #1678). But on disk <variables> always precedes every <draftBlock>
 * (VPattern::CreateEmptyFile() appends TagVariables immediately; AddDraftBlock::redo() only ever
 * appends a draft block onto a document that already has one), and VPattern::Parse() walks the DOM
 * in a single top-to-bottom pass in physical document order, so the single top-to-bottom walk parses
 * every <variables> element while `data` still holds none of the points/lines the draft blocks below
 * it define.
 *
 * This test reproduces exactly that moment: VPattern::parseVariablesElement() calling
 * formulaIdsToNames()/EvalFormula() against a `data` that has not yet seen the referenced line - the
 * same failure mode tst_danglingidtokenformula.cpp proves for an actually-deleted point, except here
 * the object is not deleted, only not-yet-parsed. formulaIdsToNames() cannot tell the two situations
 * apart: both look like an unknown token, so both are left untranslated.
 */
void TST_VariableFormulaIdTranslation::TestCustomVariableFormulaFailsWhenVariablesParseBeforeReferencedDraftGeometry()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3006;
    const quint32 id2 = 3007;

    // Step 1: VPattern::PrepareForParse() - data->ClearForFullParse() runs right before the walk.
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    // Step 2: <variables> is the first meaningful sibling in the file - parseVariablesElement()
    // runs while `data` still holds none of the points/line the draft block below will define. As
    // persisted on disk (VPattern::setVariableFormula -> formulaNamesToIds), a formula like
    // "Line_A1_A2/2" is stored as "Line_id<id1>_id<id2>/2" (see the previous test).
    const QString stored = QStringLiteral("Line_id%1_id%2/2").arg(id1).arg(id2);

    const QString formula = formulaIdsToNames(stored, idTokenToNameMap(data.data()));
    // id1/id2 are unknown to `data` at this point (not deleted - simply not parsed yet), so the
    // id-token is left exactly as stored, not translated to "Line_A1_A2/2".
    QCOMPARE(formula, stored);

    bool threw = false;
    qmu::EErrorCodes caughtCode = qmu::ecUNDEFINED;
    try
    {
        Calculator cal;
        cal.EvalFormula(data->DataVariables(), formula);
    }
    catch (qmu::QmuParserError &error)
    {
        threw = true;
        caughtCode = error.GetCode();
    }
    QVERIFY2(threw, "Expected evaluating the still-raw id-token formula against a VContainer that "
                     "has not parsed the referenced line yet to fail, exactly the way "
                     "VPattern::EvalFormula turns this into value=0, ok=false instead of the real, "
                     "correct line length.");
    QCOMPARE(caughtCode, qmu::ecUNASSIGNABLE_TOKEN);

    // This is exactly the (formula, ok) pair VPattern::parseVariablesElement would hand to
    // `new CustomVariable(...)` on a first, single-pass parse: the raw, untranslated formula and
    // ok=false - not "Line_A1_A2/2" and the real evaluated length.
    const CustomVariable variable(data.data(), QStringLiteral("#Var1"), 0, 0, formula, /*ok=*/false);
    QCOMPARE(variable.GetFormula(), stored);
    QVERIFY2(!variable.IsFormulaOk(),
             "A custom variable referencing not-yet-parsed draft geometry must not silently claim "
             "its formula evaluated correctly.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief The fix: VPattern::Parse() re-runs parseVariablesElement() for every <variables> element a
 * second time, once the single top-to-bottom walk over the whole document (and so every
 * <draftBlock>) has finished. VContainer::AddVariable() updates an existing entry matched by name in
 * place, so re-running is a correction pass, not a duplicate.
 *
 * This test picks up exactly where TestCustomVariableFormulaFailsWhenVariablesParseBeforeReferenced-
 * DraftGeometry left off: the draft block has now been parsed (data holds the line's two points),
 * and re-attempting translation+evaluation with the SAME stored formula must now produce the real
 * display name and the real evaluated length, proving the fix actually repairs the CustomVariable
 * the first pass left broken - not just that some translation function runs a second time.
 */
void TST_VariableFormulaIdTranslation::TestReparsingVariablesAfterDraftBlockFixesTranslationAndValue()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3008;
    const quint32 id2 = 3009;

    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    const QString stored = QStringLiteral("Line_id%1_id%2/2").arg(id1).arg(id2);

    // First pass (pre-fix behaviour, still runs unchanged): translation and evaluation both fail
    // because `data` is still empty of the referenced line.
    const QString firstPassFormula = formulaIdsToNames(stored, idTokenToNameMap(data.data()));
    QCOMPARE(firstPassFormula, stored);

    // The draft block is now parsed: two points on the X axis, and the line between them - exactly
    // VContainer::UpdateGObject()/AddLine() as called from VPattern::ParsePointElement()/
    // ParseToolEndLine() et al.
    data->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    data->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(id1, id2);

    Calculator lengthProbe;
    const qreal fullLineLength = lengthProbe.EvalFormula(data->DataVariables(), QStringLiteral("Line_A1_A2"));

    // Second pass (the fix): parseVariablesElement runs again against the SAME on-disk `stored`
    // formula, now with a complete `data`.
    const QString secondPassFormula = formulaIdsToNames(stored, idTokenToNameMap(data.data()));
    QCOMPARE(secondPassFormula, QStringLiteral("Line_A1_A2/2"));

    bool ok = true;
    qreal value = -1;
    try
    {
        Calculator cal;
        value = cal.EvalFormula(data->DataVariables(), secondPassFormula);
    }
    catch (qmu::QmuParserError &error)
    {
        ok = false;
        Q_UNUSED(error)
    }
    QVERIFY2(ok, "Expected the now-translated formula to evaluate cleanly against the fully "
                 "populated container.");
    QCOMPARE(value, fullLineLength / 2);

    // What VPattern::parseVariablesElement's second pass actually does: overwrite the CustomVariable
    // already registered under this name with the corrected formula/value/ok state.
    data->AddVariable(QStringLiteral("#Var1"),
                       new CustomVariable(data.data(), QStringLiteral("#Var1"), 0, value, secondPassFormula, ok));
    const CustomVariable fixedVariable = *data->getVariable<CustomVariable>(QStringLiteral("#Var1"));
    QCOMPARE(fixedVariable.GetFormula(), QStringLiteral("Line_A1_A2/2"));
    QVERIFY(fixedVariable.IsFormulaOk());
}

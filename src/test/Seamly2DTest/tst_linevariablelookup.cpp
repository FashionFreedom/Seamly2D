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

#include "tst_linevariablelookup.h"

#include <QtTest>
#include <utility>

#include "../../libs/vpatterndb/variables/vlinevariablelookup.h"
#include "../../libs/vpatterndb/variables/vlinelength.h"
#include "../../libs/vpatterndb/variables/vlineangle.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/vtranslatevars.h"
#include "../../libs/vpatterndb/calculator.h"
#include "../../libs/vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
TST_LineVariableLookup::TST_LineVariableLookup(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Mirrors what VContainer::AddLine() does when a draw tool registers its implicit line (see
 * issue #1678): two points plus a persisted line_id, then a lookup by that same line_id must find the
 * VLengthLine/VLineAngle AddLine() created for it - the same lookup VDrawTool::lineLengthName()/
 * lineAngleName() rely on to build "Copy Length"/"Copy Angle" clipboard text.
 */
void TST_LineVariableLookup::TestFindLineLengthReturnsMatchingLine()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line_id = 99;

    data->AddLine(p1_id, p2_id, line_id);

    const QSharedPointer<VLengthLine> found = findLineLength(*data, line_id);
    QVERIFY(!found.isNull());
    QCOMPARE(found->getLineId(), line_id);
    QCOMPARE(found->GetName(), QStringLiteral("Line_A1_A2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineAngleReturnsMatchingLine()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line_id = 99;

    data->AddLine(p1_id, p2_id, line_id);

    const QSharedPointer<VLineAngle> found = findLineAngle(*data, line_id);
    QVERIFY(!found.isNull());
    QCOMPARE(found->getLineId(), line_id);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineLengthReturnsNullForUnknownId()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 99);

    QVERIFY(findLineLength(*data, 12345).isNull());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineAngleReturnsNullForUnknownId()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 99);

    QVERIFY(findLineAngle(*data, 12345).isNull());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Point display names are not enforced globally unique across a whole pattern (issue
 * #1678), so two structurally different lines in unrelated draft blocks can legally share
 * endpoint names and therefore the identical "Line_<p1>_<p2>" display name.
 * VContainer::UniqueCompositeVariableName() disambiguates the second one's container key (e.g.
 * "Line_A1_A2_2"), but never updates the VLengthLine object's own internal name, which keeps
 * returning the original, undisambiguated string via GetName() - a real but DIFFERENT line's
 * name. "Copy Length" must use the name the line is actually registered under
 * (findLineLengthName()), not VLengthLine::GetName(), or it silently copies a formula that
 * resolves to the wrong line.
 */
void TST_LineVariableLookup::TestFindLineLengthNameReturnsRegisteredKeyNotObjectName()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 1);

    // A second, unrelated line whose endpoints happen to share names with the first line's.
    const quint32 p3_id = data->AddGObject(new VPointF(0, 20, QStringLiteral("A1"), 5, 5));
    const quint32 p4_id = data->AddGObject(new VPointF(10, 20, QStringLiteral("A2"), 5, 5));
    data->AddLine(p3_id, p4_id, 2);

    // Both objects report the same undisambiguated GetName() - using that for line 2 would name
    // line 1 instead.
    QCOMPARE(findLineLength(*data, 1)->GetName(), QStringLiteral("Line_A1_A2"));
    QCOMPARE(findLineLength(*data, 2)->GetName(), QStringLiteral("Line_A1_A2"));

    // The actually-registered, resolvable names differ.
    QCOMPARE(findLineLengthName(*data, 1), QStringLiteral("Line_A1_A2"));
    QCOMPARE(findLineLengthName(*data, 2), QStringLiteral("Line_A1_A2_2"));
    QVERIFY(data->lineLengthsData().contains(findLineLengthName(*data, 2)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineAngleNameReturnsRegisteredKeyNotObjectName()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 1);

    const quint32 p3_id = data->AddGObject(new VPointF(0, 20, QStringLiteral("A1"), 5, 5));
    const quint32 p4_id = data->AddGObject(new VPointF(10, 20, QStringLiteral("A2"), 5, 5));
    data->AddLine(p3_id, p4_id, 2);

    QCOMPARE(findLineAngle(*data, 1)->GetName(), QStringLiteral("AngleLine_A1_A2"));
    QCOMPARE(findLineAngle(*data, 2)->GetName(), QStringLiteral("AngleLine_A1_A2"));

    QCOMPARE(findLineAngleName(*data, 1), QStringLiteral("AngleLine_A1_A2"));
    QCOMPARE(findLineAngleName(*data, 2), QStringLiteral("AngleLine_A1_A2_2"));
    QVERIFY(data->lineAnglesData().contains(findLineAngleName(*data, 2)));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief The two tests above only prove findLineLengthName()/findLineAngleName() return the
 * correctly-disambiguated *string*. That alone doesn't prove "Copy Length" actually produces a
 * usable, correct formula - the two colliding lines in those tests happen to have the same
 * real length (both 10cm), so evaluating the wrong name would have silently returned the
 * *coincidentally* right number and this whole bug class would have gone unnoticed.
 *
 * This test closes that gap: two colliding lines with genuinely DIFFERENT real lengths (10cm
 * vs. 30cm). It takes the exact string "Copy Length" would put on the clipboard for the second
 * line, evaluates it through Calculator::EvalFormula() against the same VContainer::DataVariables()
 * a pasted formula is evaluated against (the same call VFormula::Eval() makes), and checks the
 * *number* that comes back - not just the name.
 */
void TST_LineVariableLookup::TestCopiedLineLengthFormulaEvaluatesToTheCorrectLineNotTheCollider()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    // Line 1: A1(0,0) to A2(10,0) - length 10cm.
    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 1);

    // Line 2: unrelated points that happen to share the same names "A1"/"A2", but placed so the
    // line is genuinely a different length - 30cm, not a coincidental match with line 1.
    const quint32 p3_id = data->AddGObject(new VPointF(0, 20, QStringLiteral("A1"), 5, 5));
    const quint32 p4_id = data->AddGObject(new VPointF(0, 50, QStringLiteral("A2"), 5, 5));
    data->AddLine(p3_id, p4_id, 2);

    // VInternalVariable::GetValue() has a const qreal-returning overload and a non-const
    // qreal*-returning one; QSharedPointer's operator-> is non-const here, so force the const
    // overload explicitly rather than dereferencing the pointer form.
    const qreal line1_value = std::as_const(*findLineLength(*data, 1)).GetValue();
    const qreal line2_value = std::as_const(*findLineLength(*data, 2)).GetValue();
    QVERIFY2(!qFuzzyCompare(line1_value, line2_value),
             "Test setup error: the two lines must have different real lengths, or a bug that "
             "evaluates to the wrong line's length would go unnoticed.");

    // What "Copy Length" would actually put on the clipboard for line 2.
    const QString copied_formula = findLineLengthName(*data, 2);
    QCOMPARE(copied_formula, QStringLiteral("Line_A1_A2_2"));

    // Evaluate it exactly the way a pasted formula is evaluated (VFormula::Eval's own call).
    Calculator cal;
    const qreal evaluated_value = cal.EvalFormula(data->DataVariables(), copied_formula);

    QCOMPARE(evaluated_value, line2_value);
    QVERIFY2(!qFuzzyCompare(evaluated_value, line1_value),
             "Copy Length for line 2 evaluated to line 1's length - the exact silent-wrong-value "
             "bug this fix addresses.");
}

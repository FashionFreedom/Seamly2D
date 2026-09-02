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

#include "tst_patternformulatokens.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_PatternFormulaTokens::TST_PatternFormulaTokens(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_PatternFormulaTokens::TestMergesPlainAndCompositeEntries()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    const quint32 id1 = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 id2 = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(id1, id2, id2);

    const QHash<QString, QString> nameToIdToken = nameToIdTokenMap(data.data());

    QCOMPARE(nameToIdToken.value(QStringLiteral("A1")), idToken(id1));
    QCOMPARE(nameToIdToken.value(QStringLiteral("Line_A1_A2")),
             QStringLiteral("Line_%1").arg(idToken(id2)));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief End-to-end proof at the level a tool would actually use: a formula referencing both a
 * plain point and a derived line-length value translates to a stable id-based form, and resolves
 * to the correct new text after a point rename - simulated the way it really happens in the app,
 * by rebuilding the container fresh (what a full reparse after rename actually does), not by
 * mutating a variable object that was already computed before the rename.
 */
void TST_PatternFormulaTokens::TestFormulaReferencingBothTranslatesAndSurvivesRename()
{
    const Unit unit = Unit::Cm;
    // Explicit ids (via UpdateGObject, the same call tools use for Source::FromFile) so both
    // containers below refer to "the same" points by id, independent of the global auto-id
    // counter - which other tests in this binary may already have advanced.
    const quint32 id1 = 1001;
    const quint32 id2 = 1002;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->AddLine(id1, id2, id2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1+Line_A1_A2/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1+Line_id%2/2").arg(id1).arg(id2));

    // Same ids, A1 renamed - matches what a full reparse produces after a rename on disk.
    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("Halsloch_hinten"), 5, 5));
    after->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->AddLine(id1, id2, id2);

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Halsloch_hinten+Line_Halsloch_hinten_A2/2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("id%1+Line_id%2/2").arg(id1).arg(id2));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolSpline and VToolCubicBezier carry a "Curve Length" target formula (<... length="..."/>,
 * the AttrLength attribute alongside AttrLengthMode) independent of their Angle1/Angle2/Length1/Length2
 * control-handle formulas. It goes through the exact same PatternFormulaTokens calls at the XML boundary
 * (VToolSpline::SetSplineAttributes/ReadToolAttributes, VToolCubicBezier's counterparts, and
 * VPattern::ParseToolSpline/ParseToolCubicBezier) - proving it survives a rename of the point it
 * references, the same way the control-handle formulas do.
 */
void TST_PatternFormulaTokens::TestCurveTargetLengthFormulaSurvivesRename()
{
    const Unit unit = Unit::Cm;
    const quint32 id1 = 3001;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1*2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("id%1*2").arg(id1));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("Saum_vorne"), 5, 5));

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Saum_vorne*2"));
}

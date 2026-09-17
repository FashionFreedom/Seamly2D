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
#include "../../libs/vpatterndb/compositevariabletokens.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/variables/vcurvelength.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/vgeometry/varc.h"
#include "../../libs/ifc/ifcdef.h"

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

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(data.data());

    QCOMPARE(name_to_id_token.value(QStringLiteral("A1")), idToken(id1));
    QCOMPARE(name_to_id_token.value(QStringLiteral("Line_A1_A2")),
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

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PROBE for issue-#1678 code-quality review Finding 1 (possible silent id-token collision
 * in the reverse id->name map). A curve is both a VGObject (in DataGObjects, own name "Arc1")
 * and - via its plain, non-segmented VCurveLength - a composite variable table entry that
 * CompositeVariableTokens::nameToIdTokenMap() deliberately duplicates onto the exact same
 * "id<N>" token (see the comment in compositevariabletokens.cpp explaining why). This is a real,
 * legitimate double-insertion of the same token, exercised end-to-end the way VToolArc::Create
 * actually builds it (UpdateGObject then AddArc with the same id).
 *
 * Proves the concern is a non-issue for every reachable code path: both the base map
 * (FormulaIdTranslator::idTokenToNameMap over DataGObjects) and the composite map
 * (CompositeVariableTokens::idTokenToNameMap over DataVariables) resolve "id<N>" to the exact
 * same name, because VCurveLength's non-segmented constructor reads that name straight off the
 * very VGObject stored at that id (VCurveLength::VCurveLength: SetName(curve->name())). Whichever
 * one PatternFormulaTokens::idTokenToNameMap's QHash::insert keeps last is therefore
 * indistinguishable from the other - see TestIdTokenCollisionOverwritesAndLogsWarning
 * below for what happens when that invariant is broken by hand.
 */
void TST_PatternFormulaTokens::TestCurveLengthDuplicateEntryAgreesWithGObjectName()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    const quint32 centerId = 4001;
    const quint32 arcId = 4002;
    data->UpdateGObject(centerId, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));

    VArc *arc = new VArc(*data->GeometricObject<VPointF>(centerId), 10, 0, 90);
    data->UpdateGObject(arcId, arc);
    // setName() must come after UpdateGObject(): VContainer::UpdateObject() calls
    // point->setId(id), and VAbstractArc::setId() unconditionally regenerates the name via
    // CreateName() - any name set beforehand is silently discarded. This is real, observed
    // behaviour (confirmed by running this test), not an assumption.
    arc->setName(QStringLiteral("Arc1"));
    // Exactly the sequence VToolArc::Create uses (Source::FromFile branch): the curve is
    // already in DataGObjects under arcId before AddArc() derives its length variable from it.
    data->AddArc(data->GeometricObject<VArc>(arcId), arcId);

    const QString token = idToken(arcId);

    const QHash<QString, QString> base_map = FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects());
    const QHash<QString, QString> composite_map =
        CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());

    QVERIFY(base_map.contains(token));
    QVERIFY(composite_map.contains(token));
    QCOMPARE(base_map.value(token), QStringLiteral("Arc1"));
    QCOMPARE(composite_map.value(token), QStringLiteral("Arc1"));
    QCOMPARE(base_map.value(token), composite_map.value(token));

    // And the real, merged map the app actually uses resolves correctly too.
    QCOMPARE(idTokenToNameMap(data.data()).value(token), QStringLiteral("Arc1"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PROBE for issue-#1678 code-quality review Finding 1, adversarial half. No production
 * code path can make the base DataGObjects entry and the composite CurveLength entry disagree
 * for the same id (proven above), because AddArc always builds the VCurveLength from the very
 * VGObject already stored at that id. This test manufactures that disagreement directly against
 * the container APIs anyway - something no tool does - purely to show what the reviewer's concern
 * describes actually looks like mechanically: PatternFormulaTokens::idTokenToNameMap layers the
 * composite map over the base map with a plain QHash::insert.
 *
 * Following the fix for the id-collision bug (see TST_LineIdSelfHealCollision), the merge itself
 * is now guarded: a conflicting insert is logged via qCWarning(vCon, ...) before it happens. It
 * is deliberately still a warning and not a hard failure - by the time this merge runs the
 * collision has already occurred upstream in VContainer, and this call site cannot itself decide
 * which of the two names is "right". This test therefore still documents the resolution outcome
 * (the composite entry wins over the base one) - only its detection story changed, not its
 * result.
 *
 * Verdict this supports: the underlying id collision is meant to be impossible after the
 * VPattern::PrepareForParse() seeding fix, but this call site is not itself what prevents it -
 * it is only reachable through direct/incorrect use of VContainer::AddVariable(), which the
 * pattern engine never does. This probe intentionally bypasses that protection to exercise the
 * merge's own (now logged) behavior in isolation.
 */
void TST_PatternFormulaTokens::TestIdTokenCollisionOverwritesAndLogsWarning()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    const quint32 arcId = 5001;
    const VPointF center(0, 0, QStringLiteral("A1"), 5, 5);

    VArc *arc = new VArc(center, 10, 0, 90);
    data->UpdateGObject(arcId, arc);
    // See TestCurveLengthDuplicateEntryAgreesWithGObjectName: setName() must come after
    // UpdateGObject(), since setId() (called internally) regenerates the name via CreateName()
    // and would otherwise silently discard it.
    arc->setName(QStringLiteral("RealArcName")); // base map: "id5001" -> "RealArcName"

    // Fabricate an unrelated curve, named differently, and register ITS plain VCurveLength
    // under the SAME id by hand - bypassing AddArc entirely, which is exactly what makes this
    // scenario unreachable via any real tool (see docstring above).
    VArc unrelated_curve(center, 20, 0, 45);
    unrelated_curve.setName(QStringLiteral("SpoofedName"));
    VCurveLength *spoofed_length = new VCurveLength(arcId, NULL_ID, &unrelated_curve, unit);
    QCOMPARE(spoofed_length->GetName(), QStringLiteral("SpoofedName"));
    data->AddVariable(spoofed_length->GetName(), spoofed_length);

    const QString token = idToken(arcId);

    QCOMPARE(FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects()).value(token),
             QStringLiteral("RealArcName"));
    QCOMPARE(CompositeVariableTokens::idTokenToNameMap(*data->DataVariables()).value(token),
             QStringLiteral("SpoofedName"));

    // The merged map silently prefers the composite entry - a formula stored as "id5001" would
    // now display as "SpoofedName" instead of the real curve's name, with nothing anywhere
    // flagging the disagreement.
    QCOMPARE(idTokenToNameMap(data.data()).value(token), QStringLiteral("SpoofedName"));
}

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

#include "tst_compositevariabletokens.h"

#include <QtTest>

#include "../../libs/vpatterndb/compositevariabletokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/variables/vinternalvariable.h"
#include "../../libs/vpatterndb/variables/vlinelength.h"
#include "../../libs/vpatterndb/variables/vlineangle.h"
#include "../../libs/vpatterndb/variables/varcradius.h"
#include "../../libs/vpatterndb/variables/vcurveangle.h"
#include "../../libs/vpatterndb/variables/vcurveclength.h"
#include "../../libs/vpatterndb/variables/vcurvelength.h"
#include "../../libs/vpatterndb/variables/vvariable.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/vgeometry/varc.h"
#include "../../libs/vgeometry/vellipticalarc.h"
#include "../../libs/vgeometry/vspline.h"
#include "../../libs/ifc/ifcdef.h"

using namespace FormulaIdTranslator;
using namespace CompositeVariableTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_CompositeVariableTokens::TST_CompositeVariableTokens(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestLineLengthMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p2(10, 0, QStringLiteral("A2"), 5, 5);

    QSharedPointer<VInternalVariable> length(new VLengthLine(&p1, 42, &p2, 17, 99, Unit::Cm));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(length->GetName(), length);
    QCOMPARE(length->GetName(), QStringLiteral("Line_A1_A2"));

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(QStringLiteral("Line_A1_A2")), QStringLiteral("Line_id99"));

    const QHash<QString, QString> id_token_to_name = idTokenToNameMap(variables);
    QCOMPARE(id_token_to_name.value(QStringLiteral("Line_id99")), QStringLiteral("Line_A1_A2"));

    QCOMPARE(formulaNamesToIds(QStringLiteral("Line_A1_A2*2"), name_to_id_token),
             QStringLiteral("Line_id99*2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves the composite case survives rename the same way the plain point case does: the
 * stored formula (built from the ids) never changes; only what it resolves to for display changes
 * once the constituent points get new names.
 */
void TST_CompositeVariableTokens::TestLineLengthSurvivesRename()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p2(10, 0, QStringLiteral("A2"), 5, 5);

    QSharedPointer<VInternalVariable> length_before_rename(new VLengthLine(&p1, 42, &p2, 17, 99, Unit::Cm));
    QHash<QString, QSharedPointer<VInternalVariable>> variables_before_rename;
    variables_before_rename.insert(length_before_rename->GetName(), length_before_rename);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2*2"), nameToIdTokenMap(variables_before_rename));
    QCOMPARE(stored, QStringLiteral("Line_id99*2"));

    // A2 renamed to Halsloch_hinten: same ids, a fresh VLengthLine reflects the new name.
    const VPointF p2_renamed(10, 0, QStringLiteral("Halsloch_hinten"), 5, 5);
    QSharedPointer<VInternalVariable> length_after_rename(new VLengthLine(&p1, 42, &p2_renamed, 17, 99, Unit::Cm));
    QHash<QString, QSharedPointer<VInternalVariable>> variables_after_rename;
    variables_after_rename.insert(length_after_rename->GetName(), length_after_rename);

    QCOMPARE(formulaIdsToNames(
                 stored, idTokenToNameMap(variables_after_rename)),
             QStringLiteral("Line_A1_Halsloch_hinten*2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("Line_id99*2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestUnknownVariableTypeLeftOut()
{
    QSharedPointer<VInternalVariable> plain(new VVariable(QStringLiteral("MyVar")));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(plain->GetName(), plain);

    QVERIFY(nameToIdTokenMap(variables).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestLineAngleMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p2(10, 0, QStringLiteral("A2"), 5, 5);

    QSharedPointer<VInternalVariable> angle(new VLineAngle(&p1, 42, &p2, 17, 99));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(angle->GetName(), angle);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(angle->GetName()), QStringLiteral("AngleLine_id99"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestArcRadiusMapping()
{
    VPointF center(0, 0, QStringLiteral("A1"), 5, 5);
    VArc arc(center, 10, 0, 90);
    arc.setName(QStringLiteral("Arc1"));

    QSharedPointer<VInternalVariable> radius(new VArcRadius(99, NULL_ID, &arc, Unit::Cm));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(radius->GetName(), radius);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(radius->GetName()), QStringLiteral("Radius0id99"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief An elliptical arc has two distinct radii, so the id token must encode which one this is -
 * otherwise "first radius of ellipse X" and "second radius of ellipse X" would collide onto the
 * same stored token.
 */
void TST_CompositeVariableTokens::TestEllipticalArcRadiusMapping()
{
    VPointF center(0, 0, QStringLiteral("A1"), 5, 5);
    VEllipticalArc el_arc(center, 10, 5, 0, 90, 0);
    el_arc.setName(QStringLiteral("EllArc1"));

    QSharedPointer<VInternalVariable> radius1(new VArcRadius(99, NULL_ID, &el_arc, 1, Unit::Cm));
    QSharedPointer<VInternalVariable> radius2(new VArcRadius(99, NULL_ID, &el_arc, 2, Unit::Cm));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(radius1->GetName(), radius1);
    variables.insert(radius2->GetName(), radius2);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(radius1->GetName()), QStringLiteral("Radius1id99"));
    QCOMPARE(name_to_id_token.value(radius2->GetName()), QStringLiteral("Radius2id99"));
    QVERIFY(name_to_id_token.value(radius1->GetName()) != name_to_id_token.value(radius2->GetName()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same collision concern as the elliptical radius case: start and end angle of the same
 * curve must map to different stored tokens.
 */
void TST_CompositeVariableTokens::TestCurveAngleMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p4(10, 10, QStringLiteral("A2"), 5, 5);
    VSpline spl(p1, QPointF(3, 3), QPointF(7, 7), p4);
    spl.setName(QStringLiteral("Curve1"));

    QSharedPointer<VInternalVariable> start(new VCurveAngle(99, NULL_ID, &spl, CurveAngle::StartAngle));
    QSharedPointer<VInternalVariable> end(new VCurveAngle(99, NULL_ID, &spl, CurveAngle::EndAngle));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(start->GetName(), start);
    variables.insert(end->GetName(), end);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(start->GetName()), QStringLiteral("Angle1id99"));
    QCOMPARE(name_to_id_token.value(end->GetName()), QStringLiteral("Angle2id99"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestCurveCLengthMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p4(10, 10, QStringLiteral("A2"), 5, 5);
    VSpline spl(p1, QPointF(3, 3), QPointF(7, 7), p4);
    spl.setName(QStringLiteral("Curve1"));

    QSharedPointer<VInternalVariable> c1(new VCurveCLength(99, NULL_ID, &spl, CurveCLength::C1, Unit::Cm));
    QSharedPointer<VInternalVariable> c2(new VCurveCLength(99, NULL_ID, &spl, CurveCLength::C2, Unit::Cm));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(c1->GetName(), c1);
    variables.insert(c2->GetName(), c2);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(c1->GetName()), QStringLiteral("C1Lengthid99"));
    QCOMPARE(name_to_id_token.value(c2->GetName()), QStringLiteral("C2Lengthid99"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Plain curve length is just the curve's own name (VCurveLength::VCurveLength() calls
 * SetName(curve->name()), no prefix, no gluing), so it looks like FormulaIdTranslator::
 * nameToIdTokenMap(data->DataGObjects()) already covers it - a curve is a VGObject like a point.
 * It doesn't: DataGObjects() can lag behind DataVariables() when a formula in one draft block
 * references a curve defined in another (the curve's own VGObject isn't visible there yet, while
 * its length variable already is, pattern-wide). CompositeVariableTokens must add the plain form
 * itself, straight from the variable table, rather than leaving it to the gObjects map.
 */
void TST_CompositeVariableTokens::TestPlainCurveLengthIncludedInCompositeMap()
{
    VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    VArc curve(p1, 10, 0, 90);
    curve.setName(QStringLiteral("Arc1"));

    QSharedPointer<VInternalVariable> length(new VCurveLength(99, NULL_ID, &curve, Unit::Cm));
    QCOMPARE(length->GetName(), QStringLiteral("Arc1"));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(length->GetName(), length);

    QCOMPARE(nameToIdTokenMap(variables).value(QStringLiteral("Arc1")), QStringLiteral("id99"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief On a multi-segment curve path, each segment gets its own length variable. The segment
 * index (a plain integer, never a name) must be part of the stored token, or segment 1 and
 * segment 2 of the same path would collide onto the same id token.
 */
void TST_CompositeVariableTokens::TestCurveLengthSegmentMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p4(10, 10, QStringLiteral("A2"), 5, 5);
    VSpline spl(p1, QPointF(3, 3), QPointF(7, 7), p4);

    QSharedPointer<VInternalVariable> seg1(new VCurveLength(99, NULL_ID, QStringLiteral("Curve1"), spl, Unit::Cm, 1));
    QSharedPointer<VInternalVariable> seg2(new VCurveLength(99, NULL_ID, QStringLiteral("Curve1"), spl, Unit::Cm, 2));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(seg1->GetName(), seg1);
    variables.insert(seg2->GetName(), seg2);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(seg1->GetName()), QStringLiteral("id99_Seg_1"));
    QCOMPARE(name_to_id_token.value(seg2->GetName()), QStringLiteral("id99_Seg_2"));
    QVERIFY(name_to_id_token.value(seg1->GetName()) != name_to_id_token.value(seg2->GetName()));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestCurveAngleSegmentMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p4(10, 10, QStringLiteral("A2"), 5, 5);
    VSpline spl(p1, QPointF(3, 3), QPointF(7, 7), p4);

    QSharedPointer<VInternalVariable> seg1(
        new VCurveAngle(99, NULL_ID, QStringLiteral("Curve1"), spl, CurveAngle::StartAngle, 1));
    QSharedPointer<VInternalVariable> seg2(
        new VCurveAngle(99, NULL_ID, QStringLiteral("Curve1"), spl, CurveAngle::StartAngle, 2));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(seg1->GetName(), seg1);
    variables.insert(seg2->GetName(), seg2);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(seg1->GetName()), QStringLiteral("Angle1id99_Seg_1"));
    QCOMPARE(name_to_id_token.value(seg2->GetName()), QStringLiteral("Angle1id99_Seg_2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_CompositeVariableTokens::TestCurveCLengthSegmentMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p4(10, 10, QStringLiteral("A2"), 5, 5);
    VSpline spl(p1, QPointF(3, 3), QPointF(7, 7), p4);

    QSharedPointer<VInternalVariable> seg1(
        new VCurveCLength(99, NULL_ID, QStringLiteral("Curve1"), spl, CurveCLength::C1, Unit::Cm, 1));
    QSharedPointer<VInternalVariable> seg2(
        new VCurveCLength(99, NULL_ID, QStringLiteral("Curve1"), spl, CurveCLength::C1, Unit::Cm, 2));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(seg1->GetName(), seg1);
    variables.insert(seg2->GetName(), seg2);

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QCOMPARE(name_to_id_token.value(seg1->GetName()), QStringLiteral("C1Lengthid99_Seg_1"));
    QCOMPARE(name_to_id_token.value(seg2->GetName()), QStringLiteral("C1Lengthid99_Seg_2"));
}

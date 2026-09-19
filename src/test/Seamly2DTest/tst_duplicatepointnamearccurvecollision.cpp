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

#include "tst_duplicatepointnamearccurvecollision.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/compositevariabletokens.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vabstractcurve.h"
#include "../../libs/vgeometry/vabstractbezier.h"
#include "../../libs/vgeometry/varc.h"
#include "../../libs/vgeometry/vspline.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/ifc/ifcdef.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_DuplicatePointNameArcCurveCollision::TST_DuplicatePointNameArcCurveCollision(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Two entirely unrelated arcs whose center points happen to share the same display name "A" -
 * point names have never been enforced globally unique (see issue #1678). VArc::CreateName() builds
 * an arc's own name purely from its center point's name ("Arc_A" for both arcs here), so
 * VArcRadius::GetName() ("RadiusArc_A") collides for both arcs too.
 *
 * Before the fix, VContainer::AddVariable()'s "name already exists -> update the existing object in
 * place" semantics would let the second arc's AddArc() call silently splice its own id onto the first
 * arc's radius variable name, discarding the first arc's id-token from the variable table entirely.
 * This test proves VContainer::UniqueCompositeVariableName (vcontainer.cpp) now disambiguates the
 * second registration instead, so both arcs' radii stay independently resolvable.
 */
void TST_DuplicatePointNameArcCurveCollision::TestDuplicateCenterPointNamesDoNotCollideAcrossUnrelatedArcs()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    const VPointF centerFirst(0, 0, QStringLiteral("A"), 5, 5);
    QSharedPointer<VAbstractCurve> arcFirst(new VArc(centerFirst, 10, 0, 90));
    const quint32 arcIdFirst = 500;
    data->AddArc(arcFirst, arcIdFirst);

    // A completely unrelated arc, elsewhere in the pattern, whose center point merely happens to
    // share the display name "A" - generating the exact same "RadiusArc_A" variable name.
    const VPointF centerSecond(1000, 0, QStringLiteral("A"), 5, 5);
    QSharedPointer<VAbstractCurve> arcSecond(new VArc(centerSecond, 20, 0, 90));
    const quint32 arcIdSecond = 600;
    data->AddArc(arcSecond, arcIdSecond);

    const QString firstToken  = radius_V + QStringLiteral("0") + idToken(arcIdFirst);
    const QString secondToken = radius_V + QStringLiteral("0") + idToken(arcIdSecond);

    const QHash<QString, QString> idToName =
        CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());

    QVERIFY2(idToName.contains(firstToken),
             "The first arc's radius id-token must still resolve to a name after a second, "
             "unrelated arc with the same generated display name is registered.");
    QVERIFY2(idToName.contains(secondToken),
             "The second arc's radius id-token must resolve to its own name, distinct from the "
             "first arc's, even though both arcs' center points share the same display name.");
    QVERIFY2(idToName.value(firstToken) != idToName.value(secondToken),
             "Two structurally different arcs must never resolve their radius variable to the "
             "same display name.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Two entirely unrelated curves whose endpoints happen to share display names with another
 * pair of points elsewhere in the pattern (points "A" and "A2" duplicated across draft blocks, same
 * scenario as issue #1678's jacket1_52-176.sm2d sample). VAbstractCubicBezier::CreateName() builds a
 * curve's own name purely from its P1/P4 endpoint names ("Spl_A_A2" for both curves here), so
 * VCurveLength::GetName() collides for both curves too.
 *
 * Before the fix, the second curve's AddSpline() call would silently splice its own id onto the
 * first curve's length variable name, discarding the first curve's id-token from the variable table
 * entirely. This test proves VContainer::UniqueCompositeVariableName (vcontainer.cpp) now
 * disambiguates the second registration instead, so both curves stay independently resolvable.
 */
void TST_DuplicatePointNameArcCurveCollision::TestDuplicateEndpointNamesDoNotCollideAcrossUnrelatedCurves()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    const VPointF p1First(0, 0, QStringLiteral("A"), 5, 5);
    const VPointF p4First(10, 0, QStringLiteral("A2"), 5, 5);
    QSharedPointer<VAbstractBezier> curveFirst(new VSpline(p1First, p4First, 0, 90, 1, 1, 1));
    const quint32 curveIdFirst = 700;
    data->AddSpline(curveFirst, curveIdFirst);

    // A completely unrelated curve, elsewhere in the pattern, whose endpoints merely happen to
    // share display names with the first curve's - generating the exact same "Spl_A_A2" name.
    const VPointF p1Second(1000, 0, QStringLiteral("A"), 5, 5);
    const VPointF p4Second(1010, 0, QStringLiteral("A2"), 5, 5);
    QSharedPointer<VAbstractBezier> curveSecond(new VSpline(p1Second, p4Second, 0, 90, 1, 1, 1));
    const quint32 curveIdSecond = 800;
    data->AddSpline(curveSecond, curveIdSecond);

    const QString firstToken  = idToken(curveIdFirst);
    const QString secondToken = idToken(curveIdSecond);

    const QHash<QString, QString> idToName =
        CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());

    QVERIFY2(idToName.contains(firstToken),
             "The first curve's length id-token must still resolve to a name after a second, "
             "unrelated curve with the same generated display name is registered.");
    QVERIFY2(idToName.contains(secondToken),
             "The second curve's length id-token must resolve to its own name, distinct from the "
             "first curve's, even though both curves' endpoints share the same display names.");
    QVERIFY2(idToName.value(firstToken) != idToName.value(secondToken),
             "Two structurally different curves must never resolve their length variable to the "
             "same display name.");
}

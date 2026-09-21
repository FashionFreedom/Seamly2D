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

#include "tst_emptypathsplinenamecollision.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vabstractcubicbezierpath.h"
#include "../../libs/vgeometry/vsplinepath.h"
#include "../../libs/ifc/ifcdef.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_EmptyPathSplineNameCollision::TST_EmptyPathSplineNameCollision(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reproduces, at the VContainer level, exactly the sequence VToolSplinePath::Create()
 * (src/libs/vtools/tools/drawTools/toolcurve/vtoolsplinepath.cpp) runs for a pathInteractive spline
 * with zero path points: VContainer::AddGObject() assigns the curve its real id, then
 * VContainer::AddCurveWithSegments() registers its composite variables. Before the fix, the curve's
 * own name stayed empty throughout (VSplinePath's point-vector constructor never calls CreateName()
 * at all for an empty vector, and CreateName() itself produced an empty string for zero points even
 * when called) - so this object's VCurveLength/VCurveAngle entries were filed under an empty name.
 */
void TST_EmptyPathSplineNameCollision::TestEmptyPathSplineGetsNonEmptyName()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    VSplinePath *emptyPath = new VSplinePath();
    QCOMPARE(emptyPath->name(), QString());

    const quint32 id = data->AddGObject(emptyPath);
    data->AddCurveWithSegments(data->GeometricObject<VAbstractCubicBezierPath>(id), id);

    const QSharedPointer<VAbstractCubicBezierPath> registered =
        data->GeometricObject<VAbstractCubicBezierPath>(id);
    QVERIFY2(!registered->name().isEmpty(),
             "An empty path spline must still be given a real, non-empty display name once it has "
             "been registered with its final container id.");

    const QString token = idToken(id);
    QCOMPARE(idTokenToNameMap(data.data()).value(token), registered->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Continuation of the test above: before the fix, two such empty paths both registered their
 * length/angle variables under the identical empty name, so the second one's registration silently
 * discarded the first one's id-token from the variable table - the exact "Defekte Formel" symptom
 * already fixed for id and name collisions elsewhere in issue #1678, here caused by an empty name
 * instead. This proves both stay independently resolvable after the fix.
 */
void TST_EmptyPathSplineNameCollision::TestTwoEmptyPathSplinesDoNotCollide()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    VSplinePath *first = new VSplinePath();
    const quint32 firstId = data->AddGObject(first);
    data->AddCurveWithSegments(data->GeometricObject<VAbstractCubicBezierPath>(firstId), firstId);

    VSplinePath *second = new VSplinePath();
    const quint32 secondId = data->AddGObject(second);
    data->AddCurveWithSegments(data->GeometricObject<VAbstractCubicBezierPath>(secondId), secondId);

    const QString firstToken = idToken(firstId);
    const QString secondToken = idToken(secondId);

    const QHash<QString, QString> idToName = idTokenToNameMap(data.data());
    QVERIFY2(idToName.contains(firstToken),
             "The first empty path's id-token must still resolve to a name after a second, "
             "unrelated empty path is registered.");
    QVERIFY2(idToName.contains(secondToken),
             "The second empty path's id-token must resolve to its own name, distinct from the "
             "first path's.");
    QVERIFY2(idToName.value(firstToken) != idToName.value(secondToken),
             "Two different empty-path curves must never resolve to the identical display name.");
}

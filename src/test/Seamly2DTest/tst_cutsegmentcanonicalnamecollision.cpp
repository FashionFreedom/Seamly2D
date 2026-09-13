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

#include "tst_cutsegmentcanonicalnamecollision.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/compositevariabletokens.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vabstractcurve.h"
#include "../../libs/vgeometry/varc.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/ifc/ifcdef.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_CutSegmentCanonicalNameCollision::TST_CutSegmentCanonicalNameCollision(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Investigating whether the "canonical vs. squatter" eviction fix in
 * VContainer::UniqueCompositeVariableName (commit 816784f838, issue-#1678 / PR #1681) needs its own
 * dedicated proof for VarType::ArcRadius - the one composite type its own test coverage did not
 * touch - turned up a stronger guarantee than expected: for arcs specifically, that scenario cannot
 * actually arise in the first place.
 *
 * VAbstractArc::setId() (vabstractarc.cpp) unconditionally calls CreateName() every time a real
 * container id is assigned - confirmed directly here: forcing an identical name onto two arcs via
 * setName() before registering the second one as a canonical VGObject (VContainer::UpdateGObject())
 * does not survive - the canonical arc's name is regenerated from its own center point and its own
 * (necessarily different) id the moment it gets one, so two arcs that are both genuinely registered
 * as canonical objects can never end up sharing an identical composite name to begin with. Curves
 * and splines have no equivalent safeguard (VAbstractCubicBezier::CreateName() builds a name purely
 * from endpoint names, with no id fallback) - which is exactly why the real bug this fix addresses
 * (see project history, keiko_skirt.sm2d / zigzag.sm2d) only ever manifested for curves, not arcs.
 *
 * This test locks in that guarantee rather than forcing an artificial arc collision that cannot
 * occur through the class's real lifecycle - so that if VAbstractArc::setId()'s CreateName() call is
 * ever removed without realizing its role here, this fails loudly instead of silently reopening the
 * same collision class for arcs that was fixed for lines and curves.
 */
void TST_CutSegmentCanonicalNameCollision::TestCanonicalArcKeepsPlainNameOverCutSegmentSquatter()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    const quint32 canonicalId = 950;
    const QString forcedSharedName = QStringLiteral("Arc_TestCollision");

    const VPointF canonicalCenter(1000, 0, QStringLiteral("A"), 5, 5);
    QSharedPointer<VAbstractCurve> canonicalArc(new VArc(canonicalCenter, 20, 0, 90));

    // Deliberately force a name that has nothing to do with this arc's real identity, to prove it
    // cannot survive registration as a canonical object - not to simulate a realistic authoring path.
    canonicalArc->setName(forcedSharedName);
    QCOMPARE(canonicalArc->name(), forcedSharedName);

    data->UpdateGObject(canonicalId, canonicalArc);

    QVERIFY2(canonicalArc->name() != forcedSharedName,
             "VAbstractArc::setId() must regenerate the arc's name from its own identity as soon as "
             "it becomes a real, id-bearing container object, discarding any name set before "
             "registration - this is what makes a name collision between two canonical arcs "
             "structurally impossible, unlike for curves.");
    QVERIFY2(canonicalArc->name().endsWith(QLatin1Char('_') + QString::number(canonicalId)),
             "The regenerated name must end with the arc's own real id, which is what guarantees "
             "uniqueness against every other arc's name once both have real, distinct ids.");

    data->AddArc(canonicalArc, canonicalId);

    const QString canonicalToken = radius_V + QStringLiteral("0") + idToken(canonicalId);
    const QHash<QString, QString> idToName =
        CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());

    QVERIFY2(idToName.contains(canonicalToken),
             "The canonical arc's radius id-token must resolve to a name.");
    QCOMPARE(idToName.value(canonicalToken), radius_V + canonicalArc->name());
}

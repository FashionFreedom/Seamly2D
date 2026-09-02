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

#include "tst_customvariablecontainer.h"

#include <QtTest>

#include "../../libs/vpatterndb/variables/custom_variable.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
TST_CustomVariableContainer::TST_CustomVariableContainer(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reproduces the bug found while manually testing issue #1678's rename feature: parsing the
 * <variables> XML section - which comes before the draft block in the file - used to construct
 * every CustomVariable against a VALUE COPY of the container taken at that early, incomplete
 * moment (custom_variable_p.h's old "VContainer data;" member). Because VContainer uses
 * copy-on-write, that copy stayed shared with the real container only until the real one was next
 * mutated - after that it was permanently frozen on the pre-draft-block state, so a custom
 * variable's formula could never see a point or line added later, even though the exact same
 * formula worked fine everywhere else in the app. Fixed by holding a pointer to the live container
 * instead of a copy.
 */
void TST_CustomVariableContainer::TestGetDataSeesObjectsAddedAfterConstruction()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    // Construct the CustomVariable before the container has any points - mirrors <variables>
    // being parsed before <draftBlock> in the file.
    CustomVariable variable(data.data(), QStringLiteral("#Test"), 0, 0, QStringLiteral("0"), true);

    QVERIFY(variable.GetData()->DataGObjects()->isEmpty());

    // Now populate the container - mirrors the draft block being parsed afterward.
    const quint32 id1 = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 id2 = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(id1, id2, id2);

    // GetData() must see the same live container, not a stale snapshot from before these existed.
    QCOMPARE(variable.GetData()->DataGObjects()->size(), 2);
    QVERIFY(variable.GetData()->DataVariables()->contains(QStringLiteral("Line_A1_A2")));
}

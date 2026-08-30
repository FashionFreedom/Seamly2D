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

#include "tst_vcompositevariabletokens.h"

#include <QtTest>

#include "../../libs/vpatterndb/vcompositevariabletokens.h"
#include "../../libs/vpatterndb/vformulaidtranslator.h"
#include "../../libs/vpatterndb/variables/vinternalvariable.h"
#include "../../libs/vpatterndb/variables/vlinelength.h"
#include "../../libs/vpatterndb/variables/vvariable.h"
#include "../../libs/vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
TST_VCompositeVariableTokens::TST_VCompositeVariableTokens(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCompositeVariableTokens::TestLineLengthMapping()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p2(10, 0, QStringLiteral("A2"), 5, 5);

    QSharedPointer<VInternalVariable> length(new VLengthLine(&p1, 42, &p2, 17, Unit::Cm));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(length->GetName(), length);
    QCOMPARE(length->GetName(), QStringLiteral("Line_A1_A2"));

    const QHash<QString, QString> nameToIdToken = VCompositeVariableTokens::NameToIdTokenMap(variables);
    QCOMPARE(nameToIdToken.value(QStringLiteral("Line_A1_A2")), QStringLiteral("Line_id42_id17"));

    const QHash<QString, QString> idTokenToName = VCompositeVariableTokens::IdTokenToNameMap(variables);
    QCOMPARE(idTokenToName.value(QStringLiteral("Line_id42_id17")), QStringLiteral("Line_A1_A2"));

    QCOMPARE(VFormulaIdTranslator::FormulaNamesToIds(QStringLiteral("Line_A1_A2*2"), nameToIdToken),
             QStringLiteral("Line_id42_id17*2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves the composite case survives rename the same way the plain point case does: the
 * stored formula (built from the ids) never changes; only what it resolves to for display changes
 * once the constituent points get new names.
 */
void TST_VCompositeVariableTokens::TestLineLengthSurvivesRename()
{
    const VPointF p1(0, 0, QStringLiteral("A1"), 5, 5);
    const VPointF p2(10, 0, QStringLiteral("A2"), 5, 5);

    QSharedPointer<VInternalVariable> lengthBeforeRename(new VLengthLine(&p1, 42, &p2, 17, Unit::Cm));
    QHash<QString, QSharedPointer<VInternalVariable>> variablesBeforeRename;
    variablesBeforeRename.insert(lengthBeforeRename->GetName(), lengthBeforeRename);

    const QString stored = VFormulaIdTranslator::FormulaNamesToIds(
        QStringLiteral("Line_A1_A2*2"), VCompositeVariableTokens::NameToIdTokenMap(variablesBeforeRename));
    QCOMPARE(stored, QStringLiteral("Line_id42_id17*2"));

    // A2 renamed to Halsloch_hinten: same ids, a fresh VLengthLine reflects the new name.
    const VPointF p2Renamed(10, 0, QStringLiteral("Halsloch_hinten"), 5, 5);
    QSharedPointer<VInternalVariable> lengthAfterRename(new VLengthLine(&p1, 42, &p2Renamed, 17, Unit::Cm));
    QHash<QString, QSharedPointer<VInternalVariable>> variablesAfterRename;
    variablesAfterRename.insert(lengthAfterRename->GetName(), lengthAfterRename);

    QCOMPARE(VFormulaIdTranslator::FormulaIdsToNames(
                 stored, VCompositeVariableTokens::IdTokenToNameMap(variablesAfterRename)),
             QStringLiteral("Line_A1_Halsloch_hinten*2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("Line_id42_id17*2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCompositeVariableTokens::TestUnknownVariableTypeLeftOut()
{
    QSharedPointer<VInternalVariable> plain(new VVariable(QStringLiteral("MyVar")));

    QHash<QString, QSharedPointer<VInternalVariable>> variables;
    variables.insert(plain->GetName(), plain);

    QVERIFY(VCompositeVariableTokens::NameToIdTokenMap(variables).isEmpty());
}

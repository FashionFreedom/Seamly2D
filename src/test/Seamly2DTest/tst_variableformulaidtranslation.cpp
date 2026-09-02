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
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

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
    before->AddLine(id1, id2, id2);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_C5_D1/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1/2").arg(id2));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(id1, new VPointF(0, 0, QStringLiteral("C5qqq"), 5, 5));
    after->UpdateGObject(id2, new VPointF(10, 0, QStringLiteral("D1"), 5, 5));
    after->AddLine(id1, id2, id2);

    QCOMPARE(formulaIdsToNames(stored, idTokenToNameMap(after.data())),
             QStringLiteral("Line_C5qqq_D1/2"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("Line_id%1/2").arg(id2));
}

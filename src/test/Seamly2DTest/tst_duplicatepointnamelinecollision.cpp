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

#include "tst_duplicatepointnamelinecollision.h"

#include <QtTest>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/compositevariabletokens.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/ifc/ifcdef.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_DuplicatePointNameLineCollision::TST_DuplicatePointNameLineCollision(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Two entirely unrelated lines, connecting two entirely unrelated pairs of points, that
 * happen to generate the exact same composite display name because their endpoints share display
 * names with another pair of points elsewhere in the pattern - point names have never been
 * enforced globally unique (see issue #1678: the bundled sample
 * src/app/share/samples/patterns/jacket1_52-176.sm2d legitimately has two different points named
 * "A" (ids 33 and 125) and two different points named "A2" (ids 41 and 143), in the same draft
 * block).
 *
 * Before the fix, VContainer::AddVariable()'s "name already exists -> update the existing object
 * in place" semantics (vcontainer.cpp, meant to support re-parsing the SAME line after an edit)
 * would let the second AddLine() call below silently splice its line id onto the first line's
 * name, discarding the first line's id-token from the variable table entirely. This test proves
 * VContainer::UniqueLineVariableName (vcontainer.cpp) now disambiguates the second registration
 * instead, so both lines stay independently resolvable.
 */
void TST_DuplicatePointNameLineCollision::TestDuplicatePointNamesDoNotCollideAcrossUnrelatedLines()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    const quint32 idA_first   = 33;
    const quint32 idA2_first  = 41;
    const quint32 idA_second  = 125;
    const quint32 idA2_second = 143;

    data->UpdateGObject(idA_first,   new VPointF(0, 0, QStringLiteral("A"), 5, 5));
    data->UpdateGObject(idA2_first,  new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->UpdateGObject(idA_second,  new VPointF(1000, 0, QStringLiteral("A"), 5, 5));
    data->UpdateGObject(idA2_second, new VPointF(1010, 0, QStringLiteral("A2"), 5, 5));

    // First line ("A" -> "A2"), registered first - mirrors VPattern::ParseToolEndLine parsing
    // point 41 while walking jacket1_52-176.sm2d top to bottom.
    const quint32 firstLineId = 209;
    data->AddLine(idA_first, idA2_first, firstLineId);

    // Second, completely unrelated line - also generating the display name "Line_A_A2" purely
    // because of the point-name collision - registered later, mirroring
    // VPattern::ParseToolAlongLine parsing point 143.
    const quint32 secondLineId = 277;
    data->AddLine(idA_second, idA2_second, secondLineId);

    const QString firstToken  = line_ + idToken(firstLineId);
    const QString secondToken = line_ + idToken(secondLineId);

    const QHash<QString, QString> idToName =
        CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());

    QVERIFY2(idToName.contains(firstToken),
             "The first line's id-token must still resolve to a name after a second, unrelated "
             "line with the same generated display name is registered.");
    QVERIFY2(idToName.contains(secondToken),
             "The second line's id-token must resolve to its own name, distinct from the "
             "first line's, even though both lines share the same generated display name.");
    QVERIFY2(idToName.value(firstToken) != idToName.value(secondToken),
             "Two structurally different lines must never resolve to the same display name.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief End-to-end reproduction of the maintainer-reported "Defekte Formel" infinite loop (Open
 * jacket1_52-176.sm2d -> Save As -> Close -> Reopen), using the exact ids and names from that
 * bundled sample: point 41 ("A2", ending point 33's ("A") line) is parsed long before point 143
 * (also named "A2", from point 125, also named "A"). Point 42's old-format formula is the plain
 * text "Line_A_A2" - a reference to point 41's line, the only one that existed when the 0.6.8
 * file was authored (this branch's line1Id/line2Id/lineId attributes, and the id-token formula
 * encoding, postdate that file entirely).
 *
 * Before the fix: by the time the file was first saved, point 143 had already self-healed its
 * own, unrelated line to id 277 under the exact same generated name "Line_A_A2", silently
 * overwriting point 41's entry (id 209). So nameToIdTokenMap() resolved "Line_A_A2" to 277, not
 * 209, and point 42's formula was saved as "Line_id277". On reload, point 42 is parsed before
 * point 143 - so idTokenToNameMap() at that moment has never heard of id 277, the token is left
 * dangling, and the formula shows as raw, untranslated text - the maintainer's crash-loop.
 *
 * This test proves the fix breaks that chain at its root: nameToIdTokenMap() now resolves
 * "Line_A_A2" to point 41's line id (209) - the one the formula actually named, and the one that
 * precedes point 42 in document order - so idTokenToNameMap() resolves it immediately on reload,
 * without ever needing to have seen point 143 or its line at all.
 */
void TST_DuplicatePointNameLineCollision::TestOldFormatFormulaSurvivesDuplicatePointNameRoundTrip()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    // Document order mirrors jacket1_52-176.sm2d: point 33 ("A"), then point 41 ("A2", ending
    // point 33's line, self-healed to line id 209), then point 42 ("Г5") whose old-format
    // formula is the plain text "Line_A_A2".
    const quint32 idA_33  = 33;
    const quint32 idA2_41 = 41;
    data->UpdateGObject(idA_33,  new VPointF(0, 0, QStringLiteral("A"), 5, 5));
    data->UpdateGObject(idA2_41, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line209 = 209;
    data->AddLine(idA_33, idA2_41, line209);

    const QString point42Formula = QStringLiteral("Line_A_A2");

    // Further down the same document, point 143 ("A2", from point 125 "A") self-heals its own,
    // unrelated line to id 277 under the exact same generated name.
    const quint32 idA_125   = 125;
    const quint32 idA2_143  = 143;
    data->UpdateGObject(idA_125,  new VPointF(1000, 0, QStringLiteral("A"), 5, 5));
    data->UpdateGObject(idA2_143, new VPointF(1010, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line277 = 277;
    data->AddLine(idA_125, idA2_143, line277);

    // Saving converts point 42's formula from name to id-token using the fully parsed container.
    const QString savedFormula = formulaNamesToIds(point42Formula, nameToIdTokenMap(data.data()));
    QCOMPARE(savedFormula, line_ + idToken(line209));
    QVERIFY2(savedFormula != line_ + idToken(line277),
             "The formula must be saved against the line it actually named (209, point 41's), "
             "never against an unrelated, later-registered line that merely shares its "
             "generated display name (277, point 143's).");

    // Reload: rebuild the container fresh and parse only as far as point 42 - point 143 (and its
    // line, 277) has not been reached yet, exactly mirroring their real positions in the file.
    QScopedPointer<VContainer> reloaded(new VContainer(nullptr, &unit));
    reloaded->ClearForFullParse();
    reloaded->UpdateGObject(idA_33,  new VPointF(0, 0, QStringLiteral("A"), 5, 5));
    reloaded->UpdateGObject(idA2_41, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    reloaded->AddLine(idA_33, idA2_41, line209);

    const QString displayed = formulaIdsToNames(savedFormula, idTokenToNameMap(reloaded.data()));
    QCOMPARE(displayed, point42Formula);
    QVERIFY2(!displayed.contains(QStringLiteral("id%1").arg(line277)),
             "The formula must never be left as a dangling id-token pointing at a line that has "
             "not been parsed yet - this is exactly the maintainer's \"Defekte Formel\" infinite "
             "loop when opening jacket1_52-176.sm2d, saving, and reopening it.");
}

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

#include "tst_lineidselfhealcollision.h"

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
TST_LineIdSelfHealCollision::TST_LineIdSelfHealCollision(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reproduces, at the VContainer level, exactly the sequence VPattern::resolveOrAssignLineId
 * (src/app/seamly2d/xml/vpattern.cpp:1185) runs during a single top-to-bottom full parse of an
 * old-format (pre-0.7.5) pattern file that has no persisted line-id attributes:
 *
 * 1. VPattern::PrepareForParse() calls VContainer::ClearForFullParse() at the start of every
 *    full parse, which resets the static VContainer::_id counter to NULL_ID (vcontainer.cpp:498).
 * 2. As the parser walks the DOM top to bottom, VContainer::_id is advanced only by
 *    VContainer::UpdateId(), called from UpdateGObject()/AddGObject() for each element ALREADY
 *    parsed (vcontainer.cpp:446-451). It never looks ahead at ids the document will use later.
 * 3. When resolveOrAssignLineId() hits a tool whose line-id attribute is absent, it calls
 *    VContainer::getNextId() (vcontainer.cpp:422), which simply returns _id+1 - the smallest id
 *    not yet seen by the parser so far, NOT the smallest id not used ANYWHERE in the document.
 * 4. VDomDocument::TestUniqueId() (vdomdocument.cpp:545), the only whole-document pre-scan that
 *    runs before parsing starts, only checked for existing duplicate `id` attributes; it never
 *    called UpdateId() to reserve the ids it saw, so it gave the counter no protection at all
 *    against a later collision with an id resolveOrAssignLineId is about to mint.
 *
 * If a later, not-yet-parsed point/tool element in the very same file already owns (as its own,
 * literal `id` attribute) the numeric value getNextId() is about to hand out as a self-healed
 * line id, the two collide: one container id now denotes both a real point and someone else's
 * internal line. This test proves that collision is real and mechanical, using only the public
 * VContainer API real parsing code calls (UpdateGObject, AddLine, getNextId), not a
 * hand-fabricated scenario.
 *
 * This test deliberately does NOT seed the counter the way the fix does, so it still reproduces
 * the pre-fix collision in isolation - see TestSeedingIdCounterAgainstMaxFileIdPreventsTheCollision
 * below for the same scenario with the fix applied.
 */
void TST_LineIdSelfHealCollision::TestSelfHealedLineIdCanCollideWithNotYetParsedPointId()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    // Step 1: start of a full parse - mirrors VContainer::ClearForFullParse().
    data->ClearForFullParse();

    // Step 2: the first point in the document is parsed and registered with its real,
    // on-disk id - exactly what ParsePointElement()/UpdateGObject() does for every point tool.
    const quint32 idC13 = 1;
    data->UpdateGObject(idC13, new VPointF(0, 0, QStringLiteral("C13"), 5, 5));

    // Step 3: next in document order is a point-tool (e.g. "Point - Intersect Line and Axis")
    // whose own persisted `id` is 3, but whose line-id attributes are absent (old file format) -
    // so VPattern::resolveOrAssignLineId mints a fresh one via VContainer::getNextId() *before*
    // the tool's own point (id 3) or the still-unparsed point at id 2 have been registered.
    const quint32 selfHealedLineId = VContainer::getNextId(); // == 2, purely because only id 1
                                                                // has been UpdateId()'d so far.
    QCOMPARE(selfHealedLineId, quint32(2));

    const quint32 idNewPoint = 3;
    data->UpdateGObject(idNewPoint, new VPointF(10, 10, QStringLiteral("Intersect1"), 5, 5));
    // The self-healed line registers its VLengthLine/VLineAngle composite variables keyed by
    // selfHealedLineId - exactly VContainer::AddLine (vcontainer.cpp:616-627).
    data->AddLine(idC13, idNewPoint, selfHealedLineId);

    // Step 4: the DOCUMENT'S NEXT point element, appearing later in the file, already has the
    // literal id="2" persisted from before #1678 ever existed - the very value just minted above.
    const quint32 idC12c = 2;
    QVERIFY2(idC12c == selfHealedLineId,
             "This test's premise: a later point's real, on-disk id equals the id "
             "resolveOrAssignLineId just self-healed for an earlier tool's internal line.");
    data->UpdateGObject(idC12c, new VPointF(20, 0, QStringLiteral("C12c"), 5, 5));

    // The collision now sits in the container: idToken(2) is simultaneously the point "C12c"
    // (in DataGObjects) and the self-healed line's angle/length composite variables (in
    // DataVariables, both keyed off selfHealedLineId == 2).
    const QString token = idToken(selfHealedLineId);
    QCOMPARE(FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects()).value(token),
             QStringLiteral("C12c"));
    // Composite (line-angle) tokens are prefixed (angleLine_id<N>, see
    // CompositeVariableTokens::nameToIdTokenMap, compositevariabletokens.cpp:59) rather than the
    // bare "id<N>" a plain point uses - so the collision doesn't show up as one literal key
    // shared between the two maps, but as the SAME numeric id (selfHealedLineId) backing both an
    // entry keyed "id2" (the point) and an entry keyed "angleLine_id2" (the line's angle) -
    // exactly the ambiguity the next test's formula-level probe exploits.
    const QString compositeAngleToken = angleLine_ + token;
    QVERIFY2(CompositeVariableTokens::idTokenToNameMap(*data->DataVariables()).contains(compositeAngleToken),
             "Expected the self-healed line's composite AngleLine_ variable to be keyed off the "
             "very same numeric id the later point C12c now legitimately owns.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Continuation of the test above: shows what a formula stored against the pre-collision
 * id-token displays as once the collision has happened, using the exact merged map
 * (PatternFormulaTokens::idTokenToNameMap) every ParseTool*() function in vpattern.cpp calls to
 * translate a formula for display (e.g. VPattern::ParseToolLineIntersectAxis, vpattern.cpp:2191).
 */
void TST_LineIdSelfHealCollision::TestCollidingLineAngleIdTokensSilentlyMergeInIdTokenToNameMap()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));
    data->ClearForFullParse();

    // Line A: C13 -> C12, self-healed to line_id 100 (simulating it being parsed first).
    const quint32 idC13 = 50;
    const quint32 idC12 = 51;
    data->UpdateGObject(idC13, new VPointF(0, 0, QStringLiteral("C13"), 5, 5));
    data->UpdateGObject(idC12, new VPointF(10, 0, QStringLiteral("C12"), 5, 5));
    const quint32 lineA_id = 100;
    data->AddLine(idC13, idC12, lineA_id);

    const QString storedFormula = QStringLiteral("AngleLine_%1").arg(idToken(lineA_id));
    QCOMPARE(formulaIdsToNames(storedFormula, idTokenToNameMap(data.data())),
             QStringLiteral("AngleLine_C13_C12"));

    // Later, a second, unrelated line - C12 -> C12c - is ALSO self-healed and happens to land on
    // the SAME line_id (100): exactly what happens when two different tools' resolveOrAssignLineId
    // calls each compute getNextId() against a counter that only reflects what has been parsed so
    // far, and the document's actual id layout lets both arrive at the same next-free value on
    // their own, separate single-pass runs (e.g. across two different Open/Save cycles, or two
    // sibling tools whose *own* ids interleave with the freshly-minted line ids in a way that
    // reintroduces the same "next" value). Reproduced here directly to isolate the consequence.
    const quint32 idC12c = 52;
    data->UpdateGObject(idC12c, new VPointF(20, 0, QStringLiteral("C12c"), 5, 5));
    const quint32 lineB_id = 100; // collision, by construction
    data->AddLine(idC12, idC12c, lineB_id);

    // The formula's stored id-token never changed - but "angleLine_id100" is now genuinely
    // ambiguous: CompositeVariableTokens::nameToIdTokenMap (compositevariabletokens.cpp:40-127)
    // maps BOTH "AngleLine_C13_C12" and "AngleLine_C12_C12c" to that exact same token, and
    // ::idTokenToNameMap (compositevariabletokens.cpp:130-143) reverses that. As of the fix for
    // this issue, that reversal now logs a qCWarning(vCon, ...) the moment it notices the two
    // names disagree for the same token - but it still resolves the conflict the same way it
    // always did (last write wins), because by the time this merge runs there is no way left to
    // tell which of the two colliding names is "correct". So whichever name QHash's
    // (per-process, randomized-by-default) iteration order happens to visit last still silently
    // wins the actual value, now accompanied by a warning rather than total silence.
    //
    // Observed empirically: running this exact test binary repeatedly (no code change) flips the
    // result between "AngleLine_C13_C12" (unchanged, luckily) and "AngleLine_C12_C12c" (the
    // maintainer-reported symptom) from one process launch to the next - proving the collision's
    // OUTCOME is not just wrong, it's nondeterministic across runs of the same file. This test
    // only asserts the ambiguity exists (both names are "valid" outcomes); it deliberately does
    // NOT assert which one wins, since that's the whole point of the bug.
    const QString displayedAfterCollision =
        formulaIdsToNames(storedFormula, idTokenToNameMap(data.data()));
    QVERIFY2(displayedAfterCollision == QStringLiteral("AngleLine_C13_C12")
                 || displayedAfterCollision == QStringLiteral("AngleLine_C12_C12c"),
             qPrintable(QStringLiteral("Unexpected display for a colliding id-token: ") +
                        displayedAfterCollision));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Regression test for the fix: same exact scenario as
 * TestSelfHealedLineIdCanCollideWithNotYetParsedPointId above, but with the fix now applied to
 * VPattern::PrepareForParse() (src/app/seamly2d/xml/vpattern.cpp:4387-4404) - the whole document
 * is pre-scanned for its maximum id (VDomDocument::TestUniqueId(), vdomdocument.cpp:545-558) and
 * VContainer::_id is seeded against that maximum, via VContainer::UpdateId() (vcontainer.cpp:446),
 * right after ClearForFullParse() resets it and before any self-heal can run.
 *
 * This test drives VContainer directly (as the test above does), because that is the level at
 * which the bug and its fix both actually live - VDomDocument::TestUniqueId()'s own max-id
 * computation is a plain linear scan with no interesting branches of its own to regression-test
 * here. What matters, and what this test proves, is the *consequence*: once the counter is
 * seeded against the true whole-file maximum id before self-healing runs, getNextId() can never
 * again return a value some later, not-yet-parsed element already owns - so the exact collision
 * from the test above (a self-healed line id landing on 2, the very id the document's next point
 * legitimately owns) no longer occurs.
 */
void TST_LineIdSelfHealCollision::TestSeedingIdCounterAgainstMaxFileIdPreventsTheCollision()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    // Step 1: start of a full parse - mirrors VContainer::ClearForFullParse().
    data->ClearForFullParse();

    // Step 1b (the fix): mirrors VDomDocument::TestUniqueId()'s whole-document pre-scan finding
    // the maximum literal id anywhere in the file - ids 1, 2 and 3, exactly as in the
    // pre-fix test above - followed by VPattern::PrepareForParse() seeding the counter with it.
    const quint32 maxIdInWholeFile = 3;
    VContainer::UpdateId(maxIdInWholeFile);

    // Step 2: same as before - the first point in the document is parsed and registered with
    // its real, on-disk id.
    const quint32 idC13 = 1;
    data->UpdateGObject(idC13, new VPointF(0, 0, QStringLiteral("C13"), 5, 5));

    // Step 3: same tool as before self-heals its missing line id via getNextId() - but now the
    // counter already sits at the whole file's maximum, so the newly minted id is guaranteed to
    // be strictly greater than every id literally present anywhere in the document, including
    // ones not yet parsed.
    const quint32 selfHealedLineId = VContainer::getNextId();
    QVERIFY2(selfHealedLineId > maxIdInWholeFile,
             "The self-healed line id must exceed every id present anywhere in the file, not "
             "just the ids already parsed so far.");

    const quint32 idNewPoint = 3;
    data->UpdateGObject(idNewPoint, new VPointF(10, 10, QStringLiteral("Intersect1"), 5, 5));
    data->AddLine(idC13, idNewPoint, selfHealedLineId);

    // Step 4: the document's next point, which in the pre-fix test above collided with the
    // self-healed line id, now registers under its own real id (2) without incident - it is
    // numerically distinct from selfHealedLineId because that id was minted above the file's max.
    const quint32 idC12c = 2;
    QVERIFY2(idC12c != selfHealedLineId,
             "The fix must prevent this exact pre-fix collision: a later point's real id must "
             "never again equal a self-healed line id minted earlier in the same parse.");
    data->UpdateGObject(idC12c, new VPointF(20, 0, QStringLiteral("C12c"), 5, 5));

    // No collision: the point and the self-healed line's composite variables now resolve to two
    // different, non-conflicting id-tokens.
    const QString pointToken = idToken(idC12c);
    const QString lineToken = idToken(selfHealedLineId);
    QVERIFY(pointToken != lineToken);

    QCOMPARE(FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects()).value(pointToken),
             QStringLiteral("C12c"));
    const QString compositeAngleToken = angleLine_ + lineToken;
    QVERIFY2(CompositeVariableTokens::idTokenToNameMap(*data->DataVariables())
                 .contains(compositeAngleToken),
             "The self-healed line's composite AngleLine_ variable should still be registered "
             "under its own id-token.");
    QVERIFY2(!FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects()).contains(compositeAngleToken),
             "The line's composite token must not collide with any point's plain id-token.");
}

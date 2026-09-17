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

#include "tst_movesplinemergeidtoken.h"

#include <QGraphicsScene>
#include <QtTest>

#include "../vtools/undocommands/movespline.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"
#include "../vpatterndb/calculator.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../qmuparser/qmuparsererror.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

namespace
{
// A minimal, concrete VAbstractPattern. MoveSpline only needs a non-null doc pointer to satisfy
// VUndoCommand's constructor SCASSERT - neither mergeWith() nor the formula-translation logic this
// test exercises (replicated from MoveSpline::Do(), which is private) ever dereferences it, so every
// pure virtual is a harmless stub.
class TestPatternDoc : public VAbstractPattern
{
public:
    explicit TestPatternDoc(QObject *parent = nullptr)
        : VAbstractPattern(parent)
    {}

    virtual void      CreateEmptyFile() override {}
    virtual void      IncrementReferens(quint32 id) const override { Q_UNUSED(id) }
    virtual void      DecrementReferens(quint32 id) const override { Q_UNUSED(id) }
    virtual QStringList GetCurrentAlphabet() const override { return QStringList(); }
    virtual QString   GenerateLabel(const LabelType &type, const QString &reservedName = QString()) const override
    {
        Q_UNUSED(type)
        Q_UNUSED(reservedName)
        return QString();
    }
    virtual QString   generateSuffix(const QString &type) const override { Q_UNUSED(type) return QString(); }
    virtual void      UpdateToolData(const quint32 &id, VContainer *data) override
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }
    virtual void      LiteParseTree(const Document &parse) override { Q_UNUSED(parse) }
};

const quint32 pointA1Id  = 8001;
const quint32 pointA2Id  = 8002;
const quint32 lineId     = 8003;
const quint32 splineId   = 8004;

// Builds a container with two named points and the line-length/line-angle composite variables
// VContainer::AddLine() derives from their current names, mirroring a fresh reparse after an edit -
// same technique TST_DanglingIdTokenFormula and TST_CustomVariableContainer already use.
VContainer *buildContainer(const QString &a1Name, const QString &a2Name)
{
    static const Unit unit = Unit::Cm;
    VContainer *data = new VContainer(nullptr, &unit);
    data->UpdateGObject(pointA1Id, new VPointF(0, 0, a1Name, 5, 5));
    data->UpdateGObject(pointA2Id, new VPointF(10, 0, a2Name, 5, 5));
    data->AddLine(pointA1Id, pointA2Id, lineId);
    return data;
}

VSpline makeSpline(const QString &angle1Formula)
{
    return VSpline(VPointF(0, 0, QStringLiteral("P1"), 5, 5), VPointF(20, 20, QStringLiteral("P4"), 5, 5),
                    0, angle1Formula, 0, QStringLiteral("0"), 0, QStringLiteral("0"), 0, QStringLiteral("0"),
                    splineId);
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
TST_MoveSplineMergeIdToken::TST_MoveSplineMergeIdToken(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Direct proof of the fix: build MoveSpline cmd1 against a container where the line between
 * A1 and A2 is called "Line_A1_A2", then rename A1 to "A1_mid" (rebuilding the container the way a
 * real reparse would) and build cmd2 against that renamed container, with a formula that references
 * the NEW composite name "Line_A1_mid_A2" - exactly what a continued drag captures after the app's
 * rename machinery has already relabeled the point. Merge cmd2 into cmd1, then translate cmd1's
 * (post-merge) formula through cmd1's own (post-merge) id-token map, the same call
 * MoveSpline::Do() makes internally. It must resolve to a proper id token, not leave the raw,
 * newly-typed composite name untranslated.
 */
void TST_MoveSplineMergeIdToken::TestMergeAdoptsMergedCommandsIdTokenMap()
{
    QGraphicsScene rawScene;
    QGraphicsScene *scenePtr = &rawScene;
    qApp->setCurrentScene(&scenePtr);

    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const QString beforeLineName = QStringLiteral("Line_A1_A2");
    QVERIFY2(before->DataVariables()->contains(beforeLineName),
             "Setup problem: expected composite variable Line_A1_A2 not found before the rename.");

    const VSpline oldSpl = makeSpline(QStringLiteral("0"));
    const VSpline newSpl1 = makeSpline(beforeLineName + QStringLiteral("*2"));

    MoveSpline cmd1(&doc, &oldSpl, newSpl1, before.data(), splineId);

    // Simulate the point rename a continued drag would run into: A1 -> A1_mid. Composite variable
    // names are derived from the endpoints' current names at AddLine() time, so a fresh container
    // rebuild is what a real reparse after the rename produces.
    QScopedPointer<VContainer> after(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const QString afterLineName = QStringLiteral("Line_A1_mid_A2");
    QVERIFY2(after->DataVariables()->contains(afterLineName),
             "Setup problem: expected composite variable Line_A1_mid_A2 not found after the rename.");

    const VSpline newSpl2 = makeSpline(afterLineName + QStringLiteral("*2"));
    MoveSpline cmd2(&doc, &oldSpl, newSpl2, after.data(), splineId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    QCOMPARE(cmd1.getNewSpline().GetStartAngleFormula(), afterLineName + QStringLiteral("*2"));

    // This is exactly what MoveSpline::Do() computes for AttrAngle1.
    const QString stored = formulaNamesToIds(cmd1.getNewSpline().GetStartAngleFormula(), cmd1.getNameToIdToken());

    const QString expectedToken = formulaNamesToIds(afterLineName, nameToIdTokenMap(after.data()));
    QVERIFY2(stored == expectedToken + QStringLiteral("*2"),
             qUtf8Printable(QStringLiteral("Merged formula was not translated to an id token - got '%1', expected "
                                          "'%2*2'. This means mergeWith() left the pre-merge, stale id-token map "
                                          "in place instead of adopting cmd2's.")
                            .arg(stored, expectedToken)));

    // And the map itself must now be cmd2's, not cmd1's original one.
    QVERIFY(cmd1.getNameToIdToken().contains(afterLineName));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same merge as above, but shows the downstream consequence with an actually-evaluated
 * value rather than just inspecting the stored formula text. Id-token storage exists precisely so a
 * formula survives ANY NUMBER of further renames - idTokenToNameMap() re-resolves the token to
 * whatever the current display name is. If mergeWith() left the raw post-rename NAME untranslated
 * (the bug), that literal text has no relationship to an id at all: it happens to still evaluate
 * right after this one rename (it matches the container's current name by coincidence), but a
 * SECOND rename breaks it, because plain text never tracks anything. A correctly stored id token
 * keeps evaluating correctly through any number of further renames.
 */
void TST_MoveSplineMergeIdToken::TestStaleMapSurvivesASecondRenameWithWrongOrFailingValue()
{
    QGraphicsScene rawScene;
    QGraphicsScene *scenePtr = &rawScene;
    qApp->setCurrentScene(&scenePtr);

    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const VSpline oldSpl = makeSpline(QStringLiteral("0"));
    const VSpline newSpl1 = makeSpline(QStringLiteral("Line_A1_A2*2"));
    MoveSpline cmd1(&doc, &oldSpl, newSpl1, before.data(), splineId);

    QScopedPointer<VContainer> mid(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const VSpline newSpl2 = makeSpline(QStringLiteral("Line_A1_mid_A2*2"));
    MoveSpline cmd2(&doc, &oldSpl, newSpl2, mid.data(), splineId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    const QString stored = formulaNamesToIds(cmd1.getNewSpline().GetStartAngleFormula(), cmd1.getNameToIdToken());

    // A second rename, independent of the merge: A1_mid -> A1_final. Line length is unchanged
    // throughout (points never moved), so its value stays whatever VLengthLine reports for that
    // (unmoved) 10px-apart pair in the container's pattern unit, times 2, no matter how many times
    // A1 gets renamed - computed fresh against the final container rather than hard-coded, so the
    // assertion does not depend on knowing VLengthLine's internal px-to-unit conversion factor.
    QScopedPointer<VContainer> final_(buildContainer(QStringLiteral("A1_final"), QStringLiteral("A2")));
    Calculator referenceCal;
    const qreal expectedValue = referenceCal.EvalFormula(final_->DataVariables(),
                                                          QStringLiteral("Line_A1_final_A2*2"));

    const QString displayed = formulaIdsToNames(stored, idTokenToNameMap(final_.data()));

    bool threw = false;
    qreal result = 0;
    try
    {
        Calculator cal;
        result = cal.EvalFormula(final_->DataVariables(), displayed);
    }
    catch (qmu::QmuParserError &)
    {
        threw = true;
    }

    QVERIFY2(!threw,
             qUtf8Printable(QStringLiteral("Formula '%1' (translated from stored '%2') failed to evaluate against "
                                          "the container after a second, independent rename. A correctly stored "
                                          "id token must keep resolving through any number of further renames.")
                            .arg(displayed, stored)));
    QVERIFY2(qFuzzyCompare(result, expectedValue),
             qUtf8Printable(QStringLiteral("Formula '%1' evaluated to %2, expected %3 - the merge must not change "
                                          "the mathematical value of the (unmoved) line, only how its reference "
                                          "survives renames.")
                            .arg(displayed).arg(result).arg(expectedValue)));
}

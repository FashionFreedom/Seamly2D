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

#include "tst_savepieceundomergeidtoken.h"

#include <QtTest>

#include "../vtools/undocommands/savepieceoptions.h"
#include "../vtools/undocommands/savepiecepathoptions.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"
#include "../vpatterndb/calculator.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../qmuparser/qmuparsererror.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

namespace
{
// A minimal, concrete VAbstractPattern. SavePieceOptions/SavePiecePathOptions only need a non-null
// doc pointer to satisfy VUndoCommand's constructor SCASSERT - neither mergeWith() nor the
// formula-translation logic this test exercises ever dereferences it, so every pure virtual is a
// harmless stub. Same technique as TST_MoveSplineMergeIdToken.
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

const quint32 pointA1Id = 9001;
const quint32 pointA2Id = 9002;
const quint32 pieceId   = 9004;
const quint32 pathId    = 9005;

// Builds a container with two named points and the line-length composite variable
// VContainer::AddLine() derives from their current names, mirroring a fresh reparse after an edit -
// same technique TST_MoveSplineMergeIdToken already uses.
VContainer *buildContainer(const QString &a1Name, const QString &a2Name)
{
    static const Unit unit = Unit::Cm;
    VContainer *data = new VContainer(nullptr, &unit);
    data->UpdateGObject(pointA1Id, new VPointF(0, 0, a1Name, 5, 5));
    data->UpdateGObject(pointA2Id, new VPointF(10, 0, a2Name, 5, 5));
    data->AddLine(pointA1Id, pointA2Id);
    return data;
}

VPiece makePiece(const QString &saWidthFormula)
{
    VPiece piece;
    piece.setSeamAllowanceWidthFormula(saWidthFormula, 0);
    return piece;
}

VPiecePath makePath(const QString &saBeforeFormula)
{
    VPiecePath path;
    VPieceNode node(pointA1Id, Tool::NodePoint);
    node.setBeforeSAFormula(saBeforeFormula);
    path.Append(node);
    return path;
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
TST_SavePieceUndoMergeIdToken::TST_SavePieceUndoMergeIdToken(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Direct proof of the fix: build SavePieceOptions cmd1 against a container where the line
 * between A1 and A2 is called "Line_A1_A2", then rename A1 to "A1_mid" (rebuilding the container the
 * way a real reparse would) and build cmd2 against that renamed container, with a seam-allowance-
 * width formula that references the NEW composite name "Line_A1_mid_A2" - exactly what a continued
 * property-editor edit captures after the app's rename machinery has already relabeled the point.
 * Merge cmd2 into cmd1, then translate cmd1's (post-merge) formula through cmd1's own (post-merge)
 * id-token map, the same call PatternPieceTool::addAttributes() makes internally. It must resolve to
 * a proper id token, not leave the raw, newly-typed composite name untranslated.
 */
void TST_SavePieceUndoMergeIdToken::TestSavePieceOptionsMergeAdoptsMergedCommandsIdTokenMap()
{
    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const QString beforeLineName = QStringLiteral("Line_A1_A2");
    QVERIFY2(before->DataVariables()->contains(beforeLineName),
             "Setup problem: expected composite variable Line_A1_A2 not found before the rename.");

    const VPiece oldPiece;
    const VPiece newPiece1 = makePiece(beforeLineName + QStringLiteral("*2"));
    SavePieceOptions cmd1(oldPiece, newPiece1, &doc, before.data(), pieceId);

    // Simulate the point rename a continued property-editor edit would run into: A1 -> A1_mid.
    QScopedPointer<VContainer> after(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const QString afterLineName = QStringLiteral("Line_A1_mid_A2");
    QVERIFY2(after->DataVariables()->contains(afterLineName),
             "Setup problem: expected composite variable Line_A1_mid_A2 not found after the rename.");

    const VPiece newPiece2 = makePiece(afterLineName + QStringLiteral("*2"));
    SavePieceOptions cmd2(oldPiece, newPiece2, &doc, after.data(), pieceId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    QCOMPARE(cmd1.getNewPiece().getSeamAllowanceWidthFormula(), afterLineName + QStringLiteral("*2"));

    // This is exactly what PatternPieceTool::addAttributes() computes for the seam-allowance width.
    const QString stored = formulaNamesToIds(cmd1.getNewPiece().getSeamAllowanceWidthFormula(),
                                             cmd1.getNameToIdToken());

    const QString expectedToken = formulaNamesToIds(afterLineName, nameToIdTokenMap(after.data()));
    QVERIFY2(stored == expectedToken + QStringLiteral("*2"),
             qUtf8Printable(QStringLiteral("Merged formula was not translated to an id token - got '%1', expected "
                                          "'%2*2'. This means mergeWith() left the pre-merge, stale id-token map "
                                          "in place instead of adopting cmd2's.")
                            .arg(stored, expectedToken)));

    QVERIFY(cmd1.getNameToIdToken().contains(afterLineName));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same merge as above, but shows the downstream consequence with an actually-evaluated value
 * rather than just inspecting the stored formula text - same shape as
 * TestStaleMapSurvivesASecondRenameWithWrongOrFailingValue() in TST_MoveSplineMergeIdToken.
 */
void TST_SavePieceUndoMergeIdToken::TestSavePieceOptionsStaleMapSurvivesASecondRenameWithWrongOrFailingValue()
{
    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const VPiece oldPiece;
    const VPiece newPiece1 = makePiece(QStringLiteral("Line_A1_A2*2"));
    SavePieceOptions cmd1(oldPiece, newPiece1, &doc, before.data(), pieceId);

    QScopedPointer<VContainer> mid(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const VPiece newPiece2 = makePiece(QStringLiteral("Line_A1_mid_A2*2"));
    SavePieceOptions cmd2(oldPiece, newPiece2, &doc, mid.data(), pieceId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    const QString stored = formulaNamesToIds(cmd1.getNewPiece().getSeamAllowanceWidthFormula(),
                                             cmd1.getNameToIdToken());

    // A second rename, independent of the merge: A1_mid -> A1_final. Line length is unchanged
    // throughout (points never moved), so the expected value is computed fresh against the final
    // container rather than hard-coded.
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

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same proof as TestSavePieceOptionsMergeAdoptsMergedCommandsIdTokenMap(), but for
 * SavePiecePathOptions - the sibling undo command for internal-path/custom-seam-allowance node
 * edits. The formula under test is a piece node's per-node "before" seam-allowance-width override
 * (VPieceNode::GetFormulaSABefore()), the field VAbstractTool::AddSANode() actually translates.
 */
void TST_SavePieceUndoMergeIdToken::TestSavePiecePathOptionsMergeAdoptsMergedCommandsIdTokenMap()
{
    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const QString beforeLineName = QStringLiteral("Line_A1_A2");
    QVERIFY2(before->DataVariables()->contains(beforeLineName),
             "Setup problem: expected composite variable Line_A1_A2 not found before the rename.");

    const VPiecePath oldPath;
    const VPiecePath newPath1 = makePath(beforeLineName + QStringLiteral("*2"));
    SavePiecePathOptions cmd1(NULL_ID, oldPath, newPath1, &doc, before.data(), pathId);

    QScopedPointer<VContainer> after(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const QString afterLineName = QStringLiteral("Line_A1_mid_A2");
    QVERIFY2(after->DataVariables()->contains(afterLineName),
             "Setup problem: expected composite variable Line_A1_mid_A2 not found after the rename.");

    const VPiecePath newPath2 = makePath(afterLineName + QStringLiteral("*2"));
    SavePiecePathOptions cmd2(NULL_ID, oldPath, newPath2, &doc, after.data(), pathId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    QCOMPARE(cmd1.newPath().at(0).GetFormulaSABefore(), afterLineName + QStringLiteral("*2"));

    // This is exactly what VAbstractTool::AddSANode() computes for AttrSABefore.
    const QString stored = formulaNamesToIds(cmd1.newPath().at(0).GetFormulaSABefore(), cmd1.getNameToIdToken());

    const QString expectedToken = formulaNamesToIds(afterLineName, nameToIdTokenMap(after.data()));
    QVERIFY2(stored == expectedToken + QStringLiteral("*2"),
             qUtf8Printable(QStringLiteral("Merged formula was not translated to an id token - got '%1', expected "
                                          "'%2*2'. This means mergeWith() left the pre-merge, stale id-token map "
                                          "in place instead of adopting cmd2's.")
                            .arg(stored, expectedToken)));

    QVERIFY(cmd1.getNameToIdToken().contains(afterLineName));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same downstream-value proof as TestSavePieceOptionsStaleMapSurvivesASecondRenameWithWrongOrFailingValue(),
 * for SavePiecePathOptions.
 */
void TST_SavePieceUndoMergeIdToken::TestSavePiecePathOptionsStaleMapSurvivesASecondRenameWithWrongOrFailingValue()
{
    TestPatternDoc doc;

    QScopedPointer<VContainer> before(buildContainer(QStringLiteral("A1"), QStringLiteral("A2")));
    const VPiecePath oldPath;
    const VPiecePath newPath1 = makePath(QStringLiteral("Line_A1_A2*2"));
    SavePiecePathOptions cmd1(NULL_ID, oldPath, newPath1, &doc, before.data(), pathId);

    QScopedPointer<VContainer> mid(buildContainer(QStringLiteral("A1_mid"), QStringLiteral("A2")));
    const VPiecePath newPath2 = makePath(QStringLiteral("Line_A1_mid_A2*2"));
    SavePiecePathOptions cmd2(NULL_ID, oldPath, newPath2, &doc, mid.data(), pathId);

    QVERIFY(cmd1.mergeWith(&cmd2));
    const QString stored = formulaNamesToIds(cmd1.newPath().at(0).GetFormulaSABefore(), cmd1.getNameToIdToken());

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

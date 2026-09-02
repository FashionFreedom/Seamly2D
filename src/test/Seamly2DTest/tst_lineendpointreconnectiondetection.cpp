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

#include "tst_lineendpointreconnectiondetection.h"

#include <QtTest>
#include <QLineF>

#include "../../libs/vpatterndb/patternformulatokens.h"
#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/calculator.h"
#include "../../libs/vgeometry/vpointf.h"
#include "../../libs/qmuparser/qmuparsererror.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
TST_LineEndpointReconnectionDetection::TST_LineEndpointReconnectionDetection(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reconnecting a line tool's second point (e.g. from A2 to A3, via the property editor's
 * VToolLine::SetSecondPoint or the tool's dialog) must NOT break a formula that references that
 * line's length/angle - the id-token is keyed by the line tool's own (stable) generating id, not by
 * the point pair, so the same token still resolves after the reconnection, just against the new
 * pairing. This is the line-tool equivalent of TestNodeSAOverrideFormulaSurvivesRename in
 * tst_pieceformulaidtranslation.cpp, which covers the point-rename case. See issue #1678.
 */
void TST_LineEndpointReconnectionDetection::TestReconnectedLineLeavesDependentFormulaUnresolved()
{
    const Unit unit = Unit::Cm;
    const quint32 idA1 = 4001;
    const quint32 idA2 = 4002;
    const quint32 idA3 = 4003;
    const quint32 lineToolId = 4010;

    // Before: the line tool connects A1 to A2, and a dependent formula references that line.
    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    before->AddLine(idA1, idA2, lineToolId);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2/2"), nameToIdTokenMap(before.data()));
    QCOMPARE(stored, QStringLiteral("Line_id%1/2").arg(lineToolId));

    // After: the user edited the line tool's second point in the property editor/dialog, so it now
    // connects A1 to A3 instead. Same tool, same generating id - just a different current pairing.
    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    after->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    after->AddLine(idA1, idA3, lineToolId);

    // The stored token must resolve against the reconnected container, correctly picking up the new
    // pairing (A1-A3) instead of staying broken.
    const QString translated = formulaIdsToNames(stored, idTokenToNameMap(after.data()));
    QCOMPARE(translated, QStringLiteral("Line_A1_A3/2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief The translated text left behind by a reconnection isn't just cosmetically correct - it
 * genuinely evaluates against the reconnected container, using the new pairing's current length.
 * This is what proves editing a line tool's endpoint no longer needs to surface a "repair formula"
 * dialog (compare to the pre-#1678 behavior, where SaveToolOptions::referencesChanged()/issue #1521
 * triggered a full reparse that would then find the formula broken).
 */
void TST_LineEndpointReconnectionDetection::TestReconnectedLineFormulaFailsEvaluation()
{
    const Unit unit = Unit::Cm;
    const quint32 idA1 = 4004;
    const quint32 idA2 = 4005;
    const quint32 idA3 = 4006;
    const quint32 lineToolId = 4011;

    QScopedPointer<VContainer> before(new VContainer(nullptr, &unit));
    before->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    before->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    before->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    before->AddLine(idA1, idA2, lineToolId);

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_A1_A2/2"), nameToIdTokenMap(before.data()));

    QScopedPointer<VContainer> after(new VContainer(nullptr, &unit));
    after->UpdateGObject(idA1, new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    after->UpdateGObject(idA2, new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    after->UpdateGObject(idA3, new VPointF(0, 10, QStringLiteral("A3"), 5, 5));
    after->AddLine(idA1, idA3, lineToolId);

    const QString translated = formulaIdsToNames(stored, idTokenToNameMap(after.data()));

    bool threw = false;
    QScopedPointer<Calculator> cal(new Calculator());
    qreal result = 0;
    try
    {
        result = cal->EvalFormula(after->DataVariables(), translated);
    }
    catch (const qmu::QmuParserError &)
    {
        threw = true;
    }
    QVERIFY2(!threw, "expected the dependent formula to evaluate cleanly against the reconnected "
                      "line, with no repair needed");
    // Half the reconnected line's own (VPointF coordinates are in pixels, not cm) length - computed
    // the same way the production code does, rather than assuming a px-per-cm factor here.
    const QSharedPointer<VPointF> a1 = after->GeometricObject<VPointF>(idA1);
    const QSharedPointer<VPointF> a3 = after->GeometricObject<VPointF>(idA3);
    const qreal expected = FromPixel(QLineF(static_cast<QPointF>(*a1), static_cast<QPointF>(*a3)).length(), unit) / 2;
    QCOMPARE(result, expected);
}

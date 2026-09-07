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

#include "tst_formulaidtranslator.h"

#include <QtTest>

#include "../../libs/vpatterndb/formulaidtranslator.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vgeometry/vpointf.h"

using namespace FormulaIdTranslator;

//---------------------------------------------------------------------------------------------------------------------
TST_FormulaIdTranslator::TST_FormulaIdTranslator(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaIdTranslator::TestFormulaNamesToIds_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<QString>("expected");

    QTest::newRow("single name")            << "A1+5"      << "id42+5";
    QTest::newRow("two names, tokens grow") << "A+B"        << "id1+id2";
    QTest::newRow("unknown name left alone") << "A1+Unknown" << "id42+Unknown";
    QTest::newRow("no names at all")        << "2+2"        << "2+2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaIdTranslator::TestFormulaNamesToIds()
{
    QFETCH(QString, formula);
    QFETCH(QString, expected);

    QHash<QString, QString> nameToIdToken;
    nameToIdToken.insert(QStringLiteral("A1"), QStringLiteral("id42"));
    nameToIdToken.insert(QStringLiteral("A"), QStringLiteral("id1"));
    nameToIdToken.insert(QStringLiteral("B"), QStringLiteral("id2"));

    QCOMPARE(formulaNamesToIds(formula, nameToIdToken), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaIdTranslator::TestFormulaIdsToNames_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<QString>("expected");

    QTest::newRow("single id")               << "id42+5"       << "A1+5";
    QTest::newRow("two ids, tokens shrink")  << "id1+id2"       << "A+B";
    QTest::newRow("unknown id left alone")   << "id42+id99"     << "A1+id99";
    QTest::newRow("no ids at all")           << "2+2"           << "2+2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaIdTranslator::TestFormulaIdsToNames()
{
    QFETCH(QString, formula);
    QFETCH(QString, expected);

    QHash<QString, QString> idTokenToName;
    idTokenToName.insert(QStringLiteral("id42"), QStringLiteral("A1"));
    idTokenToName.insert(QStringLiteral("id1"), QStringLiteral("A"));
    idTokenToName.insert(QStringLiteral("id2"), QStringLiteral("B"));

    QCOMPARE(formulaIdsToNames(formula, idTokenToName), expected);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves the actual point of the feature: once a formula is stored as ids, renaming the
 * object it refers to changes nothing about the stored formula - only the resolved display text.
 */
void TST_FormulaIdTranslator::TestRoundTripSurvivesRename()
{
    QHash<QString, QString> nameToIdToken;
    nameToIdToken.insert(QStringLiteral("A1"), QStringLiteral("id42"));

    const QString stored = formulaNamesToIds(QStringLiteral("A1+5"), nameToIdToken);
    QCOMPARE(stored, QStringLiteral("id42+5"));

    QHash<QString, QString> idTokenToNameBeforeRename;
    idTokenToNameBeforeRename.insert(QStringLiteral("id42"), QStringLiteral("A1"));
    QCOMPARE(formulaIdsToNames(stored, idTokenToNameBeforeRename), QStringLiteral("A1+5"));

    QHash<QString, QString> idTokenToNameAfterRename;
    idTokenToNameAfterRename.insert(QStringLiteral("id42"), QStringLiteral("Halsloch_hinten"));
    QCOMPARE(formulaIdsToNames(stored, idTokenToNameAfterRename),
             QStringLiteral("Halsloch_hinten+5"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("id42+5"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves the reason the translator only ever does whole-token lookup and never tries to split
 * a glued-together display name apart: a point name can itself contain an underscore, so
 * "Line_Halsloch_hinten_A2" cannot be reliably split back into "Halsloch_hinten" and "A2" by string
 * position alone. Handing over the exact composite pairing sidesteps the ambiguity entirely.
 */
void TST_FormulaIdTranslator::TestCompositeNameNoSplittingAmbiguity()
{
    QHash<QString, QString> nameToIdToken;
    nameToIdToken.insert(QStringLiteral("Line_Halsloch_hinten_A2"), QStringLiteral("Line_id42_id17"));

    const QString stored = formulaNamesToIds(
        QStringLiteral("Line_Halsloch_hinten_A2*2"), nameToIdToken);
    QCOMPARE(stored, QStringLiteral("Line_id42_id17*2"));

    QHash<QString, QString> idTokenToName;
    idTokenToName.insert(QStringLiteral("Line_id42_id17"), QStringLiteral("Line_Halsloch_hinten_A2"));
    QCOMPARE(formulaIdsToNames(stored, idTokenToName),
             QStringLiteral("Line_Halsloch_hinten_A2*2"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Same proof as TestRoundTripSurvivesRename, but against a real VContainer/VPointF pair instead
 * of hand-built hashes - confirms the maps built off VContainer::DataGObjects() behave the same way,
 * and that a rename (mutating the point object in place) is picked up live, with no separate table
 * to keep in sync.
 */
void TST_FormulaIdTranslator::TestMapsFromRealContainer()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    VPointF *point = new VPointF(10, 20, QStringLiteral("A1"), 5, 5);
    const quint32 id = data->AddGObject(point);

    const QString stored = formulaNamesToIds(
        QStringLiteral("A1+5"), nameToIdTokenMap(*data->DataGObjects()));
    QCOMPARE(stored, QStringLiteral("id%1+5").arg(id));

    QCOMPARE(formulaIdsToNames(
                 stored, idTokenToNameMap(*data->DataGObjects())),
             QStringLiteral("A1+5"));

    data->GetGObject(id)->setName(QStringLiteral("Halsloch_hinten"));

    QCOMPARE(formulaIdsToNames(
                 stored, idTokenToNameMap(*data->DataGObjects())),
             QStringLiteral("Halsloch_hinten+5"));
    QCOMPARE(stored, QStringLiteral("id%1+5").arg(id));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaIdTranslator::TestIsIdToken_data()
{
    QTest::addColumn<QString>("token");
    QTest::addColumn<bool>("expected");

    QTest::newRow("bare id token")                << "id45"             << true;
    QTest::newRow("bare id token, multi digit")    << "id123456"         << true;
    QTest::newRow("line length")                   << "Line_id45"        << true;
    QTest::newRow("line angle")                    << "AngleLine_id45"   << true;
    QTest::newRow("arc radius 1st")                << "Radius1id45"      << true;
    QTest::newRow("arc radius 2nd")                << "Radius2id45"      << true;
    QTest::newRow("curve start angle")             << "Angle1id45"       << true;
    QTest::newRow("curve end angle")                << "Angle2id45"      << true;
    QTest::newRow("curve control length C1")       << "C1Lengthid45"     << true;
    QTest::newRow("curve control length C2")       << "C2Lengthid45"     << true;
    QTest::newRow("segmented curve length")        << "id45_Seg_2"       << true;
    QTest::newRow("segmented curve angle")         << "Angle1id45_Seg_2" << true;

    // A real user-authored name never happens to look like one of the above.
    QTest::newRow("plain measurement name")        << "bust_circ"        << false;
    QTest::newRow("plain point name")              << "A1"               << false;
    QTest::newRow("custom variable name")          << "#BustCircumfence" << false;
    QTest::newRow("name that merely contains id")  << "Middle"           << false;
    QTest::newRow("id without digits")             << "id"               << false;
    QTest::newRow("empty")                         << ""                 << false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Lets a caller with only the raw, on-disk formula text (no VContainer to translate
 * against yet, e.g. VAbstractPattern::ListMeasurements() scanning formulas before the pattern is
 * parsed) tell an id token apart from a real user-authored name such as a measurement - see the
 * "Measurement file doesn't include all the required measurements" false positive this fixed.
 */
void TST_FormulaIdTranslator::TestIsIdToken()
{
    QFETCH(QString, token);
    QFETCH(bool, expected);

    QCOMPARE(isIdToken(token), expected);
}

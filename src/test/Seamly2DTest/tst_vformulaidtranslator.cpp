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

#include "tst_vformulaidtranslator.h"

#include <QtTest>

#include "../../libs/vpatterndb/vformulaidtranslator.h"

//---------------------------------------------------------------------------------------------------------------------
TST_VFormulaIdTranslator::TST_VFormulaIdTranslator(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFormulaIdTranslator::TestFormulaNamesToIds_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<QString>("expected");

    QTest::newRow("single name")            << "A1+5"      << "id42+5";
    QTest::newRow("two names, tokens grow") << "A+B"        << "id1+id2";
    QTest::newRow("unknown name left alone") << "A1+Unknown" << "id42+Unknown";
    QTest::newRow("no names at all")        << "2+2"        << "2+2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFormulaIdTranslator::TestFormulaNamesToIds()
{
    QFETCH(QString, formula);
    QFETCH(QString, expected);

    QHash<QString, quint32> nameToId;
    nameToId.insert(QStringLiteral("A1"), 42);
    nameToId.insert(QStringLiteral("A"), 1);
    nameToId.insert(QStringLiteral("B"), 2);

    QCOMPARE(VFormulaIdTranslator::FormulaNamesToIds(formula, nameToId), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFormulaIdTranslator::TestFormulaIdsToNames_data()
{
    QTest::addColumn<QString>("formula");
    QTest::addColumn<QString>("expected");

    QTest::newRow("single id")               << "id42+5"       << "A1+5";
    QTest::newRow("two ids, tokens shrink")  << "id1+id2"       << "A+B";
    QTest::newRow("unknown id left alone")   << "id42+id99"     << "A1+id99";
    QTest::newRow("no ids at all")           << "2+2"           << "2+2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFormulaIdTranslator::TestFormulaIdsToNames()
{
    QFETCH(QString, formula);
    QFETCH(QString, expected);

    QHash<quint32, QString> idToName;
    idToName.insert(42, QStringLiteral("A1"));
    idToName.insert(1, QStringLiteral("A"));
    idToName.insert(2, QStringLiteral("B"));

    QCOMPARE(VFormulaIdTranslator::FormulaIdsToNames(formula, idToName), expected);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Proves the actual point of the feature: once a formula is stored as ids, renaming the
 * object it refers to changes nothing about the stored formula - only the resolved display text.
 */
void TST_VFormulaIdTranslator::TestRoundTripSurvivesRename()
{
    QHash<QString, quint32> nameToId;
    nameToId.insert(QStringLiteral("A1"), 42);

    const QString stored = VFormulaIdTranslator::FormulaNamesToIds(QStringLiteral("A1+5"), nameToId);
    QCOMPARE(stored, QStringLiteral("id42+5"));

    QHash<quint32, QString> idToNameBeforeRename;
    idToNameBeforeRename.insert(42, QStringLiteral("A1"));
    QCOMPARE(VFormulaIdTranslator::FormulaIdsToNames(stored, idToNameBeforeRename), QStringLiteral("A1+5"));

    QHash<quint32, QString> idToNameAfterRename;
    idToNameAfterRename.insert(42, QStringLiteral("Halsloch_hinten"));
    QCOMPARE(VFormulaIdTranslator::FormulaIdsToNames(stored, idToNameAfterRename),
             QStringLiteral("Halsloch_hinten+5"));

    // The stored formula itself must be untouched by the rename.
    QCOMPARE(stored, QStringLiteral("id42+5"));
}

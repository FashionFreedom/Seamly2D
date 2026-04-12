//-----------------------------------------------------------------------------
//  @file   tst_vmeasurements.cpp
//  @author Douglas S Caskey
//  @date   14 Jul, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  @file   tst_vmeasurements.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   16 Oct, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "tst_vmeasurements.h"

#include "../ifc/xml/multi_size_converter.h"
#include "../ifc/xml/individual_size_converter.h"
#include "../vformat/measurements.h"
#include "../vpatterndb/pmsystems.h"
#include "../vpatterndb/variables/measurement_variable.h"

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_Measurements::TST_Measurements(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyMultisizeFile check if empty multisize measurement file is valid.
 */
void TST_Measurements::CreateEmptyMultisizeFile()
{
    Unit mUnit = Unit::Cm;
    const int height = 176;
    const int size = 50;

    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(height);
    VContainer::setSize(size);

    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, size, height, data.data()));
    m->setSize(VContainer::rsize());
    m->setHeight(VContainer::rheight());

    QTemporaryFile file;
    QString fileName;
    // In Windows we have problems when we try to open QSaveFile when QTemporaryFile with the same name is already open.
    if (file.open())
    {
        // So, before we try to open file in m->SaveDocument function we need to close it and remove.
        // Just closing - is not enough, if we just close QTemporaryFile we get "access denied" in Windows.
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        const bool result = m->SaveDocument(fileName, error);

        QVERIFY2(result, error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    try
    {
        VDomDocument::ValidateXML(MultiSizeConverter::CurrentSchema, fileName);
    }
    catch (VException &error)
    {
        QFAIL(error.ErrorMessage().toUtf8().constData());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CreateEmptyIndividualFile check if empty individual measurement file is valid.
 */
void TST_Measurements::CreateEmptyIndividualFile()
{
    Unit mUnit = Unit::Cm;

    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));

    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, data.data()));

    QTemporaryFile file;
    QString fileName;
    if (file.open())
    {
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        const bool result = m->SaveDocument(fileName, error);

        QVERIFY2(result, error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    try
    {
        VDomDocument::ValidateXML(IndividualSizeConverter::CurrentSchema, fileName);
    }
    catch (VException &error)
    {
        QFAIL(error.ErrorMessage().toUtf8().constData());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ValidPMCodesMultisizeFile helps to check that all current pattern making systems match pattern inside XSD
 * scheme.
 */
void TST_Measurements::ValidPMCodesMultisizeFile()
{
    Unit mUnit = Unit::Cm;
    const int height = 176;
    const int size = 50;

    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(height);
    VContainer::setSize(size);

    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, size, height, data.data()));
    m->setSize(VContainer::rsize());
    m->setHeight(VContainer::rheight());

    const QStringList listSystems = ListPMSystems();
    for (int i = 0; i < listSystems.size(); ++i)
    {
        QString code = listSystems.at(i);
        code.remove(0, 1); // remove 'p'
        m->SetPMSystem(code);

        QTemporaryFile file;
        QString fileName;
        if (file.open())
        {
            fileName = file.fileName();
            file.close();
            file.remove();
            QString error;
            const bool result = m->SaveDocument(fileName, error);

            const QString message = QString("Error: %1 for code=%2").arg(error).arg(listSystems.at(i));
            QVERIFY2(result, qUtf8Printable(message));
        }
        else
        {
            QFAIL("Can't open temporary file.");
        }

        try
        {
            VDomDocument::ValidateXML(MultiSizeConverter::CurrentSchema, fileName);
        }
        catch (VException &error)
        {
            const QString message = QString("Error: %1 for code=%2").arg(error.ErrorMessage()).arg(listSystems.at(i));
            QFAIL(qUtf8Printable(message));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ValidPMCodesIndividualFile helps to check that all current pattern making systems match pattern inside XSD
 * scheme.
 */
void TST_Measurements::ValidPMCodesIndividualFile()
{
    Unit mUnit = Unit::Cm;

    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));

    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, data.data()));

    const QStringList listSystems = ListPMSystems();
    for (int i = 0; i < listSystems.size(); ++i)
    {
        QString code = listSystems.at(i);
        code.remove(0, 1); // remove 'p'
        m->SetPMSystem(code);

        QTemporaryFile file;
        QString fileName;
        if (file.open())
        {
            fileName = file.fileName();
            file.close();
            file.remove();
            QString error;
            const bool result = m->SaveDocument(fileName, error);

            const QString message = QString("Error: %1 for code=%2").arg(error).arg(listSystems.at(i));
            QVERIFY2(result, qUtf8Printable(message));
        }
        else
        {
            QFAIL("Can't open temporary file.");
        }

        try
        {
            VDomDocument::ValidateXML(IndividualSizeConverter::CurrentSchema, fileName);
        }
        catch (VException &error)
        {
            const QString message = QString("Error: %1 for code=%2").arg(error.ErrorMessage()).arg(listSystems.at(i));
            QFAIL(qUtf8Printable(message));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::SizeRangeDefaults()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    QCOMPARE(m->minSize(), 22);
    QCOMPARE(m->maxSize(), 72);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::setSizeRange()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m =
            QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setSizeRange(36, 52, 2);
    QCOMPARE(m->minSize(), 36);
    QCOMPARE(m->maxSize(), 52);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::setSizeRangeRejectsInvalidRange()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    // Base size is 50 — range 22-40 excludes it, should be rejected
    m->setSizeRange(22, 40, 2);
    QCOMPARE(m->minSize(), 22);  // unchanged
    QCOMPARE(m->maxSize(), 72);  // unchanged
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::HeightRangeDefaults()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    QCOMPARE(m->minHeight(), 50);
    QCOMPARE(m->maxHeight(), 200);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::setHeightRange()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setHeightRange(146, 188, 6);
    QCOMPARE(m->minHeight(), 146);
    QCOMPARE(m->maxHeight(), 188);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::setHeightRangeRejectsInvalidRange()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    // Base height is 176 — range 50-170 excludes it
    m->setHeightRange(50, 170, 6);
    QCOMPARE(m->minHeight(), 50);   // unchanged
    QCOMPARE(m->maxHeight(), 200);  // unchanged
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::activeSizesFiltered()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setSizeRange(36, 52, 2);
    const QStringList active = m->activeSizes(Unit::Cm);
    QVERIFY(active.size() < MeasurementVariable::allSizesList(Unit::Cm).size());
    QVERIFY(active.contains("36"));
    QVERIFY(active.contains("50"));
    QVERIFY(active.contains("52"));
    QVERIFY(!active.contains("22"));
    QVERIFY(!active.contains("72"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::activeHeightsFiltered()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setHeightRange(146, 188, 6);
    const QStringList active = m->activeHeights(Unit::Cm);
    QVERIFY(active.size() < MeasurementVariable::allHeightsList(Unit::Cm).size());
    QVERIFY(active.contains("146"));
    QVERIFY(active.contains("176"));
    QVERIFY(active.contains("188"));
    QVERIFY(!active.contains("50"));
    QVERIFY(!active.contains("200"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::SizeRangeRoundTripThroughFile()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setSize(VContainer::rsize());
    m->setHeight(VContainer::rheight());
    m->setSizeRange(36, 52, 2);
    m->setHeightRange(146, 188, 6);

    QTemporaryFile file;
    QString fileName;
    if (file.open())
    {
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        QVERIFY2(m->SaveDocument(fileName, error), error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    QSharedPointer<VContainer> data2 = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    QSharedPointer<MeasurementDoc> m2 = QSharedPointer<MeasurementDoc>(new MeasurementDoc(data2.data()));
    m2->setXMLContent(fileName);
    QCOMPARE(m2->minSize(), 36);
    QCOMPARE(m2->maxSize(), 52);
    QCOMPARE(m2->minHeight(), 146);
    QCOMPARE(m2->maxHeight(), 188);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Measurements::MultisizeFileWithRangeValidatesSchema()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setSize(VContainer::rsize());
    m->setHeight(VContainer::rheight());
    m->setSizeRange(36, 52, 2);
    m->setHeightRange(146, 188, 6);

    QTemporaryFile file;
    QString fileName;
    if (file.open())
    {
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        QVERIFY2(m->SaveDocument(fileName, error), error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    try
    {
        VDomDocument::ValidateXML(MultiSizeConverter::CurrentSchema, fileName);
    }
    catch (VException &error)
    {
        QFAIL(error.ErrorMessage().toUtf8().constData());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief testSizeAliasesRoundTrip verifies that size aliases set on a multisize file survive
 * a save/reload cycle intact.
 */
void TST_Measurements::testSizeAliasesRoundTrip()
{
    Unit mUnit = Unit::Cm;
    QSharedPointer<VContainer> data = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    VContainer::setHeight(176);
    VContainer::setSize(50);
    QSharedPointer<MeasurementDoc> m = QSharedPointer<MeasurementDoc>(new MeasurementDoc(mUnit, 50, 176, data.data()));
    m->setSize(VContainer::rsize());
    m->setHeight(VContainer::rheight());

    QMap<int, QString> aliases;
    aliases.insert(36, QStringLiteral("XS"));
    aliases.insert(44, QStringLiteral("S"));
    aliases.insert(50, QStringLiteral("M"));
    aliases.insert(56, QStringLiteral("L"));
    QMap<int, QString> colors;
    aliases.insert(36, QStringLiteral("#000000"));
    aliases.insert(44, QStringLiteral("#333333"));
    aliases.insert(50, QStringLiteral("#345678"));
    aliases.insert(56, QStringLiteral("#444444"));
    m->setSizes(aliases, colors);

    QTemporaryFile file;
    QString fileName;
    if (file.open())
    {
        fileName = file.fileName();
        file.close();
        file.remove();
        QString error;
        QVERIFY2(m->SaveDocument(fileName, error), error.toUtf8().constData());
    }
    else
    {
        QFAIL("Can't open temporary file.");
    }

    QSharedPointer<VContainer> data2 = QSharedPointer<VContainer>(new VContainer(nullptr, &mUnit));
    QSharedPointer<MeasurementDoc> m2 = QSharedPointer<MeasurementDoc>(new MeasurementDoc(data2.data()));
    m2->setXMLContent(fileName);

    const QMap<int, QString> reloaded = m2->sizeAliases();
    QCOMPARE(reloaded.size(), aliases.size());
    QCOMPARE(reloaded.value(36), QStringLiteral("XS"));
    QCOMPARE(reloaded.value(44), QStringLiteral("S"));
    QCOMPARE(reloaded.value(50), QStringLiteral("M"));
    QCOMPARE(reloaded.value(56), QStringLiteral("L"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief testSizeDisplayAliasFallback verifies that sizeDisplayAlias returns the numeric
 * string when no alias is present for the given value.
 */
void TST_Measurements::testSizeDisplayAliasFallback()
{
    QMap<int, QString> aliases;
    aliases.insert(44, QStringLiteral("S"));
    aliases.insert(50, QStringLiteral("M"));

    // Known label
    QCOMPARE(MeasurementDoc::sizeDisplayAlias(50, aliases), QStringLiteral("M"));

    // Unknown value — should fall back to number string
    QCOMPARE(MeasurementDoc::sizeDisplayAlias(56, aliases), QStringLiteral("56"));

    // Empty map — always falls back
    QCOMPARE(MeasurementDoc::sizeDisplayAlias(36, QMap<int, QString>()), QStringLiteral("36"));
}

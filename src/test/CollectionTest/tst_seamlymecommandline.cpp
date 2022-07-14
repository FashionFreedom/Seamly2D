/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   tst_seamlymecommandline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 9, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
 *************************************************************************/

#include "tst_seamlymecommandline.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/logging.h"

#include <QtTest>

const QString tmpTestFolder = QStringLiteral("tst_seamlyme_tmp");

//---------------------------------------------------------------------------------------------------------------------
TST_SeamlyMeCommandLine::TST_SeamlyMeCommandLine(QObject *parent)
    :AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SeamlyMeCommandLine::initTestCase()
{
    QDir tmpDir(tmpTestFolder);
    if (not tmpDir.removeRecursively())
    {
        QFAIL("Fail to remove temp directory.");
    }

    if (not CopyRecursively(QCoreApplication::applicationDirPath() + QDir::separator() + QStringLiteral("tst_seamlyme"),
                            QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder))
    {
        QFAIL("Fail to prepare files for testing.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SeamlyMeCommandLine::OpenMeasurements_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("exitCode");

    // The file doesn't exist!
    QTest::newRow("Send wrong path to a file")                                     << "wrongPath.smme"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Old individual format to new version")                          << "keiko.smme"
                                                                                   << V_EX_OK;

    QTest::newRow("Open empty file")                                               << "empty.smme"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VIT file with all know measurements (v0.3.0)")         << "all_measurements_v0.3.0.smme"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VST file with all know measurements (v0.4.0)")         << "all_measurements_v0.4.0.vst"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VST file for man ru GOST (v0.3.0).")                   << "GOST_man_ru_v0.3.0.vst"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VIT file with all know measurements (v0.3.3)")         << "all_measurements_v0.3.3.smme"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VST file with all know measurements (v0.4.2)")         << "all_measurements_v0.4.2.vst"
                                                                                   << V_EX_OK;

    QTest::newRow("Open the VST file for man ru GOST (v0.4.2).")                   << "GOST_man_ru_v0.4.2.vst"
                                                                                   << V_EX_OK;

    QTest::newRow("Broken file. Not unique name.")                                 << "broken1.smme"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Broken file. Measurement name can't be empty.")                 << "broken1.smme"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Broken file. An empty value shouldn't break a file.")           << "broken3.smme"
                                                                                   << V_EX_OK;

    QTest::newRow("Broken file. Invalid measurement name.")                        << "broken4.smme"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Empty text VIT file.")                                          << "text.smme"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Empty text VST file.")                                          << "text.vst"
                                                                                   << V_EX_NOINPUT;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SeamlyMeCommandLine::OpenMeasurements()
{
    QFETCH(QString, file);
    QFETCH(int, exitCode);

    QString error;
    const int exit = Run(exitCode, SeamlyMePath(), QStringList() << "--test"
                         << QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder + QDir::separator() +
                         file, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SeamlyMeCommandLine::cleanupTestCase()
{
    QDir tmpDir(tmpTestFolder);
    if (not tmpDir.removeRecursively())
    {
        QWARN("Fail to remove temp directory.");
    }
}

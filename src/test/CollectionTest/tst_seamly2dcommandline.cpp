/******************************************************************************
 *   @file   tst_seamly2dcommandline.cpp
 **  @author Douglas S Caskey
 **  @date   30 Nov, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   tst_seamly2dcommandline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 10, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "tst_seamly2dcommandline.h"
#include "../vmisc/vsysexits.h"
#include "../vmisc/logging.h"

#include <QtTest>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QScopeGuard>

const QString tmpTestFolder = QStringLiteral("tst_seamly2d_tmp");
const QString tmpTestCollectionFolder = QStringLiteral("tst_seamly2d_collection_tmp");

TST_Seamly2DCommandLine::TST_Seamly2DCommandLine(QObject *parent)
    :AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::initTestCase()
{
    {// Test files
        QDir tmpDir(tmpTestFolder);
        if (not tmpDir.removeRecursively())
        {
            QFAIL("Fail to remove test temp directory.");
        }

        if (not CopyRecursively(QCoreApplication::applicationDirPath() + QDir::separator() +
                                QLatin1String("tst_seamly2d"),
                                QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder))
        {
            QFAIL("Fail to prepare test files for testing.");
        }
    }

    {// Collection
        QDir tmpDir(tmpTestCollectionFolder);
        if (not tmpDir.removeRecursively())
        {
            QFAIL("Fail to remove collection temp directory.");
        }

        if (not CopyRecursively(QCoreApplication::applicationDirPath() + QDir::separator() +
                                QLatin1String("tst_seamly2d_collection"),
                                QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestCollectionFolder))
        {
            QFAIL("Fail to prepare collection files for testing.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::OpenPatterns_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<int>("exitCode");

    // The file doesn't exist!
    QTest::newRow("Send wrong path to a file")                                     << "wrongPath.sm2d"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Measurement independent empty file")                            << "empty.sm2d"
                                                                                   << V_EX_OK;

    QTest::newRow("File with invalid object type")                                 << "wrong_obj_type.sm2d"
                                                                                   << V_EX_NOINPUT;

    QTest::newRow("Empty text Seamly 2D file")                                           << "txt.sm2d"
                                                                                   << V_EX_NOINPUT;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::OpenPatterns()
{
    QFETCH(QString, file);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder;
    const int exit = Run(exitCode, Seamly2DPath(), QStringList() << "--test"
                         << tmp + QDir::separator() + file, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::ExportMode_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder;

    QTest::newRow("Issue #372")<< "issue_372.sm2d"
                               << QString("-p;;0;;-d;;%1;;-b;;output").arg(tmp)
                               << V_EX_OK;

    QTest::newRow("A file with limited gradation. Multisize measurements. Wrong data.")
            << "glimited_vst.sm2d"
            << QString("-p;;0;;-d;;%1;;--gsize;;46;;--gheight;;164;;-b;;output").arg(tmp)
            << V_EX_DATAERR;

    QTest::newRow("A file with limited gradation. Multisize measurements. Correct data.")
            << "glimited_vst.sm2d"
            << QString("-p;;0;;-d;;%1;;--gsize;;40;;--gheight;;134;;-b;;output").arg(tmp)
            << V_EX_OK;

    QTest::newRow("A file with limited gradation. Individual measurements.")
            << "glimited_vit.sm2d"
            << QString("-p;;0;;-d;;%1;;--gsize;;40;;--gheight;;134;;-b;;output").arg(tmp)
            << V_EX_DATAERR;

    QTest::newRow("A file with limited gradation. No measurements.")
            << "glimited_no_m.sm2d"
            << QString("-p;;0;;-d;;%1;;--gsize;;40;;--gheight;;134;;-b;;output").arg(tmp)
            << V_EX_DATAERR;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::ExportMode()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder;
    const QStringList arg = QStringList() << tmp + QDir::separator() + file
                                          << arguments.split(";;");
    const int exit = Run(exitCode, Seamly2DPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::TestMode_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder;

    QTest::newRow("Issue #256. Correct path.")<< "issue_256.sm2d"
                               << QString("--test")
                               << V_EX_OK;

    QTest::newRow("Issue #256. Wrong path.")<< "issue_256_wrong_path.smis"
                               << QString("--test")
                               << V_EX_NOINPUT;

    QTest::newRow("Issue #256. Correct individual measurements.")<< "issue_256.sm2d"
                               << QString("--test;;-m;;%1").arg(tmp + QDir::separator() +
                                                                QLatin1String("issue_256_correct.smis"))
                               << V_EX_OK;

    QTest::newRow("Issue #256. Wrong individual measurements.")<< "issue_256.sm2d"
                               << QString("--test;;-m;;%1").arg(tmp + QDir::separator() +
                                                                QLatin1String("issue_256_wrong.smis"))
                               << V_EX_NOINPUT;

    QTest::newRow("Issue #256. Correct multisize measurements.")<< "issue_256.sm2d"
                               << QString("--test;;-m;;%1").arg(tmp + QDir::separator() +
                                                                QLatin1String("issue_256_correct.smms"))
                               << V_EX_OK;

    QTest::newRow("Issue #256. Wrong multisize measurements.")<< "issue_256.sm2d"
                               << QString("--test;;-m;;%1").arg(tmp + QDir::separator() +
                                                                QLatin1String("issue_256_wrong.smms"))
                               << V_EX_NOINPUT;

    QTest::newRow("Issue #1275. Individual measurements missing a required measurement.")
                               << "issue_1275.sm2d"
                               << QString("--test;;-m;;%1").arg(tmp + QDir::separator() +
                                                                QLatin1String("issue_1275.smis"))
                               << V_EX_NOINPUT;

    QTest::newRow("Wrong formula.")<< "wrong_formula.sm2d"
                               << QString("--test")
                               << V_EX_DATAERR;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::TestMode()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestFolder;
    const QStringList arg = QStringList() << tmp + QDir::separator() + file
                                          << arguments.split(";;");
    const int exit = Run(exitCode, Seamly2DPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::TestOpenCollection_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestCollectionFolder;
    const QString testGOST = QString("--test;;-m;;%1").arg(tmp + QDir::separator() + QLatin1String("gost_man_ru.smms"));
    const QString keyTest = QStringLiteral("--test");

    QTest::newRow("bra")               << "bra.sm2d"               << keyTest  << V_EX_OK;
    QTest::newRow("jacket1_52-176")   << "jacket1_52-176.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("jacket2_40-146")   << "jacket2_40-146.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("jacket3_40-146")   << "jacket3_40-146.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("jacket4_40-146")   << "jacket4_40-146.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("jacket5_30-110")   << "jacket5_30-110.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("jacket6_30-110")   << "jacket6_30-110.sm2d"   << testGOST << V_EX_OK;
    QTest::newRow("pants1_52-176")    << "pants1_52-176.sm2d"    << testGOST << V_EX_OK;
    QTest::newRow("pants2_40-146")    << "pants2_40-146.sm2d"    << testGOST << V_EX_OK;
    QTest::newRow("pants7")           << "pants7.sm2d"           << testGOST << V_EX_OK;
    QTest::newRow("TShirt_test")       << "tshirt_test.sm2d"       << keyTest  << V_EX_OK;
    QTest::newRow("TestDart")          << "test_dart.sm2d"          << keyTest  << V_EX_OK;
    QTest::newRow("MaleShirt")         << "male_shirt.sm2d"         << keyTest  << V_EX_OK;
    QTest::newRow("Trousers")          << "trousers.sm2d"          << keyTest  << V_EX_OK;
    QTest::newRow("Basic block women")      << "basic_block_women-2016.sm2d" << keyTest << V_EX_OK;
    QTest::newRow("Gent Jacket with tummy") << "gent_jacket_with_tummy.sm2d" << keyTest << V_EX_OK;
    QTest::newRow("Steampunk_trousers")     << "steampunk_trousers.sm2d"     << keyTest << V_EX_OK;
    QTest::newRow("patron_blusa")      << "patron_blusa.sm2d"      << keyTest  << V_EX_OK;
    QTest::newRow("PajamaTopWrap2")    << "pajama_top_wrap2.sm2d"    << keyTest  << V_EX_OK;
    QTest::newRow("Keiko_skirt")       << "keiko_skirt.sm2d"       << keyTest  << V_EX_OK;
    QTest::newRow("pantalon_base_Eli") << "pantalon_base_Eli.sm2d" << keyTest  << V_EX_OK;
    QTest::newRow("modell_2")          << "modell_2.sm2d"          << keyTest  << V_EX_OK;
    QTest::newRow("IMK_Zhaketa")       << "imk_zhaketa_poluprilegayuschego_silueta.sm2d" << keyTest << V_EX_OK;
    QTest::newRow("moulage_05_armhole_neckline") << "moulage_05_armhole_neckline.sm2d" << keyTest << V_EX_OK;
    QTest::newRow("07_armhole_adjustment_010")  << "07_armhole_adjustment_010.sm2d"  << keyTest << V_EX_OK;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::TestOpenCollection()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + tmpTestCollectionFolder;
    const QStringList arg = QStringList() << tmp + QDir::separator() + file
                                          << arguments.split(";;");
    const int exit = Run(exitCode, Seamly2DPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::automationDataDirectory_data() const
{
    QTest::addColumn<bool>("seamlyMe");
    QTest::addColumn<QString>("scenario");
    const QStringList scenarios = {"valid", "valid-empty", "relative", "file-root", "file-settings", "file-logs",
                                   "readonly-settings", "readonly-logs"};
    for (const bool seamlyMe : {false, true})
    {
        for (const QString &scenario : scenarios)
        {
            const QByteArray name = (QString(seamlyMe ? "seamlyme-" : "seamly2d-") + scenario).toUtf8();
            QTest::newRow(name.constData()) << seamlyMe << scenario;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::automationDataDirectory()
{
    QFETCH(bool, seamlyMe);
    QFETCH(QString, scenario);
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    const QString dataDir = directory.filePath("state");
    QString requested = dataDir;
    QString protectedPath;
    QFileDevice::Permissions originalPermissions;
    const auto restorePermissions = qScopeGuard([&]()
    {
        if (!protectedPath.isEmpty())
        {
            QFile::setPermissions(protectedPath, originalPermissions);
        }
    });
    int expected = V_EX_CANTCREAT;
    if (scenario.startsWith("valid"))
    {
        expected = V_EX_OK;
        // Exercise normal startup, settings sync, folders and logging; --version exits too early.
        if (scenario == "valid")
        {
            QVERIFY(QDir().mkpath(dataDir + "/settings/Seamly2DTeam"));
            QSettings settings(dataDir + "/settings/Seamly2DTeam/qt6_" +
                               (seamlyMe ? "seamlyme.ini" : "seamly2d.ini"), QSettings::IniFormat);
            settings.setValue("configuration/locale", "en_US");
            settings.setValue("automation/sentinel", "preserved");
            settings.sync();
            QCOMPARE(settings.status(), QSettings::NoError);
        }
    }
    else if (scenario == "relative")
    {
        requested = "relative-state";
        expected = V_EX_CONFIG;
    }
    else if (scenario.startsWith("file-"))
    {
        QString target = dataDir;
        if (scenario != "file-root")
        {
            QVERIFY(QDir().mkpath(dataDir));
            target += scenario == "file-settings" ? "/settings" : "/logs";
        }
        QFile file(target);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.close();
    }
    else
    {
        protectedPath = dataDir + (scenario == "readonly-settings" ? "/settings" : "/logs/seamly2d");
        // mkpath alone would pass because all directories already exist.
        QVERIFY(QDir().mkpath(dataDir + "/settings/Seamly2DTeam"));
        QVERIFY(QDir().mkpath(dataDir + "/logs/seamly2d"));
        originalPermissions = QFileInfo(protectedPath).permissions();
        QVERIFY(QFile::setPermissions(protectedPath, QFileDevice::ReadOwner | QFileDevice::ExeOwner));
        QTemporaryFile probe(protectedPath + "/test-XXXXXX");
        if (probe.open())
        {
            QSKIP("This account/filesystem can still write despite read-only bits (e.g. Windows ACLs/root).");
        }
    }

    QProcess process;
    process.setWorkingDirectory(directory.path());
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("SEAMLY_DATA_DIR", requested);
    environment.insert("QT_QPA_PLATFORM", "offscreen");
    environment.insert("LANG", "C");
    environment.insert("LC_ALL", "C");
    process.setProcessEnvironment(environment);
    const QString fixture = QCoreApplication::applicationDirPath() + QDir::separator() +
            (seamlyMe ? QString("tst_seamlyme/empty.smis") : tmpTestCollectionFolder + "/bra.sm2d");
    process.start(seamlyMe ? SeamlyMePath() : Seamly2DPath(), QStringList() << "--test" << fixture);
    QVERIFY2(process.waitForFinished(60000), qPrintable(process.errorString()));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    const QByteArray errors = process.readAllStandardError();
    QVERIFY2(process.exitCode() == expected, errors.constData());
    if (expected != V_EX_OK)
    {
        QVERIFY(errors.contains("SEAMLY_DATA_DIR"));
        QVERIFY(!QDir(directory.filePath("relative-state")).exists());
        return;
    }

    const QStringList folders = {"templates", "measurements/individual", "measurements/multisize", "label templates"};
    for (const QString &folder : folders)
    {
        QVERIFY2(QDir(dataDir + "/" + folder).exists(), qPrintable(folder));
    }
    if (scenario == "valid")
    {
        QSettings saved(dataDir + "/settings/Seamly2DTeam/qt6_" +
                        (seamlyMe ? "seamlyme.ini" : "seamly2d.ini"), QSettings::IniFormat);
        QCOMPARE(saved.value("automation/sentinel").toString(), QString("preserved"));
    }
    if (!seamlyMe)
    {
        QVERIFY(QDir(dataDir + "/patterns").exists());
        QVERIFY(QDir(dataDir + "/layouts").exists());
        QVERIFY(QDir(dataDir + "/backups").exists());
        const QDir logs(dataDir + "/logs/seamly2d");
        const QStringList files = logs.entryList(QStringList() << "seamly2d-pid*.log", QDir::Files);
        QCOMPARE(files.size(), 1);
        QFile log(logs.filePath(files.first()));
        QVERIFY(log.open(QIODevice::ReadOnly));
        QVERIFY(log.readAll().contains("Command-line arguments:"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_Seamly2DCommandLine::aamaCuttingText()
{
    QTemporaryDir directory;
    QVERIFY(directory.isValid());
    QFile measurements(directory.filePath("measurements.smms"));
    QVERIFY(measurements.open(QIODevice::WriteOnly));
    measurements.write(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<smms><version>0.4.5</version><read-only>false</read-only><unit>cm</unit><pm_system>998</pm_system>
<size base="50"/><height base="176"/><body-measurements>
<m name="@width" base="20" height_increase="0" size_increase="1"/>
</body-measurements></smms>)xml");
    measurements.close();
    QFile pattern(directory.filePath("cutting.sm2d"));
    QVERIFY(pattern.open(QIODevice::WriteOnly));
    pattern.write(R"xml(<?xml version="1.0" encoding="UTF-8"?>
<pattern><version>0.7.5</version><unit>cm</unit><measurements>measurements.smms</measurements>
<draftBlock name="Original cutting test"><calculation>
<point id="1" type="single" name="A" x="0" y="0"/>
<point id="2" type="endLine" name="B" basePoint="1" angle="0" length="@width"/>
<point id="3" type="endLine" name="C" basePoint="2" angle="270" length="30"/>
<point id="4" type="endLine" name="D" basePoint="3" angle="180" length="@width"/>
</calculation><modeling>
<point id="5" type="modeling" idObject="1"/><point id="6" type="modeling" idObject="2"/>
<point id="7" type="modeling" idObject="3"/><point id="8" type="modeling" idObject="4"/>
</modeling><pieces><piece id="9" name="Original rectangle" version="2" inLayout="true"
seamAllowance="true" width="1"><data visible="false" quantity="4"/><nodes>
<node idObject="5" type="NodePoint"/><node idObject="6" type="NodePoint"/>
<node idObject="7" type="NodePoint"/><node idObject="8" type="NodePoint"/>
</nodes></piece></pieces></draftBlock></pattern>)xml");
    pattern.close();

    QProcess process;
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert("SEAMLY_DATA_DIR", directory.filePath("state"));
    process.setProcessEnvironment(environment);
    process.start(Seamly2DPath(), QStringList() << "--basename" << "cutting"
                  << "--destination" << directory.path() << "--format" << "20"
                  << "--exportOnlyDetails" << "--gsize" << "50" << "--gheight" << "176" << pattern.fileName());
    QVERIFY(process.waitForFinished(120000));
    QCOMPARE(process.exitStatus(), QProcess::NormalExit);
    QCOMPARE(process.exitCode(), V_EX_OK);
    QFile output(directory.filePath("cutting_pieces.dxf"));
    QVERIFY(output.open(QIODevice::ReadOnly));
    const QByteArray text = output.readAll().toUpper();
    QVERIFY(text.contains("PIECE NAME: ORIGINAL RECTANGLE"));
    QVERIFY(text.contains("QUANTITY: 4"));
    QVERIFY(text.contains("SIZE: 50"));
    QVERIFY(text.contains("UNITS: MM"));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_Seamly2DCommandLine::cleanupTestCase()
{
    {
        QDir tmpDir(tmpTestFolder);
        if (not tmpDir.removeRecursively())
        {
            QWARN("Fail to remove test temp directory.");
        }
    }

    {
        QDir tmpDir(tmpTestCollectionFolder);
        if (not tmpDir.removeRecursively())
        {
            QWARN("Fail to remove collection temp directory.");
        }
    }
}

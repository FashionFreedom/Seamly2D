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

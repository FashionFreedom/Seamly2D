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
 **  @file   tst_measurementregexp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2015
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

#include "tst_measurementregexp.h"
#include "../qmuparser/qmudef.h"

#include "../vmisc/logging.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/measurements_def.h"
#include "../ifc/ifcdef.h"

#include <QtTest>
#include <QTranslator>

const quint32 TST_MeasurementRegExp::systemCounts = 56; // count of pattern making systems

//---------------------------------------------------------------------------------------------------------------------
TST_MeasurementRegExp::TST_MeasurementRegExp(quint32 systemCode, const QString &locale, QObject *parent)
    : TST_AbstractRegExp(locale, parent)
    , m_systemCode(systemCode)
    , m_pmsTranslator(nullptr)
{
}

//---------------------------------------------------------------------------------------------------------------------
TST_MeasurementRegExp::~TST_MeasurementRegExp()
{
    delete m_pmsTranslator;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::initTestCase()
{
    if (m_systemCode > systemCounts)
    {
        QFAIL("Unexpected system code.");
    }

    if (m_locale.isEmpty())
    {
        QFAIL("Empty locale code.");
    }

    const QStringList locales = SupportedLocales();

    if (not locales.contains(m_locale))
    {
        QFAIL("Unsupported locale code.");
    }

    if (loadMeasurements(m_locale) != NoError)
    {
        const QString message = QString("Couldn't load measurements. Locale = %1")
                                        .arg(m_locale);
        QSKIP(qUtf8Printable(message));
    }

    if (LoadVariables(m_locale) != NoError)
    {
        const QString message = QString("Couldn't load variables. Locale = %1")
                                        .arg(m_locale);
        QSKIP(qUtf8Printable(message));
    }

    InitTrMs();//Very important do this after loading QM files.
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoEndLine_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoEndLine()
{
    CallTestCheckNoEndLine();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckRegExpNames_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckRegExpNames()
{
    CallTestCheckRegExpNames();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckIsNamesUnique_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckIsNamesUnique()
{
    CallTestCheckIsNamesUnique();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoOriginalNamesInTranslation_data()
{
    PrepareData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::TestCheckNoOriginalNamesInTranslation()
{
    CallTestCheckNoOriginalNamesInTranslation();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::cleanupTestCase()
{
    RemoveTrMeasurements(m_locale);
    RemoveTrVariables(m_locale);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::PrepareData()
{
    static const QStringList originalNames = AllNames();

    QTest::addColumn<QString>("originalName");

    foreach(const QString &str, originalNames)
    {
        const QString tag = QString("Locale: '%1'. Name '%2'").arg(m_locale).arg(str);
        QTest::newRow(qUtf8Printable(tag)) << str;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList TST_MeasurementRegExp::AllNames()
{
    return AllGroupNames();
}

//---------------------------------------------------------------------------------------------------------------------
int TST_MeasurementRegExp::loadMeasurements(const QString &checkedLocale)
{
    const QString path = TranslationsPath();
    const QString file = QString("measurements_%1.qm").arg(checkedLocale);

    if (QFileInfo(path+QLatin1String("/")+file).size() <= 34)
    {
        const QString message = QString("Translation for locale = %1 is empty. \nFull path: %2/%3")
                .arg(checkedLocale)
                .arg(path)
                .arg(file);
        QWARN(qUtf8Printable(message));

        return ErrorSize;
    }

    delete m_pmsTranslator;
    m_pmsTranslator = new QTranslator(this);

    if (not m_pmsTranslator->load(file, path))
    {
        const QString message = QString("Can't load translation for locale = %1. \nFull path: %2/%3")
                .arg(checkedLocale)
                .arg(path)
                .arg(file);
        QWARN(qUtf8Printable(message));

        delete m_pmsTranslator;

        return ErrorLoad;
    }

    if (not QCoreApplication::installTranslator(m_pmsTranslator))
    {
        const QString message = QString("Can't install translation for locale = %1. \nFull path: %2/%3")
                .arg(checkedLocale)
                .arg(path)
                .arg(file);
        QWARN(qUtf8Printable(message));

        delete m_pmsTranslator;

        return ErrorInstall;
    }

    return NoError;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_MeasurementRegExp::RemoveTrMeasurements(const QString &checkedLocale)
{
    if (not m_pmsTranslator.isNull())
    {
        const bool result = QCoreApplication::removeTranslator(m_pmsTranslator);

        if (result == false)
        {
            const QString message = QString("Can't remove translation for locale = %1")
                    .arg(checkedLocale);
            QWARN(qUtf8Printable(message));
        }
        delete m_pmsTranslator;
    }
}

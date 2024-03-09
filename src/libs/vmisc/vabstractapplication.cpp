/***************************************************************************
 **  @file   vabstractapplication.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vabstractapplication.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vabstractapplication.h"

#include <QDir>
#include <QLibraryInfo>
#include <QMessageLogger>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QTranslator>
#include <Qt>
#include <QtDebug>

#include "../vmisc/def.h"
#include "../vmisc/logging.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractApplication::VAbstractApplication(int &argc, char **argv)
    :QApplication(argc, argv),
      undoStack(nullptr),
      mainWindow(nullptr),
      settings(nullptr),
      qtTranslator(nullptr),
      qtxmlTranslator(nullptr),
      qtBaseTranslator(nullptr),
      appTranslator(nullptr),
      pmsTranslator(nullptr),
      _patternUnit(Unit::Cm),
      _patternType(MeasurementsType::Unknown),
      patternFilePath(),
      currentScene(nullptr),
      sceneView(nullptr),
      doc(nullptr),
      data(nullptr),
      openingPattern(false)
{
    QString rules;

#if defined(V_NO_ASSERT)
    // Ignore SSL-related warnings
    // See issue #528: Error: QSslSocket: cannot resolve SSLv2_client_method.
    rules += QLatin1String("qt.network.ssl.warning=false\n");
    // See issue #568: Certificate checking on Mac OS X.
    rules += QLatin1String("qt.network.ssl.critical=false\n"
                           "qt.network.ssl.fatal=false\n");
#endif //defined(V_NO_ASSERT)

    // cppcheck-suppress reademptycontainer
    if (!rules.isEmpty())
    {
        QLoggingCategory::setFilterRules(rules);
    }

    setAttribute(Qt::AA_UseHighDpiPixmaps);

    connect(this, &QApplication::aboutToQuit, this, [this]()
    {
        // If try to use the method QApplication::exit program can't sync settings and show warning about QApplication
        // instance. Solution is to call sync() before quit.
        // Connect this slot with VApplication::aboutToQuit.
        Settings()->sync();
    });
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractApplication::~VAbstractApplication()
{}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief translationsPath return path to the root directory that contains QM files.
 * @param locale historic, not used
 * @return path to a directory that contains QM files, default from CONFIG+=embed_translations as set in translations.pri
 */
QString VAbstractApplication::translationsPath(const QString &locale) const
{
    Q_UNUSED(locale)
    return QStringLiteral(":/i18n/");
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType VAbstractApplication::patternType() const
{
    return _patternType;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setPatternType(const MeasurementsType &patternType)
{
    _patternType = patternType;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setCurrentDocument(VAbstractPattern *doc)
{
    this->doc = doc;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractPattern *VAbstractApplication::getCurrentDocument() const
{
    SCASSERT(doc != nullptr)
    return doc;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setCurrentData(VContainer *data)
{
    this->data = data;
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *VAbstractApplication::getCurrentData() const
{
    SCASSERT(data != nullptr)
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractApplication::getOpeningPattern() const
{
    return openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setOpeningPattern()
{
    openingPattern = !openingPattern;
}

//---------------------------------------------------------------------------------------------------------------------
QWidget *VAbstractApplication::getMainWindow() const
{
    return mainWindow;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setMainWindow(QWidget *value)
{
    SCASSERT(value != nullptr)
    mainWindow = value;
}

//---------------------------------------------------------------------------------------------------------------------
QUndoStack *VAbstractApplication::getUndoStack() const
{
    return undoStack;
}

//---------------------------------------------------------------------------------------------------------------------
Unit VAbstractApplication::patternUnit() const
{
    return _patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
const Unit *VAbstractApplication::patternUnitP() const
{
    return &_patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setPatternUnit(const Unit &patternUnit)
{
    _patternUnit = patternUnit;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getSettings hide settings constructor.
 * @return pointer to class for acssesing to settings in ini file.
 */
VCommonSettings *VAbstractApplication::Settings()
{
    SCASSERT(settings != nullptr)
    return settings;
}

//---------------------------------------------------------------------------------------------------------------------
QGraphicsScene *VAbstractApplication::getCurrentScene() const
{
    SCASSERT(*currentScene != nullptr)
    return *currentScene;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setCurrentScene(QGraphicsScene **value)
{
    currentScene = value;
}

//---------------------------------------------------------------------------------------------------------------------
VMainGraphicsView *VAbstractApplication::getSceneView() const
{
    return sceneView;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::setSceneView(VMainGraphicsView *value)
{
    sceneView = value;
}

//---------------------------------------------------------------------------------------------------------------------
double VAbstractApplication::toPixel(double val) const
{
    return ToPixel(val, _patternUnit);
}

//---------------------------------------------------------------------------------------------------------------------
double VAbstractApplication::fromPixel(double pix) const
{
    return FromPixel(pix, _patternUnit);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::loadTranslations(const QString &locale)
{
    if (locale.isEmpty())
    {
        qInfo() << "Locale is empty.";
        return;
    }
    qInfo() << "Checked locale:" << locale;

    ClearTranslation();

    qtTranslator     = new QTranslator(this);
    qtxmlTranslator  = new QTranslator(this);
    qtBaseTranslator = new QTranslator(this);
    appTranslator    = new QTranslator(this);
    pmsTranslator    = new QTranslator(this);

#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    qtTranslator->load("qt_" + locale, translationsPath(locale));
    qtxmlTranslator->load("qtxmlpatterns_" + locale, translationsPath(locale));
    qtBaseTranslator->load("qtbase_" + locale, translationsPath(locale));
#else
    qtTranslator->load("qt_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qtxmlTranslator->load("qtxmlpatterns_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qtBaseTranslator->load("qtbase_" + locale, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif

    appTranslator->load("seamly2d_" + locale, translationsPath(locale));
    pmsTranslator->load("measurements_" + locale, translationsPath(locale));

    installTranslator(qtTranslator);
    installTranslator(qtxmlTranslator);
    installTranslator(qtBaseTranslator);
    installTranslator(appTranslator);
    installTranslator(pmsTranslator);

    InitTrVars();//Very important do it after load QM files.
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractApplication::ClearTranslation()
{
    if (!qtTranslator.isNull())
    {
        removeTranslator(qtTranslator);
        delete qtTranslator;
    }

    if (!qtxmlTranslator.isNull())
    {
        removeTranslator(qtxmlTranslator);
        delete qtxmlTranslator;
    }

    if (!qtBaseTranslator.isNull())
    {
        removeTranslator(qtBaseTranslator);
        delete qtBaseTranslator;
    }

    if (!appTranslator.isNull())
    {
        removeTranslator(appTranslator);
        delete appTranslator;
    }

    if (!pmsTranslator.isNull())
    {
        removeTranslator(pmsTranslator);
        delete pmsTranslator;
    }
}

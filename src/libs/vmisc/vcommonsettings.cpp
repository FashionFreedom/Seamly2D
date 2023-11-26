/***************************************************************************
 **  @file   vcommonsettings.cpp
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
 **  @file   vcommonsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
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

#include "vcommonsettings.h"

#include <QApplication>
#include <QDate>
#include <QDir>
#include <QFont>
#include <QLocale>
#include <QMessageLogger>
#include <QSound>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QVariant>
#include <QtDebug>
#include <QTextCodec>

#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "../vpatterndb/pmsystems.h"

namespace
{
const QString settingPathsIndividualMeasurements         = QStringLiteral("paths/individual_measurements");
const QString settingPathsMultisizeMeasurements          = QStringLiteral("paths/standard_measurements");
const QString settingPathsTemplates                      = QStringLiteral("paths/templates");
const QString settingPathsBodyScans                      = QStringLiteral("paths/bodyscans");
const QString settingPathsLabelTemplate                  = QStringLiteral("paths/labels");

const QString settingConfigurationCompanyName            = QStringLiteral("graphicsview/companyName");
const QString settingConfigurationContact                = QStringLiteral("graphicsview/contact");
const QString settingConfigurationAddress                = QStringLiteral("graphicsview/address");
const QString settingConfigurationCity                   = QStringLiteral("graphicsview/city");
const QString settingConfigurationState                  = QStringLiteral("graphicsview/state");
const QString settingConfigurationZipcode                = QStringLiteral("graphicsview/zipcode");
const QString settingConfigurationCountry                = QStringLiteral("graphicsview/country");
const QString settingConfigurationTelephone              = QStringLiteral("graphicsview/telephone");
const QString settingConfigurationFax                    = QStringLiteral("graphicsview/fax");
const QString settingConfigurationEmail                  = QStringLiteral("graphicsview/email");
const QString settingConfigurationWebsite                = QStringLiteral("graphicsview/website");

const QString settingConfigurationOsSeparator            = QStringLiteral("configuration/osSeparator");
const QString settingConfigurationAutosaveState          = QStringLiteral("configuration/autosave/state");
const QString settingConfigurationAutosaveTime           = QStringLiteral("configuration/autosave/time");

const QString settingConfigurationUseModeType            = QStringLiteral("configuration/autosave/useModeType");
const QString settingConfigurationUseLastExportFormat    = QStringLiteral("configuration/autosave/useLastExportFormat");
const QString settingConfigurationExportFormat           = QStringLiteral("configuration/autosave/exportFormat");

const QString settingConfigurationSendReportState        = QStringLiteral("configuration/send_report/state");
const QString settingConfigurationLocale                 = QStringLiteral("configuration/locale");
const QString settingPMSystemCode                        = QStringLiteral("configuration/pmscode");
const QString settingConfigurationUnit                   = QStringLiteral("configuration/unit");
const QString settingConfigurationConfirmItemDeletion    = QStringLiteral("configuration/confirm_item_deletion");
const QString settingConfigurationConfirmFormatRewriting = QStringLiteral("configuration/confirm_format_rewriting");
const QString settingConfigurationMoveSuffix             = QStringLiteral("configuration/moveSuffix");
const QString settingConfigurationRotateSuffix           = QStringLiteral("configuration/rotateSuffix");
const QString settingConfigurationMirrorByAxisSuffix     = QStringLiteral("configuration/mirrorByAxisSuffix");
const QString settingConfigurationMirrorByLineSuffix     = QStringLiteral("configuration/mirrorByLineSuffix");

const QString settingGraphicsViewToolBarStyle            = QStringLiteral("graphicsview/tool_bar_style");
const QString settingGraphicsViewShowToolsToolBar        = QStringLiteral("graphicsview/showToolsToolbar");
const QString settingGraphicsViewShowPointToolBar        = QStringLiteral("graphicsview/showPointToolbar");
const QString settingGraphicsViewShowLineToolBar         = QStringLiteral("graphicsview/showLineToolbar");
const QString settingGraphicsViewShowCurveToolBar        = QStringLiteral("graphicsview/showCurveToolbar");
const QString settingGraphicsViewShowArcToolBar          = QStringLiteral("graphicsview/showArcToolbar");
const QString settingGraphicsViewShowOpsToolBar          = QStringLiteral("graphicsview/showOpsToolbar");
const QString settingGraphicsViewShowPieceToolBar        = QStringLiteral("graphicsview/showPieceToolbar");
const QString settingGraphicsViewShowDetailsToolBar      = QStringLiteral("graphicsview/showDetailsToolbar");
const QString settingGraphicsViewShowLayoutToolBar       = QStringLiteral("graphicsview/showLayoutToolbar");

const QString settingGraphicsViewShowScrollBars          = QStringLiteral("graphicsview/showScrollBars");
const QString settingGraphicsViewScrollBarWidth          = QStringLiteral("graphicsview/scrollBarWidth");
const QString settingGraphicsViewScrollDuration          = QStringLiteral("graphicsview/scrollDuration");
const QString settingGraphicsViewScrollUpdateInterval    = QStringLiteral("graphicsview/scrollUpdateInterval");
const QString settingGraphicsViewScrollSpeedFactor       = QStringLiteral("graphicsview/scrollSpeedFactor");
const QString settingGraphicsViewPixelDelta              = QStringLiteral("graphicsview/pixelDelta");
const QString settingGraphicsViewAngleDelta              = QStringLiteral("graphicsview/angleDelta");
const QString settingGraphicsViewZoomModKey              = QStringLiteral("graphicsview/zoomModKey");
const QString settingGraphicsViewZoomDoubleClick         = QStringLiteral("graphicsview/zoomDoubleClick");
const QString settingGraphicsViewPanActiveSpaceKey       = QStringLiteral("graphicsview/panActiveSpaceKey");
const QString settingGraphicsViewZoomSpeedFactor         = QStringLiteral("graphicsview/zoomSpeedFactor");
const QString settingGraphicsViewExportQuality           = QStringLiteral("graphicsview/exportQuality");
const QString settingGraphicsViewZoomRBPositiveColor     = QStringLiteral("graphicsview/zoomRBPositiveColor");
const QString settingGraphicsViewZoomRBNegativeColor     = QStringLiteral("graphicsview/zoomRBNegativeColor");
const QString settingGraphicsViewPointNameColor          = QStringLiteral("graphicsview/pointNameColor");
const QString settingGraphicsViewPointNameHoverColor     = QStringLiteral("graphicsview/pointNameHoverColor");
const QString settingGraphicsViewAxisOrginColor          = QStringLiteral("graphicsview/axisOrginColor");
const QString settingGraphicsViewDefaultLineColor        = QStringLiteral("graphicsview/defaultLineColor");
const QString settingGraphicsViewDefaultLineWeight       = QStringLiteral("graphicsview/defaultLineWeight");
const QString settingGraphicsViewDefaultLineType         = QStringLiteral("graphicsview/defaultLineType");
const QString settingGraphicsViewPrimaryColor            = QStringLiteral("graphicsview/primarySupportColor");
const QString settingGraphicsViewSecondaryColor          = QStringLiteral("graphicsview/secondarySupportColor");
const QString settingGraphicsViewTertiaryColor           = QStringLiteral("graphicsview/tertiarySupportColor");

const QString settingGraphicsViewConstrainValue          = QStringLiteral("graphicsview/constrainValue");
const QString settingGraphicsViewConstrainModKey         = QStringLiteral("graphicsview/constrainModKey");

const QString settingGraphicsViewPointNameSize           = QStringLiteral("graphicsview/pointNameSize");
const QString settingGraphicsViewGuiFontSize             = QStringLiteral("graphicsview/guiFontSize");
const QString settingGraphicsViewHidePointNames          = QStringLiteral("graphicsview/hidePointNames");
const QString settingGraphicsViewShowAxisOrigin          = QStringLiteral("graphicsview/showAxisOrigin");
const QString settingGraphicsViewWireframe               = QStringLiteral("graphicsview/wireframe");
const QString settingGraphicsViewShowControlPoints       = QStringLiteral("graphicsview/showControlPoints");
const QString settingGraphicsViewShowAnchorPoints        = QStringLiteral("graphicsview/showAnchorPoints");
const QString settingGraphicsUseToolColor                = QStringLiteral("graphicsview/useToolColor");

const QString settingPatternUndo                         = QStringLiteral("pattern/undo");
const QString settingSelectionSound                      = QStringLiteral("pattern/selectionSound");
const QString settingPatternForbidFlipping               = QStringLiteral("pattern/forbidFlipping");
const QString settingPatternHideSeamLine                 = QStringLiteral("pattern/hideMainPath");

const QString settingDefaultNotchLength                  = QStringLiteral("pattern/defaultNotchLength");
const QString settingDefaultNotchWidth                   = QStringLiteral("pattern/defaultNotchWidth");
const QString settingDefaultNotchType                    = QStringLiteral("pattern/defaultNotchType");
const QString settingDefaultNotchColor                   = QStringLiteral("pattern/defaultNotchColor");
const QString settingSeamlineNotch                       = QStringLiteral("pattern/doubleNotch");
const QString settingSeamAllowanceNotch                  = QStringLiteral("pattern/showSeamAllowanceNotch");

const QString settingPatternDefaultSeamAllowance         = QStringLiteral("pattern/defaultSeamAllowance");
const QString settingDefaultSeamColor                    = QStringLiteral("pattern/defaultSeamColor");
const QString settingDefaultSeamLinetype                 = QStringLiteral("pattern/defaultSeamLinetype");
const QString settingDefaultSeamLineweight               = QStringLiteral("pattern/defaultSeamLineweight");
const QString settingDefaultCutColor                     = QStringLiteral("pattern/defaultCutColor");
const QString settingDefaultCutLinetype                  = QStringLiteral("pattern/defaultCutLinetype");
const QString settingDefaultCutLineweight                = QStringLiteral("pattern/defaultCutLineweight");
const QString settingDefaultInternalColor                = QStringLiteral("pattern/defaultInternalColor");
const QString settingDefaultInternalLinetype             = QStringLiteral("pattern/defaultInternalLinetype");
const QString settingDefaultInternalLineweight           = QStringLiteral("pattern/defaultInternalLineweight");
const QString settingDefaultCutoutColor                  = QStringLiteral("pattern/defaultCutoutColor");
const QString settingDefaultCutoutLinetype               = QStringLiteral("pattern/defaultCutoutLinetype");
const QString settingDefaultCutoutLineweight             = QStringLiteral("pattern/defaultCutoutLineweight");

const QString settingShowSeamAllowances                  = QStringLiteral("pattern/showShowSeamAllowances");
const QString settingDefaultSeamAllowanceVisibilty       = QStringLiteral("pattern/defaultSeamAllowanceVisibilty");
const QString settingShowGrainlines                      = QStringLiteral("pattern/showGrainlines");
const QString settingDefaultGrainlineVisibilty           = QStringLiteral("pattern/defaultGrainlineVisibilty");
const QString settingDefaultGrainlineLength              = QStringLiteral("pattern/defaultGrainlineLength");
const QString settingDefaultGrainlineColor               = QStringLiteral("pattern/defaultGrainlineColor");
const QString settingDefaultGrainlineLineweight          = QStringLiteral("pattern/defaultGrainlineLineweight");

const QString settingShowLabels                          = QStringLiteral("pattern/showLabels");
const QString settingShowPatternLabels                   = QStringLiteral("pattern/showPatternLabels");
const QString settingShowPieceLabels                     = QStringLiteral("pattern/showPieceLabels");
const QString settingDefaultLabelWidth                   = QStringLiteral("pattern/defaultLabelWidth");
const QString settingDefaultLabelHeight                  = QStringLiteral("pattern/defaultLabelHeight");
const QString settingDefaultLabelColor                   = QStringLiteral("pattern/defaultLabelColor");
const QString settingDefaultPatternTemplate              = QStringLiteral("pattern/defaultPatternTemplate");
const QString settingDefaultPieceTemplate                = QStringLiteral("pattern/defaultPieceTemplate");

const QString settingPatternLabelFont                    = QStringLiteral("pattern/labelFont");
const QString settingPatternGuiFont                      = QStringLiteral("pattern/guiFont");
const QString settingPatternPointNameFont                = QStringLiteral("pattern/pointNameFont");

const QString settingGeneralRecentFileList               = QStringLiteral("recentFileList");
const QString settingGeneralRestoreFileList              = QStringLiteral("restoreFileList");
const QString settingGeneralGeometry                     = QStringLiteral("geometry");
const QString settingGeneralWindowState                  = QStringLiteral("windowState");
const QString settingGeneralToolbarsState                = QStringLiteral("toolbarsState");
const QString settingPreferenceDialogSize                = QStringLiteral("preferenceDialogSize");
const QString settingToolSeamAllowanceDialogSize         = QStringLiteral("toolSeamAllowanceDialogSize");
const QString settingIncrementsDialogSize                = QStringLiteral("toolIncrementsDialogSize");
const QString settingFormulaWizardDialogSize             = QStringLiteral("formulaWizardDialogSize");
const QString settingLatestSkippedVersion                = QStringLiteral("lastestSkippedVersion");
const QString settingDateOfLastRemind                    = QStringLiteral("dateOfLastRemind");

const QString settingCSVWithHeader                       = QStringLiteral("csv/withHeader");
const QString settingCSVCodec                            = QStringLiteral("csv/withCodec");
const QString settingCSVSeparator                        = QStringLiteral("csv/withSeparator");

const QString settingLabelDateFormat                     = QStringLiteral("label/dateFormat");
const QString settingLabelUserDateFormats                = QStringLiteral("label/userDateFormats");
const QString settingLabelTimeFormat                     = QStringLiteral("label/timeFormat");
const QString settingLabelUserTimeFormats                = QStringLiteral("label/userTimeFormats");

int pointNameSize = 0;

//---------------------------------------------------------------------------------------------------------------------
QStringList ClearFormats(const QStringList &predefinedFormats, QStringList formats)
{
    for (int i = 0; i < predefinedFormats.size(); ++i)
    {
        formats.removeAll(predefinedFormats.at(i));
    }
    return formats;
}
}

static const QString commonIniFilename = QStringLiteral("common");

#if !defined(Q_OS_WIN)
const QString VCommonSettings::unixStandardSharePath = QStringLiteral("/usr/share/seamly2d");
#endif

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void SymlinkCopyDirRecursive(const QString &fromDir, const QString &toDir, bool replaceOnConflit)
{
    QDir dir;
    dir.setPath(fromDir);

    foreach (QString copyFile, dir.entryList(QDir::Files))
    {
        const QString from = fromDir + QDir::separator() + copyFile;
        QString to = toDir + QDir::separator() + copyFile;

#ifdef Q_OS_WIN
        {
            // To fix issue #702 check each not symlink if it is actually broken symlink.
            // Also trying to mimic Unix symlink. If a file eaxists do not create a symlink and remove it if exists.
            QFile fileTo(to);
            if (fileTo.exists())
            {
                if (not fileTo.rename(to + QLatin1String(".lnk")))
                {
                    QFile::remove(to + QLatin1String(".lnk"));
                    fileTo.rename(to + QLatin1String(".lnk"));
                }

                QFileInfo info(to + QLatin1String(".lnk"));
                if (info.symLinkTarget().isEmpty())
                {
                    fileTo.copy(to);
                    fileTo.remove();
                    continue; // The file already exists, skip creating shortcut
                }
            }
        }

        to = to + QLatin1String(".lnk");
#endif

        if (QFile::exists(to))
        {
            if (replaceOnConflit)
            {
                QFile::remove(to);
            }
            else
            {
                continue;
            }
        }

        QFile::link(from, to);
    }

    foreach (QString copyDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        const QString from = fromDir + QDir::separator() + copyDir;
        const QString to = toDir + QDir::separator() + copyDir;

        if (dir.mkpath(to) == false)
        {
            return;
        }

        SymlinkCopyDirRecursive(from, to, replaceOnConflit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString PrepareStandardFiles(const QString &currentPath, const QString &standardPath, const QString &defPath)
{
    QDir standardPathDir(standardPath);
    QDir currentPathDir(currentPath);
    if ((currentPath == defPath || not currentPathDir.exists()) && standardPathDir.exists())
    {
        const QDir localdata (defPath);
        if (localdata.mkpath("."))
        {
            SymlinkCopyDirRecursive(standardPath, defPath, false);
        }
        return defPath;
    }
    return currentPath;
}
}

//---------------------------------------------------------------------------------------------------------------------
VCommonSettings::VCommonSettings(Format format, Scope scope, const QString &organization,
                            const QString &application, QObject *parent)
    :QSettings(format, scope, organization, application, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::SharePath(const QString &shareItem)
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + shareItem;
#elif defined(Q_OS_MAC)
    QDir dirBundle(QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources") + shareItem);
    if (dirBundle.exists())
    {
        return dirBundle.absolutePath();
    }
    else
    {
        QDir appDir = QDir(qApp->applicationDirPath());
        appDir.cdUp();
        appDir.cdUp();
        appDir.cdUp();
        QDir dir(appDir.absolutePath() + shareItem);
        if (dir.exists())
        {
            return dir.absolutePath();
        }
        else
        {
            return VCommonSettings::unixStandardSharePath + shareItem;
        }
    }
#else // Unix
#ifdef QT_DEBUG
    return QCoreApplication::applicationDirPath() + shareItem;
#else
    QDir dir(QCoreApplication::applicationDirPath() + shareItem);
    if (dir.exists())
    {
        return dir.absolutePath();
    }
    else
    {
        return VCommonSettings::unixStandardSharePath + shareItem;
    }
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::MultisizeTablesPath()
{
    return SharePath(QStringLiteral("/tables/multisize"));
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::StandardTemplatesPath()
{
    return SharePath(QStringLiteral("/tables/templates"));
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::PrepareStandardTemplates(const QString & currentPath)
{
    return PrepareStandardFiles(currentPath, StandardTemplatesPath(), getDefaultTemplatePath());
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::prepareMultisizeTables(const QString &currentPath)
{
    return PrepareStandardFiles(currentPath, MultisizeTablesPath(), getDefaultMultisizePath());
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultIndividualSizePath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("measurements") + QLatin1String("/") + tr("individual");
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getIndividualSizePath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingPathsIndividualMeasurements, getDefaultIndividualSizePath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setIndividualSizePath(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingPathsIndividualMeasurements, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultMultisizePath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("measurements") + QLatin1String("/") + tr("multisize");
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getMultisizePath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingPathsMultisizeMeasurements, getDefaultMultisizePath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setMultisizePath(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingPathsMultisizeMeasurements, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultTemplatePath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("templates");
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getTemplatePath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingPathsTemplates, getDefaultTemplatePath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setTemplatePath(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingPathsTemplates, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultBodyScansPath()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("bodyscans");
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getBodyScansPath() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingPathsBodyScans, getDefaultBodyScansPath()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setBodyScansPath(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingPathsBodyScans, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetDefPathLabelTemplate()
{
    return QDir::homePath() + QLatin1String("/seamly2d/") + tr("label templates");
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetPathLabelTemplate() const
{
    return value(settingPathsLabelTemplate, GetDefPathLabelTemplate()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathLabelTemplate(const QString &text)
{
    setValue(settingPathsLabelTemplate, text);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultPatternTemplate() const
{
    return value(settingDefaultPatternTemplate, GetPathLabelTemplate() + "default_pattern_label.xml").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultPatternTemplate(const QString &text)
{
    setValue(settingDefaultPatternTemplate, text);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultPieceTemplate() const
{
    return value(settingDefaultPieceTemplate, GetPathLabelTemplate() + "default_piece_label.xml").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultPieceTemplate(const QString &value)
{
    setValue(settingDefaultPieceTemplate, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::GetOsSeparator() const
{
    return value(settingConfigurationOsSeparator, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetOsSeparator(const bool &value)
{
    setValue(settingConfigurationOsSeparator, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::GetAutosaveState() const
{
    return value(settingConfigurationAutosaveState, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetAutosaveState(const bool &value)
{
    setValue(settingConfigurationAutosaveState, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::getAutosaveInterval() const
{
    bool ok = false;
    int val = value(settingConfigurationAutosaveTime, 1).toInt(&ok);
    if (ok == false)
    {
        qDebug() << "Could not convert value"<<value(settingConfigurationAutosaveTime, 1)
                   << "to int. Return default value for autosave time" << 1 << "minutes.";
        val = 1;
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setAutosaveInterval(const int &value)
{
    setValue(settingConfigurationAutosaveTime, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::useModeType() const
{
    return value(settingConfigurationUseModeType, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setUseModeType(const bool &value)
{
    setValue(settingConfigurationUseModeType, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::useLastExportFormat() const
{
    return value(settingConfigurationUseLastExportFormat, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setUseLastExportFormat(const bool &value)
{
    setValue(settingConfigurationUseLastExportFormat, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getExportFormat() const
{
    return value(settingConfigurationExportFormat, "SVG").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setExportFormat(const QString &value)
{
    setValue(settingConfigurationExportFormat, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::GetSendReportState() const
{
    return value(settingConfigurationSendReportState, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetSendReportState(const bool &value)
{
    setValue(settingConfigurationSendReportState, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetLocale() const
{
    return value(settingConfigurationLocale, QLocale().name()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLocale(const QString &value)
{
    setValue(settingConfigurationLocale, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetPMSystemCode() const
{
    return value(settingPMSystemCode, "p998").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPMSystemCode(const QString &value)
{
    setValue(settingPMSystemCode, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetUnit() const
{
    return value(settingConfigurationUnit,
                 QLocale().measurementSystem() == QLocale::MetricSystem ? unitCM : unitINCH).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUnit(const QString &value)
{
    setValue(settingConfigurationUnit, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getConfirmItemDelete() const
{
    return value(settingConfigurationConfirmItemDeletion, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setConfirmItemDelete(const bool &value)
{
    setValue(settingConfigurationConfirmItemDeletion, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getConfirmFormatRewriting() const
{
    return value(settingConfigurationConfirmFormatRewriting, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setConfirmFormatRewriting(const bool &value)
{
    setValue(settingConfigurationConfirmFormatRewriting, value);
}


//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getMoveSuffix() const
{
    return value(settingConfigurationMoveSuffix, "").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setMoveSuffix(const QString &value)
{
    setValue(settingConfigurationMoveSuffix, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getRotateSuffix() const
{
    return value(settingConfigurationRotateSuffix, "").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setRotateSuffix(const QString &value)
{
    setValue(settingConfigurationRotateSuffix, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getMirrorByAxisSuffix() const
{
    return value(settingConfigurationMirrorByAxisSuffix, "").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setMirrorByAxisSuffix(const QString &value)
{
    setValue(settingConfigurationMirrorByAxisSuffix, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getMirrorByLineSuffix() const
{
    return value(settingConfigurationMirrorByLineSuffix, "").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setMirrorByLineSuffix(const QString &value)
{
    setValue(settingConfigurationMirrorByLineSuffix, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getToolBarStyle() const
{
    return value(settingGraphicsViewToolBarStyle, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setToolBarStyle(const bool &value)
{
    setValue(settingGraphicsViewToolBarStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowToolsToolBar() const
{
    return value(settingGraphicsViewShowToolsToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowToolsToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowToolsToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowPointToolBar() const
{
    return value(settingGraphicsViewShowPointToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowPointToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowPointToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowLineToolBar() const
{
    return value(settingGraphicsViewShowLineToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowLineToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowLineToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowCurveToolBar() const
{
    return value(settingGraphicsViewShowCurveToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowCurveToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowCurveToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowArcToolBar() const
{
    return value(settingGraphicsViewShowArcToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowArcToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowArcToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowOpsToolBar() const
{
    return value(settingGraphicsViewShowOpsToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowOpsToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowOpsToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowPieceToolBar() const
{
    return value(settingGraphicsViewShowPieceToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowPieceToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowPieceToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowDetailsToolBar() const
{
    return value(settingGraphicsViewShowDetailsToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowDetailsToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowDetailsToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowLayoutToolBar() const
{
    return value(settingGraphicsViewShowLayoutToolBar, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowLayoutToolBar(const bool &value)
{
    setValue(settingGraphicsViewShowLayoutToolBar, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool  VCommonSettings::getShowScrollBars() const
{
    return value(settingGraphicsViewShowScrollBars, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowScrollBars(const bool  &value)
{
    setValue(settingGraphicsViewShowScrollBars, value);
}

//---------------------------------------------------------------------------------------------------------------------
int  VCommonSettings::getScrollBarWidth() const
{
    return value(settingGraphicsViewScrollBarWidth, 10).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setScrollBarWidth(const int  &width)
{
    setValue(settingGraphicsViewScrollBarWidth, width);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::getScrollDuration() const
{
    return value(settingGraphicsViewScrollDuration, 300).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setScrollDuration(const int &duration)
{
    setValue(settingGraphicsViewScrollDuration, duration);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::getScrollUpdateInterval() const
{
    return value(settingGraphicsViewScrollUpdateInterval, 30).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setScrollUpdateInterval(const int &interval)
{
    setValue(settingGraphicsViewScrollUpdateInterval, interval);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::getScrollSpeedFactor() const
{
    return value(settingGraphicsViewScrollSpeedFactor, 10).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setScrollSpeedFactor(const int &factor)
{
    setValue(settingGraphicsViewScrollSpeedFactor, factor);
}


//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getZoomModKey() const
{
    return value(settingGraphicsViewZoomModKey, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setZoomModKey(const bool &value)
{
    setValue(settingGraphicsViewZoomModKey, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::isZoomDoubleClick() const
{
    return value(settingGraphicsViewZoomDoubleClick, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setZoomDoubleClick(const bool &value)
{
    setValue(settingGraphicsViewZoomDoubleClick, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::isPanActiveSpaceKey() const
{
    return value(settingGraphicsViewPanActiveSpaceKey, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPanActiveSpaceKey(const bool &value)
{
    setValue(settingGraphicsViewPanActiveSpaceKey, value);
}

//---------------------------------------------------------------------------------------------------------------------
int  VCommonSettings::getZoomSpeedFactor() const
{
    return value(settingGraphicsViewZoomSpeedFactor, 16).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setZoomSpeedFactor(const int  &value)
{
    setValue(settingGraphicsViewZoomSpeedFactor, value);
}

//---------------------------------------------------------------------------------------------------------------------
int  VCommonSettings::getExportQuality() const
{
    return value(settingGraphicsViewExportQuality, 75).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setExportQuality(const int  &value)
{
    setValue(settingGraphicsViewExportQuality, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getZoomRBPositiveColor() const
{
    return value(settingGraphicsViewZoomRBPositiveColor, "Blue").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setZoomRBPositiveColor(const QString &value)
{
    setValue(settingGraphicsViewZoomRBPositiveColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getZoomRBNegativeColor() const
{
    return value(settingGraphicsViewZoomRBNegativeColor, "Green").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setZoomRBNegativeColor(const QString &value)
{
    setValue(settingGraphicsViewZoomRBNegativeColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getPointNameColor() const
{
    return value(settingGraphicsViewPointNameColor, "Black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPointNameColor(const QString &value)
{
    setValue(settingGraphicsViewPointNameColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getPointNameHoverColor() const
{
    return value(settingGraphicsViewPointNameHoverColor, "Magenta").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPointNameHoverColor(const QString &value)
{
    setValue(settingGraphicsViewPointNameHoverColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getAxisOrginColor() const
{
    return value(settingGraphicsViewAxisOrginColor, "Magenta").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setAxisOrginColor(const QString &value)
{
    setValue(settingGraphicsViewAxisOrginColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultLineColor() const
{
    return value(settingGraphicsViewDefaultLineColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLineColor(const QString &value)
{
    setValue(settingGraphicsViewDefaultLineColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultLineWeight() const
{
    return value(settingGraphicsViewDefaultLineWeight, 1.20).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLineWeight(const qreal &value)
{
    setValue(settingGraphicsViewDefaultLineWeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultLineType() const
{
    return value(settingGraphicsViewDefaultLineType, "solidLine").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLineType(const QString &value)
{
    setValue(settingGraphicsViewDefaultLineType, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getPrimarySupportColor() const
{
    return value(settingGraphicsViewPrimaryColor, "Magenta").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPrimarySupportColor(const QString &value)
{
    setValue(settingGraphicsViewPrimaryColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getSecondarySupportColor() const
{
    return value(settingGraphicsViewSecondaryColor, "Forest Green").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setSecondarySupportColor(const QString &value)
{
    setValue(settingGraphicsViewSecondaryColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getTertiarySupportColor() const
{
    return value(settingGraphicsViewTertiaryColor, "Navy").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setTertiarySupportColor(const QString &value)
{
    setValue(settingGraphicsViewTertiaryColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getConstrainValue() const
{
    return value(settingGraphicsViewConstrainValue, 10.00).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setConstrainValue(const qreal &value)
{
    setValue(settingGraphicsViewConstrainValue, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getConstrainModKey() const
{
    return value(settingGraphicsViewConstrainModKey, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setConstrainModKey(const bool &value)
{
    setValue(settingGraphicsViewConstrainModKey, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getCompanyName() const
{
    return value(settingConfigurationCompanyName, "").toString();
}

void VCommonSettings::setCompanyName(const QString &value)
{
    setValue(settingConfigurationCompanyName, value);
}

QString VCommonSettings::getContact() const
{
    return value(settingConfigurationContact, "").toString();
}

void VCommonSettings::setContact(const QString &value)
{
    setValue(settingConfigurationContact, value);
}

QString VCommonSettings::getAddress() const
{
    return value(settingConfigurationAddress, "").toString();
}

void VCommonSettings::setAddress(const QString &value)
{
    setValue(settingConfigurationAddress, value);
}

QString VCommonSettings::getCity() const
{
    return value(settingConfigurationCity, "").toString();
}

void VCommonSettings::setCity(const QString &value)
{
    setValue(settingConfigurationCity, value);
}

QString VCommonSettings::getState() const
{
    return value(settingConfigurationState, "").toString();
}

void VCommonSettings::setState(const QString &value)
{
    setValue(settingConfigurationState, value);
}

QString VCommonSettings::getZipcode() const
{
    return value(settingConfigurationZipcode, "").toString();
}

void VCommonSettings::setZipcode(const QString &value)
{
    setValue(settingConfigurationZipcode, value);
}

QString VCommonSettings::getCountry() const
{
    return value(settingConfigurationCountry, "").toString();
}

void VCommonSettings::setCountry(const QString &value)
{
    setValue(settingConfigurationCountry, value);
}

QString VCommonSettings::getTelephone() const
{
    return value(settingConfigurationTelephone, "").toString();
}

void VCommonSettings::setTelephone(const QString &value)
{
    setValue(settingConfigurationTelephone, value);
}

QString VCommonSettings::getFax() const
{
    return value(settingConfigurationFax, "").toString();
}

void VCommonSettings::setFax(const QString &value)
{
    setValue(settingConfigurationFax, value);
}

QString VCommonSettings::getEmail() const
{
    return value(settingConfigurationEmail, "").toString();
}

void VCommonSettings::setEmail(const QString &value)
{
    setValue(settingConfigurationEmail, value);
}

QString VCommonSettings::getWebsite() const
{
    return value(settingConfigurationWebsite, "").toString();
}

void VCommonSettings::setWebsite(const QString &value)
{
    setValue(settingConfigurationWebsite, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::GetUndoCount() const
{
    bool ok = false;
    int val = value(settingPatternUndo, 0).toInt(&ok);
    if (ok == false)
    {
        qDebug() << "Could not convert value"<<value(settingPatternUndo, 0)
                   << "to int. Return default value for undo counts 0 (no limit).";
        val = 0;
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUndoCount(const int &value)
{
    setValue(settingPatternUndo, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getSound() const
{
    return value(settingSelectionSound, "silent").toString();
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getSelectionSound() const
{
    return QStringLiteral("qrc:/sounds/") + value(settingSelectionSound, "silent").toString() + QStringLiteral(".wav");
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setSelectionSound(const QString &value)
{
    setValue(settingSelectionSound, value);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::GetRecentFileList() const
{
    const QStringList files = value(settingGeneralRecentFileList).toStringList();
    QStringList cleared;

    for (int i = 0; i < files.size(); ++i)
    {
        if (QFileInfo(files.at(i)).exists())
        {
            cleared.append(files.at(i));
        }
    }

    return cleared;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetRecentFileList(const QStringList &value)
{
    setValue(settingGeneralRecentFileList, value);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::GetRestoreFileList() const
{
    return value(settingGeneralRestoreFileList).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetRestoreFileList(const QStringList &value)
{
    setValue(settingGeneralRestoreFileList, value);
}

//---------------------------------------------------------------------------------------------------------------------
QByteArray VCommonSettings::GetGeometry() const
{
    return value(settingGeneralGeometry).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetGeometry(const QByteArray &value)
{
    setValue(settingGeneralGeometry, value);
}

//---------------------------------------------------------------------------------------------------------------------
QByteArray VCommonSettings::GetWindowState() const
{
    return value(settingGeneralWindowState).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetWindowState(const QByteArray &value)
{
    setValue(settingGeneralWindowState, value);
}

//---------------------------------------------------------------------------------------------------------------------
QByteArray VCommonSettings::GetToolbarsState() const
{
    return value(settingGeneralToolbarsState).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetToolbarsState(const QByteArray &value)
{
    setValue(settingGeneralToolbarsState, value);
}

//---------------------------------------------------------------------------------------------------------------------
QSize VCommonSettings::getPreferenceDialogSize() const
{
    return value(settingPreferenceDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPreferenceDialogSize(const QSize& sz)
{
    setValue(settingPreferenceDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
QSize VCommonSettings::getPatternPieceDialogSize() const
{
    return value(settingToolSeamAllowanceDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPatternPieceDialogSize(const QSize &sz)
{
    setValue(settingToolSeamAllowanceDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
QSize VCommonSettings::GetFormulaWizardDialogSize() const
{
    return value(settingFormulaWizardDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetFormulaWizardDialogSize(const QSize &sz)
{
    setValue(settingFormulaWizardDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
QSize VCommonSettings::GetIncrementsDialogSize() const
{
    return value(settingIncrementsDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetIncrementsDialogSize(const QSize &sz)
{
    setValue(settingIncrementsDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::GetLatestSkippedVersion() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingLatestSkippedVersion, 0x0).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLatestSkippedVersion(int value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingLatestSkippedVersion, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
QDate VCommonSettings::GetDateOfLastRemind() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingDateOfLastRemind, QDate(1900, 1, 1)).toDate();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDateOfLastRemind(const QDate &date)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingDateOfLastRemind, date);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getForbidPieceFlipping() const
{
    return value(settingPatternForbidFlipping, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setForbidPieceFlipping(bool value)
{
    setValue(settingPatternForbidFlipping, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::isHideSeamLine() const
{
    return value(settingPatternHideSeamLine, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setHideSeamLine(bool value)
{
    setValue(settingPatternHideSeamLine, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showSeamlineNotch() const
{
    return value(settingSeamlineNotch, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowSeamlineNotch(bool value)
{
    setValue(settingSeamlineNotch, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showSeamAllowanceNotch() const
{
    return value(settingSeamAllowanceNotch, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowSeamAllowanceNotch(bool value)
{
    setValue(settingSeamAllowanceNotch, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultNotchLength() const
{
   return value(settingDefaultNotchLength, .250).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultNotchLength(const qreal &value)
{
    setValue(settingDefaultNotchLength, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultNotchWidth() const
{
   return value(settingDefaultNotchWidth, .250).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultNotchWidth(const qreal &value)
{
    setValue(settingDefaultNotchWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultNotchType() const
{
   return value(settingDefaultNotchType, "Slit").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultNotchType(const QString &value)
{
    setValue(settingDefaultNotchType, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultNotchColor() const
{
   return value(settingDefaultNotchColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultNotchColor(const QString &value)
{
    setValue(settingDefaultNotchColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVWithHeader(bool withHeader)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingCSVWithHeader, withHeader);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::GetCSVWithHeader() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingCSVWithHeader, GetDefCSVWithHeader()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::GetDefCSVWithHeader() const
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVCodec(int mib)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    settings.setValue(settingCSVCodec, mib);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::GetCSVCodec() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    return settings.value(settingCSVCodec, GetDefCSVCodec()).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::GetDefCSVCodec() const
{
    return QTextCodec::codecForLocale()->mibEnum();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVSeparator(const QChar &separator)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    switch(separator.toLatin1())
    {
        case '\t':
            settings.setValue(settingCSVSeparator, 0);
            break;
        case ';':
            settings.setValue(settingCSVSeparator, 1);
            break;
        case ' ':
            settings.setValue(settingCSVSeparator, 2);
            break;
        default:
            settings.setValue(settingCSVSeparator, 3);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QChar VCommonSettings::GetCSVSeparator() const
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), commonIniFilename);
    const quint8 separator = static_cast<quint8>(settings.value(settingCSVSeparator, 3).toUInt());
    switch(separator)
    {
        case 0:
            return QChar('\t');
        case 1:
            return QChar(';');
        case 2:
            return QChar(' ');
        default:
            return QChar(',');
    }
}

//---------------------------------------------------------------------------------------------------------------------
QChar VCommonSettings::GetDefCSVSeparator() const
{
    return QChar(',');
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDefaultSeamAllowance(double value)
{
    setValue(settingPatternDefaultSeamAllowance, UnitConvertor(value, StrToUnits(GetUnit()), Unit::Cm));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief returns the default seam allowance. The corresponding unit is the default unit.
 * @return the default seam allowance
 */
double VCommonSettings::GetDefaultSeamAllowance()
{
    double defaultValue;

    const Unit globalUnit = StrToUnits(GetUnit());

    switch (globalUnit)
    {
        case Unit::Mm:
            defaultValue = 10;
            break;
        case Unit::Inch:
            defaultValue = 0.25;
            break;
        default:
        case Unit::Cm:
            defaultValue = 1;
            break;
    }

    bool ok = false;
    double val = value(settingPatternDefaultSeamAllowance, -1).toDouble(&ok);
    if (ok == false)
    {
        qDebug() <<  "Could not convert value"<<value(settingPatternDefaultSeamAllowance, 0)
                   << "to real. Return default value for default seam allowance is " << defaultValue << ".";
        val = defaultValue;
    }

    if (val < 0)
    {
        val = defaultValue;
    }
    else
    {
        val = UnitConvertor(val, Unit::Cm, globalUnit);
    }

    return val;
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultSeamColor() const
{
   return value(settingDefaultSeamColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultSeamColor(const QString &value)
{
    setValue(settingDefaultSeamColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultSeamLinetype() const
{
   return value(settingDefaultSeamLinetype, "solidLine").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultSeamLinetype(const QString &value)
{
    setValue(settingDefaultSeamLinetype, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultSeamLineweight() const
{
   return value(settingDefaultSeamLineweight, 1.20).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultSeamLineweight(const qreal &value)
{
    setValue(settingDefaultSeamLineweight, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultCutColor() const
{
   return value(settingDefaultCutColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutColor(const QString &value)
{
    setValue(settingDefaultCutColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultCutLinetype() const
{
   return value(settingDefaultCutLinetype, "solidLine").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutLinetype(const QString &value)
{
    setValue(settingDefaultCutLinetype, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultCutLineweight() const
{
   return value(settingDefaultCutLineweight, 1.20).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutLineweight(const qreal &value)
{
    setValue(settingDefaultCutLineweight, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultInternalColor() const
{
   return value(settingDefaultInternalColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultInternalColor(const QString &value)
{
    setValue(settingDefaultInternalColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultInternalLinetype() const
{
   return value(settingDefaultInternalLinetype, "solidLine").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultInternalLinetype(const QString &value)
{
    setValue(settingDefaultInternalLinetype, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultInternalLineweight() const
{
   return value(settingDefaultInternalLineweight, 1.20).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultInternalLineweight(const qreal &value)
{
    setValue(settingDefaultInternalLineweight, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultCutoutColor() const
{
   return value(settingDefaultCutoutColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutoutColor(const QString &value)
{
    setValue(settingDefaultCutoutColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultCutoutLinetype() const
{
   return value(settingDefaultCutoutLinetype, "solidLine").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutoutLinetype(const QString &value)
{
    setValue(settingDefaultCutoutLinetype, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultCutoutLineweight() const
{
   return value(settingDefaultCutoutLineweight, 1.20).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultCutoutLineweight(const qreal &value)
{
    setValue(settingDefaultCutoutLineweight, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showSeamAllowances() const
{
    return value(settingShowSeamAllowances, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowSeamAllowances(const bool &value)
{
    setValue(settingShowSeamAllowances, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getDefaultSeamAllowanceVisibilty() const
{
    return value(settingDefaultSeamAllowanceVisibilty, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultSeamAllowanceVisibilty(const bool &value)
{
    setValue(settingDefaultSeamAllowanceVisibilty, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showGrainlines() const
{
    return value(settingShowGrainlines, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowGrainlines(const bool &value)
{
    setValue(settingShowGrainlines, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getDefaultGrainlineVisibilty() const
{
    return value(settingDefaultGrainlineVisibilty, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultGrainlineVisibilty(const bool &value)
{
    setValue(settingDefaultGrainlineVisibilty, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultGrainlineLength() const
{
   return value(settingDefaultGrainlineLength, 2).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultGrainlineLength(const qreal &value)
{
    setValue(settingDefaultGrainlineLength, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultGrainlineColor() const
{
   return value(settingDefaultGrainlineColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultGrainlineColor(const QString &value)
{
    setValue(settingDefaultGrainlineColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultGrainlineLineweight() const
{
   return value(settingDefaultGrainlineLineweight, 0.25).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultGrainlineLineweight(const qreal &value)
{
    setValue(settingDefaultGrainlineLineweight, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showLabels() const
{
    return value(settingShowLabels, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowLabels(const bool &value)
{
    setValue(settingShowLabels, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showPatternLabels() const
{
    return value(settingShowPatternLabels, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowPatternLabels(const bool &value)
{
    setValue(settingShowPatternLabels, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::showPieceLabels() const
{
    return value(settingShowPieceLabels, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowPieceLabels(const bool &value)
{
    setValue(settingShowPieceLabels, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultLabelWidth() const
{
   return value(settingDefaultLabelWidth, 3).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLabelWidth(const qreal &value)
{
    setValue(settingDefaultLabelWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VCommonSettings::getDefaultLabelHeight() const
{
   return value(settingDefaultLabelHeight, 2).toReal();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLabelHeight(const qreal &value)
{
    setValue(settingDefaultLabelHeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::getDefaultLabelColor() const
{
   return value(settingDefaultLabelColor, "black").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setDefaultLabelColor(const QString &value)
{
    setValue(settingDefaultLabelColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
QFont VCommonSettings::getLabelFont() const
{
    return qvariant_cast<QFont>(value(settingPatternLabelFont, QApplication::font()));
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setLabelFont(const QFont &f)
{
    setValue(settingPatternLabelFont, f);
}

//---------------------------------------------------------------------------------------------------------------------
QFont VCommonSettings::getGuiFont() const
{
    return qvariant_cast<QFont>(value(settingPatternGuiFont, QApplication::font()));
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setGuiFont(const QFont &f)
{
    setValue(settingPatternGuiFont, f);
}

//---------------------------------------------------------------------------------------------------------------------
QFont VCommonSettings::getPointNameFont() const
{
    return qvariant_cast<QFont>(value(settingPatternPointNameFont, QApplication::font()));
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPointNameFont(const QFont &f)
{
    setValue(settingPatternPointNameFont, f);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getHidePointNames() const
{
    return value(settingGraphicsViewHidePointNames, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setHidePointNames(bool value)
{
    setValue(settingGraphicsViewHidePointNames, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowAxisOrigin() const
{
    return value(settingGraphicsViewShowAxisOrigin, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowAxisOrigin(bool value)
{
    setValue(settingGraphicsViewShowAxisOrigin, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::isWireframe() const
{
    return value(settingGraphicsViewWireframe, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setWireframe(bool value)
{
    setValue(settingGraphicsViewWireframe, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowControlPoints() const
{
    return value(settingGraphicsViewShowControlPoints, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowControlPoints(bool value)
{
    setValue(settingGraphicsViewShowControlPoints, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getShowAnchorPoints() const
{
    return value(settingGraphicsViewShowAnchorPoints, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setShowAnchorPoints(bool value)
{
    setValue(settingGraphicsViewShowAnchorPoints, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VCommonSettings::getUseToolColor() const
{
    return value(settingGraphicsUseToolColor, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setUseToolColor(bool value)
{
    setValue(settingGraphicsUseToolColor, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VCommonSettings::getPointNameSize() const
{
    if (pointNameSize <= 0)
    {
        bool ok = false;
        pointNameSize = value(settingGraphicsViewPointNameSize, 32).toInt(&ok);
        if (not ok)
        {
            pointNameSize = 32;
        }
    }
    return pointNameSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setPointNameSize(int value)
{
    setValue(settingGraphicsViewPointNameSize, value);
    pointNameSize = value;
}

int VCommonSettings::getGuiFontSize() const
{
    return value(settingGraphicsViewGuiFontSize, 9).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::setGuiFontSize(int value)
{
    setValue(settingGraphicsViewGuiFontSize, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetLabelDateFormat() const
{
    const QString format = value(settingLabelDateFormat, VCommonSettings::PredefinedDateFormats().first()).toString();
    const QStringList allFormats = VCommonSettings::PredefinedDateFormats() + GetUserDefinedDateFormats();

    if (allFormats.contains(format))
    {
        return format;
    }
    else
    {
        return VCommonSettings::PredefinedDateFormats().first();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLabelDateFormat(const QString &format)
{
    setValue(settingLabelDateFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::PredefinedDateFormats()
{
    QStringList formats = QStringList() << "MM-dd-yyyy"
                                        << "d/M/yy"
                                        << "ddddMMMM dd, yyyy"
                                        << "dd/MM/yy"
                                        << "dd/MM/yyyy"
                                        << "MMM d, yy"
                                        << "MMM d, yyyy"
                                        << "d. MMM. yyyy"
                                        << "MMMM d, yyyy"
                                        << "d. MMMM yyyy"
                                        << "ddd, MMM d, yy"
                                        << "ddd dd/MMM yy"
                                        << "ddd, MMMM d, yyyy"
                                        << "ddddMMMM d, yyyy"
                                        << "MM-dd"
                                        << "yy-MM-dd"
                                        << "yyyy-MM-dd"
                                        << "MM/yy"
                                        << "MMM dd"
                                        << "MMMM";
    return formats;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::GetUserDefinedDateFormats() const
{
    return value(settingLabelUserDateFormats, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUserDefinedDateFormats(const QStringList &formats)
{
    setValue(settingLabelUserDateFormats, ClearFormats(VCommonSettings::PredefinedDateFormats(), formats));
}

//---------------------------------------------------------------------------------------------------------------------
QString VCommonSettings::GetLabelTimeFormat() const
{
    const QString format = value(settingLabelTimeFormat, VCommonSettings::PredefinedTimeFormats().first()).toString();
    const QStringList allFormats = VCommonSettings::PredefinedTimeFormats() + GetUserDefinedTimeFormats();

    if (allFormats.contains(format))
    {
        return format;
    }
    else
    {
        return VCommonSettings::PredefinedTimeFormats().first();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLabelTimeFormat(const QString &format)
{
    setValue(settingLabelTimeFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::PredefinedTimeFormats()
{
    QStringList formats = QStringList() << "hh:mm:ss"
                                        << "hh:mm:ss AP"
                                        << "hh:mm"
                                        << "hh:mm AP";
    return formats;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VCommonSettings::GetUserDefinedTimeFormats() const
{
    return value(settingLabelUserTimeFormats, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUserDefinedTimeFormats(const QStringList &formats)
{
    setValue(settingLabelUserTimeFormats, ClearFormats(VCommonSettings::PredefinedTimeFormats(), formats));
}

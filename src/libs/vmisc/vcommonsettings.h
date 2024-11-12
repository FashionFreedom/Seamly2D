//  @file   vcommonsettings.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2024 Seamly2D project
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

/************************************************************************
 **
 **  @file   vcommonsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#ifndef VCOMMONSETTINGS_H
#define VCOMMONSETTINGS_H

#include <QByteArray>
#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "../vlayout/vbank.h"

class VCommonSettings : public QSettings
{
    Q_OBJECT
public:
                         VCommonSettings(Format format, Scope scope, const QString &organization,
                                         const QString &application = QString(), QObject *parent = nullptr);

    static QString       SharePath(const QString &shareItem);
    static QString       MultisizeTablesPath();
    static QString       StandardTemplatesPath();

    static QString       PrepareStandardTemplates(const QString &currentPath);
    static QString       prepareMultisizeTables(const QString &currentPath);

    static QString       getDefaultIndividualSizePath();
    QString              getIndividualSizePath() const;
    void                 setIndividualSizePath(const QString &value);

    static QString       getDefaultMultisizePath();
    QString              getMultisizePath() const;
    void                 setMultisizePath(const QString &value);

    static QString       getDefaultBodyScansPath();
    QString              getBodyScansPath() const;
    void                 setBodyScansPath(const QString &value);

    static QString       getDefaultTemplatePath();
    QString              getTemplatePath() const;
    void                 setTemplatePath(const QString &value);

    static QString       getDefaultLabelTemplatePath();
    QString              getLabelTemplatePath() const;
    void                 SetPathLabelTemplate(const QString &value);

    static QString       getDefaultImageFilePath();
    QString              getImageFilePath() const;
    void                 setImageFilePath(const QString &value);

    static QString       getDefaultBackupFilePath();
    QString              getBackupFilePath() const;
    void                 setBackupFilePath(const QString &value);

    QString              getDefaultPatternTemplate() const;
    void                 setDefaultPatternTemplate(const QString &text);

    QString              getDefaultPieceTemplate() const;
    void                 setDefaultPieceTemplate(const QString &text);

    bool                 getShowWelcome() const;
    void                 setShowWelcome(const bool &value);

    bool                 getOsSeparator() const;
    void                 setOsSeparator(const bool &value);

    bool                 getConvertBackupEnabled() const;
    void                 setConvertBackupEnabled(const bool &value);

    bool                 GetAutosaveState() const;
    void                 setAutosaveState(const bool &value);

    int                  getAutosaveInterval() const;
    void                 setAutosaveInterval(const int &value);

    bool                 useModeType() const;
    void                 setUseModeType(const bool &value);

    bool                 useLastExportFormat() const;
    void                 setUseLastExportFormat(const bool &value);

    QString              getExportFormat() const;
    void                 setExportFormat(const QString &value);

    bool                 GetSendReportState() const;
    void                 SetSendReportState(const bool &value);

    QString              getLocale() const;
    void                 setLocale(const QString &value);

    QString              GetPMSystemCode() const;
    void                 SetPMSystemCode(const QString &value);

    QString              getUnit() const;
    void                 SetUnit(const QString &value);

    bool                 getConfirmItemDelete() const;
    void                 setConfirmItemDelete(const bool &value);

    bool                 getConfirmFormatRewriting() const;
    void                 setConfirmFormatRewriting(const bool &value);

    QString              getMoveSuffix() const;
    void                 setMoveSuffix(const QString &value);

    QString              getRotateSuffix() const;
    void                 setRotateSuffix(const QString &value);

    QString              getMirrorByAxisSuffix() const;
    void                 setMirrorByAxisSuffix(const QString &value);

    QString              getMirrorByLineSuffix() const;
    void                 setMirrorByLineSuffix(const QString &value);

    bool                 getToolBarStyle() const;
    void                 setToolBarStyle(const bool &value);

    bool                 getShowToolsToolBar() const;
    void                 setShowToolsToolBar(const bool &value);

    bool                 getShowPointToolBar() const;
    void                 setShowPointToolBar(const bool &value);

    bool                 getShowLineToolBar() const;
    void                 setShowLineToolBar(const bool &value);

    bool                 getShowCurveToolBar() const;
    void                 setShowCurveToolBar(const bool &value);

    bool                 getShowArcToolBar() const;
    void                 setShowArcToolBar(const bool &value);

    bool                 getShowOpsToolBar() const;
    void                 setShowOpsToolBar(const bool &value);

    bool                 getShowPieceToolBar() const;
    void                 setShowPieceToolBar(const bool &value);

    bool                 getShowDetailsToolBar() const;
    void                 setShowDetailsToolBar(const bool &value);

    bool                 getShowLayoutToolBar() const;
    void                 setShowLayoutToolBar(const bool &value);

    bool                 useSecondMonitor() const;
    void                 setUseSecondMonitor(const bool &value);

    int                  getDialogPosition() const;
    void                 setDialogPosition(const int &value);

    int                  getXOffset() const;
    void                 setXOffset(const int &value);

    int                  getYOffset() const;
    void                 setYOffset(const int &value);

    bool                 getShowScrollBars() const;
    void                 setShowScrollBars(const bool &value);

    int                  getScrollBarWidth() const;
    void                 setScrollBarWidth(const int &width);

    int                  getScrollDuration() const;
    void                 setScrollDuration(const int &duration);

    int                  getScrollUpdateInterval() const;
    void                 setScrollUpdateInterval(const int &interval);

    int                  getScrollSpeedFactor() const;
    void                 setScrollSpeedFactor(const int &factor);

    bool                 getZoomModKey() const;
    void                 setZoomModKey(const bool &value);

    bool                 isZoomDoubleClick() const;
    void                 setZoomDoubleClick(const bool &value);

    bool                 isPanActiveSpaceKey() const;
    void                 setPanActiveSpaceKey(const bool &value);

    bool                 useCurrentPen() const;
    void                 setUseCurrentPen(const bool &value);

    int                  getZoomSpeedFactor() const;
    void                 setZoomSpeedFactor(const int &factor);

    int                  getExportQuality() const;
    void                 setExportQuality(const int &value);

    QString              getZoomRBPositiveColor() const;
    void                 setZoomRBPositiveColor(const QString &value);

    QString              getZoomRBNegativeColor() const;
    void                 setZoomRBNegativeColor(const QString &value);

    QString              getPointNameColor() const;
    void                 setPointNameColor(const QString &value);

    QString              getPointNameHoverColor() const;
    void                 setPointNameHoverColor(const QString &value);

    QString              getAxisOrginColor() const;
    void                 setAxisOrginColor(const QString &value);

    QString              getDefaultLineColor() const;
    void                 setDefaultLineColor(const QString &value);

    qreal                getDefaultLineWeight() const;
    void                 setDefaultLineWeight(const qreal &value);

    QString              getDefaultLineType() const;
    void                 setDefaultLineType(const QString &value);

    QString              getPrimarySupportColor() const;
    void                 setPrimarySupportColor(const QString &value);

    QString              getSecondarySupportColor() const;
    void                 setSecondarySupportColor(const QString &value);

    QString              getTertiarySupportColor() const;
    void                 setTertiarySupportColor(const QString &value);

    qreal                getConstrainValue() const;
    void                 setConstrainValue(const qreal &value);

    bool                 getConstrainModKey() const;
    void                 setConstrainModKey(const bool &value);

    QString              getCompanyName() const;
    void                 setCompanyName(const QString &value);

    QString              getContact() const;
    void                 setContact(const QString &value);

    QString              getAddress() const;
    void                 setAddress(const QString &value);

    QString              getCity() const;
    void                 setCity(const QString &value);

    QString              getState() const;
    void                 setState(const QString &value);

    QString              getZipcode() const;
    void                 setZipcode(const QString &value);

    QString              getCountry() const;
    void                 setCountry(const QString &value);

    QString              getTelephone() const;
    void                 setTelephone(const QString &value);

    QString              getFax() const;
    void                 setFax(const QString &value);

    QString              getEmail() const;
    void                 setEmail(const QString &value);

    QString              getWebsite() const;
    void                 setWebsite(const QString &value);

    int                  GetUndoCount() const;
    void                 setUndoCount(const int &value);

    QString              getSound() const;
    QString              getSelectionSound() const;
    void                 setSelectionSound(const QString &value);

    QStringList          GetRecentFileList() const;
    void                 SetRecentFileList(const QStringList &value);

    QStringList          GetRestoreFileList() const;
    void                 SetRestoreFileList(const QStringList &value);

    QByteArray           GetGeometry() const;
    void                 SetGeometry(const QByteArray &value);

    QByteArray           GetWindowState() const;
    void                 SetWindowState(const QByteArray &value);

    QByteArray           GetToolbarsState() const;
    void                 SetToolbarsState(const QByteArray &value);

    QSize                getPreferenceDialogSize() const;
    void                 setPreferenceDialogSize(const QSize& sz);

    QSize                getPatternPieceDialogSize() const;
    void                 setPatternPieceDialogSize(const QSize& sz);

    QSize                GetFormulaWizardDialogSize() const;
    void                 SetFormulaWizardDialogSize(const QSize& sz);

    QSize                getVariablesDialogSize() const;
    void                 setVariablesDialogSize(const QSize& sz);

    int                  GetLatestSkippedVersion() const;
    void                 SetLatestSkippedVersion(int value);

    QDate                GetDateOfLastRemind() const;
    void                 SetDateOfLastRemind(const QDate &date);

    bool                 getForbidPieceFlipping() const;
    void                 setForbidPieceFlipping(bool value);

    bool                 isHideSeamLine() const;
    void                 setHideSeamLine(bool value);

    bool                 showSeamlineNotch() const;
    void                 setShowSeamlineNotch(bool value);

    bool                 showSeamAllowanceNotch() const;
    void                 setShowSeamAllowanceNotch(bool value);

    qreal                getDefaultNotchLength() const;
    void                 setDefaultNotchLength(const qreal &value);

    qreal                getDefaultNotchWidth() const;
    void                 setDefaultNotchWidth(const qreal &value);

    QString              getDefaultNotchType() const;
    void                 setDefaultNotchType(const QString &value);

    QString              getDefaultNotchColor() const;
    void                 setDefaultNotchColor(const QString &value);

    void                 SetCSVWithHeader(bool withHeader);
    bool                 GetCSVWithHeader() const;
    bool                 GetDefCSVWithHeader() const;

    void                 SetCSVCodec(int mib);
    int                  GetCSVCodec() const;
    int                  GetDefCSVCodec() const;

    void                 SetCSVSeparator(const QChar &separator);
    QChar                GetCSVSeparator() const;
    QChar                GetDefCSVSeparator() const;

    void                 SetDefaultSeamAllowance(double value);
    double               GetDefaultSeamAllowance();

    QString              getDefaultSeamColor() const;
    void                 setDefaultSeamColor(const QString &value);

    QString              getDefaultSeamLinetype() const;
    void                 setDefaultSeamLinetype(const QString &value);

    qreal                getDefaultSeamLineweight() const;
    void                 setDefaultSeamLineweight(const qreal &value);

    QString              getDefaultCutColor() const;
    void                 setDefaultCutColor(const QString &value);

    QString              getDefaultCutLinetype() const;
    void                 setDefaultCutLinetype(const QString &value);

    qreal                getDefaultCutLineweight() const;
    void                 setDefaultCutLineweight(const qreal &value);

    QString              getDefaultInternalColor() const;
    void                 setDefaultInternalColor(const QString &value);

    QString              getDefaultInternalLinetype() const;
    void                 setDefaultInternalLinetype(const QString &value);

    qreal                getDefaultInternalLineweight() const;
    void                 setDefaultInternalLineweight(const qreal &value);

    QString              getDefaultCutoutColor() const;
    void                 setDefaultCutoutColor(const QString &value);

    QString              getDefaultCutoutLinetype() const;
    void                 setDefaultCutoutLinetype(const QString &value);

    qreal                getDefaultCutoutLineweight() const;
    void                 setDefaultCutoutLineweight(const qreal &value);

    bool                 showSeamAllowances() const;
    void                 setShowSeamAllowances(const bool &value);

    bool                 getDefaultSeamAllowanceVisibilty() const;
    void                 setDefaultSeamAllowanceVisibilty(const bool &value);

    bool                 showGrainlines() const;
    void                 setShowGrainlines(const bool &value);

    bool                 getDefaultGrainlineVisibilty() const;
    void                 setDefaultGrainlineVisibilty(const bool &value);

    qreal                getDefaultGrainlineLength() const;
    void                 setDefaultGrainlineLength(const qreal &value);

    QString              getDefaultGrainlineColor() const;
    void                 setDefaultGrainlineColor(const QString &value);

    qreal                getDefaultGrainlineLineweight() const;
    void                 setDefaultGrainlineLineweight(const qreal &value);

    qreal                getDefaultArrowLength() const;
    void                 setDefaultArrowLength(const qreal &value);

    bool                 showLabels() const;
    void                 setShowLabels(const bool &value);

    bool                 showPatternLabels() const;
    void                 setShowPatternLabels(const bool &value);

    bool                 showPieceLabels() const;
    void                 setShowPieceLabels(const bool &value);

    qreal                getDefaultLabelWidth() const;
    void                 setDefaultLabelWidth(const qreal &value);

    qreal                getDefaultLabelHeight() const;
    void                 setDefaultLabelHeight(const qreal &value);

    QString              getDefaultLabelColor() const;
    void                 setDefaultLabelColor(const QString &value);

    QFont                getLabelFont() const;
    void                 setLabelFont(const QFont &f);

    QFont                getGuiFont() const;
    void                 setGuiFont(const QFont &f);

    QFont                getPointNameFont() const;
    void                 setPointNameFont(const QFont &f);

#if !defined(Q_OS_WIN)
    static const QString unixStandardSharePath;
#endif

    int                  getPointNameSize() const;
    void                 setPointNameSize(int value);

    bool                 getHidePointNames() const;
    void                 setHidePointNames(bool value);

    bool                 getShowAxisOrigin() const;
    void                 setShowAxisOrigin(bool value);

    bool                 isWireframe() const;
    void                 setWireframe(bool value);

    bool                 getShowControlPoints() const;
    void                 setShowControlPoints(bool value);

    bool                 getShowAnchorPoints() const;
    void                 setShowAnchorPoints(bool value);

    bool                 getUseToolColor() const;
    void                 setUseToolColor(bool value);

    int                  getGuiFontSize() const;
    void                 setGuiFontSize(int value);

    QString              GetLabelDateFormat() const;
    void                 SetLabelDateFormat(const QString &format);

    static QStringList   PredefinedDateFormats();
    QStringList          GetUserDefinedDateFormats() const;
    void                 SetUserDefinedDateFormats(const QStringList &formats);

    QString              GetLabelTimeFormat() const;
    void                 SetLabelTimeFormat(const QString &format);

    static QStringList   PredefinedTimeFormats();
    QStringList          GetUserDefinedTimeFormats() const;
    void                 SetUserDefinedTimeFormats(const QStringList &formats);

private:
    Q_DISABLE_COPY(VCommonSettings)
};

#endif // VCOMMONSETTINGS_H

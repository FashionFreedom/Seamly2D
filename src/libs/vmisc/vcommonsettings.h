/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file   vcommonsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
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
    static QString       PrepareMultisizeTables(const QString &currentPath);

    static QString       GetDefPathIndividualMeasurements();
    QString              GetPathIndividualMeasurements() const;
    void                 SetPathIndividualMeasurements(const QString &value);

    static QString       GetDefPathMultisizeMeasurements();
    QString              GetPathMultisizeMeasurements() const;
    void                 SetPathMultisizeMeasurements(const QString &value);

    static QString       GetDefPathTemplate();
    QString              GetPathTemplate() const;
    void                 SetPathTemplate(const QString &value);

    static QString       GetDefPathLabelTemplate();
    QString              GetPathLabelTemplate() const;
    void                 SetPathLabelTemplate(const QString &value);

    bool                 GetOsSeparator() const;
    void                 SetOsSeparator(const bool &value);

    bool                 GetAutosaveState() const;
    void                 SetAutosaveState(const bool &value);

    int                  GetAutosaveTime() const;
    void                 SetAutosaveTime(const int &value);

    bool                 GetSendReportState() const;
    void                 SetSendReportState(const bool &value);

    QString              GetLocale() const;
    void                 SetLocale(const QString &value);

    QString              GetPMSystemCode() const;
    void                 SetPMSystemCode(const QString &value);

    QString              GetUnit() const;
    void                 SetUnit(const QString &value);

    bool                 GetConfirmItemDelete() const;
    void                 SetConfirmItemDelete(const bool &value);

    bool                 GetConfirmFormatRewriting() const;
    void                 SetConfirmFormatRewriting(const bool &value);

    QString              getMoveSuffix() const;
    void                 setMoveSuffix(const QString &value);

    QString              getRotateSuffix() const;
    void                 setRotateSuffix(const QString &value);

    bool                 getToolBarStyle() const;
    void                 setToolBarStyle(const bool &value);

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

    int                  getZoomSpeedFactor() const;
    void                 setZoomSpeedFactor(const int &factor);


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

    int                  GetUndoCount() const;
    void                 SetUndoCount(const int &value);

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

    QSize                GetToolSeamAllowanceDialogSize() const;
    void                 SetToolSeamAllowanceDialogSize(const QSize& sz);

    QSize                GetFormulaWizardDialogSize() const;
    void                 SetFormulaWizardDialogSize(const QSize& sz);

    QSize                GetIncrementsDialogSize() const;
    void                 SetIncrementsDialogSize(const QSize& sz);

    int                  GetLatestSkippedVersion() const;
    void                 SetLatestSkippedVersion(int value);

    QDate                GetDateOfLastRemind() const;
    void                 SetDateOfLastRemind(const QDate &date);

    bool                 GetForbidWorkpieceFlipping() const;
    void                 SetForbidWorkpieceFlipping(bool value);

    bool                 IsHideMainPath() const;
    void                 SetHideMainPath(bool value);

    bool                 showSecondNotch() const;
    void                 setShowSecondNotch(bool value);

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

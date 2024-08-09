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
 **  @file   vsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VSETTINGS_H
#define VSETTINGS_H

#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtGlobal>
#include <QMargins>

#include "../vmisc/def.h"
#include "../vlayout/vbank.h"
#include "vcommonsettings.h"

template <class T> class QSharedPointer;

class VSettings : public VCommonSettings
{
    Q_OBJECT
public:
    VSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
              QObject *parent = nullptr);

    QString  getPointNameLanguage() const;
    void     setPointNameLanguage(const QString &value);

    static QString getDefaultPatternPath();
    QString getPatternPath() const;
    void SetPathPattern(const QString &value);

    static QString getDefaultLayoutPath();
    QString getLayoutPath() const;
    void SetPathLayout(const QString &value);

    bool GetGraphicalOutput() const;
    void SetGraphicalOutput(const bool &value);

    QString GetServer() const;
    void SetServer(const QString &value);

    bool GetServerSecure() const;
    void SetServerSecure(const bool &value);

    bool GetProxy() const;
    void SetProxy(const bool &value);

    QString GetProxyAddress() const;
    void SetProxyAddress(const QString &value);

    QString GetProxyPort() const;
    void SetProxyPort(const QString &value);

    QString GetProxyUser() const;
    void SetProxyUser(const QString &value);

    QString GetProxyPass() const;
    void SetProxyPass(const QString &value);

    QString GetUsername() const;
    void SetUsername(const QString &value);

    bool GetSavePassword() const;
    void SetSavePassword(const bool &value);

    QString GetUserPassword() const;
    void SetUserPassword(const QString &value);

    // Layout settings
    qreal GetLayoutPaperHeight() const;
    void SetLayoutPaperHeight(qreal value);

    qreal GetLayoutPaperWidth() const;
    void SetLayoutPaperWidth(qreal value);

    qreal GetLayoutShift() const;
    static qreal GetDefLayoutShift();
    void SetLayoutShift(qreal value);

    qreal GetLayoutWidth() const;
    static qreal GetDefLayoutWidth();
    void SetLayoutWidth(qreal value);

    QMarginsF GetFields(const QMarginsF &def = QMarginsF()) const;
    void SetFields(const QMarginsF &value);

    Cases GetLayoutGroup() const;
    static Cases GetDefLayoutGroup();
    void SetLayoutGroup(const Cases &value);

    bool GetLayoutRotate() const;
    static bool GetDefLayoutRotate();
    void SetLayoutRotate(bool value);

    int GetLayoutRotationIncrease() const;
    static int GetDefLayoutRotationIncrease();
    void SetLayoutRotationIncrease(int value);

    bool GetLayoutAutoCrop() const;
    static bool GetDefLayoutAutoCrop();
    void SetLayoutAutoCrop(bool value);

    bool GetLayoutSaveLength() const;
    static bool GetDefLayoutSaveLength();
    void SetLayoutSaveLength(bool value);

    bool GetLayoutUnitePages() const;
    static bool GetDefLayoutUnitePages();
    void SetLayoutUnitePages(bool value);

    bool GetIgnoreAllFields() const;
    static bool GetDefIgnoreAllFields();
    void SetIgnoreAllFields(bool value);

    bool GetStripOptimization() const;
    static bool GetDefStripOptimization();
    void SetStripOptimization(bool value);

    quint8 GetMultiplier() const;
    static quint8 GetDefMultiplier();
    void SetMultiplier(quint8 value);

    bool GetTextAsPaths() const;
    static bool GetDefTextAsPaths();
    void setTextAsPaths(bool value);

    // settings for the tiled PDFs
    QMarginsF GetTiledPDFMargins(const Unit &unit) const;
    void setTiledPDFMargins(const QMarginsF &value, const Unit &unit);

    qreal getTiledPDFPaperHeight(const Unit &unit) const;
    void setTiledPDFPaperHeight(qreal value, const Unit &unit);

    qreal getTiledPDFPaperWidth(const Unit &unit) const;
    void setTiledPDFPaperWidth(qreal value, const Unit &unit);

    PageOrientation getTiledPDFOrientation() const;
    void setTiledPDFOrientation(PageOrientation value);

private:
    Q_DISABLE_COPY(VSettings)
};

#endif // VSETTINGS_H

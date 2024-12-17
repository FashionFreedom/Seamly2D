/******************************************************************************
 *   @file   layoutsettings_dialog.h
 **  @author Douglas S Caskey
 **  @date   19 Oct, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

/************************************************************************
 **
 **  @file   layoutsettings_dialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2015
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef LAYOUT_SETTINGS_DIALOG_H
#define LAYOUT_SETTINGS_DIALOG_H

#include "abstractlayout_dialog.h"

#include "../vlayout/vbank.h"
#include "../vmisc/def.h"
#include "../ifc/ifcdef.h"

#include <QCloseEvent>
#include <QMargins>

namespace Ui
{
    class LayoutSettingsDialog;
}

class VLayoutGenerator;

class LayoutSettingsDialog : public AbstractLayoutDialog
{
    Q_OBJECT
public:
    explicit          LayoutSettingsDialog(VLayoutGenerator *generator, QWidget *parent = nullptr,
                                           bool disableSettings = false);
    virtual          ~LayoutSettingsDialog();

    qreal             GetPaperHeight() const;
    void              SetPaperHeight(qreal value);

    qreal             GetPaperWidth() const;
    void              SetPaperWidth(qreal value);

    qreal             GetShift() const;
    void              SetShift(qreal value);

    qreal             getLayoutGap() const;
    void              setLayoutGap(qreal value);

    QMarginsF         GetFields() const;
    void              SetFields(const QMarginsF &value);

    Cases             GetGroup() const;
    void              SetGroup(const Cases &value);

    bool              GetRotate() const;
    void              SetRotate(bool state);

    int               GetIncrease() const;
    bool              SetIncrease(int increase);

    bool              GetAutoCrop() const;
    void              SetAutoCrop(bool autoCrop);

    bool              IsSaveLength() const;
    void              SetSaveLength(bool save);

    bool              IsUnitePages() const;
    void              SetUnitePages(bool save);

    bool              useStripOptimization() const;
    void              setStripOptimization(bool save);

    quint8            GetMultiplier() const;
    void              SetMultiplier(const quint8 &value);

    bool              IsIgnoreAllFields() const;
    void              SetIgnoreAllFields(bool value);

    bool              isTextAsPaths() const;
    void              setTextAsPaths(bool value);

    QString           SelectedPrinter() const;

    //support functions for the command line parser which uses invisible dialog to properly build layout generator
    bool              SelectTemplate(const PaperSizeFormat& id);
    bool              SelectPaperUnit(const QString& units);
    bool              SelectLayoutUnit(const QString& units);
    qreal             LayoutToPixels(qreal value) const;
    qreal             PageToPixels(qreal value) const;
    static QString    MakeGroupsHelp();

protected:
    virtual void      showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    QSizeF            getTemplateSize(const PaperSizeFormat &tmpl, const Unit &unit) const;

public slots:
    void              DialogAccepted();

private slots:
    void              ConvertPaperSize();
    void              ConvertLayoutSize();
    void              TemplateSelected();
    void              FindTemplate();
    void              PaperSizeChanged();
    void              Swap(bool checked);
    void              RestoreDefaults();
    void              PrinterMargins();

    void              CorrectMaxFileds();
    void              IgnoreAllFields(int state);

private:
    Q_DISABLE_COPY(LayoutSettingsDialog)

    Ui::LayoutSettingsDialog *ui;
    bool              disableSettings;
    Unit              oldPaperUnit;
    Unit              oldLayoutUnit;
    VLayoutGenerator *generator;
    bool              isInitialized;
    void              InitPaperUnits();
    void              InitLayoutUnits();
    void              InitPrinter();
    QSizeF            Template();

    QMarginsF         MinPrinterFields() const;
    QMarginsF         GetDefPrinterFields() const;

    Unit              PaperUnit() const;
    Unit              LayoutUnit() const;

    void              CorrectPaperDecimals();
    void              CorrectLayoutDecimals();

    void              MinimumPaperSize();
    void              MinimumLayoutSize();

    void              ReadSettings();
    void              WriteSettings() const;

    void              SheetSize(const QSizeF &size);
    void              SetAdditionalOptions(bool value);
};

#endif // LAYOUT_SETTINGS_DIALOG_H

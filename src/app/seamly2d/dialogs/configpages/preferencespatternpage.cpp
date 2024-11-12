//  @file   preferencespatternpage.cpp
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
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
 **  @file   preferencespatternpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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
 **************************************************************************/

#include "preferencespatternpage.h"
#include "ui_preferencespatternpage.h"

#include "../dialogdatetimeformats.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../../core/application_2d.h"

#include <QComboBox>
#include <QDate>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

namespace
{
QStringList initAllStringsComboBox(QComboBox *combo)
{
    SCASSERT(combo != nullptr)

    QStringList itemsInComboBox;
    for (int index = 0; index < combo->count(); ++index)
    {
        itemsInComboBox << combo->itemText(index);
    }

    return itemsInComboBox;
}
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPatternPage::PreferencesPatternPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PreferencesPatternPage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initDefaultSeamAllowance();
    initializeLabelsTab();
    initNotches();
    initGrainlines();
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPatternPage::~PreferencesPatternPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();

    settings->SetDefaultSeamAllowance(ui->defaultSeamAllowance_DoubleSpinBox->value());
    settings->setDefaultSeamColor(ui->defaultSeamColor_ComboBox->currentData().toString());
    settings->setDefaultSeamLinetype(ui->defaultSeamLinetype_ComboBox->currentData().toString());
    settings->setDefaultSeamLineweight(ui->defaultSeamLineweight_ComboBox->currentData().toReal());
    settings->setDefaultCutColor(ui->defaultCutColor_ComboBox->currentData().toString());
    settings->setDefaultCutLinetype(ui->defaultCutLinetype_ComboBox->currentData().toString());
    settings->setDefaultCutLineweight(ui->defaultCutLineweight_ComboBox->currentData().toReal());

    settings->setDefaultInternalColor(ui->defaultInternalColor_ComboBox->currentData().toString());
    settings->setDefaultInternalLinetype(ui->defaultInternalLinetype_ComboBox->currentData().toString());
    settings->setDefaultInternalLineweight(ui->defaultInternalLineweight_ComboBox->currentData().toReal());

    settings->setDefaultCutoutColor(ui->defaultCutoutColor_ComboBox->currentData().toString());
    settings->setDefaultCutoutLinetype(ui->defaultCutoutLinetype_ComboBox->currentData().toString());
    settings->setDefaultCutoutLineweight(ui->defaultCutoutLineweight_ComboBox->currentData().toReal());

    settings->setForbidPieceFlipping(ui->forbidFlipping_CheckBox->isChecked());
    settings->setHideSeamLine(ui->hideSeamLine_CheckBox->isChecked());

    settings->setDefaultNotchType(ui->defaultNotchType_ComboBox->currentData().toString());
    settings->setDefaultNotchColor(ui->defaultNotchColor_ComboBox->currentData().toString());
    settings->setDefaultNotchLength(ui->defaultNotchLength_DoubleSpinBox->value());
    settings->setDefaultNotchWidth(ui->defaultNotchWidth_DoubleSpinBox->value());
    settings->setShowSeamAllowanceNotch(ui->showSeamAllowanceNotch_CheckBox->isChecked());
    settings->setShowSeamlineNotch(ui->showSeamlineNotch_CheckBox->isChecked());

    settings->setDefaultSeamAllowanceVisibilty(ui->showSeamAllowances_CheckBox->isChecked());
    settings->setDefaultGrainlineVisibilty(ui->showGrainlines_CheckBox->isChecked());
    settings->setDefaultGrainlineLength(ui->defaultGrainlineLength_DoubleSpinBox->value());
    settings->setDefaultGrainlineColor(ui->defaultGrainlineColor_ComboBox->currentData().toString());
    settings->setDefaultGrainlineLineweight(ui->defaultGrainlineLineweight_ComboBox->currentData().toReal());
    settings->setDefaultArrowLength(ui->defaultArrowLength_DoubleSpinBox->value());

    settings->setShowPatternLabels(ui->showPatternLabels_CheckBox->isChecked());
    settings->setShowPieceLabels(ui->showPieceLabels_CheckBox->isChecked());
    settings->setDefaultLabelWidth(ui->defaultLabelWidth_DoubleSpinBox->value());
    settings->setDefaultLabelHeight(ui->defaultLabelHeight_DoubleSpinBox->value());
    settings->setDefaultLabelColor(ui->defaultLabelColor_ComboBox->currentData().toString());
    settings->SetLabelDateFormat(ui->dateFormats_ComboBox->currentText());
    settings->SetLabelTimeFormat(ui->timeFormats_ComboBox->currentText());

    settings->SetUserDefinedDateFormats(initAllStringsComboBox(ui->dateFormats_ComboBox));
    settings->SetUserDefinedTimeFormats(initAllStringsComboBox(ui->timeFormats_ComboBox));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initDefaultSeamAllowance()
{
    ui->forbidFlipping_CheckBox->setChecked(qApp->Seamly2DSettings()->getForbidPieceFlipping());
    ui->hideSeamLine_CheckBox->setChecked(qApp->Seamly2DSettings()->isHideSeamLine());
    ui->showSeamAllowances_CheckBox->setChecked(qApp->Seamly2DSettings()->getDefaultSeamAllowanceVisibilty());
    ui->defaultSeamAllowance_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->GetDefaultSeamAllowance());
    ui->defaultSeamAllowance_DoubleSpinBox->setSuffix(" " + UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->getUnit()), true));

    int index = ui->defaultSeamColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultSeamColor());
    if (index != -1)
    {
        ui->defaultSeamColor_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultSeamLinetype_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultSeamLinetype());
    if (index != -1)
    {
        ui->defaultSeamLinetype_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultSeamLineweight_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultSeamLineweight());
    if (index != -1)
    {
        ui->defaultSeamLineweight_ComboBox->setCurrentIndex(index);
    }


    index = ui->defaultCutColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutColor());
    if (index != -1)
    {
        ui->defaultCutColor_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultCutLinetype_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutLinetype());
    if (index != -1)
    {
        ui->defaultCutLinetype_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultCutLineweight_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutLineweight());
    if (index != -1)
    {
        ui->defaultCutLineweight_ComboBox->setCurrentIndex(index);
    }

    index = ui->defaultInternalColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultInternalColor());
    if (index != -1)
    {
        ui->defaultInternalColor_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultInternalLinetype_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultInternalLinetype());
    if (index != -1)
    {
        ui->defaultInternalLinetype_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultInternalLineweight_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultInternalLineweight());
    if (index != -1)
    {
        ui->defaultInternalLineweight_ComboBox->setCurrentIndex(index);
    }

    index = ui->defaultCutoutColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutoutColor());
    if (index != -1)
    {
        ui->defaultCutoutColor_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultCutoutLinetype_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutoutLinetype());
    if (index != -1)
    {
        ui->defaultCutoutLinetype_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultCutoutLineweight_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultCutoutLineweight());
    if (index != -1)
    {
        ui->defaultCutoutLineweight_ComboBox->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::editDateTimeFormats()
{
    VSettings *settings = qApp->Seamly2DSettings();

    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button == ui->editDateFormats_PushButton)
    {
        callDateTimeFormatEditor(QDate::currentDate(), settings->PredefinedDateFormats(),
                                 settings->GetUserDefinedDateFormats(), ui->dateFormats_ComboBox);
    }
    else if (button == ui->editTimeFormats_PushButton)
    {
        callDateTimeFormatEditor(QTime::currentTime(), settings->PredefinedTimeFormats(),
                                 settings->GetUserDefinedTimeFormats(), ui->timeFormats_ComboBox);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::setDefaultTemplate()
{
    QToolButton *button = qobject_cast<QToolButton *>(sender());
    VSettings *settings = qApp->Seamly2DSettings();

    QString filter(tr("Label template") + QLatin1String("(*.xml)"));
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Import template"),
                                                          settings->getLabelTemplatePath(), filter, nullptr,
                                                          QFileDialog::DontUseNativeDialog);


    if (button == ui->patternTemplate_ToolButton)
    {
        ui->patternTemplate_LineEdit->setText(fileName);
        settings->setDefaultPatternTemplate(fileName);
    }
    else if (button == ui->pieceTemplate_ToolButton)
    {
        ui->pieceTemplate_LineEdit->setText(fileName);
        settings->setDefaultPieceTemplate(fileName);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initializeLabelsTab()
{
    VSettings *settings = qApp->Seamly2DSettings();

    ui->showPatternLabels_CheckBox->setChecked(qApp->Seamly2DSettings()->showPatternLabels());
    ui->showPieceLabels_CheckBox->setChecked(qApp->Seamly2DSettings()->showPieceLabels());

    ui->defaultLabelWidth_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultLabelWidth());
    ui->defaultLabelWidth_DoubleSpinBox->setSuffix(" " + UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->getUnit()), true));
    ui->defaultLabelHeight_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultLabelHeight());
    ui->defaultLabelHeight_DoubleSpinBox->setSuffix(" " + UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->getUnit()), true));

    int index = ui->defaultLabelColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultLabelColor());
    if (index != -1)
    {
        ui->defaultLabelColor_ComboBox->setCurrentIndex(index);
    }

    initComboBoxFormats(ui->dateFormats_ComboBox,
                        VSettings::PredefinedDateFormats() + settings->GetUserDefinedDateFormats(),
                        settings->GetLabelDateFormat());
    initComboBoxFormats(ui->timeFormats_ComboBox,
                        VSettings::PredefinedTimeFormats() + settings->GetUserDefinedTimeFormats(),
                        settings->GetLabelTimeFormat());

    connect(ui->editDateFormats_PushButton, &QPushButton::clicked, this, &PreferencesPatternPage::editDateTimeFormats);
    connect(ui->editTimeFormats_PushButton, &QPushButton::clicked, this, &PreferencesPatternPage::editDateTimeFormats);

    ui->patternTemplate_LineEdit->setText(settings->getDefaultPatternTemplate());
    ui->pieceTemplate_LineEdit->setText(settings->getDefaultPieceTemplate());
    connect(ui->patternTemplate_ToolButton, &QToolButton::clicked, this, &PreferencesPatternPage::setDefaultTemplate);
    connect(ui->pieceTemplate_ToolButton,   &QToolButton::clicked, this, &PreferencesPatternPage::setDefaultTemplate);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initNotches()
{
    const Unit units = StrToUnits(qApp->Seamly2DSettings()->getUnit());
    switch (units)
    {
        case Unit::Cm:
            {
                ui->defaultNotchLength_DoubleSpinBox->setMaximum(4);
                ui->defaultNotchWidth_DoubleSpinBox->setMaximum(1.25);
                break;
            }
        case Unit::Mm:
            {
                ui->defaultNotchLength_DoubleSpinBox->setMaximum(40);
                ui->defaultNotchWidth_DoubleSpinBox->setMaximum(12.50);
                break;
            }
        case Unit::Inch:
        default:
            {
                ui->defaultNotchLength_DoubleSpinBox->setMaximum(1.50);
                ui->defaultNotchWidth_DoubleSpinBox->setMaximum(.50);
                break;
            }
    }

    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("Slit"),       "slit");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("T Notch"),    "tNotch");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("U Notch"),    "uNotch");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("V Internal"), "vInternal");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("V External"), "vExternal");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("Castle"),     "castle");
    ui->defaultNotchType_ComboBox->addItem(QIcon(), tr("Diamond"),    "diamond");

    //-----------------------  Get Default Notch
    int index = ui->defaultNotchType_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultNotchType());
    if (index != -1)
    {
        ui->defaultNotchType_ComboBox->setCurrentIndex(index);
    }

    index = ui->defaultNotchColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultNotchColor());
    if (index != -1)
    {
        ui->defaultNotchColor_ComboBox->setCurrentIndex(index);
    }
    ui->showSeamlineNotch_CheckBox->setChecked(qApp->Seamly2DSettings()->showSeamlineNotch());
    ui->showSeamAllowanceNotch_CheckBox->setChecked(qApp->Seamly2DSettings()->showSeamAllowanceNotch());
    ui->defaultNotchLength_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultNotchLength());
    ui->defaultNotchLength_DoubleSpinBox->setSuffix(" " + UnitsToStr(units, true));
    ui->defaultNotchWidth_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultNotchWidth());
    ui->defaultNotchWidth_DoubleSpinBox->setSuffix(" " + UnitsToStr(units, true));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initGrainlines()
{
    ui->showGrainlines_CheckBox->setChecked(qApp->Seamly2DSettings()->getDefaultGrainlineVisibilty());

    ui->defaultGrainlineLength_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultGrainlineLength());
    ui->defaultGrainlineLength_DoubleSpinBox->setSuffix(" " + UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->getUnit()), true));

    int index = ui->defaultGrainlineColor_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultGrainlineColor());
    if (index != -1)
    {
        ui->defaultGrainlineColor_ComboBox->setCurrentIndex(index);
    }
    index = ui->defaultGrainlineLineweight_ComboBox->findData(qApp->Seamly2DSettings()->getDefaultGrainlineLineweight());
    if (index != -1)
    {
        ui->defaultGrainlineLineweight_ComboBox->setCurrentIndex(index);
    }

    ui->defaultArrowLength_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultArrowLength());
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat)
{
    SCASSERT(box != nullptr)

    box->addItems(items);
    int index = box->findText(currentFormat);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    else
    {
        box->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void PreferencesPatternPage::callDateTimeFormatEditor(const T &type, const QStringList &predefinedFormats,
                                                      const QStringList &userDefinedFormats, QComboBox *box)
{
    SCASSERT(box != nullptr)

    DialogDateTimeFormats dialog(type, predefinedFormats, userDefinedFormats);

    if (QDialog::Accepted == dialog.exec())
    {
        const QString currentFormat = box->currentText();
        box->clear();
        box->addItems(dialog.GetFormats());

        int index = box->findText(currentFormat);
        if (index != -1)
        {
            box->setCurrentIndex(index);
        }
        else
        {
            box->setCurrentIndex(0);
        }
    }
}

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

 **************************************************************************
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
#include "../../core/vapplication.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../dialogdatetimeformats.h"

#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QComboBox>

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
    ui->graphOutput_CheckBox->setChecked(qApp->Seamly2DSettings()->GetGraphicalOutput());
    ui->undoCount_SpinBox->setValue(qApp->Seamly2DSettings()->GetUndoCount());

    initDefaultSeamAllowance();
    initLabelDateTimeFormats();
    initNotches();

    ui->forbidFlipping_CheckBox->setChecked(qApp->Seamly2DSettings()->GetForbidWorkpieceFlipping());
    ui->showSecondNotch_CheckBox->setChecked(qApp->Seamly2DSettings()->showSecondNotch());
    ui->hideMainPath_CheckBox->setChecked(qApp->Seamly2DSettings()->IsHideMainPath());
    ui->labelFont_ComboBox->setCurrentFont(qApp->Seamly2DSettings()->GetLabelFont());
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPatternPage::~PreferencesPatternPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();

    // Scene antialiasing
    settings->SetGraphicalOutput(ui->graphOutput_CheckBox->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::Antialiasing, ui->graphOutput_CheckBox->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::SmoothPixmapTransform, ui->graphOutput_CheckBox->isChecked());

    /* Maximum number of commands in undo stack may only be set when the undo stack is empty, since setting it on a
     * non-empty stack might delete the command at the current index. Calling setUndoLimit() on a non-empty stack
     * prints a warning and does nothing.*/
    settings->SetUndoCount(ui->undoCount_SpinBox->value());

    settings->SetDefaultSeamAllowance(ui->defaultSeamAllowance_DoubleSpinBox->value());

    settings->SetForbidWorkpieceFlipping(ui->forbidFlipping_CheckBox->isChecked());
    settings->SetHideMainPath(ui->hideMainPath_CheckBox->isChecked());
    settings->SetLabelFont(ui->labelFont_ComboBox->currentFont());

    settings->setDefaultNotchType(ui->defaultNotchType_ComboBox->currentData().toString());
    settings->setDefaultNotchLength(ui->defaultNotchLength_DoubleSpinBox->value());
    settings->setDefaultNotchWidth(ui->defaultNotchWidth_DoubleSpinBox->value());
    if (settings->showSecondNotch() != ui->showSecondNotch_CheckBox->isChecked())
    {
        settings->setShowSecondNotch(ui->showSecondNotch_CheckBox->isChecked());
        qApp->getCurrentDocument()->LiteParseTree(Document::LiteParse);
    }

    settings->SetLabelDateFormat(ui->dateFormats_ComboBox->currentText());
    settings->SetLabelTimeFormat(ui->timeFormats_ComboBox->currentText());

    settings->SetUserDefinedDateFormats(initAllStringsComboBox(ui->dateFormats_ComboBox));
    settings->SetUserDefinedTimeFormats(initAllStringsComboBox(ui->timeFormats_ComboBox));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initDefaultSeamAllowance()
{
    ui->defaultSeamAllowance_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->GetDefaultSeamAllowance());
    ui->defaultSeamAllowance_DoubleSpinBox->setSuffix(UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->GetUnit()), true));
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
void PreferencesPatternPage::initLabelDateTimeFormats()
{
    VSettings *settings = qApp->Seamly2DSettings();

    initComboBoxFormats(ui->dateFormats_ComboBox,
                        VSettings::PredefinedDateFormats() + settings->GetUserDefinedDateFormats(),
                        settings->GetLabelDateFormat());
    initComboBoxFormats(ui->timeFormats_ComboBox,
                        VSettings::PredefinedTimeFormats() + settings->GetUserDefinedTimeFormats(),
                        settings->GetLabelTimeFormat());

    connect(ui->editDateFormats_PushButton, &QPushButton::clicked, this, &PreferencesPatternPage::editDateTimeFormats);
    connect(ui->editTimeFormats_PushButton, &QPushButton::clicked, this, &PreferencesPatternPage::editDateTimeFormats);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::initNotches()
{
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
    ui->showSecondNotch_CheckBox->setChecked(qApp->Seamly2DSettings()->showSecondNotch());
    ui->defaultNotchLength_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultNotchLength());
    ui->defaultNotchWidth_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getDefaultNotchWidth());
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

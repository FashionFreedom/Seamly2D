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
 *************************************************************************/

#include "preferencespatternpage.h"
#include "ui_preferencespatternpage.h"
#include "../../core/vapplication.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../dialogdatetimeformats.h"

#include <QMessageBox>
#include <QDate>
#include <QTime>

namespace
{
QStringList ComboBoxAllStrings(QComboBox *combo)
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
    ui->graphOutputCheck->setChecked(qApp->Seamly2DSettings()->GetGraphicalOutput());
    ui->undoCount->setValue(qApp->Seamly2DSettings()->GetUndoCount());

    InitDefaultSeamAllowance();
    InitLabelDateTimeFormats();

    ui->forbidFlippingCheck->setChecked(qApp->Seamly2DSettings()->GetForbidWorkpieceFlipping());
    ui->doublePassmarkCheck->setChecked(qApp->Seamly2DSettings()->IsDoublePassmark());
    ui->checkBoxHideMainPath->setChecked(qApp->Seamly2DSettings()->IsHideMainPath());
    ui->fontComboBoxLabelFont->setCurrentFont(qApp->Seamly2DSettings()->GetLabelFont());
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
    settings->SetGraphicalOutput(ui->graphOutputCheck->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::Antialiasing, ui->graphOutputCheck->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::SmoothPixmapTransform, ui->graphOutputCheck->isChecked());

    /* Maximum number of commands in undo stack may only be set when the undo stack is empty, since setting it on a
     * non-empty stack might delete the command at the current index. Calling setUndoLimit() on a non-empty stack
     * prints a warning and does nothing.*/
    settings->SetUndoCount(ui->undoCount->value());

    settings->SetDefaultSeamAllowance(ui->defaultSeamAllowance->value());

    settings->SetForbidWorkpieceFlipping(ui->forbidFlippingCheck->isChecked());
    settings->SetHideMainPath(ui->checkBoxHideMainPath->isChecked());
    settings->SetLabelFont(ui->fontComboBoxLabelFont->currentFont());

    if (settings->IsDoublePassmark() != ui->doublePassmarkCheck->isChecked())
    {
        settings->SetDoublePassmark(ui->doublePassmarkCheck->isChecked());
        qApp->getCurrentDocument()->LiteParseTree(Document::LiteParse);
    }

    settings->SetLabelDateFormat(ui->comboBoxDateFormats->currentText());
    settings->SetLabelTimeFormat(ui->comboBoxTimeFormats->currentText());

    settings->SetUserDefinedDateFormats(ComboBoxAllStrings(ui->comboBoxDateFormats));
    settings->SetUserDefinedTimeFormats(ComboBoxAllStrings(ui->comboBoxTimeFormats));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitDefaultSeamAllowance()
{
    ui->defaultSeamAllowance->setValue(qApp->Seamly2DSettings()->GetDefaultSeamAllowance());
    ui->defaultSeamAllowance->setSuffix(UnitsToStr(StrToUnits(qApp->Seamly2DSettings()->GetUnit()), true));
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::EditDateTimeFormats()
{
    VSettings *settings = qApp->Seamly2DSettings();

    QPushButton *button = qobject_cast<QPushButton *>(sender());
    if (button == ui->pushButtonEditDateFormats)
    {
        CallDateTimeFormatEditor(QDate::currentDate(), settings->PredefinedDateFormats(),
                           settings->GetUserDefinedDateFormats(), ui->comboBoxDateFormats);
    }
    else if (button == ui->pushButtonEditTimeFormats)
    {
        CallDateTimeFormatEditor(QTime::currentTime(), settings->PredefinedTimeFormats(),
                           settings->GetUserDefinedTimeFormats(), ui->comboBoxTimeFormats);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitLabelDateTimeFormats()
{
    VSettings *settings = qApp->Seamly2DSettings();

    InitComboBoxFormats(ui->comboBoxDateFormats,
                        VSettings::PredefinedDateFormats() + settings->GetUserDefinedDateFormats(),
                        settings->GetLabelDateFormat());
    InitComboBoxFormats(ui->comboBoxTimeFormats,
                        VSettings::PredefinedTimeFormats() + settings->GetUserDefinedTimeFormats(),
                        settings->GetLabelTimeFormat());

    connect(ui->pushButtonEditDateFormats, &QPushButton::clicked, this, &PreferencesPatternPage::EditDateTimeFormats);
    connect(ui->pushButtonEditTimeFormats, &QPushButton::clicked, this, &PreferencesPatternPage::EditDateTimeFormats);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPatternPage::InitComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat)
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
void PreferencesPatternPage::CallDateTimeFormatEditor(const T &type, const QStringList &predefinedFormats,
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

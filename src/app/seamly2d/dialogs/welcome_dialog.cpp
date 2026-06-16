//  @file   welcome_dialog.cpp
//  @author Douglas S Caskey
//  @date   5 Jan, 2024
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2023 Seamly2D project
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

#include "welcome_dialog.h"
#include "ui_welcome_dialog.h"

#include "../core/application_2d.h"
#include "../qmuparser/qmudef.h"

#include <QPushButton>
#include <QShowEvent>
#include <QSoundEffect>


//---------------------------------------------------------------------------------------------------------------------
SeamlyWelcomeDialog::SeamlyWelcomeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeamlyWelcomeDialog)
    , m_selectionSoundChanged(false)
    , settings(qApp->Seamly2DSettings())
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //-------------------- Units setup
    initUnits();

    //-------------------- Decimal separator setup
    if (settings->getOsSeparator())
    {
        ui->userLocale_RadioButton->setChecked(true);
    }
    else
    {
        ui->cLocale_RadioButton->setChecked(true);
    }
    setLocaleTooltip(QLocale::c(), ui->cLocale_RadioButton);
    setLocaleTooltip(QLocale(), ui->userLocale_RadioButton);

    //-------------------- Languages setup
    InitLanguages(ui->language_ComboBox);
    connect(ui->language_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SeamlyWelcomeDialog::languageChanged);

    //-------------------- Selection sound
    int index = ui->selectionSound_ComboBox->findText(settings->getSound());
    if (index != -1)
    {
        ui->selectionSound_ComboBox->setCurrentIndex(index);
    }
    connect(ui->selectionSound_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_selectionSoundChanged = true;
        QSoundEffect effect;
        effect.setSource(QUrl("qrc:/sounds/" + ui->selectionSound_ComboBox->currentText() + ".wav"));
        effect.play();
    });

    ui->doNotShow_CheckBox->setChecked(settings->getShowWelcome());

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    connect(ok_Button, &QPushButton::clicked, this, &SeamlyWelcomeDialog::apply);
}

//---------------------------------------------------------------------------------------------------------------------
SeamlyWelcomeDialog::~SeamlyWelcomeDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
// @brief apply apply dialog changes
void SeamlyWelcomeDialog::apply()
{
    settings->SetUnit(qvariant_cast<QString>(ui->units_ComboBox->currentData()));
    if (ui->cLocale_RadioButton->isChecked())
    {
        settings->setOsSeparator(false);
    }
    else
    {
        settings->setOsSeparator(true);
    }
    settings->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    settings->setShowWelcome(ui->doNotShow_CheckBox->isChecked());

    if (m_selectionSoundChanged)
    {
        const QString sound = qvariant_cast<QString>(ui->selectionSound_ComboBox->currentText());
        settings->setSelectionSound(sound);
        m_selectionSoundChanged = false;
    }

    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
// @brief initUnits initinailize the units combobox
//---------------------------------------------------------------------------------------------------------------------
void SeamlyWelcomeDialog::initUnits()
{
    ui->units_ComboBox->addItem(tr("Centimeters"), unitCM);
    ui->units_ComboBox->addItem(tr("Millimeters"), unitMM);
    ui->units_ComboBox->addItem(tr("Inches")     , unitINCH);

    // set default unit
    const qint32 index = ui->units_ComboBox->findData(settings->getUnit());
    if (index != -1)
    {
        ui->units_ComboBox->setCurrentIndex(index);
    }
}

void SeamlyWelcomeDialog::languageChanged(int index)
{
    const QString locale = qvariant_cast<QString>(ui->language_ComboBox->itemData(index));
    settings->setLocale(locale);
    qApp->loadTranslations(locale);
    ui->retranslateUi(this);
    ui->units_ComboBox->setItemText(0, tr("Centimeters"));
    ui->units_ComboBox->setItemText(1, tr("Millimeters"));
    ui->units_ComboBox->setItemText(2, tr("Inches"));
}

void SeamlyWelcomeDialog::setLocaleTooltip(QLocale locale, QRadioButton *button)
{
    const QString toolTipStr = QString("<table style=font-size:12pt; font-weight:600>"
                                       "<tr> <td><b>%1: </b></b></td><td> %2</td> </tr>"
                                       "<tr> <td><b>%3: </b></b></td><td> %4</td> </tr>"
                                       "<tr> <td><b>%5: </b></b></td><td> %6</td> </tr>"
                                       "<tr> <td><b>%7: </b></b></td><td> %8</td> </tr>"
                                       "<tr> <td><b>%9: </b></b></td><td> %10</td> </tr>"
                                       "<tr> <td><b>%11: </b></b></td><td> %12</td> </tr>"
                                       "<tr> <td><b>%13: </b></b></td><td> %14</td> </tr>"
                                       "</table>")
                                       .arg(tr("Locale"))                                 //1
                                       .arg(locale.name())                                //2
                                       .arg(tr("Country"))                                //3
                                       .arg(QLocale::countryToString(locale.country()))   //4
                                       .arg(tr("Language"))                               //5
                                       .arg(QLocale::languageToString(locale.language())) //6
                                       .arg(tr("Group Separator"))                        //7
                                       .arg(locale.groupSeparator())                      //8
                                       .arg(tr("Decimal Point"))                          //9
                                       .arg(locale.decimalPoint())                        //10
                                       .arg(tr("Negative Sign"))                          //11
                                       .arg(locale.negativeSign())                        //12
                                       .arg(tr("Positive Sign"))                          //13
                                       .arg(locale.positiveSign());                       //14

    button->setToolTip(toolTipStr);
}

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

#include <QPushButton>
#include <QShowEvent>
#include <QSound>


//---------------------------------------------------------------------------------------------------------------------
SeamlyWelcomeDialog::SeamlyWelcomeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeamlyWelcomeDialog)
    , m_langChanged(false)
    , m_selectionSoundChanged(false)
    , settings(qApp->Seamly2DSettings())
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //-------------------- Units setup
    initUnits();

    //-------------------- Decimal separator setup
    ui->separator_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->separator_CheckBox->setChecked(settings->getOsSeparator());
    connect(ui->separator_CheckBox, &QCheckBox::stateChanged, this, &SeamlyWelcomeDialog::seperatorChanged);

    //-------------------- Languages setup
    InitLanguages(ui->language_ComboBox);
    connect(ui->language_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]()
    {
        m_langChanged = true;
    });

    //-------------------- Selection sound
    int index = ui->selectionSound_ComboBox->findText(settings->getSound());
    if (index != -1)
    {
        ui->selectionSound_ComboBox->setCurrentIndex(index);
    }
    connect(ui->selectionSound_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_selectionSoundChanged = true;
        QSound::play("qrc:/sounds/" + ui->selectionSound_ComboBox->currentText() + ".wav");
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
    settings->setOsSeparator(ui->separator_CheckBox->isChecked());
    settings->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    settings->setShowWelcome(ui->doNotShow_CheckBox->isChecked());

    if (m_langChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->language_ComboBox->currentData());
        settings->setLocale(locale);
        m_langChanged = false;
    }

    if (m_selectionSoundChanged)
    {
        const QString sound = qvariant_cast<QString>(ui->selectionSound_ComboBox->currentText());
        settings->setSelectionSound(sound);
        m_selectionSoundChanged = false;
    }

    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
// @brief changeEvent handle event changes
//---------------------------------------------------------------------------------------------------------------------
void SeamlyWelcomeDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        ui->separator_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

// @brief seperatorChanged handle change in decimal seperator
void SeamlyWelcomeDialog::seperatorChanged()
{
    QString seperator = ui->separator_CheckBox->isChecked() ? QString(QLocale().decimalPoint())
                                                            : QString(QLocale::c().decimalPoint());

    ui->separator_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(seperator));
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

//  @file   me_welcome_dialog.cpp
//  @author Douglas S Caskey
//  @date   31 Dec, 2023
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

#include "me_welcome_dialog.h"
#include "ui_me_welcome_dialog.h"

#include "../vpatterndb/variables/measurement_variable.h"
//#include "../vmisc/vseamlymesettings.h"
#include "../mapplication.h"

#include <QPushButton>
#include <QShowEvent>


//---------------------------------------------------------------------------------------------------------------------
SeamlyMeWelcomeDialog::SeamlyMeWelcomeDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SeamlyMeWelcomeDialog)
    , m_langChanged(false)
    , settings(qApp->SeamlyMeSettings())
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //-------------------- Units setup
    initUnits(MeasurementsType::Individual);

    //-------------------- Decimal separator setup
    ui->separator_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->separator_CheckBox->setChecked(settings->GetOsSeparator());
    connect(ui->separator_CheckBox, &QCheckBox::stateChanged, this, &SeamlyMeWelcomeDialog::seperatorChanged);

    //-------------------- Languages setup
    InitLanguages(ui->language_ComboBox);
    connect(ui->language_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]()
    {
        m_langChanged = true;
    });

    ui->doNotShow_CheckBox->setChecked(settings->getShowWelcome());

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    connect(ok_Button, &QPushButton::clicked, this, &SeamlyMeWelcomeDialog::apply);
}

//---------------------------------------------------------------------------------------------------------------------
SeamlyMeWelcomeDialog::~SeamlyMeWelcomeDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
// @brief apply apply dialog changes
void SeamlyMeWelcomeDialog::apply()
{
    settings->SetUnit(qvariant_cast<QString>(ui->units_ComboBox->currentData()));
    settings->SetOsSeparator(ui->separator_CheckBox->isChecked());
    settings->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    settings->setShowWelcome(ui->doNotShow_CheckBox->isChecked());

    if (m_langChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->language_ComboBox->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;
    }

    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
// @brief changeEvent handle event changes
// @param type event type
void SeamlyMeWelcomeDialog::changeEvent(QEvent *event)
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
void SeamlyMeWelcomeDialog::seperatorChanged()
{
    QString seperator = ui->separator_CheckBox->isChecked() ? QString(QLocale().decimalPoint())
                                                            : QString(QLocale::c().decimalPoint());

    ui->separator_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(seperator));
}


//---------------------------------------------------------------------------------------------------------------------
// @brief initUnits initinailize the units combobox
// @param type measurment type
void SeamlyMeWelcomeDialog::initUnits(const MeasurementsType &type)
{
    ui->units_ComboBox->addItem(tr("Centimeters"), unitCM);
    ui->units_ComboBox->addItem(tr("Millimeters"), unitMM);
    ui->units_ComboBox->addItem(tr("Inches")     , unitINCH);

    // set default unit
    const qint32 index = ui->units_ComboBox->findData(settings->GetUnit());
    if (index != -1)
    {
        ui->units_ComboBox->setCurrentIndex(index);
    }
}

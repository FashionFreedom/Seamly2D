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
 **  @file   preferencesconfigurationpage.cpp
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

#include "preferencesconfigurationpage.h"
#include "ui_preferencesconfigurationpage.h"
#include "../../core/vapplication.h"
#include "../vpatterndb/pmsystems.h"

#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
PreferencesConfigurationPage::PreferencesConfigurationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferencesConfigurationPage)
    , m_langChanged(false)
    , m_systemChanged()
    , m_unitChanged(false)
    , m_labelLangChanged(false)
    , m_moveSuffixChanged(false)
    , m_rotateSuffixChanged(false)
    , m_mirrorByAxisSuffixChanged(false)
    , m_mirrorByLineSuffixChanged(false)
{
    ui->setupUi(this);
    ui->autoSaveCheck->setChecked(qApp->Seamly2DSettings()->GetAutosaveState());

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    //-------------------- Decimal separator setup
    ui->osOptionCheck->setText(tr("With OS options") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->osOptionCheck->setChecked(qApp->Seamly2DSettings()->GetOsSeparator());

    //----------------------- Unit setup
    InitUnits();
    connect(ui->unitCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_unitChanged = true;
    });

    //----------------------- Label language
    SetLabelComboBox(VApplication::LabelLanguages());

    int index = ui->labelCombo->findData(qApp->Seamly2DSettings()->GetLabelLanguage());
    if (index != -1)
    {
        ui->labelCombo->setCurrentIndex(index);
    }
    connect(ui->labelCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_labelLangChanged = true;
    });

    //---------------------- Pattern making system
    InitPMSystems(ui->systemCombo);
    ui->systemBookValueLabel->setFixedHeight(4 * QFontMetrics(ui->systemBookValueLabel->font()).lineSpacing());
    connect(ui->systemCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_systemChanged = true;
        QString text = qApp->TrVars()->PMSystemAuthor(ui->systemCombo->currentData().toString());
        ui->systemAuthorValueLabel->setText(text);
        ui->systemAuthorValueLabel->setToolTip(text);

        text = qApp->TrVars()->PMSystemBook(ui->systemCombo->currentData().toString());
        ui->systemBookValueLabel->setPlainText(text);
    });

    // set default pattern making system
    index = ui->systemCombo->findData(qApp->Seamly2DSettings()->GetPMSystemCode());
    if (index != -1)
    {
        ui->systemCombo->setCurrentIndex(index);
    }
    //---------------------- Send crash reports
    //ui->sendReportCheck->setChecked(qApp->Seamly2DSettings()->GetSendReportState());
    //ui->description = new QLabel(tr("After each crash Seamly2D collects information that may help us fix the "
    //                                "problem. We do not collect any personal information. Find more about what %1"
    //                                "kind of information%2 we collect.")
    //                             .arg("<a href=\"https://wiki.seamly2d.com/wiki/UserManual:Crash_reports\">")
    //                             .arg("</a>"));

    //----------------------------- Pattern Editing
    ui->moveSuffix_ComboBox->addItem(tr("None"), "");
    ui->moveSuffix_ComboBox->addItem(tr("_M"), "_M");
    ui->moveSuffix_ComboBox->addItem(tr("_MOV"), "_MOV");
    index = ui->moveSuffix_ComboBox->findData(qApp->Seamly2DSettings()->getMoveSuffix());
    if (index != -1)
    {
        ui->moveSuffix_ComboBox->setCurrentIndex(index);
    }
    connect(ui->moveSuffix_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_moveSuffixChanged = true;
    });
    ui->rotateSuffix_ComboBox->addItem(tr("None"), "");
    ui->rotateSuffix_ComboBox->addItem(tr("_R"), "_R");
    ui->rotateSuffix_ComboBox->addItem(tr("_ROT"), "_ROT");
    index = ui->rotateSuffix_ComboBox->findData(qApp->Seamly2DSettings()->getRotateSuffix());
    if (index != -1)
    {
        ui->rotateSuffix_ComboBox->setCurrentIndex(index);
    }
    connect(ui->rotateSuffix_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_rotateSuffixChanged = true;
    });

    ui->mirrorByAxisSuffix_ComboBox->addItem(tr("None"), "");
    ui->mirrorByAxisSuffix_ComboBox->addItem(tr("_MA"), "_MA");
    ui->mirrorByAxisSuffix_ComboBox->addItem(tr("_MBA"), "_MBA");
    index = ui->mirrorByAxisSuffix_ComboBox->findData(qApp->Seamly2DSettings()->getMirrorByAxisSuffix());
    if (index != -1)
    {
        ui->mirrorByAxisSuffix_ComboBox->setCurrentIndex(index);
    }
    connect(ui->mirrorByAxisSuffix_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_mirrorByAxisSuffixChanged = true;
    });

    ui->mirrorByLineSuffix_ComboBox->addItem(tr("None"), "");
    ui->mirrorByLineSuffix_ComboBox->addItem(tr("_MB"), "_MB");
    ui->mirrorByLineSuffix_ComboBox->addItem(tr("_MBL"), "_MBL");
    index = ui->mirrorByLineSuffix_ComboBox->findData(qApp->Seamly2DSettings()->getMirrorByLineSuffix());
    if (index != -1)
    {
        ui->mirrorByLineSuffix_ComboBox->setCurrentIndex(index);
    }
    connect(ui->mirrorByLineSuffix_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_mirrorByLineSuffixChanged = true;
    });

    connect(ui->resetWarningsButton, &QPushButton::released, []()
    {
        VSettings *settings = qApp->Seamly2DSettings();

        settings->SetConfirmItemDelete(true);
        settings->SetConfirmFormatRewriting(true);
    });
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesConfigurationPage::~PreferencesConfigurationPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();
    settings->SetAutosaveState(ui->autoSaveCheck->isChecked());
    settings->SetAutosaveTime(ui->autoTime->value());

    QTimer *autoSaveTimer = qApp->getAutoSaveTimer();
    SCASSERT(autoSaveTimer)

    ui->autoSaveCheck->isChecked() ? autoSaveTimer->start(ui->autoTime->value()*60000) : autoSaveTimer->stop();

    settings->SetOsSeparator(ui->osOptionCheck->isChecked());
    //settings->SetSendReportState(ui->sendReportCheck->isChecked());

    if (m_langChanged || m_systemChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;

        const QString code = qvariant_cast<QString>(ui->systemCombo->currentData());
        settings->SetPMSystemCode(code);
        m_systemChanged = false;

        qApp->LoadTranslation(locale);
    }
    if (m_unitChanged)
    {
        const QString unit = qvariant_cast<QString>(ui->unitCombo->currentData());
        settings->SetUnit(unit);
        m_unitChanged = false;
        const QString text = tr("The Default unit has been updated and will be used as the default for the next "
                                "pattern you create.");
        QMessageBox::information(this, QCoreApplication::applicationName(), text);
    }
    if (m_labelLangChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->labelCombo->currentData());
        settings->SetLabelLanguage(locale);
        m_labelLangChanged = false;
    }
    if (m_moveSuffixChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->moveSuffix_ComboBox->currentData());
        settings->setMoveSuffix(locale);
        m_moveSuffixChanged = false;
    }
    if (m_rotateSuffixChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->rotateSuffix_ComboBox->currentData());
        settings->setRotateSuffix(locale);
        m_rotateSuffixChanged = false;
    }
    if (m_mirrorByAxisSuffixChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->mirrorByAxisSuffix_ComboBox->currentData());
        settings->setMirrorByAxisSuffix(locale);
        m_mirrorByAxisSuffixChanged = false;
    }
    if (m_mirrorByLineSuffixChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->mirrorByLineSuffix_ComboBox->currentData());
        settings->setMirrorByLineSuffix(locale);
        m_mirrorByLineSuffixChanged = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        RetranslateUi();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::SetLabelComboBox(const QStringList &list)
{
    for (int i = 0; i < list.size(); ++i)
    {
        QLocale loc = QLocale(list.at(i));
        ui->labelCombo->addItem(loc.nativeLanguageName(), list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::InitUnits()
{
    ui->unitCombo->addItem(tr("Centimeters"), unitCM);
    ui->unitCombo->addItem(tr("Millimiters"), unitMM);
    ui->unitCombo->addItem(tr("Inches"), unitINCH);

    // set default unit
    const qint32 indexUnit = ui->unitCombo->findData(qApp->Seamly2DSettings()->GetUnit());
    if (indexUnit != -1)
    {
        ui->unitCombo->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::RetranslateUi()
{
    ui->osOptionCheck->setText(tr("With OS options") + QString(" (%1)").arg(QLocale().decimalPoint()));
    //ui->description->setText(tr("After each crash Seamly2D collects information that may help us fix the "
    //                            "problem. We do not collect any personal information. Find more about what %1"
    //                            "kind of information%2 we collect.")
    //                         .arg("<a href=\"https://wiki.seamly2d.com/wiki/UserManual:Crash_reports\">")
    //                         .arg("</a>"));
}

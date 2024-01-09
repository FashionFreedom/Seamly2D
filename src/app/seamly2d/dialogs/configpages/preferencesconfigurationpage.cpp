/******************************************************************************
*   @file   preferencesconfigurationpage.h
**  @author Douglas S Caskey
**  @date   26 Oct, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   preferencesconfigurationpage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
 **  along with Valentina.  if not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "preferencesconfigurationpage.h"
#include "ui_preferencesconfigurationpage.h"
#include "../../core/vapplication.h"
#include "../vpatterndb/pmsystems.h"
#include "../vwidgets/export_format_combobox.h"

#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QSound>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
PreferencesConfigurationPage::PreferencesConfigurationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferencesConfigurationPage)
    , m_langChanged(false)
    , m_unitChanged(false)
    , m_labelLangChanged(false)
    , m_selectionSoundChanged(false)
    , m_moveSuffixChanged(false)
    , m_rotateSuffixChanged(false)
    , m_mirrorByAxisSuffixChanged(false)
    , m_mirrorByLineSuffixChanged(false)
    , m_defaultExportFormatChanged(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QRegularExpression rx("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b",
                          QRegularExpression::CaseInsensitiveOption);
    ui->email_LineEdit->setValidator(new QRegularExpressionValidator(rx, this));

    //Designer Info
    ui->companyName_LineEdit->setText(qApp->Seamly2DSettings()->getCompanyName());
    ui->contact_LineEdit->setText(qApp->Seamly2DSettings()->getContact());
    ui->address_LineEdit->setText(qApp->Seamly2DSettings()->getAddress());
    ui->city_LineEdit->setText(qApp->Seamly2DSettings()->getCity());
    ui->state_LineEdit->setText(qApp->Seamly2DSettings()->getState());
    ui->zipcode_LineEdit->setText(qApp->Seamly2DSettings()->getZipcode());
    ui->country_LineEdit->setText(qApp->Seamly2DSettings()->getCountry());
    ui->telephone_LineEdit->setText(qApp->Seamly2DSettings()->getTelephone());
    ui->fax_LineEdit->setText(qApp->Seamly2DSettings()->getFax());
    ui->email_LineEdit->setText(qApp->Seamly2DSettings()->getEmail());
    ui->website_LineEdit->setText(qApp->Seamly2DSettings()->getWebsite());

    connect(ui->email_LineEdit, &QLineEdit::textChanged, this, &PreferencesConfigurationPage::adjustTextColor);

    //Editing
    // Undo
    ui->undoCount_SpinBox->setValue(qApp->Seamly2DSettings()->GetUndoCount());

    //Selection sound
    int index = ui->selectionSound_ComboBox->findText(qApp->Seamly2DSettings()->getSound());
    if (index != -1)
    {
        ui->selectionSound_ComboBox->setCurrentIndex(index);
    }
    connect(ui->selectionSound_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_selectionSoundChanged = true;
        QSound::play("qrc:/sounds/" + ui->selectionSound_ComboBox->currentText() + ".wav");
    });

    // Warnings
    ui->confirmItemDelete_CheckBox->setChecked(qApp->Seamly2DSettings()->getConfirmItemDelete());
    ui->confirmFormatRewriting_CheckBox->setChecked(qApp->Seamly2DSettings()->getConfirmFormatRewriting());
    // Send crash reports
    //ui->sendReportCheck->setChecked(qApp->Seamly2DSettings()->GetSendReportState());
    //ui->description = new QLabel(tr("After each crash Seamly2D collects information that may help us fix the "
    //                                "problem. We do not collect any personal information. Find more about what %1"
    //                                "kind of information%2 we collect.")
    //                             .arg("<a href=\"https://wiki.seamly.net/wiki/UserManual:Crash_reports\">")
    //                             .arg("</a>"));

    // Default operations suffixes
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

    // File handling
    // Autosave
    ui->autoSave_CheckBox->setChecked(qApp->Seamly2DSettings()->GetAutosaveState());
    ui->autoInterval_Spinbox->setValue(qApp->Seamly2DSettings()->getAutosaveInterval());

    // Export Format
    ui->useModeType_CheckBox->setChecked(qApp->Seamly2DSettings()->useModeType());
    ui->uselastExportFormat_CheckBox->setChecked(qApp->Seamly2DSettings()->useLastExportFormat());
    index = ui->defaultExportFormat_ComboBox->findText(qApp->Seamly2DSettings()->getExportFormat());
    if (index != -1)
    {
        ui->defaultExportFormat_ComboBox->setCurrentIndex(index);
    }
    connect(ui->defaultExportFormat_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_defaultExportFormatChanged = true;
    });

    // Language
    InitLanguages(ui->langCombo);
    connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    // Decimal separator setup
    ui->osOptionCheck->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->osOptionCheck->setChecked(qApp->Seamly2DSettings()->GetOsSeparator());

    // Unit setup
    InitUnits();
    connect(ui->unitCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_unitChanged = true;
    });
    SetLabelComboBox(VApplication::LabelLanguages());

    index = ui->labelCombo->findData(qApp->Seamly2DSettings()->GetLabelLanguage());
    if (index == -1)
    {
        index = ui->labelCombo->findData("en");
    }
    if (index != -1)
    {
        ui->labelCombo->setCurrentIndex(index);
    }

    connect(ui->labelCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_labelLangChanged = true;
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
    if(!ui->email_LineEdit->text().isEmpty() && !ui->email_LineEdit->hasAcceptableInput())
    {
        QMessageBox::warning(this, tr("Email verification"),
                             tr("Email format is not valid."), QMessageBox::Ok);

        ui->email_LineEdit->setText(QString(""));
    }

    VSettings *settings = qApp->Seamly2DSettings();

    //Designer Info
    settings->setCompanyName(ui->companyName_LineEdit->text());
    settings->setContact(ui->contact_LineEdit->text());
    settings->setAddress(ui->address_LineEdit->text());
    settings->setCity(ui->city_LineEdit->text());
    settings->setState(ui->state_LineEdit->text());
    settings->setZipcode(ui->zipcode_LineEdit->text());
    settings->setCountry(ui->country_LineEdit->text());
    settings->setTelephone(ui->telephone_LineEdit->text());
    settings->setFax(ui->fax_LineEdit->text());
    settings->setEmail(ui->email_LineEdit->text());
    settings->setWebsite(ui->website_LineEdit->text());

    /* Maximum number of commands in undo stack may only be set when the undo stack is empty, since setting it on a
     * non-empty stack might delete the command at the current index. Calling setUndoLimit() on a non-empty stack
     * prints a warning and does nothing.*/
    settings->SetUndoCount(ui->undoCount_SpinBox->value());
    if (m_selectionSoundChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->selectionSound_ComboBox->currentText());
        settings->setSelectionSound(locale);
        m_selectionSoundChanged = false;
    }
    settings->setConfirmItemDelete(ui->confirmItemDelete_CheckBox->isChecked());
    settings->setConfirmFormatRewriting(ui->confirmFormatRewriting_CheckBox->isChecked());

    settings->SetAutosaveState(ui->autoSave_CheckBox->isChecked());
    settings->setAutosaveInterval(ui->autoInterval_Spinbox->value());

    QTimer *autoSaveTimer = qApp->getAutoSaveTimer();
    SCASSERT(autoSaveTimer)

    ui->autoSave_CheckBox->isChecked() ? autoSaveTimer->start(ui->autoInterval_Spinbox->value()*60000) : autoSaveTimer->stop();

    settings->setUseModeType(ui->useModeType_CheckBox->isChecked());
    settings->setUseLastExportFormat(ui->uselastExportFormat_CheckBox->isChecked());
    if (m_defaultExportFormatChanged)
    {
        const QString format = qvariant_cast<QString>(ui->defaultExportFormat_ComboBox->currentText());
        settings->setExportFormat(format);
        m_defaultExportFormatChanged = false;
    }

    settings->SetOsSeparator(ui->osOptionCheck->isChecked());
    //settings->SetSendReportState(ui->sendReportCheck->isChecked());

    if (m_langChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;

        qApp->loadTranslations(locale);
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
        ui->retranslateUi(this);
        ui->osOptionCheck->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::SetLabelComboBox(const QStringList &list)
{
    for (int i = 0; i < list.size(); ++i)
    {
        QLocale loc = QLocale(list.at(i));
        QString country = QLocale::countryToString(loc.country());
        QIcon ico(QString("%1/%2.png").arg(":/flags").arg(country));
        ui->labelCombo->addItem(ico, loc.nativeLanguageName(), list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesConfigurationPage::InitUnits()
{
    ui->unitCombo->addItem(tr("Centimeters"), unitCM);
    ui->unitCombo->addItem(tr("Millimeters"), unitMM);
    ui->unitCombo->addItem(tr("Inches"), unitINCH);

    // set default unit
    const qint32 indexUnit = ui->unitCombo->findData(qApp->Seamly2DSettings()->GetUnit());
    if (indexUnit != -1)
    {
        ui->unitCombo->setCurrentIndex(indexUnit);
    }
}

void PreferencesConfigurationPage::adjustTextColor()
{
    if(!ui->email_LineEdit->hasAcceptableInput())
        ui->email_LineEdit->setStyleSheet("QLineEdit {color: red;}");
    else
        ui->email_LineEdit->setStyleSheet("QLineEdit {color: black;}");
}

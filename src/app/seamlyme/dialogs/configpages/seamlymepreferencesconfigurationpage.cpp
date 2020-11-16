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
 **  @file   seamlymepreferencesconfigurationpage.cpp
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

#include "seamlymepreferencesconfigurationpage.h"
#include "ui_seamlymepreferencesconfigurationpage.h"
#include "../../mapplication.h"
#include "../vmisc/vseamlymesettings.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/pmsystems.h"

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesConfigurationPage::SeamlyMePreferencesConfigurationPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SeamlyMePreferencesConfigurationPage),
      m_langChanged(false),
      m_systemChanged(false),
      m_defGradationChanged(false)
{
    ui->setupUi(this);

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    //-------------------- Decimal separator setup
    ui->osOptionCheck->setText(tr("With OS options") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->osOptionCheck->setChecked(qApp->SeamlyMeSettings()->GetOsSeparator());

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
    int index = ui->systemCombo->findData(qApp->SeamlyMeSettings()->GetPMSystemCode());
    if (index != -1)
    {
        ui->systemCombo->setCurrentIndex(index);
    }

    //----------------------------- Measurements Editing
    connect(ui->resetWarningsButton, &QPushButton::released, []()
    {
        VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();

        settings->SetConfirmFormatRewriting(true);
    });

    //----------------------- Toolbar
    ui->toolBarStyle_CheckBox->setChecked(qApp->SeamlyMeSettings()->getToolBarStyle());

    //---------------------------Default height and size
    ui->defHeightCombo->addItems(VMeasurement::WholeListHeights(Unit::Cm));
    index = ui->defHeightCombo->findText(QString().setNum(qApp->SeamlyMeSettings()->GetDefHeight()));
    if (index != -1)
    {
        ui->defHeightCombo->setCurrentIndex(index);
    }

    auto DefGradationChanged = [this]()
    {
        m_defGradationChanged = true;
    };

    connect(ui->defHeightCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            DefGradationChanged);

    ui->defSizeCombo->addItems(VMeasurement::WholeListSizes(Unit::Cm));
    index = ui->defSizeCombo->findText(QString().setNum(qApp->SeamlyMeSettings()->GetDefSize()));
    if (index != -1)
    {
        ui->defSizeCombo->setCurrentIndex(index);
    }
    connect(ui->defHeightCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            DefGradationChanged);
}

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesConfigurationPage::~SeamlyMePreferencesConfigurationPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesConfigurationPage::Apply()
{
    VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();
    settings->SetOsSeparator(ui->osOptionCheck->isChecked());

    settings->setToolBarStyle(ui->toolBarStyle_CheckBox->isChecked());

    if (m_langChanged || m_systemChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;

        const QString code = qvariant_cast<QString>(ui->systemCombo->currentData());
        settings->SetPMSystemCode(code);
        m_systemChanged = false;

        qApp->LoadTranslation(locale);
        qApp->processEvents();// force to call changeEvent

        // Part about measurments will not be updated automatically
        qApp->RetranslateTables();
        qApp->retranslateGroups();
    }

    if (m_defGradationChanged)
    {
        settings->SetDefHeight(ui->defHeightCombo->currentText().toInt());
        settings->SetDefSize(ui->defSizeCombo->currentText().toInt());
        m_defGradationChanged = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesConfigurationPage::changeEvent(QEvent *event)
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
void SeamlyMePreferencesConfigurationPage::RetranslateUi()
{
    ui->osOptionCheck->setText(tr("With OS options") + QString(" (%1)").arg(QLocale().decimalPoint()));
}

/******************************************************************************
*   @file   seamlymepreferencesconfigurationpage.cpp
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
 **  @file   seamlymepreferencesconfigurationpage.cpp
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "seamlymepreferencesconfigurationpage.h"
#include "ui_seamlymepreferencesconfigurationpage.h"
#include "../../mapplication.h"
#include "../vmisc/vseamlymesettings.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../vpatterndb/pmsystems.h"

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesConfigurationPage::SeamlyMePreferencesConfigurationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SeamlyMePreferencesConfigurationPage)
    , m_langChanged(false)
    , m_systemChanged(false)
    , m_defGradationChanged(false)
{
    ui->setupUi(this);

    //-------------------- Startup
    ui->showWelcome_CheckBox->setChecked(qApp->SeamlyMeSettings()->getShowWelcome());

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    //-------------------- Decimal separator setup
    ui->osOption_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    ui->osOption_CheckBox->setChecked(qApp->SeamlyMeSettings()->GetOsSeparator());

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

        settings->setConfirmFormatRewriting(true);
    });

    //----------------------- Toolbar
    ui->toolBarStyle_CheckBox->setChecked(qApp->SeamlyMeSettings()->getToolBarStyle());

    //---------------------------Default height and size
    ui->defHeightCombo->addItems(MeasurementVariable::WholeListHeights(Unit::Cm));
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

    ui->defSizeCombo->addItems(MeasurementVariable::WholeListSizes(Unit::Cm));
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
void SeamlyMePreferencesConfigurationPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        ui->osOption_CheckBox->setText(tr("User locale") + QString(" (%1)").arg(QLocale().decimalPoint()));
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesConfigurationPage::Apply()
{
    VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();

    settings->setShowWelcome(ui->showWelcome_CheckBox->isChecked());
    settings->SetOsSeparator(ui->osOption_CheckBox->isChecked());

    settings->setToolBarStyle(ui->toolBarStyle_CheckBox->isChecked());

    if (m_langChanged || m_systemChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->SetLocale(locale);
        m_langChanged = false;

        const QString code = qvariant_cast<QString>(ui->systemCombo->currentData());
        settings->SetPMSystemCode(code);
        m_systemChanged = false;

        qApp->loadTranslations(locale);
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

///---------------------------------------------------------------------------------------------------------------------
/// @file   seamlymepreferencesconfigurationpage.cpp
/// @author Douglas S Caskey
/// @date   26 Oct, 2023
///
/// @brief
/// @copyright
/// This source code is part of the Seamly2D project, a pattern making
/// program to create and model patterns of clothing.
/// Copyright (C) 2017-2023 Seamly2D project
/// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
///
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
///---------------------------------------------------------------------------------------------------------------------

///---------------------------------------------------------------------------------------------------------------------
/// @file   seamlymepreferencesconfigurationpage.cpp
/// @author Roman Telezhynskyi <dismine(at)gmail.com>
/// @date   12 4, 2017
///
/// @brief
/// @copyright
/// This source code is part of the Valentine project, a pattern making
/// program, whose allow create and modeling patterns of clothing.
/// Copyright (C) 2017 Seamly2D project
/// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
///
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
///---------------------------------------------------------------------------------------------------------------------

#include "seamlymepreferencesconfigurationpage.h"
#include "ui_seamlymepreferencesconfigurationpage.h"
#include "../../application_me.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vseamlymesettings.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../vpatterndb/pmsystems.h"

#include <QLocale>
#include <QString>

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesConfigurationPage::SeamlyMePreferencesConfigurationPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SeamlyMePreferencesConfigurationPage)
    , m_themeChanged(false)
    , m_langChanged(false)
    , m_systemChanged(false)
    , m_defGradationChanged(false)
{
    ui->setupUi(this);

    //-------------------- Startup
    // Theme
    ui->theme_ComboBox->addItem("Fusion Light", 0);
    ui->theme_ComboBox->addItem("Fusion Dark", 1);
    ui->theme_ComboBox->addItem("Fusion Twilight", 2);
    ui->theme_ComboBox->addItem("System", 3);

#if defined(Q_OS_WIN)
    ui->theme_ComboBox->addItem("Classic", 4);
    ui->theme_ComboBox->addItem("Windows11", 5);
#endif

    // set default theme
    const int themeIndex = ui->theme_ComboBox->findData(qApp->seamlyMeSettings()->getAppTheme());
    if (themeIndex != -1)
    {
        ui->theme_ComboBox->setCurrentIndex(themeIndex);
    }

    connect(ui->theme_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_themeChanged = true;
    });

    InitLanguages(ui->langCombo);
    connect(ui->langCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_langChanged = true;
    });

    ui->showWelcome_CheckBox->setChecked(qApp->seamlyMeSettings()->getShowWelcome());


    //-------------------- Decimal separator setup
    if (qApp->seamlyMeSettings()->getOsSeparator())
    {
        ui->userLocale_RadioButton->setChecked(true);
    }
    else
    {
        ui->cLocale_RadioButton->setChecked(true);
    }
    setLocaleTooltip(QLocale::c(), ui->cLocale_RadioButton);
    setLocaleTooltip(QLocale(), ui->userLocale_RadioButton);

    //---------------------- Pattern making system
    InitPMSystems(ui->systemCombo);
    ui->systemBookValueLabel->setFixedHeight(4 * QFontMetrics(ui->systemBookValueLabel->font()).lineSpacing());
    connect(ui->systemCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_systemChanged = true;
        QString text = qApp->translateVariables()->PMSystemAuthor(ui->systemCombo->currentData().toString());
        ui->systemAuthorValueLabel->setText(text);
        ui->systemAuthorValueLabel->setToolTip(text);

        text = qApp->translateVariables()->PMSystemBook(ui->systemCombo->currentData().toString());
        ui->systemBookValueLabel->setPlainText(text);
    });

    // set default pattern making system
    int index = ui->systemCombo->findData(qApp->seamlyMeSettings()->GetPMSystemCode());
    if (index != -1)
    {
        ui->systemCombo->setCurrentIndex(index);
    }

    //----------------------------- Measurements Editing
    connect(ui->resetWarningsButton, &QPushButton::released, []()
    {
        VSeamlyMeSettings *settings = qApp->seamlyMeSettings();

        settings->setConfirmFormatRewriting(true);
    });

    //----------------------- Toolbar
    ui->toolBarStyle_CheckBox->setChecked(qApp->seamlyMeSettings()->getToolBarStyle());

    // ----------------------- Dialogs
    ui->use_native_checkox->setChecked(qApp->seamlyMeSettings()->useNativeDialogs());


    //---------------------------Default height and size
    ui->defHeightCombo->addItems(MeasurementVariable::WholeListHeights(Unit::Cm));
    index = ui->defHeightCombo->findText(QString().setNum(qApp->seamlyMeSettings()->GetDefHeight()));
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
    index = ui->defSizeCombo->findText(QString().setNum(qApp->seamlyMeSettings()->GetDefSize()));
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
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesConfigurationPage::Apply()
{
    VSeamlyMeSettings *settings = qApp->seamlyMeSettings();

    settings->setShowWelcome(ui->showWelcome_CheckBox->isChecked());
    if (ui->cLocale_RadioButton->isChecked())
    {
        settings->setOsSeparator(false);
    }
    else
    {
        settings->setOsSeparator(true);
    }
    settings->setToolBarStyle(ui->toolBarStyle_CheckBox->isChecked());

    settings->setUseNativeDialogs(ui->use_native_checkox->isChecked());

    if (m_themeChanged)
    {
        int theme = qvariant_cast<int>(ui->theme_ComboBox->currentData());
        settings->setAppTheme(theme);
        qApp->setTheme();
        m_themeChanged = false;
    }

    if (m_langChanged || m_systemChanged)
    {
        const QString locale = qvariant_cast<QString>(ui->langCombo->currentData());
        settings->setLocale(locale);
        m_langChanged = false;

        const QString code = qvariant_cast<QString>(ui->systemCombo->currentData());
        settings->SetPMSystemCode(code);
        m_systemChanged = false;

        qApp->loadTranslations(locale);
        qApp->processEvents();// force to call changeEvent

        // Part about measurments will not be updated automatically
        qApp->retranslateTables();
        qApp->retranslateGroups();
    }

    if (m_defGradationChanged)
    {
        settings->SetDefHeight(ui->defHeightCombo->currentText().toInt());
        settings->SetDefSize(ui->defSizeCombo->currentText().toInt());
        m_defGradationChanged = false;
    }
}

void SeamlyMePreferencesConfigurationPage::setLocaleTooltip(QLocale locale, QRadioButton *button)
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
                                       .arg(tr("Locale"))                                   //1
                                       .arg(locale.name())                                  //2
                                       .arg(tr("Country"))                                  //3
                                       .arg(QLocale::countryToString(locale.country()))     //4
                                       .arg(tr("Language"))                                 //5
                                       .arg(QLocale::languageToString(locale.language()))   //6
                                       .arg(tr("Group Separator"))                          //7
                                       .arg(locale.groupSeparator())                        //8
                                       .arg(tr("Decimal Point"))                            //9
                                       .arg(locale.decimalPoint())                          //10
                                       .arg(tr("Negative Sign"))                            //11
                                       .arg(locale.negativeSign())                          //12
                                       .arg(tr("Positive Sign"))                            //13
                                       .arg(locale.positiveSign());                         //14

    button->setToolTip(toolTipStr);
}

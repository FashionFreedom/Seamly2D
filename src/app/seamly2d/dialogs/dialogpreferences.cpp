//  @file   dialogpreferences.cpp
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2024 Seamly2D project
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

/************************************************************************
 **
 **  @file   dialogpreferences.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "dialogpreferences.h"
#include "ui_dialogpreferences.h"
#include "../core/application_2d.h"
#include "configpages/preferencesconfigurationpage.h"
#include "configpages/preferencespatternpage.h"
#include "configpages/preferencespathpage.h"
#include "configpages/preferencesgraphicsviewpage.h"

#include <QGuiApplication>
#include <QPushButton>
#include <QScreen>

//---------------------------------------------------------------------------------------------------------------------
DialogPreferences::DialogPreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogPreferences)
    , m_isInitialized(false)
    , m_configurePage(new PreferencesConfigurationPage)
    , m_patternPage(new PreferencesPatternPage)
    , m_pathPage(new PreferencesPathPage)
    , m_graphicsPage(new PreferencesGraphicsViewPage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    qApp->getSceneView()->itemClicked(nullptr); //clear any selection

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    connect(ok_Button, &QPushButton::clicked, this, &DialogPreferences::Ok);

    QPushButton *apply_Button = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(apply_Button != nullptr)
    connect(apply_Button, &QPushButton::clicked, this, &DialogPreferences::Apply);

    ui->pages_StackedWidget->insertWidget(0, m_configurePage);
    ui->pages_StackedWidget->insertWidget(1, m_patternPage);
    ui->pages_StackedWidget->insertWidget(2, m_pathPage);
    ui->pages_StackedWidget->insertWidget(3, m_graphicsPage);


    connect(ui->contents_ListWidget, &QListWidget::currentItemChanged, this, &DialogPreferences::pageChanged);
    ui->pages_StackedWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPreferences::~DialogPreferences()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMinimumSize(size());

    QSize sz = qApp->Settings()->getPreferenceDialogSize();
    if (sz.isEmpty() == false)
    {
        resize(sz);
    }

    m_isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (m_isInitialized)
    {
        qApp->Settings()->setPreferenceDialogSize(size());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::pageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int rowIndex = ui->contents_ListWidget->row(current);
    ui->pages_StackedWidget->setCurrentIndex(rowIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::Apply()
{
    m_configurePage->apply();
    m_patternPage->Apply();
    m_pathPage->Apply();
    m_graphicsPage->Apply();

    m_patternPage->initDefaultSeamAllowance();

    qApp->Seamly2DSettings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    emit updateProperties();
    setResult(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPreferences::Ok()
{
    Apply();
    done(QDialog::Accepted);
}

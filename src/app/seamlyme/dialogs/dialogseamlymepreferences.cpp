/******************************************************************************
*   @file   dialogseamlymepreferences.cpp
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
 **  @file   dialogseamlymepreferences.cpp
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

#include "dialogseamlymepreferences.h"
#include "ui_dialogseamlymepreferences.h"
#include "../mapplication.h"
#include "configpages/seamlymepreferencesconfigurationpage.h"
#include "configpages/seamlymepreferencespathpage.h"

#include <QGuiApplication>
#include <QPushButton>
#include <QScreen>
#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
DialogSeamlyMePreferences::DialogSeamlyMePreferences(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSeamlyMePreferences)
    , m_isInitialized(false)
    , m_configurationPage(new SeamlyMePreferencesConfigurationPage)
    , m_pathPage(new SeamlyMePreferencesPathPage)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    qApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    connect(ok_Button, &QPushButton::clicked, this, &DialogSeamlyMePreferences::Ok);

    QPushButton *apply_Button = ui->buttonBox->button(QDialogButtonBox::Apply);
    SCASSERT(apply_Button != nullptr)
    connect(apply_Button, &QPushButton::clicked, this, &DialogSeamlyMePreferences::Apply);

    ui->pagesWidget->insertWidget(0, m_configurationPage);
    ui->pagesWidget->insertWidget(1, m_pathPage);

    connect(ui->contentsWidget, &QListWidget::currentItemChanged, this, &DialogSeamlyMePreferences::PageChanged);
    ui->pagesWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
DialogSeamlyMePreferences::~DialogSeamlyMePreferences()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamlyMePreferences::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamlyMePreferences::showEvent(QShowEvent *event)
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
void DialogSeamlyMePreferences::resizeEvent(QResizeEvent *event)
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
void DialogSeamlyMePreferences::Apply()
{
    m_configurationPage->Apply();
    m_pathPage->Apply();

    qApp->SeamlyMeSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());
    emit updateProperties();
    setResult(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamlyMePreferences::Ok()
{
    Apply();
    done(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamlyMePreferences::PageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int rowIndex = ui->contentsWidget->row(current);
    ui->pagesWidget->setCurrentIndex(rowIndex);
}

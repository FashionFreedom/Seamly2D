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
 **  @file   seamlymepreferencespathpage.cpp
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

#include "seamlymepreferencespathpage.h"
#include "ui_seamlymepreferencespathpage.h"
#include "../../mapplication.h"
#include "../vmisc/vseamlymesettings.h"

#include <QDir>
#include <QFileDialog>

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesPathPage::SeamlyMePreferencesPathPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::SeamlyMePreferencesPathPage)
{
    ui->setupUi(this);

    InitTable();

    connect(ui->defaultButton, &QPushButton::clicked, this, &SeamlyMePreferencesPathPage::DefaultPath);
    connect(ui->editButton, &QPushButton::clicked, this, &SeamlyMePreferencesPathPage::EditPath);
}

//---------------------------------------------------------------------------------------------------------------------
SeamlyMePreferencesPathPage::~SeamlyMePreferencesPathPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesPathPage::Apply()
{
    VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();
    settings->SetPathIndividualMeasurements(ui->pathTable->item(0, 1)->text());
    settings->SetPathMultisizeMeasurements(ui->pathTable->item(1, 1)->text());
    settings->SetPathTemplate(ui->pathTable->item(2, 1)->text());
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesPathPage::DefaultPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // individual measurements
            path = VCommonSettings::GetDefPathIndividualMeasurements();
            break;
        case 1: // multisize measurements
            path = VCommonSettings::GetDefPathMultisizeMeasurements();
            break;
        case 2: // templates
            path = VCommonSettings::GetDefPathTemplate();
            break;
        default:
            break;
    }

    item->setText(path);
    item->setToolTip(path);
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesPathPage::EditPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // individual measurements
            path = qApp->SeamlyMeSettings()->GetPathIndividualMeasurements();
            break;
        case 1: // multisize measurements
            path = qApp->SeamlyMeSettings()->GetPathMultisizeMeasurements();
            path = VCommonSettings::PrepareMultisizeTables(path);
            break;
        case 2: // templates
            path = qApp->SeamlyMeSettings()->GetPathTemplate();
            break;
        default:
            break;
    }

    bool usedNotExistedDir = false;
    QDir directory(path);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    const QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), path,
                                                          QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
    {
        if (usedNotExistedDir)
        {
            QDir directory(path);
            directory.rmpath(".");
        }

        DefaultPath();
        return;
    }

    item->setText(dir);
    item->setToolTip(dir);
}

//---------------------------------------------------------------------------------------------------------------------
void SeamlyMePreferencesPathPage::InitTable()
{
    ui->pathTable->setRowCount(3);
    ui->pathTable->setColumnCount(2);

    const VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();

    {
        ui->pathTable->setItem(0, 0, new QTableWidgetItem(tr("My Individual Measurements")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathIndividualMeasurements());
        item->setToolTip(settings->GetPathIndividualMeasurements());
        ui->pathTable->setItem(0, 1, item);
    }

    {
        ui->pathTable->setItem(1, 0, new QTableWidgetItem(tr("My Multisize Measurements")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathMultisizeMeasurements());
        item->setToolTip(settings->GetPathMultisizeMeasurements());
        ui->pathTable->setItem(1, 1, item);
    }

    {
        ui->pathTable->setItem(2, 0, new QTableWidgetItem(tr("My Templates")));
        QTableWidgetItem *item = new QTableWidgetItem(settings->GetPathTemplate());
        item->setToolTip(settings->GetPathTemplate());
        ui->pathTable->setItem(2, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();
    ui->pathTable->horizontalHeader()->setStretchLastSection(true);

    connect(ui->pathTable, &QTableWidget::itemSelectionChanged, this, [this]()
    {
        ui->defaultButton->setEnabled(true);
        ui->defaultButton->setDefault(false);

        ui->editButton->setEnabled(true);
        ui->editButton->setDefault(true);
    });
}

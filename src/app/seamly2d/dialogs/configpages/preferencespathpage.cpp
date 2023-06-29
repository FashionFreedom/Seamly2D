/***************************************************************************
 **  @file   preferencespathpage.cpp
 **  @author Douglas S Caskey
 **  @date   Jul 8, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   preferencespathpage.cpp
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

#include "preferencespathpage.h"
#include "ui_preferencespathpage.h"
#include "../vmisc/vsettings.h"
#include "../../options.h"
#include "../../core/vapplication.h"

#include <QDir>
#include <QFileDialog>

//---------------------------------------------------------------------------------------------------------------------
PreferencesPathPage::PreferencesPathPage(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::PreferencesPathPage)
{
    ui->setupUi(this);

    initializeTable();

    connect(ui->defaultButton, &QPushButton::clicked, this, &PreferencesPathPage::defaultPath);
    connect(ui->editButton, &QPushButton::clicked, this, &PreferencesPathPage::editPath);
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesPathPage::~PreferencesPathPage()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();
    settings->setIndividualSizePath(ui->pathTable->item(0, 1)->text());
    settings->setMultisizePath(ui->pathTable->item(1, 1)->text());
    settings->SetPathPattern(ui->pathTable->item(2, 1)->text());
    settings->SetPathLayout(ui->pathTable->item(3, 1)->text());
    settings->setTemplatePath(ui->pathTable->item(4, 1)->text());
    settings->SetPathLabelTemplate(ui->pathTable->item(5, 1)->text());
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::defaultPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;

    switch (row)
    {
        case 1: // multisize measurements
            path = VCommonSettings::getDefaultMultisizePath();
            break;
        case 2: // pattern path
            path = VSettings::GetDefPathPattern();
            break;
        case 0: // individual measurements
            path = VCommonSettings::getDefaultIndividualSizePath();
            break;
        case 3: // layout path
            path = VSettings::GetDefPathLayout();
            break;
        case 4: // templates
            path = VCommonSettings::getDefaultTemplatePath();
            break;
        case 5: // label templates
            path = VSettings::GetDefPathLabelTemplate();
            break;
        default:
            break;
    }

    item->setText(path);
    item->setToolTip(path);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::editPath()
{
    const int row = ui->pathTable->currentRow();
    QTableWidgetItem *item = ui->pathTable->item(row, 1);
    SCASSERT(item != nullptr)

    QString path;
    switch (row)
    {
        case 0: // individual measurements
            path = qApp->Seamly2DSettings()->getIndividualSizePath();
            break;
        case 1: // multisize measurements
            path = qApp->Seamly2DSettings()->getMultisizePath();
            path = VCommonSettings::prepareMultisizeTables(path);
            break;
        case 2: // pattern path
            path = qApp->Seamly2DSettings()->GetPathPattern();
            break;
        case 3: // layout path
            path = qApp->Seamly2DSettings()->getLayoutPath();
            break;
        case 4: // templates
            path = qApp->Seamly2DSettings()->getTemplatePath();
            break;
        case 5: // label templates
            path = qApp->Seamly2DSettings()->GetPathLabelTemplate();
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
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks
                                                          | QFileDialog::DontUseNativeDialog);
    if (dir.isEmpty())
    {
        if (usedNotExistedDir)
        {
            QDir directory(path);
            directory.rmpath(".");
        }
        defaultPath();
        return;
    }

    item->setText(dir);
    item->setToolTip(dir);

    if (usedNotExistedDir)
    {
        QDir directory(path);
        directory.rmpath(".");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesPathPage::initializeTable()
{
    ui->pathTable->setRowCount(6);
    ui->pathTable->setColumnCount(2);

    const VSettings *settings = qApp->Seamly2DSettings();

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Patterns"));
        item->setIcon(QIcon("://icon/32x32/seamly2d_file.png"));
        ui->pathTable->setItem(0, 0, item);
        item = new QTableWidgetItem(settings->GetPathPattern());
        item->setToolTip(settings->GetPathPattern());
        ui->pathTable->setItem(0, 1, item);
    }

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Templates"));
        item->setIcon(QIcon("://icon/32x32/template_size_file.png"));
        ui->pathTable->setItem(1, 0, item);
        item = new QTableWidgetItem(settings->getTemplatePath());
        item->setToolTip(settings->getTemplatePath());
        ui->pathTable->setItem(1, 1, item);
    }

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Individual Measurements"));
        item->setIcon(QIcon("://icon/32x32/individual_size_file.png"));
        ui->pathTable->setItem(2, 0, item);
        item = new QTableWidgetItem(settings->getIndividualSizePath());
        item->setToolTip(settings->getIndividualSizePath());
        ui->pathTable->setItem(2, 1, item);
    }

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Multisize Measurements"));
        item->setIcon(QIcon("://icon/32x32/multisize_size_file.png"));
        ui->pathTable->setItem(3, 0, item);
        item = new QTableWidgetItem(settings->getMultisizePath());
        item->setToolTip(settings->getMultisizePath());
        ui->pathTable->setItem(3, 1, item);
    }

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Layouts"));
        item->setIcon(QIcon("://icon/32x32/layout.png"));
        ui->pathTable->setItem(4, 0, item);
        item = new QTableWidgetItem(settings->getLayoutPath());
        item->setToolTip(settings->getLayoutPath());
        ui->pathTable->setItem(4, 1, item);
    }

    {
        QTableWidgetItem *item = new QTableWidgetItem(tr("My Label templates"));
        item->setIcon(QIcon("://icon/32x32/labels.png"));
        ui->pathTable->setItem(5, 0, item);
        item = new QTableWidgetItem(settings->GetPathLabelTemplate());
        item->setToolTip(settings->GetPathLabelTemplate());
        ui->pathTable->setItem(5, 1, item);
    }

    ui->pathTable->verticalHeader()->setDefaultSectionSize(20);
    ui->pathTable->resizeColumnsToContents();
    ui->pathTable->resizeRowsToContents();

    connect(ui->pathTable, &QTableWidget::itemSelectionChanged, this, [this]()
    {
        ui->defaultButton->setEnabled(true);
        ui->defaultButton->setDefault(false);

        ui->editButton->setEnabled(true);
        ui->editButton->setDefault(true);
    });
}

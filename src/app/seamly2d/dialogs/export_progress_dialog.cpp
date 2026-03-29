//-----------------------------------------------------------------------------
//  @file   export_progress_dialog.cpp
//  @author Douglas S Caskey
//  @date   31 Mar 2026
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2026 Seamly2D project
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
//-----------------------------------------------------------------------------

#include "export_progress_dialog.h"
#include "ui_export_progress_dialog.h"

#include <QProgressBar>
#include <QTableWidget>


//---------------------------------------------------------------------------------------------------------------------
/// @brief ExportProgressDialog Progress report dialog.
///
/// This class provides a dialog to report the status and progress of files exported.
///
/// @param minimum property for the progress bar's minimum value.
/// @param maximum property for the progress bar's maximum value.
/// @param parent parent widget of dialog.
//---------------------------------------------------------------------------------------------------------------------
ExportProgressDialog::ExportProgressDialog(int minimum, int maximum, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportProgressDialog)

{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->export_ProgressBar->setMinimum(minimum);
    ui->export_ProgressBar->setMaximum(maximum);
    ui->export_ProgressBar->setValue(0);

    ui->files_TableWidget->setColumnWidth(StatusColumn, 120);
    ui->files_TableWidget->horizontalHeader()->setSectionResizeMode(FileNameColumn, QHeaderView::Stretch);

    adjustSize();
    setFixedHeight(this->height());
}

//---------------------------------------------------------------------------------------------------------------------
ExportProgressDialog::~ExportProgressDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief insertFileName Insert a new filename.
///
/// This method inserts a new row with the flename and default status of pending.
///
/// @param filename export filename.
//---------------------------------------------------------------------------------------------------------------------
void ExportProgressDialog::insertFileName(const QString &filename)
{
    ui->files_TableWidget->insertRow(0);
    ui->files_TableWidget->setItem(0, FileNameColumn, new QTableWidgetItem(filename));
    ui->files_TableWidget->setItem(0, StatusColumn, new QTableWidgetItem(tr("Pending")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setFileStatus Create a new draft block.
///
/// This method set the status of the exported file.
///
/// @param status status result of export.
//
/// @details
///  - ProgressStatus::Pending (default) displays "Pending".
///  - ProgressStatus::Completed displays "Completed".
///  - ProgressStatus::Failed displays "Failed".
//---------------------------------------------------------------------------------------------------------------------
void ExportProgressDialog::setFileStatus(const ProgressStatus &status)
{
    QTableWidgetItem *item = ui->files_TableWidget->item(0, StatusColumn);
    item->setTextAlignment(Qt::AlignHCenter);

    switch (status)
    {
        case ProgressStatus::Completed:
            item->setText(tr("Completed"));
            break;
        case ProgressStatus::Failed:
            item->setText(tr("Failed"));
            break;
        case ProgressStatus::Pending:
        default:
            item->setText(tr("Pending"));
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setProgress Sets the progress
///
/// This method sets the current progress of the progress bar widget.
///
/// @param value  current step value.
//---------------------------------------------------------------------------------------------------------------------
void ExportProgressDialog::setProgress(const int &value)
{
    ui->export_ProgressBar->setValue(value);
}

/// @brief showProgressBar show ot hide progress bar.
///
/// This method shows or hides the progress bar widget.
///
/// @param state visibilty state of the progress.
//
/// @details
///  - Visible if true.
///  - Hidden if false.
//---------------------------------------------------------------------------------------------------------------------
void ExportProgressDialog::showProgressBar(const bool &state)
{
    ui->export_ProgressBar->setVisible(state);
}

//-----------------------------------------------------------------------------
//  @file   export_progress_dialog.h
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

#ifndef EXPORT_PROGRESS_DIALOG_H
#define EXPORT_PROGRESS_DIALOG_H

#include <QDialog>
#include <QProgressBar>

enum {FileNameColumn = 0, StatusColumn};
enum class ProgressStatus
{
    Pending = 0,
    Completed,
    Failed
};

namespace Ui
{
    class ExportProgressDialog;
}


class ExportProgressDialog : public  QDialog
{
    Q_OBJECT

public:
    explicit                ExportProgressDialog(int minimum, int maximum, QWidget *parent = nullptr);

    virtual                ~ExportProgressDialog();

    void                    insertFileName(const QString &filename);
    void                    setFileStatus(const ProgressStatus &status);
    void                    setProgress(const int &value);
    void                    showProgressBar(const bool &state);

private:
    Q_DISABLE_COPY(ExportProgressDialog)
    Ui::ExportProgressDialog *ui;
};

#endif // EXPORT_PROGRESS_DIALOG_H

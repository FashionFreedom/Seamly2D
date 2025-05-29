//-------------------------------------------------------------------------------------------------
//  @file   fvupdater_dialog.h
//  @author Douglas S Caskey
//  @date   31 May, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
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
//-------------------------------------------------------------------------------------------------

#ifndef Updater_Dialog_H
#define Updater_Dialog_H

#include <QDialog>
#include <QDir>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

class QGraphicsScene;

namespace Ui
{
class UpdaterDialog;
}

class UpdaterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit           UpdaterDialog(QWidget *parent = nullptr);
    virtual           ~UpdaterDialog();
    void               updateDialog(const QString &osVersion, const QString &message);
    QDir               getDownloadDirectory();

private:
    Q_DISABLE_COPY_MOVE(UpdaterDialog)
    void               downloadUpdate();
    void               setProgressValue(int value);
    void               setDownloadDirectory();

    Ui::UpdaterDialog *m_ui;
    QDir               m_downloadDir;
};

#endif // Updater_Dialog_H

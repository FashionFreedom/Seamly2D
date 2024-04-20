// @file   about2d_dialog.h
// @author Douglas S Caskey
// @date   18 Apr, 2024
//
// @brief
// @copyright
// This source code is part of the Seamly2D project, a pattern making
// program to create and model patterns of clothing.
// Copyright (C) 2017-2024 Seamly2D project
// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.

/************************************************************************
 **
 **  @file   dialogaboutapp.h
 **  @author Patrick Proy <patrick(at)proy.org>
 **  @date   6 5, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef ABOUT2D_DIALOG_H
#define ABOUT2D_DIALOG_H

#include <QDialog>
#include <QSound>

namespace Ui
{
    class About2DAppDialog;
}

class About2DAppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit              About2DAppDialog(QWidget *parent = nullptr);
    virtual              ~About2DAppDialog();

protected:
    virtual void          showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

public slots:
	void                  setProgressValue(int val);

private:
    Ui::About2DAppDialog *ui;
    bool                  m_isInitialized;
    QSound               *m_beep;
    Q_DISABLE_COPY(About2DAppDialog)

    void                  copyToClipboard();
};

#endif // ABOUT2D_DIALOG_H

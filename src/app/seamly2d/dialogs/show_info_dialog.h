/************************************************************************
 **
 **  @file   show_info_dialog.h
 **  @author DSCaskey <dscaskey@gmail.com>
 **  @date   Nov 6, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  All Rights Reserved.
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

#ifndef SHOW_INFO_DIALOG_INFO
#define SHOW_INFO_DIALOG_INFO

#include <QDialog>
#include <QImage>

#include "../xml/vpattern.h"

namespace Ui
{
    class ShowInfoDialog;
}

class ShowInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit            ShowInfoDialog(VPattern *doc, QWidget *parent = nullptr);
    virtual            ~ShowInfoDialog();

protected:
    virtual void        showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::ShowInfoDialog *ui;
    VPattern           *doc;
    bool                m_isInitialized;
    Q_DISABLE_COPY(ShowInfoDialog)

    void                copyToClipboard();
    void                sendToPrinter();
    void                exportPdf();
};

#endif // SHOW_INFO_DIALOG_INFO

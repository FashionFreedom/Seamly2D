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

#ifndef DIALOGABOUTAPP_H
#define DIALOGABOUTAPP_H

#include <QDialog>

namespace Ui
{
    class DialogAboutApp;
}

class DialogAboutApp : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAboutApp(QWidget *parent = nullptr);
    virtual ~DialogAboutApp();

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::DialogAboutApp *ui;
    bool isInitialized;
    Q_DISABLE_COPY(DialogAboutApp)

    void FontPointSize(QWidget *w, int pointSize);
};

#endif // DIALOGABOUTAPP_H

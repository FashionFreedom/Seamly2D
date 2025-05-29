//-----------------------------------------------------------------------------
//  @file  me_about_dialog.h
//  @author Douglas S Caskey
//  @date   2 May, 2025
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   dialogaboutseamlyme.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   12 7, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#ifndef ME_ABOUT_DIALOG_H
#define ME_ABOUT_DIALOG_H

#include <QDialog>

namespace Ui
{
    class MeAboutDialog;
}

class MeAboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                 MeAboutDialog(QWidget *parent = nullptr);
    virtual                 ~MeAboutDialog();

protected:
    virtual void             showEvent(QShowEvent *event) override;

private:
    Q_DISABLE_COPY(MeAboutDialog)
    Ui::MeAboutDialog *ui;
    bool                     isInitialized;
};

#endif // ME_ABOUT_DIALOG_H

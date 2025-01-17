//  @file   me_welcome_dialog.h
//  @author Douglas S Caskey
//  @date   31 Dec, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2023 Seamly2D project
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

#ifndef ME_WELCOME_DIALOG_H
#define ME_WELCOME_DIALOG_H

#include <QDialog>

#include "../vmisc/def.h"
#include "../vmisc/vseamlymesettings.h"

namespace Ui
{
    class SeamlyMeWelcomeDialog;
}

class SeamlyMeWelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                   SeamlyMeWelcomeDialog(QWidget *parent = nullptr);
                              ~SeamlyMeWelcomeDialog();
    void                       apply();

protected:
    virtual void               changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    void                       seperatorChanged();

private:
    Q_DISABLE_COPY(SeamlyMeWelcomeDialog)
    Ui::SeamlyMeWelcomeDialog *ui;
    bool                       m_langChanged;
    VSeamlyMeSettings         *settings;

    void                       initUnits(const MeasurementsType &type);
};

#endif // ME_WELCOME_DIALOG_H

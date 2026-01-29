//  @file   welcome_dialog.h
//  @author Douglas S Caskey
//  @date   5 Jan, 2024
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

#ifndef WELCOME_DIALOG_H
#define WELCOME_DIALOG_H

#include <QLocale>
#include <QRadioButton>
#include <QDialog>

#include "../vmisc/def.h"
#include "../vmisc/vsettings.h"

namespace Ui
{
    class SeamlyWelcomeDialog;
}

class SeamlyWelcomeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                   SeamlyWelcomeDialog(QWidget *parent = nullptr);
                              ~SeamlyWelcomeDialog();
    void                       apply();

private:
    Q_DISABLE_COPY(SeamlyWelcomeDialog)
    Ui::SeamlyWelcomeDialog *ui;
    bool                       m_themeChanged;
    bool                       m_selectionSoundChanged;
    VSettings                 *settings;

    void                       initUnits();
    void                       languageChanged(int index);
    void                       setLocaleTooltip(QLocale locale, QRadioButton *button);
};

#endif // WELCOME_DIALOG_H

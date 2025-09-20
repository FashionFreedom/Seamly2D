//-----------------------------------------------------------------------------
//  @file   dialogundo.cpp
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2025 Seamly2D project
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
//  @file   dialogundo.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   Jun, 23 2014
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
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

#include "dialogundo.h"

#include <QCloseEvent>
#include <QLocale>
#include <QPushButton>
#include <Qt>

#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogundo.h"

//---------------------------------------------------------------------------------------------------------------------
DialogUndo::DialogUndo(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogUndo)
    , result(UndoButton::Undo)
{
    ui->setupUi(this);

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    bool opening = qApp->getOpeningPattern();
    if (opening)
    {
        ui->undo_PushButton->setDisabled(opening);
    }
    else
    {
        connect(ui->undo_PushButton, &QPushButton::clicked, this, [this]()
        {
            result = UndoButton::Undo;
            accept();
        });
    }
    connect(ui->fix_PushButton, &QPushButton::clicked, this, [this]()
    {
        result = UndoButton::Fix;
        accept();
    });

    setCursor(Qt::ArrowCursor);
}

//---------------------------------------------------------------------------------------------------------------------
DialogUndo::~DialogUndo()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogUndo::closeEvent(QCloseEvent *event)
{
    result = UndoButton::Undo; // set result to Undo.
    accept();                  // need to accept so calling method handles Close as if Undo was selected.
    event->accept();           // prevent propagation of event to parent.
}

//---------------------------------------------------------------------------------------------------------------------
void DialogUndo::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

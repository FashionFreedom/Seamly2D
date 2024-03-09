/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialoggroup.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "dialoggroup.h"

#include <QLineEdit>

#include "ui_dialoggroup.h"

//---------------------------------------------------------------------------------------------------------------------
EditGroupDialog::EditGroupDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::EditGroupDialog),
      group()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/group.png"));

    initializeOkCancel(ui);
    DialogTool::CheckState();

    connect(ui->lineEditName, &QLineEdit::textChanged, this, &EditGroupDialog::NameChanged);
}

//---------------------------------------------------------------------------------------------------------------------
EditGroupDialog::~EditGroupDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::SetName(const QString &name)
{
    ui->lineEditName->setText(name);
}

//---------------------------------------------------------------------------------------------------------------------
QString EditGroupDialog::GetName() const
{
    return ui->lineEditName->text();
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::ShowDialog(bool click)
{
    if (not click)
    {
        if (group.isEmpty())
        {
            return;
        }

        SetName(tr("New group"));

        setModal(true);
        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    if (selected)
    {
        group.insert(object, tool);
    }
    else
    {
        group.remove(object);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::NameChanged()
{
    ui->lineEditName->text().isEmpty() ? flagName = false : flagName = true;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, quint32> EditGroupDialog::GetGroup() const
{
    return group;
}

//-----------------------------------------------------------------------------
//  @file   dialoggroup.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   dialoggroup.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   4 Apr, 2016
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "dialoggroup.h"

#include <QLineEdit>

#include "ui_dialoggroup.h"

//---------------------------------------------------------------------------------------------------------------------
EditGroupDialog::EditGroupDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::EditGroupDialog)
    , group()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/group.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

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

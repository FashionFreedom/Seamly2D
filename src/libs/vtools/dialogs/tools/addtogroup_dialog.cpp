//-----------------------------------------------------------------------------
//  @file   addtogroup_dialog.cpp
//  @author Douglas S Caskey
//  @date   Mar 14, 2023
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

#include "addtogroup_dialog.h"
#include "ui_addtogroup_dialog.h"

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../tools/dialogtool.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/undocommands/addgroup.h"

#include <QStringList>
#include <QComboBox>

//---------------------------------------------------------------------------------------------------------------------
AddToGroupDialog::AddToGroupDialog(VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::AddToGroupDialog)
    , m_doc(qApp->getCurrentDocument())
    , m_groupData()
    , m_beep(new QSound(qApp->Settings()->getSelectionSound()))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/icon/32x32/group.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    fillNameBox();

    initializeOkCancel(ui);
}

//---------------------------------------------------------------------------------------------------------------------
 AddToGroupDialog::~AddToGroupDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void  AddToGroupDialog::fillNameBox()
{
    QMap<quint32, GroupAttributes> groups = m_doc->getGroups();
    QStringList groupNames;
    auto i = groups.constBegin();
    while (i != groups.constEnd())
    {
        const GroupAttributes data = i.value();
        groupNames.append(data.name);
        ++i;
    }
    ui->groupName_ComboBox->addItems(groupNames);
}

//---------------------------------------------------------------------------------------------------------------------
QString  AddToGroupDialog::getName() const
{
    return ui->groupName_ComboBox->currentText();
}


//---------------------------------------------------------------------------------------------------------------------
void  AddToGroupDialog::ShowDialog(bool click)
{
    if (not click)
    {
        if (m_groupData.isEmpty())
        {
            return;
        }

        setModal(true);
        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void  AddToGroupDialog::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    if (selected)
    {
        m_groupData.insert(object, tool);
        m_beep->play();
    }
    else
    {
        m_groupData.remove(object);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, quint32>  AddToGroupDialog::getGroupData() const
{
    return m_groupData;
}

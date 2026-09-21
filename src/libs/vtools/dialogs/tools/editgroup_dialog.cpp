//---------------------------------------------------------------------------------------------------------------------
//  @file   editgroup_dialog.cpp
//  @author Douglas S Caskey
//  @date   Mar 1, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialog_editgroup.cpp
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
//---------------------------------------------------------------------------------------------------------------------

#include "editgroup_dialog.h"
#include "ui_editgroup_dialog.h"

#include <QLineEdit>
#include <QStringList>
#include <QInputDialog>
#include <QComboBox>

#include "dialogtool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/undocommands/addgroup.h"
#include "../vwidgets/color_combobox.h"
#include "../vwidgets/linetype_combobox.h"
#include "../vwidgets/lineweight_combobox.h"

Q_LOGGING_CATEGORY(EditGroupDialogLog, "editGroupDialogLog")

//---------------------------------------------------------------------------------------------------------------------
EditGroupDialog::EditGroupDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::EditGroupDialog)
    , m_doc(qApp->getCurrentDocument())
    , m_group_data()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/icon/32x32/edit.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->group_color_combobox->removeItem(ui->group_color_combobox->findData(ColorByGroup));                //remove color "BY Group" item
    ui->group_linetype_combobox->removeItem(ui->group_linetype_combobox->findData(LineTypeByGroup));       //remove linetype "BY Group" item
    ui->group_linetype_combobox->removeItem(ui->group_linetype_combobox->findData(LineTypeNone));          //remove linetype "No Pen" item
    ui->group_lineweight_combobox->removeItem(ui->group_lineweight_combobox->findData(LineWeightByGroup)); //remove lineweight "BY Group" item
    ui->group_name_line_edit->setText(createGroupName());

    initializeOkCancel(ui);
    DialogTool::CheckState();
    connect(ui->group_name_line_edit, &QLineEdit::textChanged, this, &EditGroupDialog::nameChanged);
}

//---------------------------------------------------------------------------------------------------------------------
 EditGroupDialog::~EditGroupDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setName(const QString &name)
{
    ui->group_name_line_edit->setText(name);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getName() const
{
    return ui->group_name_line_edit->text();
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::ShowDialog(bool click)
{
    if (!click)
    {
        if (m_group_data.isEmpty())
        {
            return;
        }

        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    if (selected)
    {
        m_group_data.insert(object, tool);
    }
    else
    {
        m_group_data.remove(object);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::nameChanged()
{
    ui->group_name_line_edit->text().isEmpty() ? flagName = false : flagName = true;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, quint32>  EditGroupDialog::getGroupData() const
{
    return m_group_data;
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getColor() const
{
    return getComboBoxCurrentData(ui->group_color_combobox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setColor(const QString &color)
{
    changeCurrentData(ui->group_color_combobox, color);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getLineType() const
{
    return getComboBoxCurrentData(ui->group_linetype_combobox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setLineType(const QString &type)
{
    changeCurrentData(ui->group_linetype_combobox, type);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getLineWeight() const
{
    return getComboBoxCurrentData(ui->group_lineweight_combobox, DefaultLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setLineWeight(const QString &weight)
{
    changeCurrentData(ui->group_lineweight_combobox, weight);
}

//---------------------------------------------------------------------------------------------------------------------
QString EditGroupDialog::createGroupName() const
{
    QStringList group_names = m_doc->groupListByName();

    const QString default_name = tr("Group");
    QString group_name = default_name + "_1";
    int i = 0;

    while(group_names.contains(group_name))
    {
        group_name = default_name + QString("_%1").arg(++i);
    }
    return group_name;
}

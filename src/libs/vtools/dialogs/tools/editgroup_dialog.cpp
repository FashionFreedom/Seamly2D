//-----------------------------------------------------------------------------
//  @file   dialog_editgroup.cpp
//  @author Douglas S Caskey
//  @date   Mar 1, 2023
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
//-----------------------------------------------------------------------------

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
    , m_groupData()
    , m_oldGroupName()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/icon/32x32/edit.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->groupColor_ComboBox->removeItem(ui->groupColor_ComboBox->findData(ColorByGroup));                //remove color "BY Group" item
    ui->groupLineType_ComboBox->removeItem(ui->groupLineType_ComboBox->findData(LineTypeByGroup));       //remove linetype "BY Group" item
    ui->groupLineType_ComboBox->removeItem(ui->groupLineType_ComboBox->findData(LineTypeNone));          //remove linetype "No Pen" item
    ui->groupLineWeight_ComboBox->removeItem(ui->groupLineWeight_ComboBox->findData(LineWeightByGroup)); //remove lineweight "BY Group" item
    m_oldGroupName = ui->groupName_LineEdit->text();

    initializeOkCancel(ui);
    DialogTool::CheckState();
    connect(ui->groupName_LineEdit, &QLineEdit::textChanged, this, &EditGroupDialog::nameChanged);
}

//---------------------------------------------------------------------------------------------------------------------
 EditGroupDialog::~EditGroupDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setName(const QString &name)
{
    ui->groupName_LineEdit->setText(name);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getName() const
{
    return ui->groupName_LineEdit->text();
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::ShowDialog(bool click)
{
    if (not click)
    {
        if (m_groupData.isEmpty())
        {
            return;
        }

        //setName(tr("New group"));

        setModal(true);
        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    if (selected)
    {
        m_groupData.insert(object, tool);
    }
    else
    {
        m_groupData.remove(object);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void EditGroupDialog::nameChanged()
{
    ui->groupName_LineEdit->text().isEmpty() ? flagName = false : flagName = true;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, quint32>  EditGroupDialog::getGroupData() const
{
    return m_groupData;
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getColor() const
{
    return GetComboBoxCurrentData(ui->groupColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setColor(const QString &color)
{
    ChangeCurrentData(ui->groupColor_ComboBox, color);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getLineType() const
{
    return GetComboBoxCurrentData(ui->groupLineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setLineType(const QString &type)
{
    ChangeCurrentData(ui->groupLineType_ComboBox, type);
}

//---------------------------------------------------------------------------------------------------------------------
QString  EditGroupDialog::getLineWeight() const
{
    return GetComboBoxCurrentData(ui->groupLineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
void  EditGroupDialog::setLineWeight(const QString &weight)
{
    ChangeCurrentData(ui->groupLineWeight_ComboBox, weight);
}

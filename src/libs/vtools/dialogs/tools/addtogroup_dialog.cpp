//---------------------------------------------------------------------------------------------------------------------
//  @file   addtogroup_dialog.cpp
//  @author Douglas S Caskey
//  @date   Mar 14, 2023
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

#include "addtogroup_dialog.h"
#include "ui_addtogroup_dialog.h"

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../tools/dialogtool.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/dialogs/tools/editgroup_dialog.h"
#include "../vtools/undocommands/addgroup.h"
#include "../vwidgets/vabstractmainwindow.h"

#include <QStringList>
#include <QComboBox>
#include <QMessageBox>

//---------------------------------------------------------------------------------------------------------------------
AddToGroupDialog::AddToGroupDialog(VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::AddToGroupDialog)
    , m_doc(qApp->getCurrentDocument())
    , m_group_data()
    , m_beep(new QSoundEffect())
{
    ui->setupUi(this);

    m_beep->setSource(QUrl(qApp->Settings()->getSelectionSound()));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/icon/32x32/group.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    fillNameBox();

    initializeOkCancel(ui);

    connect(ui->add_group_pushbutton, &QPushButton::clicked, this,  &AddToGroupDialog::addGroup);

}

//---------------------------------------------------------------------------------------------------------------------
 AddToGroupDialog::~AddToGroupDialog()
{
    delete ui;
    delete m_beep;
}

//---------------------------------------------------------------------------------------------------------------------
void  AddToGroupDialog::fillNameBox()
{
    ui->group_name_combobox->clear();

    QStringList group_names = m_doc->groupListByName();
    ui->group_name_combobox->addItems(group_names);
}

//---------------------------------------------------------------------------------------------------------------------
QString  AddToGroupDialog::getName() const
{
    return ui->group_name_combobox->currentText();
}


//---------------------------------------------------------------------------------------------------------------------
void  AddToGroupDialog::ShowDialog(bool click)
{
    if (not click)
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
void  AddToGroupDialog::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    if (selected)
    {
        m_group_data.insert(object, tool);
        m_beep->play();
    }
    else
    {
        m_group_data.remove(object);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<quint32, quint32>  AddToGroupDialog::getGroupData() const
{
    return m_group_data;
}

void AddToGroupDialog::addGroup()
{
    QScopedPointer<EditGroupDialog> dialog(new EditGroupDialog(new VContainer(qApp->translateVariables(),
                                                                  qApp->patternUnitP()), NULL_ID, this));
    SCASSERT(dialog != nullptr)

    QString group_name;
    while (1)
    {
        const bool result = dialog->exec();
        group_name = dialog->getName();
        if (result == false || group_name.isEmpty())
        {
            return;
        }
        bool exists = m_doc->groupNameExists(group_name);
        if (exists == false)
        {
            break;
        }

        QMessageBox message_box;
        message_box.setWindowTitle(tr("Name Exists"));
        message_box.setIcon(QMessageBox::Warning);
        message_box.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        message_box.setDefaultButton(QMessageBox::Retry);
        message_box.setText(tr("The action can't be completed because the group name already exists."));
        int boxResult = message_box.exec();

        switch (boxResult)
        {
            case QMessageBox::Retry:
                break;    // Repeat Add Group Dialog
            case QMessageBox::Cancel:
                return;   // Exit Add Group Dialog
            default:
                break;    // should never be reached
        }
    }

    const quint32 next_id = VContainer::getNextId();

    const QDomElement group = m_doc->createGroup(next_id, group_name, dialog->getColor(), dialog->getLineType(),
                                                  dialog->getLineWeight(), dialog->getGroupData());

    if (!group.isNull())
    {
        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)

        AddGroup *command = new AddGroup(group, m_doc);
        connect(command, &AddGroup::updateGroups, window, &VAbstractMainWindow::updateGroups);
        qApp->getUndoStack()->push(command);

        fillNameBox();

        int index = ui->group_name_combobox->findText(group_name);
        if (index != -1)
        {
            ui->group_name_combobox->setCurrentIndex(index);
        }
    }
}

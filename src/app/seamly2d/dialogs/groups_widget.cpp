/***************************************************************************
 **  @file   groups_widget.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

 /************************************************************************
  **
  **  @file   vwidgetgroups.cpp
  **  @author Roman Telezhynskyi <dismine(at)gmail.com>
  **  @date   6 4, 2016
  **
  **  @brief
  **  @copyright
  **  This source code is part of the Valentina project, a pattern making
  **  program, whose allow create and modeling patterns of clothing.
  **  Copyright (C) 2016 Valentina project
  ** <https://bitbucket.org/dismine/valentina> All Rights Reserved.
  **
  **  Valentina is free software: you can redistribute it and/or modify
  **  it under the terms of the GNU General Public License as published by
  **  the Free Software Foundation, either version 3 of the License, or
  **  (at your option) any later version.
  **
  **  Valentina is distributed in the hope that it will be useful,
  **  but WITHOUT ANY WARRANTY; without even the implied warranty of
  **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  **  GNU General Public License for more details.
  **
  **  You should have received a copy of the GNU General Public License
  **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
  **
  *************************************************************************/

#include "groups_widget.h"
#include "ui_groups_widget.h"
#include "../core/vapplication.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/dialogs/tools/dialogtool.h"
#include "../vtools/dialogs/tools/editgroup_dialog.h"
#include "../vtools/undocommands/delgroup.h"
#include "../vtools/undocommands/addgroup.h"
#include "../vtools/undocommands/add_groupitem.h"
#include "../vtools/undocommands/remove_groupitem.h"
#include "../vtools/undocommands/move_groupitem.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vgeometry/vabstractarc.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/group_tablewidgetitem.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/logging.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"

#include <Qt>
#include <QApplication>
#include <QColor>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QMetaObject>
#include <QObject>
#include <QPointer>
#include <QScopedPointer>
#include <QSettings>
#include <QTableWidget>
#include <QtDebug>

Q_LOGGING_CATEGORY(WidgetGroups, "vwidgetgroups")

//---------------------------------------------------------------------------------------------------------------------
GroupsWidget::GroupsWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GroupsWidget)
    , m_doc(doc)
    , m_data(data)
    , m_patternHasGroups(false)
    , m_currentGroupId(0)

{
    ui->setupUi(this);
    QSettings settings;
    ui->groups_Splitter->restoreState(settings.value("splitterSizes").toByteArray());

    fillTable(m_doc->getGroups());
    ui->groups_TableWidget->sortItems(settings.value("groupSort", 4).toInt(), Qt::AscendingOrder);

    connect(m_doc, &VAbstractPattern::patternHasGroups, this, &GroupsWidget::draftBlockHasGroups);

    connect(ui->showAllGroups_ToolButton,   &QToolButton::clicked, this,  &GroupsWidget::showAllGroups);
    connect(ui->hideAllGroups_ToolButton,   &QToolButton::clicked, this,  &GroupsWidget::hideAllGroups);
    connect(ui->lockAllGroups_ToolButton,   &QToolButton::clicked, this,  &GroupsWidget::lockAllGroups);
    connect(ui->unlockAllGroups_ToolButton, &QToolButton::clicked, this,  &GroupsWidget::unlockAllGroups);
    connect(ui->addGroup_ToolButton,        &QToolButton::clicked, this,  &GroupsWidget::addGroupToList);
    connect(ui->deleteGroup_ToolButton,     &QToolButton::clicked, this,  &GroupsWidget::deleteGroupFromList);
    connect(ui->editGroup_ToolButton,       &QToolButton::clicked, this,  &GroupsWidget::editGroup);

    connect(ui->groups_TableWidget,         &QTableWidget::cellChanged,        this, &GroupsWidget::renameGroup);
    connect(ui->groups_TableWidget,         &QTableWidget::cellClicked,        this, &GroupsWidget::cellClicked);
    connect(ui->groups_TableWidget,         &QTableWidget::cellDoubleClicked,  this, &GroupsWidget::cellDoubleClicked);
    connect(ui->groups_TableWidget,         &QTableWidget::currentCellChanged, this, &GroupsWidget::fillGroupItemList);

    ui->groups_TableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->groups_TableWidget, &QTableWidget::customContextMenuRequested, this, &GroupsWidget::groupContextMenu);

    ui->groupItems_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->groupItems_ListWidget, &QListWidget::customContextMenuRequested, this, &GroupsWidget::groupItemContextMenu);
    connect(ui->groupItems_ListWidget, &QListWidget::itemDoubleClicked,          this, &GroupsWidget::itemDoubleClicked);

    connect(ui->groups_TableWidget->horizontalHeader(), &QHeaderView::sectionClicked, this, &GroupsWidget::headerClicked);
    connect(ui->groups_Splitter, &QSplitter::splitterMoved, this, &GroupsWidget::splitterMoved);
}

//---------------------------------------------------------------------------------------------------------------------
GroupsWidget::~GroupsWidget()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief changeEvent Handle language change event to retranslate the widget's ui.
 * @param event QEvent sent to widget.
 */
 void GroupsWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        updateGroups();
    }

    // remember to call base class implementation
    QWidget::changeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::groupVisibilityChanged(int row, int column)
{
    if (column != 0) return;

    QTableWidgetItem *item = ui->groups_TableWidget->item(row, column);
    const quint32 groupId = item->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(groupId);
    if (locked == false)
    {
        const bool visible = !m_doc->getGroupVisibility(groupId);
        setGroupVisibility(item, groupId, visible);
    }
        else
    {
        QApplication::beep();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::groupLockChanged(int row, int column)
{
    if (column != 1) return;

    QTableWidgetItem *item = ui->groups_TableWidget->item(row, column);
    if (!item) return;
    const quint32 groupId = item->data(Qt::UserRole).toUInt();
    const bool locked = !m_doc->getGroupLock(groupId);
    m_doc->setGroupLock(groupId, locked);
    if (locked)
    {
        item->setIcon(QIcon("://icon/32x32/lock_on.png"));
        item = ui->groups_TableWidget->item(row, 4);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
    else
    {
        item->setIcon(QIcon("://icon/32x32/lock_off.png"));
        item = ui->groups_TableWidget->item(row, 4);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::renameGroup(int row, int column)
{
    if (column != 4)
    {
        return;
    }

    const quint32 groupId = ui->groups_TableWidget->item(row, 0)->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(groupId);
    if (locked == false)
    {
        m_doc->setGroupName(groupId, ui->groups_TableWidget->item(row, column)->text());
        updateGroups();
    }
}

void GroupsWidget::showGroups(QMap<quint32,QString> groups)
{
    QMap<quint32,QString>::const_iterator i;
    for (i = groups.constBegin(); i != groups.constEnd(); ++i)
    {
        for (int j = 0; j < ui->groups_TableWidget->rowCount(); ++j)
        {
            QTableWidgetItem *item = ui->groups_TableWidget->item(j, 0);
            if (item)
            {
                quint32 groupId = item->data(Qt::UserRole).toUInt();
                if (groupId == i.key())
                {
                    setGroupVisibility(item, groupId, true);
                }
            }
        }
    }
    updateGroups();
}

void GroupsWidget::showAllGroups()
{
    ui->groups_TableWidget->setSortingEnabled(false);
    ui->groups_TableWidget->blockSignals(true);
    qCDebug(WidgetGroups, "Show All Groups");
    quint32 groupId;
    bool locked;
    for (int i = 0; i < ui->groups_TableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->groups_TableWidget->item(i, 0);
        if (!item)
        {
            return;
        }
        groupId = item->data(Qt::UserRole).toUInt();
        locked = m_doc->getGroupLock(groupId);
        if (item && locked == false)
        {
            setGroupVisibility(item, groupId, true);
        }
    }
    ui->groups_TableWidget->blockSignals(false);
    ui->groups_TableWidget->setSortingEnabled(true);
}

void GroupsWidget::hideAllGroups()
{
    ui->groups_TableWidget->setSortingEnabled(false);
    ui->groups_TableWidget->blockSignals(true);
    qCDebug(WidgetGroups, "Hide All Groups");
    quint32 groupId;
    bool locked;
    for (int row = 0; row < ui->groups_TableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_TableWidget->item(row, 0);
        if (!item)
        {
            return;
        }
        groupId = item->data(Qt::UserRole).toUInt();
        locked = m_doc->getGroupLock(groupId);
        if (item && locked == false)
        {
            setGroupVisibility(item, groupId, false);
        }
    }
    ui->groups_TableWidget->blockSignals(false);
    ui->groups_TableWidget->setSortingEnabled(true);
}

void GroupsWidget::lockAllGroups()
{
    qCDebug(WidgetGroups, "Lock All Groups");
    for (int row = 0; row < ui->groups_TableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_TableWidget->item(row, 1);
        if (!item)
        {
            return;
        }
        const quint32 groupId = item->data(Qt::UserRole).toUInt();
        m_doc->setGroupLock(groupId, true);
        item->setIcon(QIcon("://icon/32x32/lock_on.png"));
        item = ui->groups_TableWidget->item(row, 4);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

void GroupsWidget::unlockAllGroups()
{
    qCDebug(WidgetGroups, "Unlock All Groups");
    for (int row = 0; row < ui->groups_TableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_TableWidget->item(row, 1);
        if (!item)
        {
            return;
        }
        const quint32 groupId = item->data(Qt::UserRole).toUInt();
        m_doc->setGroupLock(groupId, false);
        item->setIcon(QIcon("://icon/32x32/lock_off.png"));
        item = ui->groups_TableWidget->item(row, 4);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void GroupsWidget::addGroupToList()
{
    QScopedPointer<EditGroupDialog> dialog(new EditGroupDialog(new VContainer(qApp->TrVars(),
                                                                  qApp->patternUnitP()), NULL_ID, this));
    SCASSERT(dialog != nullptr)

    QString groupName;
    while (1)
    {
        const bool result = dialog->exec();
        groupName = dialog->getName();
        if (result == false || groupName.isEmpty())
        {
            return;
        }
        bool exists = m_doc->groupNameExists(groupName);
        if (exists == false)
        {
            break;
        }

        QMessageBox messageBox;
        messageBox.setWindowTitle(tr("Name Exists"));
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Retry);
        messageBox.setText(tr("The action can't be completed because the group name already exists."));
        int boxResult = messageBox.exec();

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

    const quint32 nextId = VContainer::getNextId();
    qCDebug(WidgetGroups, "Group Name = %s", qUtf8Printable(groupName));
    qCDebug(WidgetGroups, "Next Id = %d", nextId);

    const QDomElement group = m_doc->createGroup(nextId, groupName, dialog->getColor(), dialog->getLineType(),
                                                  dialog->getLineWeight(), dialog->getGroupData());

    if (!group.isNull())
    {
        qCDebug(WidgetGroups, "Add a Group to List");
        AddGroup *command = new AddGroup(group, m_doc);
        connect(command, &AddGroup::updateGroups, this, &GroupsWidget::updateGroups);
        qApp->getUndoStack()->push(command);
    }
}

void GroupsWidget::deleteGroupFromList()
{
    const quint32 groupId = getGroupId();
    qCDebug(WidgetGroups, "Remove Group %d from List", groupId);
    const bool locked = m_doc->getGroupLock(groupId);
    QTableWidgetItem *item = ui->groups_TableWidget->currentItem();
    if (!item)
    {
        return;
    }

    if (item && (locked == false))
    {
        DelGroup *command = new DelGroup(m_doc, groupId);
        connect(command, &DelGroup::updateGroups, this, &GroupsWidget::updateGroups);
        qApp->getUndoStack()->push(command);
    }
}

void GroupsWidget::editGroup()
{
    ui->groups_TableWidget->blockSignals(true);
    qCDebug(WidgetGroups, "Edit Group List");
    const int row = ui->groups_TableWidget->currentRow();
    if (ui->groups_TableWidget->rowCount() == 0 || row == -1 || row >= ui->groups_TableWidget->rowCount())
    {
        ui->groups_TableWidget->blockSignals(false);
        return;
    }

    const quint32 groupId = ui->groups_TableWidget->item(row, 0)->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(groupId);
    QString oldGroupName = m_doc->getGroupName(groupId);
    if (locked == false)
    {
        qCDebug(WidgetGroups, "Row = %d", row);

        QScopedPointer<EditGroupDialog> dialog(new EditGroupDialog(new VContainer(qApp->TrVars(),
                                                                   qApp->patternUnitP()), NULL_ID, this));
        dialog->setName(oldGroupName);
        dialog->setColor(m_doc->getGroupColor(groupId));
        dialog->setLineType(m_doc->getGroupLineType(groupId));
        dialog->setLineWeight(m_doc->getGroupLineWeight(groupId));
        dialog->setWindowTitle(tr("Edit Group"));

        QString groupName;
        while (1)
        {
            const bool result = dialog->exec();
            groupName = dialog->getName();
            if (result == false || groupName.isEmpty())
            {
                ui->groups_TableWidget->blockSignals(false);
                return;
            }
            bool exists = m_doc->groupNameExists(groupName);
            if (exists == false || groupName == oldGroupName)
            {
                break;
            }

            QMessageBox messageBox;
            messageBox.setWindowTitle(tr("Name Exists"));
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Retry);
            messageBox.setText(tr("The action can't be completed because the group name already exists."));
            int boxResult = messageBox.exec();

            switch (boxResult)
            {
                case QMessageBox::Retry:
                    break;    // Repeat Edit Group Dialog
                case QMessageBox::Cancel:
                    ui->groups_TableWidget->blockSignals(false);
                    return;   // Exit Edit Group Dialog
                default:
                    break;    // should never be reached
            }
        }

        const QString groupColor = dialog->getColor();
        const QString groupLineType = dialog->getLineType();
        const QString groupLineWeight = dialog->getLineWeight();

        // Add color item
        QTableWidgetItem *item = ui->groups_TableWidget->item(row, 3);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setSizeHint(QSize(20, 20));
        QPixmap pixmap(20, 20);
        pixmap.fill(QColor(groupColor));
        item->setIcon(QIcon(pixmap));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Group color"));
        // Add group name item
        item = ui->groups_TableWidget->item(row, 4);
        item->setText(groupName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        m_doc->setGroupName(groupId, groupName);
        m_doc->setGroupColor(groupId, groupColor);
        m_doc->setGroupLineType(groupId, groupLineType);
        m_doc->setGroupLineWeight(groupId, groupLineWeight);

        updateGroups();
    }
    else
    {
        QApplication::beep();
    }
    ui->groups_TableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::groupContextMenu(const QPoint &pos)
{
    //ui->groups_TableWidget->setSortingEnabled(false);
    //ui->groups_TableWidget->blockSignals(true);

    QTableWidgetItem *item = ui->groups_TableWidget->itemAt(pos);
    if (!item)
    {
        return;
    }

    const int row = item->row();
    item = ui->groups_TableWidget->item(row, 0);
    const quint32 groupId = item->data(Qt::UserRole).toUInt();

    const bool locked = m_doc->getGroupLock(groupId);
    if (locked)
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionEdit = menu->addAction(QIcon("://icon/32x32/edit.png"), tr("Edit"));
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));
    QAction *selectedAction = menu->exec(ui->groups_TableWidget->viewport()->mapToGlobal(pos));
    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == actionEdit)
    {
        editGroup();
    }
    else if (selectedAction == actionDelete)
    {
        DelGroup *command = new DelGroup(m_doc, groupId);
        connect(command, &DelGroup::updateGroups, this, &GroupsWidget::updateGroups);
        qApp->getUndoStack()->push(command);
    }

    //ui->groups_TableWidget->setSortingEnabled(true);
    //ui->groups_TableWidget->blockSignals(false);

}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::updateGroups()
{
    int row = ui->groups_TableWidget->currentRow();
    fillTable(m_doc->getGroups());
    if (ui->groups_TableWidget->rowCount() != 0 || row != -1 || row <= ui->groups_TableWidget->rowCount())
    {
        ui->groups_TableWidget->selectRow(row);
    }
    fillGroupItemList();
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::fillTable(const QMap<quint32, GroupAttributes> &groups)
{
    ui->groups_TableWidget->blockSignals(true);
    ui->groups_TableWidget->clear();
    ui->groups_TableWidget->setColumnCount(5);
    ui->groups_TableWidget->setRowCount(groups.size());
    ui->groups_TableWidget->setSortingEnabled(false);

    qint32 currentRow = -1;
    auto i = groups.constBegin();
    while (i != groups.constEnd())
    {
        ++currentRow;
        const GroupAttributes data = i.value();

        // Add visibility item
        GroupTableWidgetItem *item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(20, 20));

        setGroupVisibility(item, i.key(), data.visible);

        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Show which groups in the list are visible"));
        ui->groups_TableWidget->setItem(currentRow, 0, item);

        // Add locked item
        item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(20, 20));
        item->setIcon(data.locked ? QIcon("://icon/32x32/lock_on.png") : QIcon("://icon/32x32/lock_off.png"));
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Show which groups in the list are locked"));
        ui->groups_TableWidget->setItem(currentRow, 1, item);

        // Add contain objects Item
        item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setIcon(!m_doc->isGroupEmpty(i.key()) ? QIcon("://icon/32x32/history.png") : QIcon());
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Show which groups contain objects"));
        ui->groups_TableWidget->setItem(currentRow, 2, item);

        // Add color item
        item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setSizeHint(QSize(20, 20));
        QPixmap pixmap(20, 20);
        pixmap.fill(QColor(data.color));
        item->setIcon(QIcon(pixmap));
        item->setData(Qt::UserRole, data.color);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Group color"));
        ui->groups_TableWidget->setItem(currentRow, 3, item);

        // Add group name item
        QTableWidgetItem *nameItem = new QTableWidgetItem(data.name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        nameItem->setToolTip(tr("Group name"));
        ui->groups_TableWidget->setItem(currentRow, 4, nameItem);
        if (data.locked)
        {
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        }
        ++i;
    }

    ui->groups_TableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(makeHeaderName(tr("Visible"))));
    ui->groups_TableWidget->horizontalHeaderItem(0)->setToolTip(tr("Group is visible"));
    ui->groups_TableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(makeHeaderName(tr("Locked"))));
    ui->groups_TableWidget->horizontalHeaderItem(1)->setToolTip(tr("Group is locked"));
    ui->groups_TableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(makeHeaderName(tr("Objects"))));
    ui->groups_TableWidget->horizontalHeaderItem(2)->setToolTip(tr("Group has objects"));
    ui->groups_TableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(makeHeaderName(tr("Color"))));
    ui->groups_TableWidget->horizontalHeaderItem(3)->setToolTip(tr("Group color"));
    ui->groups_TableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Name")));
    ui->groups_TableWidget->horizontalHeaderItem(4)->setToolTip(tr("Group name"));
    ui->groups_TableWidget->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->groups_TableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->groups_TableWidget->resizeColumnsToContents();
    ui->groups_TableWidget->resizeRowsToContents();
    ui->groups_TableWidget->setSortingEnabled(true);
    ui->groups_TableWidget->blockSignals(false);
}

void GroupsWidget::draftBlockHasGroups(bool value)
{
    m_patternHasGroups = value;

    ui->showAllGroups_ToolButton->setEnabled(value);
    ui->hideAllGroups_ToolButton->setEnabled(value);
    ui->lockAllGroups_ToolButton->setEnabled(value);
    ui->unlockAllGroups_ToolButton->setEnabled(value);
    ui->deleteGroup_ToolButton->setEnabled(value);
    ui->editGroup_ToolButton->setEnabled(value);
    qCDebug(WidgetGroups, "Draft Block Has Groups = %d", value);
}

void GroupsWidget::setAddGroupEnabled(bool value)
{
    ui->addGroup_ToolButton->setEnabled(value);
}

void GroupsWidget::clear()
{
    ui->groups_TableWidget->setRowCount(0);
    ui->groups_TableWidget->clearContents();
    ui->groupItems_ListWidget->clear();
}

quint32 GroupsWidget::getGroupId()
{
    QTableWidgetItem *item = ui->groups_TableWidget->currentItem();
    if (!item)
    {
        return 0;
    }
    int row = ui->groups_TableWidget->row(item);
    qCDebug(WidgetGroups, "Row = %d\n", row);
    const quint32 groupId = ui->groups_TableWidget->item(row, 0)->data(Qt::UserRole).toUInt();
    return groupId;
}

QString GroupsWidget::getCurrentGroupName()
{
    QTableWidgetItem *item = ui->groups_TableWidget->currentItem();
    if (!item)
    {
        return QString();
    }
    int row = ui->groups_TableWidget->row(item);
    qCDebug(WidgetGroups, "Row = %d\n", row);
    const QString groupName = ui->groups_TableWidget->item(row, 4)->text();
    return groupName;
}

void GroupsWidget::fillGroupItemList()
{
    ui->groupItems_ListWidget->blockSignals(true);
    ui->groupItems_ListWidget->clear();
    QString groupName = getCurrentGroupName();
    QDomElement groupDomElement = m_doc->getGroupByName(groupName);
    if (groupDomElement.isNull())
    {
        return;
    }
    QMap<quint32, Tool> history = m_doc->getGroupObjHistory();
    QPair<bool, QMap<quint32, quint32> > group = m_doc->parseItemElement(groupDomElement);
    const QMap<quint32, quint32> groupData = group.second;
    if (!groupData.isEmpty())
    {
        auto i = groupData.begin();
        while (i != groupData.end())
        {
           quint32 toolId = i.value();
           quint32 objId = i.key();
           Tool tooltype = history.value(toolId);
           addGroupItem(toolId, objId, tooltype);
           ++i;
        }
    }
    ui->groupItems_ListWidget->sortItems(Qt::AscendingOrder);
    ui->groupItems_ListWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief addGroupItem Add group item with description in listwidget.
 * @param toolID ToolID of item to add to list.
 * @param objId ObjectID of item to add to list.
 * @param tooltype Tooltype of item to add to list.
 */
void GroupsWidget::addGroupItem(const quint32 &toolId, const quint32 &objId, const Tool &tooltype)
{
    // This check helps to find missing tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were used in history.");

    QString iconFileName = "";
    QString objName = tr("Unknown Object");
    const QDomElement domElement = m_doc->elementById(toolId);
    if (domElement.isElement() == false)
    {
        qDebug() << "Can't find element by id" << Q_FUNC_INFO;
        return;
    }
        try
        {
            switch ( tooltype )
            {
                case Tool::Arrow:
                case Tool::SinglePoint:
                case Tool::DoublePoint:
                case Tool::LinePoint:
                case Tool::AbstractSpline:
                case Tool::Cut:
                case Tool::Midpoint:            // Same as Tool::AlongLine, but tool will never have such type
                case Tool::ArcIntersectAxis:    // Same as Tool::CurveIntersectAxis, but tool will never have such type
                case Tool::LAST_ONE_DO_NOT_USE:
                    Q_UNREACHABLE(); //-V501
                    break;

                case Tool::BasePoint:
                    iconFileName = ":/toolicon/32x32/point_basepoint_icon.png";
                    objName = tr("%1 - Base point").arg(getPointName(toolId));
                    break;

                case Tool::EndLine:
                    iconFileName = ":/toolicon/32x32/segment.png";
                    objName = tr("%1 - Point Length and Angle").arg(getPointName(toolId));
                    break;

                case Tool::Line:
                    iconFileName = ":/toolicon/32x32/line.png";
                    objName = tr("Line %1_%2")
                             .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                             .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                    break;

                case Tool::AlongLine:
                    iconFileName = ":/toolicon/32x32/along_line.png";
                    objName = tr("%1 - Point On Line").arg(getPointName(toolId));
                    break;

                case Tool::ShoulderPoint:
                    iconFileName = ":/toolicon/32x32/shoulder.png";
                    objName = tr("%1 - Point Length to Line").arg(getPointName(toolId));
                    break;

                case Tool::Normal:
                    iconFileName = ":/toolicon/32x32/normal.png";
                    objName = tr("%1 - Point On Perpendicular").arg(getPointName(toolId));
                    break;

                case Tool::Bisector:
                    iconFileName = ":/toolicon/32x32/bisector.png";
                    objName = tr("%1 - Point On Bisector").arg(getPointName(toolId));
                    break;

                case Tool::LineIntersect:
                    iconFileName = ":/toolicon/32x32/intersect.png";
                    objName = tr("%1 - Point Intersect Lines").arg(getPointName(toolId));
                    break;

               case Tool::Spline:
                {
                    const QSharedPointer<VSpline> spl = m_data->GeometricObject<VSpline>(toolId);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/spline.png";
                    objName = tr("%1 - Curve Interactive").arg(spl->NameForHistory(tr("Spl_")));
                    break;
                }

                case Tool::CubicBezier:
                {
                    const QSharedPointer<VCubicBezier> spl = m_data->GeometricObject<VCubicBezier>(toolId);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/cubic_bezier.png";
                    objName = tr("%1 - Curve Fixed").arg(spl->NameForHistory(tr("Spl_")));
                    break;
                }

                case Tool::Arc:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(toolId);
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc.png";
                    objName = tr("%1 - Arc Radius & Angles").arg(arc->NameForHistory(tr("Arc_")));
                    break;
                }

                case Tool::ArcWithLength:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(toolId);
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc_with_length.png";
                    objName = tr("%1 - Arc Radius & Length").arg(arc->NameForHistory(tr("Arc_")));
                    break;
                }

                case Tool::SplinePath:
                {
                    const QSharedPointer<VSplinePath> splPath = m_data->GeometricObject<VSplinePath>(toolId);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/splinePath.png";
                    objName = tr("%1 - Spline Interactive").arg(splPath->NameForHistory(tr("SplPath_")));
                    break;
                }

                case Tool::CubicBezierPath:
                {
                   const QSharedPointer<VCubicBezierPath> splPath = m_data->GeometricObject<VCubicBezierPath>(toolId);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/cubic_bezier_path.png";
                    objName = tr("%1 - Spline Fixed").arg(splPath->NameForHistory(tr("SplPath_")));
                    break;
                }

                case Tool::PointOfContact:
                    iconFileName = ":/toolicon/32x32/point_of_contact.png";
                    objName = tr("%1 - Point Intersect Arc & Line").arg(getPointName(toolId));
                    break;

                case Tool::Height:
                    iconFileName = ":/toolicon/32x32/height.png";
                    objName = tr("%1 - Point Intersect Line & Perpendicular").arg(getPointName(toolId));
                    break;

                case Tool::Triangle:
                    iconFileName = ":/toolicon/32x32/triangle.png";
                    objName = tr("%1 - Point Intersect Axis & Triangle").arg(getPointName(toolId));
                    break;

                case Tool::PointOfIntersection:
                    iconFileName = ":/toolicon/32x32/point_intersectxy_icon.png";
                    objName = tr("%1 - Point Intersect XY").arg(getPointName(toolId));
                    break;

                case Tool::CutArc:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(attrUInt(domElement, AttrArc));
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc_cut.png";
                    objName = tr("%1 - Point On Arc").arg(getPointName(toolId));
                    break;
                }

                case Tool::CutSpline:
                {
                    const quint32 splineId = attrUInt(domElement, VToolCutSpline::AttrSpline);
                    const QSharedPointer<VAbstractCubicBezier> spl =
                    m_data->GeometricObject<VAbstractCubicBezier>(splineId);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/spline_cut_point.png";
                    objName = tr("%1 - Point On Curve").arg(getPointName(toolId));
                    break;
                }

                case Tool::CutSplinePath:
                {
                    const quint32 splinePathId = attrUInt(domElement, VToolCutSplinePath::AttrSplinePath);
                    const QSharedPointer<VAbstractCubicBezierPath> splPath =
                    m_data->GeometricObject<VAbstractCubicBezierPath>(splinePathId);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/splinePath_cut_point.png";
                    objName = tr("%1 - Point On Spline").arg(getPointName(toolId));
                    break;
                }

                case Tool::LineIntersectAxis:
                    iconFileName = ":/toolicon/32x32/line_intersect_axis.png";
                    objName = tr("%1 - Point Intersect Line & Axis").arg(getPointName(toolId));
                    break;

                case Tool::CurveIntersectAxis:
                    iconFileName = ":/toolicon/32x32/arc_intersect_axis.png";
                    objName = tr("%1 - Point Intersect Curve & Axis").arg(getPointName(toolId));
                  break;

                case Tool::PointOfIntersectionArcs:
                    iconFileName = ":/toolicon/32x32/point_of_intersection_arcs.png";
                    objName = tr("%1 - Point Intersect Arcs").arg(getPointName(toolId));
                    break;

                case Tool::PointOfIntersectionCircles:
                    iconFileName = ":/toolicon/32x32/point_of_intersection_circles.png";
                    objName = tr("%1 - Point Intersect Circles").arg(getPointName(toolId));
                    break;

                case Tool::PointOfIntersectionCurves:
                    iconFileName = ":/toolicon/32x32/intersection_curves.png";
                    objName = tr("%1 - Point Intersect Curves").arg(getPointName(toolId));
                    break;

                case Tool::PointFromCircleAndTangent:
                    iconFileName = ":/toolicon/32x32/point_from_circle_and_tangent.png";
                    objName = tr("%1 - Point Intersect Circle & Tangent").arg(getPointName(toolId));
                    break;

                case Tool::PointFromArcAndTangent:
                    iconFileName = ":/toolicon/32x32/point_from_arc_and_tangent.png";
                    objName = tr("%1 - Point Intersect Arc & Tangent").arg(getPointName(toolId));
                    break;

                case Tool::TrueDarts:
                    iconFileName = ":/toolicon/32x32/true_darts.png";
                    objName = tr("%1 - True Dart %2_%3_%4")
                                 .arg(getObjName(objId))
                                 .arg(getPointName(attrUInt(domElement, AttrDartP1)))
                                 .arg(getPointName(attrUInt(domElement, AttrDartP2)))
                                 .arg(getPointName(attrUInt(domElement, AttrDartP3)));
                    break;

                case Tool::EllipticalArc:
                {
                    const QSharedPointer<VEllipticalArc> elArc = m_data->GeometricObject<VEllipticalArc>(toolId);
                    SCASSERT(!elArc.isNull())
                    iconFileName = ":/toolicon/32x32/el_arc.png";
                    objName = tr("%1 - Arc Elliptical").arg(elArc->NameForHistory(tr("ElArc_")));
                    break;
                }

                case Tool::Rotation:
                {
                    iconFileName = ":/toolicon/32x32/rotation.png";
                    objName = tr("%1 - Rotation")
                                 .arg(getObjName(objId == NULL_ID ? toolId : objId));
                    break;
                }

                case Tool::Move:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(objId);
                    iconFileName = ":/toolicon/32x32/move.png";
                    objName = tr("%1 - Move")
                                 .arg(getObjName(objId == NULL_ID ? toolId : objId));
                    break;
                }

                case Tool::MirrorByLine:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(objId);
                    iconFileName = ":/toolicon/32x32/mirror_by_line.png";
                    objName = tr("%1 - Mirror by Line")
                                 .arg(getObjName(objId == NULL_ID ? toolId : objId));
                    break;
                }

                case Tool::MirrorByAxis:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(objId);
                    iconFileName = ":/toolicon/32x32/mirror_by_axis.png";
                    objName = tr("%1 - Mirror by Axis")
                                 .arg(getObjName(objId == NULL_ID ? toolId : objId));
                    break;
                }

                case Tool::Piece:
                case Tool::Union:
                case Tool::NodeArc:
                case Tool::NodeElArc:
                case Tool::NodePoint:
                case Tool::NodeSpline:
                case Tool::NodeSplinePath:
                case Tool::Group:
                case Tool::InternalPath:
                case Tool::AnchorPoint:
                case Tool::InsertNodes:
                    return;
            }
            QList<quint32>itemData;
            itemData.append(objId);
            itemData.append(toolId);

            QListWidgetItem *item = new QListWidgetItem(objName);
            item->setIcon(QIcon(iconFileName));
            item->setData(Qt::UserRole,  QVariant::fromValue(itemData));
            ui->groupItems_ListWidget->addItem(item);
            return;
        }

        catch (const VExceptionBadId &error)
        {
            QList<quint32>itemData;
            itemData.append(objId);
            itemData.append(toolId);

            qDebug() << error.ErrorMessage() << Q_FUNC_INFO;
            QListWidgetItem *item = new QListWidgetItem(objName);
            item->setIcon(QIcon(":/icons/win.icon.theme/16x16/status/dialog-warning.png"));
            item->setData(Qt::UserRole,  QVariant::fromValue(itemData));
            ui->groupItems_ListWidget->addItem(item);
            return;
        }
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPointName get the name of the point by tool id.
 * @param toolId Id of the tool the object belongs to.
 * @return name of point.
 */
QString GroupsWidget::getPointName(quint32 toolId)
{
    return m_data->GeometricObject<VPointF>(toolId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief attrUInt get unsigned integer attribute.
 * @param domElement Dom Element.
 * @param name  Name of Attribute.
 * @return unsigned interger.
 */
quint32 GroupsWidget::attrUInt(const QDomElement &domElement, const QString &name)
{
    return m_doc->GetParametrUInt(domElement, name, "0");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getObjName get the name of the graphics object.
 * @param toolId Id of the tool the object belongs to.
 * @return name of object.
 */
QString GroupsWidget::getObjName(quint32 toolId)
{
    try
    {
        const auto obj = m_data->GetGObject(toolId);
        return obj->name();
    }
    catch (const VExceptionBadId &error)
    {
        qCDebug(WidgetGroups, "Error! Couldn't get object name by id = %s. %s %s", qUtf8Printable(QString().setNum(toolId)),
                qUtf8Printable(error.ErrorMessage()),
                qUtf8Printable(error.DetailedInformation()));
        return QString("Unknown Object");// Return Unknown string
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ContextMenu show context menu for group objects.
 * @param pos Mouse position of right click.
 */
 void GroupsWidget::groupItemContextMenu(const QPoint &pos)
{
    const quint32 groupId = getGroupId();
    const int row = ui->groupItems_ListWidget->currentRow();
    if (ui->groupItems_ListWidget->count() == 0 || row == -1 || row >= ui->groupItems_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = ui->groupItems_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    QList<quint32> itemData = rowItem->data(Qt::UserRole).value<QList<quint32>>();
    const quint32 toolId = itemData.last();
    const quint32 objId =  itemData.first();

    QMenu menu;
    // Add Move Group Item menu
    QMap<quint32,QString> groupsNotContainingItem =  m_doc->getGroupsContainingItem(toolId, objId, false);
    QActionGroup *actionMoveGroupMenu= new QActionGroup(this);

    if(!groupsNotContainingItem.empty())
    {
        QMenu *menuMoveGroupItem = menu.addMenu(QIcon("://icon/32x32/list-move_32.png"), tr("Move Group Object"));
        QStringList list = QStringList(groupsNotContainingItem.values());
        list.sort(Qt::CaseInsensitive);

        for(int i=0; i<list.count(); ++i)
        {
            QAction *actionMoveGroupItem = menuMoveGroupItem->addAction(list[i]);
            actionMoveGroupMenu->addAction(actionMoveGroupItem);
            const quint32 groupId = groupsNotContainingItem.key(list[i]);
            actionMoveGroupItem->setData(groupId);
        }
    }
    // Add Remove Group Item menu
    QAction *actionRemove = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Remove Group Object"));

    QAction *selectedAction = menu.exec(ui->groupItems_ListWidget->viewport()->mapToGlobal(pos));
    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == actionRemove)
    {
        const bool locked = m_doc->getGroupLock(groupId);
        if (locked == false)
        {
            qCDebug(WidgetGroups, "Remove Tool %s from Group %s.",
                    qUtf8Printable(QString().setNum(toolId)),
                    qUtf8Printable(QString().setNum(groupId)));

            QListWidgetItem *rowItem = ui->groupItems_ListWidget->item(row);
            SCASSERT(rowItem != nullptr);
            QList<quint32> itemData = rowItem->data(Qt::UserRole).value<QList<quint32>>();
            delete ui->groupItems_ListWidget->item(row);

            QDomElement item = m_doc->removeGroupItem(itemData.last(), itemData.first(), groupId);

            VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
            SCASSERT(scene != nullptr)
            scene->clearSelection();

            VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
            SCASSERT(window != nullptr)
            {
                RemoveGroupItem *command = new RemoveGroupItem(item, m_doc, groupId);
                connect(command, &RemoveGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
                qApp->getUndoStack()->push(command);
            }
        }
    }
    else if (selectedAction->actionGroup() == actionMoveGroupMenu)
    {
      const quint32 destinationGroupId = selectedAction->data().toUInt();
      const bool sourceLock = m_doc->getGroupLock(groupId);
      const bool destinationLock = m_doc->getGroupLock(destinationGroupId);
      if ((sourceLock == false) && (destinationLock == false))      //only move if both groups are unlocked
      {

          qCDebug(WidgetGroups, "Move Tool %s from Group %s to Group %s.",
                  qUtf8Printable(QString().setNum(toolId)),
                  qUtf8Printable(QString().setNum(groupId)),
                  qUtf8Printable(QString().setNum(destinationGroupId)));

        QListWidgetItem *rowItem = ui->groupItems_ListWidget->item(row);
        SCASSERT(rowItem != nullptr);
        QList<quint32> itemData = rowItem->data(Qt::UserRole).value<QList<quint32>>();
        delete ui->groupItems_ListWidget->item(row);

         QDomElement sourceItem = m_doc->removeGroupItem(itemData.last(), itemData.first(), groupId);
         QDomElement destItem = m_doc->addGroupItem(itemData.last(), itemData.first(), destinationGroupId);

         updateGroups();

         VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
         SCASSERT(scene != nullptr)
         scene->clearSelection();

         VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
         SCASSERT(window != nullptr)
         {
             MoveGroupItem *command = new MoveGroupItem(sourceItem, destItem, m_doc, groupId, destinationGroupId);
             connect(command, &MoveGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
             qApp->getUndoStack()->push(command);
         }
      }
    }
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief cellClicked Handle clicking of table cell.
 * @param row row of selected table widget item.
 * @param column column of selected table widget item.
 */
void GroupsWidget::cellClicked(int row, int column)
{
    if (column == 0)
    {
        groupVisibilityChanged(row, 0);
    }
    else if (column == 1)
    {
        groupLockChanged(row, 1);
    }
    else if ((column == 2) || (column == 3) || (column == 4))
    {
        QTableWidgetItem *item = ui->groups_TableWidget->item(row, 0);
        const quint32 groupId = item->data(Qt::UserRole).toUInt();
        const bool locked = m_doc->getGroupLock(groupId);
        if (locked == true)
        {
            QApplication::beep();
        }
    }

    fillGroupItemList();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief cellDoubleClicked Handle double clicking of table cell.
 * @param row row of selected table widget item.
 * @param column column of selected table widget item.
 */
void GroupsWidget::cellDoubleClicked(int row, int column)
{
    QTableWidgetItem *item = ui->groups_TableWidget->item(row, 1);
    const quint32 groupId = item->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(groupId);
    if (locked == false)
    {
        if (column == 3)
        {
            editGroup();
        }
    }
    else
    {
        QApplication::beep();
    }
    return;
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handle double clicking of list item.
 * @param item selected list widget item.
 */
void GroupsWidget::itemDoubleClicked(QListWidgetItem *item)
{
    GOType objType;
    const auto objects = m_data->DataGObjects();
    QList<quint32> itemData = item->data(Qt::UserRole).value<QList<quint32>>();
    quint32 toolId = itemData.last();
    quint32 objId =  itemData.first();
    if (objId != NULL_ID)
    {
        toolId = objId;
    }

    if (objects->contains(toolId))
    {
        objType = m_data->GetGObject(toolId)->getType();
    }
    else
    {
        //Some tools are not GObjects in the container. Need to look in the Dom Doc instead
        const QDomElement domElement = m_doc->elementById(toolId);
        if (domElement.tagName() == VAbstractPattern::TagLine)
        {
            //Use the line's FirstPoint as toolId and reset toolType
            toolId = attrUInt(domElement, AttrFirstPoint);
        }
        else
        {
            return;
        }
        objType = m_data->GetGObject(toolId)->getType();
    }

    switch(objType)
    {
        case GOType::Point:
        {
            QSharedPointer<VPointF> point = m_data->GeometricObject<VPointF>(toolId);
            zoomToObject(point);
            return;
        }

        case GOType::Arc:
        case GOType::EllipticalArc:
        {
            const QSharedPointer<VAbstractArc> curve = m_data->GeometricObject<VAbstractArc>(toolId);
            QSharedPointer<VPointF>point(new VPointF(curve->GetCenter()));

            zoomToObject(point);
            return;
        }

        case GOType::Spline:
        case GOType::SplinePath:
        case GOType::CubicBezier:
        case GOType::CubicBezierPath:
        {
            const QSharedPointer<VAbstractCurve> curve = m_data->GeometricObject<VAbstractCurve>(toolId);
            QSharedPointer<VPointF>point(new VPointF(curve->getFirstPoint()));
            zoomToObject(point);
            return;
        }

        case GOType::Unknown:
        case GOType::Curve:
        case GOType::Path:
        case GOType::AllCurves:
            default:
            break;
        }

    return;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief zoomToObject zoom to selected (point) object.
 * @param point If selected list item is a point use the item's toolId. if the list item is a curve or line then
                use the tool's first point.
 */
void GroupsWidget::zoomToObject(QSharedPointer<VPointF> point)
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
    SCASSERT(scene != nullptr)
    scene->clearSelection();

    VMainGraphicsView *view = qobject_cast<VMainGraphicsView *>(scene->views().first());
    view->zoomByScale(1.3);
    view->centerOn(point->toQPointF());

    int row = ui->groups_TableWidget->currentRow();
    setGroupVisibility(ui->groups_TableWidget->item(row, 0), getGroupId(), true);

    // show point name if it's hidden
    quint32 toolId = point->getIdTool();
    const quint32 objectId = point->getIdObject();
    if (objectId != NULL_ID)
    {
        toolId = objectId;
    }
    if (toolId != NULL_ID)
    {
        if (VAbstractTool *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(toolId)))
        {
            tool->setPointNameVisiblity(toolId, true);
        }
    }

    // show any hiden groups containing object
    showGroups(m_doc->getGroupsContainingItem(toolId, objectId, true));

    return;
}

void GroupsWidget::setGroupVisibility(QTableWidgetItem *item, const quint32 &groupId, const bool &visible)
{
    m_doc->setGroupVisibility(groupId, visible);
    if (visible)
    {
        item->setIcon(QIcon("://icon/32x32/visible_on.png"));
    }
    else
    {
        item->setIcon(QIcon("://icon/32x32/visible_off.png"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief splitterMoved Save splitter state whenever it's moved.
 * @param pos
 * @param index
 */
void GroupsWidget::splitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)

    QSettings settings;
    settings.setValue("splitterSizes", ui->groups_Splitter->saveState());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief headerClicked Sort state whenever header section clicked.
 * @param index
 */
void GroupsWidget::headerClicked(int index)
{
    QSettings settings;
    settings.setValue("groupSort", index);
}

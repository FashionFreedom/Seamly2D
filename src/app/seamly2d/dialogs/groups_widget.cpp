//-----------------------------------------------------------------------------
//  @file   groups_widget.h
//  @author Douglas S Caskey
//  @date   11 Jun, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
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
//  @file   vwidgetgroups.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   Jun 4, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
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

#include "groups_widget.h"
#include "ui_groups_widget.h"
#include "../core/application_2d.h"
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
    , m_pattern_has_groups(false)

{
    ui->setupUi(this);
    QSettings settings;
    ui->groups_splitter->restoreState(settings.value("splitterSizes").toByteArray());

    fillTable(m_doc->getGroups());
    ui->groups_table_widget->sortItems(settings.value("groupSort", 4).toInt(), Qt::AscendingOrder);

    connect(m_doc, &VAbstractPattern::patternHasGroups, this, &GroupsWidget::draftBlockHasGroups);

    connect(ui->show_all__toolbutton,    &QToolButton::clicked, this,  &GroupsWidget::showAllGroups);
    connect(ui->hide_all__toolbutton,    &QToolButton::clicked, this,  &GroupsWidget::hideAllGroups);
    connect(ui->lock_all__toolbutton,    &QToolButton::clicked, this,  &GroupsWidget::lockAllGroups);
    connect(ui->unlock_all__toolbutton,  &QToolButton::clicked, this,  &GroupsWidget::unlockAllGroups);
    connect(ui->add_group_toolbutton,    &QToolButton::clicked, this,  &GroupsWidget::addGroupToList);
    connect(ui->delete_group_toolbutton, &QToolButton::clicked, this,  &GroupsWidget::deleteGroupFromList);
    connect(ui->edit_group_toolbutton,   &QToolButton::clicked, this,  &GroupsWidget::editGroup);

    connect(ui->groups_table_widget, &QTableWidget::cellChanged,        this, &GroupsWidget::renameGroup);
    connect(ui->groups_table_widget, &QTableWidget::cellClicked,        this, &GroupsWidget::cellClicked);
    connect(ui->groups_table_widget, &QTableWidget::cellDoubleClicked,  this, &GroupsWidget::cellDoubleClicked);
    connect(ui->groups_table_widget, &QTableWidget::currentCellChanged, this, &GroupsWidget::fillGroupItemList);

    ui->groups_table_widget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->groups_table_widget, &QTableWidget::customContextMenuRequested, this, &GroupsWidget::groupContextMenu);

    ui->groupItems_list_widget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->groupItems_list_widget, &QListWidget::customContextMenuRequested, this, &GroupsWidget::groupItemContextMenu);
    connect(ui->groupItems_list_widget, &QListWidget::itemDoubleClicked,          this, &GroupsWidget::itemDoubleClicked);

    connect(ui->groups_table_widget->horizontalHeader(), &QHeaderView::sectionClicked, this, &GroupsWidget::headerClicked);
    connect(ui->groups_splitter, &QSplitter::splitterMoved, this, &GroupsWidget::splitterMoved);
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

    QTableWidgetItem *item = ui->groups_table_widget->item(row, column);
    const quint32 group_id = item->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(group_id);
    if (locked == false)
    {
        const bool visible = !m_doc->getGroupVisibility(group_id);
        setGroupVisibility(item, group_id, visible);
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

    QTableWidgetItem *item = ui->groups_table_widget->item(row, column);
    if (!item) return;
    const quint32 group_id = item->data(Qt::UserRole).toUInt();
    const bool locked = !m_doc->getGroupLock(group_id);
    m_doc->setGroupLock(group_id, locked);
    if (locked)
    {
        item->setIcon(QIcon("://icon/32x32/lock_on.png"));
        item = ui->groups_table_widget->item(row, 4);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
    else
    {
        item->setIcon(QIcon("://icon/32x32/lock_off.png"));
        item = ui->groups_table_widget->item(row, 4);
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

    const quint32 group_id = ui->groups_table_widget->item(row, 0)->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(group_id);
    if (locked == false)
    {
        QTableWidgetItem *item = ui->groups_table_widget->item(row, column);
        const QString new_group_name = item->text();
        const QString old_group_name = m_doc->getGroupName(group_id);

        if (new_group_name != old_group_name && m_doc->groupNameExists(new_group_name))
        {
            QMessageBox message_box;
            message_box.setWindowTitle(tr("Name Exists"));
            message_box.setIcon(QMessageBox::Warning);
            message_box.setStandardButtons(QMessageBox::Ok);
            message_box.setText(tr("The action can't be completed because the group name already exists."));
            message_box.exec();

            ui->groups_table_widget->blockSignals(true);
            item->setText(old_group_name);
            ui->groups_table_widget->blockSignals(false);
            return;
        }

        m_doc->setGroupName(group_id, new_group_name);
        updateGroups();
    }
}

void GroupsWidget::showGroups(QMap<quint32,QString> groups)
{
    QMap<quint32,QString>::const_iterator i;
    for (i = groups.constBegin(); i != groups.constEnd(); ++i)
    {
        for (int j = 0; j < ui->groups_table_widget->rowCount(); ++j)
        {
            QTableWidgetItem *item = ui->groups_table_widget->item(j, 0);
            if (item)
            {
                quint32 group_id = item->data(Qt::UserRole).toUInt();
                if (group_id == i.key())
                {
                    setGroupVisibility(item, group_id, true);
                }
            }
        }
    }
    updateGroups();
}

void GroupsWidget::showAllGroups()
{
    ui->groups_table_widget->setSortingEnabled(false);
    ui->groups_table_widget->blockSignals(true);
    qCDebug(WidgetGroups, "Show All Groups");
    quint32 group_id;
    bool locked;
    for (int i = 0; i < ui->groups_table_widget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->groups_table_widget->item(i, 0);
        if (!item)
        {
            return;
        }
        group_id = item->data(Qt::UserRole).toUInt();
        locked = m_doc->getGroupLock(group_id);
        if (item && locked == false)
        {
            setGroupVisibility(item, group_id, true);
        }
    }
    ui->groups_table_widget->blockSignals(false);
    ui->groups_table_widget->setSortingEnabled(true);
}

void GroupsWidget::hideAllGroups()
{
    ui->groups_table_widget->setSortingEnabled(false);
    ui->groups_table_widget->blockSignals(true);
    qCDebug(WidgetGroups, "Hide All Groups");
    quint32 group_id;
    bool locked;
    for (int row = 0; row < ui->groups_table_widget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_table_widget->item(row, 0);
        if (!item)
        {
            return;
        }
        group_id = item->data(Qt::UserRole).toUInt();
        locked = m_doc->getGroupLock(group_id);
        if (item && locked == false)
        {
            setGroupVisibility(item, group_id, false);
        }
    }
    ui->groups_table_widget->blockSignals(false);
    ui->groups_table_widget->setSortingEnabled(true);
}

void GroupsWidget::lockAllGroups()
{
    qCDebug(WidgetGroups, "Lock All Groups");
    for (int row = 0; row < ui->groups_table_widget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_table_widget->item(row, 1);
        if (!item)
        {
            return;
        }
        const quint32 group_id = item->data(Qt::UserRole).toUInt();
        m_doc->setGroupLock(group_id, true);
        item->setIcon(QIcon("://icon/32x32/lock_on.png"));
        item = ui->groups_table_widget->item(row, 4);
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
    }
}

void GroupsWidget::unlockAllGroups()
{
    qCDebug(WidgetGroups, "Unlock All Groups");
    for (int row = 0; row < ui->groups_table_widget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->groups_table_widget->item(row, 1);
        if (!item)
        {
            return;
        }
        const quint32 group_id = item->data(Qt::UserRole).toUInt();
        m_doc->setGroupLock(group_id, false);
        item->setIcon(QIcon("://icon/32x32/lock_off.png"));
        item = ui->groups_table_widget->item(row, 4);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void GroupsWidget::addGroupToList()
{
    QScopedPointer<EditGroupDialog> dialog(new EditGroupDialog(new VContainer(qApp->translateVariables(),
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
    const quint32 group_id = getGroupId();
    qCDebug(WidgetGroups, "Remove Group %d from List", group_id);
    const bool locked = m_doc->getGroupLock(group_id);
    QTableWidgetItem *item = ui->groups_table_widget->currentItem();
    if (!item)
    {
        return;
    }

    if (item && (locked == false))
    {
        DelGroup *command = new DelGroup(m_doc, group_id);
        connect(command, &DelGroup::updateGroups, this, &GroupsWidget::updateGroups);
        qApp->getUndoStack()->push(command);
    }
}

void GroupsWidget::editGroup()
{
    ui->groups_table_widget->blockSignals(true);
    qCDebug(WidgetGroups, "Edit Group List");
    const int row = ui->groups_table_widget->currentRow();
    if (ui->groups_table_widget->rowCount() == 0 || row == -1 || row >= ui->groups_table_widget->rowCount())
    {
        ui->groups_table_widget->blockSignals(false);
        return;
    }

    const quint32 group_id = ui->groups_table_widget->item(row, 0)->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(group_id);
    QString oldGroupName = m_doc->getGroupName(group_id);
    if (locked == false)
    {
        qCDebug(WidgetGroups, "Row = %d", row);

        QScopedPointer<EditGroupDialog> dialog(new EditGroupDialog(new VContainer(qApp->translateVariables(),
                                                                   qApp->patternUnitP()), NULL_ID, this));
        dialog->setName(oldGroupName);
        dialog->setColor(m_doc->getGroupColor(group_id));
        dialog->setLineType(m_doc->getGroupLineType(group_id));
        dialog->setLineWeight(m_doc->getGroupLineWeight(group_id));
        dialog->setWindowTitle(tr("Edit Group"));

        QString groupName;
        while (1)
        {
            const bool result = dialog->exec();
            groupName = dialog->getName();
            if (result == false || groupName.isEmpty())
            {
                ui->groups_table_widget->blockSignals(false);
                return;
            }
            bool exists = m_doc->groupNameExists(groupName);
            if (exists == false || groupName == oldGroupName)
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
                    break;    // Repeat Edit Group Dialog
                case QMessageBox::Cancel:
                    ui->groups_table_widget->blockSignals(false);
                    return;   // Exit Edit Group Dialog
                default:
                    break;    // should never be reached
            }
        }

        const QString groupColor = dialog->getColor();
        const QString groupLineType = dialog->getLineType();
        const QString groupLineWeight = dialog->getLineWeight();

        // Add color item
        QTableWidgetItem *item = ui->groups_table_widget->item(row, 3);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setSizeHint(QSize(20, 20));
        QPixmap pixmap(20, 20);
        pixmap.fill(QColor(groupColor));
        item->setIcon(QIcon(pixmap));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Group color"));
        // Add group name item
        item = ui->groups_table_widget->item(row, 4);
        item->setText(groupName);
        item->setFlags(item->flags() | Qt::ItemIsEditable);

        m_doc->setGroupName(group_id, groupName);
        m_doc->setGroupColor(group_id, groupColor);
        m_doc->setGroupLineType(group_id, groupLineType);
        m_doc->setGroupLineWeight(group_id, groupLineWeight);

        updateGroups();
    }
    else
    {
        QApplication::beep();
    }
    ui->groups_table_widget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::groupContextMenu(const QPoint &pos)
{
    //ui->groups_table_widget->setSortingEnabled(false);
    //ui->groups_table_widget->blockSignals(true);

    QTableWidgetItem *item = ui->groups_table_widget->itemAt(pos);
    if (!item)
    {
        return;
    }

    const int row = item->row();
    item = ui->groups_table_widget->item(row, 0);
    const quint32 group_id = item->data(Qt::UserRole).toUInt();

    const bool locked = m_doc->getGroupLock(group_id);
    if (locked)
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionEdit = menu->addAction(QIcon("://icon/32x32/edit.png"), tr("Edit"));
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));
    QAction *selectedAction = menu->exec(ui->groups_table_widget->viewport()->mapToGlobal(pos));
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
        DelGroup *command = new DelGroup(m_doc, group_id);
        connect(command, &DelGroup::updateGroups, this, &GroupsWidget::updateGroups);
        qApp->getUndoStack()->push(command);
    }

    //ui->groups_table_widget->setSortingEnabled(true);
    //ui->groups_table_widget->blockSignals(false);

}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::updateGroups()
{
    int row = ui->groups_table_widget->currentRow();
    fillTable(m_doc->getGroups());
    if (ui->groups_table_widget->rowCount() != 0 || row != -1 || row <= ui->groups_table_widget->rowCount())
    {
        ui->groups_table_widget->selectRow(row);
    }
    fillGroupItemList();
}

//---------------------------------------------------------------------------------------------------------------------
void GroupsWidget::fillTable(const QMap<quint32, GroupAttributes> &groups)
{
    ui->groups_table_widget->blockSignals(true);
    ui->groups_table_widget->clear();
    ui->groups_table_widget->setColumnCount(5);
    ui->groups_table_widget->setRowCount(groups.size());
    ui->groups_table_widget->setSortingEnabled(false);

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
        ui->groups_table_widget->setItem(currentRow, 0, item);

        // Add locked item
        item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(20, 20));
        item->setIcon(data.locked ? QIcon("://icon/32x32/lock_on.png") : QIcon("://icon/32x32/lock_off.png"));
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Show which groups in the list are locked"));
        ui->groups_table_widget->setItem(currentRow, 1, item);

        // Add contain objects Item
        item = new GroupTableWidgetItem(m_doc);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setIcon(!m_doc->isGroupEmpty(i.key()) ? QIcon("://icon/32x32/history.png") : QIcon());
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Show which groups contain objects"));
        ui->groups_table_widget->setItem(currentRow, 2, item);

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
        ui->groups_table_widget->setItem(currentRow, 3, item);

        // Add group name item
        QTableWidgetItem *nameItem = new QTableWidgetItem(data.name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        nameItem->setToolTip(tr("Group name"));
        ui->groups_table_widget->setItem(currentRow, 4, nameItem);
        if (data.locked)
        {
            nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        }
        ++i;
    }

    ui->groups_table_widget->setHorizontalHeaderItem(0, new QTableWidgetItem(makeHeaderName(tr("Visible"))));
    ui->groups_table_widget->horizontalHeaderItem(0)->setToolTip(tr("Group is visible"));
    ui->groups_table_widget->setHorizontalHeaderItem(1, new QTableWidgetItem(makeHeaderName(tr("Locked"))));
    ui->groups_table_widget->horizontalHeaderItem(1)->setToolTip(tr("Group is locked"));
    ui->groups_table_widget->setHorizontalHeaderItem(2, new QTableWidgetItem(makeHeaderName(tr("Objects"))));
    ui->groups_table_widget->horizontalHeaderItem(2)->setToolTip(tr("Group has objects"));
    ui->groups_table_widget->setHorizontalHeaderItem(3, new QTableWidgetItem(makeHeaderName(tr("Color"))));
    ui->groups_table_widget->horizontalHeaderItem(3)->setToolTip(tr("Group color"));
    ui->groups_table_widget->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Name")));
    ui->groups_table_widget->horizontalHeaderItem(4)->setToolTip(tr("Group name"));
    ui->groups_table_widget->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->groups_table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->groups_table_widget->resizeColumnsToContents();
    ui->groups_table_widget->resizeRowsToContents();
    ui->groups_table_widget->setSortingEnabled(true);
    ui->groups_table_widget->blockSignals(false);
}

void GroupsWidget::draftBlockHasGroups(bool value)
{
    m_pattern_has_groups = value;

    ui->show_all__toolbutton->setEnabled(value);
    ui->hide_all__toolbutton->setEnabled(value);
    ui->lock_all__toolbutton->setEnabled(value);
    ui->unlock_all__toolbutton->setEnabled(value);
    ui->delete_group_toolbutton->setEnabled(value);
    ui->edit_group_toolbutton->setEnabled(value);
    qCDebug(WidgetGroups, "Draft Block Has Groups = %d", value);
}

void GroupsWidget::setAddGroupEnabled(bool value)
{
    ui->add_group_toolbutton->setEnabled(value);
}

void GroupsWidget::clear()
{
    ui->groups_table_widget->setRowCount(0);
    ui->groups_table_widget->clearContents();
    ui->groupItems_list_widget->clear();
}

quint32 GroupsWidget::getGroupId()
{
    QTableWidgetItem *item = ui->groups_table_widget->currentItem();
    if (!item)
    {
        return 0;
    }
    int row = ui->groups_table_widget->row(item);
    qCDebug(WidgetGroups, "Row = %d\n", row);
    const quint32 group_id = ui->groups_table_widget->item(row, 0)->data(Qt::UserRole).toUInt();
    return group_id;
}

QString GroupsWidget::getCurrentGroupName()
{
    QTableWidgetItem *item = ui->groups_table_widget->currentItem();
    if (!item)
    {
        return QString();
    }
    int row = ui->groups_table_widget->row(item);
    qCDebug(WidgetGroups, "Row = %d\n", row);
    const QString groupName = ui->groups_table_widget->item(row, 4)->text();
    return groupName;
}

void GroupsWidget::fillGroupItemList()
{
    ui->groupItems_list_widget->blockSignals(true);
    ui->groupItems_list_widget->clear();
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
           quint32 tool_id = i.value();
           quint32 obj_id = i.key();
           Tool tool_type = history.value(tool_id);
           addGroupItem(tool_id, obj_id, tool_type);
           ++i;
        }
    }
    ui->groupItems_list_widget->sortItems(Qt::AscendingOrder);
    ui->groupItems_list_widget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief addGroupItem Add group item with description in listwidget.
 * @param toolID ToolID of item to add to list.
 * @param obj_id ObjectID of item to add to list.
 * @param tool_type Tooltype of item to add to list.
 */
void GroupsWidget::addGroupItem(const quint32 &tool_id, const quint32 &obj_id, const Tool &tool_type)
{
    // This check helps to find missing tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used in history.");

    QString iconFileName = "";
    QString objName = tr("Unknown Object");
    const QDomElement dom_element = m_doc->elementById(tool_id);
    if (dom_element.isElement() == false)
    {
        qDebug() << "Can't find element by id" << Q_FUNC_INFO;
        return;
    }
        try
        {
            switch (tool_type)
            {
                case Tool::Arrow:
                case Tool::SinglePoint:
                case Tool::DoublePoint:
                case Tool::LinePoint:
                case Tool::AbstractSpline:
                case Tool::Cut:
                case Tool::Midpoint:            // Same as Tool::AlongLine, but tool will never have such type
                case Tool::ArcIntersectAxis:    // Same as Tool::CurveIntersectAxis, but tool will never have such type
                case Tool::BackgroundImage:
                case Tool::LAST_ONE_DO_NOT_USE:
                    Q_UNREACHABLE(); //-V501
                    break;

                case Tool::BasePoint:
                    iconFileName = ":/toolicon/32x32/point_basepoint_icon.png";
                    objName = tr("%1 - Base point").arg(getPointName(tool_id));
                    break;

                case Tool::EndLine:
                    iconFileName = ":/toolicon/32x32/segment.png";
                    objName = tr("%1 - Point Length and Angle").arg(getPointName(tool_id));
                    break;

                case Tool::Line:
                    iconFileName = ":/toolicon/32x32/line.png";
                    objName = tr("Line %1_%2")
                             .arg(getPointName(attrUInt(dom_element, AttrFirstPoint)))
                             .arg(getPointName(attrUInt(dom_element, AttrSecondPoint)));
                    break;

                case Tool::AlongLine:
                    iconFileName = ":/toolicon/32x32/along_line.png";
                    objName = tr("%1 - Point On Line").arg(getPointName(tool_id));
                    break;

                case Tool::ShoulderPoint:
                    iconFileName = ":/toolicon/32x32/shoulder.png";
                    objName = tr("%1 - Point Length to Line").arg(getPointName(tool_id));
                    break;

                case Tool::Normal:
                    iconFileName = ":/toolicon/32x32/normal.png";
                    objName = tr("%1 - Point On Perpendicular").arg(getPointName(tool_id));
                    break;

                case Tool::Bisector:
                    iconFileName = ":/toolicon/32x32/bisector.png";
                    objName = tr("%1 - Point On Bisector").arg(getPointName(tool_id));
                    break;

                case Tool::LineIntersect:
                    iconFileName = ":/toolicon/32x32/intersect.png";
                    objName = tr("%1 - Point Intersect Lines").arg(getPointName(tool_id));
                    break;

               case Tool::Spline:
                {
                    const QSharedPointer<VSpline> spl = m_data->GeometricObject<VSpline>(tool_id);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/spline.png";
                    objName = tr("%1 - Curve Interactive").arg(spl->NameForHistory(tr("Spl_")));
                    break;
                }

                case Tool::CubicBezier:
                {
                    const QSharedPointer<VCubicBezier> spl = m_data->GeometricObject<VCubicBezier>(tool_id);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/cubic_bezier.png";
                    objName = tr("%1 - Curve Fixed").arg(spl->NameForHistory(tr("Spl_")));
                    break;
                }

                case Tool::Arc:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(tool_id);
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc.png";
                    objName = tr("%1 - Arc Radius & Angles").arg(arc->NameForHistory(tr("Arc_")));
                    break;
                }

                case Tool::ArcWithLength:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(tool_id);
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc_with_length.png";
                    objName = tr("%1 - Arc Radius & Length").arg(arc->NameForHistory(tr("Arc_")));
                    break;
                }

                case Tool::SplinePath:
                {
                    const QSharedPointer<VSplinePath> splPath = m_data->GeometricObject<VSplinePath>(tool_id);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/splinePath.png";
                    objName = tr("%1 - Spline Interactive").arg(splPath->NameForHistory(tr("SplPath_")));
                    break;
                }

                case Tool::CubicBezierPath:
                {
                   const QSharedPointer<VCubicBezierPath> splPath = m_data->GeometricObject<VCubicBezierPath>(tool_id);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/cubic_bezier_path.png";
                    objName = tr("%1 - Spline Fixed").arg(splPath->NameForHistory(tr("SplPath_")));
                    break;
                }

                case Tool::PointOfContact:
                    iconFileName = ":/toolicon/32x32/point_intersect_arc_line.png";
                    objName = tr("%1 - Point Intersect Arc & Line").arg(getPointName(tool_id));
                    break;

                case Tool::Height:
                    iconFileName = ":/toolicon/32x32/height.png";
                    objName = tr("%1 - Point Intersect Line & Perpendicular").arg(getPointName(tool_id));
                    break;

                case Tool::Triangle:
                    iconFileName = ":/toolicon/32x32/triangle.png";
                    objName = tr("%1 - Point Intersect Axis & Triangle").arg(getPointName(tool_id));
                    break;

                case Tool::PointOfIntersection:
                    iconFileName = ":/toolicon/32x32/point_intersectxy_icon.png";
                    objName = tr("%1 - Point Intersect XY").arg(getPointName(tool_id));
                    break;

                case Tool::CutArc:
                {
                    const QSharedPointer<VArc> arc = m_data->GeometricObject<VArc>(attrUInt(dom_element, AttrArc));
                    SCASSERT(!arc.isNull())
                    iconFileName = ":/toolicon/32x32/arc_cut.png";
                    objName = tr("%1 - Point On Arc").arg(getPointName(tool_id));
                    break;
                }

                case Tool::CutSpline:
                {
                    const quint32 splineId = attrUInt(dom_element, VToolCutSpline::AttrSpline);
                    const QSharedPointer<VAbstractCubicBezier> spl =
                    m_data->GeometricObject<VAbstractCubicBezier>(splineId);
                    SCASSERT(!spl.isNull())
                    iconFileName = ":/toolicon/32x32/spline_cut_point.png";
                    objName = tr("%1 - Point On Curve").arg(getPointName(tool_id));
                    break;
                }

                case Tool::CutSplinePath:
                {
                    const quint32 splinePathId = attrUInt(dom_element, VToolCutSplinePath::AttrSplinePath);
                    const QSharedPointer<VAbstractCubicBezierPath> splPath =
                    m_data->GeometricObject<VAbstractCubicBezierPath>(splinePathId);
                    SCASSERT(!splPath.isNull())
                    iconFileName = ":/toolicon/32x32/splinePath_cut_point.png";
                    objName = tr("%1 - Point On Spline").arg(getPointName(tool_id));
                    break;
                }

                case Tool::LineIntersectAxis:
                    iconFileName = ":/toolicon/32x32/line_intersect_axis.png";
                    objName = tr("%1 - Point Intersect Line & Axis").arg(getPointName(tool_id));
                    break;

                case Tool::CurveIntersectAxis:
                    iconFileName = ":/toolicon/32x32/arc_intersect_axis.png";
                    objName = tr("%1 - Point Intersect Curve & Axis").arg(getPointName(tool_id));
                  break;

                case Tool::PointOfIntersectionArcs:
                    iconFileName = ":/toolicon/32x32/point_of_intersection_arcs.png";
                    objName = tr("%1 - Point Intersect Arcs").arg(getPointName(tool_id));
                    break;

                case Tool::PointOfIntersectionCircles:
                    iconFileName = ":/toolicon/32x32/point_of_intersection_circles.png";
                    objName = tr("%1 - Point Intersect Circles").arg(getPointName(tool_id));
                    break;

                case Tool::PointOfIntersectionCurves:
                    iconFileName = ":/toolicon/32x32/intersection_curves.png";
                    objName = tr("%1 - Point Intersect Curves").arg(getPointName(tool_id));
                    break;

                case Tool::PointFromCircleAndTangent:
                    iconFileName = ":/toolicon/32x32/point_from_circle_and_tangent.png";
                    objName = tr("%1 - Point Intersect Circle & Tangent").arg(getPointName(tool_id));
                    break;

                case Tool::PointFromArcAndTangent:
                    iconFileName = ":/toolicon/32x32/point_from_arc_and_tangent.png";
                    objName = tr("%1 - Point Intersect Arc & Tangent").arg(getPointName(tool_id));
                    break;

                case Tool::TrueDarts:
                    iconFileName = ":/toolicon/32x32/true_darts.png";
                    objName = tr("%1 - True Dart %2_%3_%4")
                                 .arg(getObjName(obj_id))
                                 .arg(getPointName(attrUInt(dom_element, AttrDartP1)))
                                 .arg(getPointName(attrUInt(dom_element, AttrDartP2)))
                                 .arg(getPointName(attrUInt(dom_element, AttrDartP3)));
                    break;

                case Tool::EllipticalArc:
                {
                    const QSharedPointer<VEllipticalArc> elArc = m_data->GeometricObject<VEllipticalArc>(tool_id);
                    SCASSERT(!elArc.isNull())
                    iconFileName = ":/toolicon/32x32/el_arc.png";
                    objName = tr("%1 - Arc Elliptical").arg(elArc->NameForHistory(tr("ElArc_")));
                    break;
                }

                case Tool::Rotation:
                {
                    iconFileName = ":/toolicon/32x32/rotation.png";
                    objName = tr("%1 - Rotation")
                                 .arg(getObjName(obj_id == NULL_ID ? tool_id : obj_id));
                    break;
                }

                case Tool::Move:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(obj_id);
                    iconFileName = ":/toolicon/32x32/move.png";
                    objName = tr("%1 - Move")
                                 .arg(getObjName(obj_id == NULL_ID ? tool_id : obj_id));
                    break;
                }

                case Tool::MirrorByLine:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(obj_id);
                    iconFileName = ":/toolicon/32x32/mirror_by_line.png";
                    objName = tr("%1 - Mirror by Line")
                                 .arg(getObjName(obj_id == NULL_ID ? tool_id : obj_id));
                    break;
                }

                case Tool::MirrorByAxis:
                {
                    const QSharedPointer<VGObject> obj = m_data->GetGObject(obj_id);
                    iconFileName = ":/toolicon/32x32/mirror_by_axis.png";
                    objName = tr("%1 - Mirror by Axis")
                                 .arg(getObjName(obj_id == NULL_ID ? tool_id : obj_id));
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
            QList<quint32>item_data;
            item_data.append(obj_id);
            item_data.append(tool_id);

            QListWidgetItem *item = new QListWidgetItem(objName);
            item->setIcon(QIcon(iconFileName));
            item->setData(Qt::UserRole,  QVariant::fromValue(item_data));
            ui->groupItems_list_widget->addItem(item);
            return;
        }

        catch (const VExceptionBadId &error)
        {
            QList<quint32>item_data;
            item_data.append(obj_id);
            item_data.append(tool_id);

            qDebug() << error.ErrorMessage() << Q_FUNC_INFO;
            QListWidgetItem *item = new QListWidgetItem(objName);
            item->setIcon(QIcon(":/icons/win.icon.theme/16x16/status/dialog-warning.png"));
            item->setData(Qt::UserRole,  QVariant::fromValue(item_data));
            ui->groupItems_list_widget->addItem(item);
            return;
        }
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPointName get the name of the point by tool id.
 * @param tool_id Id of the tool the object belongs to.
 * @return name of point.
 */
QString GroupsWidget::getPointName(quint32 tool_id)
{
    return m_data->GeometricObject<VPointF>(tool_id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief attrUInt get unsigned integer attribute.
 * @param dom_element Dom Element.
 * @param name  Name of Attribute.
 * @return unsigned interger.
 */
quint32 GroupsWidget::attrUInt(const QDomElement &dom_element, const QString &name)
{
    return m_doc->GetParametrUInt(dom_element, name, "0");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getObjName get the name of the graphics object.
 * @param tool_id Id of the tool the object belongs to.
 * @return name of object.
 */
QString GroupsWidget::getObjName(quint32 tool_id)
{
    try
    {
        const auto obj = m_data->GetGObject(tool_id);
        return obj->name();
    }
    catch (const VExceptionBadId &error)
    {
        qCDebug(WidgetGroups, "Error! Couldn't get object name by id = %s. %s %s", qUtf8Printable(QString().setNum(tool_id)),
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
    const quint32 group_id = getGroupId();
    const int row = ui->groupItems_list_widget->currentRow();
    if (ui->groupItems_list_widget->count() == 0 || row == -1 || row >= ui->groupItems_list_widget->count())
    {
        return;
    }

    QListWidgetItem *row_item = ui->groupItems_list_widget->item(row);
    SCASSERT(row_item != nullptr);
    QList<quint32> item_data = row_item->data(Qt::UserRole).value<QList<quint32>>();
    const quint32 tool_id = item_data.last();
    const quint32 obj_id =  item_data.first();

    QMenu menu;
    // Add Move Group Item menu
    QMap<quint32,QString> groupsNotContainingItem =  m_doc->getGroupsContainingItem(tool_id, obj_id, false);
    QActionGroup *actionMoveGroupMenu= new QActionGroup(this);

    if(!groupsNotContainingItem.empty())
    {
        QMenu *menuMoveGroupItem = menu.addMenu(QIcon("://icon/svg/list-move.svg"), tr("Move Group Object"));
        QStringList list = QStringList(groupsNotContainingItem.values());
        list.sort(Qt::CaseInsensitive);

        for(int i=0; i<list.count(); ++i)
        {
            QAction *actionMoveGroupItem = menuMoveGroupItem->addAction(list[i]);
            actionMoveGroupMenu->addAction(actionMoveGroupItem);
            const quint32 group_id = groupsNotContainingItem.key(list[i]);
            actionMoveGroupItem->setData(group_id);
        }
    }
    // Add Remove Group Item menu
    QAction *actionRemove = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Remove Group Object"));

    QAction *selectedAction = menu.exec(ui->groupItems_list_widget->viewport()->mapToGlobal(pos));
    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == actionRemove)
    {
        const bool locked = m_doc->getGroupLock(group_id);
        if (locked == false)
        {
            qCDebug(WidgetGroups, "Remove Tool %s from Group %s.",
                    qUtf8Printable(QString().setNum(tool_id)),
                    qUtf8Printable(QString().setNum(group_id)));

            QListWidgetItem *row_item = ui->groupItems_list_widget->item(row);
            SCASSERT(row_item != nullptr);
            QList<quint32> item_data = row_item->data(Qt::UserRole).value<QList<quint32>>();
            delete ui->groupItems_list_widget->item(row);

            QDomElement item = m_doc->removeGroupItem(item_data.last(), item_data.first(), group_id);

            VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
            SCASSERT(scene != nullptr)
            scene->clearSelection();

            VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
            SCASSERT(window != nullptr)
            {
                RemoveGroupItem *command = new RemoveGroupItem(item, m_doc, group_id);
                connect(command, &RemoveGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
                qApp->getUndoStack()->push(command);
            }
        }
    }
    else if (selectedAction->actionGroup() == actionMoveGroupMenu)
    {
      const quint32 destinationGroupId = selectedAction->data().toUInt();
      const bool sourceLock = m_doc->getGroupLock(group_id);
      const bool destinationLock = m_doc->getGroupLock(destinationGroupId);
      if ((sourceLock == false) && (destinationLock == false))      //only move if both groups are unlocked
      {

          qCDebug(WidgetGroups, "Move Tool %s from Group %s to Group %s.",
                  qUtf8Printable(QString().setNum(tool_id)),
                  qUtf8Printable(QString().setNum(group_id)),
                  qUtf8Printable(QString().setNum(destinationGroupId)));

        QListWidgetItem *row_item = ui->groupItems_list_widget->item(row);
        SCASSERT(row_item != nullptr);
        QList<quint32> item_data = row_item->data(Qt::UserRole).value<QList<quint32>>();
        delete ui->groupItems_list_widget->item(row);

         QDomElement source_item = m_doc->removeGroupItem(item_data.last(), item_data.first(), group_id);
         QDomElement dest_item = m_doc->addGroupItem(item_data.last(), item_data.first(), destinationGroupId);

         updateGroups();

         VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
         SCASSERT(scene != nullptr)
         scene->clearSelection();

         VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
         SCASSERT(window != nullptr)
         {
             MoveGroupItem *command = new MoveGroupItem(source_item, dest_item, m_doc, group_id, destinationGroupId);
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
        QTableWidgetItem *item = ui->groups_table_widget->item(row, 0);
        const quint32 group_id = item->data(Qt::UserRole).toUInt();
        const bool locked = m_doc->getGroupLock(group_id);
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
    QTableWidgetItem *item = ui->groups_table_widget->item(row, 1);
    const quint32 group_id = item->data(Qt::UserRole).toUInt();
    const bool locked = m_doc->getGroupLock(group_id);
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
    GOType obj_type;
    const auto objects = m_data->DataGObjects();
    QList<quint32> item_data = item->data(Qt::UserRole).value<QList<quint32>>();
    quint32 tool_id = item_data.last();
    quint32 obj_id =  item_data.first();
    if (obj_id != NULL_ID)
    {
        tool_id = obj_id;
    }

    if (objects->contains(tool_id))
    {
        obj_type = m_data->GetGObject(tool_id)->getType();
    }
    else
    {
        //Some tools are not GObjects in the container. Need to look in the Dom Doc instead
        const QDomElement dom_element = m_doc->elementById(tool_id);
        if (dom_element.tagName() == VAbstractPattern::TagLine)
        {
            //Use the line's FirstPoint as tool_id and reset toolType
            tool_id = attrUInt(dom_element, AttrFirstPoint);
        }
        else
        {
            return;
        }
        obj_type = m_data->GetGObject(tool_id)->getType();
    }

    switch(obj_type)
    {
        case GOType::Point:
        {
            QSharedPointer<VPointF> point = m_data->GeometricObject<VPointF>(tool_id);
            zoomToObject(point);
            return;
        }

        case GOType::Arc:
        case GOType::EllipticalArc:
        {
            const QSharedPointer<VAbstractArc> curve = m_data->GeometricObject<VAbstractArc>(tool_id);
            QSharedPointer<VPointF>point(new VPointF(curve->GetCenter()));

            zoomToObject(point);
            return;
        }

        case GOType::Spline:
        case GOType::SplinePath:
        case GOType::CubicBezier:
        case GOType::CubicBezierPath:
        {
            const QSharedPointer<VAbstractCurve> curve = m_data->GeometricObject<VAbstractCurve>(tool_id);
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
 * @param point If selected list item is a point use the item's tool_id. if the list item is a curve or line then
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

    int row = ui->groups_table_widget->currentRow();
    setGroupVisibility(ui->groups_table_widget->item(row, 0), getGroupId(), true);

    // show point name if it's hidden
    quint32 tool_id = point->getIdTool();
    const quint32 object_id = point->getIdObject();
    if (object_id != NULL_ID)
    {
        tool_id = object_id;
    }
    if (tool_id != NULL_ID)
    {
        if (VAbstractTool *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(tool_id)))
        {
            tool->setPointNameVisiblity(tool_id, true);
        }
    }

    // show any hiden groups containing object
    showGroups(m_doc->getGroupsContainingItem(tool_id, object_id, true));

    return;
}

void GroupsWidget::setGroupVisibility(QTableWidgetItem *item, const quint32 &group_id, const bool &visible)
{
    m_doc->setGroupVisibility(group_id, visible);
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
    settings.setValue("splitterSizes", ui->groups_splitter->saveState());
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

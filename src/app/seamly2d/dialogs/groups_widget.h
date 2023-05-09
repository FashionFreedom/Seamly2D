/***************************************************************************
 **  @file   groups_widget.h
 **  @author Douglas S Caskey
 **  @date   Mar 1, 2023
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
  **  @file   vwidgetgroups.h
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

#ifndef GROUPS_WIDGET_H
#define GROUPS_WIDGET_H

#include "../ifc/xml/vabstractpattern.h"
#include "../vtools/dialogs/tools/editgroup_dialog.h"

#include <QColor>
#include <QTableWidget>
#include <QWidget>

class VAbstractPattern;
class VContainer;
class EditGroupDialog;

namespace Ui
{
    class GroupsWidget;
}

class GroupsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit          GroupsWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent = nullptr);
    virtual          ~GroupsWidget();

public slots:
    void              updateGroups();
    void              showGroups(QMap<quint32,QString> groups);
    void              showAllGroups();
    void              hideAllGroups();
    void              lockAllGroups();
    void              unlockAllGroups();
    void              addGroupToList();
    void              deleteGroupFromList();
    void              editGroup();
    quint32           getGroupId();
    QString           getCurrentGroupName();
    void              setAddGroupEnabled(bool value);
    void              clear();

private slots:
    void              groupVisibilityChanged(int row, int column);
    void              groupLockChanged(int row, int column);
    void              renameGroup(int row, int column);
    void              groupContextMenu(const QPoint &pos);
    void              draftBlockHasGroups(bool value);

private:
    Q_DISABLE_COPY(GroupsWidget)
    Ui::GroupsWidget *ui;
    VAbstractPattern *m_doc;
    VContainer       *m_data;
    bool              m_patternHasGroups;
    quint32           m_currentGroupId;

    void              fillTable(const QMap<quint32, GroupAttributes> &groups);
    void              fillGroupItemList();
    void              addGroupItem(const quint32 &toolId, const quint32 &objId, const Tool &tooltype);
    void              groupItemContextMenu(const QPoint &pos);
    void              itemDoubleClicked(QListWidgetItem *item);
    void              zoomToObject(QSharedPointer<VPointF> point);
    void              setGroupVisibility(QTableWidgetItem *item, const quint32 &groupId, const bool &visible);
    QString           getPointName(quint32 pointId);
    quint32           attrUInt(const QDomElement &domElement, const QString &name);
    QString           getObjName(quint32 id);
    void              splitterMoved(int pos, int index);
};

#endif // GROUPS_WIDGET_H

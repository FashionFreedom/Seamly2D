/***************************************************************************
 **  @file   editgroup_dialog.h
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
 **  @file   dialog_editgroup.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
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

#ifndef EDIT_GROUP_DIALOG_H
#define EDIT_GROUP_DIALOG_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "dialogtool.h"

namespace Ui
{
    class EditGroupDialog;
}

class EditGroupDialog : public DialogTool
{
    Q_OBJECT

public:
    explicit                  EditGroupDialog(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual                  ~EditGroupDialog();

    void                      setName(const QString &name);
    QString                   getName() const;

    QMap<quint32, quint32>    getGroupData() const;
    QString                   getColor() const;
    void                      setColor(const QString &color);
    QString                   getLineType() const;
    void                      setLineType(const QString &type);
    QString                   getLineWeight() const;
    void                      setLineWeight(const QString &weight);

    virtual void              ShowDialog(bool click) Q_DECL_OVERRIDE;

public slots:
    virtual void              SelectedObject(bool selected, quint32 object, quint32 tool) Q_DECL_OVERRIDE;

private slots:
    void                      nameChanged();

private:
                              Q_DISABLE_COPY(EditGroupDialog)
    Ui::EditGroupDialog      *ui;
    VAbstractPattern         *m_doc;
    QMap<quint32, quint32>    m_groupData;
    QString                   m_oldGroupName;
};

#endif // EDIT_GROUP_DIALOG_H

/***************************************************************************
 **  @file   addtogroup_dialog.h
 **  @author Douglas S Caskey
 **  @date   Mar 14, 2023
 **
 **  @copyright
 **  Copyright (C) 2023 Seamly, LLC
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

#ifndef ADD_TO_GROUP_DIALOG_H
#define ADD_TO_GROUP_DIALOG_H

#include "../tools/dialogtool.h"

#include <qcompilerdetection.h>
#include <QDialog>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QSound>
#include <QString>
#include <QtGlobal>

class VContainer;

namespace Ui
{
    class AddToGroupDialog;
}

class AddToGroupDialog : public DialogTool
{
    Q_OBJECT

public:
                              AddToGroupDialog(VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual                  ~AddToGroupDialog();

    void                      fillNameBox();

    QString                   getName() const;

    QMap<quint32, quint32>    getGroupData() const;

    virtual void              ShowDialog(bool click) Q_DECL_OVERRIDE;

public slots:
    virtual void              SelectedObject(bool selected, quint32 object, quint32 tool) Q_DECL_OVERRIDE;

private:
                              Q_DISABLE_COPY(AddToGroupDialog)
    Ui::AddToGroupDialog     *ui;
    VAbstractPattern         *m_doc;
    QMap<quint32, quint32>    m_groupData;
    QSound                   *m_beep;
};

#endif // ADD_TO_GROUP_DIALOG_H

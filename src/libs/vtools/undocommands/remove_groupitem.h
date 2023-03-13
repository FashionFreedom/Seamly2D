/***************************************************************************
 **  @file   remove_groupitem.h
 **  @author Douglas S Caskey
 **  @date   Mar 1, 2023
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

#ifndef REMOVE_GROUPITEM_H
#define REMOVE_GROUPITEM_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class RemoveGroupItem : public VUndoCommand
{
    Q_OBJECT
public:
                  RemoveGroupItem(const QDomElement &xml, VAbstractPattern *doc, quint32 groupId, QUndoCommand *parent = nullptr);
    virtual      ~RemoveGroupItem();
    virtual void  undo() Q_DECL_OVERRIDE;
    virtual void  redo() Q_DECL_OVERRIDE;

signals:
    void          updateGroups();

private:
                  Q_DISABLE_COPY(RemoveGroupItem)
    const QString m_activeDrawName;
};

#endif // REMOVE_GROUPITEM_H

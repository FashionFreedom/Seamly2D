/***************************************************************************
 **  @file   move_groupitem.h
 **  @author Douglas S Caskey
 **  @date   Mar 2, 2023
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

#ifndef MOVE_GROUPITEM_H
#define MOVE_GROUPITEM_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class MoveGroupItem : public VUndoCommand
{
    Q_OBJECT
public:
                   MoveGroupItem(const QDomElement &source, const QDomElement &dest, VAbstractPattern *doc, quint32 sourceId,
                                 quint32 destinationId, QUndoCommand *parent = nullptr);
    virtual       ~MoveGroupItem();
    virtual void   undo() Q_DECL_OVERRIDE;
    virtual void   redo() Q_DECL_OVERRIDE;

signals:
    void           updateGroups();

private:
                   Q_DISABLE_COPY(MoveGroupItem)
    QDomElement    m_source;
    QDomElement    m_dest;
    const QString  m_activeDraftblockName;
    quint32        m_sourceGroupId;
    quint32        m_destinationGroupId;
};

#endif // MOVE_GROUPITEM_H

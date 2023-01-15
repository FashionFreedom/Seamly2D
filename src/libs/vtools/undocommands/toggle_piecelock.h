/***************************************************************************
 **  @file   toggle_piecelock.h
 **  @author Douglas S Caskey
 **  @date   Jan 1, 2023
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

#ifndef TOGGLE_PIECELOCK_H
#define TOGGLE_PIECELOCK_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class TogglePieceLock : public VUndoCommand
{
    Q_OBJECT
public:
    TogglePieceLock(quint32 id, bool lock, VContainer *data, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    virtual     ~TogglePieceLock();
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
    virtual int  id() const Q_DECL_OVERRIDE;
    quint32      getpieceId() const;
    bool         getNewLock() const;

signals:
    void         updateList(quint32 m_id);

private:
    Q_DISABLE_COPY(TogglePieceLock)
    quint32      m_id;
    VContainer  *m_data;
    bool         m_oldLock;
    bool         m_newLock;

    void         doCmd(bool lock);
};

#endif // TOGGLE_PIECELOCK_H

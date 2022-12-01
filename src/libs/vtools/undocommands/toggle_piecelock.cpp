/***************************************************************************
 **  @file   toggle_piecelock.cpp
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
 **  GNU General Public License for more elements.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "toggle_piecelock.h"

#include <QDomElement>
#include <QHash>
#include <QMessageLogger>
#include <QtDebug>

#include "vundocommand.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vwidgets/vabstractmainwindow.h"

//---------------------------------------------------------------------------------------------------------------------
TogglePieceLock::TogglePieceLock(quint32 id, bool lock, VContainer *data, VAbstractPattern *doc, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_id(id)
    , m_data(data)
    , m_oldLock(!lock)
    , m_newLock(lock)
{
    setText(tr("Pattern piece lock"));
}

//---------------------------------------------------------------------------------------------------------------------
TogglePieceLock::~TogglePieceLock()
{
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceLock::undo()
{
    if (m_newLock != m_oldLock)
    {
        doCmd(m_oldLock);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceLock::redo()
{
    if (m_newLock != m_oldLock)
    {
        doCmd(m_newLock);
    }
}

//---------------------------------------------------------------------------------------------------------------------
int TogglePieceLock::id() const
{
    return static_cast<int>(UndoCommand::TogglePieceLock);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 TogglePieceLock::getpieceId() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
bool TogglePieceLock::getNewLock() const
{
    return m_newLock;
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceLock::doCmd(bool lock)
{
    QDomElement element = doc->elementById(m_id, VAbstractPattern::TagPiece);
    if (element.isElement())
    {
        doc->SetAttribute(element, AttrPieceLocked, lock);

        VPiece piece = m_data->DataPieces()->value(m_id);
        piece.setIsLocked(lock);
        m_data->UpdatePiece(m_id, piece);

        emit updateList(m_id);

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        window->ShowToolTip(tr("Piece lock changed: ") + (lock ? tr("Locked") : tr("Unlocked")));
    }
    else
    {
        return;
    }
}

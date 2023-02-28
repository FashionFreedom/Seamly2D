/***************************************************************************
 **  @file   togglepieceinlayout.cpp
 **  @author Douglas S Caskey
 **  @date   Jan 2, 2023
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   toggledetailinlayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2016
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

#include "togglepieceinlayout.h"

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
TogglePieceInLayout::TogglePieceInLayout(quint32 id, bool state, VContainer *data, VAbstractPattern *doc,
                                         QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_id(id)
    , m_data(data)
    , m_oldState(!state)
    , m_newState(state)
{
    setText(tr("Piece in Layout List"));
}

//---------------------------------------------------------------------------------------------------------------------
TogglePieceInLayout::~TogglePieceInLayout()
{
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::undo()
{
    qCDebug(vUndo, "TogglePieceInLayout::undo().");

    if (m_newState != m_oldState)
    {
        doCmd(m_oldState);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::redo()
{
    qCDebug(vUndo, "TogglePieceInLayout::redo().");

    if (m_newState != m_oldState)
    {
        doCmd(m_newState);
    }
}

//---------------------------------------------------------------------------------------------------------------------
int TogglePieceInLayout::id() const
{
    return static_cast<int>(UndoCommand::TogglePieceInLayout);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 TogglePieceInLayout::getPieceId() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
bool TogglePieceInLayout::getNewState() const
{
    return m_newState;
}

//---------------------------------------------------------------------------------------------------------------------
void TogglePieceInLayout::doCmd(bool state)
{
    QDomElement element = doc->elementById(m_id, VAbstractPattern::TagPiece);
    if (element.isElement())
    {
        doc->SetAttribute(element, AttrInLayout, state);

        VPiece piece = m_data->DataPieces()->value(m_id);
        piece.SetInLayout(state);
        m_data->UpdatePiece(m_id, piece);
        emit updateList(m_id);

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        window->ShowToolTip(tr("Include piece in layout changed: ") + (state ? tr("Include") : tr("Exclude")));
    }
    else
    {
        qDebug("Can't get piece by id = %u.", m_id);
        return;
    }
}

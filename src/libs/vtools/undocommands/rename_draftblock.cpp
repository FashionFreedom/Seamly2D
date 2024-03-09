/***************************************************************************
 **  @file   rename_draftblock.cpp
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

 /************************************************************************
 **
 **  @file   renamepp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 7, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Valentina project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#include "rename_draftblock.h"

#include <QComboBox>
#include <QDomElement>

#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
RenameDraftBlock::RenameDraftBlock(VAbstractPattern *doc, const QString &newBlockName, QComboBox *combo, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_combo(combo)
    , m_newBlockName(newBlockName)
    , m_oldBlockName(QString())
{
    setText(tr("rename pattern piece"));
    SCASSERT(m_combo != nullptr)
    m_oldBlockName = doc->getActiveDraftBlockName();
}

//---------------------------------------------------------------------------------------------------------------------
RenameDraftBlock::~RenameDraftBlock()
{}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::undo()
{
    qCDebug(vUndo, "Undo.");

    changeName(m_newBlockName, m_oldBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::redo()
{
    qCDebug(vUndo, "Redo.");

    changeName(m_oldBlockName, m_newBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
bool RenameDraftBlock::mergeWith(const QUndoCommand *command)
{
    const RenameDraftBlock *renameCommand = static_cast<const RenameDraftBlock *>(command);
    SCASSERT(renameCommand != nullptr)

    const QString oldName = renameCommand->getOldName();
    if (m_newBlockName != oldName)
    {
        return false;
    }

    m_newBlockName = renameCommand->getNewName();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int RenameDraftBlock::id() const
{
    return static_cast<int>(UndoCommand::RenameDraftBlock);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::changeName(const QString &oldName, const QString &newName)
{
    if (doc->renameDraftBlock(oldName, newName))
    {
        m_combo->setItemText(m_combo->findText(oldName), newName);
    }
    else
    {
        qCWarning(vUndo, "Can't change pattern piece name");
    }
}

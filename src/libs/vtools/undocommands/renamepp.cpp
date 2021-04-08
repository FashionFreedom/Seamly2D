/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
 **
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   renamepp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 7, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "renamepp.h"

#include <QComboBox>
#include <QDomElement>

#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
RenameDraftBlock::RenameDraftBlock(VAbstractPattern *doc, const QString &newPPname, QComboBox *combo, QUndoCommand *parent)
    :VUndoCommand(QDomElement(), doc, parent), combo(combo), newPPname(newPPname), oldPPname(QString())
{
    setText(tr("rename pattern piece"));
    SCASSERT(combo != nullptr)
    oldPPname = doc->GetNameActivPP();
}

//---------------------------------------------------------------------------------------------------------------------
RenameDraftBlock::~RenameDraftBlock()
{}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::undo()
{
    qCDebug(vUndo, "Undo.");

    ChangeName(newPPname, oldPPname);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::redo()
{
    qCDebug(vUndo, "Redo.");

    ChangeName(oldPPname, newPPname);
}

//---------------------------------------------------------------------------------------------------------------------
bool RenameDraftBlock::mergeWith(const QUndoCommand *command)
{
    const RenameDraftBlock *renameCommand = static_cast<const RenameDraftBlock *>(command);
    SCASSERT(renameCommand != nullptr)

    const QString oldName = renameCommand->getOldPPname();
    if (newPPname != oldName)
    {
        return false;
    }

    newPPname = renameCommand->getNewPPname();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int RenameDraftBlock::id() const
{
    return static_cast<int>(UndoCommand::RenameDraftBlock);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameDraftBlock::ChangeName(const QString &oldName, const QString &newName)
{
    if (doc->ChangeNamePP(oldName, newName))
    {
        combo->setItemText(combo->findText(oldName), newName);
    }
    else
    {
        qCWarning(vUndo, "Can't change pattern piece name");
    }
}

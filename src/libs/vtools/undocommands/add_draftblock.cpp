/***************************************************************************
 **  @file   add_draftblock.cpp
 **  @author Douglas S Caskey
 **  @date   Dec 27, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
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
 **  @file   addpatternpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "add_draftblock.h"

#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
AddDraftBlock::AddDraftBlock(const QDomElement &xml, VAbstractPattern *doc, const QString &draftBlockName,
                             QUndoCommand *parent)
    : VUndoCommand(xml, doc, parent)
    , draftBlockName(draftBlockName)
{
    SCASSERT(draftBlockName.isEmpty() == false)
    setText(tr("add draft block %1").arg(draftBlockName));
}

//---------------------------------------------------------------------------------------------------------------------
AddDraftBlock::~AddDraftBlock()
{}

//---------------------------------------------------------------------------------------------------------------------
void AddDraftBlock::undo()
{
    qCDebug(vUndo, "Undo.");

    if (doc->draftBlockCount() <= 1)
    {
        emit ClearScene();
    }
    else
    {
        QDomElement rootElement = doc->documentElement();
        QDomElement draftBlock = doc->getDraftBlockElement(draftBlockName);
        rootElement.removeChild(draftBlock);
        emit NeedFullParsing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AddDraftBlock::redo()
{
    qCDebug(vUndo, "Redo.");

    if (doc->draftBlockCount() == 0)
    {
        doc->CreateEmptyFile();
    }

    QDomElement rootElement = doc->documentElement();

    rootElement.appendChild(xml);

    RedoFullParsing();
}

/***************************************************************************
 **  @file   delete_draftblock.h
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
 **  @file   deletepatternpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2014
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

#include "delete_draftblock.h"

#include <QDomNode>
#include <QDomNodeList>

#include "vundocommand.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/logging.h"

//---------------------------------------------------------------------------------------------------------------------
DeleteDraftBlock::DeleteDraftBlock(VAbstractPattern *doc, const QString &draftBlockName, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , draftBlockName(draftBlockName)
    , draftBlock(QDomElement())
    , previousDraftBlockName(QString())
{
    setText(tr("delete draft block %1").arg(draftBlockName));

    const QDomElement block = doc->getDraftBlockElement(draftBlockName);
    draftBlock = block.cloneNode().toElement();
    const QDomElement previousDraftBlock = block.previousSibling().toElement();//find previous pattern piece
    if (not previousDraftBlock.isNull() && previousDraftBlock.tagName() == VAbstractPattern::TagDraftBlock)
    {
        previousDraftBlockName = doc->GetParametrString(previousDraftBlock, VAbstractPattern::AttrName, "");
    }
}

//---------------------------------------------------------------------------------------------------------------------
DeleteDraftBlock::~DeleteDraftBlock()
{}

//---------------------------------------------------------------------------------------------------------------------
void DeleteDraftBlock::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement rootElement = doc->documentElement();

    if (not previousDraftBlockName.isEmpty())
    { // not first in the list, add after tag draft block
        const QDomNode previousDraftBlock = doc->getDraftBlockElement(previousDraftBlockName);
        rootElement.insertAfter(draftBlock, previousDraftBlock);
    }
    else
    { // first in the list, add before tag draftBlock
        const QDomNodeList list = rootElement.elementsByTagName(VAbstractPattern::TagDraftBlock);
        QDomElement block;

        if (not list.isEmpty())
        {
            block = list.at(0).toElement();
        }

        Q_ASSERT_X(not block.isNull(), Q_FUNC_INFO, "Couldn't' find tag draft block");
        rootElement.insertBefore(draftBlock, block);
    }

    emit NeedFullParsing();
    doc->changeActiveDraftBlock(draftBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
void DeleteDraftBlock::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement rootElement = doc->documentElement();
    const QDomElement draftBlock = doc->getDraftBlockElement(draftBlockName);
    rootElement.removeChild(draftBlock);
    emit NeedFullParsing();
}

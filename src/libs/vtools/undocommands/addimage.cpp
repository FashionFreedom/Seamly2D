/******************************************************************************
 **  @file   addimage.cpp
 **  @author Evans PERRET
 **  @date   July 28, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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
 *****************************************************************************/

#include "addimage.h"

//---------------------------------------------------------------------------------------------------------------------
AddImage::AddImage(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent)
    : VUndoCommand(xml, doc, parent)
{
    setText(tr("add image"));
    nodeId = doc->getParameterId(xml);
}

//---------------------------------------------------------------------------------------------------------------------
AddImage::~AddImage()
{}

//---------------------------------------------------------------------------------------------------------------------
void AddImage::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement draftImages = doc->createDraftImages();
    if (not draftImages.isNull())
    {
        QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagDraftImage);
        if (domElement.isElement())
        {
            if (draftImages.removeChild(domElement).isNull())
            {
                qCDebug(vUndo, "Can't delete node");
                return;
            }
        }
        else
        {
            qCDebug(vUndo, "Can't get node by id = %u.", nodeId);
            return;
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find tag %s.", qUtf8Printable(VAbstractPattern::TagDraftImages));
        return;
    }
    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void AddImage::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement draftImages = doc->createDraftImages();
    if (not draftImages.isNull())
    {
        draftImages.appendChild(xml);
    }
    else
    {
        qCDebug(vUndo, "Can't get tag backgroundImages.");
        return;
    }

    RedoFullParsing();
}

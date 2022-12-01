/***************************************************************************
 **  @file   addpiece.cpp
 **  @author Douglas S Caskey
 **  @date   Dec 11, 2022
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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
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
 **  GNU General Public License for more pieces.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "addpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"

//---------------------------------------------------------------------------------------------------------------------
AddPiece::AddPiece(const QDomElement &xml, VAbstractPattern *doc, const VPiece &piece, const QString &blockName,
                   QUndoCommand *parent)
    : VUndoCommand(xml, doc, parent)
    , m_piece(piece)
    , m_blockName(blockName)
{
    setText(tr("add piece"));
    nodeId = doc->getParameterId(xml);
}

//---------------------------------------------------------------------------------------------------------------------
AddPiece::~AddPiece()
{}

//---------------------------------------------------------------------------------------------------------------------
void AddPiece::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement pieces = getPiecesElement();
    if (not pieces.isNull())
    {
        QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPiece);
        if (domElement.isElement())
        {
            if (pieces.removeChild(domElement).isNull())
            {
                qCDebug(vUndo, "Can't delete node");
                return;
            }

            DecrementReferences(m_piece.GetPath().GetNodes());
            DecrementReferences(m_piece.GetCustomSARecords());
            DecrementReferences(m_piece.GetInternalPaths());
            DecrementReferences(m_piece.getAnchors());
        }
        else
        {
            qCDebug(vUndo, "Can't get node by id = %u.", nodeId);
            return;
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find tag %s.", qUtf8Printable(VAbstractPattern::TagPieces));
        return;
    }
    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void AddPiece::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement pieces = getPiecesElement();
    if (not pieces.isNull())
    {
        pieces.appendChild(xml);
    }
    else
    {
        qCDebug(vUndo, "Can't find tag %s.", qUtf8Printable(VAbstractPattern::TagPieces));
        return;
    }
    RedoFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement AddPiece::getPiecesElement() const
{
    QDomElement pieces;
    if (m_blockName.isEmpty())
    {
        doc->getActiveNodeElement(VAbstractPattern::TagPieces, pieces);
    }
    else
    {
        pieces = doc->getDraftBlockElement(m_blockName).firstChildElement(VAbstractPattern::TagPieces);
    }
    return pieces;
}

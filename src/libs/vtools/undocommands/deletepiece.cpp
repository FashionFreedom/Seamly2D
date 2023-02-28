/***************************************************************************
 **  @file   deletepiece.cpp
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
 **  @date   9 11, 2016
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

#include "deletepiece.h"

#include <QDomElement>
#include <QHash>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/logging.h"
#include "../vmisc/def.h"
#include "../tools/vdatatool.h"
#include "vundocommand.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"

//---------------------------------------------------------------------------------------------------------------------
DeletePiece::DeletePiece(VAbstractPattern *doc, quint32 id, const VPiece &piece, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_parentNode()
    , m_siblingId(NULL_ID)
    , m_piece (piece)
{
    setText(tr("delete tool"));
    nodeId = id;
    QDomElement domElement = doc->elementById(id, VAbstractPattern::TagPiece);
    if (domElement.isElement())
    {
        xml = domElement.cloneNode().toElement();
        m_parentNode = domElement.parentNode();
        QDomNode previousPiece = domElement.previousSibling();
        if (previousPiece.isNull())
        {
            m_siblingId = NULL_ID;
        }
        else
        {
            // Better save id of previous piece instead of reference to node.
            m_siblingId = doc->GetParametrUInt(previousPiece.toElement(), VAbstractPattern::AttrId, NULL_ID_STR);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't get piece by id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
DeletePiece::~DeletePiece()
{}

//---------------------------------------------------------------------------------------------------------------------
void DeletePiece::undo()
{
    qCDebug(vUndo, "Undo.");

    UndoDeleteAfterSibling(m_parentNode, m_siblingId);
    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePiece::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPiece);
    if (domElement.isElement())
    {
        m_parentNode.removeChild(domElement);

        // Union delete two old pieces and create one new.
        // So when UnionDetail delete piece we can't use FullParsing. So we hide piece on scene directly.
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(nodeId));
        SCASSERT(tool != nullptr);
        tool->hide();

        DecrementReferences(m_piece.GetPath().GetNodes());
        DecrementReferences(m_piece.GetCustomSARecords());
        DecrementReferences(m_piece.GetInternalPaths());
        DecrementReferences(m_piece.getAnchors());
        emit NeedFullParsing(); // Doesn't work when UnionDetail delete piece.
    }
    else
    {
        qCDebug(vUndo, "Can't get piece by id = %u.", nodeId);
        return;
    }
}

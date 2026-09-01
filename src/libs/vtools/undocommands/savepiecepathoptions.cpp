//---------------------------------------------------------------------------------------------------------------------
//  @file   savepiecepathoptions.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2023 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   savepiecepathoptions.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   26 11, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "savepiecepathoptions.h"

#include <QDomElement>
#include <QUndoCommand>
#include <QDebug>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/logging.h"
#include "../tools/nodeDetails/internal_path_tool.h"
#include "../tools/pattern_piece_tool.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/patternformulatokens.h"

using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
SavePiecePathOptions::SavePiecePathOptions(quint32 pieceId, const VPiecePath &oldPath, const VPiecePath &newPath,
                                           VAbstractPattern *doc, VContainer *data, quint32 id,
                                           QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_oldPath(oldPath)
    , m_newPath(newPath)
    , m_data(data)
    , m_pieceId(pieceId)
    , m_nameToIdToken(nameToIdTokenMap(data))
{
    setText(tr("save path options"));
    nodeId = id;
}

//---------------------------------------------------------------------------------------------------------------------
SavePiecePathOptions::~SavePiecePathOptions()
{}

//---------------------------------------------------------------------------------------------------------------------
void SavePiecePathOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPath);
    if (domElement.isElement())
    {
        InternalPathTool::addAttributes(doc, domElement, nodeId, m_oldPath);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        InternalPathTool::addNodes(doc, domElement, m_oldPath, m_nameToIdToken);

        IncrementReferences(m_oldPath.MissingNodes(m_newPath));

        SCASSERT(m_data);
        m_data->UpdatePiecePath(nodeId, m_oldPath);

        if (m_pieceId != NULL_ID)
        {
            if (PatternPieceTool *piece = qobject_cast<PatternPieceTool *>(VAbstractPattern::getTool(m_pieceId)))
            {
                piece->RefreshGeometry();
            }
        }
    }
    else
    {
        qCWarning(vUndo, "Can't find path with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SavePiecePathOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPath);
    if (domElement.isElement())
    {
        InternalPathTool::addAttributes(doc, domElement, nodeId, m_newPath);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        InternalPathTool::addNodes(doc, domElement, m_newPath, m_nameToIdToken);

        DecrementReferences(m_oldPath.MissingNodes(m_newPath));

        SCASSERT(m_data);
        m_data->UpdatePiecePath(nodeId, m_newPath);

        if (m_pieceId != NULL_ID)
        {
            if (PatternPieceTool *piece = qobject_cast<PatternPieceTool *>(VAbstractPattern::getTool(m_pieceId)))
            {
                piece->RefreshGeometry();
            }
        }
    }
    else
    {
        qCWarning(vUndo, "Can't find path with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool SavePiecePathOptions::mergeWith(const QUndoCommand *command)
{
    const SavePiecePathOptions *saveCommand = static_cast<const SavePiecePathOptions *>(command);
    SCASSERT(saveCommand != nullptr);
    const quint32 id = saveCommand->pathId();

    if (id != nodeId)
    {
        return false;
    }

    m_newPath = saveCommand->newPath();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int SavePiecePathOptions::id() const
{
    return static_cast<int>(UndoCommand::SavePiecePathOptions);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 SavePiecePathOptions::pathId() const
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath SavePiecePathOptions::newPath() const
{
    return m_newPath;
}

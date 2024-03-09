/***************************************************************************
 **  @file   savepieceoptions.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
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
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "savepieceoptions.h"

#include <QDomElement>
#include <QPointF>
#include <QUndoCommand>
#include <QDebug>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/logging.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../tools/pattern_piece_tool.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
SavePieceOptions::SavePieceOptions(const VPiece &oldPiece, const VPiece &newPiece, VAbstractPattern *doc, quint32 id,
                                   QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_oldPiece(oldPiece)
    , m_newPiece(newPiece)
{
    setText(tr("save detail option"));
    nodeId = id;
}

//---------------------------------------------------------------------------------------------------------------------
SavePieceOptions::~SavePieceOptions()
{}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPiece);
    if (domElement.isElement())
    {
        PatternPieceTool::AddAttributes(doc, domElement, nodeId, m_oldPiece);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        PatternPieceTool::AddPatternPieceData(doc, domElement, m_oldPiece);
        PatternPieceTool::AddPatternInfo(doc, domElement, m_oldPiece);
        PatternPieceTool::AddGrainline(doc, domElement, m_oldPiece);
        PatternPieceTool::AddNodes(doc, domElement, m_oldPiece);
        PatternPieceTool::AddCSARecords(doc, domElement, m_oldPiece.GetCustomSARecords());
        PatternPieceTool::AddInternalPaths(doc, domElement, m_oldPiece.GetInternalPaths());
        PatternPieceTool::addAnchors(doc, domElement, m_oldPiece.getAnchors());

        IncrementReferences(m_oldPiece.MissingNodes(m_newPiece));
        IncrementReferences(m_oldPiece.MissingCSAPath(m_newPiece));
        IncrementReferences(m_oldPiece.MissingInternalPaths(m_newPiece));
        IncrementReferences(m_oldPiece.missingAnchors(m_newPiece));
        emit NeedLiteParsing(Document::LiteParse);
    }
    else
    {
        qCWarning(vUndo, "Can't find piece with id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPiece);
    if (domElement.isElement())
    {
        PatternPieceTool::AddAttributes(doc, domElement, nodeId, m_newPiece);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        PatternPieceTool::AddPatternPieceData(doc, domElement, m_newPiece);
        PatternPieceTool::AddPatternInfo(doc, domElement, m_newPiece);
        PatternPieceTool::AddGrainline(doc, domElement, m_newPiece);
        PatternPieceTool::AddNodes(doc, domElement, m_newPiece);
        PatternPieceTool::AddCSARecords(doc, domElement, m_newPiece.GetCustomSARecords());
        PatternPieceTool::AddInternalPaths(doc, domElement, m_newPiece.GetInternalPaths());
        PatternPieceTool::addAnchors(doc, domElement, m_newPiece.getAnchors());

        DecrementReferences(m_oldPiece.MissingNodes(m_newPiece));
        DecrementReferences(m_oldPiece.MissingCSAPath(m_newPiece));
        DecrementReferences(m_oldPiece.MissingInternalPaths(m_newPiece));
        DecrementReferences(m_oldPiece.missingAnchors(m_newPiece));

        emit NeedLiteParsing(Document::LiteParse);
    }
    else
    {
        qCWarning(vUndo, "Can't find piece with id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool SavePieceOptions::mergeWith(const QUndoCommand *command)
{
    const SavePieceOptions *saveCommand = static_cast<const SavePieceOptions *>(command);
    SCASSERT(saveCommand != nullptr);
    const quint32 id = saveCommand->pieceId();

    if (id != nodeId)
    {
        return false;
    }

    m_newPiece = saveCommand->getNewPiece();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int SavePieceOptions::id() const
{
    return static_cast<int>(UndoCommand::SavePieceOptions);
}

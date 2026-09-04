//---------------------------------------------------------------------------------------------------------------------
//  @file   savepieceoptions.cpp
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
//  @file   savepieceoptions.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   9 11, 2016
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
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"
#include "../tools/pattern_piece_tool.h"
#include "vundocommand.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
SavePieceOptions::SavePieceOptions(const VPiece &oldPiece, const VPiece &newPiece, VAbstractPattern *doc,
                                   VContainer *data, quint32 id, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_oldPiece(oldPiece)
    , m_newPiece(newPiece)
    , m_nameToIdToken(nameToIdTokenMap(data))
{
    setText(tr("save piece options"));
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
        PatternPieceTool::addAttributes(doc, domElement, nodeId, m_oldPiece, m_nameToIdToken);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        PatternPieceTool::addPieceLabel(doc, domElement, m_oldPiece, m_nameToIdToken);
        PatternPieceTool::addPatternLabel(doc, domElement, m_oldPiece, m_nameToIdToken);
        PatternPieceTool::addGrainline(doc, domElement, m_oldPiece, m_nameToIdToken);
        PatternPieceTool::addNodes(doc, domElement, m_oldPiece, m_nameToIdToken);
        PatternPieceTool::addCSARecords(doc, domElement, m_oldPiece.getCustomSARecords());
        PatternPieceTool::addInternalPaths(doc, domElement, m_oldPiece.getInternalPaths());
        PatternPieceTool::addAnchors(doc, domElement, m_oldPiece.getAnchors());

        IncrementReferences(m_oldPiece.MissingNodes(m_newPiece));
        IncrementReferences(m_oldPiece.MissingCSAPath(m_newPiece));
        IncrementReferences(m_oldPiece.MissingInternalPaths(m_newPiece));
        IncrementReferences(m_oldPiece.missingAnchors(m_newPiece));

        DecrementReferences(m_newPiece.MissingNodes(m_oldPiece));
        DecrementReferences(m_newPiece.MissingCSAPath(m_oldPiece));
        DecrementReferences(m_newPiece.MissingInternalPaths(m_oldPiece));
        DecrementReferences(m_newPiece.missingAnchors(m_oldPiece));

        emit NeedLiteParsing(Document::LiteParse);

        if (auto *tool = qobject_cast<PatternPieceTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->updatePiece(m_oldPiece);
        }
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
        PatternPieceTool::addAttributes(doc, domElement, nodeId, m_newPiece, m_nameToIdToken);
        doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
        PatternPieceTool::addPieceLabel(doc, domElement, m_newPiece, m_nameToIdToken);
        PatternPieceTool::addPatternLabel(doc, domElement, m_newPiece, m_nameToIdToken);
        PatternPieceTool::addGrainline(doc, domElement, m_newPiece, m_nameToIdToken);
        PatternPieceTool::addNodes(doc, domElement, m_newPiece, m_nameToIdToken);
        PatternPieceTool::addCSARecords(doc, domElement, m_newPiece.getCustomSARecords());
        PatternPieceTool::addInternalPaths(doc, domElement, m_newPiece.getInternalPaths());
        PatternPieceTool::addAnchors(doc, domElement, m_newPiece.getAnchors());

        IncrementReferences(m_newPiece.MissingNodes(m_oldPiece));
        IncrementReferences(m_newPiece.MissingCSAPath(m_oldPiece));
        IncrementReferences(m_newPiece.MissingInternalPaths(m_oldPiece));
        IncrementReferences(m_newPiece.missingAnchors(m_oldPiece));

        DecrementReferences(m_oldPiece.MissingNodes(m_newPiece));
        DecrementReferences(m_oldPiece.MissingCSAPath(m_newPiece));
        DecrementReferences(m_oldPiece.MissingInternalPaths(m_newPiece));
        DecrementReferences(m_oldPiece.missingAnchors(m_newPiece));

        emit NeedLiteParsing(Document::LiteParse);

        if (auto *tool = qobject_cast<PatternPieceTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->updatePiece(m_newPiece);
        }
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

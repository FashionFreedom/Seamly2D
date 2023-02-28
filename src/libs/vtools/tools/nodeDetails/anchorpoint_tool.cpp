/***************************************************************************
 **  @file   anchorpoint_tool.cpp
 **  @author Douglas S Caskey
 **  @date   Jan 3, 2023
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vtoolpin.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "anchorpoint_tool.h"
#include "../../dialogs/tools/anchorpoint_dialog.h"
#include "../../undocommands/savepieceoptions.h"
#include "../pattern_piece_tool.h"
#include "../vgeometry/vpointf.h"

const QString AnchorPointTool::ToolType = QStringLiteral("anchor");

//---------------------------------------------------------------------------------------------------------------------
AnchorPointTool *AnchorPointTool::Create(QSharedPointer<DialogTool> dialog, VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull());
    QSharedPointer<AnchorPointDialog> dialogTool = dialog.objectCast<AnchorPointDialog>();
    SCASSERT(not dialogTool.isNull())
    const quint32 pointId = dialogTool->GetPointId();
    const quint32 pieceId = dialogTool->GetPieceId();

    return Create(0, pointId, pieceId, doc, data, Document::FullParse, Source::FromGui);
}

//---------------------------------------------------------------------------------------------------------------------
AnchorPointTool *AnchorPointTool::Create(quint32 _id, quint32 pointId, quint32 pieceId, VAbstractPattern *doc, VContainer *data,
                           const Document &parse, const Source &typeCreation, const QString &blockName,
                           const quint32 &idTool)
{
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = CreateNode<VPointF>(data, pointId);
    }
    else
    {
        QSharedPointer<VPointF> point;
        try
        {
            point = data->GeometricObject<VPointF>(pointId);
        }
        catch (const VExceptionBadId &e)
        { // Possible case. Parent was deleted, but the node object is still here.
            Q_UNUSED(e)
            data->UpdateId(id);
            return nullptr;// Just ignore
        }
        VPointF *anchorPoint = new VPointF(*point);
        anchorPoint->setMode(Draw::Modeling);
        data->UpdateGObject(id, anchorPoint);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }
    VAbstractTool::AddRecord(id, Tool::AnchorPoint, doc);
    AnchorPointTool *point = nullptr;
    if (parse == Document::FullParse)
    {
        point = new AnchorPointTool(doc, data, id, pointId, pieceId, typeCreation, blockName, idTool, doc);

        VAbstractPattern::AddTool(id, point);
        if (idTool != NULL_ID)
        {
            //Some nodes we don't show on scene. Tool that create this nodes must free memory.
            VDataTool *tool = VAbstractPattern::getTool(idTool);
            SCASSERT(tool != nullptr)
            point->setParent(tool);// Adopted by a tool
        }
        else
        {
            // Help to delete the node before each FullParse
            doc->AddToolOnRemove(point);
        }
    }
    else
    {
        doc->UpdateToolData(id, data);
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
QString AnchorPointTool::getTagName() const
{
    return VAbstractPattern::TagPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointTool::AllowHover(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointTool::AllowSelecting(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());

    doc->SetAttribute(domElement, VDomDocument::AttrId, m_id);
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrIdObject, idNode);
    if (idTool != NULL_ID)
    {
        doc->SetAttribute(domElement, AttrIdTool, idTool);
    }

    AddToModeling(domElement);

    if (m_pieceId > NULL_ID)
    {
        const VPiece oldPiece = VAbstractTool::data.GetPiece(m_pieceId);
        VPiece newPiece = oldPiece;

        newPiece.getAnchors().append(m_id);

        SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_pieceId);
        qApp->getUndoStack()->push(saveCommand);// First push then make a connect
        VAbstractTool::data.UpdatePiece(m_pieceId, newPiece);// Update piece because first save will not call lite update
        connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }
}

//---------------------------------------------------------------------------------------------------------------------
AnchorPointTool::AnchorPointTool(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 pointId, quint32 pieceId,
                   const Source &typeCreation, const QString &blockName, const quint32 &idTool, QObject *qoParent)
    : VAbstractNode(doc, data, id, pointId, blockName, idTool, qoParent)
    , m_pieceId(pieceId)
{
    ToolCreation(typeCreation);
}

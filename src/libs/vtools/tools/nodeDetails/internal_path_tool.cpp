//---------------------------------------------------------------------------------------------------------------------
//  @file   internal_path_tool.cpp
//  @author Douglas S Caskey
//  @date   7 Dec, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
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
//  @file   vtoolinternalpath.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   24 11, 2016
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
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "internal_path_tool.h"
#include "../../dialogs/tools/piece/internal_path_dialog.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/patternformulatokens.h"
#include "../../undocommands/savepieceoptions.h"
#include "../../undocommands/savepiecepathoptions.h"
#include "../vmisc/vcommonsettings.h"
#include "../pattern_piece_tool.h"
#include "../ifc/xml/vabstractpattern.h"

using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
InternalPathTool *InternalPathTool::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                       VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull());
    QSharedPointer<InternalPathDialog> dialogTool = dialog.objectCast<InternalPathDialog>();
    SCASSERT(not dialogTool.isNull())
    VPiecePath path = dialogTool->getPiecePath();
    const quint32 pieceId = dialogTool->getPieceId();
    qApp->getUndoStack()->beginMacro("add internal path");
    path.setNodes(PrepareNodes(path, scene, doc, data));

    InternalPathTool *pathTool = Create(NULL_ID, path, pieceId, scene, doc, data, Document::FullParse, Source::FromGui);
    return pathTool;
}

//---------------------------------------------------------------------------------------------------------------------
InternalPathTool *InternalPathTool::Create(quint32 _id, const VPiecePath &path, quint32 pieceId, VMainGraphicsScene *scene,
                                       VAbstractPattern *doc, VContainer *data, const Document &parse,
                                       const Source &typeCreation, const QString &blockName, const quint32 &toolId)
{
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = data->AddPiecePath(path);
    }
    else
    {
        data->UpdatePiecePath(id, path);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(id, Tool::InternalPath, doc);
        //TODO Need create garbage collector and remove all nodes, that we don't use.
        //Better check garbage before each saving file. Check only modeling tags.
        InternalPathTool *pathTool = new InternalPathTool(doc, data, id, pieceId, typeCreation, blockName, toolId, doc);

        VAbstractPattern::AddTool(id, pathTool);
        if (toolId != NULL_ID)
        {
            //Some nodes we don't show on scene. Tool that create this nodes must free memory.
            VDataTool *tool = VAbstractPattern::getTool(toolId);
            SCASSERT(tool != nullptr);
            pathTool->setParent(tool);// Adopted by a tool
        }
        else
        {
            if (typeCreation == Source::FromGui && path.getType() == PiecePathType::InternalPath)
            { // Seam allowance tool already initializated and can't init the path
                SCASSERT(pieceId > NULL_ID);
                PatternPieceTool *saTool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(pieceId));
                SCASSERT(saTool != nullptr);
                pathTool->setParentItem(saTool);
                pathTool->SetParentType(ParentType::Item);
            }
            else
            {
                // Try to prevent memory leak
                scene->addItem(pathTool);// First adopted by scene
                pathTool->hide();// If no one will use node, it will stay hidden
                pathTool->SetParentType(ParentType::Scene);
            }
        }
        return pathTool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString InternalPathTool::getTagName() const
{
    return VAbstractPattern::TagPath;
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const VPiecePath path = VAbstractTool::data.getPiecePath(m_id);
    QColor color          = QColor(path.getLineColor());
    Qt::PenStyle lineType = path.getLineType();
    qreal lineWeight      = ToPixel(path.getLineWeight().toDouble(), Unit::Mm);
    this->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())), lineType, Qt::RoundCap, Qt::RoundJoin));

    QGraphicsPathItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::incrementReferens()
{
    VAbstractTool::incrementReferens();
    if (_referens == 1)
    {
        if (idTool != NULL_ID)
        {
            doc->IncrementReferens(idTool);
        }

        incrementNodes(VAbstractTool::data.getPiecePath(m_id));

        ShowNode();
        QDomElement domElement = doc->elementById(m_id, getTagName());
        if (domElement.isElement())
        {
            doc->SetParametrUsage(domElement, AttrInUse, NodeUsage::InUse);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::decrementReferens()
{
    VAbstractTool::decrementReferens();
    if (_referens == 0)
    {
        if (idTool != NULL_ID)
        {
            doc->DecrementReferens(idTool);
        }

        decrementNodes(VAbstractTool::data.getPiecePath(m_id));

        HideNode();
        QDomElement domElement = doc->elementById(m_id, getTagName());
        if (domElement.isElement())
        {
            doc->SetParametrUsage(domElement, AttrInUse, NodeUsage::NotInUse);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::addAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiecePath &path)
{
    doc->SetAttribute(domElement, VDomDocument::AttrId, id);
    doc->SetAttribute(domElement, AttrName, path.getName());
    doc->SetAttribute(domElement, AttrType, static_cast<int>(path.getType()));
    doc->SetAttribute(domElement, AttrLineColor, path.getLineColor());
    doc->SetAttribute(domElement, AttrLineType, PenStyleToLineType(path.getLineType()));
    doc->SetAttribute(domElement, AttrLineWeight, path.getLineWeight());

    if (path.getType() == PiecePathType::InternalPath)
    {
        doc->SetAttribute(domElement, AttrCut,              path.isCutPath());
        doc->SetAttribute(domElement, AttrExtendStartPoint, path.extendStartPoint());
        doc->SetAttribute(domElement, AttrExtendEndPoint,   path.extendEndPoint());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::FullUpdateFromFile()
{
    refreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::AllowHover(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::AllowSelecting(bool enabled)
{
    Q_UNUSED(enabled)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    const VPiecePath path = VAbstractTool::data.getPiecePath(m_id);
    const VPiecePath newPath = path;

    addAttributes(doc, domElement, m_id, path);

    if (idTool != NULL_ID)
    {
        doc->SetAttribute(domElement, AttrIdTool, idTool);
    }

    addNodes(doc, domElement, path, nameToIdTokenMap(&(VAbstractTool::data)));

    AddToModeling(domElement);

    if (m_pieceId > NULL_ID)
    {
        const VPiece oldPiece = VAbstractTool::data.GetPiece(m_pieceId);
        VPiece newPiece = oldPiece;

        if (path.getType() == PiecePathType::InternalPath)
        {
            newPiece.getInternalPaths().append(m_id);
            incrementReferens();
        }
        else if (path.getType() == PiecePathType::CustomSeamAllowance)
        {
            CustomSARecord record;
            record.path = m_id;

            newPiece.getCustomSARecords().append(record);
            incrementReferens();
        }

        SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_pieceId);
        qApp->getUndoStack()->push(saveCommand);
        connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::ShowNode()
{
    if (parentType != ParentType::Scene)
    {
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::HideNode()
{
    hide();
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::ToolCreation(const Source &typeCreation)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        AddToFile();
        if (typeCreation != Source::FromTool)
        {
            qApp->getUndoStack()->endMacro();
        }
    }
    else
    {
        RefreshDataInFile();
    }
}

//---------------------------------------------------------------------------------------------------------------------
InternalPathTool::InternalPathTool(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 pieceId,
                               const Source &typeCreation, const QString &blockName, const quint32 &toolId,
                               QObject *objParent, QGraphicsItem *parent)
    : VAbstractNode(doc, data, id, NULL_ID, blockName, toolId, objParent)
    , QGraphicsPathItem(parent)
    , m_pieceId(pieceId)
{
    refreshGeometry();
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::refreshGeometry()
{
    const VPiecePath path = VAbstractTool::data.getPiecePath(m_id);
    if (path.getType() == PiecePathType::InternalPath)
    {
        QVector<QPointF> cutLinePath;
        const quint32 pieceId = VAbstractTool::data.pieceIdOfPath(m_id);
        if (pieceId > NULL_ID)
        {
            VPiece piece = VAbstractTool::data.GetPiece(pieceId);
            const VContainer pieceData = VAbstractPattern::getTool(pieceId)->getData();
            cutLinePath = piece.cutPathPoints(&pieceData);
        }
        QPainterPath painterPath = path.PainterPath(this->getData(), cutLinePath);
        painterPath.setFillRule(Qt::OddEvenFill);
        this->setPath(painterPath);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::incrementNodes(const VPiecePath &path) const
{
    for (int i = 0; i < path.nodeCount(); ++i)
    {
        quint32 nodeId = path.at(i).GetId();
        const QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(nodeId);
        quint32 objId = obj->getIdTool();
        doc->IncrementReferens(objId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathTool::decrementNodes(const VPiecePath &path) const
{
    for (int i = 0; i < path.nodeCount(); ++i)
    {
        quint32 nodeId = path.at(i).GetId();
        const QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(nodeId);
        quint32 objId = obj->getIdTool();
        doc->DecrementReferens(objId);
    }
}

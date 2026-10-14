//---------------------------------------------------------------------------------------------------------------------
//  @file   tst_nodepointreferences.cpp
//  @date   14 Oct, 2026
//
//  @copyright
//  Copyright (C)  2026 Seamly, LLC
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

#include "tst_nodepointreferences.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vtools/tools/nodeDetails/vnodepoint.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/undocommands/savepieceoptions.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"

#include <QUndoStack>
#include <QtTest>

namespace
{
const Unit PatternUnit = Unit::Cm;

QString blockName()
{
    return QStringLiteral("measure");
}
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief MeasurePattern minimal pattern document that forwards reference counting to the registered tools the same
/// way VPattern does, without needing the main window.
//---------------------------------------------------------------------------------------------------------------------
class MeasurePattern : public VAbstractPattern
{
public:
    MeasurePattern()
        : VAbstractPattern(nullptr)
    {
    }

    virtual void CreateEmptyFile() override
    {
    }

    virtual void IncrementReferens(quint32 id) const override
    {
        if (VDataTool *tool = tools.value(id, nullptr))
        {
            tool->incrementReferens();
        }
    }

    virtual void DecrementReferens(quint32 id) const override
    {
        if (VDataTool *tool = tools.value(id, nullptr))
        {
            tool->decrementReferens();
        }
    }

    virtual QStringList GetCurrentAlphabet() const override
    {
        return QStringList();
    }

    virtual QString GenerateLabel(const LabelType &type, const QString &reserved_name = QString()) const override
    {
        Q_UNUSED(type)
        Q_UNUSED(reserved_name)
        return QString();
    }

    virtual QString generateSuffix(const QString &type) const override
    {
        Q_UNUSED(type)
        return QString();
    }

    virtual void UpdateToolData(const quint32 &id, VContainer *data) override
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }

    virtual void LiteParseTree(const Document &parse) override
    {
        Q_UNUSED(parse)
    }

    static void clearTools()
    {
        tools.clear();
    }
};

//---------------------------------------------------------------------------------------------------------------------
TST_NodePointReferences::TST_NodePointReferences(QObject *parent)
    : QObject(parent)
    , m_doc(nullptr)
    , m_data(nullptr)
    , m_scene(nullptr)
    , m_view(nullptr)
    , m_base_points()
{
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief init builds an empty draft block with modeling and pieces sections, a scene, a view and five base points.
//---------------------------------------------------------------------------------------------------------------------
void TST_NodePointReferences::init()
{
    QVERIFY(qApp->getUndoStack() != nullptr);
    MeasurePattern::clearTools();

    m_doc = new MeasurePattern();
    QDomElement pattern = m_doc->createElement(QStringLiteral("pattern"));
    QDomElement draft_block = m_doc->createElement(VAbstractPattern::TagDraftBlock);
    draft_block.setAttribute(VAbstractPattern::AttrName, blockName());
    draft_block.appendChild(m_doc->createElement(VAbstractPattern::TagCalculation));
    draft_block.appendChild(m_doc->createElement(VAbstractPattern::TagModeling));
    draft_block.appendChild(m_doc->createElement(VAbstractPattern::TagPieces));
    pattern.appendChild(draft_block);
    m_doc->appendChild(pattern);
    m_doc->changeActiveDraftBlock(blockName(),Document::LiteParse);

    m_data = new VContainer(nullptr, &PatternUnit);
    m_scene = new VMainGraphicsScene();
    m_view = new VMainGraphicsView();
    qApp->setSceneView(m_view);

    m_base_points.clear();
    m_base_points.append(addBasePoint(0, 0, QStringLiteral("A1")));
    m_base_points.append(addBasePoint(100, 0, QStringLiteral("A2")));
    m_base_points.append(addBasePoint(150, 80, QStringLiteral("A3")));
    m_base_points.append(addBasePoint(100, 160, QStringLiteral("A4")));
    m_base_points.append(addBasePoint(0, 160, QStringLiteral("A5")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_NodePointReferences::cleanup()
{
    qApp->getUndoStack()->clear();
    qApp->setSceneView(nullptr);
    delete m_view;
    m_view = nullptr;
    delete m_scene;
    m_scene = nullptr;
    MeasurePattern::clearTools();
    delete m_doc;
    m_doc = nullptr;
    delete m_data;
    m_data = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief removeNodesAfterLoad creates the piece the way the pattern parser does (Source::FromFile), then removes two
/// main path nodes with SavePieceOptions. The removed nodes must end with no references and hidden.
//---------------------------------------------------------------------------------------------------------------------
void TST_NodePointReferences::removeNodesAfterLoad()
{
    QVector<quint32> node_ids;
    for (quint32 base_id : m_base_points)
    {
        node_ids.append(loadNodePoint(base_id));
    }
    const quint32 piece_id = loadPiece(node_ids);
    logNodes(QStringLiteral("removeNodesAfterLoad: after load"), node_ids);

    const QVector<quint32> removed_ids {node_ids.at(1), node_ids.at(2)};
    pushPieceWithout(piece_id, removed_ids);
    logNodes(QStringLiteral("removeNodesAfterLoad: after SavePieceOptions::redo"), node_ids);

    qApp->getUndoStack()->undo();
    logNodes(QStringLiteral("removeNodesAfterLoad: after SavePieceOptions::undo"), node_ids);
    qApp->getUndoStack()->redo();
    logNodes(QStringLiteral("removeNodesAfterLoad: after second redo"), node_ids);

    for (quint32 node_id : node_ids)
    {
        VNodePoint *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node_id));
        QVERIFY(tool != nullptr);
        const bool removed = removed_ids.contains(node_id);
        QCOMPARE(tool->referens(), removed ? 0u : 1u);
        QCOMPARE(tool->isVisible(), !removed);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 TST_NodePointReferences::addBasePoint(qreal x, qreal y, const QString &name)
{
    return m_data->AddGObject(new VPointF(x, y, name, 5, 10));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief loadNodePoint creates a modeling node of a base point as VPattern::ParseNodePoint does when loading a file.
//---------------------------------------------------------------------------------------------------------------------
quint32 TST_NodePointReferences::loadNodePoint(quint32 base_id)
{
    const QSharedPointer<VPointF> point = m_data->GeometricObject<VPointF>(base_id);
    const quint32 node_id = m_data->AddGObject(new VPointF(static_cast<QPointF>(*point), point->name(), point->mx(),
                                                           point->my(), base_id, Draw::Modeling));
    VNodePoint::Create(m_doc, m_data, m_scene, node_id, base_id, Document::FullParse, Source::FromFile, "", NULL_ID);
    return node_id;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief loadPiece creates a piece with the given main path nodes as VPattern does when loading a file, including
/// its piece element in the document.
//---------------------------------------------------------------------------------------------------------------------
quint32 TST_NodePointReferences::loadPiece(const QVector<quint32> &node_ids)
{
    VPiece piece;
    for (quint32 node_id : node_ids)
    {
        piece.GetPath().Append(VPieceNode(node_id, Tool::NodePoint));
    }
    const quint32 piece_id = m_data->AddPiece(piece);

    QDomElement piece_element = m_doc->createElement(VAbstractPattern::TagPiece);
    piece_element.setAttribute(VDomDocument::AttrId, piece_id);
    piece_element.setAttribute(PatternPieceTool::AttrVersion, 2);
    m_doc->getDraftBlockElement(blockName()).firstChildElement(VAbstractPattern::TagPieces).appendChild(piece_element);

    QString width = QStringLiteral("0");
    PatternPieceTool *tool = PatternPieceTool::Create(piece_id, piece, width, m_scene, m_doc, m_data,
                                                      Document::FullParse, Source::FromFile);
    if (tool == nullptr)
    {
        qWarning("loadPiece: PatternPieceTool::Create returned nullptr");
    }
    return piece_id;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief logNodes prints reference count and visibility of each node, so the numbers are visible even on failure.
//---------------------------------------------------------------------------------------------------------------------
void TST_NodePointReferences::logNodes(const QString &stage, const QVector<quint32> &node_ids) const
{
    for (quint32 node_id : node_ids)
    {
        VNodePoint *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node_id));
        if (tool == nullptr)
        {
            qInfo().noquote() << stage << "node" << node_id << "no VNodePoint registered";
            continue;
        }
        const QGraphicsItem *parent_item = tool->parentItem();
        qInfo().noquote() << stage << "node" << node_id << "referens" << tool->referens() << "isVisible"
                          << tool->isVisible() << "isVisibleTo(parentItem)"
                          << (parent_item != nullptr ? tool->isVisibleTo(parent_item) : tool->isVisible())
                          << "parentType" << static_cast<int>(tool->GetParentType());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief pushPieceWithout pushes a SavePieceOptions command whose new piece lacks the given main path nodes.
//---------------------------------------------------------------------------------------------------------------------
void TST_NodePointReferences::pushPieceWithout(quint32 piece_id, const QVector<quint32> &removed_ids)
{
    const VPiece old_piece = m_data->GetPiece(piece_id);
    VPiece new_piece = old_piece;
    QVector<VPieceNode> nodes;
    for (const VPieceNode &node : old_piece.GetPath().getNodes())
    {
        if (!removed_ids.contains(node.GetId()))
        {
            nodes.append(node);
        }
    }
    new_piece.GetPath().setNodes(nodes);
    qApp->getUndoStack()->push(new SavePieceOptions(old_piece, new_piece, m_doc, piece_id));
}

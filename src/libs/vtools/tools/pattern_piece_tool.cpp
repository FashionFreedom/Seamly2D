//---------------------------------------------------------------------------------------------------------------------
//  @file   pattern_piece_dialog.cpp
//  @author Douglas S Caskey
//  @date   Dec 11, 2022
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//  @file   vtoolseamallowance.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   6 11, 2016
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
//  along with Valentina.  if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "pattern_piece_tool.h"

#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/internal_path_tool.h"
#include "../dialogs/tools/piece/pattern_piece_dialog.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vtools/tools/vdatatool.h"
#include "../qmuparser/qmutokenparser.h"
#include "../undocommands/addpiece.h"
#include "../undocommands/deletepiece.h"
#include "../undocommands/movepiece.h"
#include "../undocommands/savepieceoptions.h"
#include "../undocommands/togglepieceinlayout.h"
#include "../undocommands/toggle_piecelock.h"
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vwidgets/nonscalingfill_pathitem.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QInputDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QPainterPathStroker>

// Current version of seam allowance tag need for backward compatibility
const quint8 PatternPieceTool::pieceVersion = 2;

const QString PatternPieceTool::TagCSA     = QStringLiteral("csa");
const QString PatternPieceTool::TagRecord  = QStringLiteral("record");
const QString PatternPieceTool::TagIPaths  = QStringLiteral("iPaths");
const QString PatternPieceTool::TagAnchors = QStringLiteral("anchors");

const QString PatternPieceTool::AttrVersion              = QStringLiteral("version");
const QString PatternPieceTool::AttrForbidFlipping       = QStringLiteral("forbidFlipping");
const QString PatternPieceTool::AttrPieceColor           = QStringLiteral("color");
const QString PatternPieceTool::AttrPieceFill            = QStringLiteral("fill");
const QString PatternPieceTool::AttrPieceLock            = QStringLiteral("locked");
const QString PatternPieceTool::AttrSeamAllowance        = QStringLiteral("seamAllowance");
const QString PatternPieceTool::AttrHideSeamLine         = QStringLiteral("hideMainPath");
const QString PatternPieceTool::AttrSeamAllowanceBuiltIn = QStringLiteral("seamAllowanceBuiltIn");
const QString PatternPieceTool::AttrHeight               = QStringLiteral("height");
const QString PatternPieceTool::AttrUnited               = QStringLiteral("united");
const QString PatternPieceTool::AttrFont                 = QStringLiteral("fontSize");
const QString PatternPieceTool::AttrTopLeftAnchor        = QStringLiteral("topLeftAnchor");
const QString PatternPieceTool::AttrBottomRightAnchor    = QStringLiteral("bottomRightAnchor");
const QString PatternPieceTool::AttrCenterAnchor         = QStringLiteral("centerAnchor");
const QString PatternPieceTool::AttrTopAnchorPoint       = QStringLiteral("topAnchor");
const QString PatternPieceTool::AttrBottomAnchorPoint    = QStringLiteral("bottomAnchor");

//---------------------------------------------------------------------------------------------------------------------
PatternPieceTool *PatternPieceTool::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                               VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(!dialog.isNull());
    QSharedPointer<PatternPieceDialog> dialogTool = dialog.objectCast<PatternPieceDialog>();
    SCASSERT(!dialogTool.isNull())
    VPiece piece = dialogTool->GetPiece();
    QString width = piece.getSeamAllowanceWidthFormula();
    qApp->getUndoStack()->beginMacro("add pattern piece");

    piece.GetPath().setNodes(PrepareNodes(piece.GetPath(), scene, doc, data));

    PatternPieceTool *patternPiece = Create(0, piece, width, scene, doc, data, Document::FullParse, Source::FromGui);

    if (patternPiece != nullptr)
    {
        patternPiece->m_dialog = dialogTool;
        patternPiece->RefreshGeometry();
    }
    return patternPiece;
}

//---------------------------------------------------------------------------------------------------------------------
PatternPieceTool *PatternPieceTool::Create(quint32 id, VPiece newPiece, QString &width, VMainGraphicsScene *scene,
                                               VAbstractPattern *doc, VContainer *data, const Document &parse,
                                               const Source &typeCreation, const QString &blockName)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        data->AddVariable(currentSeamAllowance, new CustomVariable(data, currentSeamAllowance, 0, newPiece.GetSAWidth(),
                                                               width, true, tr("Current seam allowance")));
        id = data->AddPiece(newPiece);
    }
    else
    {
        const qreal calcWidth = CheckFormula(id, width, data);
        newPiece.setSeamAllowanceWidthFormula(width, calcWidth);

        data->AddVariable(currentSeamAllowance, new CustomVariable(data, currentSeamAllowance, 0, calcWidth,
                                                               width, true, tr("Current seam allowance")));

        data->UpdatePiece(id, newPiece);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    PatternPieceTool *patternPiece = nullptr;
    if (parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(id, Tool::Piece, doc);
        patternPiece = new PatternPieceTool(doc, data, id, typeCreation, scene, blockName);
        scene->addItem(patternPiece);
        connect(patternPiece, &PatternPieceTool::chosenTool,           scene,        &VMainGraphicsScene::chosenItem);
        connect(scene, &VMainGraphicsScene::EnableDetailItemHover,     patternPiece, &PatternPieceTool::AllowHover);
        connect(scene, &VMainGraphicsScene::EnableDetailItemSelection, patternPiece, &PatternPieceTool::AllowSelecting);
        connect(scene, &VMainGraphicsScene::highlightPiece,            patternPiece, &PatternPieceTool::highlight);
        connect(scene, &VMainGraphicsScene::pieceLockedChanged,        patternPiece, &PatternPieceTool::pieceLockedChanged);

        VAbstractPattern::AddTool(id, patternPiece);
        patternPiece->RefreshGeometry(); // Refresh internal paths
    }
    //Very important to delete it. Only this tool need this special variable.
    data->RemoveVariable(currentSeamAllowance);
    return patternPiece;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::Remove(bool ask)
{
    try
    {
        deleteTool(ask);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error);
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::insertNodes(const QVector<VPieceNode> &nodes, quint32 pieceId, VMainGraphicsScene *scene,
                                    VContainer *data, VAbstractPattern *doc)
{
    SCASSERT(scene != nullptr)
    SCASSERT(data != nullptr)
    SCASSERT(doc != nullptr)

    if (pieceId > NULL_ID && !nodes.isEmpty())
    {
        VPiece oldPiece;
        try
        {
            oldPiece = data->GetPiece(pieceId);
        }
        catch (const VExceptionBadId &)
        {
            return;
        }

        QVector<VPieceNode> newNodes = removeDuplicateNodePoints(oldPiece, nodes, data);

        VPiece newPiece = oldPiece;
        for (auto node : newNodes)
        {
            const quint32 id = PrepareNode(node, scene, doc, data);
                if (id == NULL_ID)
                {
                return;
            }

            node.SetId(id);
            newPiece.GetPath().Append(node);

            // Seam allowance tool already initializated and can't init the node
            PatternPieceTool *patternPiece = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(pieceId));
            SCASSERT(patternPiece != nullptr);

            initializeNode(node, scene, data, doc, patternPiece);
        }

        SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, data, pieceId);
        qApp->getUndoStack()->push(saveCommand);// First push then make a connect

        data->UpdatePiece(pieceId, newPiece);// Update piece because first save will not call lite update
        connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece,
                                     const QHash<QString, QString> &nameToIdToken)
{
    SCASSERT(doc != nullptr);

    doc->SetAttribute(domElement, VDomDocument::AttrId, id);
    doc->SetAttribute(domElement, AttrName,             piece.GetName());
    doc->SetAttribute(domElement, AttrPieceColor,       piece.getColor());
    doc->SetAttribute(domElement, AttrPieceFill,        piece.getFill());
    doc->SetAttribute(domElement, AttrPieceLocked,      piece.isLocked());
    doc->SetAttribute(domElement, AttrVersion,          QString().setNum(pieceVersion));
    doc->SetAttribute(domElement, AttrMx,               qApp->fromPixel(piece.GetMx()));
    doc->SetAttribute(domElement, AttrMy,               qApp->fromPixel(piece.GetMy()));
    doc->SetAttribute(domElement, AttrInLayout,         piece.isInLayout());
    doc->SetAttribute(domElement, AttrForbidFlipping,   piece.IsForbidFlipping());
    doc->SetAttribute(domElement, AttrSeamAllowance,    piece.hasSeamAllowance());
    doc->SetAttribute(domElement, AttrHideSeamLine,     piece.isHideSeamLine());

    const bool saBuiltIn = piece.hasSeamAllowanceBuiltIn();
    if (saBuiltIn)
    {
        doc->SetAttribute(domElement, AttrSeamAllowanceBuiltIn, saBuiltIn);
    }
    else
    { // For backward compatebility.
        domElement.removeAttribute(AttrSeamAllowanceBuiltIn);
    }

    doc->SetAttribute(domElement, VAbstractPattern::AttrWidth,
                      formulaNamesToIds(piece.getSeamAllowanceWidthFormula(), nameToIdToken));
    doc->SetAttribute(domElement, AttrUnited, piece.IsUnited());
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, const CustomSARecord &record)
{
    QDomElement recordNode = doc->createElement(PatternPieceTool::TagRecord);

    doc->SetAttribute(recordNode, VAbstractPattern::AttrStart,       record.startPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrPath,        record.path);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrEnd,         record.endPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrNodeReverse, record.reverse);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrIncludeAs,   static_cast<unsigned int>(record.includeType));

    domElement.appendChild(recordNode);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addCSARecords(VAbstractPattern *doc, QDomElement &domElement,
                                       const QVector<CustomSARecord> &records)
{
    if (records.size() > 0)
    {
        QDomElement csaRecordsElement = doc->createElement(PatternPieceTool::TagCSA);
        for (int i = 0; i < records.size(); ++i)
        {
            AddCSARecord(doc, csaRecordsElement, records.at(i));
        }
        domElement.appendChild(csaRecordsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths)
{
    if (paths.size() > 0)
    {
        QDomElement iPathsElement = doc->createElement(PatternPieceTool::TagIPaths);
        for (int i = 0; i < paths.size(); ++i)
        {
            QDomElement recordNode = doc->createElement(PatternPieceTool::TagRecord);
            doc->SetAttribute(recordNode, VAbstractPattern::AttrPath, paths.at(i));
            iPathsElement.appendChild(recordNode);
        }
        domElement.appendChild(iPathsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addAnchors(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &anchors)
{
    if (anchors.size() > 0)
    {
        QDomElement anchorsElement = doc->createElement(PatternPieceTool::TagAnchors);
        for (int i = 0; i < anchors.size(); ++i)
        {
            QDomElement recordNode = doc->createElement(PatternPieceTool::TagRecord);
            recordNode.appendChild(doc->createTextNode(QString().setNum(anchors.at(i))));
            anchorsElement.appendChild(recordNode);
        }
        domElement.appendChild(anchorsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addPieceLabel(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                                     const QHash<QString, QString> &nameToIdToken)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagData);
    const VPieceLabelData &data = piece.GetPatternPieceData();
    doc->SetAttribute(domData, VAbstractPattern::AttrLetter,       data.GetLetter());
    doc->SetAttribute(domData, VAbstractPattern::AttrAnnotation,   data.GetAnnotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrOrientation,  data.GetOrientation());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotationWay,  data.getRotationWay());
    doc->SetAttribute(domData, VAbstractPattern::AttrTilt,         data.GetTilt());
    doc->SetAttribute(domData, VAbstractPattern::AttrFoldPosition, data.GetFoldPosition());
    doc->SetAttribute(domData, VAbstractPattern::AttrQuantity,     data.GetQuantity());
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,      data.IsVisible());
    doc->SetAttribute(domData, VAbstractPattern::AttrOnFold,       data.IsOnFold());
    doc->SetAttribute(domData, AttrMx,                             data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                             data.GetPos().y());
    doc->SetAttribute(domData, VAbstractPattern::AttrWidth,
                      formulaNamesToIds(data.GetLabelWidth(), nameToIdToken));
    doc->SetAttribute(domData, AttrHeight,
                      formulaNamesToIds(data.GetLabelHeight(), nameToIdToken));
    doc->SetAttribute(domData, AttrFont,                           data.getFontSize());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation,
                      formulaNamesToIds(data.getRotation(), nameToIdToken));

    if (data.centerAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterAnchor, data.centerAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrCenterAnchor);
    }

    if (data.topLeftAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopLeftAnchor, data.topLeftAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrTopLeftAnchor);
    }

    if (data.bottomRightAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomRightAnchor, data.bottomRightAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrBottomRightAnchor);
    }

    doc->SetLabelTemplate(domData, data.GetLabelTemplate());

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addPatternLabel(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                                       const QHash<QString, QString> &nameToIdToken)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagPatternInfo);
    const VPatternLabelData &data = piece.GetPatternInfo();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,  data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                         data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                         data.GetPos().y());
    doc->SetAttribute(domData, VAbstractPattern::AttrWidth,
                      formulaNamesToIds(data.GetLabelWidth(), nameToIdToken));
    doc->SetAttribute(domData, AttrHeight,
                      formulaNamesToIds(data.GetLabelHeight(), nameToIdToken));
    doc->SetAttribute(domData, AttrFont,                       data.getFontSize());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation,
                      formulaNamesToIds(data.getRotation(), nameToIdToken));

    if (data.centerAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterAnchor, data.centerAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrCenterAnchor);
    }

    if (data.topLeftAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopLeftAnchor, data.topLeftAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrTopLeftAnchor);
    }

    if (data.bottomRightAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomRightAnchor, data.bottomRightAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrBottomRightAnchor);
    }

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::addGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                                    const QHash<QString, QString> &nameToIdToken)
{
    // grainline
    QDomElement domData = doc->createElement(VAbstractPattern::TagGrainline);
    const VGrainlineData &data = piece.GetGrainlineGeometry();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,     data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                            data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                            data.GetPos().y());
    doc->SetAttribute(domData, AttrLength,
                      formulaNamesToIds(data.getLength(), nameToIdToken));
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation,
                      formulaNamesToIds(data.getRotation(), nameToIdToken));
    doc->SetAttribute(domData, VAbstractPattern::AttrArrows,      int(data.getArrowType()));
    doc->SetAttribute(domData, VAbstractPattern::AttrArrowLength,
                      formulaNamesToIds(data.getArrowLength(), nameToIdToken));

    if (data.centerAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterAnchor, data.centerAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrCenterAnchor);
    }

    if (data.topAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopAnchorPoint, data.topAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrTopAnchorPoint);
    }

    if (data.bottomAnchorPoint() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomAnchorPoint, data.bottomAnchorPoint());
    }
    else
    {
        domData.removeAttribute(AttrBottomAnchorPoint);
    }

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
QString PatternPieceTool::getTagName() const
{
    return VAbstractPattern::TagPiece;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object);
    Q_UNUSED(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::FullUpdateFromFile()
{
    updateExcludeState();
    RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::EnableToolMove(bool move)
{
    setFlag(QGraphicsItem::ItemIsMovable, move);
    m_grainLine->setFlag(QGraphicsItem::ItemIsMovable, move);
    m_dataLabel->setFlag(QGraphicsItem::ItemIsMovable, move);
    m_patternInfo->setFlag(QGraphicsItem::ItemIsMovable, move);

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    for (int i = 0; i< piece.GetPath().nodeCount(); ++i)
    {
        const VPieceNode &node = piece.GetPath().at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            VNodePoint *tool = qobject_cast<VNodePoint*>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            tool->EnableToolMove(move);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::pieceLockedChanged(quint32 id, bool lock)
{
    if (m_id == id)
    {
        setFlag(QGraphicsItem::ItemIsMovable, lock);
        m_grainLine->setFlag(QGraphicsItem::ItemIsMovable, lock);
        m_dataLabel->setFlag(QGraphicsItem::ItemIsMovable, lock);
        m_patternInfo->setFlag(QGraphicsItem::ItemIsMovable, lock);

        const VPiece piece = VAbstractTool::data.GetPiece(m_id);
        for (int i = 0; i< piece.GetPath().nodeCount(); ++i)
        {
            const VPieceNode &node = piece.GetPath().at(i);
            if (node.GetTypeTool() == Tool::NodePoint)
            {
                VNodePoint *tool = qobject_cast<VNodePoint*>(VAbstractPattern::getTool(node.GetId()));
                SCASSERT(tool != nullptr);

                tool->EnableToolMove(lock);
            }
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::resetChildren(QGraphicsItem *pItem)
{
    const bool selected = isSelected();
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    VTextGraphicsItem *pVGI = qgraphicsitem_cast<VTextGraphicsItem*>(pItem);
    if (pVGI != m_dataLabel)
    {
        if (piece.GetPatternPieceData().IsVisible())
        {
            m_dataLabel->reset();
        }
    }
    if (pVGI != m_patternInfo)
    {
        if (piece.GetPatternInfo().IsVisible())
        {
            m_patternInfo->reset();
        }
    }
    VGrainlineItem *pGLI = qgraphicsitem_cast<VGrainlineItem*>(pItem);
    if (pGLI != m_grainLine)
    {
        if (piece.GetGrainlineGeometry().IsVisible())
        {
            m_grainLine->reset();
        }
    }

    setSelected(selected);
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::retranslateUi()
{
    updatePieceLabel();
    updatePatternLabel();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::highlight(quint32 id)
{
    setSelected(m_id == id);
    RefreshGeometry();
}

void PatternPieceTool::updatePieceDetails()
{
    updatePieceLabel();
    updatePatternLabel();
    updateGrainline();
    updateInternalPaths();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief UpdateLabel updates the text label, making it just big enough for the text to fit it
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updatePieceLabel()
{

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    qDebug() << "Update Piece label: " << piece.GetName();
    const VPieceLabelData &labelData = piece.GetPatternPieceData();

    if (labelData.IsVisible() & qApp->Settings()->showLabels())
    {
        QPointF pos;
        qreal labelAngle = 0;

        if (PrepareLabelData(labelData, m_dataLabel, pos, labelAngle))
        {
            m_dataLabel->updateData(piece.GetName(), labelData);
            UpdateLabelItem(m_dataLabel, pos, labelAngle);
        }
    }
    else
    {
        m_dataLabel->hide();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief updatePatternLabel updates the pattern info label
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updatePatternLabel()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    qDebug() << "Update Pattern label: " << piece.GetName();
    const VPatternLabelData &data = piece.GetPatternInfo();

    if (data.IsVisible() & qApp->Settings()->showLabels())
    {
        QPointF pos;
        qreal labelAngle = 0;

        if (PrepareLabelData(data, m_patternInfo, pos, labelAngle))
        {
            m_patternInfo->updateData(doc);
            UpdateLabelItem(m_patternInfo, pos, labelAngle);
        }
    }
    else
    {
        m_patternInfo->hide();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief updateGrainline updates the grain line item
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updateGrainline()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    const VGrainlineData &data = piece.GetGrainlineGeometry();

    qDebug() << "Update Grainline IsVisible() = " << data.IsVisible();

    if (data.IsVisible() & qApp->Settings()->showGrainlines())
    {
        QPointF pos;
        qreal rotation = 0;
        qreal length = 0;
        qreal arrowLength = 0;

        const VGrainlineItem::MoveTypes type = findGrainlineGeometry(data, length, rotation, arrowLength, pos);
        if (type & VGrainlineItem::Error)
        {
            m_grainLine->hide();
            return;
        }

        m_grainLine->setMoveType(type);
        m_grainLine->updateGeometry(pos, rotation, ToPixel(length, *VDataTool::data.GetPatternUnit()),
                                    data.getArrowType(), ToPixel(arrowLength, *VDataTool::data.GetPatternUnit()));
        m_grainLine->show();
    }
    else
    {
        m_grainLine->hide();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief saveMovePiece saves the move piece operation to the undo stack
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::saveMovePiece(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetPos(ptPos);

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    moveCommand->setText(tr("move pattern piece label"));
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief saveResizePiece saves the resize piece label operation to the undo stack
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::saveResizePiece(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelWidth(QString().setNum(dLabelW));
    const qreal height = FromPixel(m_dataLabel->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternPieceData().SetFontSize(iFontSize);

    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    resizeCommand->setText(tr("resize pattern piece label"));
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief savePieceRotation saves the rotation piece label operation to the undo stack
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::savePieceRotation(qreal dRot)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetPos(m_dataLabel->pos());
    newPiece.GetPatternPieceData().SetFontSize(m_dataLabel->getFontSize());

    // Tranform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newPiece.GetPatternPieceData().SetRotation(QString().setNum(line.angle()));

    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    rotateCommand->setText(tr("rotate pattern piece label"));
    qApp->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SaveMovePattern saves the pattern label position
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveMovePattern(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetPos(ptPos);

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    moveCommand->setText(tr("move pattern info label"));
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief: SaveResizePattern saves the pattern label width and font size
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveResizePattern(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelWidth(QString().setNum(dLabelW));
    qreal height = FromPixel(m_patternInfo->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternInfo().SetFontSize(iFontSize);

    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    resizeCommand->setText(tr("resize pattern info label"));
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveRotationPattern(qreal dRot)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    newPiece.GetPatternInfo().SetPos(m_patternInfo->pos());
    newPiece.GetPatternInfo().SetFontSize(m_patternInfo->getFontSize());

    // Tranform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newPiece.GetPatternInfo().SetRotation(QString().setNum(line.angle()));

    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    rotateCommand->setText(tr("rotate pattern info label"));
    qApp->getUndoStack()->push(rotateCommand);
}


//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveMoveGrainline(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    qDebug() << "******* new grainline pos" << ptPos;

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    moveCommand->setText(tr("move grainline"));
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveResizeGrainline(qreal dLength)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLength = FromPixel(dLength, *VDataTool::data.GetPatternUnit());
    newPiece.GetGrainlineGeometry().SetPos(m_grainLine->pos());
    newPiece.GetGrainlineGeometry().setLength(QString().setNum(dLength));
    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    resizeCommand->setText(tr("resize grainline"));
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveRotateGrainline(qreal dRot, const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    newPiece.GetGrainlineGeometry().setRotation(QString().setNum(qRadiansToDegrees(dRot)));
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    rotateCommand->setText(tr("rotate grainline"));
    qApp->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VToolDetail::paint draws a bounding box around piece, if one of its text or grainline items is not idle.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor  color;
    QString lineType;
    qreal   lineWeight;

    //set cutline pen
    color      = QColor(qApp->Settings()->getDefaultCutColor());
    lineType   = qApp->Settings()->getDefaultCutLinetype();
    lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);

    m_cutLine->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                           lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));
    m_cutLine->setZValue(-10);

    //set seamline pen
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);

    if (qApp->Settings()->showSeamAllowances())
    {
        if (piece.hasSeamAllowance() && !piece.hasSeamAllowanceBuiltIn())
        {
            color      = QColor(qApp->Settings()->getDefaultSeamColor());
            lineType   = qApp->Settings()->getDefaultSeamLinetype();
            lineWeight = ToPixel(qApp->Settings()->getDefaultSeamLineweight(), Unit::Mm);
        }
        else
        {
            color      = QColor(qApp->Settings()->getDefaultCutColor());
            lineType   = qApp->Settings()->getDefaultCutLinetype();
            lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);
        }

        this->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                                lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));

        QBrush brush = QBrush(QColor(piece.getColor()));

        //set pattern piece color & brush style
        int index = fills().indexOf(piece.getFill());
        brush.setStyle(static_cast<Qt::BrushStyle>(index));
        brush.setTransform(brush.transform().scale(150.0, 150.0));
        brush.setTransform(painter->combinedTransform().inverted());
        this->setBrush(brush);
    }

    //set allowance brush
    m_allowanceFill->setPen(Qt::NoPen);

    //set notches pen
    color      = QColor(qApp->Settings()->getDefaultNotchColor());
    lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);

    m_notches->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                           Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));


    if ((m_dataLabel->isIdle() == false
            || m_patternInfo->isIdle() == false
            || m_grainLine->isIdle() == false) && not isSelected())
    {
        setSelected(true);
    }
    QGraphicsPathItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
QRectF PatternPieceTool::boundingRect() const
{
    if (m_pieceRect.isNull())
    {
        return QGraphicsPathItem::boundingRect();
    }
    else
    {
        return m_pieceRect;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath PatternPieceTool::shape() const
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    if (m_mainPath == QPainterPath() && m_cutPath == QPainterPath())
    {
        return QGraphicsPathItem::shape();
    }
    else if (piece.isHideSeamLine())
    {
        return itemShapeFromPath(m_cutPath, pen());
    }
    else
    {
        return itemShapeFromPath(m_mainPath + m_cutPath, pen());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::AddToFile()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    const QHash<QString, QString> nameToIdToken = nameToIdTokenMap(&(VAbstractTool::data));

    QDomElement domElement = doc->createElement(getTagName());

    addAttributes(doc, domElement, m_id, piece, nameToIdToken);
    addPieceLabel(doc, domElement, piece, nameToIdToken);
    addPatternLabel(doc, domElement, piece, nameToIdToken);
    addGrainline(doc, domElement, piece, nameToIdToken);

    // nodes
    addNodes(doc, domElement, piece, nameToIdToken);
    //custom seam allowance
    addCSARecords(doc, domElement, piece.getCustomSARecords());
    addInternalPaths(doc, domElement, piece.getInternalPaths());
    addAnchors(doc, domElement, piece.getAnchors());

    AddPiece *cmd = new AddPiece(domElement, doc, piece, m_blockName);
    connect(cmd, &AddPiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(cmd);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::RefreshDataInFile()
{
    QDomElement domElement = doc->elementById(m_id, getTagName());
    if (domElement.isElement())
    {
        // Refresh only parts that we possibly need to update
        {
            // TODO. Delete if minimal supported version is 0.4.0
            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                              "Time to refactor the code.");

            const uint version = doc->GetParametrUInt(domElement, PatternPieceTool::AttrVersion, "1");
            if (version == 1)
            {
                const VPiece piece = VAbstractTool::data.GetPiece(m_id);
                const QHash<QString, QString> nameToIdToken =
                    nameToIdTokenMap(&(VAbstractTool::data));

                doc->SetAttribute(domElement, AttrVersion, QString().setNum(pieceVersion));

                doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
                addPieceLabel(doc, domElement, piece, nameToIdToken);
                addPatternLabel(doc, domElement, piece, nameToIdToken);
                addGrainline(doc, domElement, piece, nameToIdToken);
                addNodes(doc, domElement, piece, nameToIdToken);
                addCSARecords(doc, domElement, piece.getCustomSARecords());
                addInternalPaths(doc, domElement, piece.getInternalPaths());
                addAnchors(doc, domElement, piece.getAnchors());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVariant PatternPieceTool::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
            changeFinished = false;

            // value - this is new position.
            const QPointF newPos = value.toPointF();

            MovePiece *cmd = new MovePiece(doc, newPos.x(), newPos.y(), m_id, scene());
            //connect(moveDet, &MovePiece::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            qApp->getUndoStack()->push(cmd);

            const QList<QGraphicsView *> viewList = scene()->views();
            if (!viewList.isEmpty())
            {
                if (VMainGraphicsView *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                {
                    const qreal scale = sceneScale(scene());
                    const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(view);
                    const QRectF itemRect = mapToScene(boundingRect()|childrenBoundingRect()).boundingRect();

                    // Ensure visible only small rect around a cursor
                    VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                    SCASSERT(currentScene);
                    const QPointF cursorPosition = currentScene->getScenePos();
                    view->ensureVisible(QRectF(cursorPosition.x()-5/scale, cursorPosition.y()-5/scale,
                                               10/scale, 10/scale));
                }
            }

            // Don't forget to update geometry, because first change never call full parse
            VPiece piece = VAbstractTool::data.GetPiece(m_id);
            piece.SetMx(newPos.x());
            piece.SetMy(newPos.y());
            VAbstractTool::data.UpdatePiece(m_id, piece);

            RefreshGeometry();

            changeFinished = true;
        }
    }

    if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (value == true)
        {
            // do stuff if selected
            this->setFocus();
        }
        else
        {
            // do stuff if not selected
        }
    }

    return QGraphicsPathItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsPathItem::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (!(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        }
    }

    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        doc->selectedPiece(m_id);
        emit chosenTool(m_id, SceneObject::Piece);
    }

    event->accept();// Special for not selectable item first need to call standard mousePressEvent then accept event
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    QGraphicsPathItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Disable cursor-arrow-openhand
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }
    QGraphicsPathItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_suppressContextMenu)
    {
        return;
    }

    VPiece piece = VAbstractTool::data.GetPiece(m_id);
    bool lock = !piece.isLocked();

    QMenu menu;
    QAction *editProperties = menu.addAction(QIcon::fromTheme("preferences-other"), tr("Properties") + "\tP");
    editProperties->setEnabled(lock);

    QAction *actionLockPiece = menu.addAction(tr("Lock Pattern Piece") + "\tCtrl + L");
    actionLockPiece->setCheckable(true);
    actionLockPiece->setChecked(piece.isLocked());

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);

    QAction *inLayoutOption = menu.addAction(tr("Include in Layout") + "\tI");
    inLayoutOption->setCheckable(true);
    inLayoutOption->setChecked(piece.isInLayout());
    inLayoutOption->setEnabled(lock);

    QAction *forbidFlipping = menu.addAction(tr("Forbid Flipping") + "\tF");
    forbidFlipping->setCheckable(true);
    forbidFlipping->setChecked(piece.IsForbidFlipping());
    forbidFlipping->setEnabled(lock);

    separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);

    QAction *raiseToTop = menu.addAction(tr("Raise to top") + "\tCtrl + Home");
    QAction *lowerToBottom = menu.addAction(tr("Lower to bottom") + "\tCtrl + End");

    separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);

    QAction *hideMainPath = menu.addAction(tr("Hide Seam Line") + "\t|");
    hideMainPath->setCheckable(true);
    hideMainPath->setChecked(piece.isHideSeamLine());
    hideMainPath->setEnabled(piece.hasSeamAllowance() && qApp->Settings()->showSeamAllowances());

    QAction *showSeamAllowance = menu.addAction(QIcon("://icon/32x32/seam_allowance.png"),
                                                tr("Show Seam Allowance") + "\tS");
    showSeamAllowance->setCheckable(true);
    showSeamAllowance->setChecked(piece.hasSeamAllowance());

    QAction *showGrainline = menu.addAction(QIcon("://icon/32x32/grainline.png"), tr("Show Grainline") + "\tG");
    showGrainline->setCheckable(true);
    qDebug() << "Grainline IsVisible() = " << piece.GetGrainlineGeometry().IsVisible();
    showGrainline->setChecked(piece.GetGrainlineGeometry().IsVisible());

    QAction *showPatternLabel = menu.addAction(QIcon("://icon/32x32/pattern_label.png"), tr("Show Pattern Label") + "\t[");
    showPatternLabel->setCheckable(true);
    showPatternLabel->setChecked(piece.GetPatternInfo().IsVisible());

    QAction *showPieceLabel = menu.addAction(QIcon("://icon/32x32/piece_label.png"), tr("Show Piece Label") + "\t]");
    showPieceLabel->setCheckable(true);
    showPieceLabel->setChecked(piece.GetPatternPieceData().IsVisible());

    separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);

    QAction *rename = menu.addAction(tr("Rename...") + "\tF2");
    rename->setEnabled(lock);

    QAction *deletePiece = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete") + "\tDel");
    _referens > 0 ? deletePiece->setEnabled(false) : deletePiece->setEnabled(true);
    deletePiece->setEnabled(lock);

    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == editProperties)
    {
        editPieceProperties();
    }
    else if (selectedAction == actionLockPiece)
    {
        togglePieceLock(selectedAction->isChecked());
    }
    else if (selectedAction == inLayoutOption)
    {
        toggleInLayout(selectedAction->isChecked());
    }
    else if (selectedAction == forbidFlipping)
    {
        toggleFlipping(selectedAction->isChecked());
    }
    else if (selectedAction == raiseToTop)
    {
        raiseItemToTop(focusItem());
    }
    else if (selectedAction == lowerToBottom)
    {
        lowerItemToBottom(focusItem());
    }
    else if (selectedAction == hideMainPath)
    {
        toggleSeamLine(selectedAction->isChecked());
    }
    else if (selectedAction == showSeamAllowance)
    {
        toggleSeamAllowance(selectedAction->isChecked());
    }
    else if (selectedAction == showGrainline)
    {
        toggleGrainline(selectedAction->isChecked());
    }
    else if (selectedAction == showPatternLabel)
    {
        togglePatternLabel(selectedAction->isChecked());
    }
    else if (selectedAction == showPieceLabel)
    {
        togglePieceLabel(selectedAction->isChecked());
    }
    else if (selectedAction == rename)
    {
        renamePiece(piece);
    }
    else if (selectedAction == deletePiece)
    {
        try
        {
            deleteTool();
        }
        catch(const VExceptionToolWasDeleted &error)
        {
            Q_UNUSED(error);
            return;//Leave this method immediately!!!
        }
        return; //Leave this method immediately after call!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::keyReleaseEvent(QKeyEvent *event)
{
    VPiece piece = VAbstractTool::data.GetPiece(m_id);
    switch (event->key())
    {
        case Qt::Key_Delete:
            if (!piece.isLocked())
            {
                try
                {
                    deleteTool();
                }
                catch(const VExceptionToolWasDeleted &error)
                {
                    Q_UNUSED(error);
                    return;//Leave this method immediately!!!
                }
            }
            break;

        case Qt::Key_P:
            {
                if (!piece.isLocked())
                {
                    editPieceProperties();
                }
                break;
            }

        case Qt::Key_L:
            {
                if (event->modifiers() & Qt::ControlModifier)
                {
                    togglePieceLock(!piece.isLocked());
                }
                break;
            }

        case Qt::Key_I:
            {
                if (!piece.isLocked())
                {
                    toggleInLayout(!piece.isInLayout());
                }
                break;
            }

        case Qt::Key_F:
            {
                if (!piece.isLocked())
                {
                    toggleFlipping(!piece.IsForbidFlipping());
                }
                break;
            }

        case Qt::Key_Bar:
            {
                if (piece.hasSeamAllowance() && qApp->Settings()->showSeamAllowances())
                {
                    toggleSeamLine(!piece.isHideSeamLine()); //Seam line is only valid if there is a seam allowance
                }
                break;
            }

        case Qt::Key_S:
            {
                if (event->modifiers() & Qt::ControlModifier)
                {
                    break;
                }
                toggleSeamAllowance(!piece.hasSeamAllowance());
                break;
            }

        case Qt::Key_G:
            {
                toggleGrainline(!piece.GetGrainlineGeometry().IsVisible());
                break;
            }

        case Qt::Key_BracketLeft:
            {
                togglePatternLabel(!piece.GetPatternInfo().IsVisible());
                break;
            }

        case Qt::Key_BracketRight:
            {
                togglePieceLabel(!piece.GetPatternPieceData().IsVisible());
                break;
            }

        case Qt::Key_F2:
            {
                if (!piece.isLocked())
                {
                    renamePiece(piece);
                }
                break;
            }
        case Qt::Key_Home:
            {
                if (event->modifiers() & Qt::ControlModifier)
                {
                    raiseItemToTop(focusItem());
                }
                break;
            }
        case Qt::Key_End:
            {
                if (event->modifiers() & Qt::ControlModifier)
                {
                    lowerItemToBottom(focusItem());
                }
                break;
            }
        default:
            break;
    }

    QGraphicsPathItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SetDialog()
{
    SCASSERT(!m_dialog.isNull());
    QSharedPointer<PatternPieceDialog> dialogTool = m_dialog.objectCast<PatternPieceDialog>();
    SCASSERT(!dialogTool.isNull())
    dialogTool->SetPiece(VAbstractTool::data.GetPiece(m_id));
    dialogTool->enableApply(true);
}

//---------------------------------------------------------------------------------------------------------------------
PatternPieceTool::PatternPieceTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                       const Source &typeCreation, VMainGraphicsScene *scene,
                                       const QString &blockName, QGraphicsItem *parent)
    : VInteractiveTool(doc, data, id)
    , QGraphicsPathItem(parent)
    , m_mainPath()
    , m_pieceRect()
    , m_cutPath()
    , m_pieceScene(scene)
    , m_blockName(blockName)
    , m_cutLine(new NonScalingFillPathItem(this))
    , m_allowanceFill(new NonScalingFillPathItem(this))
    , m_dataLabel(new VTextGraphicsItem(this))
    , m_patternInfo(new VTextGraphicsItem(this))
    , m_grainLine(new VGrainlineItem(this))
    , m_notches(new QGraphicsPathItem(this))
{
    VPiece piece = data->GetPiece(id);
    initializeNodes(piece, scene);
    InitCSAPaths(piece);
    InitInternalPaths(piece);
    initializeAnchorPoints(piece);
    EnableToolMove(!piece.isLocked());
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    RefreshGeometry();

    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    connect(scene, &VMainGraphicsScene::EnableToolMove, this, &PatternPieceTool::EnableToolMove);
    connect(scene, &VMainGraphicsScene::ItemClicked,    this, &PatternPieceTool::resetChildren);
    ToolCreation(typeCreation);
    setAcceptHoverEvents(true);

    connect(m_dataLabel, &VTextGraphicsItem::itemMoved,   this, &PatternPieceTool::saveMovePiece);
    connect(m_dataLabel, &VTextGraphicsItem::itemResized, this, &PatternPieceTool::saveResizePiece);
    connect(m_dataLabel, &VTextGraphicsItem::itemRotated, this, &PatternPieceTool::savePieceRotation);

    connect(m_patternInfo, &VTextGraphicsItem::itemMoved,   this, &PatternPieceTool::SaveMovePattern);
    connect(m_patternInfo, &VTextGraphicsItem::itemResized, this, &PatternPieceTool::SaveResizePattern);
    connect(m_patternInfo, &VTextGraphicsItem::itemRotated, this, &PatternPieceTool::SaveRotationPattern);

    connect(m_grainLine, &VGrainlineItem::itemMoved,   this, &PatternPieceTool::SaveMoveGrainline);
    connect(m_grainLine, &VGrainlineItem::itemResized, this, &PatternPieceTool::SaveResizeGrainline);
    connect(m_grainLine, &VGrainlineItem::itemRotated, this, &PatternPieceTool::SaveRotateGrainline);

    connect(doc, &VAbstractPattern::updatePatternLabel, this, &PatternPieceTool::updatePatternLabel);
    connect(doc, &VAbstractPattern::patternParsed,      this, &PatternPieceTool::updatePieceDetails);

    connect(m_pieceScene, &VMainGraphicsScene::DimensionsChanged, this, &PatternPieceTool::updatePieceDetails);
    connect(m_pieceScene, &VMainGraphicsScene::LanguageChanged,   this, &PatternPieceTool::retranslateUi);

    updatePieceDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updateExcludeState()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    for (int i = 0; i< piece.GetPath().nodeCount(); ++i)
    {
        const VPieceNode &node = piece.GetPath().at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            VNodePoint *tool = qobject_cast<VNodePoint*>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            tool->SetExluded(node.isExcluded());
            tool->setVisible(!node.isExcluded());//Hide excluded point
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updateInternalPaths()
{
    VPiece piece = VAbstractTool::data.GetPiece(m_id);
    const QVector<quint32> paths = piece.getInternalPaths();
    for (auto path : paths)
    {
        try
        {
            if (InternalPathTool *tool = qobject_cast<InternalPathTool *>(VAbstractPattern::getTool(path)))
            {
                tool->refreshGeometry();
            }
        }
        catch (const VExceptionBadId &)
        {
            // do nothing
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::updatePiece(const VPiece &piece)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    VAbstractTool::data.UpdatePiece(m_id, piece);
    RefreshGeometry();
    VMainGraphicsView::NewSceneRect(m_pieceScene, qApp->getSceneView(), this);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::RefreshGeometry()
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    m_cutLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);

    QString pieceColor = piece.getColor();

    //set pattern piece color & brush style
    int index = fills().indexOf(piece.getFill());
    QBrush newBrush = QBrush(QColor(pieceColor), static_cast<Qt::BrushStyle>(index));
    this->setBrush(newBrush);

    QPainterPath path = piece.mainPath(this->getData());

    if (!piece.isHideSeamLine() || !piece.hasSeamAllowance() || piece.hasSeamAllowanceBuiltIn())
    {
        m_mainPath = QPainterPath();
        m_allowanceFill->setBrush(QBrush(QColor(qApp->Settings()->getDefaultCutColor()), Qt::Dense7Pattern));
    }
    else
    {
        m_mainPath = path; // need for returning a bounding rect when main path is not visible
        path = QPainterPath();
        m_allowanceFill->setBrush(QBrush(Qt::NoBrush)); // Disable if the main path was hidden
    }

    this->setPath(path);

    QVector<QPointF> seamAllowancePoints;

    if (piece.hasSeamAllowance())
    {
        seamAllowancePoints = piece.seamAllowancePoints(this->getData());
    }

    if (piece.hasSeamAllowance() && !piece.hasSeamAllowanceBuiltIn() && qApp->Settings()->showSeamAllowances())
    {
        m_cutPath = piece.seamAllowancePath(seamAllowancePoints);
        m_cutLine->setPath(m_cutPath);
        m_pieceRect = m_cutLine->boundingRect();

        QPainterPath allowancePath = path;
        allowancePath.addPath(m_cutPath);
        allowancePath.setFillRule(Qt::OddEvenFill);
        m_allowanceFill->setPath(allowancePath);

        if (piece.isHideSeamLine())
        {
            this->setPath(QPainterPath());
        }
    }
    else
    {
        m_cutLine->setPath(QPainterPath());
        m_allowanceFill->setPath(QPainterPath());
        m_pieceRect = path.boundingRect();
    }

    m_notches->setPath(piece.getNotchesPath(this->getData(), seamAllowancePoints));

    updatePieceDetails();

    this->setPos(piece.GetMx(), piece.GetMy());
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveDialogChange()
{
    SCASSERT(!m_dialog.isNull());
    PatternPieceDialog *dialogTool = qobject_cast<PatternPieceDialog*>(m_dialog.data());
    SCASSERT(dialogTool != nullptr);
    const VPiece newPiece = dialogTool->GetPiece();
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);

    SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(saveCommand);

    updatePieceLabel();
}

//******************************************************************************
/// @brief nodeAngleChanged handle seam allowance corner type changes.
///
/// This method handles the signal sent that the Seam Allowance corner type has changed.
///
/// @param id  node id.
/// @param type  Type of Seam Allowance corner type.
///
/// @details
/// -Method loops through the piece nodes looking for the chosen nodem and when found
///  sets the corner type in a new copy of the piece. Calls the SavePieceOptions undo
///  command.
//******************************************************************************
void PatternPieceTool::nodeAngleChanged(quint32 id, PieceNodeAngle type)
{
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    if (!oldPiece.isLocked())
    {
        VPiece newPiece = oldPiece;

        for (int i = 0; i< oldPiece.GetPath().nodeCount(); ++i)
        {
            VPieceNode node = oldPiece.GetPath().at(i);
            if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
            {
                node.SetAngleType(type);
                newPiece.GetPath()[i] = node;

                SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
                undoCommand->setText(tr("Update Node Angle"));
                connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
                qApp->getUndoStack()->push(undoCommand);
                return;
            }
        }
    }
}

//******************************************************************************
/// @brief notchChanged handle notch changes.
///
/// This method handles the signal sent that the Notch properties have changed.
///
/// @param id  node id.
/// @param notchData  struct of the notch data.
///
/// @details
/// -Method loops through the piece nodes looking for the chosen nodem and when found
///  sets the change of the notch properties  in a new copy of the piece. Calls the
///  SavePieceOptions undo command.
//******************************************************************************
void PatternPieceTool::notchChanged(quint32 id, NotchData notchData)
{
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    bool isBuiltInSA = oldPiece.hasSeamAllowanceBuiltIn();
    if (!oldPiece.isLocked())
    {
        VPiece newPiece = oldPiece;

        for (int i = 0; i< oldPiece.GetPath().nodeCount(); ++i)
        {
            VPieceNode node = oldPiece.GetPath().at(i);
            if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
            {
                node.setNotch(notchData.isNotch);
                node.setShowNotch(notchData.showCutline);
                node.setShowSeamlineNotch(notchData.showSeamline && !isBuiltInSA);
                node.setNotchType(notchData.type);
                node.setNotchSubType(notchData.subType);
                node.setNotchLength(notchData.length);
                node.setNotchWidth(notchData.width);
                node.setNotchCount(notchData.count);
                newPiece.GetPath()[i] = node;

                SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
                undoCommand->setText(tr("Update Notch"));
                connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
                qApp->getUndoStack()->push(undoCommand);
                return;
            }
        }
    }
}

//******************************************************************************
/// @brief nodeExcluded handle excluding node.
///
/// This method handles the signal sent that the node should be exculded.
///
/// @param id  node id.
///
/// @details
/// -Method loops through the piece nodes looking for the chosen nodem and when found
///  sets the node to be excluded from the main path in a new copy of the piece. Calls the
///  SavePieceOptions undo command.
//******************************************************************************
void PatternPieceTool::nodeExcluded(quint32 id)
{
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    if (!oldPiece.isLocked())
    {
        VPiece newPiece = oldPiece;

        for (int i = 0; i< oldPiece.GetPath().nodeCount(); ++i)
        {
            VPieceNode node = oldPiece.GetPath().at(i);
            if (node.GetId() == id && node.GetTypeTool() == Tool::NodePoint)
            {
                node.SetExcluded(true);
                newPiece.GetPath()[i] = node;

                SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
                undoCommand->setText(tr("Exclude Node"));
                connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
                qApp->getUndoStack()->push(undoCommand);
                return;
            }
        }
    }
}

//******************************************************************************
/// @brief nodeDeleted handle delete node.
///
/// This method handles the signal sent that the node should be deleted.
///
/// @param id  node id.
///
/// @details
/// -Method removes node form the main path of the piece.
//******************************************************************************
void PatternPieceTool::nodeDeleted(quint32 id)
{
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    if (!oldPiece.isLocked())
    {
        VPiece newPiece = oldPiece;
        VPiecePath path = newPiece.GetPath();
        int index = path.indexOfNode(id);
        QVector<VPieceNode> nodes = path.getNodes();
        VPieceNode node = nodes.at(index);

        if (node.GetTypeTool() == Tool::NodePoint)
        {
            QVector<VPieceNode> newNodes = path.removeNode(id);
            path.setNodes(newNodes);
            newPiece.SetPath(path);

            SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
            undoCommand->setText(tr("Delete Node"));
            connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            qApp->getUndoStack()->push(undoCommand);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::MoveTypes PatternPieceTool::findLabelGeometry(const VPatternLabelData &labelData, qreal &rotationAngle,
                                                            qreal &labelWidth, qreal &labelHeight, QPointF &pos)
{
    qDebug() << "Find label Geometery";
    VPieceItem::MoveTypes restrictions = VPieceItem::AllModifications;
    try
    {
        if (!qmu::QmuTokenParser::IsSingle(labelData.getRotation()))
        {
            restrictions &= ~ VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), labelData.getRotation());
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return VPieceItem::Error;
    }

    const quint32 topLeftAnchorPoint = labelData.topLeftAnchorPoint();
    const quint32 bottomRightAnchorPoint = labelData.bottomRightAnchorPoint();

    if (topLeftAnchorPoint != NULL_ID && bottomRightAnchorPoint != NULL_ID)
    {
        try
        {
            const auto topLeftAnchorPointPoint = VAbstractTool::data.GeometricObject<VPointF>(topLeftAnchorPoint);
            const auto bottomRightAnchorPointPoint = VAbstractTool::data.GeometricObject<VPointF>(bottomRightAnchorPoint);

            const QRectF labelRect = QRectF(static_cast<QPointF>(*topLeftAnchorPointPoint),
                                            static_cast<QPointF>(*bottomRightAnchorPointPoint));
            labelWidth = FromPixel(qAbs(labelRect.width()), *VDataTool::data.GetPatternUnit());
            labelHeight = FromPixel(qAbs(labelRect.height()), *VDataTool::data.GetPatternUnit());

            pos = labelRect.topLeft();

            restrictions &= ~ VPieceItem::IsMovable;
            restrictions &= ~ VPieceItem::IsResizable;

            return restrictions;
        }
        catch(const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        const bool widthIsSingle = qmu::QmuTokenParser::IsSingle(labelData.GetLabelWidth());

        Calculator cal1;
        labelWidth = cal1.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetLabelWidth());

        const bool heightIsSingle = qmu::QmuTokenParser::IsSingle(labelData.GetLabelHeight());

        Calculator cal2;
        labelHeight = cal2.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetLabelHeight());


        if (!widthIsSingle || not heightIsSingle)
        {
            restrictions &= ~ VPieceItem::IsResizable;
        }
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return VPieceItem::Error;
    }

    const quint32 centerAnchor = labelData.centerAnchorPoint();
    if (centerAnchor != NULL_ID)
    {
        try
        {
            const auto centerAnchorPoint = VAbstractTool::data.GeometricObject<VPointF>(centerAnchor);

            const qreal lWidth = ToPixel(labelWidth, *VDataTool::data.GetPatternUnit());
            const qreal lHeight = ToPixel(labelHeight, *VDataTool::data.GetPatternUnit());
            pos = static_cast<QPointF>(*centerAnchorPoint) - QRectF(0, 0, lWidth, lHeight).center();

            restrictions &= ~ VPieceItem::IsMovable;
        }
        catch(const VExceptionBadId &)
        {
            pos = labelData.GetPos();
        }
    }
    else
    {
        pos = labelData.GetPos();
    }

    return restrictions;
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::MoveTypes PatternPieceTool::findGrainlineGeometry(const VGrainlineData &data, qreal &length,
                                                              qreal &rotationAngle, qreal &arrowLength, QPointF &pos)
{
    const quint32 topAnchorPoint = data.topAnchorPoint();
    const quint32 bottomAnchorPoint = data.bottomAnchorPoint();

    if (topAnchorPoint != NULL_ID && bottomAnchorPoint != NULL_ID)
    {
        try
        {
            const auto topAnchor_Point = VAbstractTool::data.GeometricObject<VPointF>(topAnchorPoint);
            const auto bottomAnchor_Point = VAbstractTool::data.GeometricObject<VPointF>(bottomAnchorPoint);

            QLineF grainline(static_cast<QPointF>(*bottomAnchor_Point), static_cast<QPointF>(*topAnchor_Point));
            length = FromPixel(grainline.length(), *VDataTool::data.GetPatternUnit());
            rotationAngle = grainline.angle();

            Calculator cal3;
            arrowLength = cal3.EvalFormula(VAbstractTool::data.DataVariables(), data.getArrowLength());

            // override arrow length formula to ensure that the arrow length does not excede
            // 1/2 the length of the grainline length when using the top and bottom anchor points.
            if (arrowLength > length / 2)
            {
                arrowLength = length / 2.1;
            }

            if (!VFuzzyComparePossibleNulls(rotationAngle, 0))
            {
                grainline.setAngle(0);
            }

            pos = grainline.p1();

            return VPieceItem::NotMovable;
        }
        catch(const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    VPieceItem::MoveTypes restrictions = VPieceItem::AllModifications;
    try
    {
        if (!qmu::QmuTokenParser::IsSingle(data.getRotation()))
        {
            restrictions &= ~ VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), data.getRotation());

        if (!qmu::QmuTokenParser::IsSingle(data.getLength()))
        {
            restrictions &= ~ VPieceItem::IsResizable;
        }

        Calculator cal2;
        length = cal2.EvalFormula(VAbstractTool::data.DataVariables(), data.getLength());

        Calculator cal3;
        arrowLength = cal3.EvalFormula(VAbstractTool::data.DataVariables(), data.getArrowLength());
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return VPieceItem::Error;
    }

    const quint32 centerAnchor = data.centerAnchorPoint();
    if (centerAnchor != NULL_ID)
    {
        try
        {
            const auto centerAnchorPoint = VAbstractTool::data.GeometricObject<VPointF>(centerAnchor);

            const qreal cLength = ToPixel(length, *VDataTool::data.GetPatternUnit());
            QLineF grainline(centerAnchorPoint->x(), centerAnchorPoint->y(),
                             centerAnchorPoint->x() + cLength / 2.0, centerAnchorPoint->y());

            grainline.setAngle(rotationAngle);
            grainline = QLineF(grainline.p2(), grainline.p1());
            grainline.setLength(cLength);

            pos = grainline.p2();
            restrictions &= ~ VPieceItem::IsMovable;
        }
        catch(const VExceptionBadId &)
        {
            pos = data.GetPos();
        }
    }
    else
    {
        pos = data.GetPos();
    }

    return restrictions;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::initializeNodes(const VPiece &piece, VMainGraphicsScene *scene)
{
    for (int i = 0; i< piece.GetPath().nodeCount(); ++i)
    {
        initializeNode(piece.GetPath().at(i), scene, &(VAbstractTool::data), doc, this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::initializeNode(const VPieceNode &node, VMainGraphicsScene *scene, VContainer *data,
                                  VAbstractPattern *doc, PatternPieceTool *parent)
{
    SCASSERT(scene != nullptr)
    SCASSERT(data != nullptr)
    SCASSERT(doc != nullptr)
    SCASSERT(parent != nullptr)

    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
        {
            VNodePoint *tool = qobject_cast<VNodePoint*>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            if (tool->parent() != parent)
            {
                connect(tool, &VNodePoint::chosenTool, scene, &VMainGraphicsScene::chosenItem, Qt::UniqueConnection);
                connect(tool, &VNodePoint::notchChanged, parent, &PatternPieceTool::notchChanged, Qt::UniqueConnection);
                connect(tool, &VNodePoint::nodeAngleChanged, parent,
                        &PatternPieceTool::nodeAngleChanged, Qt::UniqueConnection);
                connect(tool, &VNodePoint::nodeExcluded, parent, &PatternPieceTool::nodeExcluded, Qt::UniqueConnection);
                connect(tool, &VNodePoint::nodeDeleted, parent, &PatternPieceTool::nodeDeleted, Qt::UniqueConnection);
                tool->setParentItem(parent);
                tool->SetParentType(ParentType::Item);
                tool->SetExluded(node.isExcluded());
                doc->IncrementReferens(node.GetId());
            }
            tool->setVisible(!node.isExcluded()); //Hide excluded point
            break;
        }
        case (Tool::NodeArc):
        case (Tool::NodeElArc):
        case (Tool::NodeSpline):
        case (Tool::NodeSplinePath):
            doc->IncrementReferens(data->GetGObject(node.GetId())->getIdTool());
            break;
        default:
            qDebug() << "Get wrong tool type. Ignore.";
            break;
    }
}

/// @brief removeDuplicateNodePoints remove duplcate node points
///
///  This method creates a list of nodes that removes duplicate point nodes from a list of selected modes.
///  Curve nodes are not removed.
///
/// @param piece Pattern piece that selected nodes are to be inserted into.
/// @param nodes List of selected nodes.
/// @param data Pointer to data container.
/// @return uniqueNodes Vector of nodes with unique point nodes.
QVector<VPieceNode> PatternPieceTool::removeDuplicateNodePoints(const VPiece &piece, const QVector<VPieceNode> &nodes,
                                                                VContainer *data)
{
    QVector<quint32> pieceNodeObjIds;
    const QVector<VPieceNode> pieceNodes = piece.GetPath().getNodes();
    for (auto node : pieceNodes)
    {
        quint32 id = node.GetId();
        QSharedPointer<VGObject> object = data->GetGObject(id);
        const quint32 objectId = object->getIdObject();
        pieceNodeObjIds.append(objectId);
    }

    QVector<VPieceNode> uniqueNodes;
    for (auto node : nodes)
    {
        quint32 id = node.GetId();
        if (node.GetTypeTool() != Tool::NodePoint ||
           (!pieceNodeObjIds.contains(id) && node.GetTypeTool() == Tool::NodePoint))
        {
            uniqueNodes.append(node);
        }
    }
    return uniqueNodes;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::InitCSAPaths(const VPiece &piece)
{
    for (int i = 0; i < piece.getCustomSARecords().size(); ++i)
    {
        doc->IncrementReferens(piece.getCustomSARecords().at(i).path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::InitInternalPaths(const VPiece &piece)
{
    const QVector<quint32> pathIds = piece.getInternalPaths();
    for (int i = 0; i < pathIds.size(); ++i)
    {
        auto *tool = qobject_cast<InternalPathTool*>(VAbstractPattern::getTool(pathIds.at(i)));
        SCASSERT(tool != nullptr);
        tool->setParentItem(this);
        tool->SetParentType(ParentType::Item);
        tool->show();
        doc->IncrementReferens(piece.getInternalPaths().at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::initializeAnchorPoints(const VPiece &piece)
{
    for (int i = 0; i < piece.getAnchors().size(); ++i)
    {
        doc->IncrementReferens(piece.getAnchors().at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::deleteTool(bool ask)
{
    QScopedPointer<DeletePiece> cmd(new DeletePiece(doc, m_id, VAbstractTool::data.GetPiece(m_id)));
    if (ask)
    {
        if (ConfirmDeletion() == QMessageBox::No)
        {
            return;
        }
        /* If Union tool delete piece no need emit FullParsing.*/
        connect(cmd.data(), &DeletePiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    }

    // If Union tool delete the piece this object will be deleted only after full parse.
    // Deleting inside Union could cause crash.
    // Because this object should be inactive from no one we disconnect all signals that may cause a crash
    // KEEP THIS LIST ACTUALL!!!
    disconnect(doc, nullptr, this, nullptr);
    if (QGraphicsScene *toolScene = scene())
    {
        disconnect(toolScene, nullptr, this, nullptr);
    }
    disconnect(m_dataLabel, nullptr, this, nullptr);
    disconnect(m_patternInfo, nullptr, this, nullptr);
    disconnect(m_grainLine, nullptr, this, nullptr);
    disconnect(m_pieceScene, nullptr, this, nullptr);

    hide();// User shouldn't see this object

    qApp->getUndoStack()->push(cmd.take());

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    VExceptionToolWasDeleted e("Tool was used after deleting.");
    throw e;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::ToolCreation(const Source &typeCreation)
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
bool PatternPieceTool::PrepareLabelData(const VPatternLabelData &labelData, VTextGraphicsItem *labelItem,
                                          QPointF &pos, qreal &labelAngle)
{
    SCASSERT(labelItem != nullptr)

    qDebug() << "Prepare Label Data";

    qreal labelWidth = 0;
    qreal labelHeight = 0;
    const VTextGraphicsItem::MoveTypes type = findLabelGeometry(labelData, labelAngle, labelWidth, labelHeight, pos);
    if (type & VGrainlineItem::Error)
    {
        labelItem->hide();
        return false;
    }
    labelItem->setMoveType(type);

    QFont fnt = qApp->Settings()->getLabelFont();
    {
        const int iFS = labelData.getFontSize();
        qDebug() << " Label Font Size = " << iFS;
        iFS < MIN_FONT_SIZE ? fnt.setPixelSize(MIN_FONT_SIZE) : fnt.setPixelSize(iFS);
        if (iFS < MIN_FONT_SIZE)
        {
        qDebug() << " Label Font Pixel Size = " << MIN_FONT_SIZE;
        }
        else {
        qDebug() << " Label Font Pixel Size = " << iFS;
        }
    }
    labelItem->setFont(fnt);
    labelItem->setSize(ToPixel(labelWidth, *VDataTool::data.GetPatternUnit()),
                       ToPixel(labelHeight, *VDataTool::data.GetPatternUnit()));

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::UpdateLabelItem(VTextGraphicsItem *labelItem, QPointF pos, qreal labelAngle)
{
    SCASSERT(labelItem != nullptr)

    QRectF rectBB;
    rectBB.setTopLeft(pos);
    rectBB.setWidth(labelItem->boundingRect().width());
    rectBB.setHeight(labelItem->boundingRect().height());
    qreal dX;
    qreal dY;
    if (labelItem->isContained(rectBB, labelAngle, dX, dY) == false)
    {
        pos.setX(pos.x() + dX);
        pos.setY(pos.y() + dY);
    }

    labelItem->setPos(pos);
    labelItem->setRotation(-labelAngle);// expects clockwise direction
    labelItem->updateItem();
    labelItem->getTextLines() > 0 ? labelItem->show() : labelItem->hide();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief editPieceProperties - routine to edit pattern piece properties .
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::editPieceProperties()
{
    QSharedPointer<PatternPieceDialog> dialog = QSharedPointer<PatternPieceDialog>(new PatternPieceDialog(getData(),
                                                                                   m_id, qApp->getMainWindow()));
    dialog->enableApply(true);
    m_dialog = dialog;
    m_dialog->setModal(true);
    connect(m_dialog.data(), &DialogTool::DialogClosed, this, &PatternPieceTool::FullUpdateFromGuiOk);
    connect(m_dialog.data(), &DialogTool::DialogApplied, this, &PatternPieceTool::FullUpdateFromGuiApply);
    SetDialog();
    m_dialog->show();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief toggleInLayout - routine to toggle if pattern piece is included and visible in layout.
/// @param checked - true if piece is included.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::toggleInLayout(bool checked)
{
    TogglePieceInLayout *cmd = new TogglePieceInLayout(m_id, checked, &(VAbstractTool::data), doc);
    connect(cmd, &TogglePieceInLayout::updateList, doc, &VAbstractPattern::updatePieceList);
    qApp->getUndoStack()->push(cmd);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief togglePieceLock - routine to toggle if pattern piece is locked and editiable.
/// @param checked - true if piece is locked.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::togglePieceLock(bool checked)
{
    TogglePieceLock *cmd = new TogglePieceLock(m_id, checked, &(VAbstractTool::data), doc);
    connect(cmd, &TogglePieceLock::updateList, doc, &VAbstractPattern::updatePieceList);
    qApp->getUndoStack()->push(cmd);

    EnableToolMove(!checked);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief toggleFlipping - routine to toggle forbidding flipping.
/// @param checked - true if flipping is forbidden.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::toggleFlipping(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetForbidFlipping(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Forbid Flipping"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Forbid Flipping changed: ") + (checked ? tr("Enabled") : tr("Disabled")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief toggleSeamLine - routine to toggle the visibility of the seam line.
/// @param checked - true if seam line is visible.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::toggleSeamLine(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.setHideSeamLine(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Hide Seam Line"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam line visibility changed: ") + (checked ? tr("Hide") : tr("Show")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief toggleSeamAllowance - routine to toggle the visibility of the seam allowance.
/// @param checked - true if seam allowance is visible.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::toggleSeamAllowance(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetSeamAllowance(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Show seam allowance"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam allowance visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief toggleGrainline - routine to toggle the visibility of the  piece grainline.
/// @param checked - true if grainline is visible.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::toggleGrainline(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Show grainline"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Grainline visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief togglePatternLabel - routine to toggle the visibility of the  pattern label.
/// @param checked - true if pattern label is visible.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::togglePatternLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Show pattern label"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Pattern label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief togglePieceLabel - routine to toggle the visibility of the piece label.
/// @param checked - true if piece label is visible.
//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::togglePieceLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
    undoCommand->setText(tr("Show piece label"));
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Piece label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief renamePiece - routine to rename pattern piece.
//--------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::renamePiece(VPiece piece)
{
    QInputDialog *dialog = new QInputDialog(nullptr);
    dialog->setInputMode( QInputDialog::TextInput );
    dialog->setLabelText(tr("Piece name:"));
    dialog->setTextEchoMode(QLineEdit::Normal);
    dialog->setWindowTitle(tr("Rename Pattern Piece"));
    dialog->setWindowIcon(QIcon());
    dialog->setWindowFlags(dialog->windowFlags() & ~Qt::WindowContextHelpButtonHint
                                                 & ~Qt::WindowMaximizeButtonHint
                                                 & ~Qt::WindowMinimizeButtonHint);
    dialog->resize(300, 100);
    dialog->setTextValue(piece.GetName());
    QString pieceName;
    const bool result = dialog->exec();
    pieceName = dialog->textValue();

    if (result == true && !pieceName.isEmpty())
    {
        VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
        VPiece newPiece = oldPiece;
        newPiece.SetName(pieceName);

        SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, &(VAbstractTool::data), m_id);
        undoCommand->setText(tr("Rename pattern piece"));
        qApp->getUndoStack()->push(undoCommand);

        showStatus(tr("Piece renamed to: ") + pieceName);
    }
}

void PatternPieceTool::showStatus(QString message)
{
    VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
    SCASSERT(window != nullptr)

    window->setStatusMessage(message);
}


void PatternPieceTool::raiseItemToTop(QGraphicsItem *item)
{
    if (item)
    {
        m_pieceScene->removeItem(item);
        m_pieceScene->addItem(item);
    }
}

void PatternPieceTool::lowerItemToBottom(QGraphicsItem *item)
{
    if (item != nullptr)
    {
        QList<QGraphicsItem *> items = m_pieceScene->items();
        if (!items.isEmpty())
        {
            for (int i = 0; i < items.count(); ++i)
            {
                if (items.at(i)->parentItem() == nullptr)
                {
                    item->stackBefore(items.at(i));
                }
            }
        }
    }
}

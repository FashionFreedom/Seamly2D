/***************************************************************************
 **  @file   pattern_piece_tool.cpp
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
 **  @file   PatternPieceTool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
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

#include "pattern_piece_tool.h"

#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/vtoolinternalpath.h"
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
#include "../qmuparser/qmutokenparser.h"
#include "../undocommands/addpiece.h"
#include "../undocommands/deletepiece.h"
#include "../undocommands/movepiece.h"
#include "../undocommands/savepieceoptions.h"
#include "../undocommands/togglepieceinlayout.h"
#include "../undocommands/toggle_piecelock.h"
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
    SCASSERT(not dialog.isNull());
    QSharedPointer<PatternPieceDialog> dialogTool = dialog.objectCast<PatternPieceDialog>();
    SCASSERT(not dialogTool.isNull())
    VPiece piece = dialogTool->GetPiece();
    QString width = piece.getSeamAllowanceWidthFormula();
    qApp->getUndoStack()->beginMacro("add pattern piece");

    piece.GetPath().SetNodes(PrepareNodes(piece.GetPath(), scene, doc, data));

    PatternPieceTool *patternPiece = Create(0, piece, width, scene, doc, data, Document::FullParse, Source::FromGui);

    if (patternPiece != nullptr)
    {
        patternPiece->m_dialog = dialogTool;
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
        connect(scene, &VMainGraphicsScene::highlightPiece,            patternPiece, &PatternPieceTool::Highlight);
        connect(scene, &VMainGraphicsScene::pieceLockedChanged,        patternPiece, &PatternPieceTool::pieceLockedChanged);

        VAbstractPattern::AddTool(id, patternPiece);
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

        VPiece newPiece = oldPiece;

        for (auto node : nodes)
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

        SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, pieceId);
        qApp->getUndoStack()->push(saveCommand);// First push then make a connect
        data->UpdatePiece(pieceId, newPiece);// Update piece because first save will not call lite update
        connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece)
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
    doc->SetAttribute(domElement, AttrSeamAllowance,    piece.IsSeamAllowance());
    doc->SetAttribute(domElement, AttrHideSeamLine,     piece.isHideSeamLine());

    const bool saBuiltIn = piece.IsSeamAllowanceBuiltIn();
    if (saBuiltIn)
    {
        doc->SetAttribute(domElement, AttrSeamAllowanceBuiltIn, saBuiltIn);
    }
    else
    { // For backward compatebility.
        domElement.removeAttribute(AttrSeamAllowanceBuiltIn);
    }

    doc->SetAttribute(domElement, VAbstractPattern::AttrWidth, piece.getSeamAllowanceWidthFormula());
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
void PatternPieceTool::AddCSARecords(VAbstractPattern *doc, QDomElement &domElement,
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
void PatternPieceTool::AddInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths)
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
void PatternPieceTool::AddPatternPieceData(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagData);
    const VPieceLabelData &data = piece.GetPatternPieceData();
    doc->SetAttribute(domData, VAbstractPattern::AttrLetter,       data.GetLetter());
    doc->SetAttribute(domData, VAbstractPattern::AttrAnnotation,   data.GetAnnotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrOrientation,  data.GetOrientation());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotationWay,  data.GetRotationWay());
    doc->SetAttribute(domData, VAbstractPattern::AttrTilt,         data.GetTilt());
    doc->SetAttribute(domData, VAbstractPattern::AttrFoldPosition, data.GetFoldPosition());
    doc->SetAttribute(domData, VAbstractPattern::AttrQuantity,     data.GetQuantity());
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,      data.IsVisible());
    doc->SetAttribute(domData, VAbstractPattern::AttrOnFold,       data.IsOnFold());
    doc->SetAttribute(domData, AttrMx,                             data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                             data.GetPos().y());
    doc->SetAttribute(domData, VAbstractPattern::AttrWidth,        data.GetLabelWidth());
    doc->SetAttribute(domData, AttrHeight,                         data.GetLabelHeight());
    doc->SetAttribute(domData, AttrFont,                           data.getFontSize());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation,     data.GetRotation());

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
void PatternPieceTool::AddPatternInfo(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagPatternInfo);
    const VPatternLabelData &data = piece.GetPatternInfo();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,  data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                         data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                         data.GetPos().y());
    doc->SetAttribute(domData, VAbstractPattern::AttrWidth,    data.GetLabelWidth());
    doc->SetAttribute(domData, AttrHeight,                     data.GetLabelHeight());
    doc->SetAttribute(domData, AttrFont,                       data.getFontSize());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, data.GetRotation());

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
void PatternPieceTool::AddGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    // grainline
    QDomElement domData = doc->createElement(VAbstractPattern::TagGrainline);
    const VGrainlineData &data = piece.GetGrainlineGeometry();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,  data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                         data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                         data.GetPos().y());
    doc->SetAttribute(domData, AttrLength,                     data.GetLength());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, data.GetRotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrArrows,   int(data.GetArrowType()));

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
    UpdateExcludeState();
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
    for (int i = 0; i< piece.GetPath().CountNodes(); ++i)
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
        for (int i = 0; i< piece.GetPath().CountNodes(); ++i)
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
void PatternPieceTool::ResetChildren(QGraphicsItem *pItem)
{
    const bool selected = isSelected();
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    VTextGraphicsItem *pVGI = qgraphicsitem_cast<VTextGraphicsItem*>(pItem);
    if (pVGI != m_dataLabel)
    {
        if (piece.GetPatternPieceData().IsVisible())
        {
            m_dataLabel->Reset();
        }
    }
    if (pVGI != m_patternInfo)
    {
        if (piece.GetPatternInfo().IsVisible())
        {
            m_patternInfo->Reset();
        }
    }
    VGrainlineItem *pGLI = qgraphicsitem_cast<VGrainlineItem*>(pItem);
    if (pGLI != m_grainLine)
    {
        if (piece.GetGrainlineGeometry().IsVisible())
        {
            m_grainLine->Reset();
        }
    }

    setSelected(selected);
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::UpdateAll()
{
    m_pieceScene->update();
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::retranslateUi()
{
    UpdatePieceLabel();
    UpdatePatternLabel();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::Highlight(quint32 id)
{
    setSelected(m_id == id);
    RefreshGeometry();
}

void PatternPieceTool::updatePieceDetails()
{
    UpdatePieceLabel();
    UpdatePatternLabel();
    UpdateGrainline();
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateLabel updates the text label, making it just big enough for the text to fit it
 */
void PatternPieceTool::UpdatePieceLabel()
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
/**
 * @brief UpdatePatternLabel updates the pattern info label
 */
void PatternPieceTool::UpdatePatternLabel()
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
/**
 * @brief VToolDetail::UpdateGrainline updates the grain line item
 */
void PatternPieceTool::UpdateGrainline()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    const VGrainlineData &data = piece.GetGrainlineGeometry();

    qDebug() << "Update Grainline IsVisible() = " << data.IsVisible();

    if (data.IsVisible() & qApp->Settings()->showGrainlines())
    {
        QPointF pos;
        qreal dRotation = 0;
        qreal dLength = 0;

        const VGrainlineItem::MoveTypes type = FindGrainlineGeometry(data, dLength, dRotation, pos);
        if (type & VGrainlineItem::Error)
        {
            m_grainLine->hide();
            return;
        }

        m_grainLine->SetMoveType(type);
        m_grainLine->UpdateGeometry(pos, dRotation, ToPixel(dLength, *VDataTool::data.GetPatternUnit()),
                                    data.GetArrowType());
        m_grainLine->show();
    }
    else
    {
        m_grainLine->hide();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief saveMovePiece saves the move piece operation to the undo stack
 */
void PatternPieceTool::saveMovePiece(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetPos(ptPos);

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move pattern piece label"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief saveResizePiece saves the resize piece label operation to the undo stack
 */
void PatternPieceTool::saveResizePiece(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelWidth(QString().setNum(dLabelW));
    const qreal height = FromPixel(m_dataLabel->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternPieceData().SetFontSize(iFontSize);

    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize pattern piece label"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief savePieceRotation saves the rotation piece label operation to the undo stack
 */
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

    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate pattern piece label"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveMovePattern saves the pattern label position
 */
void PatternPieceTool::SaveMovePattern(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetPos(ptPos);

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move pattern info label"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief: SaveResizePattern saves the pattern label width and font size
 */
void PatternPieceTool::SaveResizePattern(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelWidth(QString().setNum(dLabelW));
    qreal height = FromPixel(m_patternInfo->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternInfo().SetFontSize(iFontSize);

    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize pattern info label"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
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

    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate pattern info label"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}


//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveMoveGrainline(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    qDebug() << "******* new grainline pos" << ptPos;

    SavePieceOptions *moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move grainline"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveResizeGrainline(qreal dLength)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLength = FromPixel(dLength, *VDataTool::data.GetPatternUnit());
    newPiece.GetGrainlineGeometry().SetPos(m_grainLine->pos());
    newPiece.GetGrainlineGeometry().SetLength(QString().setNum(dLength));
    SavePieceOptions *resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize grainline"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveRotateGrainline(qreal dRot, const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    newPiece.GetGrainlineGeometry().SetRotation(QString().setNum(qRadiansToDegrees(dRot)));
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    SavePieceOptions *rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate grainline"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolDetail::paint draws a bounding box around piece, if one of its text or grainline items is not idle.
 */
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
        if (piece.IsSeamAllowance() && !piece.IsSeamAllowanceBuiltIn())
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

        m_seamLine->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                                lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));

        QBrush brush = QBrush(QColor(piece.getColor()));
        brush.setStyle(static_cast<Qt::BrushStyle>(fills().indexOf(QRegExp(piece.getFill()))));
        brush.setTransform(brush.transform().scale(150.0, 150.0));
        brush.setTransform(painter->combinedTransform().inverted());
        m_seamLine->setBrush(brush);
        this->setBrush(brush);
    }

    //set notches pen
    color = QColor(qApp->Settings()->getDefaultNotchColor());
    m_notches->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                           Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));


    if ((m_dataLabel->IsIdle() == false
            || m_patternInfo->IsIdle() == false
            || m_grainLine->IsIdle() == false) && not isSelected())
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
    if (m_mainPath == QPainterPath() && m_cutPath == QPainterPath())
    {
        return QGraphicsPathItem::shape();
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

    QDomElement domElement = doc->createElement(getTagName());

    AddAttributes(doc, domElement, m_id, piece);
    AddPatternPieceData(doc, domElement, piece);
    AddPatternInfo(doc, domElement, piece);
    AddGrainline(doc, domElement, piece);

    // nodes
    AddNodes(doc, domElement, piece);
    //custom seam allowance
    AddCSARecords(doc, domElement, piece.GetCustomSARecords());
    AddInternalPaths(doc, domElement, piece.GetInternalPaths());
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

                doc->SetAttribute(domElement, AttrVersion, QString().setNum(pieceVersion));

                doc->RemoveAllChildren(domElement);//Very important to clear before rewrite
                AddPatternPieceData(doc, domElement, piece);
                AddPatternInfo(doc, domElement, piece);
                AddGrainline(doc, domElement, piece);
                AddNodes(doc, domElement, piece);
                AddCSARecords(doc, domElement, piece.GetCustomSARecords());
                AddInternalPaths(doc, domElement, piece.GetInternalPaths());
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
            if (not viewList.isEmpty())
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
    if (not (flags() & ItemIsSelectable) && scene())
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
    hideMainPath->setEnabled(piece.IsSeamAllowance() && qApp->Settings()->showSeamAllowances());

    QAction *showSeamAllowance = menu.addAction(QIcon("://icon/32x32/seam_allowance.png"),
                                                tr("Show Seam Allowance") + "\tS");
    showSeamAllowance->setCheckable(true);
    showSeamAllowance->setChecked(piece.IsSeamAllowance());

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
    _referens > 1 ? deletePiece->setEnabled(false) : deletePiece->setEnabled(true);
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
                if (piece.IsSeamAllowance() && qApp->Settings()->showSeamAllowances())
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
                toggleSeamAllowance(!piece.IsSeamAllowance());
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
    SCASSERT(not m_dialog.isNull());
    QSharedPointer<PatternPieceDialog> dialogTool = m_dialog.objectCast<PatternPieceDialog>();
    SCASSERT(not dialogTool.isNull())
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
    , m_seamLine(new NonScalingFillPathItem(this))
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
    connect(scene, &VMainGraphicsScene::ItemClicked,    this, &PatternPieceTool::ResetChildren);
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

    connect(doc, &VAbstractPattern::UpdatePatternLabel, this, &PatternPieceTool::UpdatePatternLabel);
    connect(doc, &VAbstractPattern::patternParsed,        this, &PatternPieceTool::updatePieceDetails);

    connect(m_pieceScene, &VMainGraphicsScene::DimensionsChanged, this, &PatternPieceTool::updatePieceDetails);
    connect(m_pieceScene, &VMainGraphicsScene::LanguageChanged,   this, &PatternPieceTool::retranslateUi);

    updatePieceDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::UpdateExcludeState()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    for (int i = 0; i< piece.GetPath().CountNodes(); ++i)
    {
        const VPieceNode &node = piece.GetPath().at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            VNodePoint *tool = qobject_cast<VNodePoint*>(VAbstractPattern::getTool(node.GetId()));
            SCASSERT(tool != nullptr);

            tool->SetExluded(node.isExcluded());
            tool->setVisible(not node.isExcluded());//Hide excluded point
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::RefreshGeometry()
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    m_cutLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);

    QPainterPath path = piece.MainPathPath(this->getData());

    if (!piece.isHideSeamLine() || !piece.IsSeamAllowance() || piece.IsSeamAllowanceBuiltIn())
    {
        m_mainPath = QPainterPath();
        m_seamLine->setPath(m_mainPath);
        m_cutLine->setBrush(QBrush(QColor(qApp->Settings()->getDefaultCutColor()), Qt::Dense7Pattern));
    }
    else
    {
        m_mainPath = path; // need for returning a bounding rect when main path is not visible
        path = QPainterPath();
        m_seamLine->setPath(QPainterPath());
        m_cutLine->setBrush(QBrush(Qt::NoBrush)); // Disable if the main path was hidden
    }

    this->setPath(path);

    QVector<QPointF> seamAllowancePoints;

    if (piece.IsSeamAllowance())
    {
        seamAllowancePoints = piece.SeamAllowancePoints(this->getData());
    }

    m_notches->setPath(piece.getNotchesPath(this->getData(), seamAllowancePoints));

    if (piece.IsSeamAllowance() && !piece.IsSeamAllowanceBuiltIn() && qApp->Settings()->showSeamAllowances())
    {
        path.addPath(piece.SeamAllowancePath(seamAllowancePoints));
        path.setFillRule(Qt::OddEvenFill);
        m_cutPath = path;
        m_cutLine->setPath(m_cutPath);
        if (piece.isHideSeamLine())
        {
            m_seamLine->setPath(QPainterPath());
        }
        else
        {
            m_seamLine->setPath(m_mainPath);
        }
    }
    else
    {
        m_cutLine->setPath(QPainterPath());
    }

    m_pieceRect = path.boundingRect();
    this->setPos(piece.GetMx(), piece.GetMy());
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::SaveDialogChange()
{
    SCASSERT(not m_dialog.isNull());
    PatternPieceDialog *dialogTool = qobject_cast<PatternPieceDialog*>(m_dialog.data());
    SCASSERT(dialogTool != nullptr);
    const VPiece newPiece = dialogTool->GetPiece();
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);

    SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(saveCommand);
    UpdatePieceLabel();
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::MoveTypes PatternPieceTool::FindLabelGeometry(const VPatternLabelData &labelData, qreal &rotationAngle,
                                                            qreal &labelWidth, qreal &labelHeight, QPointF &pos)
{
    qDebug() << "Find label Geometery";
    VPieceItem::MoveTypes restrictions = VPieceItem::AllModifications;
    try
    {
        if (not qmu::QmuTokenParser::IsSingle(labelData.GetRotation()))
        {
            restrictions &= ~ VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetRotation());
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
        qDebug() << " Label width: " << labelWidth;
        qDebug() << " Label width is single: " << widthIsSingle;
        const bool heightIsSingle = qmu::QmuTokenParser::IsSingle(labelData.GetLabelHeight());

        Calculator cal2;
        labelHeight = cal2.EvalFormula(VAbstractTool::data.DataVariables(), labelData.GetLabelHeight());
        qDebug() << " Label height: " << labelHeight;
        qDebug() << " Label height is single: " << heightIsSingle;
        if (not widthIsSingle || not heightIsSingle)
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
            qDebug() << " Anchor center point: " << centerAnchorPoint;
            const qreal lWidth = ToPixel(labelWidth, *VDataTool::data.GetPatternUnit());
            const qreal lHeight = ToPixel(labelHeight, *VDataTool::data.GetPatternUnit());
            qDebug() << " Label pixel width: " << lWidth;
            qDebug() << " Label pixel height: " << lHeight;
            pos = static_cast<QPointF>(*centerAnchorPoint) - QRectF(0, 0, lWidth, lHeight).center();
            qDebug() << " Anchor point position: " << pos;
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
VPieceItem::MoveTypes PatternPieceTool::FindGrainlineGeometry(const VGrainlineData &data, qreal &length,
                                                               qreal &rotationAngle, QPointF &pos)
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

            if (not VFuzzyComparePossibleNulls(rotationAngle, 0))
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
        if (not qmu::QmuTokenParser::IsSingle(data.GetRotation()))
        {
            restrictions &= ~ VPieceItem::IsRotatable;
        }

        Calculator cal1;
        rotationAngle = cal1.EvalFormula(VAbstractTool::data.DataVariables(), data.GetRotation());

        if (not qmu::QmuTokenParser::IsSingle(data.GetLength()))
        {
            restrictions &= ~ VPieceItem::IsResizable;
        }

        Calculator cal2;
        length = cal2.EvalFormula(VAbstractTool::data.DataVariables(), data.GetLength());
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
    for (int i = 0; i< piece.GetPath().CountNodes(); ++i)
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

            connect(tool, &VNodePoint::chosenTool, scene, &VMainGraphicsScene::chosenItem, Qt::UniqueConnection);
            tool->setParentItem(parent);
            tool->SetParentType(ParentType::Item);
            tool->SetExluded(node.isExcluded());
            tool->setVisible(not node.isExcluded());//Hide excluded point
            doc->IncrementReferens(node.GetId());
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

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::InitCSAPaths(const VPiece &piece)
{
    for (int i = 0; i < piece.GetCustomSARecords().size(); ++i)
    {
        doc->IncrementReferens(piece.GetCustomSARecords().at(i).path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceTool::InitInternalPaths(const VPiece &piece)
{
    const QVector<quint32> pathIds = piece.GetInternalPaths();
    for (int i = 0; i < pathIds.size(); ++i)
    {
        const VPiecePath path = this->getData()->GetPiecePath(pathIds.at(i));
        QColor  color;
        if (path.IsCutPath())
        {
            color = QColor(qApp->Settings()->getDefaultCutoutColor());
        }
        else
        {
            color = QColor(qApp->Settings()->getDefaultInternalColor());
        }
        Qt::PenStyle lineType   = path.GetPenType();
        qreal   lineWeight = ToPixel(qApp->Settings()->getDefaultInternalLineweight(), Unit::Mm);

        auto *tool = qobject_cast<VToolInternalPath*>(VAbstractPattern::getTool(pathIds.at(i)));
        SCASSERT(tool != nullptr);
        tool->setParentItem(this);
        tool->SetParentType(ParentType::Item);
        tool->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())), lineType, Qt::RoundCap, Qt::RoundJoin));
        tool->show();
        doc->IncrementReferens(piece.GetInternalPaths().at(i));
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
    const VTextGraphicsItem::MoveTypes type = FindLabelGeometry(labelData, labelAngle, labelWidth, labelHeight, pos);
    if (type & VGrainlineItem::Error)
    {
        labelItem->hide();
        return false;
    }
    labelItem->SetMoveType(type);

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
    labelItem->Update();
    labelItem->getTextLines() > 0 ? labelItem->show() : labelItem->hide();
}

/**
 * @brief editPieceProperties - routine to edit pattern piece properties .
 */
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

/**
 * @brief toggleInLayout - routine to toggle if pattern piece is included and visible in layout.
 * @param checked - true if piece is included.
 */
void PatternPieceTool::toggleInLayout(bool checked)
{
    TogglePieceInLayout *cmd = new TogglePieceInLayout(m_id, checked, &(VAbstractTool::data), doc);
    connect(cmd, &TogglePieceInLayout::updateList, doc, &VAbstractPattern::updatePieceList);
    qApp->getUndoStack()->push(cmd);
}

/**
 * @brief togglePieceLock - routine to toggle if pattern piece is locked and editiable.
 * @param checked - true if piece is locked.
 */
void PatternPieceTool::togglePieceLock(bool checked)
{
    TogglePieceLock *cmd = new TogglePieceLock(m_id, checked, &(VAbstractTool::data), doc);
    connect(cmd, &TogglePieceLock::updateList, doc, &VAbstractPattern::updatePieceList);
    qApp->getUndoStack()->push(cmd);

    EnableToolMove(!checked);
}

/**
 * @brief toggleFlipping - routine to toggle forbidding flipping.
 * @param checked - true if flipping is forbidden.
 */
void PatternPieceTool::toggleFlipping(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetForbidFlipping(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Forbid Flipping"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Forbid Flipping changed: ") + (checked ? tr("Enabled") : tr("Disabled")));
}

/**
 * @brief toggleSeamLine - routine to toggle the visibility of the seam line.
 * @param checked - true if seam line is visible.
 */
void PatternPieceTool::toggleSeamLine(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.setHideSeamLine(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Hide Seam Line"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam line visibility changed: ") + (checked ? tr("Hide") : tr("Show")));
}

/**
 * @brief toggleSeamAllowance - routine to toggle the visibility of the seam allowance.
 * @param checked - true if seam allowance is visible.
 */
void PatternPieceTool::toggleSeamAllowance(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetSeamAllowance(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show seam allowance"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam allowance visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief toggleGrainline - routine to toggle the visibility of the  piece grainline.
 * @param checked - true if grainline is visible.
 */
void PatternPieceTool::toggleGrainline(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show grainline"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Grainline visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief togglePatternLabel - routine to toggle the visibility of the  pattern label.
 * @param checked - true if pattern label is visible.
 */
void PatternPieceTool::togglePatternLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show pattern label"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Pattern label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief togglePieceLabel - routine to toggle the visibility of the piece label.
 * @param checked - true if piece label is visible.
 */
void PatternPieceTool::togglePieceLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetVisible(checked);

    SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show piece label"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Piece label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief renamePiece - routine to rename pattern piece.
 */
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

        SavePieceOptions *undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
        undoCommand->setText(tr("Rename pattern piece"));
        connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(undoCommand);

        showStatus(tr("Piece renamed to: ") + pieceName);
    }
}

void PatternPieceTool::showStatus(QString toolTip)
{
    VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
    SCASSERT(window != nullptr)

    window->ShowToolTip(toolTip);
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

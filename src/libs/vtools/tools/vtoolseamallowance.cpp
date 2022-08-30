/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
 **
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vtoolseamallowance.h"
#include "../dialogs/tools/piece/dialogseamallowance.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/vtoolinternalpath.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../undocommands/addpiece.h"
#include "../undocommands/deletepiece.h"
#include "../undocommands/movepiece.h"
#include "../undocommands/savepieceoptions.h"
#include "../undocommands/togglepieceinlayout.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vwidgets/vnobrushscalepathitem.h"
#include "../qmuparser/qmutokenparser.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>


// Current version of seam allowance tag need for backward compatibility
const quint8 VToolSeamAllowance::pieceVersion = 2;

const QString VToolSeamAllowance::TagCSA     = QStringLiteral("csa");
const QString VToolSeamAllowance::TagRecord  = QStringLiteral("record");
const QString VToolSeamAllowance::TagIPaths  = QStringLiteral("iPaths");
const QString VToolSeamAllowance::TagPins    = QStringLiteral("pins");

const QString VToolSeamAllowance::AttrVersion              = QStringLiteral("version");
const QString VToolSeamAllowance::AttrForbidFlipping       = QStringLiteral("forbidFlipping");
const QString VToolSeamAllowance::AttrSeamAllowance        = QStringLiteral("seamAllowance");
const QString VToolSeamAllowance::AttrHideSeamLine         = QStringLiteral("hideMainPath");
const QString VToolSeamAllowance::AttrSeamAllowanceBuiltIn = QStringLiteral("seamAllowanceBuiltIn");
const QString VToolSeamAllowance::AttrHeight               = QStringLiteral("height");
const QString VToolSeamAllowance::AttrUnited               = QStringLiteral("united");
const QString VToolSeamAllowance::AttrFont                 = QStringLiteral("fontSize");
const QString VToolSeamAllowance::AttrTopLeftPin           = QStringLiteral("topLeftPin");
const QString VToolSeamAllowance::AttrBottomRightPin       = QStringLiteral("bottomRightPin");
const QString VToolSeamAllowance::AttrCenterPin            = QStringLiteral("centerPin");
const QString VToolSeamAllowance::AttrTopPin               = QStringLiteral("topPin");
const QString VToolSeamAllowance::AttrBottomPin            = QStringLiteral("bottomPin");

//---------------------------------------------------------------------------------------------------------------------
VToolSeamAllowance *VToolSeamAllowance::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                               VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull());
    QSharedPointer<DialogSeamAllowance> dialogTool = dialog.objectCast<DialogSeamAllowance>();
    SCASSERT(not dialogTool.isNull())
    VPiece piece = dialogTool->GetPiece();
    QString width = piece.GetFormulaSAWidth();
    qApp->getUndoStack()->beginMacro("add pattern piece");
    piece.GetPath().SetNodes(PrepareNodes(piece.GetPath(), scene, doc, data));

    VToolSeamAllowance *patternPiece = Create(0, piece, width, scene, doc, data, Document::FullParse, Source::FromGui);

    if (patternPiece != nullptr)
    {
        patternPiece->m_dialog = dialogTool;
    }
    return patternPiece;
}

//---------------------------------------------------------------------------------------------------------------------
VToolSeamAllowance *VToolSeamAllowance::Create(quint32 id, VPiece newPiece, QString &width, VMainGraphicsScene *scene,
                                               VAbstractPattern *doc, VContainer *data, const Document &parse,
                                               const Source &typeCreation, const QString &drawName)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        data->AddVariable(currentSeamAllowance, new VIncrement(data, currentSeamAllowance, 0, newPiece.GetSAWidth(),
                                                               width, true, tr("Current seam allowance")));
        id = data->AddPiece(newPiece);
    }
    else
    {
        const qreal calcWidth = CheckFormula(id, width, data);
        newPiece.SetFormulaSAWidth(width, calcWidth);

        data->AddVariable(currentSeamAllowance, new VIncrement(data, currentSeamAllowance, 0, calcWidth,
                                                               width, true, tr("Current seam allowance")));

        data->UpdatePiece(id, newPiece);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    VToolSeamAllowance *patternPiece = nullptr;
    if (parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(id, Tool::Piece, doc);
        patternPiece = new VToolSeamAllowance(doc, data, id, typeCreation, scene, drawName);
        scene->addItem(patternPiece);
        connect(patternPiece, &VToolSeamAllowance::chosenTool, scene, &VMainGraphicsScene::chosenItem);
        connect(scene, &VMainGraphicsScene::EnableDetailItemHover,     patternPiece, &VToolSeamAllowance::AllowHover);
        connect(scene, &VMainGraphicsScene::EnableDetailItemSelection, patternPiece, &VToolSeamAllowance::AllowSelecting);
        connect(scene, &VMainGraphicsScene::HighlightDetail,           patternPiece, &VToolSeamAllowance::Highlight);
        VAbstractPattern::AddTool(id, patternPiece);
    }
    //Very important to delete it. Only this tool need this special variable.
    data->RemoveVariable(currentSeamAllowance);
    return patternPiece;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::Remove(bool ask)
{
    try
    {
        deleteTool(ask);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e);
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InsertNode(VPieceNode node, quint32 pieceId, VMainGraphicsScene *scene,
                                    VContainer *data, VAbstractPattern *doc)
{
    SCASSERT(scene != nullptr)
    SCASSERT(data != nullptr)
    SCASSERT(doc != nullptr)

    if (pieceId > NULL_ID)
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

        const quint32 id = PrepareNode(node, scene, doc, data);
        if (id == NULL_ID)
        {
            return;
        }

        node.SetId(id);
        newPiece.GetPath().Append(node);

        // Seam allowance tool already initialized and can't init the node
        VToolSeamAllowance *patternPiece = qobject_cast<VToolSeamAllowance*>(VAbstractPattern::getTool(pieceId));
        SCASSERT(patternPiece != nullptr);

        InitNode(node, scene, data, doc, patternPiece);

        SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, pieceId);
        qApp->getUndoStack()->push(saveCommand);// First push then make a connect
        data->UpdatePiece(pieceId, newPiece);// Update piece because first save will not call lite update
        connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece)
{
    SCASSERT(doc != nullptr);

    doc->SetAttribute(domElement, VDomDocument::AttrId, id);
    doc->SetAttribute(domElement, AttrName,             piece.GetName());
    doc->SetAttribute(domElement, AttrVersion,          QString().setNum(pieceVersion));
    doc->SetAttribute(domElement, AttrMx,               qApp->fromPixel(piece.GetMx()));
    doc->SetAttribute(domElement, AttrMy,               qApp->fromPixel(piece.GetMy()));
    doc->SetAttribute(domElement, AttrInLayout,         piece.IsInLayout());
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

    doc->SetAttribute(domElement, VAbstractPattern::AttrWidth, piece.GetFormulaSAWidth());
    doc->SetAttribute(domElement, AttrUnited, piece.IsUnited());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, const CustomSARecord &record)
{
    QDomElement recordNode = doc->createElement(VToolSeamAllowance::TagRecord);

    doc->SetAttribute(recordNode, VAbstractPattern::AttrStart,       record.startPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrPath,        record.path);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrEnd,         record.endPoint);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrNodeReverse, record.reverse);
    doc->SetAttribute(recordNode, VAbstractPattern::AttrIncludeAs,   static_cast<unsigned int>(record.includeType));

    domElement.appendChild(recordNode);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddCSARecords(VAbstractPattern *doc, QDomElement &domElement,
                                       const QVector<CustomSARecord> &records)
{
    if (records.size() > 0)
    {
        QDomElement csaRecordsElement = doc->createElement(VToolSeamAllowance::TagCSA);
        for (int i = 0; i < records.size(); ++i)
        {
            AddCSARecord(doc, csaRecordsElement, records.at(i));
        }
        domElement.appendChild(csaRecordsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths)
{
    if (paths.size() > 0)
    {
        QDomElement iPathsElement = doc->createElement(VToolSeamAllowance::TagIPaths);
        for (int i = 0; i < paths.size(); ++i)
        {
            QDomElement recordNode = doc->createElement(VToolSeamAllowance::TagRecord);
            doc->SetAttribute(recordNode, VAbstractPattern::AttrPath, paths.at(i));
            iPathsElement.appendChild(recordNode);
        }
        domElement.appendChild(iPathsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPins(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &pins)
{
    if (pins.size() > 0)
    {
        QDomElement pinsElement = doc->createElement(VToolSeamAllowance::TagPins);
        for (int i = 0; i < pins.size(); ++i)
        {
            QDomElement recordNode = doc->createElement(VToolSeamAllowance::TagRecord);
            recordNode.appendChild(doc->createTextNode(QString().setNum(pins.at(i))));
            pinsElement.appendChild(recordNode);
        }
        domElement.appendChild(pinsElement);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPatternPieceData(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagData);
    const VPieceLabelData& data = piece.GetPatternPieceData();
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

    if (data.CenterPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterPin, data.CenterPin());
    }
    else
    {
        domData.removeAttribute(AttrCenterPin);
    }

    if (data.TopLeftPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopLeftPin, data.TopLeftPin());
    }
    else
    {
        domData.removeAttribute(AttrTopLeftPin);
    }

    if (data.BottomRightPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomRightPin, data.BottomRightPin());
    }
    else
    {
        domData.removeAttribute(AttrBottomRightPin);
    }

    doc->SetLabelTemplate(domData, data.GetLabelTemplate());

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddPatternInfo(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    QDomElement domData = doc->createElement(VAbstractPattern::TagPatternInfo);
    const VPatternLabelData& data = piece.GetPatternInfo();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,  data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                         data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                         data.GetPos().y());
    doc->SetAttribute(domData, VAbstractPattern::AttrWidth,    data.GetLabelWidth());
    doc->SetAttribute(domData, AttrHeight,                     data.GetLabelHeight());
    doc->SetAttribute(domData, AttrFont,                       data.getFontSize());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, data.GetRotation());

    if (data.CenterPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterPin, data.CenterPin());
    }
    else
    {
        domData.removeAttribute(AttrCenterPin);
    }

    if (data.TopLeftPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopLeftPin, data.TopLeftPin());
    }
    else
    {
        domData.removeAttribute(AttrTopLeftPin);
    }

    if (data.BottomRightPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomRightPin, data.BottomRightPin());
    }
    else
    {
        domData.removeAttribute(AttrBottomRightPin);
    }

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece)
{
    // grainline
    QDomElement domData = doc->createElement(VAbstractPattern::TagGrainline);
    const VGrainlineData& data = piece.GetGrainlineGeometry();
    doc->SetAttribute(domData, VAbstractPattern::AttrVisible,  data.IsVisible());
    doc->SetAttribute(domData, AttrMx,                         data.GetPos().x());
    doc->SetAttribute(domData, AttrMy,                         data.GetPos().y());
    doc->SetAttribute(domData, AttrLength,                     data.GetLength());
    doc->SetAttribute(domData, VAbstractPattern::AttrRotation, data.GetRotation());
    doc->SetAttribute(domData, VAbstractPattern::AttrArrows,   int(data.GetArrowType()));

    if (data.CenterPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrCenterPin, data.CenterPin());
    }
    else
    {
        domData.removeAttribute(AttrCenterPin);
    }

    if (data.TopPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrTopPin, data.TopPin());
    }
    else
    {
        domData.removeAttribute(AttrTopPin);
    }

    if (data.BottomPin() > NULL_ID)
    {
        doc->SetAttribute(domData, AttrBottomPin, data.BottomPin());
    }
    else
    {
        domData.removeAttribute(AttrBottomPin);
    }

    domElement.appendChild(domData);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolSeamAllowance::getTagName() const
{
    return VAbstractPattern::TagDetail;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object);
    Q_UNUSED(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::FullUpdateFromFile()
{
    UpdateExcludeState();
    RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::EnableToolMove(bool move)
{
    setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ResetChildren(QGraphicsItem *pItem)
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
    VGrainlineItem* pGLI = qgraphicsitem_cast<VGrainlineItem*>(pItem);
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
void VToolSeamAllowance::UpdateAll()
{
    m_pieceScene->update();
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::retranslateUi()
{
    UpdateDetailLabel();
    UpdatePatternInfo();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::Highlight(quint32 id)
{
    setSelected(m_id == id);
}

void VToolSeamAllowance::updatePieceDetails()
{
    UpdateDetailLabel();
    UpdatePatternInfo();
    UpdateGrainline();
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateLabel updates the text label, making it just big enough for the text to fit it
 */
void VToolSeamAllowance::UpdateDetailLabel()
{

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    qDebug() << "Update Piece label: " << piece.GetName();
    const VPieceLabelData& labelData = piece.GetPatternPieceData();

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
 * @brief UpdatePatternInfo updates the pattern info label
 */
void VToolSeamAllowance::UpdatePatternInfo()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    qDebug() << "Update Pattern label: " << piece.GetName();
    const VPatternLabelData& geom = piece.GetPatternInfo();

    if (geom.IsVisible() & qApp->Settings()->showLabels())
    {
        QPointF pos;
        qreal labelAngle = 0;

        if (PrepareLabelData(geom, m_patternInfo, pos, labelAngle))
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
void VToolSeamAllowance::UpdateGrainline()
{
    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    const VGrainlineData& data = piece.GetGrainlineGeometry();

    qDebug()<<"Update Grainline IsVisible() = "<< data.IsVisible();

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
 * @brief SaveMoveDetail saves the move piece operation to the undo stack
 */
void VToolSeamAllowance::SaveMoveDetail(const QPointF& ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetPos(ptPos);

    SavePieceOptions* moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move pattern piece label"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveResizeDetail saves the resize piece label operation to the undo stack
 */
void VToolSeamAllowance::SaveResizeDetail(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelWidth(QString().setNum(dLabelW));
    const qreal height = FromPixel(m_dataLabel->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternPieceData().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternPieceData().SetFontSize(iFontSize);

    SavePieceOptions* resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize pattern piece label"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveRotationDetail saves the rotation piece label operation to the undo stack
 */
void VToolSeamAllowance::SaveRotationDetail(qreal dRot)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetPos(m_dataLabel->pos());
    newPiece.GetPatternPieceData().SetFontSize(m_dataLabel->getFontSize());

    // Transform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newPiece.GetPatternPieceData().SetRotation(QString().setNum(line.angle()));

    SavePieceOptions* rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate pattern piece label"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveMovePattern saves the pattern label position
 */
void VToolSeamAllowance::SaveMovePattern(const QPointF &ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetPos(ptPos);

    SavePieceOptions* moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move pattern info label"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief: SaveResizePattern saves the pattern label width and font size
 */
void VToolSeamAllowance::SaveResizePattern(qreal dLabelW, int iFontSize)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLabelW = FromPixel(dLabelW, *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelWidth(QString().setNum(dLabelW));
    qreal height = FromPixel(m_patternInfo->boundingRect().height(), *VDataTool::data.GetPatternUnit());
    newPiece.GetPatternInfo().SetLabelHeight(QString().setNum(height));
    newPiece.GetPatternInfo().SetFontSize(iFontSize);

    SavePieceOptions* resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize pattern info label"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveRotationPattern(qreal dRot)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    newPiece.GetPatternInfo().SetPos(m_patternInfo->pos());
    newPiece.GetPatternInfo().SetFontSize(m_patternInfo->getFontSize());

    // Transform angle to anticlockwise
    QLineF line(0, 0, 100, 0);
    line.setAngle(-dRot);
    newPiece.GetPatternInfo().SetRotation(QString().setNum(line.angle()));

    SavePieceOptions* rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate pattern info label"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}


//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveMoveGrainline(const QPointF& ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    qDebug() << "******* new grainline pos" << ptPos;

    SavePieceOptions* moveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    moveCommand->setText(tr("move grainline"));
    connect(moveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveResizeGrainline(qreal dLength)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    dLength = FromPixel(dLength, *VDataTool::data.GetPatternUnit());
    newPiece.GetGrainlineGeometry().SetPos(m_grainLine->pos());
    newPiece.GetGrainlineGeometry().SetLength(QString().setNum(dLength));
    SavePieceOptions* resizeCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    resizeCommand->setText(tr("resize grainline"));
    connect(resizeCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(resizeCommand);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveRotateGrainline(qreal dRot, const QPointF& ptPos)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;

    newPiece.GetGrainlineGeometry().SetRotation(QString().setNum(qRadiansToDegrees(dRot)));
    newPiece.GetGrainlineGeometry().SetPos(ptPos);
    SavePieceOptions* rotateCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    rotateCommand->setText(tr("rotate grainline"));
    connect(rotateCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(rotateCommand);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolDetail::paint draws a bounding box around piece, if one of its text or grainline items is not idle.
 */
void VToolSeamAllowance::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

    //set seamline, grainline, & labels pen
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


        this->setPen(QPen(color, scaleWidth(lineWeight, sceneScale(scene())),
                       lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));

        this->setBrush(QBrush(QColor(Qt::white), Qt::SolidPattern));

        //set notches pen
        color = QColor(qApp->Settings()->getDefaultNotchColor());
        m_notches->setPen(QPen(color, scaleWidth(lineWeight,
        sceneScale(scene())), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }

    if ((m_dataLabel->IsIdle() == false
            || m_patternInfo->IsIdle() == false
            || m_grainLine->IsIdle() == false) && not isSelected())
    {
        setSelected(true);
    }
    QGraphicsPathItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VToolSeamAllowance::boundingRect() const
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
QPainterPath VToolSeamAllowance::shape() const
{
    if (m_mainPath == QPainterPath())
    {
        return QGraphicsPathItem::shape();
    }
    else
    {
        return ItemShapeFromPath(m_mainPath, pen());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::AddToFile()
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
    AddPins(doc, domElement, piece.GetPins());

    AddPiece *addDet = new AddPiece(domElement, doc, piece, m_drawName);
    connect(addDet, &AddPiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(addDet);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::RefreshDataInFile()
{
    QDomElement domElement = doc->elementById(m_id, getTagName());
    if (domElement.isElement())
    {
        // Refresh only parts that we possibly need to update
        {
            // TODO. Delete if minimal supported version is 0.4.0
            Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 4, 0),
                              "Time to refactor the code.");

            const uint version = doc->GetParametrUInt(domElement, VToolSeamAllowance::AttrVersion, "1");
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
                AddPins(doc, domElement, piece.GetPins());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVariant VToolSeamAllowance::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
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
                    const int xmargin = 50;
                    const int ymargin = 50;

                    const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(view);
                    const QRectF itemRect = mapToScene(boundingRect()|childrenBoundingRect()).boundingRect();

                    // If item's rect is bigger than view's rect ensureVisible works very unstable.
                    if (itemRect.height() + 2*ymargin < viewRect.height() &&
                        itemRect.width() + 2*xmargin < viewRect.width())
                    {
                        view->ensureVisible(itemRect, xmargin, ymargin);
                    }
                    else
                    {
                        // Ensure visible only small rect around a cursor
                        VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                        SCASSERT(currentScene);
                        const QPointF cursorPosition = currentScene->getScenePos();
                        view->ensureVisible(QRectF(cursorPosition.x()-5/scale, cursorPosition.y()-5/scale,
                                                   10/scale, 10/scale));
                    }
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
void VToolSeamAllowance::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
        doc->SelectedDetail(m_id);
        emit chosenTool(m_id, SceneObject::Detail);
    }

    event->accept();// Special for not selectable item first need to call standard mousePressEvent then accept event
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    QGraphicsPathItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //Disable cursor-arrow-openhand
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }
    QGraphicsPathItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_suppressContextMenu)
    {
        return;
    }

    VPiece piece = VAbstractTool::data.GetPiece(m_id);

    QMenu menu;
    QAction *editProperties = menu.addAction(QIcon::fromTheme("preferences-other"), tr("Properties") + "\tP");

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    menu.addAction(separator);

    QAction *inLayoutOption = menu.addAction(tr("Include in Layout") + "\tI");
    inLayoutOption->setCheckable(true);
    inLayoutOption->setChecked(piece.IsInLayout());

    QAction *forbidFlipping = menu.addAction(tr("Forbid Flipping") + "\tF");
    forbidFlipping->setCheckable(true);
    forbidFlipping->setChecked(piece.IsForbidFlipping());

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
    qDebug()<<"Grainline IsVisible() = "<< piece.GetGrainlineGeometry().IsVisible();
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
    QAction *deletePiece = menu.addAction(QIcon::fromTheme("edit-delete"), tr("Delete") + "\tDel");
    _referens > 1 ? deletePiece->setEnabled(false) : deletePiece->setEnabled(true);

    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == editProperties)
    {
        editPieceProperties();
    }
    else if (selectedAction == inLayoutOption)
    {
        toggleInLayout(selectedAction->isChecked());
    }
    else if (selectedAction == forbidFlipping)
    {
        toggleFlipping(selectedAction->isChecked());
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
        catch(const VExceptionToolWasDeleted &e)
        {
            Q_UNUSED(e);
            return;//Leave this method immediately!!!
        }
        return; //Leave this method immediately after call!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::keyReleaseEvent(QKeyEvent *event)
{
    VPiece piece = VAbstractTool::data.GetPiece(m_id);
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                deleteTool();
            }
            catch(const VExceptionToolWasDeleted &e)
            {
                Q_UNUSED(e);
                return;//Leave this method immediately!!!
            }
            break;

        case Qt::Key_P:
            editPieceProperties();
            break;

        case Qt::Key_I:
            {
                toggleInLayout(!piece.IsInLayout());
                break;
            }

        case Qt::Key_F:
            {
                toggleFlipping(!piece.IsForbidFlipping());
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
            renamePiece(piece);
            break;
        default:
            break;
    }

    QGraphicsPathItem::keyReleaseEvent (event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SetDialog()
{
    SCASSERT(not m_dialog.isNull());
    QSharedPointer<DialogSeamAllowance> dialogTool = m_dialog.objectCast<DialogSeamAllowance>();
    SCASSERT(not dialogTool.isNull())
    dialogTool->SetPiece(VAbstractTool::data.GetPiece(m_id));
    dialogTool->EnableApply(true);
}

//---------------------------------------------------------------------------------------------------------------------
VToolSeamAllowance::VToolSeamAllowance(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                       const Source &typeCreation, VMainGraphicsScene *scene,
                                       const QString &drawName, QGraphicsItem *parent)
    : VInteractiveTool(doc, data, id)
    , QGraphicsPathItem(parent)
    , m_mainPath()
    , m_pieceRect()
    , m_cutPath()
    , m_pieceScene(scene)
    , m_drawName(drawName)
    , m_seamLine(new VNoBrushScalePathItem(this))
    , m_cutLine(new VNoBrushScalePathItem(this))
    , m_dataLabel(new VTextGraphicsItem(this))
    , m_patternInfo(new VTextGraphicsItem(this))
    , m_grainLine(new VGrainlineItem(this))
    , m_notches(new QGraphicsPathItem(this))
{
    VPiece piece = data->GetPiece(id);
    InitNodes(piece, scene);
    InitCSAPaths(piece);
    InitInternalPaths(piece);
    InitPins(piece);
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    RefreshGeometry();

    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    connect(scene, &VMainGraphicsScene::EnableToolMove, this, &VToolSeamAllowance::EnableToolMove);
    connect(scene, &VMainGraphicsScene::ItemClicked,    this, &VToolSeamAllowance::ResetChildren);
    ToolCreation(typeCreation);
    setAcceptHoverEvents(true);

    connect(m_dataLabel, &VTextGraphicsItem::itemMoved,   this, &VToolSeamAllowance::SaveMoveDetail);
    connect(m_dataLabel, &VTextGraphicsItem::itemResized, this, &VToolSeamAllowance::SaveResizeDetail);
    connect(m_dataLabel, &VTextGraphicsItem::itemRotated, this, &VToolSeamAllowance::SaveRotationDetail);

    connect(m_patternInfo, &VTextGraphicsItem::itemMoved,   this, &VToolSeamAllowance::SaveMovePattern);
    connect(m_patternInfo, &VTextGraphicsItem::itemResized, this, &VToolSeamAllowance::SaveResizePattern);
    connect(m_patternInfo, &VTextGraphicsItem::itemRotated, this, &VToolSeamAllowance::SaveRotationPattern);

    connect(m_grainLine, &VGrainlineItem::itemMoved,   this, &VToolSeamAllowance::SaveMoveGrainline);
    connect(m_grainLine, &VGrainlineItem::itemResized, this, &VToolSeamAllowance::SaveResizeGrainline);
    connect(m_grainLine, &VGrainlineItem::itemRotated, this, &VToolSeamAllowance::SaveRotateGrainline);

    connect(doc, &VAbstractPattern::UpdatePatternLabel, this, &VToolSeamAllowance::UpdatePatternInfo);
    connect(doc, &VAbstractPattern::CheckLayout,        this, &VToolSeamAllowance::updatePieceDetails);

    connect(m_pieceScene, &VMainGraphicsScene::DimensionsChanged, this, &VToolSeamAllowance::updatePieceDetails);
    connect(m_pieceScene, &VMainGraphicsScene::LanguageChanged,   this, &VToolSeamAllowance::retranslateUi);

    updatePieceDetails();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateExcludeState()
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
void VToolSeamAllowance::RefreshGeometry()
{
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    m_cutLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

    const VPiece piece = VAbstractTool::data.GetPiece(m_id);
    QPainterPath path = piece.MainPathPath(this->getData());

    if (!piece.isHideSeamLine() || !piece.IsSeamAllowance() || piece.IsSeamAllowanceBuiltIn())
    {
        m_mainPath = QPainterPath();
        m_pieceRect = QRectF();
        m_cutLine->setBrush(QBrush(QColor(qApp->Settings()->getDefaultCutColor()), Qt::Dense7Pattern));
    }
    else
    {
        m_cutLine->setBrush(QBrush(Qt::NoBrush)); // Disable if the main path was hidden
        // need for returning a bounding rect when main path is not visible
        m_mainPath = path;
        m_pieceRect = m_mainPath.controlPointRect();
        path = QPainterPath();
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
        m_cutPath = piece.SeamAllowancePath(seamAllowancePoints);
        m_cutPath.setFillRule(Qt::OddEvenFill);
        path.setFillRule(Qt::OddEvenFill);
        m_cutLine->setPath(m_cutPath);
        m_pieceRect = m_cutLine->boundingRect();
    }
    else
    {
        m_cutLine->setPath(QPainterPath());
    }

    this->setPos(piece.GetMx(), piece.GetMy());
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::SaveDialogChange()
{
    SCASSERT(not m_dialog.isNull());
    DialogSeamAllowance *dialogTool = qobject_cast<DialogSeamAllowance*>(m_dialog.data());
    SCASSERT(dialogTool != nullptr);
    const VPiece newPiece = dialogTool->GetPiece();
    const VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);

    SavePieceOptions *saveCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    connect(saveCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(saveCommand);
    UpdateDetailLabel();
}

//---------------------------------------------------------------------------------------------------------------------
VPieceItem::MoveTypes VToolSeamAllowance::FindLabelGeometry(const VPatternLabelData &labelData, qreal &rotationAngle,
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
    catch(qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    const quint32 topLeftPin = labelData.TopLeftPin();
    const quint32 bottomRightPin = labelData.BottomRightPin();

    if (topLeftPin != NULL_ID && bottomRightPin != NULL_ID)
    {
        try
        {
            const auto topLeftPinPoint = VAbstractTool::data.GeometricObject<VPointF>(topLeftPin);
            const auto bottomRightPinPoint = VAbstractTool::data.GeometricObject<VPointF>(bottomRightPin);

            const QRectF labelRect = QRectF(static_cast<QPointF>(*topLeftPinPoint),
                                            static_cast<QPointF>(*bottomRightPinPoint));
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
    catch(qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    const quint32 centerPin = labelData.CenterPin();
    if (centerPin != NULL_ID)
    {
        try
        {
            const auto centerPinPoint = VAbstractTool::data.GeometricObject<VPointF>(centerPin);
            qDebug() << " Anchor center point: " << centerPinPoint;
            const qreal lWidth = ToPixel(labelWidth, *VDataTool::data.GetPatternUnit());
            const qreal lHeight = ToPixel(labelHeight, *VDataTool::data.GetPatternUnit());
            qDebug() << " Label pixel width: " << lWidth;
            qDebug() << " Label pixel height: " << lHeight;
            pos = static_cast<QPointF>(*centerPinPoint) - QRectF(0, 0, lWidth, lHeight).center();
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
VPieceItem::MoveTypes VToolSeamAllowance::FindGrainlineGeometry(const VGrainlineData& data, qreal &length,
                                                               qreal &rotationAngle, QPointF &pos)
{
    const quint32 topPin = data.TopPin();
    const quint32 bottomPin = data.BottomPin();

    if (topPin != NULL_ID && bottomPin != NULL_ID)
    {
        try
        {
            const auto topPinPoint = VAbstractTool::data.GeometricObject<VPointF>(topPin);
            const auto bottomPinPoint = VAbstractTool::data.GeometricObject<VPointF>(bottomPin);

            QLineF grainline(static_cast<QPointF>(*bottomPinPoint), static_cast<QPointF>(*topPinPoint));
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
    catch(qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return VPieceItem::Error;
    }

    const quint32 centerPin = data.CenterPin();
    if (centerPin != NULL_ID)
    {
        try
        {
            const auto centerPinPoint = VAbstractTool::data.GeometricObject<VPointF>(centerPin);

            const qreal cLength = ToPixel(length, *VDataTool::data.GetPatternUnit());
            QLineF grainline(centerPinPoint->x(), centerPinPoint->y(),
                             centerPinPoint->x() + cLength / 2.0, centerPinPoint->y());

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
void VToolSeamAllowance::InitNodes(const VPiece &piece, VMainGraphicsScene *scene)
{
    for (int i = 0; i< piece.GetPath().CountNodes(); ++i)
    {
        InitNode(piece.GetPath().at(i), scene, &(VAbstractTool::data), doc, this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitNode(const VPieceNode &node, VMainGraphicsScene *scene, VContainer *data,
                                  VAbstractPattern *doc, VToolSeamAllowance *parent)
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
            qDebug()<<"Get wrong tool type. Ignore.";
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitCSAPaths(const VPiece &piece)
{
    for (int i = 0; i < piece.GetCustomSARecords().size(); ++i)
    {
        doc->IncrementReferens(piece.GetCustomSARecords().at(i).path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::InitInternalPaths(const VPiece &piece)
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
void VToolSeamAllowance::InitPins(const VPiece &piece)
{
    for (int i = 0; i < piece.GetPins().size(); ++i)
    {
        doc->IncrementReferens(piece.GetPins().at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::deleteTool(bool ask)
{
    QScopedPointer<DeletePiece> delDet(new DeletePiece(doc, m_id, VAbstractTool::data.GetPiece(m_id)));
    if (ask)
    {
        if (ConfirmDeletion() == QMessageBox::No)
        {
            return;
        }
        /* If UnionDetails tool delete piece no need emit FullParsing.*/
        connect(delDet.data(), &DeletePiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    }

    // If UnionDetails tool delete the piece this object will be deleted only after full parse.
    // Deleting inside UnionDetails cause crash.
    // Because this object should be inactive from no one we disconnect all signals that may cause a crash
    // KEEP THIS LIST ACTUAL!!!
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

    qApp->getUndoStack()->push(delDet.take());

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    VExceptionToolWasDeleted e("Tool was used after deleting.");
    throw e;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::ToolCreation(const Source &typeCreation)
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
bool VToolSeamAllowance::PrepareLabelData(const VPatternLabelData &labelData, VTextGraphicsItem *labelItem,
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

    qDebug() << " Label Width = " << labelWidth;
    qDebug() << " Label Pixel Width = " << ToPixel(labelWidth, *VDataTool::data.GetPatternUnit());
    qDebug() << " Label Height = " << labelHeight;
    qDebug() << " Label Pixel Height = " << ToPixel(labelHeight, *VDataTool::data.GetPatternUnit());

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSeamAllowance::UpdateLabelItem(VTextGraphicsItem *labelItem, QPointF pos, qreal labelAngle)
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
void VToolSeamAllowance::editPieceProperties()
{
    QSharedPointer<DialogSeamAllowance> dialog =
            QSharedPointer<DialogSeamAllowance>(new DialogSeamAllowance(getData(), m_id, qApp->getMainWindow()));
    dialog->EnableApply(true);
    m_dialog = dialog;
    m_dialog->setModal(true);
    connect(m_dialog.data(), &DialogTool::DialogClosed, this, &VToolSeamAllowance::FullUpdateFromGuiOk);
    connect(m_dialog.data(), &DialogTool::DialogApplied, this, &VToolSeamAllowance::FullUpdateFromGuiApply);
    SetDialog();
    m_dialog->show();
}

/**
 * @brief toggleInLayout - routine to toggle if pattern piece is included and visible in layout.
 * @param checked - true if piece is included.
 */
void VToolSeamAllowance::toggleInLayout(bool checked)
{
    TogglePieceInLayout *togglePiece = new TogglePieceInLayout(m_id, checked,
                                                               &(VAbstractTool::data), doc);
    connect(togglePiece, &TogglePieceInLayout::UpdateList, doc, &VAbstractPattern::CheckInLayoutList);
    qApp->getUndoStack()->push(togglePiece);

    showStatus(tr("Include piece in layout changed: ") + (checked ? tr("Include") : tr("Exclude")));
}

/**
 * @brief toggleFlipping - routine to toggle forbidding flipping.
 * @param checked - true if flipping is forbidden.
 */
void VToolSeamAllowance::toggleFlipping(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetForbidFlipping(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Forbid Flipping"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Forbid Flipping changed: ") + (checked ? tr("Enabled") : tr("Disabled")));
}

/**
 * @brief toggleSeamLine - routine to toggle the visibility of the seam line.
 * @param checked - true if seam line is visible.
 */
void VToolSeamAllowance::toggleSeamLine(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.setHideSeamLine(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Hide Seam Line"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam line visibility changed: ") + (checked ? tr("Hide") : tr("Show")));
}

/**
 * @brief toggleSeamAllowance - routine to toggle the visibility of the seam allowance.
 * @param checked - true if seam allowance is visible.
 */
void VToolSeamAllowance::toggleSeamAllowance(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.SetSeamAllowance(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show seam allowance"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Seam allowance visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief toggleGrainline - routine to toggle the visibility of the  piece grainline.
 * @param checked - true if grainline is visible.
 */
void VToolSeamAllowance::toggleGrainline(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetGrainlineGeometry().SetVisible(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show grainline"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Grainline visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief togglePatternLabel - routine to toggle the visibility of the  pattern label.
 * @param checked - true if pattern label is visible.
 */
void VToolSeamAllowance::togglePatternLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternInfo().SetVisible(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show pattern label"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Pattern label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

/**
 * @brief togglePieceLabel - routine to toggle the visibility of the piece label.
 * @param checked - true if piece label is visible.
 */
void VToolSeamAllowance::togglePieceLabel(bool checked)
{
    VPiece oldPiece = VAbstractTool::data.GetPiece(m_id);
    VPiece newPiece = oldPiece;
    newPiece.GetPatternPieceData().SetVisible(checked);

    SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
    undoCommand->setText(tr("Show piece label"));
    connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(undoCommand);

    showStatus(tr("Piece label visibility changed: ") + (checked ? tr("Show") : tr("Hide")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief renamePiece - routine to rename pattern piece.
 */
void VToolSeamAllowance::renamePiece(VPiece piece)
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

        SavePieceOptions* undoCommand = new SavePieceOptions(oldPiece, newPiece, doc, m_id);
        undoCommand->setText(tr("Rename pattern piece"));
        connect(undoCommand, &SavePieceOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(undoCommand);

        showStatus(tr("Piece renamed to: ") + pieceName);
    }
}

void VToolSeamAllowance::showStatus(QString toolTip)
{
    VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
    SCASSERT(window != nullptr)

    window->ShowToolTip(toolTip);
}

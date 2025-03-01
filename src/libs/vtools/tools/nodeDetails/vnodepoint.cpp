//******************************************************************************
//  @file   vnodepoint.cpp
//  @author Douglas S Caskey
//  @date  1 Mar, 2025
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
//  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
//******************************************************************************

//******************************************************************************
//
//   @file   vnodepoint.cpp
//   @author Roman Telezhynskyi <dismine(at)gmail.com>
//   @date   November 15, 2013
//
//   @brief
//   @copyright
//   This source code is part of the Valentine project, a pattern making
//   program, whose allow create and modeling patterns of clothing.
//   Copyright (C) 2013-2015 Seamly2D project
//   <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//   Seamly2D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Seamly2D is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//
//******************************************************************************

#include "vnodepoint.h"

#include <QBrush>
#include <QDomElement>
#include <QFlags>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <Qt>
#include <QIcon>
#include <QMenu>
#include <QToolTip>
#include <QRect>
#include <new>

#include "../../../vgeometry/vpointf.h"
#include "../../../vwidgets/vgraphicssimpletextitem.h"
#include "../../undocommands/savepieceoptions.h"
#include "../../undocommands/label/showpointname.h"
#include "../../undocommands/label/movelabel.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vabstracttool.h"
#include "../vdatatool.h"
#include "../pattern_piece_tool.h"
#include "vabstractnode.h"

const QString VNodePoint::ToolType = QStringLiteral("modeling");

//******************************************************************************
/// @brief VNodePoint constructor.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
/// @param idPoint object id in containerPoint.
/// @param typeCreation way we create this tool.
/// @param idTool tool id.
/// @param qoParent QObject parent
/// @param parent parent object.
//******************************************************************************
VNodePoint::VNodePoint(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 idPoint, const Source &typeCreation,
                       const QString &blockName, const quint32 &idTool, QObject *qoParent, QGraphicsItem *parent)
    : VAbstractNode(doc, data, id, idPoint, blockName, idTool, qoParent)
    , VScenePoint(QColor(qApp->Settings()->getPointNameColor()), parent)
{
    m_pointName->setShowParentTooltip(false);
    connect(m_pointName, &VGraphicsSimpleTextItem::pointChosen,         this, &VNodePoint::pointChosen);
    connect(m_pointName, &VGraphicsSimpleTextItem::nameChangedPosition, this, &VNodePoint::nameChangedPosition);
    connect(m_pointName, &VGraphicsSimpleTextItem::showContextMenu,
            this, [this](QGraphicsSceneContextMenuEvent *event)
    {
        contextMenuEvent(event);
    });

    refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(id));
    ToolCreation(typeCreation);
    setCursor(Qt::ArrowCursor);
}

//******************************************************************************
/// @brief Create help create tool.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
/// @param idPoint object id in containerPoint.
/// @param parse parser file mode.
/// @param typeCreation way we create this tool.
/// @param idTool tool id.
//******************************************************************************
void VNodePoint::Create(VAbstractPattern *doc, VContainer *data, VMainGraphicsScene *scene,
                        quint32 id, quint32 idPoint, const Document &parse,
                        const Source &typeCreation, const QString &blockName, const quint32 &idTool)
{
    if (parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(id, Tool::NodePoint, doc);
        //TODO Need create garbage collector and remove all nodes that we don't use.
        //Better check garbage before each saving file. Check only modeling tags.
        VNodePoint *point = new VNodePoint(doc, data, id, idPoint, typeCreation, blockName, idTool, doc);

        connect(scene, &VMainGraphicsScene::EnableToolMove,           point, &VNodePoint::EnableToolMove);
        connect(scene, &VMainGraphicsScene::EnablePointItemHover,     point, &VNodePoint::AllowHover);
        connect(scene, &VMainGraphicsScene::EnablePointItemSelection, point, &VNodePoint::AllowSelecting);
        connect(scene, &VMainGraphicsScene::enableTextItemHover,      point, &VNodePoint::allowTextHover);
        connect(scene, &VMainGraphicsScene::enableTextItemSelection,  point, &VNodePoint::allowTextSelectable);
        VAbstractPattern::AddTool(id, point);
        if (idTool != NULL_ID)
        {
            //Some nodes we don't show on scene. Tool that create this node must free memory.
            VDataTool *tool = VAbstractPattern::getTool(idTool);
            SCASSERT(tool != nullptr)
            point->setParent(tool); // Adopted by a tool
        }
        else
        {
            // Try to prevent memory leak
            scene->addItem(point); // First adopted by scene
            point->hide();         // If no one will use node, it will stay hidden
            point->SetParentType(ParentType::Scene);
        }
    }
    else
    {
        doc->UpdateToolData(id, data);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VNodePoint::getTagName() const
{
    return VAbstractPattern::TagPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::pointChosen()
{
    emit chosenTool(m_id, SceneObject::Point);
}

//******************************************************************************
/// @brief FullUpdateFromFile update tool data form file.
//******************************************************************************
void VNodePoint::FullUpdateFromFile()
{
    try
    {
        refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
    }
    catch (const VExceptionBadId &)
    {
        // do nothing
    }
}

//******************************************************************************
/// @brief AddToFile add tag with Information about tool into file.
//******************************************************************************
void VNodePoint::AddToFile()
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    QDomElement domElement = doc->createElement(getTagName());

    doc->SetAttribute(domElement, VDomDocument::AttrId, m_id);
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrIdObject, idNode);
    doc->SetAttribute(domElement, AttrMx, qApp->fromPixel(point->mx()));
    doc->SetAttribute(domElement, AttrMy, qApp->fromPixel(point->my()));
    doc->SetAttribute<bool>(domElement, AttrShowPointName, point->isShowPointName());

    if (idTool != NULL_ID)
    {
        doc->SetAttribute(domElement, AttrIdTool, idTool);
    }

    AddToModeling(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    VScenePoint::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not (flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    event->accept();// Special for not selectable item first need to call standard mousePressEvent then accept event
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //Use QToolTip::showText() to offset tooltip so it's not hidden under cursor
    QPoint point = event->screenPos();
    QToolTip::showText(QPoint(point.x()+10, point.y()+10), VAbstractTool::data.GetGObject(m_id)->name());
    VScenePoint::hoverEnterEvent(event);
}

//******************************************************************************
/// @brief mouseReleaseEvent handle mouse release events.
/// @param event mouse release event.
//******************************************************************************
void VNodePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit chosenTool(m_id, SceneObject::Point);
    }
    VScenePoint::mouseReleaseEvent(event);
}

//******************************************************************************
/// @brief setPointNamePosition change name text position.
/// @param pos new position.
//******************************************************************************
void VNodePoint::setPointNamePosition(quint32 id, const QPointF &pos)
{
    if (id == m_id)
    {
        QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setMx(pos.x());
        point->setMy(pos.y());
        m_pointName->blockSignals(true);
        m_pointName->setPosition(pos);
        m_pointName->blockSignals(false);
        refreshLeader();

        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, qApp->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::setPointNameVisiblity(quint32 id, bool visible)
{
    if (id == m_id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setShowPointName(visible);
        refreshPointGeometry(*point);
    }
}

//******************************************************************************
/// @brief nameChangedPosition point name change position.
/// @param pos new position.
//******************************************************************************
void VNodePoint::nameChangedPosition(const QPointF &pos)
{
    qApp->getUndoStack()->push(new MoveLabel(doc, pos - this->pos(), m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::ShowNode()
{
    if (parentType != ParentType::Scene && not m_exluded)
    {
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::HideNode()
{
    hide();
}

//******************************************************************************
/// @brief contextMenuEvent show context menu for point nodes.
/// @param event  contextmenu event.
//******************************************************************************
void VNodePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_suppressContextMenu)
    {
        return;
    }

    PatternPieceTool *tool = qgraphicsitem_cast<PatternPieceTool *>(parentItem());
    if (tool)
    {
        QMenu menu;

        const VPiece piece = VAbstractTool::data.GetPiece(tool->getId());
        const int index = piece.GetPath().indexOfNode(m_id);
        VPieceNode node = piece.GetPath().at(index);

        QAction *actionShowPointName = menu.addAction(QIcon("://icon/16x16/open_eye.png"), tr("Show Point Name"));
        actionShowPointName->setCheckable(true);
        actionShowPointName->setChecked(VAbstractTool::data.GeometricObject<VPointF>(m_id)->isShowPointName());

        //Setup actions for Seam Allowance corner type sub menu
        QAction *actionByLength        = nullptr;
        QAction *actionByIntersection  = nullptr;
        QAction *actionFirstEdgeSym    = nullptr;
        QAction *actionSecondEdgeSym   = nullptr;
        QAction *actionFirstEdgeRight  = nullptr;
        QAction *actionSecondEdgeRight = nullptr;
        QMenu *angleTypeMenu  = menu.addMenu(tr("Seam Allowance Angle"));
        actionByLength        = angleTypeMenu->addAction(QIcon(), tr("By length"));
        actionByIntersection  = angleTypeMenu->addAction(QIcon(), tr("Intersection"));
        actionFirstEdgeSym    = angleTypeMenu->addAction(QIcon(), tr("First edge symmetry"));
        actionSecondEdgeSym   = angleTypeMenu->addAction(QIcon(), tr("Second edge symmetry"));
        actionFirstEdgeRight  = angleTypeMenu->addAction(QIcon(), tr("First edge right angle"));
        actionSecondEdgeRight = angleTypeMenu->addAction(QIcon(), tr("Second edge right angle"));

        // Set checkmark for current Seam Allowance corner type
        switch (static_cast<int>(node.GetAngleType()))
        {
            case (PieceNodeAngle::ByLength):
            {
                actionByLength->setCheckable(true);
                actionByLength->setChecked(true);
                break;
            }
            case (PieceNodeAngle::ByPointsIntersection):
            {
                actionByIntersection->setCheckable(true);
                actionByIntersection->setChecked(true);
                break;
            }
            case (PieceNodeAngle::ByFirstEdgeSymmetry):
            {
                actionFirstEdgeSym->setCheckable(true);
                actionFirstEdgeSym->setChecked(true);
                break;
            }
            case (PieceNodeAngle::BySecondEdgeSymmetry):
            {
                actionSecondEdgeSym->setCheckable(true);
                actionSecondEdgeSym->setChecked(true);
                break;
            }
            case (PieceNodeAngle::ByFirstEdgeRightAngle):
            {
                actionFirstEdgeRight->setCheckable(true);
                actionFirstEdgeRight->setChecked(true);
                break;
            }
            case (PieceNodeAngle::BySecondEdgeRightAngle):
            {
                actionSecondEdgeRight->setCheckable(true);
                actionSecondEdgeRight->setChecked(true);
                break;
            }
            default:
                break;
        }

        // Set default Notch values
        NotchType notchType = node.getNotchType();
        NotchSubType notchSubType = node.getNotchSubType();
        int notchCount = node.getNotchCount();
        bool isNotch = false;

        //Setup Notch menu
        QAction *actionNotch     = nullptr;
        QMenu *notchMenu = menu.addMenu(tr("Notch"));
        actionNotch = notchMenu->menuAction();
        actionNotch->setCheckable(true);
        actionNotch->setChecked(node.isNotch());

        //Setup Notch type submenu
        QAction *actionNone      = nullptr;
        QAction *actionSlit      = nullptr;
        QAction *actionTNotch    = nullptr;
        QAction *actionUNotch    = nullptr;
        QAction *actionVInternal = nullptr;
        QAction *actionVExternal = nullptr;
        QAction *actionCastle    = nullptr;
        QAction *actionDiamond   = nullptr;
        QMenu *notchTypeMenu = notchMenu->addMenu(tr("Type"));
        actionNone      = notchTypeMenu->addAction( tr("None") + QStringLiteral("\tShift + N"));
        actionSlit      = notchTypeMenu->addAction(QIcon("://icon/24x24/slit_notch.png"),       tr("Slit") + QStringLiteral("\tShift + S"));
        actionTNotch    = notchTypeMenu->addAction(QIcon("://icon/24x24/t_notch.png"),          tr("TNotch") + QStringLiteral("\tShift + T"));
        actionUNotch    = notchTypeMenu->addAction(QIcon("://icon/24x24/u_notch.png"),          tr("UNotch") + QStringLiteral("\tShift + U"));
        actionVInternal = notchTypeMenu->addAction(QIcon("://icon/24x24/internal_v_notch.png"), tr("VInternal") + QStringLiteral("\tShift + I"));
        actionVExternal = notchTypeMenu->addAction(QIcon("://icon/24x24/external_v_notch.png"), tr("VExternal") + QStringLiteral("\tShift + E"));
        actionCastle    = notchTypeMenu->addAction(QIcon("://icon/24x24/castle_notch.png"),     tr("Castle") + QStringLiteral("\tShift + C"));
        actionDiamond   = notchTypeMenu->addAction(QIcon("://icon/24x24/diamond_notch.png"),    tr("Diamond") + QStringLiteral("\tShift + D"));

        //Setup Notch subtype submenu
        QAction *actionStraightforward = nullptr;
        QAction *actionBisector        = nullptr;
        QAction *actionIntersection    = nullptr;
        QMenu *notchSubtypeMenu = notchMenu->addMenu(tr("Subtype"));
        actionStraightforward = notchSubtypeMenu->addAction(QIcon(), tr("Straightforward") + QStringLiteral("\tShift + F"));
        actionBisector        = notchSubtypeMenu->addAction(QIcon(), tr("Bisector") + QStringLiteral("\tShift + B"));
        actionIntersection    = notchSubtypeMenu->addAction(QIcon(), tr("Intersection") + QStringLiteral("\tShift + X"));

        //setup Notch count submenu
        QAction *action1Notch = nullptr;
        QAction *action2Notch = nullptr;
        QAction *action3Notch = nullptr;
        QMenu *notchCountMenu = notchMenu->addMenu(tr("Count"));
        action1Notch = notchCountMenu->addAction(QIcon(), QStringLiteral("1"));
        action2Notch = notchCountMenu->addAction(QIcon(), QStringLiteral("2"));
        action3Notch = notchCountMenu->addAction(QIcon(), QStringLiteral("3"));

        //Setup Exclude node menu
        QAction *actionExcludeNode = menu.addAction(QIcon(), tr("Excluded"));

        //Setup Delete Node menu
        QAction *actionDeleteNode = menu.addAction(QIcon(QIcon::fromTheme("edit-delete")), tr("Delete"));

        //Execute menu at current mouse position and handle selected item. 
        QAction *selectedAction = menu.exec(event->screenPos());
        if (selectedAction == actionShowPointName)
        {
            qApp->getUndoStack()->push(new ShowPointName(doc, m_id, selectedAction->isChecked()));
        }
        else if (selectedAction == actionExcludeNode)
        {
            emit nodeExcluded(m_id);
        }
        else if (selectedAction == actionDeleteNode)
        {
            emit nodeDeleted(m_id);
        }
        else if (selectedAction == actionByLength)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::ByLength);
        }
        else if (selectedAction == actionByIntersection)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::ByPointsIntersection);
        }
        else if (selectedAction == actionFirstEdgeSym)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::ByFirstEdgeSymmetry);
        }
        else if (selectedAction == actionSecondEdgeSym)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::BySecondEdgeSymmetry);
        }
        else if (selectedAction == actionFirstEdgeRight)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::ByFirstEdgeRightAngle);
        }
        else if (selectedAction == actionSecondEdgeRight)
        {
            emit nodeAngleChanged(m_id, PieceNodeAngle::BySecondEdgeRightAngle);
        }
        else
        {
            if (selectedAction == actionNone)
            {
                isNotch = false;
                notchType = NotchType::Slit;
            }
            else if (selectedAction == actionSlit)
            {
                isNotch = true;
                notchType = NotchType::Slit;
            }
            else if (selectedAction == actionTNotch)
            {
                isNotch = true;
                notchType = NotchType::TNotch;
            }
            else if (selectedAction == actionUNotch)
            {
                isNotch = true;
                notchType = NotchType::UNotch;
            }
            else if (selectedAction == actionVInternal)
            {
                isNotch = true;
                notchType = NotchType::VInternal;
            }
            else if (selectedAction == actionVExternal)
            {
                isNotch = true;
                notchType = NotchType::VExternal;
            }
            else if (selectedAction == actionCastle)
            {
                isNotch = true;
                notchType = NotchType::Castle;
            }
            else if (selectedAction == actionDiamond)
            {
                isNotch = true;
                notchType = NotchType::Diamond;
            }
            else if (selectedAction == actionStraightforward)
            {
                isNotch = true;
                notchSubType = NotchSubType::Straightforward;
            }
            else if (selectedAction == actionBisector)
            {
                isNotch = true;
                notchSubType = NotchSubType::Bisector;
            }
            else if (selectedAction == actionIntersection)
            {
                isNotch = true;
                notchSubType = NotchSubType::Intersection;
            }
            else if (selectedAction == action1Notch)
            {
                isNotch = true;
                notchCount = 1;
            }
            else if (selectedAction == action2Notch)
            {
                isNotch = true;
                notchCount = 2;
            }
            else if (selectedAction == action3Notch)
            {
                isNotch = true;
                notchCount = 3;
            }

            if (node.GetId() == m_id && node.GetTypeTool() == Tool::NodePoint)
            {
                NotchData notchData;
                notchData.isNotch = isNotch;
                notchData.type    = notchType;
                notchData.subType = notchSubType;
                notchData.length  = ToPixel(node.getNotchLength(), *VDataTool::data.GetPatternUnit());
                notchData.width   = ToPixel(node.getNotchWidth(),  *VDataTool::data.GetPatternUnit());
                notchData.angle   = node.getNotchAngle();
                notchData.count   = notchCount;

                emit notchChanged(m_id, notchData);
            }
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::EnableToolMove(bool move)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::allowTextHover(bool enabled)
{
    m_pointName->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::allowTextSelectable(bool enabled)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

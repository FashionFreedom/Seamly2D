//---------------------------------------------------------------------------------------------------------------------
//  @file   basepoint_tool.cpp
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
//  @file   vtoolbasepoint.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
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

#include "basepoint_tool.h"

#include <QApplication>
#include <QDomElement>
#include <QEvent>
#include <QFlags>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>
#include <QMessageBox>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QSharedPointer>
#include <QString>
#include <QUndoStack>
#include <new>

#include "vtoolsinglepoint.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../../vdrawtool.h"
#include "../../../vabstracttool.h"
#include "../../../vdatatool.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../dialogs/tools/basepoint_dialog.h"
#include "../../../../undocommands/add_draftblock.h"
#include "../../../../undocommands/delete_draftblock.h"
#include "../../../../undocommands/movespoint.h"

const QString BasePointTool::ToolType = QStringLiteral("single");

//---------------------------------------------------------------------------------------------------------------------
/// @brief BasePointTool constructor.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
/// @param type_creation way we create this tool.
/// @param parent parent object.
//---------------------------------------------------------------------------------------------------------------------
BasePointTool::BasePointTool (VAbstractPattern *doc, VContainer *data, quint32 id, const Source &type_creation,
                                const QString &draft_block_name, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(Qt::red), parent)
    , m_draft_block_name(draft_block_name)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    ToolCreation(type_creation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setDialog set dialog when user want change tool option.
//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<BasePointDialog> dialogTool = m_dialog.objectCast<BasePointDialog>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetData(point->name(), static_cast<QPointF>(*point));
}

//---------------------------------------------------------------------------------------------------------------------
BasePointTool *BasePointTool::Create(quint32 _id, const QString &active_draft_block, VPointF *point,
                                       VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                       const Document &parse, const Source &type_creation)
{
    SCASSERT(point != nullptr)

    quint32 id = _id;
    if (type_creation == Source::FromGui)
    {
        id = data->AddGObject(point);
    }
    else
    {
        data->UpdateGObject(id, point);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::BasePoint, doc);
        BasePointTool *base_point = new BasePointTool(doc, data, id, type_creation, active_draft_block);
        scene->addItem(base_point);
        InitToolConnections(scene, base_point);
        VAbstractPattern::AddTool(id, base_point);
        return base_point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::ShowVisualization(bool show)
{
    Q_UNUSED(show) //don't have any visualization for base point yet
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief AddToFile add tag with Information about tool into file.
//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::AddToFile()
{
    Q_ASSERT_X(!m_draft_block_name.isEmpty(), Q_FUNC_INFO, "name pattern piece is empty");

    QDomElement base_point = doc->createElement(getTagName());

    // Create Simple Point tag
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(base_point, obj);

    //Create draft block structure
    QDomElement draft_block = doc->createElement(VAbstractPattern::TagDraftBlock);
    doc->SetAttribute(draft_block, AttrName, m_draft_block_name);

    QDomElement calc_element = doc->createElement(VAbstractPattern::TagCalculation);
    calc_element.appendChild(base_point);

    draft_block.appendChild(calc_element);
    draft_block.appendChild(doc->createElement(VAbstractPattern::TagModeling));
    draft_block.appendChild(doc->createElement(VAbstractPattern::TagPieces));
    draft_block.appendChild(doc->createElement(VAbstractPattern::TagGroups));
    draft_block.appendChild(doc->createElement(VAbstractPattern::TagDraftImages));

    AddDraftBlock *undo_command = new AddDraftBlock(draft_block, doc, m_draft_block_name);
    connect(undo_command, &AddDraftBlock::ClearScene, doc, &VAbstractPattern::ClearScene);
    connect(undo_command, &AddDraftBlock::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(undo_command);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief itemChange handle tool change.
/// @param change change.
/// @param value value.
/// @return value.
//---------------------------------------------------------------------------------------------------------------------
QVariant BasePointTool::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene())
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool change_finished = true;
        if (change_finished)
        {
            change_finished = false;
            // value - this is new position.
            QPointF new_position = value.toPointF();

            MoveSPoint *undo_command = new MoveSPoint(doc, new_position.x(), new_position.y(), m_id, this->scene());
            connect(undo_command, &MoveSPoint::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            qApp->getUndoStack()->push(undo_command);
            const QList<QGraphicsView *> view_list = scene()->views();
            if (not view_list.isEmpty())
            {
                if (QGraphicsView *view = view_list.at(0))
                {
                    const int x_margin = 50;
                    const int y_margin = 50;

                    const QRectF view_rect = VMainGraphicsView::SceneVisibleArea(view);
                    const QRectF item_rect = mapToScene(boundingRect()).boundingRect();

                    // If item's rect is bigger than view's rect ensureVisible works very unstable.
                    if (item_rect.height() +  2 * y_margin < view_rect.height() &&
                        item_rect.width() + 2 * x_margin < view_rect.width())
                    {
                         view->ensureVisible(item_rect, x_margin, y_margin);
                    }
                    else
                    {
                        // Ensure visible only small rect around a cursor
                        VMainGraphicsScene *current_scene = qobject_cast<VMainGraphicsScene *>(scene());
                        SCASSERT(current_scene)
                        const QPointF cursor_position = current_scene->getScenePos();
                        view->ensureVisible(QRectF(cursor_position.x()-5, cursor_position.y()-5, 10, 10));
                    }
                }
            }
            change_finished = true;
        }
    }
    return VToolSinglePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
QPointF BasePointTool::GetBasePointPos() const
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    QPointF position(qApp->fromPixel(point->x()), qApp->fromPixel(point->y()));
    return position;
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::SetBasePointPos(const QPointF &pos)
{
    QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    point->setX(qApp->toPixel(pos.x()));
    point->setY(qApp->toPixel(pos.y()));

    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(point);

    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::deleteTool(bool ask)
{
    qCDebug(vTool, "Deleting base point.");
    qApp->getSceneView()->itemClicked(nullptr);
    if (ask)
    {
        qCDebug(vTool, "Asking.");
        if (ConfirmDeletion() == QMessageBox::No)
        {
            qCDebug(vTool, "User said no.");
            return;
        }
    }

    qCDebug(vTool, "Begin deleting.");
    DeleteDraftBlock *undo_command = new DeleteDraftBlock(doc, activeBlockName);
    connect(undo_command, &DeleteDraftBlock::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(undo_command);

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    VExceptionToolWasDeleted e("Tool was used after deleting.");
    throw e;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SaveDialog save options into file after change in dialog.
//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::SaveDialog(QDomElement &dom_element)
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<BasePointDialog> dialogTool = m_dialog.objectCast<BasePointDialog>();
    SCASSERT(!dialogTool.isNull())
    const QPointF point = dialogTool->GetPoint();
    const QString name = dialogTool->getPointName();
    doc->SetAttribute(dom_element, AttrName, name);
    doc->SetAttribute(dom_element, AttrX, QString().setNum(qApp->fromPixel(point.x())));
    doc->SetAttribute(dom_element, AttrY, QString().setNum(qApp->fromPixel(point.y())));
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolSinglePoint::hoverEnterEvent(event);

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    VToolSinglePoint::hoverLeaveEvent(event);

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        }
    }
    VToolSinglePoint::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }
    VToolSinglePoint::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    QSharedPointer<VPointF> point = qSharedPointerDynamicCast<VPointF>(obj);
    SCASSERT(point.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrX, qApp->fromPixel(point->x()));
    doc->SetAttribute(tag, AttrY, qApp->fromPixel(point->y()));
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::ReadToolAttributes(const QDomElement &dom_element)
{
    Q_UNUSED(dom_element) // This tool doesn't need to read attributes from file.
}

//---------------------------------------------------------------------------------------------------------------------
QString BasePointTool::makeToolTip() const
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QString tool_tip_string = QString("<table style=font-size:11pt; font-weight:600>"
                                            "<tr> <td><b>%1:</b> %2</td> </tr>"
                                            "</table>")
                                            .arg(tr("Name"))
                                            .arg(point->name());
    return tool_tip_string;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showContextMenu handle context menu events.
/// @param event context menu event.
//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    qCDebug(vTool, "Context menu base point");
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
    qCDebug(vTool, "Restored overridden cursor");
#endif

    try
    {
        if (doc->draftBlockCount() > 1)
        {
            qCDebug(vTool, "Draft Block count > 1");
            ContextMenu<BasePointDialog>(event, id, RemoveOption::Enable, Referens::Ignore);
        }
        else
        {
            qCDebug(vTool, "Draft Block count = 1");
            ContextMenu<BasePointDialog>(event, id, RemoveOption::Disable);
        }
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        qCDebug(vTool, "Tool was deleted. Leave method immediately.");
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
    qCDebug(vTool, "Context menu was closed. Tool was not deleted.");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief FullUpdateFromFile update tool data form file.
//---------------------------------------------------------------------------------------------------------------------
void  BasePointTool::FullUpdateFromFile()
{
    refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void BasePointTool::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);
    VToolSinglePoint::EnableToolMove(move);
}

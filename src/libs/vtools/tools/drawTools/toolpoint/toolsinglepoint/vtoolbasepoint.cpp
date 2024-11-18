//---------------------------------------------------------------------------------------------------------------------
//  @file   vtoolbasepoint.cpp
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

#include "vtoolbasepoint.h"

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
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
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
#include "../../../../dialogs/tools/dialogsinglepoint.h"
#include "../../../../undocommands/add_draftblock.h"
#include "../../../../undocommands/delete_draftblock.h"
#include "../../../../undocommands/movespoint.h"

const QString VToolBasePoint::ToolType = QStringLiteral("single");

//---------------------------------------------------------------------------------------------------------------------
/// @brief VToolBasePoint constructor.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
/// @param typeCreation way we create this tool.
/// @param parent parent object.
//---------------------------------------------------------------------------------------------------------------------
VToolBasePoint::VToolBasePoint (VAbstractPattern *doc, VContainer *data, quint32 id, const Source &typeCreation,
                                const QString &draftBlockName, QGraphicsItem *parent )
    : VToolSinglePoint(doc, data, id, QColor(Qt::red), parent)
    , draftBlockName(draftBlockName)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setDialog set dialog when user want change tool option.
//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogSinglePoint> dialogTool = m_dialog.objectCast<DialogSinglePoint>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetData(p->name(), static_cast<QPointF>(*p));
}

//---------------------------------------------------------------------------------------------------------------------
VToolBasePoint *VToolBasePoint::Create(quint32 _id, const QString &activeDraftBlock, VPointF *point,
                                       VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                       const Document &parse, const Source &typeCreation)
{
    SCASSERT(point != nullptr)

    quint32 id = _id;
    if (typeCreation == Source::FromGui)
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
        VToolBasePoint *spoint = new VToolBasePoint(doc, data, id, typeCreation, activeDraftBlock);
        scene->addItem(spoint);
        InitToolConnections(scene, spoint);
        VAbstractPattern::AddTool(id, spoint);
        return spoint;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::ShowVisualization(bool show)
{
    Q_UNUSED(show) //don't have any visualization for base point yet
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief AddToFile add tag with Information about tool into file.
//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::AddToFile()
{
    Q_ASSERT_X(not draftBlockName.isEmpty(), Q_FUNC_INFO, "name pattern piece is empty");

    QDomElement sPoint = doc->createElement(getTagName());

    // Create Simple Point tag
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(sPoint, obj);

    //Create draft block structure
    QDomElement draftblock = doc->createElement(VAbstractPattern::TagDraftBlock);
    doc->SetAttribute(draftblock, AttrName, draftBlockName);

    QDomElement calcElement = doc->createElement(VAbstractPattern::TagCalculation);
    calcElement.appendChild(sPoint);

    draftblock.appendChild(calcElement);
    draftblock.appendChild(doc->createElement(VAbstractPattern::TagModeling));
    draftblock.appendChild(doc->createElement(VAbstractPattern::TagPieces));
    draftblock.appendChild(doc->createElement(VAbstractPattern::TagGroups));
    draftblock.appendChild(doc->createElement(VAbstractPattern::TagDraftImages));

    AddDraftBlock *addPP = new AddDraftBlock(draftblock, doc, draftBlockName);
    connect(addPP, &AddDraftBlock::ClearScene, doc, &VAbstractPattern::ClearScene);
    connect(addPP, &AddDraftBlock::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(addPP);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief itemChange handle tool change.
/// @param change change.
/// @param value value.
/// @return value.
//---------------------------------------------------------------------------------------------------------------------
QVariant VToolBasePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
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
            QPointF newPos = value.toPointF();

            MoveSPoint *moveSP = new MoveSPoint(doc, newPos.x(), newPos.y(), m_id, this->scene());
            connect(moveSP, &MoveSPoint::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            qApp->getUndoStack()->push(moveSP);
            const QList<QGraphicsView *> viewList = scene()->views();
            if (not viewList.isEmpty())
            {
                if (QGraphicsView *view = viewList.at(0))
                {
                    const int xmargin = 50;
                    const int ymargin = 50;

                    const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(view);
                    const QRectF itemRect = mapToScene(boundingRect()).boundingRect();

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
                        SCASSERT(currentScene)
                        const QPointF cursorPosition = currentScene->getScenePos();
                        view->ensureVisible(QRectF(cursorPosition.x()-5, cursorPosition.y()-5, 10, 10));
                    }
                }
            }
            changeFinished = true;
        }
    }
    return VToolSinglePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief decrementReferens decrement referens parents objects.
//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::decrementReferens()
{
    if (_referens > 1)
    {
        --_referens;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VToolBasePoint::GetBasePointPos() const
{
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    QPointF pos(qApp->fromPixel(p->x()), qApp->fromPixel(p->y()));
    return pos;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::SetBasePointPos(const QPointF &pos)
{
    QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    p->setX(qApp->toPixel(pos.x()));
    p->setY(qApp->toPixel(pos.y()));

    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(p);

    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::deleteTool(bool ask)
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
    DeleteDraftBlock *deletePP = new DeleteDraftBlock(doc, activeBlockName);
    connect(deletePP, &DeleteDraftBlock::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(deletePP);

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    VExceptionToolWasDeleted e("Tool was used after deleting.");
    throw e;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SaveDialog save options into file after change in dialog.
//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogSinglePoint> dialogTool = m_dialog.objectCast<DialogSinglePoint>();
    SCASSERT(not dialogTool.isNull())
    const QPointF p = dialogTool->GetPoint();
    const QString name = dialogTool->getPointName();
    doc->SetAttribute(domElement, AttrName, name);
    doc->SetAttribute(domElement, AttrX, QString().setNum(qApp->fromPixel(p.x())));
    doc->SetAttribute(domElement, AttrY, QString().setNum(qApp->fromPixel(p.y())));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolSinglePoint::hoverEnterEvent(event);

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    VToolSinglePoint::hoverLeaveEvent(event);

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
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
void VToolBasePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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
void VToolBasePoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    QSharedPointer<VPointF> point = qSharedPointerDynamicCast<VPointF>(obj);
    SCASSERT(point.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrX, qApp->fromPixel(point->x()));
    doc->SetAttribute(tag, AttrY, qApp->fromPixel(point->y()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::ReadToolAttributes(const QDomElement &domElement)
{
    Q_UNUSED(domElement)
    // This tool doesn't need read attributes from file.
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolBasePoint::makeToolTip() const
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QString toolTipStr = QString("<table style=font-size:11pt; font-weight:600>"
                                       "<tr> <td><b>%1:</b> %2</td> </tr>"
                                       "</table>")
                                       .arg(tr("Name"))
                                       .arg(point->name());
    return toolTipStr;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showContextMenu handle context menu events.
/// @param event context menu event.
//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
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
            ContextMenu<DialogSinglePoint>(event, id, RemoveOption::Enable, Referens::Ignore);
        }
        else
        {
            qCDebug(vTool, "Draft Block count = 1");
            ContextMenu<DialogSinglePoint>(event, id, RemoveOption::Disable);
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
void  VToolBasePoint::FullUpdateFromFile()
{
    refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);
    VToolSinglePoint::EnableToolMove(move);
}

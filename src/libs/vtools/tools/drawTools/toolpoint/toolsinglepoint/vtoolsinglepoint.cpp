/***************************************************************************
 **  @file   vtoolsinglepoint.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   vtoolsinglepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vtoolsinglepoint.h"

#include <QBrush>
#include <QFlags>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QUndoStack>
#include <Qt>
#include <new>

#include "../../../../undocommands/label/movelabel.h"
#include "../../../../undocommands/label/showpointname.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/scalesceneitems.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../vabstractpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vToolSinglePoint, "v.toolSinglePoint")

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolSinglePoint constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param parent parent object.
 */
VToolSinglePoint::VToolSinglePoint(VAbstractPattern *doc, VContainer *data, quint32 id,
                                   const QColor &lineColor, QGraphicsItem *parent)
    : VAbstractPoint(doc, data, id)
    , VScenePoint(lineColor, parent)
{
    connect(m_pointName, &VGraphicsSimpleTextItem::showContextMenu,     this, &VToolSinglePoint::contextMenuEvent);
    connect(m_pointName, &VGraphicsSimpleTextItem::deleteTool,          this, &VToolSinglePoint::deletePoint);
    connect(m_pointName, &VGraphicsSimpleTextItem::pointChosen,         this, &VToolSinglePoint::pointChosen);
    connect(m_pointName, &VGraphicsSimpleTextItem::pointSelected,       this, &VToolSinglePoint::pointSelected);
    connect(m_pointName, &VGraphicsSimpleTextItem::nameChangedPosition, this, &VToolSinglePoint::pointnameChangedPosition);
    refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(id));
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolSinglePoint::name() const
{
    return ObjectName<VPointF>(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::setName(const QString &name)
{
    SetPointName(m_id, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::SetEnabled(bool enabled)
{
    setEnabled(enabled);
    m_pointLeader->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
bool VToolSinglePoint::isPointNameVisible(quint32 id) const
{
    if (m_id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        return point->isShowPointName();
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::setPointNameVisiblity(quint32 id, bool visible)
{
    if (m_id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setShowPointName(visible);
        refreshPointGeometry(*point);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::updatePointNameVisibility(quint32 id, bool visible)
{
    if (id == m_id)
    {
        qApp->getUndoStack()->push(new ShowPointName(doc, id, visible));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::setPointNamePosition(quint32 id, const QPointF &pos)
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
/**
 * @brief pointnameChangedPosition handle change position point text.
 * @param pos new position.
 */
void VToolSinglePoint::pointnameChangedPosition(const QPointF &pos)
{
    updatePointNamePosition(m_id, pos - this->pos());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief updatePointNamePosition save new position text to the pattern file.
 */
void VToolSinglePoint::updatePointNamePosition(quint32 id, const QPointF &pos)
{
    if (id == m_id)
    {
        qApp->getUndoStack()->push(new MoveLabel(doc, pos, id));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    VScenePoint::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not (flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (selectionType == SelectionType::ByMouseRelease)
    {
        event->accept();// Special for not selectable item first need to call standard mousePressEvent then accept event
    }
    else
    {
        if (event->button() == Qt::LeftButton)
        {
            pointChosen();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::Disable(bool disable, const QString &draftBlockName)
{
    const bool enabled = !CorrectDisable(disable, draftBlockName);
    SetEnabled(enabled);
    m_pointName->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::EnableToolMove(bool move)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::pointChosen()
{
    emit chosenTool(m_id, SceneObject::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::pointSelected(bool selected)
{
    setSelected(selected);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolSinglePoint::FullUpdateFromFile()
{
    ReadAttributes();
    refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent  handle mouse release events.
 * @param event mouse release event.
 */
void VToolSinglePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (selectionType == SelectionType::ByMouseRelease)
    {
        if (event->button() == Qt::LeftButton)
        {
            pointChosen();
        }
    }
    VScenePoint::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setToolTip(makeToolTip());
    VScenePoint::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange hadle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
QVariant VToolSinglePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        m_pointName->blockSignals(true);
        m_pointName->setSelected(value.toBool());
        m_pointName->blockSignals(false);
        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return VScenePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key release events.
 * @param event key release event.
 */
void VToolSinglePoint::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                deleteTool();
            }
            catch(const VExceptionToolWasDeleted &error)
            {
                Q_UNUSED(error)
                return;//Leave this method immediately!!!
            }
            break;
        default:
            break;
    }
    VScenePoint::keyReleaseEvent ( event );
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    QSharedPointer<VPointF> point = qSharedPointerDynamicCast<VPointF>(obj);
    SCASSERT(point.isNull() == false)

    doc->SetAttribute(tag, AttrName, point->name());
    doc->SetAttribute(tag, AttrMx, qApp->fromPixel(point->mx()));
    doc->SetAttribute(tag, AttrMy, qApp->fromPixel(point->my()));
    doc->SetAttribute<bool>(tag, AttrShowPointName, point->isShowPointName());

}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::allowTextHover(bool enabled)
{
    m_pointName->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::allowTextSelectable(bool enabled)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::ToolSelectionType(const SelectionType &type)
{
    VAbstractTool::ToolSelectionType(type);
    m_pointName->textSelectionType(type);
}

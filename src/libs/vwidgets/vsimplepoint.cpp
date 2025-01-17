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
 **  @file   vsimplepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#include "vsimplepoint.h"

#include <QBrush>
#include <QFlags>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QLineF>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <Qt>

#include "global.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "vgraphicssimpletextitem.h"
#include "../vmisc/vabstractapplication.h"

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint::VSimplePoint(quint32 id, const QColor &currentColor, QObject *parent)
    : VAbstractSimple(id, parent)
    , VScenePoint(currentColor)
    , m_visualizationMode(false)
    , m_alwaysHovered(false)
{
    m_pointColor = currentColor;
    connect(m_pointName, &VGraphicsSimpleTextItem::showContextMenu,    this, &VSimplePoint::contextMenuEvent);
    connect(m_pointName, &VGraphicsSimpleTextItem::deleteTool,         this, &VSimplePoint::deletePoint);
    connect(m_pointName, &VGraphicsSimpleTextItem::pointChosen,        this, &VSimplePoint::pointChosen);
    connect(m_pointName, &VGraphicsSimpleTextItem::pointSelected,      this, &VSimplePoint::pointSelected);
    connect(m_pointName, &VGraphicsSimpleTextItem::nameChangedPosition, this, &VSimplePoint::pointnameChangedPosition);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::SetVisualizationMode(bool value)
{
    m_visualizationMode = value;
    setFlag(QGraphicsItem::ItemIsFocusable, not m_visualizationMode);
}

//---------------------------------------------------------------------------------------------------------------------
bool VSimplePoint::IsVisualizationMode() const
{
    return m_visualizationMode;
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::SetPointHighlight(bool value)
{
    m_alwaysHovered = value;
    m_isHovered = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::SetEnabled(bool enabled)
{
    setEnabled(enabled);
    m_pointName->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::EnableToolMove(bool move)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::allowTextHover(bool enabled)
{
    m_pointName->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::allowTextSelectable(bool enabled)
{
    m_pointName->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::ToolSelectionType(const SelectionType &type)
{
    VAbstractSimple::ToolSelectionType(type);
    m_pointName->textSelectionType(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::deletePoint()
{
    emit Delete();
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::pointChosen()
{
    emit Choosed(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::pointSelected(bool selected)
{
    setSelected(selected);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::pointnameChangedPosition(const QPointF &pos)
{
    emit nameChangedPosition(pos, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_visualizationMode)
    {
        event->ignore();
    }
    else
    {
        // Special for not selectable item first need to call standard mousePressEvent then accept event
        QGraphicsEllipseItem::mousePressEvent(event);

        // Somehow clicking on notselectable object do not clean previous selections.
        if (not (flags() & ItemIsSelectable) && scene())
        {
            scene()->clearSelection();
        }

        if (selectionType == SelectionType::ByMouseRelease)
        {// Special for not selectable item first need to call standard mousePressEvent then accept event
            event->accept();
        }
        else
        {
            if (event->button() == Qt::LeftButton)
            {
                emit Choosed(id);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (not m_visualizationMode)
    {
        if (selectionType == SelectionType::ByMouseRelease)
        {
            if (event->button() == Qt::LeftButton)
            {
                emit Choosed(id);
            }
        }
        VScenePoint::mouseReleaseEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (not m_alwaysHovered)
    {
        m_isHovered = false;
    }
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            emit Delete();
            return; //Leave this method immediately after call!!!
        default:
            break;
    }
    VScenePoint::keyReleaseEvent ( event );
}

//---------------------------------------------------------------------------------------------------------------------
QVariant VSimplePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        m_pointName->blockSignals(true);
        m_pointName->setSelected(value.toBool());
        m_pointName->blockSignals(false);
        emit Selected(value.toBool(), id);
    }

    return VScenePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimplePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit showContextMenu(event, id);
}

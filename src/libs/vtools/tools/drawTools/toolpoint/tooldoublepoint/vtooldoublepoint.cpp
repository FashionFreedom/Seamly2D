/***************************************************************************
 **  @file   vtooldoublepoint.cpp
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
 **  @file   vtooldoublepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 6, 2015
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

#include "vtooldoublepoint.h"

#include <QColor>
#include <QDomElement>
#include <QKeyEvent>
#include <QPoint>
#include <QSharedPointer>
#include <QUndoStack>
#include <Qt>
#include <new>

#include "../../../../undocommands/label/movedoublelabel.h"
#include "../../../../undocommands/label/showdoublepointname.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/../ifc/ifcdef.h"
#include "../vwidgets/vsimplepoint.h"
#include "../../../vabstracttool.h"
#include "../../../vdatatool.h"
#include "../../vdrawtool.h"
#include "../vabstractpoint.h"

//---------------------------------------------------------------------------------------------------------------------
VToolDoublePoint::VToolDoublePoint(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 p1id, quint32 p2id,
                                   QGraphicsItem *parent)
    : VAbstractPoint(doc, data, id)
    , QGraphicsPathItem(parent)
    , firstPoint(nullptr)
    , secondPoint(nullptr)
    , p1id(p1id)
    , p2id(p2id)
{
    firstPoint = new VSimplePoint(p1id, QColor(qApp->Settings()->getPointNameColor()));
    firstPoint->setParentItem(this);
    firstPoint->setToolTip(complexToolTip(p1id));
    connect(firstPoint, &VSimplePoint::Choosed, this, &VToolDoublePoint::point1Chosen);
    connect(firstPoint, &VSimplePoint::Selected, this, &VToolDoublePoint::point1Selected);
    connect(firstPoint, &VSimplePoint::showContextMenu, this, &VToolDoublePoint::showContextMenu);
    connect(firstPoint, &VSimplePoint::Delete, this, &VToolDoublePoint::deletePoint);
    connect(firstPoint, &VSimplePoint::nameChangedPosition, this, &VToolDoublePoint::changePointName1Position);
    firstPoint->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(p1id));

    secondPoint = new VSimplePoint(p2id, QColor(qApp->Settings()->getPointNameColor()));
    secondPoint->setParentItem(this);
    secondPoint->setToolTip(complexToolTip(p2id));
    connect(secondPoint, &VSimplePoint::Choosed, this, &VToolDoublePoint::point2Chosen);
    connect(secondPoint, &VSimplePoint::Selected, this, &VToolDoublePoint::point2Selected);
    connect(secondPoint, &VSimplePoint::showContextMenu, this, &VToolDoublePoint::showContextMenu);
    connect(secondPoint, &VSimplePoint::Delete, this, &VToolDoublePoint::deletePoint);
    connect(secondPoint, &VSimplePoint::nameChangedPosition, this, &VToolDoublePoint::changePointName2Position);
    secondPoint->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(p2id));
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolDoublePoint::nameP1() const
{
    return ObjectName<VPointF>(p1id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::setNameP1(const QString &name)
{
    SetPointName(p1id, name);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolDoublePoint::nameP2() const
{
    return ObjectName<VPointF>(p2id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::setNameP2(const QString &name)
{
    SetPointName(p2id, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::GroupVisibility(quint32 object, bool visible)
{
    if (object == p1id)
    {
        firstPoint->setVisible(visible);
    }
    else if (object == p2id)
    {
        secondPoint->setVisible(visible);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VToolDoublePoint::isPointNameVisible(quint32 id) const
{
    if (p1id == id)
    {
        return VAbstractTool::data.GeometricObject<VPointF>(p1id)->isShowPointName();
    }
    else if (p2id == id)
    {
        return VAbstractTool::data.GeometricObject<VPointF>(p2id)->isShowPointName();
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::setPointNameVisiblity(quint32 id, bool visible)
{
    if (p1id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(p1id);
        point->setShowPointName(visible);
        firstPoint->refreshPointGeometry(*point);
    }
    else if (p2id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(p2id);
        point->setShowPointName(visible);
        secondPoint->refreshPointGeometry(*point);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::changePointName1Position(const QPointF &pos)
{
    updatePointNamePosition(p1id, pos - firstPoint->pos());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::changePointName2Position(const QPointF &pos)
{
    updatePointNamePosition(p2id, pos - secondPoint->pos());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::Disable(bool disable, const QString &draftBlockName)
{
    const bool enabled = !CorrectDisable(disable, draftBlockName);
    this->setEnabled(enabled);
    firstPoint->SetEnabled(enabled);
    secondPoint->SetEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::EnableToolMove(bool move)
{
    firstPoint->EnableToolMove(move);
    secondPoint->EnableToolMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::point1Chosen()
{
    emit chosenTool(p1id, SceneObject::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::point2Chosen()
{
    emit chosenTool(p2id, SceneObject::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::point1Selected(bool selected)
{
    emit ChangedToolSelection(selected, p1id, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::point2Selected(bool selected)
{
    emit ChangedToolSelection(selected, p2id, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::FullUpdateFromFile()
{
    ReadAttributes();
    firstPoint->setToolTip(complexToolTip(p1id));
    firstPoint->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(p1id));

    secondPoint->setToolTip(complexToolTip(p2id));
    secondPoint->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(p2id));
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::setPointNamePosition(quint32 id, const QPointF &pos)
{
    if (id == p1id)
    {
        VPointF *point = new VPointF(*VAbstractTool::data.GeometricObject<VPointF>(p1id));
        point->setMx(pos.x());
        point->setMy(pos.y());
        firstPoint->refreshPointGeometry(*point);

        if (QGraphicsScene *sc = firstPoint->scene())
        {
            VMainGraphicsView::NewSceneRect(sc, qApp->getSceneView(), firstPoint);
        }
    }
    else if (id == p2id)
    {
        VPointF *point = new VPointF(*VAbstractTool::data.GeometricObject<VPointF>(p2id));
        point->setMx(pos.x());
        point->setMy(pos.y());
        secondPoint->refreshPointGeometry(*point);

        if (QGraphicsScene *sc = secondPoint->scene())
        {
            VMainGraphicsView::NewSceneRect(sc, qApp->getSceneView(), secondPoint);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::AllowHover(bool enabled)
{
    firstPoint->setAcceptHoverEvents(enabled);
    secondPoint->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::AllowSelecting(bool enabled)
{
    firstPoint->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    secondPoint->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::allowTextHover(bool enabled)
{
    firstPoint->allowTextHover(enabled);
    secondPoint->allowTextHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::allowTextSelectable(bool enabled)
{
    firstPoint->allowTextSelectable(enabled);
    secondPoint->allowTextSelectable(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::ToolSelectionType(const SelectionType &type)
{
    VAbstractTool::ToolSelectionType(type);
    firstPoint->ToolSelectionType(type);
    secondPoint->ToolSelectionType(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::updatePointNamePosition(quint32 id, const QPointF &pos)
{
    if (id == p1id)
    {
        qApp->getUndoStack()->push(new MoveDoubleLabel(doc, pos, MoveDoublePoint::FirstPoint, m_id, p1id));
    }
    else if (id == p2id)
    {
        qApp->getUndoStack()->push(new MoveDoubleLabel(doc, pos, MoveDoublePoint::SecondPoint, m_id, p2id));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange hadle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
QVariant VToolDoublePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
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
/**
 * @brief keyReleaseEvent handle key release events.
 * @param event key release event.
 */
void VToolDoublePoint::keyReleaseEvent(QKeyEvent *event)
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
    QGraphicsPathItem::keyReleaseEvent ( event );
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    if (obj->id() == p1id)
    {
        QSharedPointer<VPointF> point = qSharedPointerDynamicCast<VPointF>(obj);
        SCASSERT(point.isNull() == false)

        doc->SetAttribute(tag, AttrName1, point->name());
        doc->SetAttribute(tag, AttrMx1, qApp->fromPixel(point->mx()));
        doc->SetAttribute(tag, AttrMy1, qApp->fromPixel(point->my()));
        doc->SetAttribute<bool>(tag, AttrShowPointName1, point->isShowPointName());
    }
    else if (obj->id() == p2id)
    {
        QSharedPointer<VPointF> point = qSharedPointerDynamicCast<VPointF>(obj);
        SCASSERT(point.isNull() == false)

        doc->SetAttribute(tag, AttrName2, point->name());
        doc->SetAttribute(tag, AttrMx2, qApp->fromPixel(point->mx()));
        doc->SetAttribute(tag, AttrMy2, qApp->fromPixel(point->my()));
        doc->SetAttribute<bool>(tag, AttrShowPointName2, point->isShowPointName());
    }
    else
    {
        VPointF *p1 = VAbstractTool::data.GeometricObject<VPointF>(p1id).data();
        VPointF *p2 = VAbstractTool::data.GeometricObject<VPointF>(p2id).data();

        doc->SetAttribute(tag, AttrPoint1, p1id);
        doc->SetAttribute(tag, AttrName1, p1->name());
        doc->SetAttribute(tag, AttrMx1, qApp->fromPixel(p1->mx()));
        doc->SetAttribute(tag, AttrMy1, qApp->fromPixel(p1->my()));
        doc->SetAttribute<bool>(tag, AttrShowPointName1, p1->isShowPointName());

        doc->SetAttribute(tag, AttrPoint2, p2id);
        doc->SetAttribute(tag, AttrName2, p2->name());
        doc->SetAttribute(tag, AttrMx2, qApp->fromPixel(p2->mx()));
        doc->SetAttribute(tag, AttrMy2, qApp->fromPixel(p2->my()));
        doc->SetAttribute<bool>(tag, AttrShowPointName2, p2->isShowPointName());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolDoublePoint::updatePointNameVisibility(quint32 id, bool visible)
{
    if (id == p1id)
    {
        qApp->getUndoStack()->push(new ShowDoublePointName(doc, m_id, p1id, visible, ShowDoublePoint::FirstPoint));
    }
    else if (id == p2id)
    {
        qApp->getUndoStack()->push(new ShowDoublePointName(doc, m_id, p2id, visible, ShowDoublePoint::SecondPoint));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolDoublePoint::complexToolTip(quint32 itemId) const
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(itemId);

    const QString toolTipStr = QString("<table>"
                                       "<tr> <td><b>%1:</b> %2</td> </tr>"
                                       "%3"
                                       "</table>")
                                       .arg(tr("Name"), point->name(), makeToolTip());
    return toolTipStr;
}

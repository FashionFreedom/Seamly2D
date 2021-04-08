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
 **  @date   12 9, 2016
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
#include <QtDebug>

#include "vabstractoperation.h"
#include "../../../undocommands/label/moveoperationlabel.h"
#include "../../../undocommands/label/showoperationpointname.h"
#include "../vgeometry/vpointf.h"

const QString VAbstractOperation::TagItem        = QStringLiteral("item");
const QString VAbstractOperation::TagSource      = QStringLiteral("source");
const QString VAbstractOperation::TagDestination = QStringLiteral("destination");

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractOperation::getTagName() const
{
    return VAbstractPattern::TagOperation;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractOperation::Suffix() const
{
    return suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::setSuffix(const QString &suffix)
{
    // Don't know if need check name here.
    this->suffix = suffix;
    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::GroupVisibility(quint32 object, bool visible)
{
    if (operatedObjects.contains(object))
    {
        VAbstractSimple *obj = operatedObjects.value(object);
        if (obj && obj->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            item->setVisible(visible);
        }
        else
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(obj);
            SCASSERT(item != nullptr)
            item->setVisible(visible);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(widget)
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstractOperation::isPointNameVisible(quint32 id) const
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            return VAbstractTool::data.GeometricObject<VPointF>(id)->isShowPointName();
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::setPointNameVisiblity(quint32 id, bool visible)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
            point->setShowPointName(visible);
            item->refreshPointGeometry(*point);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::updatePointNameVisibility(quint32 id, bool visible)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto dItem = std::find_if(destination.begin(), destination.end(),
                                    [id](const DestinationItem &dItem) { return dItem.id == id; });
            if (dItem != destination.end())
            {
                dItem->showPointName = visible;
            }
            qApp->getUndoStack()->push(new ShowOperationPointName(doc, m_id, id, visible));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::setPointNamePosition(quint32 id, const QPointF &pos)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
            point->setMx(pos.x());
            point->setMy(pos.y());
            item->refreshPointGeometry(*(point.data()));

            if (QGraphicsScene *sc = scene())
            {
                VMainGraphicsView::NewSceneRect(sc, qApp->getSceneView(), item);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::pointNamePositionChanged(const QPointF &pos, quint32 labelId)
{
    if (operatedObjects.contains(labelId))
    {
        VAbstractSimple *obj = operatedObjects.value(labelId);
        if (obj && obj->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(obj);
            SCASSERT(item != nullptr)
            updatePointNamePosition(labelId, pos - item->pos());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::updatePointNamePosition(quint32 id, const QPointF &pos)
{
    if (operatedObjects.contains(id))
    {
        VAbstractSimple *obj = operatedObjects.value(id);
        if (obj && obj->GetType() == GOType::Point)
        {
            auto dItem = std::find_if(destination.begin(), destination.end(),
                                    [id](const DestinationItem &dItem) { return dItem.id == id; });
            if (dItem != destination.end())
            {
                dItem->mx = pos.x();
                dItem->my = pos.y();
            }
            qApp->getUndoStack()->push(new MoveOperationLabel(m_id, doc, pos, id));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ExtractData(const QDomElement &domElement, QVector<quint32> &source,
                                     QVector<DestinationItem> &destination)
{
    const QDomNodeList nodeList = domElement.childNodes();
    for (qint32 i = 0; i < nodeList.size(); ++i)
    {
        const QDomElement dataElement = nodeList.at(i).toElement();
        if (not dataElement.isNull() && dataElement.tagName() == TagSource)
        {
            source.clear();
            const QDomNodeList srcList = dataElement.childNodes();
            for (qint32 j = 0; j < srcList.size(); ++j)
            {
                const QDomElement element = srcList.at(j).toElement();
                if (not element.isNull())
                {
                    source.append(VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR));
                }
            }
        }

        if (not dataElement.isNull() && dataElement.tagName() == TagDestination)
        {
            destination.clear();
            const QDomNodeList srcList = dataElement.childNodes();
            for (qint32 j = 0; j < srcList.size(); ++j)
            {
                const QDomElement element = srcList.at(j).toElement();
                if (not element.isNull())
                {
                    DestinationItem d;
                    d.id = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR);
                    d.mx = qApp->toPixel(VDomDocument::GetParametrDouble(element, AttrMx, QString::number(INT_MAX)));
                    d.my = qApp->toPixel(VDomDocument::GetParametrDouble(element, AttrMy, QString::number(INT_MAX)));
                    d.showPointName = VDomDocument::getParameterBool(element, AttrShowPointName1, trueStr);
                    destination.append(d);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::FullUpdateFromFile()
{
    ReadAttributes();
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setToolTip(complexPointToolTip(i.key()));
            item->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(i.key()));
        }
        else
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setToolTip(complexCurveToolTip(i.key()));
            item->RefreshGeometry(VAbstractTool::data.GeometricObject<VAbstractCurve>(i.key()));
        }
    }
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowHover(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
        else
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSelecting(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
        else
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::EnableToolMove(bool move)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->EnableToolMove(move);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointHover(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setAcceptHoverEvents(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointSelecting(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointLabelHover(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->allowTextHover(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowPointLabelSelecting(bool enabled)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->allowTextSelectable(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplineHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::Spline);
    AllowCurveHover(enabled, GOType::CubicBezier);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplineSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::Spline);
    AllowCurveSelecting(enabled, GOType::CubicBezier);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplinePathHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::SplinePath);
    AllowCurveHover(enabled, GOType::CubicBezierPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowSplinePathSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::SplinePath);
    AllowCurveSelecting(enabled, GOType::CubicBezierPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowArcHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::Arc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowArcSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::Arc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowElArcHover(bool enabled)
{
    AllowCurveHover(enabled, GOType::EllipticalArc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowElArcSelecting(bool enabled)
{
    AllowCurveSelecting(enabled, GOType::EllipticalArc);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ToolSelectionType(const SelectionType &type)
{
    VAbstractTool::ToolSelectionType(type);
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->ToolSelectionType(selectionType);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::Disable(bool disable, const QString &draftBlockName)
{
    const bool enabled = !CorrectDisable(disable, draftBlockName);
    setEnabled(enabled);

    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() == GOType::Point)
        {
            VSimplePoint *item = qobject_cast<VSimplePoint *>(i.value());
            SCASSERT(item != nullptr)
            item->SetEnabled(enabled);
        }
        else
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            item->setEnabled(enabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::ObjectSelected(bool selected, quint32 objId)
{
    emit ChangedToolSelection(selected, objId, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::deletePoint()
{
    try
    {
        deleteTool();
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractOperation::VAbstractOperation(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &suffix,
                                       const QVector<quint32> &source, const QVector<DestinationItem> &destination,
                                       QGraphicsItem *parent)
    : VDrawTool(doc, data, id)
    , QGraphicsLineItem(parent)
    , suffix(suffix)
    , source(source)
    , destination(destination)
    , operatedObjects()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveSourceDestination(QDomElement &tag)
{
    doc->RemoveAllChildren(tag);

    QDomElement tagObjects = doc->createElement(TagSource);
    for (int i = 0; i < source.size(); ++i)
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, source.at(i));
        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);

    tagObjects = doc->createElement(TagDestination);
    for (int i = 0; i < destination.size(); ++i)
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, destination.at(i).id);

        if (not VFuzzyComparePossibleNulls(destination.at(i).mx, INT_MAX) &&
            not VFuzzyComparePossibleNulls(destination.at(i).my, INT_MAX))
        {
            doc->SetAttribute(item, AttrMx, qApp->fromPixel(destination.at(i).mx));
            doc->SetAttribute(item, AttrMy, qApp->fromPixel(destination.at(i).my));
            doc->SetAttribute<bool>(item, AttrShowPointName, destination.at(i).showPointName);
        }

        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::InitCurve(quint32 id, VContainer *data, GOType curveType, SceneObject sceneType)
{
    const QSharedPointer<VAbstractCurve> initCurve = data->GeometricObject<VAbstractCurve>(id);
    VSimpleCurve *curve = new VSimpleCurve(id, initCurve);
    curve->setParentItem(this);
    curve->SetType(curveType);
    curve->setToolTip(complexCurveToolTip(id));
    connect(curve, &VSimpleCurve::Selected, this, &VAbstractOperation::ObjectSelected);
    connect(curve, &VSimpleCurve::showContextMenu, this, [this](QGraphicsSceneContextMenuEvent * event, quint32 id)
    {
        showContextMenu(event, id);
    });
    connect(curve, &VSimpleCurve::Choosed, this, [this, sceneType](quint32 id)
    {
        emit chosenTool(id, sceneType);
    });
    connect(curve, &VSimpleCurve::Delete, this, &VAbstractOperation::deletePoint);
    curve->RefreshGeometry(VAbstractTool::data.GeometricObject<VAbstractCurve>(id));
    operatedObjects.insert(id, curve);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowCurveHover(bool enabled, GOType type)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() != GOType::Point)
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            if (item->GetType() == type)
            {
                item->setAcceptHoverEvents(enabled);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::AllowCurveSelecting(bool enabled, GOType type)
{
    QMapIterator<quint32, VAbstractSimple *> i(operatedObjects);
    while (i.hasNext())
    {
        i.next();
        if (i.value()->GetType() != GOType::Point)
        {
            VSimpleCurve *item = qobject_cast<VSimpleCurve *>(i.value());
            SCASSERT(item != nullptr)
            if (item->GetType() == type)
            {
                item->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::InitOperatedObjects()
{
    for (int i = 0; i < destination.size(); ++i)
    {
        const DestinationItem object = destination.at(i);
        const QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(object.id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                VSimplePoint *point = new VSimplePoint(object.id, QColor(Qt::black));
                point->setParentItem(this);
                point->SetType(GOType::Point);
                point->setToolTip(complexPointToolTip(object.id));
                connect(point, &VSimplePoint::Choosed, this, [this](quint32 id)
                {
                    emit chosenTool(id, SceneObject::Point);
                });
                connect(point, &VSimplePoint::Selected, this, &VAbstractOperation::ObjectSelected);
                connect(point, &VSimplePoint::showContextMenu,
                        this, [this](QGraphicsSceneContextMenuEvent * event, quint32 id)
                {
                    showContextMenu(event, id);
                });
                connect(point, &VSimplePoint::Delete, this, &VAbstractOperation::deletePoint);
                connect(point, &VSimplePoint::nameChangedPosition, this, &VAbstractOperation::pointNamePositionChanged);
                point->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(object.id));
                operatedObjects.insert(object.id, point);
                break;
            }
            case GOType::Arc:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::Arc);
                break;
            case GOType::EllipticalArc:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::ElArc);
                break;
            case GOType::Spline:
            case GOType::CubicBezier:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::Spline);
                break;
            case GOType::SplinePath:
            case GOType::CubicBezierPath:
                InitCurve(object.id, &(VAbstractTool::data), obj->getType(), SceneObject::SplinePath);
                break;
            case GOType::Unknown:
                break;
        }
QT_WARNING_POP
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractOperation::complexPointToolTip(quint32 itemId) const
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(itemId);

    const QString toolTipStr = QString("<table>"
                                       "<tr> <td><b>%1:</b> %2</td> </tr>"
                                       "%3"
                                       "</table>")
                                       .arg(tr("Name"), point->name(), makeToolTip());
    return toolTipStr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractOperation::complexCurveToolTip(quint32 itemId) const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(itemId);

    const QString toolTipStr = QString("<table>"
                                       "<tr> <td><b>  %1:</b> %2</td> </tr>"
                                       "<tr> <td><b>%3:</b> %4 %5</td> </tr>"
                                       "%6"
                                       "</table>")
                                       .arg(tr("Name"))
                                       .arg(curve->name())
                                       .arg(tr("Length"))
                                       .arg(qApp->fromPixel(curve->GetLength()))
                                       .arg(UnitsToStr(qApp->patternUnit(), true))
                                       .arg(makeToolTip());
    return toolTipStr;
}

/***************************************************************************
 **  @file   vabstractoperation.cpp
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
 **  @file   vabstractoperation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2016 Valentina project
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

#include <QtDebug>

#include "vabstractoperation.h"
#include "../../../undocommands/label/moveoperationlabel.h"
#include "../../../undocommands/label/showoperationpointname.h"
#include "../vgeometry/vpointf.h"

const QString VAbstractOperation::TagItem        = QStringLiteral("item");
const QString VAbstractOperation::TagSource      = QStringLiteral("source");
const QString VAbstractOperation::TagDestination = QStringLiteral("destination");

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> sourceToObjects(const QVector<SourceItem> &source)
{
    QVector<quint32> ids;
    ids.reserve(source.size());

    for (auto s: source)
    {
        ids.append(s.id);
    }

    return ids;
}

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
void VAbstractOperation::ExtractData(const QDomElement &domElement, QVector<SourceItem> &source,
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
                    SourceItem item;
                    item.id       = VDomDocument::GetParametrUInt(element, AttrIdObject, NULL_ID_STR);
                    item.alias    = VDomDocument::GetParametrEmptyString(element, AttrAlias);
                    item.lineType = VDomDocument::GetParametrString(element, AttrLineType, LineTypeSolidLine);
                    item.color    = VDomDocument::GetParametrString(element, AttrColor, "black");
                    source.append(item);
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
                    d.showPointName = VDomDocument::getParameterBool(element, AttrShowPointName, trueStr);
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
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractOperation::VAbstractOperation(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &suffix,
                                       const QVector<SourceItem> &source, const QVector<DestinationItem> &destination,
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
void VAbstractOperation::ReadToolAttributes(const QDomElement &domElement)
{
    ExtractData(domElement, source, destination);
    suffix      = doc->GetParametrString(domElement, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrSuffix, suffix);

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractOperation::SaveSourceDestination(QDomElement &tag)
{
    doc->RemoveAllChildren(tag);

    QDomElement tagObjects = doc->createElement(TagSource);
    for (auto sourceItem : qAsConst(source))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, sourceItem.id);
        doc->SetAttribute(item, AttrAlias,    sourceItem.alias);
        doc->SetAttribute(item, AttrLineType, sourceItem.lineType);
        doc->SetAttribute(item, AttrColor,    sourceItem.color);
        tagObjects.appendChild(item);
    }
    tag.appendChild(tagObjects);

    tagObjects = doc->createElement(TagDestination);
    for (auto destinationItem : qAsConst(destination))
    {
        QDomElement item = doc->createElement(TagItem);
        doc->SetAttribute(item, AttrIdObject, destinationItem.id);

        if (not VFuzzyComparePossibleNulls(destinationItem.mx, INT_MAX) &&
            not VFuzzyComparePossibleNulls(destinationItem.my, INT_MAX))
        {
            doc->SetAttribute(item, AttrMx, qApp->fromPixel(destinationItem.mx));
            doc->SetAttribute(item, AttrMy, qApp->fromPixel(destinationItem.my));
            doc->SetAttribute<bool>(item, AttrShowPointName, destinationItem.showPointName);
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
        const DestinationItem item = destination.at(i);
        const QSharedPointer<VGObject> object = VAbstractTool::data.GetGObject(item.id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
        switch(static_cast<GOType>(object->getType()))
        {
            case GOType::Point:
            {
                VSimplePoint *point = new VSimplePoint(item.id, QColor(Qt::black));
                point->setParentItem(this);
                point->SetType(GOType::Point);
                point->setToolTip(complexPointToolTip(item.id));
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
                point->refreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(item.id));
                operatedObjects.insert(item.id, point);
                break;
            }
            case GOType::Arc:
                InitCurve(item.id, &(VAbstractTool::data), object->getType(), SceneObject::Arc);
                break;
            case GOType::EllipticalArc:
                InitCurve(item.id, &(VAbstractTool::data), object->getType(), SceneObject::ElArc);
                break;
            case GOType::Spline:
            case GOType::CubicBezier:
                InitCurve(item.id, &(VAbstractTool::data), object->getType(), SceneObject::Spline);
                break;
            case GOType::SplinePath:
            case GOType::CubicBezierPath:
                InitCurve(item.id, &(VAbstractTool::data), object->getType(), SceneObject::SplinePath);
                break;
            case GOType::Unknown:
            case GOType::Curve:
            case GOType::Path:
            case GOType::AllCurves:
            default:
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

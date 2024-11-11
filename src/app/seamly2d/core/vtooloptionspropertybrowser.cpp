/******************************************************************************
*   @file   vtooloptionspropertybrowser.cpp
**  @author Douglas S Caskey
**  @date   17 Sep, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   vtooloptionspropertybrowser.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
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

#include "vtooloptionspropertybrowser.h"

#include "vformulaproperty.h"
#include "../core/application_2d.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpropertyexplorer/vproperties.h"
#include "../vtools/tools/drawTools/drawtools.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vsimplepoint.h"
#include "../vwidgets/vsimplecurve.h"
#include "../vwidgets/vcurvepathitem.h"

#include <QDockWidget>
//#include <QEvent>
#include <QHBoxLayout>
#include <QDebug>
#include <QRegularExpression>

//---------------------------------------------------------------------------------------------------------------------
VToolOptionsPropertyBrowser::VToolOptionsPropertyBrowser(const VContainer *data, QDockWidget *parent)
    : QObject(parent)
    , m_data(data)
    , propertyModel(nullptr)
    , formView(nullptr)
    , currentItem(nullptr)
    , propertyToId(QMap<VPE::VProperty *, QString>())
    , idToProperty(QMap<QString, VPE::VProperty *>())
    , m_centerPointStr(tr("Center point"))
{
    propertyModel = new VPE::VPropertyModel(this);
    formView = new VPE::VPropertyFormView(propertyModel, parent);
    formView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QScrollArea *scroll = new QScrollArea(parent);
    scroll->setWidgetResizable(true);
    scroll->setWidget(formView);

    parent->setWidget(scroll);

    connect(propertyModel, &VPE::VPropertyModel::onDataChangedByEditor, this,
            &VToolOptionsPropertyBrowser::userChangedData);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::clearPropertyBrowser()
{
    propertyModel->clear();
    propertyToId.clear();
    idToProperty.clear();
    currentItem = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showItemOptions(QGraphicsItem *item)
{
    // This check helps to find missing tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used in switch.");

    switch (item->type())
    {
        case VToolBasePoint::Type:
            showOptionsToolSinglePoint(item);
            break;
        case VToolEndLine::Type:
            showOptionsToolEndLine(item);
            break;
        case VToolAlongLine::Type:
            showOptionsToolAlongLine(item);
            break;
        case VToolArc::Type:
            showOptionsToolArc(item);
            break;
        case VToolArcWithLength::Type:
            showOptionsToolArcWithLength(item);
            break;
        case VToolBisector::Type:
            showOptionsToolBisector(item);
            break;
        case VToolCutArc::Type:
            showOptionsToolCutArc(item);
            break;
        case VToolCutSpline::Type:
            showOptionsToolCutSpline(item);
            break;
        case VToolCutSplinePath::Type:
            showOptionsToolCutSplinePath(item);
            break;
        case VToolHeight::Type:
            showOptionsToolHeight(item);
            break;
        case VToolLine::Type:
            showOptionsToolLine(item);
            break;
        case VToolLineIntersect::Type:
            showOptionsToolLineIntersect(item);
            break;
        case VToolNormal::Type:
            showOptionsToolNormal(item);
            break;
        case VToolPointOfContact::Type:
            showOptionsToolPointOfContact(item);
            break;
        case PointIntersectXYTool::Type:
            showOptionsToolPointOfIntersection(item);
            break;
        case VToolPointOfIntersectionArcs::Type:
            showOptionsToolPointOfIntersectionArcs(item);
            break;
        case IntersectCirclesTool::Type:
            showOptionsToolPointOfIntersectionCircles(item);
            break;
        case VToolPointOfIntersectionCurves::Type:
            showOptionsToolPointOfIntersectionCurves(item);
            break;
        case VToolShoulderPoint::Type:
            showOptionsToolShoulderPoint(item);
            break;
        case VToolSpline::Type:
            showOptionsToolSpline(item);
            break;
        case VToolCubicBezier::Type:
            showOptionsToolCubicBezier(item);
            break;
        case VToolSplinePath::Type:
            showOptionsToolSplinePath(item);
            break;
        case VToolCubicBezierPath::Type:
            showOptionsToolCubicBezierPath(item);
            break;
        case VToolTriangle::Type:
            showOptionsToolTriangle(item);
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
        case VSimplePoint::Type:
        case VSimpleCurve::Type:
            currentItem = item->parentItem();
            showItemOptions(currentItem);
            break;
        case VToolLineIntersectAxis::Type:
            showOptionsToolLineIntersectAxis(item);
            break;
        case VToolCurveIntersectAxis::Type:
            showOptionsToolCurveIntersectAxis(item);
            break;
        case IntersectCircleTangentTool::Type:
            showOptionsToolPointFromCircleAndTangent(item);
            break;
        case VToolPointFromArcAndTangent::Type:
            showOptionsToolPointFromArcAndTangent(item);
            break;
        case VToolTrueDarts::Type:
            showOptionsToolTrueDarts(item);
            break;
        case VToolRotation::Type:
            showOptionsToolRotation(item);
            break;
        case VToolMirrorByLine::Type:
            showOptionsToolMirrorByLine(item);
            break;
        case VToolMirrorByAxis::Type:
            showOptionsToolMirrorByAxis(item);
            break;
        case VToolMove::Type:
            showOptionsToolMove(item);
            break;
        case VToolEllipticalArc::Type:
            showOptionsToolEllipticalArc(item);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptions()
{
    if (currentItem == nullptr)
    {
        return;
    }

    // This check helps to find missing tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used in switch.");

    switch (currentItem->type())
    {
        case VToolBasePoint::Type:
            updateOptionsToolSinglePoint();
            break;
        case VToolEndLine::Type:
            updateOptionsToolEndLine();
            break;
        case VToolAlongLine::Type:
            updateOptionsToolAlongLine();
            break;
        case VToolArc::Type:
            updateOptionsToolArc();
            break;
        case VToolArcWithLength::Type:
            updateOptionsToolArcWithLength();
            break;
        case VToolBisector::Type:
            updateOptionsToolBisector();
            break;
        case VToolCutArc::Type:
            updateOptionsToolCutArc();
            break;
        case VToolCutSpline::Type:
            updateOptionsToolCutSpline();
            break;
        case VToolCutSplinePath::Type:
            updateOptionsToolCutSplinePath();
            break;
        case VToolHeight::Type:
            updateOptionsToolHeight();
            break;
        case VToolLine::Type:
            updateOptionsToolLine();
            break;
        case VToolLineIntersect::Type:
            updateOptionsToolLineIntersect();
            break;
        case VToolNormal::Type:
            updateOptionsToolNormal();
            break;
        case VToolPointOfContact::Type:
            updateOptionsToolPointOfContact();
            break;
        case PointIntersectXYTool::Type:
            updateOptionsToolPointOfIntersection();
            break;
        case VToolPointOfIntersectionArcs::Type:
            updateOptionsToolPointOfIntersectionArcs();
            break;
        case IntersectCirclesTool::Type:
            updateOptionsToolPointOfIntersectionCircles();
            break;
        case VToolPointOfIntersectionCurves::Type:
            updateOptionsToolPointOfIntersectionCurves();
            break;
        case VToolShoulderPoint::Type:
            updateOptionsToolShoulderPoint();
            break;
        case VToolSpline::Type:
            updateOptionsToolSpline();
            break;
        case VToolCubicBezier::Type:
            updateOptionsToolCubicBezier();
            break;
        case VToolSplinePath::Type:
            updateOptionsToolSplinePath();
            break;
        case VToolCubicBezierPath::Type:
            updateOptionsToolCubicBezierPath();
            break;
        case VToolTriangle::Type:
            updateOptionsToolTriangle();
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
            showItemOptions(currentItem->parentItem());
            break;
        case VToolLineIntersectAxis::Type:
            updateOptionsToolLineIntersectAxis();
            break;
        case VToolCurveIntersectAxis::Type:
            updateOptionsToolCurveIntersectAxis();
            break;
        case IntersectCircleTangentTool::Type:
            updateOptionsToolPointFromCircleAndTangent();
            break;
        case VToolPointFromArcAndTangent::Type:
            updateOptionsToolPointFromArcAndTangent();
            break;
        case VToolTrueDarts::Type:
            updateOptionsToolTrueDarts();
            break;
        case VToolRotation::Type:
            updateOptionsToolRotation();
            break;
        case VToolMirrorByLine::Type:
            updateOptionsToolMirrorByLine();
            break;
        case VToolMirrorByAxis::Type:
            updateOptionsToolMirrorByAxis();
            break;
        case VToolMove::Type:
            updateOptionsToolMove();
            break;
        case VToolEllipticalArc::Type:
            updateOptionsToolEllipticalArc();
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::refreshOptions()
{
    QGraphicsItem *item = currentItem;
    itemClicked(nullptr); //close options
    itemClicked(item);    //reopen options
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::userChangedData(VPE::VProperty *property)
{
    VPE::VProperty *prop = property;
    if (!propertyToId.contains(prop))
    {
        if (!propertyToId.contains(prop->getParent()))// Maybe we know parent
        {
            return;
        }
        else
        {
            prop = prop->getParent();
        }
    }

    if (!currentItem)
    {
        return;
    }

    // This check helps to find missing tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used in switch.");

    switch (currentItem->type())
    {
        case VToolBasePoint::Type:
            changeDataToolSinglePoint(prop);
            break;
        case VToolEndLine::Type:
            changeDataToolEndLine(prop);
            break;
        case VToolAlongLine::Type:
            changeDataToolAlongLine(prop);
            break;
        case VToolArc::Type:
            changeDataToolArc(prop);
            break;
        case VToolArcWithLength::Type:
            changeDataToolArcWithLength(prop);
            break;
        case VToolBisector::Type:
            changeDataToolBisector(prop);
            break;
        case VToolCutArc::Type:
            changeDataToolCutArc(prop);
            break;
        case VToolCutSpline::Type:
            changeDataToolCutSpline(prop);
            break;
        case VToolCutSplinePath::Type:
            changeDataToolCutSplinePath(prop);
            break;
        case VToolHeight::Type:
            changeDataToolHeight(prop);
            break;
        case VToolLine::Type:
            changeDataToolLine(prop);
            break;
        case VToolLineIntersect::Type:
            changeDataToolLineIntersect(prop);
            break;
        case VToolNormal::Type:
            changeDataToolNormal(prop);
            break;
        case VToolPointOfContact::Type:
            changeDataToolPointOfContact(prop);
            break;
        case PointIntersectXYTool::Type:
            changeDataToolPointOfIntersection(prop);
            break;
        case VToolPointOfIntersectionArcs::Type:
            changeDataToolPointOfIntersectionArcs(prop);
            break;
        case IntersectCirclesTool::Type:
            changeDataToolPointOfIntersectionCircles(prop);
            break;
        case VToolPointOfIntersectionCurves::Type:
            changeDataToolPointOfIntersectionCurves(prop);
            break;
        case VToolShoulderPoint::Type:
            changeDataToolShoulderPoint(prop);
            break;
        case VToolSpline::Type:
            changeDataToolSpline(prop);
            break;
        case VToolCubicBezier::Type:
            changeDataToolCubicBezier(prop);
            break;
        case VToolSplinePath::Type:
            changeDataToolSplinePath(prop);
            break;
        case VToolCubicBezierPath::Type:
            changeDataToolCubicBezierPath(prop);
            break;
        case VToolTriangle::Type:
            changeDataToolTriangle(prop);
            break;
        case VToolLineIntersectAxis::Type:
            changeDataToolLineIntersectAxis(prop);
            break;
        case VToolCurveIntersectAxis::Type:
            changeDataToolCurveIntersectAxis(prop);
            break;
        case IntersectCircleTangentTool::Type:
            changeDataToolPointFromCircleAndTangent(prop);
            break;
        case VToolPointFromArcAndTangent::Type:
            changeDataToolPointFromArcAndTangent(prop);
            break;
        case VToolTrueDarts::Type:
            changeDataToolTrueDarts(prop);
            break;
        case VToolRotation::Type:
            changeDataToolRotation(prop);
            break;
        case VToolMirrorByLine::Type:
            changeDataToolMirrorByLine(prop);
            break;
        case VToolMirrorByAxis::Type:
            changeDataToolMirrorByAxis(prop);
            break;
        case VToolMove::Type:
            changeDataToolMove(prop);
            break;
        case VToolEllipticalArc::Type:
            changeDataToolEllipticalArc(prop);
            break;
        default:
            break;
    }
    qApp->getSceneView()->update();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::itemClicked(QGraphicsItem *item)
{
    if (item != nullptr)
    {
        if (item->isEnabled()==false)
        {
            return;
        }

        if(item->type() == QGraphicsItem::UserType + static_cast<int>(Tool::BackgroundImage))
        {
            item = nullptr;
        }
    }

    if (currentItem == item && item != nullptr)
    {
        updateOptions();
        return;
    }

    propertyModel->clear();
    propertyToId.clear();
    idToProperty.clear();

    if (currentItem != nullptr)
    {
        VAbstractTool *previousTool = dynamic_cast<VAbstractTool *>(currentItem);
        if (previousTool != nullptr)
        {
            previousTool->ShowVisualization(false); // hide for previous tool
        }
    }

    currentItem = item;
    if (currentItem == nullptr)
    {
        formView->setTitle("");
        return;
    }

    showItemOptions(currentItem);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::addPropertyFormula(const QString &propertyName, const VFormula &formula,
                                                     const QString &attrName)
{
    VFormulaProperty *itemLength = new VFormulaProperty(propertyName);
    itemLength->SetFormula(formula);
    addProperty(itemLength, attrName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyObjectName(Tool *tool, const QString &propertyName, bool readOnly)
{
    VPE::VStringProperty *itemName = new VPE::VStringProperty(propertyName);
    readOnly == true ? itemName->setClearButtonEnable(false) : itemName->setClearButtonEnable(true);
    itemName->setValue(qApp->translateVariables()->VarToUser(tool->name()));
    itemName->setReadOnly(readOnly);
    addProperty(itemName, AttrName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyPointName1(Tool *tool, const QString &propertyName)
{
    VPE::VStringProperty *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(tool->nameP1());
    addProperty(itemName, AttrName1);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyPointName2(Tool *tool, const QString &propertyName)
{
    VPE::VStringProperty *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(tool->nameP2());
    addProperty(itemName, AttrName2);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyOperationSuffix(Tool *tool, const QString &propertyName, bool readOnly)
{
    VPE::VStringProperty *itemSuffix = new VPE::VStringProperty(propertyName);
    itemSuffix->setClearButtonEnable(true);
    itemSuffix->setValue(tool->Suffix());
    itemSuffix->setReadOnly(readOnly);
    addProperty(itemSuffix, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyLineName(Tool *tool, const QString &propertyName, bool readOnly)
{
    auto lineName = new VPE::VStringProperty(propertyName);
    lineName->setValue(tr("Line_") + tool->FirstPointName() + "_" + tool->SecondPointName());
    lineName->setReadOnly(readOnly);
    addProperty(lineName, AttrObjName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyCurveName(Tool *tool, const QString &propertyName, const QString &prefix,
                                                       const QString &firstPoint, const QString &secondPoint,
                                                       bool readOnly)
{
    Q_UNUSED(tool)

    auto arcName = new VPE::VStringProperty(propertyName);
    arcName->setValue(prefix + firstPoint + "_" + secondPoint);
    arcName->setReadOnly(readOnly);
    addProperty(arcName, AttrObjName);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::addPropertyParentPointName(const QString &pointName, const QString &propertyName,
                                                             const QString &propertyAttribute)
{
    auto *itemParentPoint = new VPE::VLabelProperty(propertyName);
    itemParentPoint->setValue(pointName);
    addProperty(itemParentPoint, propertyAttribute);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::addPropertyLabel(const QString &propertyName, const QString &propertyAttribute)
{
    auto label = new VPE::VLabelProperty("<b>"+propertyName+"</b>");
    label->setValue("");
    label->setPropertyType(VPE::Property::Label);
    addProperty(label, propertyAttribute);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyCrossPoint(Tool *tool, const QString &propertyName)
{
    VPE::VEnumProperty* itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList() <<  tr("First point") << tr("Second point"));
    itemProperty->setValue(static_cast<int>(tool->GetCrossCirclesPoint())-1);
    addProperty(itemProperty, AttrCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyVCrossPoint(Tool *tool, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList() <<  tr("Highest point") << tr("Lowest point"));
    itemProperty->setValue(static_cast<int>(tool->GetVCrossPoint())-1);
    addProperty(itemProperty, AttrVCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyHCrossPoint(Tool *tool, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList() <<  tr("Leftmost point") << tr("Rightmost point"));
    itemProperty->setValue(static_cast<int>(tool->GetHCrossPoint())-1);
    addProperty(itemProperty, AttrHCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyAxisType(Tool *tool, const QString &propertyName)
{
    auto *itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList() <<  tr("Vertical axis") << tr("Horizontal axis"));
    itemProperty->setValue(static_cast<int>(tool->getAxisType())-1);
    addProperty(itemProperty, AttrAxisType);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyLineType(Tool *tool, const QString &propertyName)
{
    VPE::LineTypeProperty *lineTypeProperty = new VPE::LineTypeProperty(propertyName);

    QMap<QString, QString> lineTypes = lineTypeList();
    if (tool->type() == VToolLine::Type)
    {
        lineTypes = lineTypeNoPenRemovedList();
    }
    lineTypeProperty->setLineTypes(lineTypes);

    const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypes, tool->getLineType());
    if (index == -1)
    {
        qWarning() << "Can't find line type" << tool->getLineType() << "in list";
    }
    lineTypeProperty->setValue(index);
    addProperty(lineTypeProperty, AttrLineType);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyCurveLineType(Tool *tool, const QString &propertyName)
{
    VPE::LineTypeProperty *penStyleProperty = new VPE::LineTypeProperty(propertyName);
    penStyleProperty->setLineTypes(lineTypeNoPenRemovedList());
    const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
    if (index == -1)
    {
        qWarning() << "Can't find line type" << tool->getLineType() <<  "in list";
    }
    penStyleProperty->setValue(index);
    addProperty(penStyleProperty, AttrPenStyle);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyLineWeight(Tool *tool, const QString &propertyName)
{
    VPE::LineWeightProperty *lineWeightProperty = new VPE::LineWeightProperty(propertyName);
    lineWeightProperty->setLineWeights(lineWeightList());
    const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
    if (index == -1)
    {
        qWarning() << "Can't find line weight" << tool->getLineWeight() <<  "in list";
    }
    lineWeightProperty->setValue(index);
    addProperty(lineWeightProperty, AttrLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyLineColor(Tool *tool, const QString &propertyName, const QString &id)
{
    VPE::VLineColorProperty *lineColorProperty = new VPE::VLineColorProperty(propertyName);
    lineColorProperty->setColors(VAbstractTool::ColorsList());
    const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
    if (index == -1)
    {
        qWarning() << "Can't find line color" << tool->getLineColor() <<  "in list";
    }
    lineColorProperty->setValue(index);
    addProperty(lineColorProperty, id);
}

// @brief Adds a directionproperty.
//
// Adds a direction combobox property to the Property Editor form widget. Gets the combobox index to
// the tool's direction. Throws a warning if the property is not found in the combobox item list.
// If the index exists it sets the combobox index to the tool's direction.
//
// @tparam tool Tool of the property.
// @param propertyName Name of the property.
template<class Tool>
void VToolOptionsPropertyBrowser::addPropertyDirection(Tool *tool, const QString &propertyName)
{
    VPE::DirectionProperty *directionProperty = new VPE::DirectionProperty(propertyName);
    directionProperty->setDirections(directionList());
    const qint32 index = VPE::DirectionProperty::indexOfDirection(directionList(), tool->getDirection());
    if (index == -1)
    {
        qWarning() << "Can't find direction" << tool->getDirection() <<  "in list";
    }
    directionProperty->setValue(index);
    addProperty(directionProperty, AttrDirection);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::addObjectProperty(Tool *tool, const QString &pointName,
                                                   const QString &propertyName, const QString &id, GOType objType)
{
    QMap<QString, quint32> list = getObjectList(tool, objType);
    VPE::VObjectProperty *pointsProperty = new VPE::VObjectProperty(propertyName);
    pointsProperty->setObjectsList(list);
    const qint32 index = VPE::VObjectProperty::indexOfObject(list, pointName);
    if (index == -1)
    {
        qWarning() << "Can't find point" << pointName << "in list";
    }
    pointsProperty->setValue(index);
    addProperty(pointsProperty, id);
}

template<class Tool>
QMap<QString, quint32> VToolOptionsPropertyBrowser::getObjectList(Tool *tool, GOType objType)
{
    quint32 toolId = tool->getId();
    QHash<quint32, QSharedPointer<VGObject>> objects;

    QVector<VToolRecord> history = qApp->getCurrentDocument()->getBlockHistory();
    for (qint32 i = 0; i < history.size(); ++i)
    {
        const VToolRecord record = history.at(i);
        quint32 recId = record.getId();
        if (recId != toolId)
        {
            switch (static_cast<int>(record.getTypeTool()))
            {
                case 42:    //Tool::True Darts
                {
                    QVector<quint32> list = qApp->getCurrentDocument()->getDartItems(recId);
                    for (qint32 j = 0; j < list.size(); ++j)
                    {
                        quint32 id = list.at(j);
                            VToolRecord newRecord = VToolRecord(id, record.getTypeTool(),
                                                        qApp->getCurrentDocument()->getActiveDraftBlockName());

                        const QHash<quint32, QSharedPointer<VGObject> > *objs = m_data->DataGObjects();
                        if (objs->contains(id)) //Avoid badId Get GObject only if not a line tool which is not an object
                        {
                            QSharedPointer<VGObject> obj = m_data->GetGObject(id);
                            objects.insert(id, obj);
                        }
                    }
                    break;
                }

                case 45:    //Tool::Rotation
                case 46:    //Tool::MirrorByLine
                case 47:    //Tool::MirrorByAxis
                case 48:    //Tool::Move
                {
                    QVector<quint32> list = qApp->getCurrentDocument()->getOpItems(recId, QStringLiteral("destination"));
                    for (qint32 j = 0; j < list.size(); ++j)
                    {
                        quint32 id = list.at(j);
                            VToolRecord newRecord = VToolRecord(id, record.getTypeTool(),
                                                        qApp->getCurrentDocument()->getActiveDraftBlockName());

                        const QHash<quint32, QSharedPointer<VGObject> > *objs = m_data->DataGObjects();
                        if (objs->contains(id)) //Avoid badId Get GObject only if not a line tool which is not an object
                        {
                            QSharedPointer<VGObject> obj = m_data->GetGObject(id);
                            objects.insert(id, obj);
                        }
                    }
                    break;
                }
                default:
                {
                    const QHash<quint32, QSharedPointer<VGObject> > *objs = m_data->DataGObjects();
                    if (objs->contains(recId)) //Avoid badId Get GObject only if not a line tool which is not an object
                    {
                        QSharedPointer<VGObject> obj = m_data->GetGObject(recId);
                        objects.insert(recId, obj);
                    }
                    break;
                }
            }
        }
        else
        {
            goto endLoop; //Copied objects upto selected tool - exit switch and for loops
        }
    }

endLoop:
    QMap<QString, quint32> map;
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;
    for (i = objects.constBegin(); i != objects.constEnd(); ++i)
    {
        QSharedPointer<VGObject> obj = i.value();
        if (obj->getMode() == Draw::Calculation)
        {
            switch (objType)
            {
                case GOType::Point:
                case GOType::Arc:
                case GOType::Spline:
                case GOType::CubicBezier:
                case GOType::SplinePath:
                case GOType::CubicBezierPath:
                    if (obj->getType() == objType)
                    {
                        map.insert(obj->name(), i.key());
                    }
                    break;

                case GOType::Curve:
                    if (obj->getType() == GOType::Spline || obj->getType() == GOType::CubicBezier)
                    {
                        map.insert(obj->name(), i.key());
                    }
                    break;

                case GOType::Path:
                    if (obj->getType() == GOType::SplinePath || obj->getType() == GOType::CubicBezierPath)
                    {
                        map.insert(obj->name(), i.key());
                    }
                    break;

                case GOType::AllCurves:
                    if ((obj->getType() == GOType::Spline ||
                         obj->getType() == GOType::SplinePath ||
                         obj->getType() == GOType::CubicBezier ||
                         obj->getType() == GOType::CubicBezierPath ||
                         obj->getType() == GOType::Arc ||
                         obj->getType() == GOType::EllipticalArc) &&
                         obj->getMode() == Draw::Calculation)
                    {
                        map.insert(obj->name(), i.key());
                    }
                    break;

                case GOType::EllipticalArc:
                case GOType::Unknown:
                default:
                    break;
            }
        }
    }

    if (tool->type() == VToolMove::Type)
    {
        map.insert(m_centerPointStr, NULL_ID);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setPointName(const QString &name)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == tool->name())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName]->setValue(tool->name());
        }
        else
        {
            tool->setName(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setPointName1(const QString &name)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == tool->nameP1())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName1]->setValue(tool->nameP1());
        }
        else
        {
            tool->setNameP1(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setPointName2(const QString &name)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == tool->nameP2())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName2]->setValue(tool->nameP2());
        }
        else
        {
            tool->setNameP2(name);
        }
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setOperationSuffix(const QString &suffix)
{
    if (Tool *item = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (suffix == item->Suffix())
        {
            return;
        }

        if (suffix.isEmpty())
        {
            idToProperty[AttrSuffix]->setValue(item->Suffix());
            return;
        }

        QRegularExpression rx(NameRegExp());
        const QStringList uniqueNames = VContainer::AllUniqueNames();
        for (int i=0; i < uniqueNames.size(); ++i)
        {
            const QString name = uniqueNames.at(i) + suffix;
            if (not rx.match(name).hasMatch() || not VContainer::IsUnique(name))
            {
                idToProperty[AttrSuffix]->setValue(item->Suffix());
                return;
            }
        }

        item->setSuffix(suffix);
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Type>
Type VToolOptionsPropertyBrowser::getCrossPoint(const QVariant &value)
{
    bool ok = false;
    const int val = value.toInt(&ok);

    auto cross = static_cast<Type>(1);
    if (ok)
    {
        switch(val)
        {
            case 0:
            case 1:
                cross = static_cast<Type>(val+1);
                break;
            default:
                break;
        }
    }

    return cross;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setCirclesCrossPoint(const QVariant &value)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        tool->setCirclesCrossPoint(getCrossPoint<CrossCirclesPoint>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setCurveVCrossPoint(const QVariant &value)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        tool->SetVCrossPoint(getCrossPoint<VCrossCurvesPoint>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setCurveHCrossPoint(const QVariant &value)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        tool->SetHCrossPoint(getCrossPoint<HCrossCurvesPoint>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setAxisType(const QVariant &value)
{
    if (Tool *tool = qgraphicsitem_cast<Tool *>(currentItem))
    {
        tool->setAxisType(getCrossPoint<AxisType>(value));
    }
    else
    {
        qWarning() << "Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::addProperty(VPE::VProperty *property, const QString &id)
{
    propertyToId[property] = id;
    idToProperty[id] = property;
    propertyModel->addProperty(property, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolSinglePoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolBasePoint *tool = qgraphicsitem_cast<VToolBasePoint *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolBasePoint>(value.toString());
            break;
        case 1: // QLatin1String("Coordinates")
            tool->SetBasePointPos(value.toPointF());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolEndLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolEndLine *tool = qgraphicsitem_cast<VToolEndLine *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolEndLine>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 5: // AttrAngle
            tool->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolAlongLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolAlongLine *tool = qgraphicsitem_cast<VToolAlongLine *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolAlongLine>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 7: // AttrSecondPoint
            tool->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolArc *tool = qgraphicsitem_cast<VToolArc *>(currentItem);
    SCASSERT(tool != nullptr)

    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 8: // AttrRadius
            tool->SetFormulaRadius(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            tool->SetFormulaF1(value.value<VFormula>());
            break;
        case 10: // AttrAngle2
            tool->SetFormulaF2(value.value<VFormula>());
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            tool->setCenter(value.toInt());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolArcWithLength(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolArcWithLength *tool = qgraphicsitem_cast<VToolArcWithLength *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 8: // AttrRadius
            tool->SetFormulaRadius(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            tool->SetFormulaF1(value.value<VFormula>());
            break;
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            tool->setCenter(value.toInt());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolBisector(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolBisector *tool = qgraphicsitem_cast<VToolBisector *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolBisector>(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 6:  // AttrFirstPoint
            tool->SetFirstPointId(value.toInt());
            break;
        case 2:  // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 12: // AttrThirdPoint
            tool->SetThirdPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolTrueDarts(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolTrueDarts *tool = qgraphicsitem_cast<VToolTrueDarts *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 32: // AttrName1
            setPointName1<VToolTrueDarts>(value.toString());
            break;
        case 33: // AttrName2
            setPointName2<VToolTrueDarts>(value.toString());
            break;
        case 6:  // AttrFirstPoint
            tool->SetBaseLineP1Id(value.toInt());
            break;
        case 7: // AttrSecondPoint
            tool->SetBaseLineP2Id(value.toInt());
            break;
        case 43: // AttrDartP1
            tool->SetDartP1Id(value.toInt());
            break;
        case 44: // AttrDartP2
            tool->SetDartP2Id(value.toInt());
            break;
        case 45: // AttrDartP3
            tool->SetDartP3Id(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCutArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutArc *tool = qgraphicsitem_cast<VToolCutArc *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolCutArc>(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormula(value.value<VFormula>());
            break;
        case 13: // AttrArc
            tool->setCurveCutId(value.toInt());
            break;
        case 62: // AttrDiretion
            tool->setDirection(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCutSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutSpline *tool = qgraphicsitem_cast<VToolCutSpline *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolCutSpline>(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormula(value.value<VFormula>());
            break;
        case 46: // AttrCurve
            tool->setCurveCutId(value.toInt());
            break;
        case 62: // AttrDiretion
            tool->setDirection(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCutSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutSplinePath *tool = qgraphicsitem_cast<VToolCutSplinePath *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolCutSplinePath>(value.toString());
            break;
        case 4: // AttrLength
            tool->SetFormula(value.value<VFormula>());
            break;
        case 46: // AttrCurve
            tool->setCurveCutId(value.toInt());
            break;
        case 62: // AttrDiretion
            tool->setDirection(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolHeight(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolHeight *tool = qgraphicsitem_cast<VToolHeight *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolHeight>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 2:  // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 16: // AttrP1Line
            tool->SetP1LineId(value.toInt());
            break;
        case 17: // AttrP2Line
            tool->SetP2LineId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLine *tool = qgraphicsitem_cast<VToolLine *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 6: // AttrFirstPoint
            tool->SetFirstPoint(value.toInt());
            break;
        case 7: // AttrSecondPoint
            tool->SetSecondPoint(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolLineIntersect(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLineIntersect *tool = qgraphicsitem_cast<VToolLineIntersect *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolLineIntersect>(value.toString());
            break;
        case 18: // AttrP1Line1
            tool->SetP1Line1(value.toInt());
            break;
        case 19: // AttrP2Line1
            tool->SetP2Line1(value.toInt());
            break;
        case 20: // AttrP1Line2
            tool->SetP1Line2(value.toInt());
            break;
        case 21: // AttrP2Line2
            tool->SetP2Line2(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolNormal(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolNormal *tool = qgraphicsitem_cast<VToolNormal *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 0: // AttrName
            setPointName<VToolNormal>(value.toString());
            break;
        case 5: // AttrAngle
            tool->SetAngle(value.toDouble());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 2: // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 7: // AttrSecondPoint
            tool->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointOfContact(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfContact *tool = qgraphicsitem_cast<VToolPointOfContact *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 8: // AttrRadius
            tool->setArcRadius(value.value<VFormula>());
            break;
        case 0: // AttrName
            setPointName<VToolPointOfContact>(value.toString());
            break;
        case 11: // AttrCenter
            tool->setCenter(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            tool->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            tool->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointOfIntersection(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    PointIntersectXYTool *tool = qgraphicsitem_cast<PointIntersectXYTool *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<PointIntersectXYTool>(value.toString());
            break;
        case 6:  // AttrFirstPoint
            tool->setFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            tool->setSecondPointId(value.toInt());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointOfIntersectionArcs(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfIntersectionArcs *tool = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolPointOfIntersectionArcs>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCirclesCrossPoint<VToolPointOfIntersectionArcs>(value);
            break;
        }
        case 47: // AttrFirstArc
            tool->SetFirstArcId(value.toInt());
            break;
        case 48: // AttrSecondArc
            tool->SetSecondArcId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointOfIntersectionCircles(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    IntersectCirclesTool *tool = qgraphicsitem_cast<IntersectCirclesTool *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<IntersectCirclesTool>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCirclesCrossPoint<IntersectCirclesTool>(value);
            break;
        }
        case 29: // AttrC1Radius
            tool->SetFirstCircleRadius(value.value<VFormula>());
            break;
        case 30: // AttrC2Radius
            tool->SetSecondCircleRadius(value.value<VFormula>());
            break;
        case 49: // AttrC1Center
            tool->SetFirstCircleCenterId(value.toInt());
            break;
        case 50: // AttrC2Center
            tool->SetSecondCircleCenterId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointOfIntersectionCurves(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfIntersectionCurves *tool = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolPointOfIntersectionCurves>(value.toString());
            break;
        case 34: // AttrVCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCurveVCrossPoint<VToolPointOfIntersectionCurves>(value);
            break;
        }
        case 35: // AttrHCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCurveHCrossPoint<VToolPointOfIntersectionCurves>(value);
            break;
        }
        case 51: // AttrCurve1
            tool->SetFirstCurveId(value.toInt());
            break;
        case 52: // AttrCurve2
            tool->SetSecondCurveId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointFromCircleAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    IntersectCircleTangentTool *tool = qgraphicsitem_cast<IntersectCircleTangentTool *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<IntersectCircleTangentTool>(value.toString());
            break;
        case 31: // AttrCRadius
            tool->SetCircleRadius(value.value<VFormula>());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCirclesCrossPoint<IntersectCircleTangentTool>(value);
            break;
        }
        case 53: // AttrCCenter
            tool->SetCircleCenterId(value.toInt());
            break;
        case 54: // AttrTangent
            tool->SetTangentPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolPointFromArcAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointFromArcAndTangent *tool = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolPointFromArcAndTangent>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setCirclesCrossPoint<VToolPointFromArcAndTangent>(value);
            break;
        }
        case 54: // AttrTangent
            tool->SetTangentPointId(value.toInt());
            break;
        case 13: // AttrArc
            tool->SetArcId(value.toInt());
            break;;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolShoulderPoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolShoulderPoint *tool = qgraphicsitem_cast<VToolShoulderPoint *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 0: // AttrName
            setPointName<VToolShoulderPoint>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 6:  // AttrFirstPoint
            tool->SetP2Line(value.toInt());
            break;
        case 2:  // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 12: // AttrThirdPoint
            tool->setPShoulder(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolSpline *tool = qgraphicsitem_cast<VToolSpline *>(currentItem);
    SCASSERT(tool != nullptr)

    auto spl = tool->getSpline();
    VPointF point;

    const VFormula f = value.value<VFormula>();

    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 6:  // AttrFirstPoint
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spl.SetP1(point);
            tool->setSpline(spl);
            break;
        case 7:  // AttrSecondPoint
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spl.SetP4(point);
            tool->setSpline(spl);
            break;
        case 9: // AttrAngle1
            if (not f.error())
            {
                spl.SetStartAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                tool->setSpline(spl);
            }
            break;
        case 10: // AttrAngle2
            if (not f.error())
            {
                spl.SetEndAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                tool->setSpline(spl);
            }
            break;
        case 36: // AttrLength1
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC1Length(qApp->toPixel(f.getDoubleValue()), f.GetFormula(FormulaType::FromUser));
                tool->setSpline(spl);
            }
            break;
        case 37: // AttrLength2
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC2Length(qApp->toPixel(f.getDoubleValue()), f.GetFormula(FormulaType::FromUser));
                tool->setSpline(spl);
            }
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCubicBezier(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCubicBezier *tool = qgraphicsitem_cast<VToolCubicBezier *>(currentItem);
    SCASSERT(tool != nullptr)

    auto spline = tool->getSpline();
    VPointF point;

    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 55: // AttrPoint1
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP1(point);
            tool->setSpline(spline);
            break;
        case 56: // AttrPoint2
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP2(point);
            tool->setSpline(spline);
            break;
        case 57: // AttrPoint3
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP3(point);
            tool->setSpline(spline);
            break;
        case 58: // AttrPoint4
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP4(point);
            tool->setSpline(spline);
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolSplinePath *tool = qgraphicsitem_cast<VToolSplinePath *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCubicBezierPath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCubicBezierPath *tool = qgraphicsitem_cast<VToolCubicBezierPath *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolTriangle(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolTriangle *tool = qgraphicsitem_cast<VToolTriangle *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolTriangle>(value.toString());
            break;
        case 23: // AttrAxisP1
            tool->SetAxisP1Id(value.toInt());
            break;
        case 24: // AttrAxisP2
            tool->SetAxisP2Id(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            tool->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            tool->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolLineIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLineIntersectAxis *tool = qgraphicsitem_cast<VToolLineIntersectAxis *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolLineIntersectAxis>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 5: // AttrAngle
            tool->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            tool->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            tool->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolCurveIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCurveIntersectAxis *tool = qgraphicsitem_cast<VToolCurveIntersectAxis *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            setPointName<VToolCurveIntersectAxis>(value.toString());
            break;
        case 3: // AttrLineType
            tool->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            tool->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 5: // AttrAngle
            tool->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2:  // AttrBasePoint
            tool->SetBasePointId(value.toInt());
            break;
        case 46: // AttrCurve
            tool->setCurveId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolRotation(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolRotation *tool = qgraphicsitem_cast<VToolRotation *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            setOperationSuffix<VToolRotation>(value.toString());
            break;
        case 5: // AttrAngle
            tool->SetFormulaAngle(value.value<VFormula>());
            break;
        case 11: // AttrCenter
            tool->setOriginPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolMove(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMove *tool = qgraphicsitem_cast<VToolMove *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 42: // AttrRotationAngle
            tool->setFormulaRotation(value.value<VFormula>());
            break;
        case 38: // AttrSuffix
            setOperationSuffix<VToolMove>(value.toString());
            break;
        case 5: // AttrAngle
            tool->SetFormulaAngle(value.value<VFormula>());
            break;
        case 4: // AttrLength
            tool->SetFormulaLength(value.value<VFormula>());
            break;
        case 11: // AttrCenter
            tool->setOriginPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolMirrorByLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMirrorByLine *tool = qgraphicsitem_cast<VToolMirrorByLine *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            setOperationSuffix<VToolMirrorByLine>(value.toString());
            break;
        case 16:  // AttrP1Line
            tool->setFirstLinePointId(value.toInt());
            break;
        case 17:  // AttrP2Line
            tool->setSecondLinePointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolMirrorByAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMirrorByAxis *tool = qgraphicsitem_cast<VToolMirrorByAxis *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 39: // AttrAxisType
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setAxisType<VToolMirrorByAxis>(value);
            break;
        }
        case 38: // AttrSuffix
            setOperationSuffix<VToolMirrorByAxis>(value.toString());
            break;
        case 11: // AttrCenter
            tool->setOriginPointId(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::changeDataToolEllipticalArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolEllipticalArc *tool = qgraphicsitem_cast<VToolEllipticalArc *>(currentItem);
    SCASSERT(tool != nullptr)
    switch (propertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 40: // AttrRadius1
            tool->SetFormulaRadius1(value.value<VFormula>());
            break;
        case 41: // AttrRadius2
            tool->SetFormulaRadius2(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            tool->SetFormulaF1(value.value<VFormula>());
            break;
        case 10: // AttrAngle2
            tool->SetFormulaF2(value.value<VFormula>());
            break;
        case 42: // AttrRotationAngle
            tool->SetFormulaRotationAngle(value.value<VFormula>());
            break;
        case 27: // AttrColor
            tool->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            tool->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            tool->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            tool->setCenter(value.toInt());
            break;
        default:
            qWarning() << "Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolSinglePoint(QGraphicsItem *item)
{
    VToolBasePoint *tool = qgraphicsitem_cast<VToolBasePoint *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Base point"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));

    VPE::VPointFProperty* itemCoordinates = new VPE::VPointFProperty(tr("Coordinates"));
    itemCoordinates->setValue(tool->GetBasePointPos());
    addProperty(itemCoordinates, QLatin1String("Coordinates"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolEndLine(QGraphicsItem *item)
{
    VToolEndLine *tool = qgraphicsitem_cast<VToolEndLine *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Length and Angle"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("Base point:"), AttrBasePoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);
    addPropertyFormula(tr("Angle:"), tool->GetFormulaAngle(), AttrAngle);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolAlongLine(QGraphicsItem *item)
{
    VToolAlongLine *tool = qgraphicsitem_cast<VToolAlongLine *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Line"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolArc(QGraphicsItem *item)
{
    VToolArc *tool = qgraphicsitem_cast<VToolArc *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Arc - Radius and Angles"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("Arc_"), tool->CenterPointName(), QString().setNum(tool->getId()), true);
    addObjectProperty(tool, tool->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Radius:"), tool->GetFormulaRadius(), AttrRadius);
    addPropertyFormula(tr("First angle:"), tool->GetFormulaF1(), AttrAngle1);
    addPropertyFormula(tr("Second angle:"), tool->GetFormulaF2(), AttrAngle2);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolArcWithLength(QGraphicsItem *item)
{
    VToolArcWithLength *tool = qgraphicsitem_cast<VToolArcWithLength *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Arc - Radius and Length"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("Arc_"), tool->CenterPointName(), QString().setNum(tool->getId()), true);
    addObjectProperty(tool, tool->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Radius:"), tool->GetFormulaRadius(), AttrRadius);
    addPropertyFormula(tr("First angle:"), tool->GetFormulaF1(), AttrAngle1);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolBisector(QGraphicsItem *item)
{
    VToolBisector *tool = qgraphicsitem_cast<VToolBisector *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Bisector"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->BasePointName(), tr("Second point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->ThirdPointName(), tr("Third point:"), AttrThirdPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolTrueDarts(QGraphicsItem *item)
{
    VToolTrueDarts *tool = qgraphicsitem_cast<VToolTrueDarts *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("True darts"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyPointName1(tool, tr("Point 1 label:"));
    addPropertyPointName2(tool, tr("Point 2 label:"));
    addObjectProperty(tool, tool->BaseLineP1Name(), tr("First base point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->BaseLineP2Name(), tr("Second base point:"), AttrSecondPoint, GOType::Point);
    addObjectProperty(tool, tool->DartP1Name(), tr("First dart point:"), AttrDartP1, GOType::Point);
    addObjectProperty(tool, tool->DartP2Name(), tr("Second dart point:"), AttrDartP2, GOType::Point);
    addObjectProperty(tool, tool->DartP3Name(), tr("Third dart point:"), AttrDartP3, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCutArc(QGraphicsItem *item)
{
    VToolCutArc *tool = qgraphicsitem_cast<VToolCutArc *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Arc"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->CurveName(), tr("Arc:"), AttrArc, GOType::Arc);
    addPropertyDirection(tool, tr("Direction:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormula(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCutSpline(QGraphicsItem *item)
{
    VToolCutSpline *tool = qgraphicsitem_cast<VToolCutSpline *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Curve"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->CurveName(), tr("Curve:"), AttrCurve, GOType::Curve);
    addPropertyDirection(tool, tr("Direction:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormula(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCutSplinePath(QGraphicsItem *item)
{
    VToolCutSplinePath *tool = qgraphicsitem_cast<VToolCutSplinePath *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Spline"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->CurveName(), tr("Curve:"), AttrCurve, GOType::Path);
    addPropertyDirection(tool, tr("Direction:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormula(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolHeight(QGraphicsItem *item)
{
    VToolHeight *tool = qgraphicsitem_cast<VToolHeight *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Line and Perpendicular"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("Base point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->FirstLinePointName(), tr("First line point:"), AttrP1Line, GOType::Point);
    addObjectProperty(tool, tool->SecondLinePointName(), tr("Second line point:"), AttrP2Line, GOType::Point);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolLine(QGraphicsItem *item)
{
    VToolLine *tool = qgraphicsitem_cast<VToolLine *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Line"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyLineName(tool, tr("Name:"), true);
    addObjectProperty(tool, tool->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolLineIntersect(QGraphicsItem *item)
{
    VToolLineIntersect *tool = qgraphicsitem_cast<VToolLineIntersect *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Lines"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addPropertyLabel(tr("First line"), AttrName);
    addObjectProperty(tool, tool->Line1P1Name(), tr("First point:"), AttrP1Line1, GOType::Point);
    addObjectProperty(tool, tool->Line1P2Name(), tr("Second point:"), AttrP2Line1, GOType::Point);
    addPropertyLabel(tr("Second line"), AttrName);
    addObjectProperty(tool, tool->Line2P1Name(), tr("First point:"), AttrP1Line2, GOType::Point);
    addObjectProperty(tool, tool->Line2P2Name(), tr("Second point:"), AttrP2Line2, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolNormal(QGraphicsItem *item)
{
    VToolNormal *tool = qgraphicsitem_cast<VToolNormal *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - On Perpendicular"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);
    addObjectProperty(tool, tool->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    VPE::DoubleSpinboxProperty* itemAngle = new VPE::DoubleSpinboxProperty(tr("Rotation:"));
    itemAngle->setValue(tool->GetAngle());
    itemAngle->setSetting("Min", -360);
    itemAngle->setSetting("Max", 360);
    itemAngle->setSetting("Precision", 3);
    addProperty(itemAngle, AttrAngle);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointOfContact(QGraphicsItem *item)
{
    VToolPointOfContact *tool = qgraphicsitem_cast<VToolPointOfContact *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arc and Line"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->ArcCenterPointName(), tr("Center of arc:"), AttrCenter, GOType::Point);
    addObjectProperty(tool, tool->FirstPointName(), tr("1st line point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("2nd line point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Radius:"), tool->getArcRadius(), AttrRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointOfIntersection(QGraphicsItem *item)
{
    PointIntersectXYTool *tool = qgraphicsitem_cast<PointIntersectXYTool *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect XY"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->firstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->secondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointOfIntersectionArcs(QGraphicsItem *item)
{
    VToolPointOfIntersectionArcs *tool = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arcs"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->FirstArcName(), tr("First arc:"), AttrFirstArc, GOType::Arc);
    addObjectProperty(tool, tool->SecondArcName(), tr("Second arc:"), AttrSecondArc, GOType::Arc);
    addPropertyCrossPoint(tool, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointOfIntersectionCircles(QGraphicsItem *item)
{
    IntersectCirclesTool *tool = qgraphicsitem_cast<IntersectCirclesTool *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Circles"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addPropertyCrossPoint(tool, tr("Take:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyLabel(tr("First circle:"), AttrName);
    addObjectProperty(tool, tool->FirstCircleCenterPointName(), tr("Center:"), AttrC1Center, GOType::Point);
    addPropertyFormula(tr("Radius:"), tool->GetFirstCircleRadius(), AttrC1Radius);

    addPropertyLabel(tr("Second circle:"), AttrName);
    addObjectProperty(tool, tool->SecondCircleCenterPointName(), tr("Center:"), AttrC2Center, GOType::Point);
    addPropertyFormula(tr("Radius:"), tool->GetSecondCircleRadius(), AttrC2Radius);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointOfIntersectionCurves(QGraphicsItem *item)
{
    VToolPointOfIntersectionCurves *tool = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Curves"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->FirstCurveName(), tr("First curve:"), AttrCurve1, GOType::AllCurves);
    addObjectProperty(tool, tool->SecondCurveName(), tr("Second curve:"), AttrCurve2, GOType::AllCurves);
    addPropertyVCrossPoint(tool, tr("Vertical take:"));
    addPropertyHCrossPoint(tool, tr("Horizontal take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointFromCircleAndTangent(QGraphicsItem *item)
{
    IntersectCircleTangentTool *tool = qgraphicsitem_cast<IntersectCircleTangentTool *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Circle and Tangent"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->CircleCenterPointName(), tr("Center point:"), AttrCCenter, GOType::Point);
    addObjectProperty(tool, tool->TangentPointName(), tr("Tangent point:"), AttrTangent, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Circle radius:"), tool->GetCircleRadius(), AttrCRadius);
    addPropertyCrossPoint(tool, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolPointFromArcAndTangent(QGraphicsItem *item)
{
    VToolPointFromArcAndTangent *tool = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arc and Tangent"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->TangentPointName(), tr("Tangent point:"), AttrTangent, GOType::Point);
    addObjectProperty(tool, tool->ArcName(), tr("Arc:"), AttrArc, GOType::Arc);
    addPropertyCrossPoint(tool, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolShoulderPoint(QGraphicsItem *item)
{
    VToolShoulderPoint *tool = qgraphicsitem_cast<VToolShoulderPoint *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Length to Line"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);
    addObjectProperty(tool, tool->ShoulderPointName(), tr("Third point:"), AttrThirdPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolSpline(QGraphicsItem *item)
{
    VToolSpline *tool = qgraphicsitem_cast<VToolSpline *>(item);
    tool->ShowVisualization(true);
    const auto spl = tool->getSpline();

    formView->setTitle(tr("Curve - Interactive"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("Spl_"), spl.GetP1().name(), spl.GetP4().name(), true);

    addObjectProperty(tool, spl.GetP1().name(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, spl.GetP4().name(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);

    VFormula length1(spl.GetC1LengthFormula(), tool->getData());
    length1.setCheckZero(false);
    length1.setToolId(tool->getId());
    length1.setPostfix(UnitsToStr(qApp->patternUnit()));
    addPropertyFormula(tr("C1: length:"), length1, AttrLength1);

    VFormula angle1(spl.GetStartAngleFormula(), tool->getData());
    angle1.setCheckZero(false);
    angle1.setToolId(tool->getId());
    angle1.setPostfix(degreeSymbol);
    addPropertyFormula(tr("C1: angle:"), angle1, AttrAngle1);

    VFormula length2(spl.GetC2LengthFormula(), tool->getData());
    length2.setCheckZero(false);
    length2.setToolId(tool->getId());
    length2.setPostfix(UnitsToStr(qApp->patternUnit()));
    addPropertyFormula(tr("C2: length:"), length2, AttrLength2);

    VFormula angle2(spl.GetEndAngleFormula(), tool->getData());
    angle2.setCheckZero(false);
    angle2.setToolId(tool->getId());
    angle2.setPostfix(degreeSymbol);
    addPropertyFormula(tr("C2: angle:"), angle2, AttrAngle2);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCubicBezier(QGraphicsItem *item)
{
    VToolCubicBezier *tool = qgraphicsitem_cast<VToolCubicBezier *>(item);
    tool->ShowVisualization(true);
    const auto spl = tool->getSpline();

    formView->setTitle(tr("Curve - Fixed"));
    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("Spl_"), spl.GetP1().name(), spl.GetP4().name(), true);
    addObjectProperty(tool, spl.GetP1().name(), tr("First point:"),  AttrPoint1, GOType::Point);
    addObjectProperty(tool, spl.GetP2().name(), tr("Second point:"), AttrPoint2, GOType::Point);
    addObjectProperty(tool, spl.GetP3().name(), tr("Third point:"),  AttrPoint3, GOType::Point);
    addObjectProperty(tool, spl.GetP4().name(), tr("Fourth point:"), AttrPoint4, GOType::Point);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolSplinePath(QGraphicsItem *item)
{
    VToolSplinePath *tool = qgraphicsitem_cast<VToolSplinePath *>(item);
    tool->ShowVisualization(true);

    const auto spl = tool->getSplinePath();

    formView->setTitle(tr("Spline - Interactive"));
    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("SplPath_"), spl.FirstPoint().name(), spl.LastPoint().name(), true);
    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCubicBezierPath(QGraphicsItem *item)
{
    VToolCubicBezierPath *tool = qgraphicsitem_cast<VToolCubicBezierPath *>(item);
    tool->ShowVisualization(true);
    const auto spl = tool->getSplinePath();

    formView->setTitle(tr("Spline - Fixed"));
    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("SplPath_"), spl.FirstPoint().name(), spl.LastPoint().name(), true);
    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolTriangle(QGraphicsItem *item)
{
    VToolTriangle *tool = qgraphicsitem_cast<VToolTriangle *>(item);
    tool->ShowVisualization(true);

    formView->setTitle(tr("Point - Intersect Axis and Triangle"));
    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->AxisP1Name(), tr("1st axis point:"), AttrAxisP1, GOType::Point);
    addObjectProperty(tool, tool->AxisP2Name(), tr("2nd axis point:"), AttrAxisP2, GOType::Point);
    addObjectProperty(tool, tool->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolLineIntersectAxis(QGraphicsItem *item)
{
    VToolLineIntersectAxis *tool = qgraphicsitem_cast<VToolLineIntersectAxis *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Line and Axis"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("Axis point:"), AttrBasePoint, GOType::Point);
    addObjectProperty(tool, tool->FirstLinePoint(), tr("First point:"), AttrFirstPoint, GOType::Point);
    addObjectProperty(tool, tool->SecondLinePoint(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Angle:"), tool->GetFormulaAngle(), AttrAngle);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolCurveIntersectAxis(QGraphicsItem *item)
{
    VToolCurveIntersectAxis *tool = qgraphicsitem_cast<VToolCurveIntersectAxis *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Curve and Axis"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyObjectName(tool, tr("Name:"));
    addObjectProperty(tool, tool->BasePointName(), tr("Axis point:"), AttrBasePoint, GOType::Point);

    addObjectProperty(tool, tool->CurveName(), tr("Curve:"), AttrCurve, GOType::AllCurves);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Angle:"), tool->GetFormulaAngle(), AttrAngle);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrLineColor);
    addPropertyLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolRotation(QGraphicsItem *item)
{
    VToolRotation *tool = qgraphicsitem_cast<VToolRotation *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Rotation"));

    addPropertyLabel(tr("Selection"), AttrName);
    addObjectProperty(tool, tool->getOriginPointName(), tr("Rotation point:"), AttrCenter, GOType::Point);
    addPropertyOperationSuffix(tool, tr("Suffix:"));

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Angle:"), tool->GetFormulaAngle(), AttrAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolMove(QGraphicsItem *item)
{
    VToolMove *tool = qgraphicsitem_cast<VToolMove *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Move"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyOperationSuffix(tool, tr("Suffix:"));
    QString originPointName = tool->getOriginPointName();
    addObjectProperty(tool, originPointName, tr("Origin point:"), AttrCenter, GOType::Point);
    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Angle:"), tool->GetFormulaAngle(), AttrAngle);
    addPropertyFormula(tr("Length:"), tool->GetFormulaLength(), AttrLength);
    addPropertyFormula(tr("Rotation angle:"), tool->getFormulaRotation(), AttrRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolMirrorByLine(QGraphicsItem *item)
{
    VToolMirrorByLine *tool = qgraphicsitem_cast<VToolMirrorByLine *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Mirror by Line"));

    addPropertyLabel(tr("Selection"), AttrName);

    addObjectProperty(tool, tool->firstLinePointName(), tr("First line point:"), AttrP1Line, GOType::Point);
    addObjectProperty(tool, tool->secondLinePointName(), tr("Second line point:"), AttrP2Line, GOType::Point);

    addPropertyOperationSuffix(tool, tr("Suffix:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolMirrorByAxis(QGraphicsItem *item)
{
    VToolMirrorByAxis *tool = qgraphicsitem_cast<VToolMirrorByAxis *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Mirror by Axis"));

    addPropertyLabel(tr("Selection"), AttrName);
    addObjectProperty(tool, tool->getOriginPointName(), tr("Axis point:"), AttrCenter, GOType::Point);
    addPropertyAxisType(tool, tr("Axis type:"));
    addPropertyOperationSuffix(tool, tr("Suffix:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::showOptionsToolEllipticalArc(QGraphicsItem *item)
{
    VToolEllipticalArc *tool = qgraphicsitem_cast<VToolEllipticalArc *>(item);
    tool->ShowVisualization(true);
    formView->setTitle(tr("Arc - Elliptical"));

    addPropertyLabel(tr("Selection"), AttrName);
    addPropertyCurveName(tool, tr("Name:"), tr("Arc_"), tool->CenterPointName(), QString().setNum(tool->getId()), true);
    addObjectProperty(tool, tool->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    addPropertyLabel(tr("Geometry"), AttrName);
    addPropertyFormula(tr("Radius:"), tool->GetFormulaRadius1(), AttrRadius1);
    addPropertyFormula(tr("Radius:"), tool->GetFormulaRadius2(), AttrRadius2);
    addPropertyFormula(tr("First angle:"), tool->GetFormulaF1(), AttrAngle1);
    addPropertyFormula(tr("Second angle:"), tool->GetFormulaF2(), AttrAngle2);
    addPropertyFormula(tr("Rotation angle:"), tool->GetFormulaRotationAngle(), AttrRotationAngle);

    addPropertyLabel(tr("Attributes"), AttrName);
    addPropertyLineColor(tool, tr("Color:"), AttrColor);
    addPropertyCurveLineType(tool, tr("Linetype:"));
    addPropertyLineWeight(tool, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolSinglePoint()
{
    VToolBasePoint *tool = qgraphicsitem_cast<VToolBasePoint *>(currentItem);
    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[QLatin1String("Coordinates")]->setValue(tool->GetBasePointPos());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolEndLine()
{
    VToolEndLine *tool = qgraphicsitem_cast<VToolEndLine *>(currentItem);
    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueAngle;
    valueAngle.setValue(tool->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolAlongLine()
{
    VToolAlongLine *tool = qgraphicsitem_cast<VToolAlongLine *>(currentItem);
    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolArc()
{
    VToolArc *tool = qgraphicsitem_cast<VToolArc *>(currentItem);
    idToProperty[AttrObjName]->setValue(tr("Arc_") + tool->CenterPointName() + "_" + QString().setNum(tool->getId()));

    QVariant valueRadius;
    valueRadius.setValue(tool->GetFormulaRadius());
    idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(tool->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueSecondAngle;
    valueSecondAngle.setValue(tool->GetFormulaF2());
    idToProperty[AttrAngle2]->setValue(valueSecondAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolArcWithLength()
{
    VToolArcWithLength *tool = qgraphicsitem_cast<VToolArcWithLength *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Arc_") + tool->CenterPointName() + "_" + QString().setNum(tool->getId()));

    QVariant valueRadius;
    valueRadius.setValue(tool->GetFormulaRadius());
    idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(tool->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueLength;
    valueLength.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueLength);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolBisector()
{
    VToolBisector *tool = qgraphicsitem_cast<VToolBisector *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->ThirdPointName());
        idToProperty[AttrThirdPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolTrueDarts()
{
    VToolTrueDarts *tool = qgraphicsitem_cast<VToolTrueDarts *>(currentItem);

    idToProperty[AttrName1]->setValue(tool->nameP1());
    idToProperty[AttrName2]->setValue(tool->nameP2());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BaseLineP1Name());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BaseLineP2Name());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->DartP1Name());
        idToProperty[AttrDartP1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->DartP2Name());
        idToProperty[AttrDartP2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->DartP3Name());
        idToProperty[AttrDartP3]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCutArc()
{
    VToolCutArc *tool = qgraphicsitem_cast<VToolCutArc *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Arc),
                                                                               tool->CurveName());
        idToProperty[AttrArc]->setValue(index);
    }

    {
        const qint32 index = VPE::DirectionProperty::indexOfDirection(directionList(), tool->getDirection());
        idToProperty[AttrDirection]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCutSpline()
{
    VToolCutSpline *tool = qgraphicsitem_cast<VToolCutSpline *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Spline),
                                                                               tool->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    {
        const qint32 index = VPE::DirectionProperty::indexOfDirection(directionList(), tool->getDirection());
        idToProperty[AttrDirection]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCutSplinePath()
{
    VToolCutSplinePath *tool = qgraphicsitem_cast<VToolCutSplinePath *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::SplinePath),
                                                                               tool->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    {
        const qint32 index = VPE::DirectionProperty::indexOfDirection(directionList(), tool->getDirection());
        idToProperty[AttrDirection]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolHeight()
{
    VToolHeight *tool = qgraphicsitem_cast<VToolHeight *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstLinePointName());
        idToProperty[AttrP1Line]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondLinePointName());
        idToProperty[AttrP2Line]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolLine()
{
    VToolLine *tool = qgraphicsitem_cast<VToolLine *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Line_") + tool->FirstPointName() + "_" + tool->SecondPointName());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolLineIntersect()
{
    VToolLineIntersect *tool = qgraphicsitem_cast<VToolLineIntersect *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->Line1P1Name());
        idToProperty[AttrP1Line1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->Line1P2Name());
        idToProperty[AttrP2Line1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->Line2P1Name());
        idToProperty[AttrP1Line2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->Line2P2Name());
        idToProperty[AttrP2Line2]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolNormal()
{
    VToolNormal *tool = qgraphicsitem_cast<VToolNormal *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(tool->name());

    idToProperty[AttrAngle]->setValue( tool->GetAngle());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointOfContact()
{
    VToolPointOfContact *tool = qgraphicsitem_cast<VToolPointOfContact *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(tool->getArcRadius());
    idToProperty[AttrRadius]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->ArcCenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointOfIntersection()
{
    PointIntersectXYTool *tool = qgraphicsitem_cast<PointIntersectXYTool *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->firstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->secondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointOfIntersectionArcs()
{
    VToolPointOfIntersectionArcs *tool = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(tool->GetCrossCirclesPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Arc),
                                                                               tool->FirstArcName());
        idToProperty[AttrFirstArc]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Arc),
                                                                               tool->SecondArcName());
        idToProperty[AttrSecondArc]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointOfIntersectionCircles()
{
    IntersectCirclesTool *tool = qgraphicsitem_cast<IntersectCirclesTool *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(tool->GetCrossCirclesPoint())-1);

    QVariant c1Radius;
    c1Radius.setValue(tool->GetFirstCircleRadius());
    idToProperty[AttrC1Radius]->setValue(c1Radius);

    QVariant c2Radius;
    c2Radius.setValue(tool->GetSecondCircleRadius());
    idToProperty[AttrC2Radius]->setValue(c2Radius);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstCircleCenterPointName());
        idToProperty[AttrC1Center]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondCircleCenterPointName());
        idToProperty[AttrC2Center]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointOfIntersectionCurves()
{
    VToolPointOfIntersectionCurves *tool = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[AttrVCrossPoint]->setValue(static_cast<int>(tool->GetVCrossPoint())-1);
    idToProperty[AttrHCrossPoint]->setValue(static_cast<int>(tool->GetHCrossPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstCurveName());
        idToProperty[AttrCurve1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondCurveName());
        idToProperty[AttrCurve2]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointFromCircleAndTangent()
{
    IntersectCircleTangentTool *tool = qgraphicsitem_cast<IntersectCircleTangentTool *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(tool->GetCrossCirclesPoint())-1);

    QVariant cRadius;
    cRadius.setValue(tool->GetCircleRadius());
    idToProperty[AttrCRadius]->setValue(cRadius);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->CircleCenterPointName());
        idToProperty[AttrCCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->TangentPointName());
        idToProperty[AttrTangent]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolPointFromArcAndTangent()
{
    VToolPointFromArcAndTangent *tool = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(tool->GetCrossCirclesPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->TangentPointName());
        idToProperty[AttrTangent]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Arc),
                                                                               tool->ArcName());
        idToProperty[AttrArc]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolShoulderPoint()
{
    VToolShoulderPoint *tool = qgraphicsitem_cast<VToolShoulderPoint *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->ShoulderPointName());
        idToProperty[AttrThirdPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolSpline()
{
    VToolSpline *tool = qgraphicsitem_cast<VToolSpline *>(currentItem);
    const VSpline spl = tool->getSpline();
    idToProperty[AttrObjName]->setValue(tr("Spl_") + spl.GetP1().name() + "_" + spl.GetP4().name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP1().name());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP4().name());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    VFormula angle1F(spl.GetStartAngleFormula(), tool->getData());
    angle1F.setCheckZero(false);
    angle1F.setToolId(tool->getId());
    angle1F.setPostfix(degreeSymbol);
    QVariant angle1;
    angle1.setValue(angle1F);
    idToProperty[AttrAngle1]->setValue(angle1);

    VFormula length1F(spl.GetC1LengthFormula(), tool->getData());
    length1F.setCheckZero(false);
    length1F.setToolId(tool->getId());
    length1F.setPostfix(UnitsToStr(qApp->patternUnit()));
    QVariant length1;
    length1.setValue(length1F);
    idToProperty[AttrLength1]->setValue(length1);

    VFormula angle2F(spl.GetEndAngleFormula(), tool->getData());
    angle2F.setCheckZero(false);
    angle2F.setToolId(tool->getId());
    angle2F.setPostfix(degreeSymbol);
    QVariant angle2;
    angle2.setValue(angle2F);
    idToProperty[AttrAngle2]->setValue(angle2);

    VFormula length2F(spl.GetC2LengthFormula(), tool->getData());
    length2F.setCheckZero(false);
    length2F.setToolId(tool->getId());
    length2F.setPostfix(UnitsToStr(qApp->patternUnit()));
    QVariant length2;
    length2.setValue(length2F);
    idToProperty[AttrLength2]->setValue(length2);



    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            tool->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCubicBezier()
{
    VToolCubicBezier *tool = qgraphicsitem_cast<VToolCubicBezier *>(currentItem);
    const auto spl = tool->getSpline();
    idToProperty[AttrObjName]->setValue(tr("Spl_") + spl.GetP1().name() + "_" + spl.GetP4().name());

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            tool->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP1().name());
        idToProperty[AttrPoint1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP2().name());
        idToProperty[AttrPoint2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP3().name());
        idToProperty[AttrPoint3]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               spl.GetP4().name());
        idToProperty[AttrPoint4]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolSplinePath()
{
    VToolSplinePath *tool = qgraphicsitem_cast<VToolSplinePath *>(currentItem);

    idToProperty[AttrName]->setValue(qApp->translateVariables()->VarToUser(tool->name()));

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            tool->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCubicBezierPath()
{
    VToolCubicBezierPath *tool = qgraphicsitem_cast<VToolCubicBezierPath *>(currentItem);

    idToProperty[AttrName]->setValue(qApp->translateVariables()->VarToUser(tool->name()));

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            tool->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolTriangle()
{
    VToolTriangle *tool = qgraphicsitem_cast<VToolTriangle *>(currentItem);

    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->AxisP1Name());
        idToProperty[AttrAxisP1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->AxisP2Name());
        idToProperty[AttrAxisP2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolLineIntersectAxis()
{
    VToolLineIntersectAxis *tool = qgraphicsitem_cast<VToolLineIntersectAxis *>(currentItem);
    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->FirstLinePoint());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->SecondLinePoint());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(tool->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolCurveIntersectAxis()
{
    VToolCurveIntersectAxis *tool = qgraphicsitem_cast<VToolCurveIntersectAxis *>(currentItem);
    idToProperty[AttrName]->setValue(tool->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeList(), tool->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::AllCurves),
                                                                               tool->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(tool->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolRotation()
{
    VToolRotation *tool = qgraphicsitem_cast<VToolRotation *>(currentItem);
    idToProperty[AttrSuffix]->setValue(tool->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(tool->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolMove()
{
    VToolMove *tool = qgraphicsitem_cast<VToolMove *>(currentItem);
    idToProperty[AttrSuffix]->setValue(tool->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(tool->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueLength;
    valueLength.setValue(tool->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueLength);

    QVariant valueRotation;
    valueRotation.setValue(tool->getFormulaRotation());
    idToProperty[AttrRotationAngle]->setValue(valueRotation);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolMirrorByLine()
{
    VToolMirrorByLine *tool = qgraphicsitem_cast<VToolMirrorByLine *>(currentItem);
    idToProperty[AttrSuffix]->setValue(tool->Suffix());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->firstLinePointName());
        idToProperty[AttrP1Line]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->secondLinePointName());
        idToProperty[AttrP2Line]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolMirrorByAxis()
{
    VToolMirrorByAxis *tool = qgraphicsitem_cast<VToolMirrorByAxis *>(currentItem);
    idToProperty[AttrAxisType]->setValue(static_cast<int>(tool->getAxisType())-1);
    idToProperty[AttrSuffix]->setValue(tool->Suffix());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::updateOptionsToolEllipticalArc()
{
    VToolEllipticalArc *tool = qgraphicsitem_cast<VToolEllipticalArc *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Arc_") + tool->CenterPointName() + "_" + QString().setNum(tool->getId()));

    QVariant valueFormulaRadius1;
    valueFormulaRadius1.setValue(tool->GetFormulaRadius1());
    idToProperty[AttrRadius1]->setValue(valueFormulaRadius1);

    QVariant valueFormulaRadius2;
    valueFormulaRadius2.setValue(tool->GetFormulaRadius2());
    idToProperty[AttrRadius2]->setValue(valueFormulaRadius2);

    QVariant valueFormulaF1;
    valueFormulaF1.setValue(tool->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFormulaF1);

    QVariant valueFormulaF2;
    valueFormulaF2.setValue(tool->GetFormulaF2());
    idToProperty[AttrAngle2]->setValue(valueFormulaF2);

    QVariant valueFormulaRotationAngle;
    valueFormulaRotationAngle.setValue(tool->GetFormulaRotationAngle());
    idToProperty[AttrRotationAngle]->setValue(valueFormulaRotationAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(tool, GOType::Point),
                                                                               tool->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), tool->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(lineTypeNoPenRemovedList(), tool->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), tool->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VToolOptionsPropertyBrowser::propertiesList() const
{
    static QStringList attr = QStringList() << AttrName                           /* 0 */
                                            << QLatin1String("Coordinates")       /* 1 */
                                            << AttrBasePoint                      /* 2 */
                                            << AttrLineType                       /* 3 */
                                            << AttrLength                         /* 4 */
                                            << AttrAngle                          /* 5 */
                                            << AttrFirstPoint                     /* 6 */
                                            << AttrSecondPoint                    /* 7 */
                                            << AttrRadius                         /* 8 */
                                            << AttrAngle1                         /* 9 */
                                            << AttrAngle2                         /* 10 */
                                            << AttrCenter                         /* 11 */
                                            << AttrThirdPoint                     /* 12 */
                                            << AttrArc                            /* 13 */
                                            << VToolCutSpline::AttrSpline         /* 14 */
                                            << VToolCutSplinePath::AttrSplinePath /* 15 */
                                            << AttrP1Line                         /* 16 */
                                            << AttrP2Line                         /* 17 */
                                            << AttrP1Line1                        /* 18 */
                                            << AttrP2Line1                        /* 19 */
                                            << AttrP1Line2                        /* 20 */
                                            << AttrP2Line2                        /* 21 */
                                            << AttrPShoulder                      /* 22 */
                                            << AttrAxisP1                         /* 23 */
                                            << AttrAxisP2                         /* 24 */
                                            << AttrKCurve /*Not used*/            /* 25 */
                                            << AttrLineColor                      /* 26 */
                                            << AttrColor                          /* 27 */
                                            << AttrCrossPoint                     /* 28 */
                                            << AttrC1Radius                       /* 29 */
                                            << AttrC2Radius                       /* 30 */
                                            << AttrCRadius                        /* 31 */
                                            << AttrName1                          /* 32 */
                                            << AttrName2                          /* 33 */
                                            << AttrVCrossPoint                    /* 34 */
                                            << AttrHCrossPoint                    /* 35 */
                                            << AttrLength1                        /* 36 */
                                            << AttrLength2                        /* 37 */
                                            << AttrSuffix                         /* 38 */
                                            << AttrAxisType                       /* 39 */
                                            << AttrRadius1                        /* 40 */
                                            << AttrRadius2                        /* 41 */
                                            << AttrRotationAngle                  /* 42 */
                                            << AttrDartP1                         /* 43 */
                                            << AttrDartP2                         /* 44 */
                                            << AttrDartP3                         /* 45 */
                                            << AttrCurve                          /* 46 */
                                            << AttrFirstArc                       /* 47 */
                                            << AttrSecondArc                      /* 48 */
                                            << AttrC1Center                       /* 49 */
                                            << AttrC2Center                       /* 50 */
                                            << AttrCurve1                         /* 51 */
                                            << AttrCurve2                         /* 52 */
                                            << AttrCCenter                        /* 53 */
                                            << AttrTangent                        /* 54 */
                                            << AttrPoint1                         /* 55 */
                                            << AttrPoint2                         /* 56 */
                                            << AttrPoint3                         /* 57 */
                                            << AttrPoint4                         /* 58 */
                                            << AttrPenStyle                       /* 59 */
                                            << AttrLineWeight                     /* 60 */
                                            << AttrObjName                        /* 61 */
                                            << AttrDirection;                     /* 62 */
    return attr;
}

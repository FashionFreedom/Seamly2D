/******************************************************************************
 *   @file   vtooloptionspropertybrowser.cpp
 **  @author Douglas S Caskey
 **  @date   21 Mar, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

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
#include "../core/vapplication.h"
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
void VToolOptionsPropertyBrowser::ClearPropertyBrowser()
{
    propertyModel->clear();
    propertyToId.clear();
    idToProperty.clear();
    currentItem = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowItemOptions(QGraphicsItem *item)
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were used in switch.");

    switch (item->type())
    {
        case VToolBasePoint::Type:
            ShowOptionsToolSinglePoint(item);
            break;
        case VToolEndLine::Type:
            ShowOptionsToolEndLine(item);
            break;
        case VToolAlongLine::Type:
            ShowOptionsToolAlongLine(item);
            break;
        case VToolArc::Type:
            ShowOptionsToolArc(item);
            break;
        case VToolArcWithLength::Type:
            ShowOptionsToolArcWithLength(item);
            break;
        case VToolBisector::Type:
            ShowOptionsToolBisector(item);
            break;
        case VToolCutArc::Type:
            ShowOptionsToolCutArc(item);
            break;
        case VToolCutSpline::Type:
            ShowOptionsToolCutSpline(item);
            break;
        case VToolCutSplinePath::Type:
            ShowOptionsToolCutSplinePath(item);
            break;
        case VToolHeight::Type:
            ShowOptionsToolHeight(item);
            break;
        case VToolLine::Type:
            ShowOptionsToolLine(item);
            break;
        case VToolLineIntersect::Type:
            ShowOptionsToolLineIntersect(item);
            break;
        case VToolNormal::Type:
            ShowOptionsToolNormal(item);
            break;
        case VToolPointOfContact::Type:
            ShowOptionsToolPointOfContact(item);
            break;
        case PointIntersectXYTool::Type:
            ShowOptionsToolPointOfIntersection(item);
            break;
        case VToolPointOfIntersectionArcs::Type:
            ShowOptionsToolPointOfIntersectionArcs(item);
            break;
        case IntersectCirclesTool::Type:
            ShowOptionsToolPointOfIntersectionCircles(item);
            break;
        case VToolPointOfIntersectionCurves::Type:
            ShowOptionsToolPointOfIntersectionCurves(item);
            break;
        case VToolShoulderPoint::Type:
            ShowOptionsToolShoulderPoint(item);
            break;
        case VToolSpline::Type:
            ShowOptionsToolSpline(item);
            break;
        case VToolCubicBezier::Type:
            ShowOptionsToolCubicBezier(item);
            break;
        case VToolSplinePath::Type:
            ShowOptionsToolSplinePath(item);
            break;
        case VToolCubicBezierPath::Type:
            ShowOptionsToolCubicBezierPath(item);
            break;
        case VToolTriangle::Type:
            ShowOptionsToolTriangle(item);
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
        case VSimplePoint::Type:
        case VSimpleCurve::Type:
            currentItem = item->parentItem();
            ShowItemOptions(currentItem);
            break;
        case VToolLineIntersectAxis::Type:
            ShowOptionsToolLineIntersectAxis(item);
            break;
        case VToolCurveIntersectAxis::Type:
            ShowOptionsToolCurveIntersectAxis(item);
            break;
        case IntersectCircleTangentTool::Type:
            ShowOptionsToolPointFromCircleAndTangent(item);
            break;
        case VToolPointFromArcAndTangent::Type:
            ShowOptionsToolPointFromArcAndTangent(item);
            break;
        case VToolTrueDarts::Type:
            ShowOptionsToolTrueDarts(item);
            break;
        case VToolRotation::Type:
            ShowOptionsToolRotation(item);
            break;
        case VToolMirrorByLine::Type:
            ShowOptionsToolMirrorByLine(item);
            break;
        case VToolMirrorByAxis::Type:
            ShowOptionsToolMirrorByAxis(item);
            break;
        case VToolMove::Type:
            ShowOptionsToolMove(item);
            break;
        case VToolEllipticalArc::Type:
            ShowOptionsToolEllipticalArc(item);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptions()
{
    if (currentItem == nullptr)
    {
        return;
    }

    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were used in switch.");

    switch (currentItem->type())
    {
        case VToolBasePoint::Type:
            UpdateOptionsToolSinglePoint();
            break;
        case VToolEndLine::Type:
            UpdateOptionsToolEndLine();
            break;
        case VToolAlongLine::Type:
            UpdateOptionsToolAlongLine();
            break;
        case VToolArc::Type:
            UpdateOptionsToolArc();
            break;
        case VToolArcWithLength::Type:
            UpdateOptionsToolArcWithLength();
            break;
        case VToolBisector::Type:
            UpdateOptionsToolBisector();
            break;
        case VToolCutArc::Type:
            UpdateOptionsToolCutArc();
            break;
        case VToolCutSpline::Type:
            UpdateOptionsToolCutSpline();
            break;
        case VToolCutSplinePath::Type:
            UpdateOptionsToolCutSplinePath();
            break;
        case VToolHeight::Type:
            UpdateOptionsToolHeight();
            break;
        case VToolLine::Type:
            UpdateOptionsToolLine();
            break;
        case VToolLineIntersect::Type:
            UpdateOptionsToolLineIntersect();
            break;
        case VToolNormal::Type:
            UpdateOptionsToolNormal();
            break;
        case VToolPointOfContact::Type:
            UpdateOptionsToolPointOfContact();
            break;
        case PointIntersectXYTool::Type:
            UpdateOptionsToolPointOfIntersection();
            break;
        case VToolPointOfIntersectionArcs::Type:
            UpdateOptionsToolPointOfIntersectionArcs();
            break;
        case IntersectCirclesTool::Type:
            UpdateOptionsToolPointOfIntersectionCircles();
            break;
        case VToolPointOfIntersectionCurves::Type:
            UpdateOptionsToolPointOfIntersectionCurves();
            break;
        case VToolShoulderPoint::Type:
            UpdateOptionsToolShoulderPoint();
            break;
        case VToolSpline::Type:
            UpdateOptionsToolSpline();
            break;
        case VToolCubicBezier::Type:
            UpdateOptionsToolCubicBezier();
            break;
        case VToolSplinePath::Type:
            UpdateOptionsToolSplinePath();
            break;
        case VToolCubicBezierPath::Type:
            UpdateOptionsToolCubicBezierPath();
            break;
        case VToolTriangle::Type:
            UpdateOptionsToolTriangle();
            break;
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
            ShowItemOptions(currentItem->parentItem());
            break;
        case VToolLineIntersectAxis::Type:
            UpdateOptionsToolLineIntersectAxis();
            break;
        case VToolCurveIntersectAxis::Type:
            UpdateOptionsToolCurveIntersectAxis();
            break;
        case IntersectCircleTangentTool::Type:
            UpdateOptionsToolPointFromCircleAndTangent();
            break;
        case VToolPointFromArcAndTangent::Type:
            UpdateOptionsToolPointFromArcAndTangent();
            break;
        case VToolTrueDarts::Type:
            UpdateOptionsToolTrueDarts();
            break;
        case VToolRotation::Type:
            UpdateOptionsToolRotation();
            break;
        case VToolMirrorByLine::Type:
            UpdateOptionsToolMirrorByLine();
            break;
        case VToolMirrorByAxis::Type:
            UpdateOptionsToolMirrorByAxis();
            break;
        case VToolMove::Type:
            UpdateOptionsToolMove();
            break;
        case VToolEllipticalArc::Type:
            UpdateOptionsToolEllipticalArc();
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::RefreshOptions()
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

    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were used in switch.");

    switch (currentItem->type())
    {
        case VToolBasePoint::Type:
            ChangeDataToolSinglePoint(prop);
            break;
        case VToolEndLine::Type:
            ChangeDataToolEndLine(prop);
            break;
        case VToolAlongLine::Type:
            ChangeDataToolAlongLine(prop);
            break;
        case VToolArc::Type:
            ChangeDataToolArc(prop);
            break;
        case VToolArcWithLength::Type:
            ChangeDataToolArcWithLength(prop);
            break;
        case VToolBisector::Type:
            ChangeDataToolBisector(prop);
            break;
        case VToolCutArc::Type:
            ChangeDataToolCutArc(prop);
            break;
        case VToolCutSpline::Type:
            ChangeDataToolCutSpline(prop);
            break;
        case VToolCutSplinePath::Type:
            ChangeDataToolCutSplinePath(prop);
            break;
        case VToolHeight::Type:
            ChangeDataToolHeight(prop);
            break;
        case VToolLine::Type:
            ChangeDataToolLine(prop);
            break;
        case VToolLineIntersect::Type:
            ChangeDataToolLineIntersect(prop);
            break;
        case VToolNormal::Type:
            ChangeDataToolNormal(prop);
            break;
        case VToolPointOfContact::Type:
            ChangeDataToolPointOfContact(prop);
            break;
        case PointIntersectXYTool::Type:
            ChangeDataToolPointOfIntersection(prop);
            break;
        case VToolPointOfIntersectionArcs::Type:
            ChangeDataToolPointOfIntersectionArcs(prop);
            break;
        case IntersectCirclesTool::Type:
            ChangeDataToolPointOfIntersectionCircles(prop);
            break;
        case VToolPointOfIntersectionCurves::Type:
            ChangeDataToolPointOfIntersectionCurves(prop);
            break;
        case VToolShoulderPoint::Type:
            ChangeDataToolShoulderPoint(prop);
            break;
        case VToolSpline::Type:
            ChangeDataToolSpline(prop);
            break;
        case VToolCubicBezier::Type:
            ChangeDataToolCubicBezier(prop);
            break;
        case VToolSplinePath::Type:
            ChangeDataToolSplinePath(prop);
            break;
        case VToolCubicBezierPath::Type:
            ChangeDataToolCubicBezierPath(prop);
            break;
        case VToolTriangle::Type:
            ChangeDataToolTriangle(prop);
            break;
        case VToolLineIntersectAxis::Type:
            ChangeDataToolLineIntersectAxis(prop);
            break;
        case VToolCurveIntersectAxis::Type:
            ChangeDataToolCurveIntersectAxis(prop);
            break;
        case IntersectCircleTangentTool::Type:
            ChangeDataToolPointFromCircleAndTangent(prop);
            break;
        case VToolPointFromArcAndTangent::Type:
            ChangeDataToolPointFromArcAndTangent(prop);
            break;
        case VToolTrueDarts::Type:
            ChangeDataToolTrueDarts(prop);
            break;
        case VToolRotation::Type:
            ChangeDataToolRotation(prop);
            break;
        case VToolMirrorByLine::Type:
            ChangeDataToolMirrorByLine(prop);
            break;
        case VToolMirrorByAxis::Type:
            ChangeDataToolMirrorByAxis(prop);
            break;
        case VToolMove::Type:
            ChangeDataToolMove(prop);
            break;
        case VToolEllipticalArc::Type:
            ChangeDataToolEllipticalArc(prop);
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
    }

    if (currentItem == item && item != nullptr)
    {
        UpdateOptions();
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

    ShowItemOptions(currentItem);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyFormula(const QString &propertyName, const VFormula &formula,
                                                     const QString &attrName)
{
    VFormulaProperty* itemLength = new VFormulaProperty(propertyName);
    itemLength->SetFormula(formula);
    AddProperty(itemLength, attrName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyObjectName(Tool *i, const QString &propertyName, bool readOnly)
{
    auto itemName = new VPE::VStringProperty(propertyName);
    readOnly == true ? itemName->setClearButtonEnable(false) : itemName->setClearButtonEnable(true);
    itemName->setValue(qApp->TrVars()->VarToUser(i->name()));
    itemName->setReadOnly(readOnly);
    AddProperty(itemName, AttrName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyPointName1(Tool *i, const QString &propertyName)
{
    VPE::VStringProperty *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(i->nameP1());
    AddProperty(itemName, AttrName1);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyPointName2(Tool *i, const QString &propertyName)
{
    VPE::VStringProperty *itemName = new VPE::VStringProperty(propertyName);
    itemName->setClearButtonEnable(true);
    itemName->setValue(i->nameP2());
    AddProperty(itemName, AttrName2);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyOperationSuffix(Tool *i, const QString &propertyName, bool readOnly)
{
    auto itemSuffix = new VPE::VStringProperty(propertyName);
    itemSuffix->setClearButtonEnable(true);
    itemSuffix->setValue(i->Suffix());
    itemSuffix->setReadOnly(readOnly);
    AddProperty(itemSuffix, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineName(Tool *i, const QString &propertyName, bool readOnly)
{
    auto lineName = new VPE::VStringProperty(propertyName);
    lineName->setValue(tr("Line_") + i->FirstPointName() + "_" + i->SecondPointName());
    lineName->setReadOnly(readOnly);
    AddProperty(lineName, AttrObjName);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyCurveName(Tool *i, const QString &propertyName, const QString &prefix,
                                                       const QString &firstPoint, const QString &secondPoint,
                                                       bool readOnly)
{
    auto arcName = new VPE::VStringProperty(propertyName);
    arcName->setValue(prefix + firstPoint + "_" + secondPoint);
    arcName->setReadOnly(readOnly);
    AddProperty(arcName, AttrObjName);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyParentPointName(const QString &pointName, const QString &propertyName,
                                                             const QString &propertyAttribute)
{
    auto itemParentPoint = new VPE::VLabelProperty(propertyName);
    itemParentPoint->setValue(pointName);
    AddProperty(itemParentPoint, propertyAttribute);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddPropertyLabel(const QString &propertyName, const QString &propertyAttribute)
{
    auto label = new VPE::VLabelProperty("<b>"+propertyName+"</b>");
    label->setValue("");
    label->setPropertyType(VPE::Property::Label);
    AddProperty(label, propertyAttribute);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyCrossPoint(Tool *i, const QString &propertyName)
{
    VPE::VEnumProperty* itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList()<< tr("First point") << tr("Second point"));
    itemProperty->setValue(static_cast<int>(i->GetCrossCirclesPoint())-1);
    AddProperty(itemProperty, AttrCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyVCrossPoint(Tool *i, const QString &propertyName)
{
    auto itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList()<< tr("Highest point") << tr("Lowest point"));
    itemProperty->setValue(static_cast<int>(i->GetVCrossPoint())-1);
    AddProperty(itemProperty, AttrVCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyHCrossPoint(Tool *i, const QString &propertyName)
{
    auto itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList()<< tr("Leftmost point") << tr("Rightmost point"));
    itemProperty->setValue(static_cast<int>(i->GetHCrossPoint())-1);
    AddProperty(itemProperty, AttrHCrossPoint);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyAxisType(Tool *i, const QString &propertyName)
{
    auto itemProperty = new VPE::VEnumProperty(propertyName);
    itemProperty->setLiterals(QStringList()<< tr("Vertical axis") << tr("Horizontal axis"));
    itemProperty->setValue(static_cast<int>(i->getAxisType())-1);
    AddProperty(itemProperty, AttrAxisType);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineType(Tool *i, const QString &propertyName)
{
    VPE::LineTypeProperty *lineTypeProperty = new VPE::LineTypeProperty(propertyName);
    lineTypeProperty->setLineTypes(LineTypeList());
    const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
    if (index == -1)
    {
        qWarning()<<"Can't find line type" << i->getLineType()<<"in list";
    }
    lineTypeProperty->setValue(index);
    AddProperty(lineTypeProperty, AttrLineType);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyCurveLineType(Tool *i, const QString &propertyName)
{
    VPE::LineTypeProperty *penStyleProperty = new VPE::LineTypeProperty(propertyName);
    penStyleProperty->setLineTypes(CurveLineTypeList());
    const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
    if (index == -1)
    {
        qWarning()<<"Can't find line type" << i->getLineType()<<"in list";
    }
    penStyleProperty->setValue(index);
    AddProperty(penStyleProperty, AttrPenStyle);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineWeight(Tool *i, const QString &propertyName)
{
    VPE::LineWeightProperty *lineWeightProperty = new VPE::LineWeightProperty(propertyName);
    lineWeightProperty->setLineWeights(lineWeightList());
    const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
    if (index == -1)
    {
        qWarning()<<"Can't find line weight" << i->getLineWeight()<<"in list";
    }
    lineWeightProperty->setValue(index);
    AddProperty(lineWeightProperty, AttrLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddPropertyLineColor(Tool *i, const QString &propertyName, const QString &id)
{
    VPE::VLineColorProperty *lineColorProperty = new VPE::VLineColorProperty(propertyName);
    lineColorProperty->setColors(VAbstractTool::ColorsList());
    const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
    if (index == -1)
    {
        qWarning()<<"Can't find line color" << i->getLineColor()<<"in list";
    }
    lineColorProperty->setValue(index);
    AddProperty(lineColorProperty, id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::AddObjectProperty(Tool *tool, const QString &pointName,
                                                   const QString &propertyName, const QString &id, GOType objType)
{
    QMap<QString, quint32> list = getObjectList(tool, objType);
    VPE::VObjectProperty *pointsProperty = new VPE::VObjectProperty(propertyName);
    pointsProperty->setObjectsList(list);
    const qint32 index = VPE::VObjectProperty::indexOfObject(list, pointName);
    if (index == -1)
    {
        qWarning()<<"Can't find point" << pointName << "in list";
    }
    pointsProperty->setValue(index);
    AddProperty(pointsProperty, id);
}

template<class Tool>
QMap<QString, quint32> VToolOptionsPropertyBrowser::getObjectList(Tool *tool, GOType objType)
{
    quint32 toolId = tool->getId();
    QHash<quint32, QSharedPointer<VGObject>> objects;

    QVector<VToolRecord> history = qApp->getCurrentDocument()->getLocalHistory();
    for (qint32 i = 0; i < history.size(); ++i)
    {
        const VToolRecord record = history.at(i);
        quint32 recId = record.getId();
        if (recId != toolId)
        {
            switch (static_cast<int>(record.getTypeTool()))
            {
                case 45:    //Tool::Rotation
                case 46:    //Tool::MirrorByLine
                case 47:    //Tool::MirrorByAxis
                case 48:    //Tool::Move
                {
                    QVector<quint32> list = qApp->getCurrentDocument()->getOpItems(recId);
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
        map.insert("Center point", NULL_ID);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetPointName(const QString &name)
{
    if (Tool *i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == i->name())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName]->setValue(i->name());
        }
        else
        {
            i->setName(name);
        }
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetPointName1(const QString &name)
{
    if (Tool *i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == i->nameP1())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName1]->setValue(i->nameP1());
        }
        else
        {
            i->setNameP1(name);
        }
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetPointName2(const QString &name)
{
    if (Tool *i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        if (name == i->nameP2())
        {
            return;
        }

        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || VContainer::IsUnique(name) == false || rx.match(name).hasMatch() == false)
        {
            idToProperty[AttrName2]->setValue(i->nameP2());
        }
        else
        {
            i->setNameP2(name);
        }
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetOperationSuffix(const QString &suffix)
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
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Type>
Type VToolOptionsPropertyBrowser::GetCrossPoint(const QVariant &value)
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
void VToolOptionsPropertyBrowser::SetCrossCirclesPoint(const QVariant &value)
{
    if (Tool *i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        i->SetCrossCirclesPoint(GetCrossPoint<CrossCirclesPoint>(value));
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetVCrossCurvesPoint(const QVariant &value)
{
    if (auto i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        i->SetVCrossPoint(GetCrossPoint<VCrossCurvesPoint>(value));
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::SetHCrossCurvesPoint(const QVariant &value)
{
    if (auto i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        i->SetHCrossPoint(GetCrossPoint<HCrossCurvesPoint>(value));
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Tool>
void VToolOptionsPropertyBrowser::setAxisType(const QVariant &value)
{
    if (auto i = qgraphicsitem_cast<Tool *>(currentItem))
    {
        i->setAxisType(GetCrossPoint<AxisType>(value));
    }
    else
    {
        qWarning()<<"Can't cast item";
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::AddProperty(VPE::VProperty *property, const QString &id)
{
    propertyToId[property] = id;
    idToProperty[id] = property;
    propertyModel->addProperty(property, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSinglePoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolBasePoint *i = qgraphicsitem_cast<VToolBasePoint *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolBasePoint>(value.toString());
            break;
        case 1: // QLatin1String("Coordinates")
            i->SetBasePointPos(value.toPointF());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolEndLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolEndLine *i = qgraphicsitem_cast<VToolEndLine *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolEndLine>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 5: // AttrAngle
            i->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolAlongLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolAlongLine *i = qgraphicsitem_cast<VToolAlongLine *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolAlongLine>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 7: // AttrSecondPoint
            i->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolArc *i = qgraphicsitem_cast<VToolArc *>(currentItem);
    SCASSERT(i != nullptr)

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 8: // AttrRadius
            i->SetFormulaRadius(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            i->SetFormulaF1(value.value<VFormula>());
            break;
        case 10: // AttrAngle2
            i->SetFormulaF2(value.value<VFormula>());
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            i->setCenter(value.toInt());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolArcWithLength(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolArcWithLength *i = qgraphicsitem_cast<VToolArcWithLength *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 8: // AttrRadius
            i->SetFormulaRadius(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            i->SetFormulaF1(value.value<VFormula>());
            break;
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            i->setCenter(value.toInt());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolBisector(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolBisector *i = qgraphicsitem_cast<VToolBisector *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolBisector>(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 6:  // AttrFirstPoint
            i->SetFirstPointId(value.toInt());
            break;
        case 2:  // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 12: // AttrThirdPoint
            i->SetThirdPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolTrueDarts(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolTrueDarts *i = qgraphicsitem_cast<VToolTrueDarts *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 32: // AttrName1
            SetPointName1<VToolTrueDarts>(value.toString());
            break;
        case 33: // AttrName2
            SetPointName2<VToolTrueDarts>(value.toString());
            break;
        case 6:  // AttrFirstPoint
            i->SetBaseLineP1Id(value.toInt());
            break;
        case 7: // AttrSecondPoint
            i->SetBaseLineP2Id(value.toInt());
            break;
        case 43: // AttrDartP1
            i->SetDartP1Id(value.toInt());
            break;
        case 44: // AttrDartP2
            i->SetDartP2Id(value.toInt());
            break;
        case 45: // AttrDartP3
            i->SetDartP3Id(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutArc *i = qgraphicsitem_cast<VToolCutArc *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutArc>(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormula(value.value<VFormula>());
            break;
        case 13: // AttrArc
            i->setCurveCutId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutSpline *i = qgraphicsitem_cast<VToolCutSpline *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutSpline>(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormula(value.value<VFormula>());
            break;
        case 46: // AttrCurve
            i->setCurveCutId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCutSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCutSplinePath *i = qgraphicsitem_cast<VToolCutSplinePath *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCutSplinePath>(value.toString());
            break;
        case 4: // AttrLength
            i->SetFormula(value.value<VFormula>());
            break;
        case 46: // AttrCurve
            i->setCurveCutId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolHeight(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolHeight *i = qgraphicsitem_cast<VToolHeight *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolHeight>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 2:  // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 16: // AttrP1Line
            i->SetP1LineId(value.toInt());
            break;
        case 17: // AttrP2Line
            i->SetP2LineId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLine *i = qgraphicsitem_cast<VToolLine *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 6: // AttrFirstPoint
            i->SetFirstPoint(value.toInt());
            break;
        case 7: // AttrSecondPoint
            i->SetSecondPoint(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLineIntersect(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLineIntersect *i = qgraphicsitem_cast<VToolLineIntersect *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolLineIntersect>(value.toString());
            break;
        case 18: // AttrP1Line1
            i->SetP1Line1(value.toInt());
            break;
        case 19: // AttrP2Line1
            i->SetP2Line1(value.toInt());
            break;
        case 20: // AttrP1Line2
            i->SetP1Line2(value.toInt());
            break;
        case 21: // AttrP2Line2
            i->SetP2Line2(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolNormal(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolNormal *i = qgraphicsitem_cast<VToolNormal *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 0: // AttrName
            SetPointName<VToolNormal>(value.toString());
            break;
        case 5: // AttrAngle
            i->SetAngle(value.toDouble());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 2: // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 7: // AttrSecondPoint
            i->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfContact(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfContact *i = qgraphicsitem_cast<VToolPointOfContact *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 8: // AttrRadius
            i->setArcRadius(value.value<VFormula>());
            break;
        case 0: // AttrName
            SetPointName<VToolPointOfContact>(value.toString());
            break;
        case 11: // AttrCenter
            i->setCenter(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            i->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            i->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersection(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    PointIntersectXYTool *i = qgraphicsitem_cast<PointIntersectXYTool *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<PointIntersectXYTool>(value.toString());
            break;
        case 6:  // AttrFirstPoint
            i->setFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            i->setSecondPointId(value.toInt());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionArcs(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfIntersectionArcs *i = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersectionArcs>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetCrossCirclesPoint<VToolPointOfIntersectionArcs>(value);
            break;
        }
        case 47: // AttrFirstArc
            i->SetFirstArcId(value.toInt());
            break;
        case 48: // AttrSecondArc
            i->SetSecondArcId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionCircles(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    IntersectCirclesTool *i = qgraphicsitem_cast<IntersectCirclesTool *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<IntersectCirclesTool>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetCrossCirclesPoint<IntersectCirclesTool>(value);
            break;
        }
        case 29: // AttrC1Radius
            i->SetFirstCircleRadius(value.value<VFormula>());
            break;
        case 30: // AttrC2Radius
            i->SetSecondCircleRadius(value.value<VFormula>());
            break;
        case 49: // AttrC1Center
            i->SetFirstCircleCenterId(value.toInt());
            break;
        case 50: // AttrC2Center
            i->SetSecondCircleCenterId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointOfIntersectionCurves(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointOfIntersectionCurves *i = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointOfIntersectionCurves>(value.toString());
            break;
        case 34: // AttrVCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetVCrossCurvesPoint<VToolPointOfIntersectionCurves>(value);
            break;
        }
        case 35: // AttrHCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetHCrossCurvesPoint<VToolPointOfIntersectionCurves>(value);
            break;
        }
        case 51: // AttrCurve1
            i->SetFirstCurveId(value.toInt());
            break;
        case 52: // AttrCurve2
            i->SetSecondCurveId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointFromCircleAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    IntersectCircleTangentTool *i = qgraphicsitem_cast<IntersectCircleTangentTool *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<IntersectCircleTangentTool>(value.toString());
            break;
        case 31: // AttrCRadius
            i->SetCircleRadius(value.value<VFormula>());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetCrossCirclesPoint<IntersectCircleTangentTool>(value);
            break;
        }
        case 53: // AttrCCenter
            i->SetCircleCenterId(value.toInt());
            break;
        case 54: // AttrTangent
            i->SetTangentPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolPointFromArcAndTangent(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolPointFromArcAndTangent *i = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolPointFromArcAndTangent>(value.toString());
            break;
        case 28: // AttrCrossPoint
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            SetCrossCirclesPoint<VToolPointFromArcAndTangent>(value);
            break;
        }
        case 54: // AttrTangent
            i->SetTangentPointId(value.toInt());
            break;
        case 13: // AttrArc
            i->SetArcId(value.toInt());
            break;;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolShoulderPoint(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolShoulderPoint *i = qgraphicsitem_cast<VToolShoulderPoint *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 0: // AttrName
            SetPointName<VToolShoulderPoint>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 6:  // AttrFirstPoint
            i->SetP2Line(value.toInt());
            break;
        case 2:  // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 12: // AttrThirdPoint
            i->setPShoulder(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSpline(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    auto i = qgraphicsitem_cast<VToolSpline *>(currentItem);
    SCASSERT(i != nullptr)

    auto spl = i->getSpline();
    VPointF point;

    const VFormula f = value.value<VFormula>();

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 6:  // AttrFirstPoint
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spl.SetP1(point);
            i->setSpline(spl);
            break;
        case 7:  // AttrSecondPoint
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spl.SetP4(point);
            i->setSpline(spl);
            break;
        case 9: // AttrAngle1
            if (not f.error())
            {
                spl.SetStartAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 10: // AttrAngle2
            if (not f.error())
            {
                spl.SetEndAngle(f.getDoubleValue(), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 36: // AttrLength1
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC1Length(qApp->toPixel(f.getDoubleValue()), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 37: // AttrLength2
            if (not f.error() && f.getDoubleValue() >= 0)
            {
                spl.SetC2Length(qApp->toPixel(f.getDoubleValue()), f.GetFormula(FormulaType::FromUser));
                i->setSpline(spl);
            }
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCubicBezier(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    auto i = qgraphicsitem_cast<VToolCubicBezier *>(currentItem);
    SCASSERT(i != nullptr)

    auto spline = i->getSpline();
    VPointF point;

    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 55: // AttrPoint1
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP1(point);
            i->setSpline(spline);
            break;
        case 56: // AttrPoint2
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP2(point);
            i->setSpline(spline);
            break;
        case 57: // AttrPoint3
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP3(point);
            i->setSpline(spline);
            break;
        case 58: // AttrPoint4
            point = *m_data->GeometricObject<VPointF>(value.toInt());
            spline.SetP4(point);
            i->setSpline(spline);
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolSplinePath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolSplinePath *i = qgraphicsitem_cast<VToolSplinePath *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCubicBezierPath(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCubicBezierPath *i = qgraphicsitem_cast<VToolCubicBezierPath *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolTriangle(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolTriangle *i = qgraphicsitem_cast<VToolTriangle *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolTriangle>(value.toString());
            break;
        case 23: // AttrAxisP1
            i->SetAxisP1Id(value.toInt());
            break;
        case 24: // AttrAxisP2
            i->SetAxisP2Id(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            i->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            i->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolLineIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolLineIntersectAxis *i = qgraphicsitem_cast<VToolLineIntersectAxis *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolLineIntersectAxis>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 5: // AttrAngle
            i->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2: // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 6:  // AttrFirstPoint
            i->SetFirstPointId(value.toInt());
            break;
        case 7:  // AttrSecondPoint
            i->SetSecondPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolCurveIntersectAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolCurveIntersectAxis *i = qgraphicsitem_cast<VToolCurveIntersectAxis *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            SetPointName<VToolCurveIntersectAxis>(value.toString());
            break;
        case 3: // AttrLineType
            i->setLineType(value.toString());
            break;
        case 26: // AttrLineColor
            i->setLineColor(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 5: // AttrAngle
            i->SetFormulaAngle(value.value<VFormula>());
            break;
        case 2:  // AttrBasePoint
            i->SetBasePointId(value.toInt());
            break;
        case 46: // AttrCurve
            i->setCurveId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolRotation(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolRotation *i = qgraphicsitem_cast<VToolRotation *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            SetOperationSuffix<VToolRotation>(value.toString());
            break;
        case 5: // AttrAngle
            i->SetFormulaAngle(value.value<VFormula>());
            break;
        case 11: // AttrCenter
            i->setOriginPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolMove(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMove *i = qgraphicsitem_cast<VToolMove *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 42: // AttrRotationAngle
            i->setFormulaRotation(value.value<VFormula>());
            break;
        case 38: // AttrSuffix
            SetOperationSuffix<VToolMove>(value.toString());
            break;
        case 5: // AttrAngle
            i->SetFormulaAngle(value.value<VFormula>());
            break;
        case 4: // AttrLength
            i->SetFormulaLength(value.value<VFormula>());
            break;
        case 11: // AttrCenter
            i->setOriginPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolMirrorByLine(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMirrorByLine *i = qgraphicsitem_cast<VToolMirrorByLine *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 38: // AttrSuffix
            SetOperationSuffix<VToolMirrorByLine>(value.toString());
            break;
        case 16:  // AttrP1Line
            i->setFirstLinePointId(value.toInt());
            break;
        case 17:  // AttrP2Line
            i->setSecondLinePointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolMirrorByAxis(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolMirrorByAxis *i = qgraphicsitem_cast<VToolMirrorByAxis *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 39: // AttrAxisType
        {
            const QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::EditRole);
            setAxisType<VToolMirrorByAxis>(value);
            break;
        }
        case 38: // AttrSuffix
            SetOperationSuffix<VToolMirrorByAxis>(value.toString());
            break;
        case 11: // AttrCenter
            i->setOriginPointId(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ChangeDataToolEllipticalArc(VPE::VProperty *property)
{
    SCASSERT(property != nullptr)

    QVariant value = property->data(VPE::VProperty::DPC_Data, Qt::DisplayRole);
    const QString id = propertyToId[property];

    VToolEllipticalArc *i = qgraphicsitem_cast<VToolEllipticalArc *>(currentItem);
    SCASSERT(i != nullptr)
    switch (PropertiesList().indexOf(id))
    {
        case 0: // AttrName
            Q_UNREACHABLE();//The attribute is read only
            break;
        case 40: // AttrRadius1
            i->SetFormulaRadius1(value.value<VFormula>());
            break;
        case 41: // AttrRadius2
            i->SetFormulaRadius2(value.value<VFormula>());
            break;
        case 9: // AttrAngle1
            i->SetFormulaF1(value.value<VFormula>());
            break;
        case 10: // AttrAngle2
            i->SetFormulaF2(value.value<VFormula>());
            break;
        case 42: // AttrRotationAngle
            i->SetFormulaRotationAngle(value.value<VFormula>());
            break;
        case 27: // AttrColor
            i->setLineColor(value.toString());
            break;
        case 59: // AttrPenStyle
            i->SetPenStyle(value.toString());
            break;
        case 60: // AttrLineWeight
            i->setLineWeight(value.toString());
            break;
        case 11: // AttrCenter
            i->setCenter(value.toInt());
            break;
        default:
            qWarning()<<"Unknown property type. id = "<<id;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSinglePoint(QGraphicsItem *item)
{
    VToolBasePoint *i = qgraphicsitem_cast<VToolBasePoint *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Base point"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));

    VPE::VPointFProperty* itemCoordinates = new VPE::VPointFProperty(tr("Coordinates"));
    itemCoordinates->setValue(i->GetBasePointPos());
    AddProperty(itemCoordinates, QLatin1String("Coordinates"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolEndLine(QGraphicsItem *item)
{
    VToolEndLine *i = qgraphicsitem_cast<VToolEndLine *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Length and Angle"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("Base point:"), AttrBasePoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolAlongLine(QGraphicsItem *item)
{
    VToolAlongLine *i = qgraphicsitem_cast<VToolAlongLine *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Line"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolArc(QGraphicsItem *item)
{
    VToolArc *i = qgraphicsitem_cast<VToolArc *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Arc - Radius and Angles"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("Arc_"), i->CenterPointName(), QString().setNum(i->getId()), true);
    AddObjectProperty(i, i->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius(), AttrRadius);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Second angle:"), i->GetFormulaF2(), AttrAngle2);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolArcWithLength(QGraphicsItem *item)
{
    VToolArcWithLength *i = qgraphicsitem_cast<VToolArcWithLength *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Arc - Radius and Length"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("Arc_"), i->CenterPointName(), QString().setNum(i->getId()), true);
    AddObjectProperty(i, i->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius(), AttrRadius);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolBisector(QGraphicsItem *item)
{
    VToolBisector *i = qgraphicsitem_cast<VToolBisector *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Bisector"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->BasePointName(), tr("Second point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->ThirdPointName(), tr("Third point:"), AttrThirdPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolTrueDarts(QGraphicsItem *item)
{
    VToolTrueDarts *i = qgraphicsitem_cast<VToolTrueDarts *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("True darts"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyPointName1(i, tr("Point 1 label:"));
    AddPropertyPointName2(i, tr("Point 2 label:"));
    AddObjectProperty(i, i->BaseLineP1Name(), tr("First base point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->BaseLineP2Name(), tr("Second base point:"), AttrSecondPoint, GOType::Point);
    AddObjectProperty(i, i->DartP1Name(), tr("First dart point:"), AttrDartP1, GOType::Point);
    AddObjectProperty(i, i->DartP2Name(), tr("Second dart point:"), AttrDartP2, GOType::Point);
    AddObjectProperty(i, i->DartP3Name(), tr("Third dart point:"), AttrDartP3, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutArc(QGraphicsItem *item)
{
    VToolCutArc *i = qgraphicsitem_cast<VToolCutArc *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Arc"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->CurveName(), tr("Arc:"), AttrArc, GOType::Arc);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormula(), AttrLength);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutSpline(QGraphicsItem *item)
{
    VToolCutSpline *i = qgraphicsitem_cast<VToolCutSpline *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Curve"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->CurveName(), tr("Curve:"), AttrCurve, GOType::Curve);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormula(), AttrLength);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCutSplinePath(QGraphicsItem *item)
{
    VToolCutSplinePath *i = qgraphicsitem_cast<VToolCutSplinePath *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Spline"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->CurveName(), tr("Curve:"), AttrCurve, GOType::Path);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormula(), AttrLength);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolHeight(QGraphicsItem *item)
{
    VToolHeight *i = qgraphicsitem_cast<VToolHeight *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Line and Perpendicular"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("Base point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->FirstLinePointName(), tr("First line point:"), AttrP1Line, GOType::Point);
    AddObjectProperty(i, i->SecondLinePointName(), tr("Second line point:"), AttrP2Line, GOType::Point);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLine(QGraphicsItem *item)
{
    VToolLine *i = qgraphicsitem_cast<VToolLine *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Line"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyLineName(i, AttrObjName, true);
    AddObjectProperty(i, i->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    QMap<QString, QString> lineType = LineTypeList();
    lineType.remove(LineTypeNone);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLineIntersect(QGraphicsItem *item)
{
    VToolLineIntersect *i = qgraphicsitem_cast<VToolLineIntersect *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Lines"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddPropertyLabel(tr("First line"), AttrName);
    AddObjectProperty(i, i->Line1P1Name(), tr("First point:"), AttrP1Line1, GOType::Point);
    AddObjectProperty(i, i->Line1P2Name(), tr("Second point:"), AttrP2Line1, GOType::Point);
    AddPropertyLabel(tr("Second line"), AttrName);
    AddObjectProperty(i, i->Line2P1Name(), tr("First point:"), AttrP1Line2, GOType::Point);
    AddObjectProperty(i, i->Line2P2Name(), tr("Second point:"), AttrP2Line2, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolNormal(QGraphicsItem *item)
{
    VToolNormal *i = qgraphicsitem_cast<VToolNormal *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - On Perpendicular"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddObjectProperty(i, i->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    VPE::DoubleSpinboxProperty* itemAngle = new VPE::DoubleSpinboxProperty(tr("Rotation:"));
    itemAngle->setValue(i->GetAngle());
    itemAngle->setSetting("Min", -360);
    itemAngle->setSetting("Max", 360);
    itemAngle->setSetting("Precision", 3);
    AddProperty(itemAngle, AttrAngle);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfContact(QGraphicsItem *item)
{
    VToolPointOfContact *i = qgraphicsitem_cast<VToolPointOfContact *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arc and Line"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->ArcCenterPointName(), tr("Center of arc:"), AttrCenter, GOType::Point);
    AddObjectProperty(i, i->FirstPointName(), tr("1st line point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("2nd line point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Radius:"), i->getArcRadius(), AttrRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersection(QGraphicsItem *item)
{
    PointIntersectXYTool *i = qgraphicsitem_cast<PointIntersectXYTool *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect XY"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->firstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->secondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionArcs(QGraphicsItem *item)
{
    VToolPointOfIntersectionArcs *i = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arcs"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->FirstArcName(), tr("First arc:"), AttrFirstArc, GOType::Arc);
    AddObjectProperty(i, i->SecondArcName(), tr("Second arc:"), AttrSecondArc, GOType::Arc);
    AddPropertyCrossPoint(i, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionCircles(QGraphicsItem *item)
{
    IntersectCirclesTool *i = qgraphicsitem_cast<IntersectCirclesTool *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Circles"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddPropertyCrossPoint(i, tr("Take:"));

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyLabel(tr("First circle:"), AttrName);
    AddObjectProperty(i, i->FirstCircleCenterPointName(), tr("Center:"), AttrC1Center, GOType::Point);
    AddPropertyFormula(tr("Radius:"), i->GetFirstCircleRadius(), AttrC1Radius);

    AddPropertyLabel(tr("Second circle:"), AttrName);
    AddObjectProperty(i, i->SecondCircleCenterPointName(), tr("Center:"), AttrC2Center, GOType::Point);
    AddPropertyFormula(tr("Radius:"), i->GetSecondCircleRadius(), AttrC2Radius);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointOfIntersectionCurves(QGraphicsItem *item)
{
    auto i = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Curves"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->FirstCurveName(), tr("First curve:"), AttrCurve1, GOType::AllCurves);
    AddObjectProperty(i, i->SecondCurveName(), tr("Second curve:"), AttrCurve2, GOType::AllCurves);
    AddPropertyVCrossPoint(i, tr("Vertical take:"));
    AddPropertyHCrossPoint(i, tr("Horizontal take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointFromCircleAndTangent(QGraphicsItem *item)
{
    IntersectCircleTangentTool *i = qgraphicsitem_cast<IntersectCircleTangentTool *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Circle and Tangent"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->CircleCenterPointName(), tr("Center point:"), AttrCCenter, GOType::Point);
    AddObjectProperty(i, i->TangentPointName(), tr("Tangent point:"), AttrTangent, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Circle radius:"), i->GetCircleRadius(), AttrCRadius);
    AddPropertyCrossPoint(i, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolPointFromArcAndTangent(QGraphicsItem *item)
{
    VToolPointFromArcAndTangent *i = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Arc and Tangent"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->TangentPointName(), tr("Tangent point:"), AttrTangent, GOType::Point);
    AddObjectProperty(i, i->ArcName(), tr("Arc:"), AttrArc, GOType::Arc);
    AddPropertyCrossPoint(i, tr("Take:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolShoulderPoint(QGraphicsItem *item)
{
    VToolShoulderPoint *i = qgraphicsitem_cast<VToolShoulderPoint *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Length to Line"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("First point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);
    AddObjectProperty(i, i->ShoulderPointName(), tr("Third point:"), AttrThirdPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSpline(QGraphicsItem *item)
{
    auto i = qgraphicsitem_cast<VToolSpline *>(item);
    i->ShowVisualization(true);
    const auto spl = i->getSpline();

    formView->setTitle(tr("Curve - Interactive"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("Spl_"), spl.GetP1().name(), spl.GetP4().name(), true);

    AddObjectProperty(i, spl.GetP1().name(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, spl.GetP4().name(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    VFormula angle1(spl.GetStartAngleFormula(), i->getData());
    angle1.setCheckZero(false);
    angle1.setToolId(i->getId());
    angle1.setPostfix(degreeSymbol);
    AddPropertyFormula(tr("C1: angle:"), angle1, AttrAngle1);

    VFormula length1(spl.GetC1LengthFormula(), i->getData());
    length1.setCheckZero(false);
    length1.setToolId(i->getId());
    length1.setPostfix(UnitsToStr(qApp->patternUnit()));
    AddPropertyFormula(tr("C1: length:"), length1, AttrLength1);

    VFormula angle2(spl.GetEndAngleFormula(), i->getData());
    angle2.setCheckZero(false);
    angle2.setToolId(i->getId());
    angle2.setPostfix(degreeSymbol);
    AddPropertyFormula(tr("C2: angle:"), angle2, AttrAngle2);

    VFormula length2(spl.GetC2LengthFormula(), i->getData());
    length2.setCheckZero(false);
    length2.setToolId(i->getId());
    length2.setPostfix(UnitsToStr(qApp->patternUnit()));
    AddPropertyFormula(tr("C2: length:"), length2, AttrLength2);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCubicBezier(QGraphicsItem *item)
{
    auto i = qgraphicsitem_cast<VToolCubicBezier *>(item);
    i->ShowVisualization(true);
    const auto spl = i->getSpline();

    formView->setTitle(tr("Curve - Fixed"));
    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("Spl_"), spl.GetP1().name(), spl.GetP4().name(), true);
    AddObjectProperty(i, spl.GetP1().name(), tr("First point:"),  AttrPoint1, GOType::Point);
    AddObjectProperty(i, spl.GetP2().name(), tr("Second point:"), AttrPoint2, GOType::Point);
    AddObjectProperty(i, spl.GetP3().name(), tr("Third point:"),  AttrPoint3, GOType::Point);
    AddObjectProperty(i, spl.GetP4().name(), tr("Fourth point:"), AttrPoint4, GOType::Point);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolSplinePath(QGraphicsItem *item)
{
    VToolSplinePath *i = qgraphicsitem_cast<VToolSplinePath *>(item);
    i->ShowVisualization(true);

    const auto spl = i->getSplinePath();

    formView->setTitle(tr("Spline - Interactive"));
    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("SplPath_"), spl.FirstPoint().name(), spl.LastPoint().name(), true);
    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCubicBezierPath(QGraphicsItem *item)
{
    VToolCubicBezierPath *i = qgraphicsitem_cast<VToolCubicBezierPath *>(item);
    i->ShowVisualization(true);
    const auto spl = i->getSplinePath();

    formView->setTitle(tr("Spline - Fixed"));
    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("SplPath_"), spl.FirstPoint().name(), spl.LastPoint().name(), true);
    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolTriangle(QGraphicsItem *item)
{
    VToolTriangle *i = qgraphicsitem_cast<VToolTriangle *>(item);
    i->ShowVisualization(true);

    formView->setTitle(tr("Point - Intersect Axis and Triangle"));
    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->AxisP1Name(), tr("1st axis point:"), AttrAxisP1, GOType::Point);
    AddObjectProperty(i, i->AxisP2Name(), tr("2nd axis point:"), AttrAxisP2, GOType::Point);
    AddObjectProperty(i, i->FirstPointName(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->SecondPointName(), tr("Second point:"), AttrSecondPoint, GOType::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolLineIntersectAxis(QGraphicsItem *item)
{
    VToolLineIntersectAxis *i = qgraphicsitem_cast<VToolLineIntersectAxis *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Line and Axis"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("Axis point:"), AttrBasePoint, GOType::Point);
    AddObjectProperty(i, i->FirstLinePoint(), tr("First point:"), AttrFirstPoint, GOType::Point);
    AddObjectProperty(i, i->SecondLinePoint(), tr("Second point:"), AttrSecondPoint, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolCurveIntersectAxis(QGraphicsItem *item)
{
    VToolCurveIntersectAxis *i = qgraphicsitem_cast<VToolCurveIntersectAxis *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Point - Intersect Curve and Axis"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyObjectName(i, tr("Point name:"));
    AddObjectProperty(i, i->BasePointName(), tr("Axis point:"), AttrBasePoint, GOType::Point);

    AddObjectProperty(i, i->CurveName(), tr("Curve:"), AttrCurve, GOType::AllCurves);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrLineColor);
    AddPropertyLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolRotation(QGraphicsItem *item)
{
    VToolRotation *i = qgraphicsitem_cast<VToolRotation *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Rotation"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddObjectProperty(i, i->getOriginPointName(), tr("Rotation point:"), AttrCenter, GOType::Point);
    AddPropertyOperationSuffix(i, tr("Suffix:"));

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolMove(QGraphicsItem *item)
{
    VToolMove *i = qgraphicsitem_cast<VToolMove *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Move"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyOperationSuffix(i, tr("Suffix:"));
    AddObjectProperty(i, i->getOriginPointName(), tr("Origin point:"), AttrCenter, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Angle:"), i->GetFormulaAngle(), AttrAngle);
    AddPropertyFormula(tr("Length:"), i->GetFormulaLength(), AttrLength);
    AddPropertyFormula(tr("Rotation angle:"), i->getFormulaRotation(), AttrRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolMirrorByLine(QGraphicsItem *item)
{
    VToolMirrorByLine *i = qgraphicsitem_cast<VToolMirrorByLine *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Mirror by Line"));

    AddPropertyLabel(tr("Selection"), AttrName);

    AddObjectProperty(i, i->firstLinePointName(), tr("First line point:"), AttrP1Line, GOType::Point);
    AddObjectProperty(i, i->secondLinePointName(), tr("Second line point:"), AttrP2Line, GOType::Point);

    AddPropertyOperationSuffix(i, tr("Suffix:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolMirrorByAxis(QGraphicsItem *item)
{
    VToolMirrorByAxis *i = qgraphicsitem_cast<VToolMirrorByAxis *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Mirror by Axis"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddObjectProperty(i, i->getOriginPointName(), tr("Axis point:"), AttrCenter, GOType::Point);
    AddPropertyAxisType(i, tr("Axis type:"));
    AddPropertyOperationSuffix(i, tr("Suffix:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::ShowOptionsToolEllipticalArc(QGraphicsItem *item)
{
    VToolEllipticalArc *i = qgraphicsitem_cast<VToolEllipticalArc *>(item);
    i->ShowVisualization(true);
    formView->setTitle(tr("Arc - Elliptical"));

    AddPropertyLabel(tr("Selection"), AttrName);
    AddPropertyCurveName(i, tr("Name:"), tr("Arc_"), i->CenterPointName(), QString().setNum(i->getId()), true);
    AddObjectProperty(i, i->CenterPointName(), tr("Center point:"), AttrCenter, GOType::Point);

    AddPropertyLabel(tr("Geometry"), AttrName);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius1(), AttrRadius1);
    AddPropertyFormula(tr("Radius:"), i->GetFormulaRadius2(), AttrRadius2);
    AddPropertyFormula(tr("First angle:"), i->GetFormulaF1(), AttrAngle1);
    AddPropertyFormula(tr("Second angle:"), i->GetFormulaF2(), AttrAngle2);
    AddPropertyFormula(tr("Rotation angle:"), i->GetFormulaRotationAngle(), AttrRotationAngle);

    AddPropertyLabel(tr("Attributes"), AttrName);
    AddPropertyLineColor(i, tr("Color:"), AttrColor);
    AddPropertyCurveLineType(i, tr("Linetype:"));
    AddPropertyLineWeight(i, tr("Lineweight:"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSinglePoint()
{
    VToolBasePoint *i = qgraphicsitem_cast<VToolBasePoint *>(currentItem);
    idToProperty[AttrName]->setValue(i->name());
    idToProperty[QLatin1String("Coordinates")]->setValue(i->GetBasePointPos());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolEndLine()
{
    VToolEndLine *i = qgraphicsitem_cast<VToolEndLine *>(currentItem);
    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolAlongLine()
{
    VToolAlongLine *i = qgraphicsitem_cast<VToolAlongLine *>(currentItem);
    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolArc()
{
    VToolArc *i = qgraphicsitem_cast<VToolArc *>(currentItem);
    idToProperty[AttrObjName]->setValue(tr("Arc_") + i->CenterPointName() + "_" + QString().setNum(i->getId()));

    QVariant valueRadius;
    valueRadius.setValue(i->GetFormulaRadius());
    idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(i->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueSecondAngle;
    valueSecondAngle.setValue(i->GetFormulaF2());
    idToProperty[AttrAngle2]->setValue(valueSecondAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolArcWithLength()
{
    VToolArcWithLength *i = qgraphicsitem_cast<VToolArcWithLength *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Arc_") + i->CenterPointName() + "_" + QString().setNum(i->getId()));

    QVariant valueRadius;
    valueRadius.setValue(i->GetFormulaRadius());
    idToProperty[AttrRadius]->setValue(valueRadius);

    QVariant valueFirstAngle;
    valueFirstAngle.setValue(i->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFirstAngle);

    QVariant valueLength;
    valueLength.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueLength);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolBisector()
{
    VToolBisector *i = qgraphicsitem_cast<VToolBisector *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->ThirdPointName());
        idToProperty[AttrThirdPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolTrueDarts()
{
    VToolTrueDarts *i = qgraphicsitem_cast<VToolTrueDarts *>(currentItem);

    idToProperty[AttrName1]->setValue(i->nameP1());
    idToProperty[AttrName2]->setValue(i->nameP2());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BaseLineP1Name());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BaseLineP2Name());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->DartP1Name());
        idToProperty[AttrDartP1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->DartP2Name());
        idToProperty[AttrDartP2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->DartP3Name());
        idToProperty[AttrDartP3]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutArc()
{
    VToolCutArc *i = qgraphicsitem_cast<VToolCutArc *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Arc),
                                                                               i->CurveName());
        idToProperty[AttrArc]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutSpline()
{
    VToolCutSpline *i = qgraphicsitem_cast<VToolCutSpline *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Spline),
                                                                               i->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCutSplinePath()
{
    VToolCutSplinePath *i = qgraphicsitem_cast<VToolCutSplinePath *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::SplinePath),
                                                                               i->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormula());
    idToProperty[AttrLength]->setValue(valueFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolHeight()
{
    VToolHeight *i = qgraphicsitem_cast<VToolHeight *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstLinePointName());
        idToProperty[AttrP1Line]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondLinePointName());
        idToProperty[AttrP2Line]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLine()
{
    VToolLine *i = qgraphicsitem_cast<VToolLine *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Line_") + i->FirstPointName() + "_" + i->SecondPointName());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }
    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLineIntersect()
{
    VToolLineIntersect *i = qgraphicsitem_cast<VToolLineIntersect *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->Line1P1Name());
        idToProperty[AttrP1Line1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->Line1P2Name());
        idToProperty[AttrP2Line1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->Line2P1Name());
        idToProperty[AttrP1Line2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->Line2P2Name());
        idToProperty[AttrP2Line2]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolNormal()
{
    VToolNormal *i = qgraphicsitem_cast<VToolNormal *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(i->name());

    idToProperty[AttrAngle]->setValue( i->GetAngle());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfContact()
{
    VToolPointOfContact *i = qgraphicsitem_cast<VToolPointOfContact *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->getArcRadius());
    idToProperty[AttrRadius]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->ArcCenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersection()
{
    PointIntersectXYTool *i = qgraphicsitem_cast<PointIntersectXYTool *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->firstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->secondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionArcs()
{
    VToolPointOfIntersectionArcs *i = qgraphicsitem_cast<VToolPointOfIntersectionArcs *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Arc),
                                                                               i->FirstArcName());
        idToProperty[AttrFirstArc]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Arc),
                                                                               i->SecondArcName());
        idToProperty[AttrSecondArc]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionCircles()
{
    IntersectCirclesTool *i = qgraphicsitem_cast<IntersectCirclesTool *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint())-1);

    QVariant c1Radius;
    c1Radius.setValue(i->GetFirstCircleRadius());
    idToProperty[AttrC1Radius]->setValue(c1Radius);

    QVariant c2Radius;
    c2Radius.setValue(i->GetSecondCircleRadius());
    idToProperty[AttrC2Radius]->setValue(c2Radius);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstCircleCenterPointName());
        idToProperty[AttrC1Center]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondCircleCenterPointName());
        idToProperty[AttrC2Center]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointOfIntersectionCurves()
{
    auto i = qgraphicsitem_cast<VToolPointOfIntersectionCurves *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());
    idToProperty[AttrVCrossPoint]->setValue(static_cast<int>(i->GetVCrossPoint())-1);
    idToProperty[AttrHCrossPoint]->setValue(static_cast<int>(i->GetHCrossPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstCurveName());
        idToProperty[AttrCurve1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondCurveName());
        idToProperty[AttrCurve2]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointFromCircleAndTangent()
{
    IntersectCircleTangentTool *i = qgraphicsitem_cast<IntersectCircleTangentTool *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint())-1);

    QVariant cRadius;
    cRadius.setValue(i->GetCircleRadius());
    idToProperty[AttrCRadius]->setValue(cRadius);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->CircleCenterPointName());
        idToProperty[AttrCCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->TangentPointName());
        idToProperty[AttrTangent]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolPointFromArcAndTangent()
{
    VToolPointFromArcAndTangent *i = qgraphicsitem_cast<VToolPointFromArcAndTangent *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());
    idToProperty[AttrCrossPoint]->setValue(static_cast<int>(i->GetCrossCirclesPoint())-1);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->TangentPointName());
        idToProperty[AttrTangent]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Arc),
                                                                               i->ArcName());
        idToProperty[AttrArc]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolShoulderPoint()
{
    VToolShoulderPoint *i = qgraphicsitem_cast<VToolShoulderPoint *>(currentItem);

    QVariant valueFormula;
    valueFormula.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueFormula);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->ShoulderPointName());
        idToProperty[AttrThirdPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSpline()
{
    auto i = qgraphicsitem_cast<VToolSpline *>(currentItem);
    const VSpline spl = i->getSpline();
    idToProperty[AttrObjName]->setValue(tr("Spl_") + spl.GetP1().name() + "_" + spl.GetP4().name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP1().name());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP4().name());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    VFormula angle1F(spl.GetStartAngleFormula(), i->getData());
    angle1F.setCheckZero(false);
    angle1F.setToolId(i->getId());
    angle1F.setPostfix(degreeSymbol);
    QVariant angle1;
    angle1.setValue(angle1F);
    idToProperty[AttrAngle1]->setValue(angle1);

    VFormula length1F(spl.GetC1LengthFormula(), i->getData());
    length1F.setCheckZero(false);
    length1F.setToolId(i->getId());
    length1F.setPostfix(UnitsToStr(qApp->patternUnit()));
    QVariant length1;
    length1.setValue(length1F);
    idToProperty[AttrLength1]->setValue(length1);

    VFormula angle2F(spl.GetEndAngleFormula(), i->getData());
    angle2F.setCheckZero(false);
    angle2F.setToolId(i->getId());
    angle2F.setPostfix(degreeSymbol);
    QVariant angle2;
    angle2.setValue(angle2F);
    idToProperty[AttrAngle2]->setValue(angle2);

    VFormula length2F(spl.GetC2LengthFormula(), i->getData());
    length2F.setCheckZero(false);
    length2F.setToolId(i->getId());
    length2F.setPostfix(UnitsToStr(qApp->patternUnit()));
    QVariant length2;
    length2.setValue(length2F);
    idToProperty[AttrLength2]->setValue(length2);



    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            i->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCubicBezier()
{
    auto i = qgraphicsitem_cast<VToolCubicBezier *>(currentItem);
    const auto spl = i->getSpline();
    idToProperty[AttrObjName]->setValue(tr("Spl_") + spl.GetP1().name() + "_" + spl.GetP4().name());

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            i->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP1().name());
        idToProperty[AttrPoint1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP2().name());
        idToProperty[AttrPoint2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP3().name());
        idToProperty[AttrPoint3]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               spl.GetP4().name());
        idToProperty[AttrPoint4]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolSplinePath()
{
    auto i = qgraphicsitem_cast<VToolSplinePath *>(currentItem);

    idToProperty[AttrName]->setValue(qApp->TrVars()->VarToUser(i->name()));

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            i->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCubicBezierPath()
{
    auto i = qgraphicsitem_cast<VToolCubicBezierPath *>(currentItem);

    idToProperty[AttrName]->setValue(qApp->TrVars()->VarToUser(i->name()));

    idToProperty[AttrColor]->setValue(VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(),
                                                                            i->getLineColor()));

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolTriangle()
{
    VToolTriangle *i = qgraphicsitem_cast<VToolTriangle *>(currentItem);

    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->AxisP1Name());
        idToProperty[AttrAxisP1]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->AxisP2Name());
        idToProperty[AttrAxisP2]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstPointName());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondPointName());
        idToProperty[AttrSecondPoint]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolLineIntersectAxis()
{
    VToolLineIntersectAxis *i = qgraphicsitem_cast<VToolLineIntersectAxis *>(currentItem);
    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->FirstLinePoint());
        idToProperty[AttrFirstPoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->SecondLinePoint());
        idToProperty[AttrSecondPoint]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolCurveIntersectAxis()
{
    VToolCurveIntersectAxis *i = qgraphicsitem_cast<VToolCurveIntersectAxis *>(currentItem);
    idToProperty[AttrName]->setValue(i->name());

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrLineColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(LineTypeList(), i->getLineType());
        idToProperty[AttrLineType]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->BasePointName());
        idToProperty[AttrBasePoint]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::AllCurves),
                                                                               i->CurveName());
        idToProperty[AttrCurve]->setValue(index);
    }

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolRotation()
{
    VToolRotation *i = qgraphicsitem_cast<VToolRotation *>(currentItem);
    idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolMove()
{
    VToolMove *i = qgraphicsitem_cast<VToolMove *>(currentItem);
    idToProperty[AttrSuffix]->setValue(i->Suffix());

    QVariant valueAngle;
    valueAngle.setValue(i->GetFormulaAngle());
    idToProperty[AttrAngle]->setValue(valueAngle);

    QVariant valueLength;
    valueLength.setValue(i->GetFormulaLength());
    idToProperty[AttrLength]->setValue(valueLength);

    QVariant valueRotation;
    valueRotation.setValue(i->getFormulaRotation());
    idToProperty[AttrRotationAngle]->setValue(valueRotation);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolMirrorByLine()
{
    VToolMirrorByLine *i = qgraphicsitem_cast<VToolMirrorByLine *>(currentItem);
    idToProperty[AttrSuffix]->setValue(i->Suffix());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->firstLinePointName());
        idToProperty[AttrP1Line]->setValue(index);
    }

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->secondLinePointName());
        idToProperty[AttrP2Line]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolMirrorByAxis()
{
    VToolMirrorByAxis *i = qgraphicsitem_cast<VToolMirrorByAxis *>(currentItem);
    idToProperty[AttrAxisType]->setValue(static_cast<int>(i->getAxisType())-1);
    idToProperty[AttrSuffix]->setValue(i->Suffix());

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->getOriginPointName());
        idToProperty[AttrCenter]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolOptionsPropertyBrowser::UpdateOptionsToolEllipticalArc()
{
    VToolEllipticalArc *i = qgraphicsitem_cast<VToolEllipticalArc *>(currentItem);

    idToProperty[AttrObjName]->setValue(tr("Arc_") + i->CenterPointName() + "_" + QString().setNum(i->getId()));

    QVariant valueFormulaRadius1;
    valueFormulaRadius1.setValue(i->GetFormulaRadius1());
    idToProperty[AttrRadius1]->setValue(valueFormulaRadius1);

    QVariant valueFormulaRadius2;
    valueFormulaRadius2.setValue(i->GetFormulaRadius2());
    idToProperty[AttrRadius2]->setValue(valueFormulaRadius2);

    QVariant valueFormulaF1;
    valueFormulaF1.setValue(i->GetFormulaF1());
    idToProperty[AttrAngle1]->setValue(valueFormulaF1);

    QVariant valueFormulaF2;
    valueFormulaF2.setValue(i->GetFormulaF2());
    idToProperty[AttrAngle2]->setValue(valueFormulaF2);

    QVariant valueFormulaRotationAngle;
    valueFormulaRotationAngle.setValue(i->GetFormulaRotationAngle());
    idToProperty[AttrRotationAngle]->setValue(valueFormulaRotationAngle);

    {
        const qint32 index = VPE::VObjectProperty::indexOfObject(getObjectList(i, GOType::Point),
                                                                               i->CenterPointName());
        idToProperty[AttrCenter]->setValue(index);
    }

    {
        const qint32 index = VPE::VLineColorProperty::indexOfColor(VAbstractTool::ColorsList(), i->getLineColor());
        idToProperty[AttrColor]->setValue(index);
    }

    {
        const qint32 index = VPE::LineTypeProperty::indexOfLineType(CurveLineTypeList(), i->GetPenStyle());
        idToProperty[AttrPenStyle]->setValue(index);
    }

    {
        const qint32 index = VPE::LineWeightProperty::indexOfLineWeight(lineWeightList(), i->getLineWeight());
        idToProperty[AttrLineWeight]->setValue(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VToolOptionsPropertyBrowser::PropertiesList() const
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
                                            << AttrObjName;                      /* 61 */
    return attr;
}

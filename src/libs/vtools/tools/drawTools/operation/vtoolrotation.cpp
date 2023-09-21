/***************************************************************************
 **  @file   vtoolrotation.cpp
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
 **  @file   vtoolrotation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2016
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

#include "vtoolrotation.h"

#include <limits.h>
#include <qiterator.h>
#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QUndoStack>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogrotation.h"
#include "../../../visualization/line/operation/vistoolrotation.h"
#include "../../../visualization/visualization.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../ifc/ifcdef.h"
#include "../ifc/exception/vexception.h"
#include "../vwidgets/vabstractsimple.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../../vdatatool.h"
#include "../vdrawtool.h"

class QDomElement;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
template <class T> class QSharedPointer;

const QString VToolRotation::ToolType = QStringLiteral("rotation");

//---------------------------------------------------------------------------------------------------------------------
VToolRotation::VToolRotation(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 originPointId,
                             const QString &angle, const QString &suffix, const QVector<SourceItem> &source,
                             const QVector<DestinationItem> &destination, const Source &typeCreation,
                             QGraphicsItem *parent)
    : VAbstractOperation(doc, data, id, suffix, source, destination, parent)
    , m_originPointId(originPointId)
    , formulaAngle(angle)
{
    InitOperatedObjects();
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogRotation> dialogTool = m_dialog.objectCast<DialogRotation>();
    SCASSERT(not dialogTool.isNull())
    dialogTool->setOriginPointId(m_originPointId);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->setSuffix(suffix);
}

//---------------------------------------------------------------------------------------------------------------------
VToolRotation *VToolRotation::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogRotation> dialogTool = dialog.objectCast<DialogRotation>();
    SCASSERT(not dialogTool.isNull())
    const quint32 originPointId = dialogTool->getOriginPointId();
    QString angle = dialogTool->GetAngle();
    const QString suffix = dialogTool->getSuffix();
    const QVector<SourceItem> source = dialogTool->getSourceObjects();

    VToolRotation* operation = Create(0, originPointId, angle, suffix, source, QVector<DestinationItem>(),
                                      scene, doc, data, Document::FullParse, Source::FromGui);
    if (operation != nullptr)
    {
        operation->m_dialog = dialogTool;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
VToolRotation *VToolRotation::Create(const quint32 _id, const quint32 &origin, QString &angle, const QString &suffix,
                                     const QVector<SourceItem> &source,
                                     const QVector<DestinationItem> &destination,
                                     VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                     const Document &parse, const Source &typeCreation)
{
    qreal calcAngle = 0;

    calcAngle = CheckFormula(_id, angle, data);

    const auto originPoint = *data->GeometricObject<VPointF>(origin);
    const QPointF oPoint = static_cast<QPointF>(originPoint);

    QVector<DestinationItem> dest = destination;

    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        dest.clear();// Try to avoid mistake, value must be empty

        id = VContainer::getNextId();//Just reserve id for tool

        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            const quint32 objectId = item.id;
            const QSharedPointer<VGObject> obj = data->GetGObject(objectId);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
            switch(static_cast<GOType>(obj->getType()))
            {
                case GOType::Point:
                    dest.append(createPoint(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::Arc:
                    dest.append(createArc<VArc>(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::EllipticalArc:
                    dest.append(createArc<VEllipticalArc>(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::Spline:
                    dest.append(createCurve<VSpline>(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::SplinePath:
                    dest.append(createCurveWithSegments<VSplinePath>(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::CubicBezier:
                    dest.append(createCurve<VCubicBezier>(id, objectId, oPoint, calcAngle, suffix, data));
                    break;
                case GOType::CubicBezierPath:
                    dest.append(createCurveWithSegments<VCubicBezierPath>(id, objectId, oPoint, calcAngle, suffix,
                                                                          data));
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
    else
    {
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            const quint32 objectId = item.id;
            const QSharedPointer<VGObject> obj = data->GetGObject(objectId);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
            switch(static_cast<GOType>(obj->getType()))
            {
                case GOType::Point:
                {
                    const DestinationItem &item = dest.at(i);
                    updatePoint(id, objectId, oPoint, calcAngle, suffix, data, item);
                    break;
                }
                case GOType::Arc:
                    updateArc<VArc>(id, objectId, oPoint, calcAngle, suffix, data, dest.at(i).id);
                    break;
                case GOType::EllipticalArc:
                    updateArc<VEllipticalArc>(id, objectId, oPoint, calcAngle, suffix, data, dest.at(i).id);
                    break;
                case GOType::Spline:
                    updateCurve<VSpline>(id, objectId, oPoint, calcAngle, suffix, data, dest.at(i).id);
                    break;
                case GOType::SplinePath:
                    updateCurveWithSegments<VSplinePath>(id, objectId, oPoint, calcAngle, suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezier:
                    updateCurve<VCubicBezier>(id, objectId, oPoint, calcAngle, suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezierPath:
                    updateCurveWithSegments<VCubicBezierPath>(id, objectId, oPoint, calcAngle, suffix, data,
                                                              dest.at(i).id);
                    break;
                case GOType::Unknown:
                case GOType::Curve:
                case GOType::Path:
                case GOType::AllCurves:
                    break;
            }
QT_WARNING_POP
        }
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Rotation, doc);
        VToolRotation *tool = new VToolRotation(doc, data, id, origin, angle, suffix, source, dest, typeCreation);
        scene->addItem(tool);
        initOperationToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);
        doc->IncrementReferens(originPoint.getIdTool());
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            doc->IncrementReferens(data->GetGObject(item.id)->getIdTool());
        }
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolRotation::getOriginPointName() const
{
    return VAbstractTool::data.GetGObject(m_originPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolRotation::getOriginPointId() const
{
    return m_originPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::setOriginPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        m_originPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolRotation::GetFormulaAngle() const
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolRotation>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolRotation *visual = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObjects(sourceToObjects(source));
        visual->SetOriginPointId(m_originPointId);
        visual->SetAngle(qApp->TrVars()->FormulaToUser(formulaAngle, qApp->Settings()->GetOsSeparator()));
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogRotation> dialogTool = m_dialog.objectCast<DialogRotation>();
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->getOriginPointId()));
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->getSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ReadToolAttributes(const QDomElement &domElement)
{
    m_originPointId = doc->GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    formulaAngle = doc->GetParametrString(domElement, AttrAngle, "0");
    suffix = doc->GetParametrString(domElement, AttrSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, QString().setNum(m_originPointId));
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrSuffix, suffix);

    SaveSourceDestination(tag);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolRotation::makeToolTip() const
{
    const QString toolTipStr = QString("<tr> <td><b>%1:</b> %2</td> </tr>"
                                       "<tr> <td><b>%3:</b> %4°</td> </tr>")
                                       .arg(tr("  Origin point"))
                                       .arg(getOriginPointName())
                                       .arg(tr("Rotation angle"))
                                       .arg(GetFormulaAngle().getDoubleValue());
    return toolTipStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogRotation>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
DestinationItem VToolRotation::createPoint(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                          const QString &suffix, VContainer *data)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF rotated = point->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);

    DestinationItem item;
    item.mx = rotated.mx();
    item.my = rotated.my();
    item.showPointName = rotated.isShowPointName();
    item.id = data->AddGObject(new VPointF(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolRotation::createItem(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                           const QString &suffix, VContainer *data)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item rotated = i->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);

    DestinationItem item;
    item.mx = INT_MAX;
    item.my = INT_MAX;
    item.id = data->AddGObject(new Item(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolRotation::createArc(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                          const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, origin, angle, suffix, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolRotation::createCurve(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                           const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, origin, angle, suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolRotation::createCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &origin,
                                                       qreal angle, const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, origin, angle, suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::updatePoint(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data, const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF rotated = point->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);
    rotated.setMx(item.mx);
    rotated.setMy(item.my);
    rotated.setShowPointName(item.showPointName);
    data->UpdateGObject(item.id, new VPointF(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::updateItem(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                               const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item rotated = i->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);
    data->UpdateGObject(id, new Item(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::updateArc(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                              const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, origin, angle, suffix, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::updateCurve(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, origin, angle, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::updateCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                            const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, origin, angle, suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

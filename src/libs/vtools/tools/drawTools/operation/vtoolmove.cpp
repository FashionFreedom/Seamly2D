/***************************************************************************
 **  @file   vtoolmove.cpp
 **  @author Douglas S Caskey
 **  @date   18 Nov, 2023
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
 **  @file   vtoolmove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 10, 2016
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

#include "vtoolmove.h"

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
#include "../../../dialogs/tools/dialogmove.h"
#include "../../../visualization/line/operation/vistoolmove.h"
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
#include "../ifc/xml/vabstractpattern.h"
#include "../vwidgets/vabstractsimple.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../../vdatatool.h"
#include "../vdrawtool.h"
#include "../vmisc/logging.h"

template <class T> class QSharedPointer;

const QString VToolMove::ToolType = QStringLiteral("moving");

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
QPointF findRotationOrigin(const QVector<SourceItem> objects, const VContainer *data, qreal calcLength, qreal calcAngle)
{
    QPolygonF originObjects;
    qCDebug(vTool, "Find center of objects: ");
    for (auto item : objects)
    {
        qCDebug(vTool, "Object:  %d", item.id);
        const QSharedPointer<VGObject> object = data->GetGObject(item.id);

        Q_STATIC_ASSERT_X(static_cast<int>(GOType::AllCurves) == 10, "Not all objects were handled.");

        switch(static_cast<GOType>(object->getType()))
        {
            case GOType::Point:
                originObjects.append(data->GeometricObject<VPointF>(item.id)->toQPointF());
                break;
            case GOType::Arc:
            case GOType::EllipticalArc:
            case GOType::Spline:
            case GOType::SplinePath:
            case GOType::CubicBezier:
            case GOType::CubicBezierPath:
                originObjects.append(data->GeometricObject<VAbstractCurve>(item.id)->getPoints());
                break;
            case GOType::Unknown:
            case GOType::Curve:
            case GOType::Path:
            case GOType::AllCurves:
                default:
                Q_UNREACHABLE();
                break;
        }
    }

    QPointF rotationOrigin = originObjects.boundingRect().center();
    QLineF move(rotationOrigin, QPointF(rotationOrigin.x() + calcLength, rotationOrigin.y()));
    move.setAngle(calcAngle);
    return move.p2();}
QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
VToolMove::VToolMove(VAbstractPattern *doc, VContainer *data, quint32 id,
                         const QString &formulaAngle, const QString &formulaLength, const QString &formulaRotation,
                         const quint32 &originPointId, const QString &suffix, const QVector<SourceItem> &source,
                         const QVector<DestinationItem> &destination,
                         const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractOperation(doc, data, id, suffix, source, destination, parent)
    , formulaAngle(formulaAngle)
    , formulaLength(formulaLength)
    , formulaRotation(formulaRotation)
    , m_originPointId(originPointId)
{
    InitOperatedObjects();
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::setDialog()
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogMove> dialogTool = m_dialog.objectCast<DialogMove>();
    SCASSERT(!dialogTool.isNull())
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetLength(formulaLength);
    dialogTool->setRotation(formulaRotation);
    dialogTool->setSuffix(suffix);
    dialogTool->setOriginPointId(m_originPointId);
    dialogTool->setSourceObjects(source);
}

//---------------------------------------------------------------------------------------------------------------------
VToolMove *VToolMove::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                             VContainer *data)
{
    SCASSERT(!dialog.isNull())

    QSharedPointer<DialogMove> dialogTool = dialog.objectCast<DialogMove>();
    SCASSERT(!dialogTool.isNull())
    QString angle                    = dialogTool->GetAngle();
    QString length                   = dialogTool->GetLength();
    QString rotation                 = dialogTool->getRotation();
    quint32 originPointId            = dialogTool->getOriginPointId();
    const QString suffix             = dialogTool->getSuffix();
    const QVector<SourceItem> source = dialogTool->getSourceObjects();

    VToolMove* operation = Create(0, angle, length, rotation, originPointId, suffix, source,
                                  QVector<DestinationItem>(), scene, doc, data, Document::FullParse, Source::FromGui);

    if (operation != nullptr)
    {
        operation->m_dialog = dialogTool;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
VToolMove *VToolMove::Create(quint32 _id, QString &formulaAngle, QString &formulaLength, QString &formulaRotation,
                             quint32 &originPointId, const QString &suffix, const QVector<SourceItem> &source,
                             const QVector<DestinationItem> &destination, VMainGraphicsScene *scene,
                             VAbstractPattern *doc, VContainer *data, const Document &parse, const Source &typeCreation)
{
    qreal calcAngle    = 0;
    qreal calcLength   = 0;
    qreal calcRotation = 0;

    calcAngle    = CheckFormula(_id, formulaAngle, data);
    calcLength   = qApp->toPixel(CheckFormula(_id, formulaLength, data));
    calcRotation = CheckFormula(_id, formulaRotation, data);

    QPointF rotationOrigin;
    QSharedPointer<VPointF> originPoint;

    if (originPointId == NULL_ID)
    {
        rotationOrigin = findRotationOrigin(source, data, calcLength, calcAngle);
    }
    else
    {
        originPoint = data->GeometricObject<VPointF>(originPointId);
        rotationOrigin = static_cast<QPointF>(*originPoint);
        QLineF moveLine(rotationOrigin, QPointF(rotationOrigin.x() + calcLength, rotationOrigin.y()));
        moveLine.setAngle(calcAngle);
        rotationOrigin = moveLine.p2();
    }

    QVector<DestinationItem> dest = destination;

    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        dest.clear();// Try to avoid mistake, value must be empty

        id = VContainer::getNextId();//Just reserve id for tool

        qCDebug(vTool, "Create SourceItem GUI");
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            qCDebug(vTool, "Add Object:  %d", item.id);
            const QSharedPointer<VGObject> object = data->GetGObject(item.id);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::AllCurves) == 10, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
                    qCDebug(vTool, "VToolMove - Rotation Origin");
                    qCDebug(vTool, "X:  %f", rotationOrigin.x());
                    qCDebug(vTool, "Y:  %f", rotationOrigin.y());
            switch(static_cast<GOType>(object->getType()))
            {
                case GOType::Point:
                    qCDebug(vTool, "VToolMove - Point");
                    qCDebug(vTool, "length:  %f", calcLength);
                    qCDebug(vTool, "angle:  %f", calcAngle);
                    qCDebug(vTool, "rotation:  %f\n", calcRotation);
                    dest.append(createPoint(id, item.id, calcAngle, calcLength, calcRotation,
                                            rotationOrigin, suffix, data));
                    break;
                case GOType::Arc:
                    dest.append(createArc<VArc>(id, item.id, calcAngle, calcLength, calcRotation,
                                                rotationOrigin, suffix, data));
                    break;
                case GOType::EllipticalArc:
                    dest.append(createArc<VEllipticalArc>(id, item.id, calcAngle, calcLength, calcRotation,
                                                          rotationOrigin, suffix, data));
                    break;
                case GOType::Spline:
                    qCDebug(vTool, "VToolMove - Spline");
                    qCDebug(vTool, "length:  %f", calcLength);
                    qCDebug(vTool, "angle:  %f", calcAngle);
                    qCDebug(vTool, "rotation:  %f\n", calcRotation);
                    dest.append(createCurve<VSpline>(id, item.id, calcAngle, calcLength, calcRotation,
                                                     rotationOrigin, suffix, data));
                    break;
                case GOType::SplinePath:
                    dest.append(createCurveWithSegments<VSplinePath>(id, item.id, calcAngle, calcLength,
                                                                     calcRotation, rotationOrigin, suffix,
                                                                     data));
                    break;
                case GOType::CubicBezier:
                    dest.append(createCurve<VCubicBezier>(id, item.id, calcAngle, calcLength, calcRotation,
                                                          rotationOrigin, suffix, data));
                    break;
                case GOType::CubicBezierPath:
                    qCDebug(vTool, "VToolMove - VCubicBezier");
                    qCDebug(vTool, "length:  %f", calcLength);
                    qCDebug(vTool, "angle:  %f", calcAngle);
                    qCDebug(vTool, "rotation:  %f\n", calcRotation);
                    dest.append(createCurveWithSegments<VCubicBezierPath>(id, item.id, calcAngle, calcLength,
                                                                         calcRotation, rotationOrigin, suffix,
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

        qCDebug(vTool, "Create sourceItem\n");
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            qCDebug(vTool, "Add Object:  %d\n", item.id);
            const QSharedPointer<VGObject> object = data->GetGObject(item.id);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::AllCurves) == 10, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
            switch(static_cast<GOType>(object->getType()))
            {
                case GOType::Point:
                {
                    updatePoint(id, item.id, calcAngle, calcLength, calcRotation, rotationOrigin,
                                suffix, data, dest.at(i));
                    break;
                }
                case GOType::Arc:
                    updateArc<VArc>(id, item.id, calcAngle, calcLength, calcRotation, rotationOrigin,
                                    suffix, data, dest.at(i).id);
                    break;
                case GOType::EllipticalArc:
                    updateArc<VEllipticalArc>(id, item.id, calcAngle, calcLength, calcRotation, rotationOrigin,
                                              suffix, data, dest.at(i).id);
                    break;
                case GOType::Spline:
                    updateCurve<VSpline>(id, item.id, calcAngle, calcLength, calcRotation, rotationOrigin,
                                         suffix, data, dest.at(i).id);
                    break;
                case GOType::SplinePath:
                    updateCurveWithSegments<VSplinePath>(id, item.id, calcAngle, calcLength, calcRotation,
                                                         rotationOrigin, suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezier:
                    updateCurve<VCubicBezier>(id, item.id, calcAngle, calcLength, calcRotation, rotationOrigin,
                                              suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezierPath:
                    updateCurveWithSegments<VCubicBezierPath>(id, item.id, calcAngle, calcLength, calcRotation,
                                                              rotationOrigin, suffix, data, dest.at(i).id);
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
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Move, doc);
        VToolMove *tool = new VToolMove(doc, data, id, formulaAngle, formulaLength, formulaRotation, originPointId,
                                        suffix, source, dest, typeCreation);
        scene->addItem(tool);
        initOperationToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);

        if (!originPoint.isNull())
        {
            doc->IncrementReferens(originPoint->getIdTool());
        }
        qCDebug(vTool, "Increment references");
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            qCDebug(vTool, "Object:  %d", item.id);
            doc->IncrementReferens(data->GetGObject(item.id)->getIdTool());
        }
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolMove::GetFormulaAngle() const
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolMove::GetFormulaLength() const
{
    VFormula fLength(formulaLength, getData());
    fLength.setCheckZero(true);
    fLength.setToolId(m_id);
    fLength.setPostfix(UnitsToStr(qApp->patternUnit()));
    fLength.Eval();
    return fLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetFormulaLength(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaLength = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolMove::getFormulaRotation() const
{
    VFormula fRotation(formulaRotation, getData());
    fRotation.setCheckZero(false);
    fRotation.setToolId(m_id);
    fRotation.setPostfix(degreeSymbol);
    fRotation.Eval();
    return fRotation;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::setFormulaRotation(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaRotation = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMove::getOriginPointName() const
{
    if (m_originPointId == NULL_ID)
    {
        return tr("Center point");
    }
    else
    {
        return VAbstractTool::data.GetGObject(m_originPointId)->name();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolMove::getOriginPointId() const
{
    return m_originPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::setOriginPointId(const quint32 &value)
{
    m_originPointId = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetFakeGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolMove>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetVisualization()
{
    if (!vis.isNull())
    {
        VisToolMove *visual = qobject_cast<VisToolMove *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObjects(sourceToObjects(source));
        visual->SetAngle(qApp->translateVariables()->FormulaToUser(formulaAngle, qApp->Settings()->getOsSeparator()));
        visual->SetLength(qApp->translateVariables()->FormulaToUser(formulaLength, qApp->Settings()->getOsSeparator()));
        visual->setRotation(qApp->translateVariables()->FormulaToUser(formulaRotation, qApp->Settings()->getOsSeparator()));
        visual->setOriginPointId(m_originPointId);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SaveDialog(QDomElement &domElement)
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogMove> dialogTool = m_dialog.objectCast<DialogMove>();
    SCASSERT(!dialogTool.isNull())

    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    QString length = dialogTool->GetLength();
    doc->SetAttribute(domElement, AttrLength, length);
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->getSuffix());
    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->getOriginPointId()));
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->getRotation());

    source = dialogTool->getSourceObjects();
    SaveSourceDestination(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::ReadToolAttributes(const QDomElement &domElement)
{
    VAbstractOperation::ReadToolAttributes(domElement);

    formulaAngle      = doc->GetParametrString(domElement, AttrAngle, "0");
    formulaLength     = doc->GetParametrString(domElement, AttrLength, "0");
    formulaRotation   = doc->GetParametrString(domElement, AttrRotationAngle, "0");
    suffix            = doc->GetParametrString(domElement, AttrSuffix);
    m_originPointId   = doc->GetParametrUInt(domElement,   AttrCenter, NULL_ID_STR);

}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractOperation::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType,     ToolType);
    doc->SetAttribute(tag, AttrAngle,    formulaAngle);
    doc->SetAttribute(tag, AttrLength,   formulaLength);
    doc->SetAttribute(tag, AttrRotationAngle, formulaRotation);
    doc->SetAttribute(tag, AttrSuffix,   suffix);
    doc->SetAttribute(tag, AttrCenter,   QString().setNum(m_originPointId));
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolMove::makeToolTip() const
{
    const QString toolTipStr = QString("<tr> <td><b>%1:</b> %2°</td> </tr>"
                                       "<tr> <td><b>%3:</b> %4 %5</td> </tr>"
                                       "<tr> <td><b>%6:</b> %7</td> </tr>"
                                       "<tr> <td><b>%8:</b> %9°</td> </tr>")
                                       .arg(tr("Angle"))                                //1
                                       .arg(GetFormulaAngle().getDoubleValue())         //2
                                       .arg(tr("Length"))                               //3
                                       .arg(GetFormulaLength().getDoubleValue())        //4
                                       .arg(UnitsToStr(qApp->patternUnit(), true))      //5
                                       .arg(tr("Rotation point"))                       //6
                                       .arg(getOriginPointName())                       //7
                                       .arg(tr("Rotation angle"))                       //8
                                       .arg(getFormulaRotation().getDoubleValue());     //9

    return toolTipStr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogMove>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
DestinationItem VToolMove::createPoint(quint32 idTool, quint32 idItem, qreal angle,
                                         qreal length, qreal rotation, const QPointF &rotationOrigin,
                                         const QString &suffix, VContainer *data)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF moved = point->Move(length, angle, suffix).Rotate(rotationOrigin, rotation);
    moved.setIdObject(idTool);

    DestinationItem item;
    item.mx = moved.mx();
    item.my = moved.my();
    item.showPointName = moved.isShowPointName();
    item.id = data->AddGObject(new VPointF(moved));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolMove::createItem(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                                      const QPointF &rotationOrigin, const QString &suffix, VContainer *data)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item moved = i->Move(length, angle, suffix).Rotate(rotationOrigin, rotation);
    moved.setIdObject(idTool);

    DestinationItem item;
    item.mx = INT_MAX;
    item.my = INT_MAX;
    item.id = data->AddGObject(new Item(moved));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolMove::createArc(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                                     const QPointF &rotationOrigin, const QString &suffix,
                                     VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, angle, length,  rotation, rotationOrigin,
                                                  suffix, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolMove::createCurve(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                                       const QPointF &rotationOrigin, const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, angle, length,  rotation, rotationOrigin,
                                                  suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VToolMove::createCurveWithSegments(quint32 idTool, quint32 idItem, qreal angle,
                                                   qreal length, qreal rotation, const QPointF &rotationOrigin,
                                                   const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, angle, length,  rotation, rotationOrigin,
                                                  suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::updatePoint(quint32 idTool, quint32 idItem, qreal angle, qreal length,  qreal rotation,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data, const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF moved = point->Move(length, angle, suffix).Rotate(rotationOrigin, rotation);
    moved.setIdObject(idTool);
    moved.setMx(item.mx);
    moved.setMy(item.my);
    moved.setShowPointName(item.showPointName);
    data->UpdateGObject(item.id, new VPointF(moved));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::updateItem(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                           const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item moved = i->Move(length, angle, suffix).Rotate(rotationOrigin, rotation);
    moved.setIdObject(idTool);
    data->UpdateGObject(id, new Item(moved));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::updateArc(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, angle, length, rotation, rotationOrigin, suffix, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::updateCurve(quint32 idTool, quint32 idItem, qreal angle, qreal length, qreal rotation,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, angle, length, rotation, rotationOrigin, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::updateCurveWithSegments(quint32 idTool, quint32 idItem, qreal angle, qreal length,
                                        qreal rotation, const QPointF &rotationOrigin,
                                        const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, angle, length, rotation, rotationOrigin,  suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

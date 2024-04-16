/***************************************************************************
 **  @file   vtoolbisector.cpp
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
 **  @file   vtoolbisector.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
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

#include "vtoolbisector.h"

#include <QLineF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../../dialogs/tools/dialogbisector.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/line/vistoolbisector.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

const QString VToolBisector::ToolType = QStringLiteral("bisector");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolBisector constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param formula string with formula length of bisector.
 * @param firstPointId id first point of angle.
 * @param secondPointId id second point of angle.
 * @param thirdPointId id third point of angle.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolBisector::VToolBisector(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                             const QString &lineType, const QString &lineWeight,
                             const QString &lineColor, const QString &formula, const quint32 &firstPointId,
                             const quint32 &secondPointId, const quint32 &thirdPointId, const Source &typeCreation,
                             QGraphicsItem *parent)
    : VToolLinePoint(doc, data, id, lineType, lineWeight, lineColor, formula, secondPointId, 0, parent)
    , firstPointId(firstPointId)
    , thirdPointId(thirdPointId)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VToolBisector::BisectorAngle(const QPointF &firstPoint, const QPointF &secondPoint, const QPointF &thirdPoint)
{
    QLineF line1(secondPoint, firstPoint);
    QLineF line2(secondPoint, thirdPoint);
    qreal angle = line1.angleTo(line2);
    if (angle>180)
    {
        angle = 360 - angle;
        return line1.angle()-angle/2;
    }
    else
    {
        return line1.angle()+angle/2;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint find bisector point.
 * @param firstPoint first point of angle.
 * @param secondPoint second point of angle.
 * @param thirdPoint third point of angle.
 * @param length bisector length.
 * @return bisector point.
 */
QPointF VToolBisector::FindPoint(const QPointF &firstPoint, const QPointF &secondPoint,
                                 const QPointF &thirdPoint, const qreal &length)
{
    QLineF line1(secondPoint, firstPoint);
    line1.setAngle(BisectorAngle(firstPoint, secondPoint, thirdPoint));
    line1.setLength(length);
    return line1.p2();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolBisector::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogBisector> dialogTool = m_dialog.objectCast<DialogBisector>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetFirstPointId(firstPointId);
    dialogTool->SetSecondPointId(basePointId);
    dialogTool->SetThirdPointId(thirdPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
VToolBisector* VToolBisector::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogBisector> dialogTool = dialog.objectCast<DialogBisector>();
    SCASSERT(not dialogTool.isNull())
    QString formula = dialogTool->GetFormula();
    const quint32 firstPointId  = dialogTool->GetFirstPointId();
    const quint32 secondPointId = dialogTool->GetSecondPointId();
    const quint32 thirdPointId  = dialogTool->GetThirdPointId();
    const QString lineType      = dialogTool->getLineType();
    const QString lineWeight    = dialogTool->getLineWeight();
    const QString lineColor     = dialogTool->getLineColor();
    const QString pointName     = dialogTool->getPointName();
    VToolBisector *point = Create(0, formula, firstPointId, secondPointId, thirdPointId, lineType, lineWeight, lineColor,
                                  pointName, 5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param formula string with formula.
 * @param firstPointId id first point of angle.
 * @param secondPointId id second point of angle.
 * @param thirdPointId id third point of angle.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param pointName point name.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param showPointName show/hide point name text.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolBisector* VToolBisector::Create(const quint32 _id, QString &formula, quint32 firstPointId, quint32 secondPointId,
                                     quint32 thirdPointId, const QString &lineType,
                                     const QString &lineWeight, const QString &lineColor,
                                     const QString &pointName, qreal mx, qreal my, bool showPointName,
                                     VMainGraphicsScene *scene, VAbstractPattern *doc,
                                     VContainer *data, const Document &parse, const Source &typeCreation)
{
    const QSharedPointer<VPointF> firstPoint = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = data->GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> thirdPoint = data->GeometricObject<VPointF>(thirdPointId);

    const qreal result = CheckFormula(_id, formula, data);

    QPointF fPoint = VToolBisector::FindPoint(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint),
                                              static_cast<QPointF>(*thirdPoint), qApp->toPixel(result));
    quint32 id = _id;
    VPointF *p = new VPointF(fPoint, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(secondPointId, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(secondPointId, id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Bisector, doc);
        VToolBisector *point = new VToolBisector(doc, data, id, lineType, lineWeight, lineColor, formula, firstPointId,
                                                 secondPointId, thirdPointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstPoint->getIdTool());
        doc->IncrementReferens(secondPoint->getIdTool());
        doc->IncrementReferens(thirdPoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolBisector::FirstPointName() const
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolBisector::ThirdPointName() const
{
    return VAbstractTool::data.GetGObject(thirdPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolBisector::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogBisector>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolBisector::RemoveReferens()
{
    const auto firstPoint = VAbstractTool::data.GetGObject(firstPointId);
    const auto thirdPoint = VAbstractTool::data.GetGObject(thirdPointId);

    doc->DecrementReferens(firstPoint->getIdTool());
    doc->DecrementReferens(thirdPoint->getIdTool());
    VToolLinePoint::RemoveReferens();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolBisector::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogBisector> dialogTool = m_dialog.objectCast<DialogBisector>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,       dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,   dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,  dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLength,     dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPointId()));
    doc->SetAttribute(domElement, AttrThirdPoint, QString().setNum(dialogTool->GetThirdPointId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrFirstPoint, firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, basePointId);
    doc->SetAttribute(tag, AttrThirdPoint, thirdPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType    = doc->GetParametrString(domElement, AttrLineType, LineTypeSolidLine);
    m_lineWeight  = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor     = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    formulaLength = doc->GetParametrString(domElement, AttrLength, "");
    firstPointId  = doc->GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    basePointId   = doc->GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
    thirdPointId  = doc->GetParametrUInt(domElement, AttrThirdPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolBisector *visual = qobject_cast<VisToolBisector *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstPointId);
        visual->setObject2Id(basePointId);
        visual->setObject3Id(thirdPointId);
        visual->setLength(qApp->translateVariables()->FormulaToUser(formulaLength, qApp->Settings()->getOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolBisector::GetThirdPointId() const
{
    return thirdPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::SetThirdPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        thirdPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolBisector>(show);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolBisector::GetFirstPointId() const
{
    return firstPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBisector::SetFirstPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

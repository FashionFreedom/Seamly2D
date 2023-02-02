/**************************************************************************
 **
 **  @file   point_intersectxy_tool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @author Douglas S Caskey
 **  @date   7.21.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#include "point_intersectxy_tool.h"

#include "toollinepoint/doubleline_point_tool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vdrawtool.h"
#include "../../../vabstracttool.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../dialogs/tools/point_intersectxy_dialog.h"
#include "../../../../visualization/visualization.h"
#include "../../../../visualization/line/point_intersectxy_visual.h"

#include <QPointF>
#include <QSharedPointer>
#include <QString>
#include <new>

template <class T> class QSharedPointer;

const QString PointIntersectXYTool::ToolType = QStringLiteral("intersectXY");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PointIntersectXYTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
PointIntersectXYTool::PointIntersectXYTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                   const QString &lineType, const QString &lineWeight,
                                                   const QString &lineColor,
                                                   const quint32 &firstPointId, const quint32 &secondPointId,
                                                   const Source &typeCreation, QGraphicsItem *parent)
    : DoubleLinePointTool(doc, data, id, lineType, lineWeight, lineColor, firstPointId, secondPointId, parent)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void PointIntersectXYTool::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<PointIntersectXYDialog> dialogTool = m_dialog.objectCast<PointIntersectXYDialog>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setPointName(point->name());
    dialogTool->setFirstPointId(firstPointId);
    dialogTool->setSecondPointId(secondPointId);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
PointIntersectXYTool *PointIntersectXYTool::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                                           VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<PointIntersectXYDialog> dialogTool = dialog.objectCast<PointIntersectXYDialog>();
    SCASSERT(not dialogTool.isNull())
    const QString pointName     = dialogTool->getPointName();
    const quint32 firstPointId  = dialogTool->getFirstPointId();
    const quint32 secondPointId = dialogTool->getSecondPointId();
    const QString lineType      = dialogTool->getLineType();
    const QString lineWeight    = dialogTool->getLineWeight();
    const QString lineColor     = dialogTool->getLineColor();
    PointIntersectXYTool *point = Create(0, pointName, lineType, lineWeight, lineColor, firstPointId, secondPointId,
                                         5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
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
 * @param pointName point name.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 * @return the created tool
 */
PointIntersectXYTool *PointIntersectXYTool::Create(const quint32 _id, const QString &pointName,
                                                           const QString &lineType, const QString &lineWeight,
                                                           const QString &lineColor,
                                                           const quint32 &firstPointId, const quint32 &secondPointId,
                                                           qreal mx, qreal my, bool showPointName,
                                                           VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                           VContainer *data, const Document &parse,
                                                           const Source &typeCreation)
{
    const QSharedPointer<VPointF> firstPoint = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = data->GeometricObject<VPointF>(secondPointId);

    QPointF point(firstPoint->x(), secondPoint->y());
    quint32 id = _id;
    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(firstPointId, id);
        data->AddLine(secondPointId, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(firstPointId, id);
        data->AddLine(secondPointId, id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointOfIntersection, doc);
        PointIntersectXYTool *point = new PointIntersectXYTool(doc, data, id, lineType, lineWeight,lineColor,
                                                               firstPointId, secondPointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(firstPoint->getIdTool());
        doc->IncrementReferens(secondPoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString PointIntersectXYTool::firstPointName() const
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString PointIntersectXYTool::secondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void PointIntersectXYTool::RemoveReferens()
{
    const auto firstPoint = VAbstractTool::data.GetGObject(firstPointId);
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);

    doc->DecrementReferens(firstPoint->getIdTool());
    doc->DecrementReferens(secondPoint->getIdTool());
    DoubleLinePointTool::RemoveReferens();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void PointIntersectXYTool::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<PointIntersectXYDialog>(event, id);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void PointIntersectXYTool::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<PointIntersectXYDialog> dialogTool = m_dialog.objectCast<PointIntersectXYDialog>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,        dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,    dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight,  dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,   dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrFirstPoint,  dialogTool->getFirstPointId());
    doc->SetAttribute(domElement, AttrSecondPoint, dialogTool->getSecondPointId());
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    DoubleLinePointTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType,        ToolType);
    doc->SetAttribute(tag, AttrFirstPoint,  firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYTool::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType    = doc->GetParametrString(domElement, AttrLineType,    LineTypeDashLine);
    m_lineWeight  = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor     = doc->GetParametrString(domElement, AttrLineColor,   ColorBlack);
    firstPointId  = doc->GetParametrUInt(domElement,   AttrFirstPoint,  NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement,   AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYTool::SetVisualization()
{
    if (not vis.isNull())
    {
        PointIntersectXYVisual *visual = qobject_cast<PointIntersectXYVisual *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstPointId);
        visual->setPoint2Id(secondPointId);
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYTool::ShowVisualization(bool show)
{
    ShowToolVisualization<PointIntersectXYVisual>(show);
}

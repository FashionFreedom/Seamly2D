//---------------------------------------------------------------------------------------------------------------------
//  @file   intersect_arc_line_tool.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vtoolpointofcontact.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "intersect_arc_line_tool.h"

#include <QLineF>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QString>
#include <QtDebug>
#include <new>

#include "../../../../dialogs/tools/intersect_arc_line_dialog.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/visualization.h"
#include "../../../../visualization/line/intersect_arc_line_visual.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString IntersectArcLineTool::ToolType = QStringLiteral("pointOfContact");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IntersectArcLineTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param radius string with formula radius arc.
 * @param center id center arc point.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
IntersectArcLineTool::IntersectArcLineTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                         const QString &radius, const quint32 &center,
                                         const quint32 &firstPointId, const quint32 &secondPointId,
                                         const Source &typeCreation, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , arcRadius(radius)
    , center(center)
    , firstPointId(firstPointId)
    , secondPointId(secondPointId)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void IntersectArcLineTool::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectArcLineDialog> dialogTool = m_dialog.objectCast<IntersectArcLineDialog>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setRadius(arcRadius);
    dialogTool->setCenter(center);
    dialogTool->SetFirstPoint(firstPointId);
    dialogTool->SetSecondPoint(secondPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint return point intersection arc and line.
 * @param radius string with formula arc radius.
 * @param center center arc point.
 * @param firstPoint first line point.
 * @param secondPoint second line point.
 * @return point intersection.
 */
QPointF IntersectArcLineTool::FindPoint(const qreal &radius, const QPointF &center, const QPointF &firstPoint,
                                       const QPointF &secondPoint)
{
    QPointF p1, p2;
    qint32 res = VGObject::LineIntersectCircle(center, radius, QLineF(firstPoint, secondPoint), p1, p2);
    switch (res)
    {
        case 0:
            return QPointF();
        case 1:
            return p1;
        case 2:
        {
            const bool flagP1 = VGObject::IsPointOnLineSegment (p1, firstPoint, secondPoint);
            const bool flagP2 = VGObject::IsPointOnLineSegment (p2, firstPoint, secondPoint);
            if ((flagP1 == true && flagP2 == true) ||
                (flagP1 == false && flagP2 == false)/*In case we have something wrong*/)
            {
                // We don't have options for choosing correct point. Use closest to segment first point.
                if (QLineF(firstPoint, p1).length() <= QLineF(firstPoint, p2).length())
                {
                    return p1;
                }
                else
                {
                    return p2;
                }
            }
            else
            { // In this case we have one real and one theoretical intersection.
                if (flagP1)
                {
                    return p1;
                }
                else
                {
                    return p2;
                }
            }
        }
        default:
            qWarning() << "Unxpected value" << res;
            break;
    }
    return QPointF();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
IntersectArcLineTool* IntersectArcLineTool::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                                 VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<IntersectArcLineDialog> dialogTool = dialog.objectCast<IntersectArcLineDialog>();
    SCASSERT(not dialogTool.isNull())
    QString radius = dialogTool->getRadius();
    const quint32 center = dialogTool->getCenter();
    const quint32 firstPointId = dialogTool->GetFirstPoint();
    const quint32 secondPointId = dialogTool->GetSecondPoint();
    const QString pointName = dialogTool->getPointName();
    IntersectArcLineTool *point = Create(0, radius, center, firstPointId, secondPointId, pointName, 5, 10, true, scene, doc,
                                        data, Document::FullParse, Source::FromGui);
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
 * @param radius string with formula radius arc.
 * @param center id center arc point.
 * @param firstPointId id first line point.
 * @param secondPointId id second line point.
 * @param pointName point name.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
IntersectArcLineTool* IntersectArcLineTool::Create(const quint32 _id, QString &radius, const quint32 &center,
                                                 const quint32 &firstPointId, const quint32 &secondPointId,
                                                 const QString &pointName, qreal mx, qreal my, bool showPointName,
                                                 VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                                 const Document &parse, const Source &typeCreation)
{
    const QSharedPointer<VPointF> centerP = data->GeometricObject<VPointF>(center);
    const QSharedPointer<VPointF> firstP = data->GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondP = data->GeometricObject<VPointF>(secondPointId);

    const qreal result = CheckFormula(_id, radius, data);

    QPointF fPoint = IntersectArcLineTool::FindPoint(qApp->toPixel(result), static_cast<QPointF>(*centerP),
                                                    static_cast<QPointF>(*firstP), static_cast<QPointF>(*secondP));
    quint32 id =  _id;

    VPointF *p = new VPointF(fPoint, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(firstPointId, id);
        data->AddLine(secondPointId, id);
        data->AddLine(center, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(firstPointId, id);
        data->AddLine(secondPointId, id);
        data->AddLine(center, id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::IntersectArcLine, doc);
        IntersectArcLineTool *point = new IntersectArcLineTool(doc, data, id, radius, center,
                                                             firstPointId, secondPointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(centerP->getIdTool());
        doc->IncrementReferens(firstP->getIdTool());
        doc->IncrementReferens(secondP->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectArcLineTool::ArcCenterPointName() const
{
    return VAbstractTool::data.GetGObject(center)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectArcLineTool::FirstPointName() const
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectArcLineTool::SecondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void IntersectArcLineTool::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<IntersectArcLineDialog>(event, id);
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
void IntersectArcLineTool::RemoveReferens()
{
    const auto c = VAbstractTool::data.GetGObject(center);
    const auto firstPoint = VAbstractTool::data.GetGObject(firstPointId);
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);

    doc->DecrementReferens(c->getIdTool());
    doc->DecrementReferens(firstPoint->getIdTool());
    doc->DecrementReferens(secondPoint->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void IntersectArcLineTool::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectArcLineDialog> dialogTool = m_dialog.objectCast<IntersectArcLineDialog>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrRadius, dialogTool->getRadius());
    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->getCenter()));
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPoint()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPoint()));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrRadius, arcRadius);
    doc->SetAttribute(tag, AttrCenter, center);
    doc->SetAttribute(tag, AttrFirstPoint, firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::ReadToolAttributes(const QDomElement &domElement)
{
    arcRadius = doc->GetParametrString(domElement, AttrRadius, "");
    center = doc->GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    firstPointId = doc->GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::SetVisualization()
{
    if (not vis.isNull())
    {
        IntersectArcLineVisual *visual = qobject_cast<IntersectArcLineVisual *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstPointId);
        visual->setLineP2Id(secondPointId);
        visual->setRadiusId(center);
        visual->setRadius(qApp->translateVariables()->FormulaToUser(arcRadius, qApp->Settings()->getOsSeparator()));
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectArcLineTool::makeToolTip() const
{
    const QSharedPointer<VPointF> p1 = VAbstractTool::data.GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> p2 = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> centerP = VAbstractTool::data.GeometricObject<VPointF>(center);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF p1ToCur(static_cast<QPointF>(*p1), static_cast<QPointF>(*current));
    const QLineF p2ToCur(static_cast<QPointF>(*p2), static_cast<QPointF>(*current));
    const QLineF centerToCur(static_cast<QPointF>(*centerP), static_cast<QPointF>(*current));

    const QString toolTip = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr> <td><b>%10:</b> %11</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                                    "<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                                    "<tr> <td><b>%8:</b> %9°</td> </tr>"
                                    "</table>")
                                    .arg(QString("%1->%2").arg(p1->name(), current->name()))
                                    .arg(qApp->fromPixel(p1ToCur.length()))
                                    .arg(UnitsToStr(qApp->patternUnit(), true))
                                    .arg(QString("%1->%2").arg(p2->name(), current->name()))
                                    .arg(qApp->fromPixel(p2ToCur.length()))
                                    .arg(QString("%1 %2->%3").arg(tr("Length"), centerP->name(), current->name()))
                                    .arg(qApp->fromPixel(centerToCur.length()))
                                    .arg(QString("%1 %2->%3").arg(tr("Angle"), centerP->name(), current->name()))
                                    .arg(centerToCur.angle())
                                    .arg(tr("Name"))
                                    .arg(current->name());

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectArcLineTool::GetSecondPointId() const
{
    return secondPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::SetSecondPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::ShowVisualization(bool show)
{
    ShowToolVisualization<IntersectArcLineVisual>(show);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectArcLineTool::GetFirstPointId() const
{
    return firstPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::SetFirstPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectArcLineTool::getCenter() const
{
    return center;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::setCenter(const quint32 &value)
{
    if (value != NULL_ID)
    {
        center = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula IntersectArcLineTool::getArcRadius() const
{
    VFormula radius(arcRadius, this->getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(qApp->patternUnit()));

    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectArcLineTool::setArcRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        arcRadius = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

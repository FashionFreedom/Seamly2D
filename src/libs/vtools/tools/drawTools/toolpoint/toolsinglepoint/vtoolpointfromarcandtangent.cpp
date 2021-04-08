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
 **  @file   vtoolpointfromarcandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#include "vtoolpointfromarcandtangent.h"

#include <QLineF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../dialogs/tools/dialogpointfromarcandtangent.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/visualization.h"
#include "../../../../visualization/line/vistoolpointfromarcandtangent.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolPointFromArcAndTangent::ToolType = QStringLiteral("pointFromArcAndTangent");

//---------------------------------------------------------------------------------------------------------------------
VToolPointFromArcAndTangent::VToolPointFromArcAndTangent(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                         quint32 arcId, quint32 tangentPointId,
                                                         CrossCirclesPoint crossPoint, const Source &typeCreation,
                                                         QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , arcId(arcId)
    , tangentPointId(tangentPointId)
    , crossPoint(crossPoint)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointFromArcAndTangent> dialogTool = m_dialog.objectCast<DialogPointFromArcAndTangent>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetArcId(arcId);
    dialogTool->SetCrossCirclesPoint(crossPoint);
    dialogTool->SetTangentPointId(tangentPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointFromArcAndTangent *VToolPointFromArcAndTangent::Create(QSharedPointer<DialogTool> dialog,
                                                                 VMainGraphicsScene *scene,
                                                                 VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogPointFromArcAndTangent> dialogTool = dialog.objectCast<DialogPointFromArcAndTangent>();
    SCASSERT(not dialogTool.isNull())
    const quint32 arcId = dialogTool->GetArcId();
    const quint32 tangentPointId = dialogTool->GetTangentPointId();
    const CrossCirclesPoint pType = dialogTool->GetCrossCirclesPoint();
    const QString pointName = dialogTool->getPointName();
    VToolPointFromArcAndTangent *point = Create(0, pointName, arcId, tangentPointId, pType, 5, 10, true, scene, doc,
                                                data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointFromArcAndTangent *VToolPointFromArcAndTangent::Create(const quint32 _id,
                                                                 const QString &pointName,
                                                                 quint32 arcId,
                                                                 quint32 tangentPointId,
                                                                 CrossCirclesPoint crossPoint,
                                                                 qreal mx, qreal my,
                                                                 bool showPointName,
                                                                 VMainGraphicsScene *scene,
                                                                 VAbstractPattern *doc,
                                                                 VContainer *data,
                                                                 const Document &parse,
                                                                 const Source &typeCreation)
{
    const VArc arc = *data->GeometricObject<VArc>(arcId);
    const VPointF tPoint = *data->GeometricObject<VPointF>(tangentPointId);

    const QPointF point = VToolPointFromArcAndTangent::FindPoint(static_cast<QPointF>(tPoint), &arc, crossPoint);
    quint32 id = _id;

    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
    }
    else
    {
        data->UpdateGObject(id, p);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointFromArcAndTangent, doc);
        VToolPointFromArcAndTangent *point = new VToolPointFromArcAndTangent(doc, data, id, arcId, tangentPointId,
                                                                             crossPoint, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(arc.getIdTool());
        doc->IncrementReferens(tPoint.getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VToolPointFromArcAndTangent::FindPoint(const QPointF &p, const VArc *arc, const CrossCirclesPoint pType)
{
    QPointF p1, p2;
    const QPointF center = static_cast<QPointF>(arc->GetCenter());
    const qreal radius = arc->GetRadius();
    const int res = VGObject::ContactPoints (p, center, radius, p1, p2);

    QLineF r1Arc(center, p1);
    r1Arc.setLength(radius+10);

    QLineF r2Arc(center, p2);
    r2Arc.setLength(radius+10);

    switch(res)
    {
        case 2:
        {
            int localRes = 0;
            bool flagP1 = false;

            if (arc->IsIntersectLine(r1Arc))
            {
                ++localRes;
                flagP1 = true;
            }

            if (arc->IsIntersectLine(r2Arc))
            {
                ++localRes;
            }

            switch(localRes)
            {
                case 2:
                    if (pType == CrossCirclesPoint::FirstPoint)
                    {
                        return p1;
                    }
                    else
                    {
                        return p2;
                    }
                case 1:
                    if (flagP1)
                    {
                        return p1;
                    }
                    else
                    {
                        return p2;
                    }
                case 0:
                default:
                    return QPointF();
            }

            break;
        }
        case 1:
            if (arc->IsIntersectLine(r1Arc))
            {
                return p1;
            }
            else
            {
                return QPointF();
            }
        case 3:
        case 0:
        default:
            break;
    }
    return QPointF();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointFromArcAndTangent::TangentPointName() const
{
    return VAbstractTool::data.GetGObject(tangentPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointFromArcAndTangent::ArcName() const
{
    return VAbstractTool::data.GetGObject(arcId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointFromArcAndTangent::GetTangentPointId() const
{
    return tangentPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SetTangentPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        tangentPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointFromArcAndTangent::GetArcId() const
{
    return arcId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SetArcId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        arcId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint VToolPointFromArcAndTangent::GetCrossCirclesPoint() const
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SetCrossCirclesPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointFromArcAndTangent>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::RemoveReferens()
{
    const auto arc = VAbstractTool::data.GetGObject(arcId);
    const auto tP = VAbstractTool::data.GetGObject(tangentPointId);

    doc->DecrementReferens(arc->getIdTool());
    doc->DecrementReferens(tP->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointFromArcAndTangent>(event, id);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogPointFromArcAndTangent> dialogTool = m_dialog.objectCast<DialogPointFromArcAndTangent>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrArc, QString().setNum(dialogTool->GetArcId()));
    doc->SetAttribute(domElement, AttrTangent, QString().setNum(dialogTool->GetTangentPointId()));
    doc->SetAttribute(domElement, AttrCrossPoint,
                      QString().setNum(static_cast<int>(dialogTool->GetCrossCirclesPoint())));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrArc, arcId);
    doc->SetAttribute(tag, AttrTangent, tangentPointId);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::ReadToolAttributes(const QDomElement &domElement)
{
    arcId = doc->GetParametrUInt(domElement, AttrArc, NULL_ID_STR);
    tangentPointId = doc->GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
    crossPoint = static_cast<CrossCirclesPoint>(doc->GetParametrUInt(domElement, AttrCrossPoint, "1"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromArcAndTangent::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolPointFromArcAndTangent *visual = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(tangentPointId);
        visual->setArcId(arcId);
        visual->setCrossPoint(crossPoint);
        visual->RefreshGeometry();
    }
}

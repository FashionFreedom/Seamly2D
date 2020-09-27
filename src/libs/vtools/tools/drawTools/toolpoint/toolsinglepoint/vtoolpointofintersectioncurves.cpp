/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   vtoolpointofintersectioncurves.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2016
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

#include "vtoolpointofintersectioncurves.h"

#include <QLineF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../dialogs/tools/dialogpointofintersectioncurves.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/path/../visualization.h"
#include "../../../../visualization/path/vistoolpointofintersectioncurves.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "toolcut/vtoolcutsplinepath.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolPointOfIntersectionCurves::ToolType = QStringLiteral("pointOfIntersectionCurves");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCurves::VToolPointOfIntersectionCurves(VAbstractPattern *doc, VContainer *data,
                                                               const quint32 &id, const quint32 firstCurveId,
                                                               quint32 secondCurveId, VCrossCurvesPoint vCrossPoint,
                                                               HCrossCurvesPoint hCrossPoint, const Source &typeCreation,
                                                               QGraphicsItem *parent)
    :VToolSinglePoint(doc, data, id, parent),
      firstCurveId(firstCurveId),
      secondCurveId(secondCurveId),
      vCrossPoint(vCrossPoint),
      hCrossPoint(hCrossPoint)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogPointOfIntersectionCurves*>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    auto p = VAbstractTool::data.GeometricObject<VPointF>(id);
    dialogTool->SetFirstCurveId(firstCurveId);
    dialogTool->SetSecondCurveId(secondCurveId);
    dialogTool->SetVCrossPoint(vCrossPoint);
    dialogTool->SetHCrossPoint(hCrossPoint);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCurves *VToolPointOfIntersectionCurves::Create(QSharedPointer<DialogTool> dialog,
                                                                       VMainGraphicsScene *scene,
                                                                       VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogPointOfIntersectionCurves> dialogTool = dialog.objectCast<DialogPointOfIntersectionCurves>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstCurveId = dialogTool->GetFirstCurveId();
    const quint32 secondCurveId = dialogTool->GetSecondCurveId();
    const VCrossCurvesPoint vCrossPoint = dialogTool->GetVCrossPoint();
    const HCrossCurvesPoint hCrossPoint = dialogTool->GetHCrossPoint();
    const QString pointName = dialogTool->getPointName();
    VToolPointOfIntersectionCurves *point = Create(0, pointName, firstCurveId, secondCurveId, vCrossPoint, hCrossPoint,
                                                   5, 10, scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCurves *VToolPointOfIntersectionCurves::Create(const quint32 _id, const QString &pointName,
                                                                       quint32 firstCurveId, quint32 secondCurveId,
                                                                       VCrossCurvesPoint vCrossPoint,
                                                                       HCrossCurvesPoint hCrossPoint, const qreal &mx,
                                                                       const qreal &my, VMainGraphicsScene *scene,
                                                                       VAbstractPattern *doc, VContainer *data,
                                                                       const Document &parse,
                                                                       const Source &typeCreation)
{
    auto curve1 = data->GeometricObject<VAbstractCurve>(firstCurveId);
    auto curve2 = data->GeometricObject<VAbstractCurve>(secondCurveId);

    const QPointF point = VToolPointOfIntersectionCurves::FindPoint(curve1->GetPoints(), curve2->GetPoints(),
                                                                    vCrossPoint, hCrossPoint);
    const qreal segLength1 = curve1->GetLengthByPoint(point);
    const qreal segLength2 = curve2->GetLengthByPoint(point);
    quint32 id = _id;
    VPointF *p = new VPointF(point, pointName, mx, my);
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);

        InitSegments(curve1->getType(), segLength1, p, firstCurveId, data);
        InitSegments(curve2->getType(), segLength2, p, secondCurveId, data);
    }
    else
    {
        data->UpdateGObject(id, p);

        InitSegments(curve1->getType(), segLength1, p, firstCurveId, data);
        InitSegments(curve2->getType(), segLength2, p, secondCurveId, data);

        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointOfIntersectionCurves, doc);
        auto point = new VToolPointOfIntersectionCurves(doc, data, id, firstCurveId, secondCurveId, vCrossPoint,
                                                        hCrossPoint, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(curve1->getIdTool());
        doc->IncrementReferens(curve2->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VToolPointOfIntersectionCurves::FindPoint(const QVector<QPointF> &curve1Points,
                                                  const QVector<QPointF> &curve2Points,
                                                  VCrossCurvesPoint vCrossPoint, HCrossCurvesPoint hCrossPoint)
{
    if (curve1Points.isEmpty() || curve2Points.isEmpty())
    {
        return QPointF();
    }

    QVector<QPointF> intersections;
    for ( auto i = 0; i < curve1Points.count()-1; ++i )
    {
        intersections << VAbstractCurve::CurveIntersectLine(curve2Points,
                                                            QLineF(curve1Points.at(i), curve1Points.at(i+1)));
    }

    if (intersections.isEmpty())
    {
        return QPointF();
    }

    if (intersections.size() == 1)
    {
        return intersections.at(0);
    }

    QVector<QPointF> vIntersections;
    if (vCrossPoint == VCrossCurvesPoint::HighestPoint)
    {
        qreal minY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for ( auto i = 1; i < intersections.count(); ++i )
        {
            const QPointF p = intersections.at(i);
            if (p.y() > minY)
            {
                continue;
            }
            else if (p.y() < minY)
            {
                minY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else
            {
                vIntersections.append(p);
            }
        }
    }
    else
    {
        qreal maxY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for ( auto i = 1; i < intersections.count(); ++i )
        {
            const QPointF p = intersections.at(i);
            if (p.y() > maxY)
            {
                maxY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else if (p.y() < maxY)
            {
                continue;
            }
            else
            {
                vIntersections.append(p);
            }
        }
    }

    if (vIntersections.isEmpty())
    {
        return QPointF();
    }

    if (vIntersections.size() == 1)
    {
        return vIntersections.at(0);
    }

    QPointF crossPoint = vIntersections.at(0);

    if (hCrossPoint == HCrossCurvesPoint::RightmostPoint)
    {
        qreal maxX = vIntersections.at(0).x();

        for ( auto i = 1; i < vIntersections.count(); ++i )
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() > maxX)
            {
                maxX = p.x();
                crossPoint = p;
            }
        }
    }
    else
    {
        qreal minX = vIntersections.at(0).x();

        for ( auto i = 1; i < vIntersections.count(); ++i )
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() < minX)
            {
                minX = p.x();
                crossPoint = p;
            }
        }
    }

    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersectionCurves::FirstCurveName() const
{
    return VAbstractTool::data.GetGObject(firstCurveId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolPointOfIntersectionCurves::SecondCurveName() const
{
    return VAbstractTool::data.GetGObject(secondCurveId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersectionCurves::GetFirstCurveId() const
{
    return firstCurveId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetFirstCurveId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstCurveId = value;

        auto obj = VAbstractTool::data.GetGObject(id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolPointOfIntersectionCurves::GetSecondCurveId() const
{
    return secondCurveId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetSecondCurveId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondCurveId = value;

        auto obj = VAbstractTool::data.GetGObject(id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VCrossCurvesPoint VToolPointOfIntersectionCurves::GetVCrossPoint() const
{
    return vCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVCrossPoint(const VCrossCurvesPoint &value)
{
    vCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
HCrossCurvesPoint VToolPointOfIntersectionCurves::GetHCrossPoint() const
{
    return hCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetHCrossPoint(const HCrossCurvesPoint &value)
{
    hCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionCurves>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::RemoveReferens()
{
    const auto curve1 = VAbstractTool::data.GetGObject(firstCurveId);
    const auto curve2 = VAbstractTool::data.GetGObject(secondCurveId);

    doc->DecrementReferens(curve1->getIdTool());
    doc->DecrementReferens(curve2->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionCurves>(this, event);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogPointOfIntersectionCurves*>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrCurve1, QString().setNum(dialogTool->GetFirstCurveId()));
    doc->SetAttribute(domElement, AttrCurve2, QString().setNum(dialogTool->GetSecondCurveId()));
    doc->SetAttribute(domElement, AttrVCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetVCrossPoint())));
    doc->SetAttribute(domElement, AttrHCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetHCrossPoint())));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCurve1, firstCurveId);
    doc->SetAttribute(tag, AttrCurve2, secondCurveId);
    doc->SetAttribute(tag, AttrVCrossPoint, static_cast<int>(vCrossPoint));
    doc->SetAttribute(tag, AttrHCrossPoint, static_cast<int>(hCrossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ReadToolAttributes(const QDomElement &domElement)
{
    firstCurveId = doc->GetParametrUInt(domElement, AttrCurve1, NULL_ID_STR);
    secondCurveId = doc->GetParametrUInt(domElement, AttrCurve2, NULL_ID_STR);
    vCrossPoint = static_cast<VCrossCurvesPoint>(doc->GetParametrUInt(domElement, AttrVCrossPoint, "1"));
    hCrossPoint = static_cast<HCrossCurvesPoint>(doc->GetParametrUInt(domElement, AttrHCrossPoint, "1"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVisualization()
{
    if (not vis.isNull())
    {
        auto visual = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstCurveId);
        visual->setObject2Id(secondCurveId);
        visual->setVCrossPoint(vCrossPoint);
        visual->setHCrossPoint(hCrossPoint);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolPointOfIntersectionCurves::InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId)
{
    QSharedPointer<Item> a1;
    QSharedPointer<Item> a2;

    const QSharedPointer<Item> arc = data->GeometricObject<Item>(curveId);
    Item arc1;
    Item arc2;

    if (not VFuzzyComparePossibleNulls(segLength, -1))
    {
        arc->CutArc(segLength, arc1, arc2);
    }
    else
    {
        arc->CutArc(0, arc1, arc2);
    }

    // Arc highly depend on id. Need for creating the name.
    arc1.setId(p->id() + 1);
    arc2.setId(p->id() + 2);

    if (not VFuzzyComparePossibleNulls(segLength, -1))
    {
        a1 = QSharedPointer<Item>(new Item(arc1));
        a2 = QSharedPointer<Item>(new Item(arc2));
    }
    else
    {
        a1 = QSharedPointer<Item>(new Item());
        a2 = QSharedPointer<Item>(new Item());

        // Take names for empty arcs from donors.
        a1->setName(arc1.name());
        a2->setName(arc2.name());
    }

    data->AddArc(a1, arc1.id(), p->id());
    data->AddArc(a2, arc2.id(), p->id());
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
void VToolPointOfIntersectionCurves::InitSegments(const GOType &curveType, qreal segLength, const VPointF *p, quint32 curveId,
                                                  VContainer *data)
{
    switch(curveType)
    {
        case GOType::EllipticalArc:
            InitArc<VEllipticalArc>(data, segLength, p, curveId);
            break;
        case GOType::Arc:
            InitArc<VArc>(data, segLength, p, curveId);
            break;
        case GOType::CubicBezier:
        case GOType::Spline:
        {
            QSharedPointer<VAbstractBezier> spline1;
            QSharedPointer<VAbstractBezier> spline2;

            const auto spl = data->GeometricObject<VAbstractCubicBezier>(curveId);
            QPointF spl1p2, spl1p3, spl2p2, spl2p3;
            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                spl->CutSpline(segLength, spl1p2, spl1p3, spl2p2, spl2p3);
            }
            else
            {
                spl->CutSpline(0, spl1p2, spl1p3, spl2p2, spl2p3);
            }

            VSpline *spl1 = new VSpline(spl->GetP1(), spl1p2, spl1p3, *p);
            VSpline *spl2 = new VSpline(*p, spl2p2, spl2p3, spl->GetP4());

            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                spline1 = QSharedPointer<VAbstractBezier>(spl1);
                spline2 = QSharedPointer<VAbstractBezier>(spl2);
            }
            else
            {
                spline1 = QSharedPointer<VAbstractBezier>(new VSpline());
                spline2 = QSharedPointer<VAbstractBezier>(new VSpline());

                // Take names for empty splines from donors.
                spline1->setName(spl1->name());
                spline2->setName(spl2->name());

                delete spl1;
                delete spl2;
            }

            data->AddSpline(spline1, NULL_ID, p->id());
            data->AddSpline(spline2, NULL_ID, p->id());
            break;
        }
        case GOType::CubicBezierPath:
        case GOType::SplinePath:
        {
            QSharedPointer<VAbstractBezier> splP1;
            QSharedPointer<VAbstractBezier> splP2;

            const auto splPath = data->GeometricObject<VAbstractCubicBezierPath>(curveId);
            VSplinePath *splPath1 = nullptr;
            VSplinePath *splPath2 = nullptr;
            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                VPointF *pC = VToolCutSplinePath::CutSplinePath(segLength, splPath, p->name(), &splPath1, &splPath2);
                delete pC;
            }
            else
            {
                VPointF *pC = VToolCutSplinePath::CutSplinePath(0, splPath, p->name(), &splPath1, &splPath2);
                delete pC;
            }

            SCASSERT(splPath1 != nullptr)
            SCASSERT(splPath2 != nullptr)

            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                splP1 = QSharedPointer<VAbstractBezier>(splPath1);
                splP2 = QSharedPointer<VAbstractBezier>(splPath2);
            }
            else
            {
                splP1 = QSharedPointer<VAbstractBezier>(new VSplinePath());
                splP2 = QSharedPointer<VAbstractBezier>(new VSplinePath());

                // Take names for empty spline paths from donors.
                splP1->setName(splPath1->name());
                splP2->setName(splPath2->name());

                delete splPath1;
                delete splPath2;
            }

            data->AddSpline(splP1, NULL_ID, p->id());
            data->AddSpline(splP2, NULL_ID, p->id());
            break;
        }
        case GOType::Point:
        case GOType::Unknown:
            Q_UNREACHABLE();
            break;
    }
}

QT_WARNING_POP

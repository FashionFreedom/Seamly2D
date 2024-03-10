/***************************************************************************
 **  @file   vtoolcurveintersectaxis.cpp
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
 **  @file   vtoolcurveintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Valentina project
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

#include "vtoolcurveintersectaxis.h"

#include <limits.h>
#include <QLineF>
#include <QMap>
#include <QRectF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QVector>
#include <QMessageBox>
#include <new>

#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../dialogs/tools/dialogcurveintersectaxis.h"
#include "../ifc/ifcdef.h"
#include "../ifc/exception/vexception.h"
#include "../qmuparser/qmudef.h"
#include "../toolcut/vtoolcutsplinepath.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/visualization/visualization.h"
#include "../vtools/visualization/line/vistoolcurveintersectaxis.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

const QString VToolCurveIntersectAxis::ToolType = QStringLiteral("curveIntersectAxis");

//---------------------------------------------------------------------------------------------------------------------
VToolCurveIntersectAxis::VToolCurveIntersectAxis(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                 const QString &lineType, const QString &lineWeight,
                                                 const QString &lineColor,
                                                 const QString &formulaAngle, const quint32 &basePointId,
                                                 const quint32 &curveId, const Source &typeCreation,
                                                 QGraphicsItem *parent)
    : VToolLinePoint(doc, data, id, lineType, lineWeight, lineColor, QString(), basePointId, 0, parent)
    , formulaAngle(formulaAngle)
    , curveId(curveId)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    m_dialog->setModal(true);
    QSharedPointer<DialogCurveIntersectAxis> dialogTool = m_dialog.objectCast<DialogCurveIntersectAxis>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> intersectPoint = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->setCurveId(curveId);
    dialogTool->SetPointName(intersectPoint->name());
}

//---------------------------------------------------------------------------------------------------------------------
VToolCurveIntersectAxis *VToolCurveIntersectAxis::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                                         VAbstractPattern *doc,
                                                         VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogCurveIntersectAxis> dialogTool = dialog.objectCast<DialogCurveIntersectAxis>();
    SCASSERT(not dialogTool.isNull())
    const QString pointName    = dialogTool->getPointName();
    const QString lineType     = dialogTool->getLineType();
    const QString lineWeight   = dialogTool->getLineWeight();
    const QString lineColor    = dialogTool->getLineColor();
          QString formulaAngle = dialogTool->GetAngle();
    const quint32 basePointId  = dialogTool->GetBasePointId();
    const quint32 curveId      = dialogTool->getCurveId();

    VToolCurveIntersectAxis *point = Create(0, pointName, lineType, lineWeight, lineColor, formulaAngle, basePointId,
                                            curveId, 5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCurveIntersectAxis *VToolCurveIntersectAxis::Create(const quint32 _id, const QString &pointName,
                                                         const QString &lineType, const QString &lineWeight,
                                                         const QString &lineColor,
                                                         QString &formulaAngle, quint32 basePointId,
                                                         quint32 curveId, qreal mx, qreal my, bool showPointName,
                                                         VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                         VContainer *data,
                                                         const Document &parse, const Source &typeCreation)
{
    const QSharedPointer<VPointF> basePoint = data->GeometricObject<VPointF>(basePointId);
    const qreal angle = CheckFormula(_id, formulaAngle, data);
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(curveId);

    QPointF intersectPoint;
    const bool isIntersect = FindPoint(static_cast<QPointF>(*basePoint), angle, curve, &intersectPoint);

    if (not isIntersect)
    {
        const QString msg = tr("<b><big>Can not create intersection point %1 from point %2</big></b><br>"
                               "<b><big>to curve %3 with an axis angle of %4°</big></b><br><br>"
                               "Using origin point as a place holder until pattern is corrected.")
                               .arg(pointName)
                               .arg(basePoint->name())
                               .arg(curve->name())
                               .arg(angle);
        QMessageBox msgBox(qApp->getMainWindow());
        msgBox.setWindowTitle(tr("Intersection Point of Curve & Axis"));
        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        msgBox.setWindowIcon(QIcon(":/toolicon/32x32/curve_intersect_axis.png"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    const qreal segLength = curve->GetLengthByPoint(intersectPoint);
    quint32 id = _id;
    VPointF *p = new VPointF(intersectPoint, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(basePointId, id);

        VContainer::getNextId();
        VContainer::getNextId();
        InitSegments(curve->getType(), segLength, p, curveId, data);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(basePointId, id);

        InitSegments(curve->getType(), segLength, p, curveId, data);

        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::CurveIntersectAxis, doc);
        VToolCurveIntersectAxis *point = new VToolCurveIntersectAxis(doc, data, id, lineType, lineWeight, lineColor,
                                                                     formulaAngle, basePointId, curveId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(basePoint->getIdTool());
        doc->IncrementReferens(curve->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
bool VToolCurveIntersectAxis::FindPoint(const QPointF &axisPoint, qreal angle,
                                           const QSharedPointer<VAbstractCurve> &curve, QPointF *intersectPoint)
{
    QRectF rectangle = QRectF(0, 0, INT_MAX, INT_MAX);
    rectangle.translate(-INT_MAX/2.0, -INT_MAX/2.0);

    QLineF axis = QLineF(axisPoint, VGObject::BuildRay(axisPoint, angle, rectangle));
    QVector<QPointF> points = curve->IntersectLine(axis);

    if (points.isEmpty())
    {
        QLineF axis2 = QLineF(axisPoint, VGObject::BuildRay(axisPoint, angle + 180, rectangle));
        points = curve->IntersectLine(axis2);
    }

    if (points.size() > 0)
    {
        if (points.size() == 1)
        {
            *intersectPoint = points.at(0);
            return true;
        }

        QMap<qreal, int> lengths;

        for ( qint32 i = 0; i < points.size(); ++i )
        {
            lengths.insert(QLineF(points.at(i), axisPoint).length(), i);
        }

        QMap<qreal, int>::const_iterator i = lengths.constBegin();
        if (i != lengths.constEnd())
        {
            *intersectPoint = points.at(i.value());
            return true;
        }

    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCurveIntersectAxis::GetFormulaAngle() const
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCurveIntersectAxis::CurveName() const
{
    return VAbstractTool::data.GetGObject(curveId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolCurveIntersectAxis::getCurveId() const
{
    return curveId;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::setCurveId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        curveId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCurveIntersectAxis>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCurveIntersectAxis>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogCurveIntersectAxis> dialogTool = m_dialog.objectCast<DialogCurveIntersectAxis>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,       dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,   dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,  dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrAngle,      dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrBasePoint,  QString().setNum(dialogTool->GetBasePointId()));
    doc->SetAttribute(domElement, AttrCurve,      QString().setNum(dialogTool->getCurveId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType,      ToolType);
    doc->SetAttribute(tag, AttrAngle,     formulaAngle);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
    doc->SetAttribute(tag, AttrCurve,     curveId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType   = doc->GetParametrString(domElement, AttrLineType, LineTypeSolidLine);
    m_lineWeight = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor    = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    basePointId  = doc->GetParametrUInt(domElement,   AttrBasePoint, NULL_ID_STR);
    curveId      = doc->GetParametrUInt(domElement,   AttrCurve, NULL_ID_STR);
    formulaAngle = doc->GetParametrString(domElement, AttrAngle, "");
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolCurveIntersectAxis *visual = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(curveId);
        visual->setAxisPointId(basePointId);
        visual->SetAngle(qApp->translateVariables()->FormulaToUser(formulaAngle, qApp->Settings()->getOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolCurveIntersectAxis::InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId)
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
void VToolCurveIntersectAxis::InitSegments(const GOType &curveType, qreal segLength, const VPointF *p, quint32 curveId,
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
        case GOType::Curve:
        case GOType::Path:
        case GOType::AllCurves:
        default:
            Q_UNREACHABLE();
            break;
    }
}

QT_WARNING_POP

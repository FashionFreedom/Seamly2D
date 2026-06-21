/***************************************************************************
 **  @file   vtoolcubicbezier.cpp
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
 **  @file   vtoolcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vtoolcubicbezier.h"

#include <QDomElement>
#include <QLineF>
#include <QPair>
#include <QPen>
#include <QSharedPointer>
#include <QString>
#include <QtMath>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogcubicbezier.h"
#include "../../../visualization/visualization.h"
#include "../../../visualization/path/vistoolcubicbezier.h"
#include "../ifc/exception/vexception.h"
#include "../vgeometry/../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "vabstractspline.h"

const QString VToolCubicBezier::ToolType = QStringLiteral("cubicBezier");

//---------------------------------------------------------------------------------------------------------------------
static QPair<qreal, qreal> hobbyHandleLengths(const VPointF &p1, const VPointF &p4,
                                               qreal angle1Deg, qreal angle2Deg)
{
    const QLineF chord(static_cast<QPointF>(p1), static_cast<QPointF>(p4));
    const qreal d = chord.length();

    if (qFuzzyIsNull(d))
    {
        return {0.0, 0.0};
    }

    const qreal chordRad = qDegreesToRadians(chord.angle());
    qreal theta = qDegreesToRadians(angle1Deg) - chordRad;
    qreal phi   = qDegreesToRadians(angle2Deg) - qDegreesToRadians(chord.angle() + 180.0);

    const qreal twoPi = 2.0 * M_PI;
    while (theta >  M_PI)
    {
        theta -= twoPi;
    }
    while (theta < -M_PI)
    {
        theta += twoPi;
    }
    while (phi >  M_PI)
    {
        phi -= twoPi;
    }
    while (phi < -M_PI)
    {
        phi += twoPi;
    }

    const qreal sq2   = qSqrt(2.0);
    const qreal sqrt5 = qSqrt(5.0);
    const qreal cA    = 0.5 * (sqrt5 - 1.0);
    const qreal cB    = 0.5 * (3.0 - sqrt5);

    auto velocity = [&](qreal a, qreal b) -> qreal
    {
        const qreal num = 2.0 + sq2 * (qSin(a) - qSin(b) / 16.0)
                                    * (qSin(b) - qSin(a) / 16.0)
                                    * (qCos(a) - qCos(b));
        const qreal den = 3.0 * (1.0 + cA * qCos(a) + cB * qCos(b));
        return (qAbs(den) > 1e-9) ? num / den : (1.0 / 3.0);
    };

    const qreal c1 = qBound(1e-4, velocity(theta, phi) * d, d * 8.0);
    const qreal c2 = qBound(1e-4, velocity(phi, theta) * d, d * 8.0);

    return {c1, c2};
}

//---------------------------------------------------------------------------------------------------------------------
static void applyHobbyToSpline(VCubicBezier *spline, const VContainer *data,
                               quint32 p2Id, quint32 p3Id)
{
    const VPointF p1 = spline->GetP1();
    const VPointF p4 = spline->GetP4();
    const auto p2canvas = data->GeometricObject<VPointF>(p2Id);
    const auto p3canvas = data->GeometricObject<VPointF>(p3Id);

    const QLineF h1(static_cast<QPointF>(p1), static_cast<QPointF>(*p2canvas));
    const QLineF h2(static_cast<QPointF>(p4), static_cast<QPointF>(*p3canvas));

    const QPair<qreal, qreal> hobby = hobbyHandleLengths(p1, p4, h1.angle(), h2.angle());

    QLineF newH1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(hobby.first, 0.0));
    newH1.setAngle(h1.angle());
    QLineF newH2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(hobby.second, 0.0));
    newH2.setAngle(h2.angle());

    VPointF newP2(newH1.p2(), p2canvas->name(), p2canvas->mx(), p2canvas->my(),
                  p2canvas->getIdObject(), p2canvas->getMode());
    newP2.setId(p2Id);
    VPointF newP3(newH2.p2(), p3canvas->name(), p3canvas->mx(), p3canvas->my(),
                  p3canvas->getIdObject(), p3canvas->getMode());
    newP3.setId(p3Id);

    spline->SetP2(newP2);
    spline->SetP3(newP3);
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier::VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                                   bool autoSmooth,
                                   const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , m_autoSmooth(autoSmooth)
    , m_p2Id(0)
    , m_p3Id(0)
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(id);
    m_p2Id = spl->GetP2().id();
    m_p3Id = spl->GetP3().id();
    m_sceneType = SceneObject::Spline;

    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezier*>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    dialogTool->SetSpline(*spl);
    dialogTool->SetAutoSmooth(m_autoSmooth);
    dialogTool->setLineColor(spl->getLineColor());
    dialogTool->setPenStyle(spl->GetPenStyle());
    dialogTool->setLineWeight(spl->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                           VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogCubicBezier> dialogTool = dialog.objectCast<DialogCubicBezier>();
    SCASSERT(not dialogTool.isNull())

    VCubicBezier *spline = new VCubicBezier(dialogTool->GetSpline());
    spline->setLineColor(dialogTool->getLineColor());
    spline->SetPenStyle(dialogTool->getPenStyle());
    spline->setLineWeight(dialogTool->getLineWeight());
    const bool autoSmooth = dialogTool->GetAutoSmooth();

    auto spl = Create(0, spline, autoSmooth, scene, doc, data, Document::FullParse, Source::FromGui);

    if (spl != nullptr)
    {
        spl->m_dialog = dialogTool;
    }
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(const quint32 _id, VCubicBezier *spline,
                                           bool autoSmooth,
                                           VMainGraphicsScene *scene,
                                           VAbstractPattern *doc, VContainer *data,
                                           const Document &parse, const Source &typeCreation)
{
    const quint32 origP2Id = spline->GetP2().id();
    const quint32 origP3Id = spline->GetP3().id();

    if (autoSmooth)
    {
        applyHobbyToSpline(spline, data, origP2Id, origP3Id);
    }

    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(spline);
        data->AddSpline(data->GeometricObject<VAbstractBezier>(id), id);
    }
    else
    {
        data->UpdateGObject(id, spline);
        data->AddSpline(data->GeometricObject<VAbstractBezier>(id), id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::CubicBezier, doc);
        auto _spl = new VToolCubicBezier(doc, data, id, autoSmooth, typeCreation);
        scene->addItem(_spl);
        initSplineToolConnections(scene, _spl);
        VAbstractPattern::AddTool(id, _spl);
        doc->IncrementReferens(spline->GetP1().getIdTool());
        doc->IncrementReferens(spline->GetP2().getIdTool());
        doc->IncrementReferens(spline->GetP3().getIdTool());
        doc->IncrementReferens(spline->GetP4().getIdTool());
        return _spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::FirstPointName() const
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP1().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::SecondPointName() const
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP2().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::ThirdPointName() const
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP3().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::ForthPointName() const
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP4().name();
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier VToolCubicBezier::getSpline() const
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return *spline.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::setSpline(const VCubicBezier &spl)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VCubicBezier> spline = qSharedPointerDynamicCast<VCubicBezier>(obj);
    *spline.data() = spl;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
bool VToolCubicBezier::GetAutoSmooth() const
{
    return m_autoSmooth;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetAutoSmooth(bool value)
{
    m_autoSmooth = value;
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::GetTargetLength() const
{
    return QString();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetTargetLength(const QString &value)
{
    Q_UNUSED(value)
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCubicBezier>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)

    try
    {
        ContextMenu<DialogCubicBezier>(event);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::ReadToolAttributes(const QDomElement &domElement)
{
    m_autoSmooth = (domElement.attribute(AttrAutoSmooth) == QStringLiteral("true"));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::RemoveReferens()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    doc->DecrementReferens(spl->GetP1().getIdTool());
    doc->DecrementReferens(spl->GetP2().getIdTool());
    doc->DecrementReferens(spl->GetP3().getIdTool());
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezier*>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    const VCubicBezier spl = dialogTool->GetSpline();
    m_autoSmooth = dialogTool->GetAutoSmooth();

    SetSplineAttributes(domElement, spl);
    doc->SetAttribute(domElement, AttrColor,      dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrPenStyle,   dialogTool->getPenStyle());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    auto spl = qSharedPointerDynamicCast<VCubicBezier>(obj);
    SCASSERT(spl.isNull() == false)
    SetSplineAttributes(tag, *spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetVisualization()
{
    if (not vis.isNull())
    {
        auto visual = qobject_cast<VisToolCubicBezier *>(vis);
        SCASSERT(visual != nullptr)

        const QSharedPointer<VCubicBezier> spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);

        visual->setObject1Id(spl->GetP1().id());
        visual->setObject2Id(spl->GetP2().id());
        visual->setObject3Id(spl->GetP3().id());
        visual->setObject4Id(spl->GetP4().id());
        visual->setLineStyle(lineTypeToPenStyle(spl->GetPenStyle()));
        visual->setLineWeight(spl->getLineWeight());
        visual->setShowPoints(static_cast<QPointF>(spl->GetP1()),
                              static_cast<QPointF>(spl->GetP2()),
                              static_cast<QPointF>(spl->GetP3()),
                              static_cast<QPointF>(spl->GetP4()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::refreshGeometry()
{
    const QSharedPointer<VCubicBezier> spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    this->setPath(spl->GetPath());

    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl)
{
    SCASSERT(doc != nullptr)

    doc->SetAttribute(domElement, AttrType,    ToolType);
    doc->SetAttribute(domElement, AttrPoint1,  spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint2,  m_p2Id);
    doc->SetAttribute(domElement, AttrPoint3,  m_p3Id);
    doc->SetAttribute(domElement, AttrPoint4,  spl.GetP4().id());

    if (spl.GetDuplicate() > 0)
    {
        doc->SetAttribute(domElement, AttrDuplicate, spl.GetDuplicate());
    }
    else
    {
        if (domElement.hasAttribute(AttrDuplicate))
        {
            domElement.removeAttribute(AttrDuplicate);
        }
    }

    if (m_autoSmooth)
    {
        doc->SetAttribute(domElement, AttrAutoSmooth, QStringLiteral("true"));
    }
    else
    {
        domElement.removeAttribute(AttrAutoSmooth);
    }
}

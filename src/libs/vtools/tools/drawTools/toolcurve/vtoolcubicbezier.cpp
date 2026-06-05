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
 **  @file   vtoolcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 *************************************************************************/

#include "vtoolcubicbezier.h"

#include <QDomElement>
#include <QLineF>
#include <QPair>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <QString>
#include <Qt>
#include <QtMath>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogcubicbezier.h"
#include "../../../visualization/visualization.h"
#include "../../../visualization/path/vistoolcubicbezier.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "vabstractspline.h"

const QString VToolCubicBezier::ToolType = QStringLiteral("cubicBezier");

// ---------------------------------------------------------------------------
// State 2: Bisection – find c2Length (pixels) so that the cubic Bézier
// P1-P2-P3(c2)-P4 has exactly targetLength.
//
// Fallback: returns 0.0 when targetLength <= minimum achievable length
// (degenerate curve with c2=0). This keeps the model computable when
// formulas temporarily produce impossible values.
// ---------------------------------------------------------------------------
static qreal findC2Length(const VPointF &p1, const VPointF &p2,
                           const VPointF &p4, qreal angle2Deg,
                           qreal targetLength)
{
    const qreal eps = ToPixel(0.01, Unit::Mm);

    auto curveLen = [&](qreal c2) -> qreal {
        QLineF h(static_cast<QPointF>(p4),
                 static_cast<QPointF>(p4) + QPointF(c2, 0.0));
        h.setAngle(angle2Deg);
        return VCubicBezier(p1, p2, VPointF(h.p2()), p4).GetLength();
    };

    const qreal minLen = curveLen(0.0);
    if (targetLength <= minLen)
        return 0.0;

    qreal lo = 0.0;
    qreal hi = qMax(targetLength * 2.0, 1.0);
    for (int guard = 0; guard < 64 && curveLen(hi) < targetLength; ++guard)
        hi *= 2.0;

    while ((hi - lo) > eps)
    {
        const qreal mid = (lo + hi) * 0.5;
        if (curveLen(mid) < targetLength)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) * 0.5;
}

// ---------------------------------------------------------------------------
// State 3/4: Hobby approximation – compute the ideal handle lengths for
// the smoothest cubic Bézier given two endpoint angles (degrees, Qt system).
//
// Implements Hobby (1986) "Smooth, Easy to Compute Interpolating Splines"
// with tension = 1.  Returns {c1Px, c2Px} in pixels.
//
// Fallback: if the chord is degenerate (d≈0) or the velocity function
// collapses (f≤0), both handles default to d/3 (standard Bézier thirds).
// ---------------------------------------------------------------------------
static QPair<qreal, qreal> hobbyHandleLengths(const VPointF &p1, const VPointF &p4,
                                               qreal angle1Deg, qreal angle2Deg)
{
    const QLineF chord(static_cast<QPointF>(p1), static_cast<QPointF>(p4));
    const qreal d = chord.length();

    if (qFuzzyIsNull(d))
        return {0.0, 0.0};

    // Qt angle convention: degrees CCW from +x, where "up" visually = 90°
    // This matches standard math angles, so we can use the formula directly.
    const qreal chordRad = qDegreesToRadians(chord.angle());

    // θ = outgoing angle at P1 relative to chord direction
    qreal theta = qDegreesToRadians(angle1Deg) - chordRad;
    // φ = incoming angle at P4 relative to reversed chord direction
    qreal phi   = qDegreesToRadians(angle2Deg) - qDegreesToRadians(chord.angle() + 180.0);

    // Normalise to (−π, π]
    const qreal twoPi = 2.0 * M_PI;
    while (theta >  M_PI) theta -= twoPi;
    while (theta < -M_PI) theta += twoPi;
    while (phi   >  M_PI) phi   -= twoPi;
    while (phi   < -M_PI) phi   += twoPi;

    // Hobby's velocity function f(a,b), tension = 1
    //   c1 = d * 2 / (3 * f(θ, φ))
    //   c2 = d * 2 / (3 * f(φ, θ))
    // f(0,0) = 2  →  c = d/3  (standard smooth Bézier thirds)
    const qreal sq2 = qSqrt(2.0);
    auto f = [&](qreal a, qreal b) -> qreal {
        return (2.0 + sq2
                    * (qSin(a) - qSin(b) / 16.0)
                    * (qSin(b) - qSin(a) / 16.0)
                    * (qCos(a) - qCos(b))) / 3.0;
    };

    const qreal ftp = f(theta, phi);
    const qreal fpt = f(phi, theta);
    const qreal fallback = d / 3.0;

    const qreal c1 = (ftp > 1e-9) ? qBound(1e-4, d * 2.0 / (3.0 * ftp), d * 8.0) : fallback;
    const qreal c2 = (fpt > 1e-9) ? qBound(1e-4, d * 2.0 / (3.0 * fpt), d * 8.0) : fallback;

    return {c1, c2};
}

// ---------------------------------------------------------------------------
// State 4: Bisection – find a global scale factor so that
//   VCubicBezier(P1, P2(c1H·s), P3(c2H·s), P4).GetLength() == targetLength
// where c1H and c2H are the Hobby handles.
// ---------------------------------------------------------------------------
static qreal findScaleFactor(const VPointF &p1, const VPointF &p4,
                              qreal angle1Deg, qreal angle2Deg,
                              qreal hobbyC1, qreal hobbyC2,
                              qreal targetLength)
{
    const qreal eps = ToPixel(0.01, Unit::Mm);

    if (hobbyC1 <= 0.0 && hobbyC2 <= 0.0)
        return 0.0;

    auto curveLen = [&](qreal scale) -> qreal {
        QLineF h1(static_cast<QPointF>(p1),
                  static_cast<QPointF>(p1) + QPointF(hobbyC1 * scale, 0.0));
        h1.setAngle(angle1Deg);
        QLineF h2(static_cast<QPointF>(p4),
                  static_cast<QPointF>(p4) + QPointF(hobbyC2 * scale, 0.0));
        h2.setAngle(angle2Deg);
        return VCubicBezier(p1, VPointF(h1.p2()), VPointF(h2.p2()), p4).GetLength();
    };

    const qreal minLen = curveLen(0.0);
    if (targetLength <= minLen)
        return 0.0;

    qreal lo = 0.0;
    qreal hi = 1.0;
    for (int guard = 0; guard < 64 && curveLen(hi) < targetLength; ++guard)
        hi *= 2.0;

    const qreal refLen = qMax(hobbyC1, hobbyC2);
    while ((hi - lo) * refLen > eps)
    {
        const qreal mid = (lo + hi) * 0.5;
        if (curveLen(mid) < targetLength)
            lo = mid;
        else
            hi = mid;
    }
    return (lo + hi) * 0.5;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier::VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                                    const QString &angle1, const QString &angle2,
                                    const QString &c1Length, const QString &c2Length,
                                    const QString &targetLength, bool autoSmooth,
                                    const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , m_angle1(angle1)
    , m_angle2(angle2)
    , m_c1Length(c1Length)
    , m_c2Length(c2Length)
    , m_targetLength(targetLength)
    , m_autoSmooth(autoSmooth)
{
    m_sceneType = SceneObject::Spline;
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezier *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    dialogTool->SetPoint1(spl->GetP1().id());
    dialogTool->SetPoint4(spl->GetP4().id());
    dialogTool->SetAngle1(m_angle1);
    dialogTool->SetAngle2(m_angle2);
    dialogTool->SetC1Length(m_c1Length);
    dialogTool->SetC2Length(m_c2Length);
    dialogTool->SetTargetLength(m_targetLength);
    dialogTool->SetAutoSmooth(m_autoSmooth);
    dialogTool->setLineColor(spl->getLineColor());
    dialogTool->setPenStyle(spl->GetPenStyle());
    dialogTool->setLineWeight(spl->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(QSharedPointer<DialogTool> dialog,
                                            VMainGraphicsScene *scene,
                                            VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogCubicBezier> dialogTool = dialog.objectCast<DialogCubicBezier>();
    SCASSERT(not dialogTool.isNull())

    const quint32 p1Id      = dialogTool->GetPoint1();
    const quint32 p4Id      = dialogTool->GetPoint4();
    QString angle1          = dialogTool->GetAngle1();
    QString angle2          = dialogTool->GetAngle2();
    QString c1Length        = dialogTool->GetC1Length();
    QString c2Length        = dialogTool->GetC2Length();
    QString targetLength    = dialogTool->GetTargetLength();
    const bool autoSmooth   = dialogTool->GetAutoSmooth();
    const QString color     = dialogTool->getLineColor();
    const QString penStyle  = dialogTool->getPenStyle();
    const QString lineWeight = dialogTool->getLineWeight();

    VToolCubicBezier *tool = Create(0, p1Id, p4Id,
                                    angle1, angle2, c1Length, c2Length,
                                    targetLength, autoSmooth,
                                    color, penStyle, lineWeight,
                                    scene, doc, data, Document::FullParse, Source::FromGui);
    if (tool != nullptr)
        tool->m_dialog = dialogTool;
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(const quint32 _id,
                                            const quint32 point1Id, const quint32 point4Id,
                                            QString &angle1, QString &angle2,
                                            QString &c1Length, QString &c2Length,
                                            QString &targetLength, bool autoSmooth,
                                            const QString &color, const QString &penStyle,
                                            const QString &lineWeight,
                                            VMainGraphicsScene *scene, VAbstractPattern *doc,
                                            VContainer *data, const Document &parse,
                                            const Source &typeCreation)
{
    const qreal calcAngle1 = CheckFormula(_id, angle1, data);
    const qreal calcAngle2 = CheckFormula(_id, angle2, data);

    const VPointF p1 = *data->GeometricObject<VPointF>(point1Id);
    const VPointF p4 = *data->GeometricObject<VPointF>(point4Id);

    const bool hasTarget   = !targetLength.isEmpty();

    VPointF p2, p3;

    if (autoSmooth)
    {
        // State 3 or 4: Hobby algorithm determines handle lengths
        const QPair<qreal,qreal> hobby = hobbyHandleLengths(p1, p4, calcAngle1, calcAngle2);
        qreal c1Px = hobby.first;
        qreal c2Px = hobby.second;

        if (hasTarget)
        {
            // State 4: scale Hobby handles proportionally to hit target length
            const qreal calcTarget = qApp->toPixel(CheckFormula(_id, targetLength, data));
            const qreal scale = findScaleFactor(p1, p4, calcAngle1, calcAngle2,
                                                c1Px, c2Px, calcTarget);
            c1Px *= scale;
            c2Px *= scale;
        }

        QLineF h1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(c1Px, 0.0));
        h1.setAngle(calcAngle1);
        p2 = VPointF(h1.p2());

        QLineF h2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(c2Px, 0.0));
        h2.setAngle(calcAngle2);
        p3 = VPointF(h2.p2());
    }
    else if (hasTarget)
    {
        // State 2: bisection finds c2 to hit target length
        const qreal calcC1  = qApp->toPixel(CheckFormula(_id, c1Length, data));
        const qreal calcTgt = qApp->toPixel(CheckFormula(_id, targetLength, data));

        QLineF h1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(calcC1, 0.0));
        h1.setAngle(calcAngle1);
        p2 = VPointF(h1.p2());

        const qreal calcC2 = findC2Length(p1, p2, p4, calcAngle2, calcTgt);
        QLineF h2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(calcC2, 0.0));
        h2.setAngle(calcAngle2);
        p3 = VPointF(h2.p2());
    }
    else
    {
        // State 1: fully manual — both c1 and c2 from formulas
        const qreal calcC1 = qApp->toPixel(CheckFormula(_id, c1Length, data));
        const qreal calcC2 = qApp->toPixel(CheckFormula(_id, c2Length, data));

        QLineF h1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(calcC1, 0.0));
        h1.setAngle(calcAngle1);
        p2 = VPointF(h1.p2());

        QLineF h2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(calcC2, 0.0));
        h2.setAngle(calcAngle2);
        p3 = VPointF(h2.p2());
    }

    VCubicBezier *spline = new VCubicBezier(p1, p2, p3, p4);
    spline->setLineColor(color);
    spline->SetPenStyle(penStyle);
    spline->setLineWeight(lineWeight);

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
            doc->UpdateToolData(id, data);
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::CubicBezier, doc);
        auto tool = new VToolCubicBezier(doc, data, id,
                                         angle1, angle2, c1Length, c2Length,
                                         targetLength, autoSmooth,
                                         typeCreation);
        scene->addItem(tool);
        initSplineToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);
        doc->IncrementReferens(p1.getIdTool());
        doc->IncrementReferens(p4.getIdTool());
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::GetP1Name() const
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spl->GetP1().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::GetP4Name() const
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spl->GetP4().name();
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezier::GetFormulaAngle1() const
{
    VFormula f(m_angle1, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetFormulaAngle1(const VFormula &value)
{
    m_angle1 = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezier::GetFormulaAngle2() const
{
    VFormula f(m_angle2, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetFormulaAngle2(const VFormula &value)
{
    m_angle2 = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezier::GetFormulaC1Length() const
{
    VFormula f(m_c1Length, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetFormulaC1Length(const VFormula &value)
{
    m_c1Length = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezier::GetFormulaC2Length() const
{
    VFormula f(m_c2Length, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetFormulaC2Length(const VFormula &value)
{
    m_c2Length = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezier::GetFormulaTargetLength() const
{
    VFormula f(m_targetLength, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetFormulaTargetLength(const VFormula &value)
{
    m_targetLength = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
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
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::getMinLengthString() const
{
    // Minimum arc length when c2 = 0 (P3 == P4)
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    VCubicBezier minSpl(spl->GetP1(), spl->GetP2(), spl->GetP4(), spl->GetP4());
    const qreal minPx = minSpl.GetLength();
    const qreal minMm = qApp->fromPixel(minPx);
    return QString::number(minMm, 'f', 2) + " " + UnitsToStr(qApp->patternUnit(), true);
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
    catch (const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::RemoveReferens()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    doc->DecrementReferens(spl->GetP1().getIdTool());
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezier *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    m_angle1       = dialogTool->GetAngle1();
    m_angle2       = dialogTool->GetAngle2();
    m_c1Length     = dialogTool->GetC1Length();
    m_c2Length     = dialogTool->GetC2Length();
    m_targetLength = dialogTool->GetTargetLength();
    m_autoSmooth   = dialogTool->GetAutoSmooth();

    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    SetSplineAttributes(domElement, *spl);
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

        const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
        visual->setObject1Id(spl->GetP1().id());
        visual->setObject4Id(spl->GetP4().id());

        // Pass the computed handle geometry so the visualization matches exactly.
        const QLineF c1Line(static_cast<QPointF>(spl->GetP1()),
                            static_cast<QPointF>(spl->GetP2()));
        const QLineF c2Line(static_cast<QPointF>(spl->GetP4()),
                            static_cast<QPointF>(spl->GetP3()));
        visual->setAngle1(QString::number(c1Line.angle()));
        visual->setAngle2(QString::number(c2Line.angle()));
        visual->setC1Length(QString::number(qApp->fromPixel(c1Line.length())));
        visual->setC2Length(c2Line.length());

        visual->setLineStyle(lineTypeToPenStyle(spl->GetPenStyle()));
        visual->setLineWeight(spl->getLineWeight());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::refreshGeometry()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    this->setPath(spl->GetPath());
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl)
{
    SCASSERT(doc != nullptr)

    doc->SetAttribute(domElement, AttrType,   ToolType);
    doc->SetAttribute(domElement, AttrPoint1, spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint4, spl.GetP4().id());
    doc->SetAttribute(domElement, AttrAngle1, m_angle1);
    doc->SetAttribute(domElement, AttrAngle2, m_angle2);

    if (!m_autoSmooth)
    {
        // States 1 and 2: manual c1 always present
        doc->SetAttribute(domElement, AttrLength1, m_c1Length);
        // State 1 only: manual c2
        if (m_targetLength.isEmpty())
            doc->SetAttribute(domElement, AttrLength2, m_c2Length);
        else
            domElement.removeAttribute(AttrLength2);
    }
    else
    {
        // States 3 and 4: no manual handle lengths
        domElement.removeAttribute(AttrLength1);
        domElement.removeAttribute(AttrLength2);
    }

    if (!m_targetLength.isEmpty())
        doc->SetAttribute(domElement, AttrLength, m_targetLength);
    else
        domElement.removeAttribute(AttrLength);

    if (m_autoSmooth)
        doc->SetAttribute(domElement, AttrAutoSmooth, QStringLiteral("true"));
    else
        domElement.removeAttribute(AttrAutoSmooth);

    if (spl.GetDuplicate() > 0)
        doc->SetAttribute(domElement, AttrDuplicate, spl.GetDuplicate());
    else if (domElement.hasAttribute(AttrDuplicate))
        domElement.removeAttribute(AttrDuplicate);
}

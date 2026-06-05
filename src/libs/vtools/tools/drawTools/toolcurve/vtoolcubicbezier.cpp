/***************************************************************************
 **  @file   vtoolcubicbezier.cpp
 **  @author Douglas S Caskey / Seamly2D contributors
 **
 **  "Kurve Fixiert" — user selects 4 canvas points; two optional checkboxes
 **  modify how handle points P2/P3 are interpreted.
 **************************************************************************/

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
// State 2: Bisection — find a scale factor so that
//   VCubicBezier(P1, P1+c1*s, P4+c2*s, P4).GetLength() == targetLength
// where c1 and c2 are the ORIGINAL handle vectors (P1→P2, P4→P3).
// ---------------------------------------------------------------------------
static qreal findScaleFactor(const VPointF &p1, const VPointF &p4,
                              qreal angle1Deg, qreal angle2Deg,
                              qreal baseC1Px, qreal baseC2Px,
                              qreal targetLength)
{
    const qreal eps = ToPixel(0.01, Unit::Mm);

    if (baseC1Px <= 0.0 && baseC2Px <= 0.0)
        return 1.0;

    auto curveLen = [&](qreal scale) -> qreal {
        QLineF h1(static_cast<QPointF>(p1),
                  static_cast<QPointF>(p1) + QPointF(baseC1Px * scale, 0.0));
        h1.setAngle(angle1Deg);
        QLineF h2(static_cast<QPointF>(p4),
                  static_cast<QPointF>(p4) + QPointF(baseC2Px * scale, 0.0));
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

    const qreal ref = qMax(baseC1Px, baseC2Px);
    while ((hi - lo) * ref > eps)
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
// States 3/4: Hobby (1986) — compute optimal handle LENGTHS from the angles
// derived from the selected P2/P3 canvas points.
//
// P2/P3 define DIRECTIONS only. Hobby sets lengths for maximum smoothness.
// Returns {c1Px, c2Px} in pixels.
// ---------------------------------------------------------------------------
static QPair<qreal, qreal> hobbyHandleLengths(const VPointF &p1, const VPointF &p4,
                                               qreal angle1Deg, qreal angle2Deg)
{
    const QLineF chord(static_cast<QPointF>(p1), static_cast<QPointF>(p4));
    const qreal d = chord.length();

    if (qFuzzyIsNull(d))
        return {0.0, 0.0};

    const qreal chordRad = qDegreesToRadians(chord.angle());
    qreal theta = qDegreesToRadians(angle1Deg) - chordRad;
    qreal phi   = qDegreesToRadians(angle2Deg) - qDegreesToRadians(chord.angle() + 180.0);

    const qreal twoPi = 2.0 * M_PI;
    while (theta >  M_PI) theta -= twoPi;
    while (theta < -M_PI) theta += twoPi;
    while (phi   >  M_PI) phi   -= twoPi;
    while (phi   < -M_PI) phi   += twoPi;

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
// Core geometry computation — returns a VCubicBezier whose P2/P3 positions
// reflect the active state.  p2canvas/p3canvas are the ORIGINAL canvas points.
// targetPx == 0 means no target length.
// ---------------------------------------------------------------------------
static VCubicBezier computeSplineGeometry(
    const VPointF &p1, const VPointF &p2canvas,
    const VPointF &p3canvas, const VPointF &p4,
    qreal targetPx, bool autoSmooth)
{
    // Derive angles and pixel lengths from selected canvas points
    const QLineF h1orig(static_cast<QPointF>(p1), static_cast<QPointF>(p2canvas));
    const QLineF h2orig(static_cast<QPointF>(p4), static_cast<QPointF>(p3canvas));
    const qreal angle1 = h1orig.angle();
    const qreal angle2 = h2orig.angle();
    const qreal c1orig = h1orig.length();
    const qreal c2orig = h2orig.length();

    const bool hasTarget = (targetPx > 0.0);

    if (!autoSmooth && !hasTarget)
    {
        // State 1: original geometry, no modification
        return VCubicBezier(p1, p2canvas, p3canvas, p4);
    }

    qreal c1, c2;

    if (autoSmooth)
    {
        // States 3/4: Hobby computes lengths from the P2/P3 directions
        const QPair<qreal,qreal> hobby = hobbyHandleLengths(p1, p4, angle1, angle2);
        c1 = hobby.first;
        c2 = hobby.second;

        if (hasTarget)
        {
            // State 4: scale Hobby handles proportionally to hit target
            const qreal scale = findScaleFactor(p1, p4, angle1, angle2, c1, c2, targetPx);
            c1 *= scale;
            c2 *= scale;
        }
    }
    else
    {
        // State 2: preserve original proportions, scale to hit target
        const qreal scale = findScaleFactor(p1, p4, angle1, angle2, c1orig, c2orig, targetPx);
        c1 = c1orig * scale;
        c2 = c2orig * scale;
    }

    QLineF nh1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(c1, 0.0));
    nh1.setAngle(angle1);
    QLineF nh2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(c2, 0.0));
    nh2.setAngle(angle2);

    return VCubicBezier(p1, VPointF(nh1.p2()), VPointF(nh2.p2()), p4);
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier::VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                                    quint32 p2Id, quint32 p3Id,
                                    const QString &targetLength, bool autoSmooth,
                                    const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , m_p2Id(p2Id)
    , m_p3Id(p3Id)
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
    // P1 and P4 are always the original canvas points
    dialogTool->SetP1Id(spl->GetP1().id());
    dialogTool->SetP2Id(m_p2Id);
    dialogTool->SetP3Id(m_p3Id);
    dialogTool->SetP4Id(spl->GetP4().id());
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

    const quint32 p1Id     = dialogTool->GetP1Id();
    const quint32 p2Id     = dialogTool->GetP2Id();
    const quint32 p3Id     = dialogTool->GetP3Id();
    const quint32 p4Id     = dialogTool->GetP4Id();
    QString targetLength   = dialogTool->GetTargetLength();
    const bool autoSmooth  = dialogTool->GetAutoSmooth();
    const QString color    = dialogTool->getLineColor();
    const QString penStyle = dialogTool->getPenStyle();
    const QString lineWeight = dialogTool->getLineWeight();

    VToolCubicBezier *tool = Create(0, p1Id, p2Id, p3Id, p4Id,
                                     targetLength, autoSmooth,
                                     color, penStyle, lineWeight,
                                     scene, doc, data, Document::FullParse, Source::FromGui);
    if (tool != nullptr)
        tool->m_dialog = dialogTool;
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(const quint32 _id,
                                            const quint32 p1Id, const quint32 p2Id,
                                            const quint32 p3Id, const quint32 p4Id,
                                            const QString &targetLength, bool autoSmooth,
                                            const QString &color, const QString &penStyle,
                                            const QString &lineWeight,
                                            VMainGraphicsScene *scene, VAbstractPattern *doc,
                                            VContainer *data, const Document &parse,
                                            const Source &typeCreation)
{
    // Fetch the 4 canvas points
    const VPointF p1 = *data->GeometricObject<VPointF>(p1Id);
    const VPointF p2 = *data->GeometricObject<VPointF>(p2Id);
    const VPointF p3 = *data->GeometricObject<VPointF>(p3Id);
    const VPointF p4 = *data->GeometricObject<VPointF>(p4Id);

    // Evaluate target length formula to pixels (0 if unused)
    qreal targetPx = 0.0;
    QString tl = targetLength;  // local copy for CheckFormula
    if (!tl.isEmpty())
        targetPx = qApp->toPixel(CheckFormula(_id, tl, data));

    // Compute the actual curve geometry based on state
    VCubicBezier *spline = new VCubicBezier(
        computeSplineGeometry(p1, p2, p3, p4, targetPx, autoSmooth));
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
                                          p2Id, p3Id,
                                          targetLength, autoSmooth,
                                          typeCreation);
        scene->addItem(tool);
        initSplineToolConnections(scene, tool);
        VAbstractPattern::AddTool(id, tool);
        doc->IncrementReferens(p1.getIdTool());
        doc->IncrementReferens(p2.getIdTool());
        doc->IncrementReferens(p3.getIdTool());
        doc->IncrementReferens(p4.getIdTool());
        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::FirstPointName() const
{
    return VAbstractTool::data.GeometricObject<VCubicBezier>(m_id)->GetP1().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::SecondPointName() const
{
    // Return the original canvas P2 name, not the computed position
    return VAbstractTool::data.GeometricObject<VPointF>(m_p2Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::ThirdPointName() const
{
    return VAbstractTool::data.GeometricObject<VPointF>(m_p3Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::ForthPointName() const
{
    return VAbstractTool::data.GeometricObject<VCubicBezier>(m_id)->GetP4().name();
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier VToolCubicBezier::getSpline() const
{
    return *VAbstractTool::data.GeometricObject<VCubicBezier>(m_id).data();
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
    recomputeGeometry();
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
    recomputeGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::recomputeGeometry()
{
    // Rebuild the stored VCubicBezier from the original canvas points + active state.
    const auto stored = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    const VPointF p1 = stored->GetP1();
    const VPointF p4 = stored->GetP4();
    const VPointF p2 = *VAbstractTool::data.GeometricObject<VPointF>(m_p2Id);
    const VPointF p3 = *VAbstractTool::data.GeometricObject<VPointF>(m_p3Id);

    qreal targetPx = 0.0;
    if (!m_targetLength.isEmpty())
    {
        QString tl = m_targetLength;
        targetPx = qApp->toPixel(CheckFormula(m_id, tl, &(VAbstractTool::data)));
    }

    VCubicBezier rebuilt = computeSplineGeometry(p1, p2, p3, p4, targetPx, m_autoSmooth);
    rebuilt.setLineColor(stored->getLineColor());
    rebuilt.SetPenStyle(stored->GetPenStyle());
    rebuilt.setLineWeight(stored->getLineWeight());
    rebuilt.SetDuplicate(stored->GetDuplicate());

    setSpline(rebuilt);   // updates container + SaveOption
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezier::getMinLengthString() const
{
    // Minimum arc length is the straight chord P1→P4 (all handles collapsed)
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    VCubicBezier minSpl(spl->GetP1(), spl->GetP1(), spl->GetP4(), spl->GetP4());
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
    // Decrement all 4 original canvas point references
    doc->DecrementReferens(spl->GetP1().getIdTool());
    // P2/P3 are tracked via m_p2Id/m_p3Id (not from stored spline which has computed positions)
    if (m_p2Id != NULL_ID)
    {
        const auto p2 = VAbstractTool::data.GeometricObject<VPointF>(m_p2Id);
        doc->DecrementReferens(p2->getIdTool());
    }
    if (m_p3Id != NULL_ID)
    {
        const auto p3 = VAbstractTool::data.GeometricObject<VPointF>(m_p3Id);
        doc->DecrementReferens(p3->getIdTool());
    }
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezier *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    m_p2Id         = dialogTool->GetP2Id();
    m_p3Id         = dialogTool->GetP3Id();
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

        // Pass the COMPUTED handle geometry (P2'/P3' positions) for the preview.
        // These may differ from the original P2/P3 canvas points in States 2/3/4.
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
    doc->SetAttribute(domElement, AttrPoint2, m_p2Id);   // original canvas P2
    doc->SetAttribute(domElement, AttrPoint3, m_p3Id);   // original canvas P3
    doc->SetAttribute(domElement, AttrPoint4, spl.GetP4().id());

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

/***************************************************************************
 **  @file   vtoolcubicbezierlength.cpp
 **  @author Seamly2D contributors
 **
 **  @brief  Cubic Bézier curve whose length is matched to a reference via bisection.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#include "vtoolcubicbezierlength.h"

#include <QDomElement>
#include <QLineF>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogcubicbezierlength.h"
#include "../../../visualization/visualization.h"
#include "../../../visualization/path/vistoolcubicbezierlength.h"
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

const QString VToolCubicBezierLength::ToolType = QStringLiteral("cubicBezierLength");

// ---------------------------------------------------------------------------
// Bisection: find c2Length so that the cubic Bézier P1-P2-P3(c2)-P4
// has the given targetLength (pixels).
//
// Fallback: returns 0.0 when targetLength <= the minimum achievable length
// (c2 = 0). This keeps the model computable when formulas produce temporarily
// impossible measurements, instead of aborting with an exception.
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

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezierLength::VToolCubicBezierLength(VAbstractPattern *doc, VContainer *data, quint32 id,
                                                const QString &angle1, const QString &angle2,
                                                const QString &c1Length, const QString &targetLength,
                                                const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , m_angle1(angle1)
    , m_angle2(angle2)
    , m_c1Length(c1Length)
    , m_targetLength(targetLength)
{
    m_sceneType = SceneObject::Spline;
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezierLength *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    dialogTool->SetPoint1(spl->GetP1().id());
    dialogTool->SetPoint4(spl->GetP4().id());
    dialogTool->SetAngle1(m_angle1);
    dialogTool->SetAngle2(m_angle2);
    dialogTool->SetC1Length(m_c1Length);
    dialogTool->SetTargetLength(m_targetLength);
    dialogTool->setLineColor(spl->getLineColor());
    dialogTool->setPenStyle(spl->GetPenStyle());
    dialogTool->setLineWeight(spl->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezierLength *VToolCubicBezierLength::Create(QSharedPointer<DialogTool> dialog,
                                                        VMainGraphicsScene *scene,
                                                        VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogCubicBezierLength> dialogTool = dialog.objectCast<DialogCubicBezierLength>();
    SCASSERT(not dialogTool.isNull())

    const quint32 p1Id       = dialogTool->GetPoint1();
    const quint32 p4Id       = dialogTool->GetPoint4();
    QString angle1            = dialogTool->GetAngle1();
    QString angle2            = dialogTool->GetAngle2();
    QString c1Length          = dialogTool->GetC1Length();
    QString targetLength      = dialogTool->GetTargetLength();
    const QString color       = dialogTool->getLineColor();
    const QString penStyle    = dialogTool->getPenStyle();
    const QString lineWeight  = dialogTool->getLineWeight();

    VToolCubicBezierLength *tool = Create(0, p1Id, p4Id, angle1, angle2, c1Length, targetLength,
                                           color, penStyle, lineWeight,
                                           scene, doc, data, Document::FullParse, Source::FromGui);
    if (tool != nullptr)
        tool->m_dialog = dialogTool;
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezierLength *VToolCubicBezierLength::Create(const quint32 _id,
                                                        const quint32 point1Id, const quint32 point4Id,
                                                        QString &angle1, QString &angle2,
                                                        QString &c1Length, QString &targetLength,
                                                        const QString &color, const QString &penStyle,
                                                        const QString &lineWeight,
                                                        VMainGraphicsScene *scene, VAbstractPattern *doc,
                                                        VContainer *data, const Document &parse,
                                                        const Source &typeCreation)
{
    // Evaluate formula strings to numeric values
    const qreal calcAngle1  = CheckFormula(_id, angle1, data);
    const qreal calcC1Len   = qApp->toPixel(CheckFormula(_id, c1Length, data));
    const qreal calcAngle2  = CheckFormula(_id, angle2, data);
    const qreal calcTarget  = qApp->toPixel(CheckFormula(_id, targetLength, data));

    // Fetch base points from container
    const VPointF p1 = *data->GeometricObject<VPointF>(point1Id);
    const VPointF p4 = *data->GeometricObject<VPointF>(point4Id);

    // P2 = P1 displaced by c1 along angle1
    QLineF h1(static_cast<QPointF>(p1), static_cast<QPointF>(p1) + QPointF(calcC1Len, 0.0));
    h1.setAngle(calcAngle1);
    const VPointF p2(h1.p2());

    // Bisection: determine c2Length so the curve length matches the target
    const qreal calcC2Len = findC2Length(p1, p2, p4, calcAngle2, calcTarget);

    // P3 = P4 displaced by c2 along angle2
    QLineF h2(static_cast<QPointF>(p4), static_cast<QPointF>(p4) + QPointF(calcC2Len, 0.0));
    h2.setAngle(calcAngle2);
    const VPointF p3(h2.p2());

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
        VDrawTool::AddRecord(id, Tool::CubicBezierLength, doc);
        auto tool = new VToolCubicBezierLength(doc, data, id,
                                                angle1, angle2, c1Length, targetLength,
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
VFormula VToolCubicBezierLength::GetFormulaAngle1() const
{
    VFormula f(m_angle1, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetFormulaAngle1(const VFormula &value)
{
    m_angle1 = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezierLength::GetFormulaAngle2() const
{
    VFormula f(m_angle2, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetFormulaAngle2(const VFormula &value)
{
    m_angle2 = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezierLength::GetFormulaC1Length() const
{
    VFormula f(m_c1Length, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetFormulaC1Length(const VFormula &value)
{
    m_c1Length = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolCubicBezierLength::GetFormulaTargetLength() const
{
    VFormula f(m_targetLength, this->getData());
    f.setCheckZero(false);
    f.Eval();
    return f;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetFormulaTargetLength(const VFormula &value)
{
    m_targetLength = value.GetFormula(FormulaType::FromUser);
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCubicBezierLength>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogCubicBezierLength>(event);
    }
    catch (const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::RemoveReferens()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    doc->DecrementReferens(spl->GetP1().getIdTool());
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogCubicBezierLength *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    m_angle1        = dialogTool->GetAngle1();
    m_angle2        = dialogTool->GetAngle2();
    m_c1Length      = dialogTool->GetC1Length();
    m_targetLength  = dialogTool->GetTargetLength();

    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    SetSplineAttributes(domElement, *spl);
    doc->SetAttribute(domElement, AttrColor,      dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrPenStyle,   dialogTool->getPenStyle());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    auto spl = qSharedPointerDynamicCast<VCubicBezier>(obj);
    SCASSERT(spl.isNull() == false)
    SetSplineAttributes(tag, *spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetVisualization()
{
    if (not vis.isNull())
    {
        auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
        SCASSERT(visual != nullptr)

        const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
        visual->setObject1Id(spl->GetP1().id());
        visual->setObject4Id(spl->GetP4().id());
        visual->setAngle1(m_angle1);
        visual->setAngle2(m_angle2);
        visual->setC1Length(m_c1Length);
        visual->setTargetLength(m_targetLength);
        // Pass actual computed c2Length so P3 handle is drawn correctly
        const QLineF c2Line(static_cast<QPointF>(spl->GetP4()),
                            static_cast<QPointF>(spl->GetP3()));
        visual->setC2Length(c2Line.length());
        visual->setLineStyle(lineTypeToPenStyle(spl->GetPenStyle()));
        visual->setLineWeight(spl->getLineWeight());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::refreshGeometry()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    this->setPath(spl->GetPath());
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezierLength::getMinLengthString() const
{
    // Minimum arc length is achieved when c2Length == 0 (P3 == P4)
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    VCubicBezier minSpl(spl->GetP1(), spl->GetP2(), spl->GetP4(), spl->GetP4());
    const qreal minPx = minSpl.GetLength();
    const qreal minMm = qApp->fromPixel(minPx);
    return QString::number(minMm, 'f', 2) + " " + UnitsToStr(qApp->patternUnit(), true);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezierLength::GetP1Name() const
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spl->GetP1().name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCubicBezierLength::GetP4Name() const
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spl->GetP4().name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierLength::SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl)
{
    SCASSERT(doc != nullptr)
    doc->SetAttribute(domElement, AttrType,    ToolType);
    doc->SetAttribute(domElement, AttrPoint1,  spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint4,  spl.GetP4().id());
    doc->SetAttribute(domElement, AttrAngle1,  m_angle1);
    doc->SetAttribute(domElement, AttrAngle2,  m_angle2);
    doc->SetAttribute(domElement, AttrLength1, m_c1Length);
    doc->SetAttribute(domElement, AttrLength,  m_targetLength);

    if (spl.GetDuplicate() > 0)
        doc->SetAttribute(domElement, AttrDuplicate, spl.GetDuplicate());
    else if (domElement.hasAttribute(AttrDuplicate))
        domElement.removeAttribute(AttrDuplicate);
}

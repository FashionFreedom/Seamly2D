//  @file   vtoolspline.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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

/************************************************************************
 **  @file   vtoolspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vtoolspline.h"

#include <QDomElement>
#include <QEvent>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QString>
#include <QUndoStack>
#include <QVector>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogspline.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../undocommands/movespline.h"
#include "../../../visualization/visualization.h"
#include "../../../visualization/path/vistoolspline.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "vabstractspline.h"

#include <QPair>
#include <QtMath>

const QString VToolSpline::ToolType = QStringLiteral("simpleInteractive");
const QString VToolSpline::OldToolType = QStringLiteral("simple");

// 8-point Gauss-Legendre arc-length for a cubic Bézier — ~14 digits accuracy,
// 8 evaluations of |B'(t)|. Orders of magnitude faster than recursive subdivision
// for repeated calls inside a bisection loop.
static qreal cubicBezierLengthGL(const QPointF &p1, const QPointF &p2,
                                  const QPointF &p3, const QPointF &p4)
{
    static const qreal t[] = {0.01985071506835568, 0.10166676129318664,
                               0.23723379504183550, 0.40828267875217509,
                               0.59171732124782494, 0.76276620495816450,
                               0.89833323870681336, 0.98014928493164430};
    static const qreal w[] = {0.05061426814518813, 0.11119051722668723,
                               0.15685332293894364, 0.18134189168918099,
                               0.18134189168918099, 0.15685332293894364,
                               0.11119051722668723, 0.05061426814518813};
    qreal len = 0.0;
    for (int i = 0; i < 8; ++i) {
        const qreal s = t[i], q = 1.0 - s;
        const QPointF d = 3.0 * (p2 - p1) * (q * q)
                        + 6.0 * (p3 - p2) * (q * s)
                        + 3.0 * (p4 - p3) * (s * s);
        len += w[i] * qSqrt(d.x() * d.x() + d.y() * d.y());
    }
    return len;
}

// ---------------------------------------------------------------------------
// State 2: Secant method — find scale factor so VSpline arc-length == targetPx.
// Both handles are scaled proportionally by the same factor to preserve shape.
// Converges in ~3-5 iterations (was: 64-iteration bisection).
// ---------------------------------------------------------------------------
static qreal findScaleForSpline(const VPointF &p1, const VPointF &p4,
                                 qreal a1, qreal a2,
                                 qreal baseC1Px, qreal targetPx)
{
    const qreal eps = ToPixel(0.05, Unit::Mm);

    const QPointF p1pt = static_cast<QPointF>(p1);
    const QPointF p4pt = static_cast<QPointF>(p4);
    // Qt screen coords: setAngle(a) → direction (cos(a), -sin(a))
    const qreal a1rad = qDegreesToRadians(a1);
    const qreal a2rad = qDegreesToRadians(a2);
    const QPointF dir1(qCos(a1rad), -qSin(a1rad));
    const QPointF dir2(qCos(a2rad), -qSin(a2rad));

    auto curveLen = [&](qreal s) -> qreal {
        return cubicBezierLengthGL(p1pt, p1pt + dir1 * (baseC1Px * s),
                                   p4pt + dir2 * (baseC1Px * s), p4pt);
    };

    const qreal minLen = curveLen(0.0);
    if (targetPx <= minLen)
        return 0.0;

    qreal hi = 1.0;
    qreal f_hi = curveLen(hi) - targetPx;
    for (int guard = 0; guard < 64 && f_hi < 0.0; ++guard) {
        hi *= 2.0;
        f_hi = curveLen(hi) - targetPx;
    }

    qreal x0 = hi * 0.5, f0 = curveLen(x0) - targetPx;
    qreal x1 = hi,       f1 = f_hi;

    for (int i = 0; i < 10; ++i) {
        if (qAbs(f1 - f0) < 1e-12) break;
        qreal xn = x1 - f1 * (x1 - x0) / (f1 - f0);
        if (xn < 0.0) xn = x1 * 0.5;
        const qreal fn = curveLen(xn) - targetPx;
        if (qAbs(fn) < eps) return xn;
        x0 = x1; f0 = f1;
        x1 = xn; f1 = fn;
    }
    return x1;
}

// ---------------------------------------------------------------------------
// States 3/4: Hobby algorithm — compute optimal handle lengths from angles
// ---------------------------------------------------------------------------
static QPair<qreal, qreal> hobbyHandleLengthsForSpline(const VPointF &p1, const VPointF &p4,
                                                        qreal angle1Deg, qreal angle2Deg)
{
    const QLineF chord(static_cast<QPointF>(p1), static_cast<QPointF>(p4));
    const qreal d = chord.length();
    if (qFuzzyIsNull(d))
        return {d/3.0, d/3.0};

    const qreal chordRad = qDegreesToRadians(chord.angle());
    qreal theta = qDegreesToRadians(angle1Deg) - chordRad;
    qreal phi   = qDegreesToRadians(angle2Deg) - qDegreesToRadians(chord.angle() + 180.0);

    const qreal twoPi = 2.0 * M_PI;
    while (theta >  M_PI) theta -= twoPi;
    while (theta < -M_PI) theta += twoPi;
    while (phi   >  M_PI) phi   -= twoPi;
    while (phi   < -M_PI) phi   += twoPi;

    // Standard Hobby/MetaPost velocity function — same fix as vtoolcubicbezier.
    // The earlier code dropped the denominator factor and inverted the velocity,
    // producing handles ~3x too long.
    const qreal sq2   = qSqrt(2.0);
    const qreal sqrt5 = qSqrt(5.0);
    const qreal cA    = 0.5 * (sqrt5 - 1.0);   // ~0.618
    const qreal cB    = 0.5 * (3.0 - sqrt5);   // ~0.382

    auto velocity = [&](qreal a, qreal b) -> qreal {
        const qreal num = 2.0 + sq2 * (qSin(a) - qSin(b)/16.0)
                                    * (qSin(b) - qSin(a)/16.0)
                                    * (qCos(a) - qCos(b));
        const qreal den = 3.0 * (1.0 + cA*qCos(a) + cB*qCos(b));
        return (qAbs(den) > 1e-9) ? num / den : (1.0/3.0);
    };

    const qreal c1 = qBound(1e-4, velocity(theta, phi) * d, d * 8.0);
    const qreal c2 = qBound(1e-4, velocity(phi, theta) * d, d * 8.0);
    return {c1, c2};
}

// ---------------------------------------------------------------------------
// State 4: Secant method — find global scale factor for Hobby handles to hit
// targetPx. Converges in ~3-5 iterations (was: 64-iteration bisection).
// ---------------------------------------------------------------------------
static qreal findScaleFactorForSpline(const VPointF &p1, const VPointF &p4,
                                       qreal a1, const QString &a1F,
                                       qreal a2, const QString &a2F,
                                       qreal hobbyC1, qreal hobbyC2,
                                       qreal targetPx)
{
    Q_UNUSED(a1F) Q_UNUSED(a2F)
    const qreal eps = ToPixel(0.05, Unit::Mm);
    if (hobbyC1 <= 0.0 && hobbyC2 <= 0.0) return 1.0;

    // Pre-compute direction vectors (Qt screen coords: y points down)
    const qreal a1rad = qDegreesToRadians(a1);
    const qreal a2rad = qDegreesToRadians(a2);
    const qreal cos1 = qCos(a1rad), sin1 = qSin(a1rad);
    const qreal cos2 = qCos(a2rad), sin2 = qSin(a2rad);
    const QPointF p1pt = static_cast<QPointF>(p1);
    const QPointF p4pt = static_cast<QPointF>(p4);

    auto curveLen = [&](qreal scale) -> qreal {
        const qreal c1 = hobbyC1 * scale;
        const qreal c2 = hobbyC2 * scale;
        return cubicBezierLengthGL(p1pt,
                                   p1pt + QPointF(c1 * cos1, -c1 * sin1),
                                   p4pt + QPointF(c2 * cos2, -c2 * sin2),
                                   p4pt);
    };

    const qreal minLen = curveLen(0.0);
    if (targetPx <= minLen) return 0.0;

    qreal hi = 1.0;
    qreal f_hi = curveLen(hi) - targetPx;
    for (int g = 0; g < 64 && f_hi < 0.0; ++g) {
        hi *= 2.0;
        f_hi = curveLen(hi) - targetPx;
    }

    // Secant iteration — starts from the bracket [0, hi]
    qreal x0 = 0.0,  f0 = minLen - targetPx;   // f0 < 0
    qreal x1 = hi,   f1 = f_hi;                // f1 >= 0

    for (int i = 0; i < 10; ++i) {
        if (qAbs(f1 - f0) < 1e-12) break;
        qreal xn = x1 - f1 * (x1 - x0) / (f1 - f0);
        if (xn < 0.0) xn = x1 * 0.5;
        const qreal fn = curveLen(xn) - targetPx;
        if (qAbs(fn) < eps) return xn;
        x0 = x1; f0 = f1;
        x1 = xn; f1 = fn;
    }
    return x1;
}

// ---------------------------------------------------------------------------
// Shared 4-state geometry builder for "Kurve Interaktiv".
// _id may be 0 (GUI) — used only to evaluate formulas.
// Returns a fully-computed VSpline reflecting the active state.
// l1/l2 are user-format formula strings; the returned spline carries the
// (possibly computed) length formulas.
// ---------------------------------------------------------------------------
static VSpline buildStateSpline(quint32 _id, const VPointF &p1, const VPointF &p4,
                                 QString &a1, QString &a2, QString &l1, QString &l2,
                                 const QString &targetLength, bool autoSmooth,
                                 VContainer *data)
{
    const qreal calcAngle1 = VAbstractTool::CheckFormula(_id, a1, data);
    const qreal calcAngle2 = VAbstractTool::CheckFormula(_id, a2, data);

    const bool hasTarget = !targetLength.isEmpty();

    qreal calcLength1, calcLength2;
    QString finalL1 = l1, finalL2 = l2;

    if (autoSmooth)
    {
        const QPair<qreal,qreal> hobby = hobbyHandleLengthsForSpline(p1, p4, calcAngle1, calcAngle2);
        qreal c1Px = hobby.first;
        qreal c2Px = hobby.second;

        if (hasTarget)
        {
            QString tl = targetLength;
            const qreal targetPx = qApp->toPixel(VAbstractTool::CheckFormula(_id, tl, data));
            const qreal scale = findScaleFactorForSpline(p1, p4, calcAngle1, a1, calcAngle2, a2,
                                                          c1Px, c2Px, targetPx);
            c1Px *= scale;
            c2Px *= scale;
        }
        calcLength1 = c1Px;
        calcLength2 = c2Px;
        finalL1 = QString::number(qApp->fromPixel(c1Px));
        finalL2 = QString::number(qApp->fromPixel(c2Px));
    }
    else if (hasTarget)
    {
        calcLength1 = qApp->toPixel(VAbstractTool::CheckFormula(_id, l1, data));
        QString tl = targetLength;
        const qreal targetPx = qApp->toPixel(VAbstractTool::CheckFormula(_id, tl, data));
        const qreal scale = findScaleForSpline(p1, p4, calcAngle1, calcAngle2, calcLength1, targetPx);
        calcLength1 *= scale;
        calcLength2  = calcLength1;
        finalL1 = QString::number(qApp->fromPixel(calcLength1));
        finalL2 = QString::number(qApp->fromPixel(calcLength2));
    }
    else
    {
        calcLength1 = qApp->toPixel(VAbstractTool::CheckFormula(_id, l1, data));
        calcLength2 = qApp->toPixel(VAbstractTool::CheckFormula(_id, l2, data));
    }

    return VSpline(p1, p4, calcAngle1, a1, calcAngle2, a2,
                   calcLength1, finalL1, calcLength2, finalL2);
}

// @brief VToolSpline constructor.
// @param doc dom document container.
// @param data container with variables.
// @param id object id in container.
// @param typeCreation way we create this tool.
// @param parent parent object.
VToolSpline::VToolSpline(VAbstractPattern *doc, VContainer *data, quint32 id,
                         const QString &targetLength, bool autoSmooth,
                         const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , oldPosition()
    , m_targetLength(targetLength)
    , m_autoSmooth(autoSmooth)
{
    m_sceneType = SceneObject::Spline;

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(id);

    const bool freeAngle1 = qmu::QmuTokenParser::IsSingle(spl->GetStartAngleFormula());
    const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl->GetC1LengthFormula());

    auto *controlPoint1 = new VControlPointSpline(1, SplinePointPosition::FirstPoint,
                                                  static_cast<QPointF>(spl->GetP2()),
                                                  freeAngle1, freeLength1, this);
    connect(controlPoint1, &VControlPointSpline::controlPointPositionChanged, this,
            &VToolSpline::controlPointPositionChanged);
    connect(this, &VToolSpline::setEnabledPoint, controlPoint1, &VControlPointSpline::setEnabledPoint);
    connect(controlPoint1, &VControlPointSpline::showContextMenu, this, &VToolSpline::contextMenuEvent);
    m_controlPoints.append(controlPoint1);

    const bool freeAngle2 = qmu::QmuTokenParser::IsSingle(spl->GetEndAngleFormula());
    const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl->GetC2LengthFormula());

    auto *controlPoint2 = new VControlPointSpline(1, SplinePointPosition::LastPoint,
                                                  static_cast<QPointF>(spl->GetP3()),
                                                  freeAngle2, freeLength2, this);
    connect(controlPoint2, &VControlPointSpline::controlPointPositionChanged, this,
            &VToolSpline::controlPointPositionChanged);
    connect(this, &VToolSpline::setEnabledPoint, controlPoint2, &VControlPointSpline::setEnabledPoint);
    connect(controlPoint2, &VControlPointSpline::showContextMenu, this, &VToolSpline::contextMenuEvent);
    m_controlPoints.append(controlPoint2);

    VToolSpline::refreshCtrlPoints();
    showHandles(m_piecesMode);
    ToolCreation(typeCreation);
}

// @brief setDialog set dialog when user want change tool option.
void VToolSpline::setDialog()
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogSpline> dialogTool = m_dialog.objectCast<DialogSpline>();
    SCASSERT(!dialogTool.isNull())
    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    dialogTool->SetSpline(*spl);
    dialogTool->SetAutoSmooth(m_autoSmooth);
    dialogTool->SetTargetLength(m_targetLength);
    dialogTool->setLineColor(spl->getLineColor());
    dialogTool->setLineWeight(spl->getLineWeight());
    dialogTool->setPenStyle(spl->GetPenStyle());
}

// @brief Create help create tool from GUI.
// @param dialog dialog.
// @param scene pointer to scene.
// @param doc dom document container.
// @param data container with variables.
// @return the created tool
VToolSpline* VToolSpline::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data)
{
    SCASSERT(!dialog.isNull())
    QSharedPointer<DialogSpline> dialogTool = dialog.objectCast<DialogSpline>();
    SCASSERT(!dialogTool.isNull())

    const VSpline dialogSpline = dialogTool->GetSpline();
    const QString targetLength  = dialogTool->GetTargetLength();
    const bool    autoSmooth    = dialogTool->GetAutoSmooth();

    const quint32 point1 = dialogSpline.GetP1().id();
    const quint32 point4 = dialogSpline.GetP4().id();
    QString a1 = dialogSpline.GetStartAngleFormula();
    QString a2 = dialogSpline.GetEndAngleFormula();
    QString l1 = dialogSpline.GetC1LengthFormula();
    QString l2 = dialogSpline.GetC2LengthFormula();
    const quint32 duplicate = dialogSpline.GetDuplicate();

    // Use the formula-based Create which implements the 4-state computation logic.
    auto spl = Create(0, point1, point4, a1, a2, l1, l2, duplicate,
                      dialogTool->getLineColor(), dialogTool->getPenStyle(), dialogTool->getLineWeight(),
                      scene, doc, data, Document::FullParse, Source::FromGui,
                      targetLength, autoSmooth);

    if (spl != nullptr)
    {
        spl->m_dialog = dialogTool;
    }
    return spl;
}

// @brief Create help create tool.
// @param _id tool id, 0 if tool doesn't exist yet.
// @param spline spline.
// @param scene pointer to scene.
// @param doc dom document container.
// @param data container with variables.
// @param parse parser file mode.
// @param typeCreation way we create this tool.
// @return the created tool
VToolSpline* VToolSpline::Create(const quint32 _id, VSpline *spline, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data, const Document &parse, const Source &typeCreation)
{
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
        VDrawTool::AddRecord(id, Tool::Spline, doc);
        auto _spl = new VToolSpline(doc, data, id, QString(), false, typeCreation);
        scene->addItem(_spl);
        initSplineToolConnections(scene, _spl);
        VAbstractPattern::AddTool(id, _spl);
        doc->IncrementReferens(spline->GetP1().getIdTool());
        doc->IncrementReferens(spline->GetP4().getIdTool());
        return _spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VToolSpline *VToolSpline::Create(const quint32 _id, quint32 point1, quint32 point4, QString &a1, QString &a2,
                                 QString &l1, QString &l2, quint32 duplicate, const QString &color,
                                 const QString &penStyle, const QString &lineWeight, VMainGraphicsScene *scene,
                                 VAbstractPattern *doc, VContainer *data, const Document &parse,
                                 const Source &typeCreation,
                                 const QString &targetLength, bool autoSmooth)
{
    auto p1 = data->GeometricObject<VPointF>(point1);
    auto p4 = data->GeometricObject<VPointF>(point4);

    const bool hasTarget = !targetLength.isEmpty();

    // Build the spline geometry using the shared 4-state logic
    VSpline builtSpline = buildStateSpline(_id, *p1, *p4, a1, a2, l1, l2,
                                            targetLength, autoSmooth, data);

    auto spline = new VSpline(builtSpline);
    if (duplicate > 0)
        spline->SetDuplicate(duplicate);

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
        VDrawTool::AddRecord(id, Tool::Spline, doc);
        // Disable control point dragging when auto-smooth or target is active
        const bool locked = autoSmooth || hasTarget;
        auto _spl = new VToolSpline(doc, data, id, targetLength, autoSmooth, typeCreation);
        scene->addItem(_spl);
        initSplineToolConnections(scene, _spl);
        VAbstractPattern::AddTool(id, _spl);
        doc->IncrementReferens(spline->GetP1().getIdTool());
        doc->IncrementReferens(spline->GetP4().getIdTool());

        if (locked)
        {
            // Lock control points so the user can't accidentally drag them
            for (auto *cp : _spl->m_controlPoints)
            {
                cp->setFlag(QGraphicsItem::ItemIsMovable, false);
                cp->setFlag(QGraphicsItem::ItemIsFocusable, false);
            }
        }
        return _spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
VSpline VToolSpline::getSpline() const
{
    auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    return *spline.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::setSpline(const VSpline &spl)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VSpline> spline = qSharedPointerDynamicCast<VSpline>(obj);
    *spline.data() = spl;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolSpline>(show);
}

// @brief controlPointPositionChanged handle position change of control point.
// @param splineIndex position spline in spline list.
// @param position position point in spline.
// @param pos new position.
void VToolSpline::controlPointPositionChanged(const qint32 &splineIndex, const SplinePointPosition &position,
                                             const QPointF &pos)
{
    Q_UNUSED(splineIndex)
    const QSharedPointer<VSpline> spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    const VSpline spl = correctedSpline(*spline, position, pos);

    MoveSpline *moveSpl = new MoveSpline(doc, spline.data(), spl, m_id);
    connect(moveSpl, &MoveSpline::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    qApp->getUndoStack()->push(moveSpl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);

    for (auto *point : qAsConst(m_controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemIsMovable, move);
    }
}

// @brief contextMenuEvent handle context menu events.
// @param event context menu event.
void VToolSpline::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)

    try
    {
        ContextMenu<DialogSpline>(event);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

// @brief RemoveReferens decrement value of reference.
void VToolSpline::RemoveReferens()
{
    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    doc->DecrementReferens(spl->GetP1().getIdTool());
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

// @brief SaveDialog save options into file after change in dialog.
void VToolSpline::SaveDialog(QDomElement &domElement)
{
    SCASSERT(!m_dialog.isNull())
    auto dialogTool = qobject_cast<DialogSpline*>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    // Capture the new optional settings
    m_targetLength = dialogTool->GetTargetLength();
    m_autoSmooth   = dialogTool->GetAutoSmooth();

    // Recompute the spline geometry using the active state.
    const VSpline dialogSpline = dialogTool->GetSpline();
    QString a1 = dialogSpline.GetStartAngleFormula();
    QString a2 = dialogSpline.GetEndAngleFormula();
    QString l1 = dialogSpline.GetC1LengthFormula();
    QString l2 = dialogSpline.GetC2LengthFormula();

    const auto p1 = VAbstractTool::data.GeometricObject<VPointF>(dialogSpline.GetP1().id());
    const auto p4 = VAbstractTool::data.GeometricObject<VPointF>(dialogSpline.GetP4().id());

    VSpline spl = buildStateSpline(m_id, *p1, *p4, a1, a2, l1, l2,
                                    m_targetLength, m_autoSmooth, &(VAbstractTool::data));
    spl.SetDuplicate(dialogSpline.GetDuplicate());

    m_controlPoints[0]->blockSignals(true);
    m_controlPoints[1]->blockSignals(true);

    m_controlPoints[0]->setPos(static_cast<QPointF>(spl.GetP2()));
    m_controlPoints[1]->setPos(static_cast<QPointF>(spl.GetP3()));

    m_controlPoints[0]->blockSignals(false);
    m_controlPoints[1]->blockSignals(false);

    // Lock or unlock control point dragging based on state
    const bool locked = m_autoSmooth || !m_targetLength.isEmpty();
    for (auto *cp : m_controlPoints)
    {
        cp->setFlag(QGraphicsItem::ItemIsMovable, !locked);
        cp->setFlag(QGraphicsItem::ItemIsFocusable, !locked);
    }

    SetSplineAttributes(domElement, spl);
    doc->SetAttribute(domElement, AttrColor,      dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrPenStyle,   dialogTool->getPenStyle());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    auto spl = qSharedPointerDynamicCast<VSpline>(obj);
    SCASSERT(spl.isNull() == false)
    SetSplineAttributes(tag, *spl);
}

// @brief mousePressEvent  handle mouse press events.
// @param event mouse release event.
void VToolSpline::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            if (IsMovable())
            {
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
                oldPosition = event->scenePos();
                event->accept();
            }
        }
    }
    VAbstractSpline::mousePressEvent(event);
}

// @brief mouseReleaseEvent  handle mouse release events.
// @param event mouse release event.
void VToolSpline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            if (IsMovable())
            {
                SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
            }
        }
    }
    VAbstractSpline::mouseReleaseEvent(event);
}

// @brief mouseMoveEvent  handle mouse move events.
// @param event mouse move event.
void VToolSpline::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (IsMovable())
    {
        // Don't need check if left mouse button was pressed. According to the Qt documentation "If you do receive this
        // event, you can be certain that this item also received a mouse press event, and that this item is the current
        // mouse grabber.".

        // Magic Bezier Drag Equations follow!
        // "weight" describes how the influence of the drag should be distributed
        // among the handles; 0 = front handle only, 1 = back handle only.

        const auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);
        const qreal t = spline->ParamT(oldPosition);

        if (qFloor(t) == -1)
        {
            return;
        }

        double weight;
        if (t <= 1.0 / 6.0)
        {
            weight = 0;
        }
        else if (t <= 0.5)
        {
            weight = (pow((6 * t - 1) / 2.0, 3)) / 2;
        }
        else if (t <= 5.0 / 6.0)
        {
            weight = (1 - pow((6 * (1-t) - 1) / 2.0, 3)) / 2 + 0.5;
        }
        else
        {
            weight = 1;
        }

        const QPointF delta = event->scenePos() - oldPosition;
        const QPointF offset0 = ((1-weight)/(3*t*(1-t)*(1-t))) * delta;
        const QPointF offset1 = (weight/(3*t*t*(1-t))) * delta;

        const QPointF p2 = static_cast<QPointF>(spline->GetP2()) + offset0;
        const QPointF p3 = static_cast<QPointF>(spline->GetP3()) + offset1;

        oldPosition = event->scenePos(); // Now mouse here

        VSpline spl = VSpline(spline->GetP1(), p2, p3, spline->GetP4());

        MoveSpline *moveSpl = new MoveSpline(doc, spline.data(), spl, m_id);
        connect(moveSpl, &MoveSpline::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(moveSpl);

        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
           changeFinished = false;

           const QList<QGraphicsView *> viewList = scene()->views();
           if (!viewList.isEmpty())
           {
               if (QGraphicsView *view = viewList.at(0))
               {
                   VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                   SCASSERT(currentScene)
                   const QPointF cursorPosition = currentScene->getScenePos();
                   view->ensureVisible(QRectF(cursorPosition.x()-5, cursorPosition.y()-5, 10, 10));
               }
           }
           changeFinished = true;
        }
    }
}

// @brief hoverEnterEvent handle hover enter events.
// @param event hover enter event.
void VToolSpline::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (IsMovable())
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }

    VAbstractSpline::hoverEnterEvent(event);
}

// @brief hoverLeaveEvent handle hover leave events.
// @param event hover leave event.
void VToolSpline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (IsMovable())
        {
            setCursor(QCursor());
        }
    }

    VAbstractSpline::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SetVisualization()
{
    if (!vis.isNull())
    {
        VisToolSpline *visual = qobject_cast<VisToolSpline *>(vis);
        SCASSERT(visual != nullptr)

        const QSharedPointer<VSpline> spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);
        visual->setObject1Id(spl->GetP1().id());
        visual->setObject4Id(spl->GetP4().id());
        visual->SetAngle1(spl->GetStartAngle());
        visual->SetAngle2(spl->GetEndAngle());
        visual->SetKAsm1(spl->GetKasm1());
        visual->SetKAsm2(spl->GetKasm2());
        visual->SetKCurve(spl->GetKcurve());
        visual->setLineStyle(lineTypeToPenStyle(spl->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VToolSpline::IsMovable() const
{
    const auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    return qmu::QmuTokenParser::IsSingle(spline->GetStartAngleFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetEndAngleFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetC1LengthFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetC2LengthFormula());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::refreshCtrlPoints()
{
    // Very important to disable control points. Without it the pogram can't move the curve.
    foreach (auto *point, m_controlPoints)
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    }

    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    m_controlPoints[0]->blockSignals(true);
    m_controlPoints[1]->blockSignals(true);

    {
        const bool freeAngle1 = qmu::QmuTokenParser::IsSingle(spl->GetStartAngleFormula());
        const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl->GetC1LengthFormula());

        const QPointF splinePoint =
                static_cast<QPointF>(*VAbstractTool::data.GeometricObject<VPointF>(spl->GetP1().id()));
        m_controlPoints[0]->refreshCtrlPoint(1, SplinePointPosition::FirstPoint, static_cast<QPointF>(spl->GetP2()),
                                           static_cast<QPointF>(splinePoint), freeAngle1, freeLength1);
    }

    {
        const bool freeAngle2 = qmu::QmuTokenParser::IsSingle(spl->GetEndAngleFormula());
        const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl->GetC2LengthFormula());

        const QPointF splinePoint =
                static_cast<QPointF>(*VAbstractTool::data.GeometricObject<VPointF>(spl->GetP4().id()));
        m_controlPoints[1]->refreshCtrlPoint(1, SplinePointPosition::LastPoint, static_cast<QPointF>(spl->GetP3()),
                                           static_cast<QPointF>(splinePoint), freeAngle2, freeLength2);
    }

    m_controlPoints[0]->blockSignals(false);
    m_controlPoints[1]->blockSignals(false);

    foreach (auto *point, m_controlPoints)
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SetTargetLength(const QString &value)
{
    m_targetLength = value;
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

void VToolSpline::SetAutoSmooth(bool value)
{
    m_autoSmooth = value;
    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SetSplineAttributes(QDomElement &domElement, const VSpline &spl)
{
    SCASSERT(doc != nullptr)

    doc->SetAttribute(domElement, AttrType,    ToolType);
    doc->SetAttribute(domElement, AttrPoint1,  spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint4,  spl.GetP4().id());
    doc->SetAttribute(domElement, AttrAngle1,  spl.GetStartAngleFormula());
    doc->SetAttribute(domElement, AttrAngle2,  spl.GetEndAngleFormula());
    doc->SetAttribute(domElement, AttrLength1, spl.GetC1LengthFormula());

    // In States 2/3/4, length2 is computed; we still save it for display/fallback
    doc->SetAttribute(domElement, AttrLength2, spl.GetC2LengthFormula());

    // Optional new attributes
    if (!m_targetLength.isEmpty())
        doc->SetAttribute(domElement, AttrLength, m_targetLength);
    else
        domElement.removeAttribute(AttrLength);

    if (m_autoSmooth)
        doc->SetAttribute(domElement, AttrAutoSmooth, QStringLiteral("true"));
    else
        domElement.removeAttribute(AttrAutoSmooth);

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

    if (domElement.hasAttribute(AttrKCurve))
    {
        domElement.removeAttribute(AttrKCurve);
    }

    if (domElement.hasAttribute(AttrKAsm1))
    {
        domElement.removeAttribute(AttrKAsm1);
    }

    if (domElement.hasAttribute(AttrKAsm2))
    {
        domElement.removeAttribute(AttrKAsm2);
    }
}

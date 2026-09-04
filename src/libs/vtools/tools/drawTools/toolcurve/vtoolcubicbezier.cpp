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
#include <QPen>
#include <QSharedPointer>
#include <QString>
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
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"
#include "vabstractspline.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

const QString VToolCubicBezier::ToolType = QStringLiteral("cubicBezier");

//---------------------------------------------------------------------------------------------------------------------
// Set the spline's control points from handle lengths (c1, c2) along the given
// directions, preserving the original canvas point IDs.
static void setHandlesFromLengths(VCubicBezier *spline, const QPointF &p1pt, const QPointF &p4pt,
                                  qreal angle1, qreal angle2, qreal c1, qreal c2,
                                  quint32 p2Id, quint32 p3Id)
{
    QLineF newH1(p1pt, p1pt + QPointF(c1, 0.0));
    newH1.setAngle(angle1);
    QLineF newH2(p4pt, p4pt + QPointF(c2, 0.0));
    newH2.setAngle(angle2);

    const VPointF oldP2 = spline->GetP2();
    const VPointF oldP3 = spline->GetP3();
    VPointF newP2(newH1.p2(), oldP2.name(), oldP2.mx(), oldP2.my(),
                  oldP2.getIdObject(), oldP2.getMode());
    newP2.setId(p2Id);
    VPointF newP3(newH2.p2(), oldP3.name(), oldP3.mx(), oldP3.my(),
                  oldP3.getIdObject(), oldP3.getMode());
    newP3.setId(p3Id);

    spline->SetP2(newP2);
    spline->SetP3(newP3);
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier::VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                                   bool autoSmooth, int lengthMode,
                                   const QString &targetLength,
                                   const Source &typeCreation, QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
    , m_autoSmooth(autoSmooth)
    , m_lengthMode(lengthMode)
    , m_targetLength(targetLength)
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
    dialogTool->SetLengthMode(m_lengthMode);
    dialogTool->SetTargetLength(m_targetLength);
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
    const int lengthMode = dialogTool->GetLengthMode();
    const QString targetLength = dialogTool->GetTargetLength();

    auto spl = Create(0, spline, autoSmooth, lengthMode, targetLength, scene, doc, data, Document::FullParse, Source::FromGui);

    if (spl != nullptr)
    {
        spl->m_dialog = dialogTool;
    }
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier *VToolCubicBezier::Create(const quint32 _id, VCubicBezier *spline,
                                           bool autoSmooth, int lengthMode,
                                           const QString &targetLength,
                                           VMainGraphicsScene *scene,
                                           VAbstractPattern *doc, VContainer *data,
                                           const Document &parse, const Source &typeCreation)
{
    const quint32 origP2Id = spline->GetP2().id();
    const quint32 origP3Id = spline->GetP3().id();

    // Mutual-exclusive dispatch: the three handle-computation modes (Hobby,
    // length-solver, combined tension-solver) each produce a DIFFERENT set of
    // handle lengths from the SAME inputs. Running them sequentially (Hobby
    // first, then solver on the result) would feed solver inputs that are
    // already transformed, producing wrong geometry. The if/else-if ensures
    // exactly one code-path modifies the handles per Create() call.
    {
        const QPointF p1pt = static_cast<QPointF>(spline->GetP1());
        const QPointF p4pt = static_cast<QPointF>(spline->GetP4());
        const QLineF h1(p1pt, static_cast<QPointF>(spline->GetP2()));
        const QLineF h2(p4pt, static_cast<QPointF>(spline->GetP3()));
        const qreal angle1 = h1.angle();
        const qreal angle2 = h2.angle();

        const bool hasTarget = (lengthMode > 0 && !targetLength.isEmpty());
        qreal targetPx = 0.0;
        if (hasTarget)
        {
            QString tl = targetLength;
            targetPx = qApp->toPixel(CheckFormula(_id, tl, data));
        }

        qreal c1 = h1.length();
        qreal c2 = h2.length();
        bool modified = false;

        if (autoSmooth && hasTarget && targetPx > 0.0)
        {
            // Combined: vary Hobby tension to hit the target length.
            const QPair<qreal, qreal> s = VAbstractCubicBezier::SolveHobbyTension(
                p1pt, p4pt, angle1, angle2, targetPx, lengthMode);
            c1 = s.first;
            c2 = s.second;
            modified = true;
        }
        else if (autoSmooth)
        {
            const QPair<qreal, qreal> s = VAbstractCubicBezier::HobbyHandleLengths(
                p1pt, p4pt, angle1, angle2);
            c1 = s.first;
            c2 = s.second;
            modified = true;
        }
        else if (hasTarget && targetPx > 0.0)
        {
            const QPair<qreal, qreal> s = VAbstractCubicBezier::SolveHandleLengths(
                p1pt, p4pt, angle1, angle2, h1.length(), h2.length(), targetPx, lengthMode);
            c1 = s.first;
            c2 = s.second;
            modified = true;
        }

        if (modified)
        {
            setHandlesFromLengths(spline, p1pt, p4pt, angle1, angle2, c1, c2, origP2Id, origP3Id);
        }
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
        auto _spl = new VToolCubicBezier(doc, data, id, autoSmooth, lengthMode, targetLength, typeCreation);
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
    if (m_targetLength.isEmpty())
    {
        const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
        return QString::number(qApp->fromPixel(spl->GetLength()));
    }
    return m_targetLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetTargetLength(const QString &value)
{
    m_targetLength = value;
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
int VToolCubicBezier::GetLengthMode() const
{
    return m_lengthMode;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetLengthMode(int value)
{
    m_lengthMode = value;
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
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
    m_lengthMode = domElement.attribute(AttrLengthMode, QStringLiteral("0")).toInt();
    m_targetLength = formulaIdsToNames(domElement.attribute(AttrLength, QString()),
                                       idTokenToNameMap(&(this->VAbstractTool::data)));
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
    m_lengthMode = dialogTool->GetLengthMode();
    m_targetLength = dialogTool->GetTargetLength();

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

    const QHash<QString, QString> nameToIdToken = nameToIdTokenMap(&(this->VAbstractTool::data));
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

    if (m_lengthMode > 0)
    {
        doc->SetAttribute(domElement, AttrLengthMode, m_lengthMode);
    }
    else
    {
        domElement.removeAttribute(AttrLengthMode);
    }

    // Persist the target length independently of the mode so toggling the mode
    // Off and back On does not lose the user's entered value.
    if (!m_targetLength.isEmpty())
    {
        doc->SetAttribute(domElement, AttrLength, formulaNamesToIds(m_targetLength, nameToIdToken));
    }
    else
    {
        domElement.removeAttribute(AttrLength);
    }
}

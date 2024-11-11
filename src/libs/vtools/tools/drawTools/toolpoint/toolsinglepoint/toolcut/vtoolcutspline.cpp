//  @file   vtoolcutspline.cpp
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   vtoolcutspline.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 12, 2013
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
//-----------------------------------------------------------------------------

#include "vtoolcutspline.h"

#include <QPointF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../../dialogs/tools/dialogcutspline.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/path/vistoolcutspline.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoolcut.h"

template <class T> class QSharedPointer;

const QString VToolCutSpline::ToolType   = QStringLiteral("cutSpline");
const QString VToolCutSpline::AttrSpline = QStringLiteral("spline");

//---------------------------------------------------------------------------------------------------------------------
/// @brief VToolCutSpline constructor.
/// @param doc dom document container.
/// @param data container with variables.
/// @param id object id in container.
/// @param formula string with formula length first spline.
/// @param splineId id spline in data container.
/// @param typeCreation way we create this tool.
/// @param parent parent object.
//---------------------------------------------------------------------------------------------------------------------
VToolCutSpline::VToolCutSpline(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                               QString &direction, const QString &formula, const QString &lineColor,
                               const quint32 &splineId, const Source &typeCreation, QGraphicsItem *parent)
    : VToolCut(doc, data, id, direction, formula, lineColor, splineId, parent)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setDialog set dialog when user want change tool option.
//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::setDialog()
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogCutSpline> dialogTool = m_dialog.objectCast<DialogCutSpline>();
    SCASSERT(!dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setDirection(m_direction);
    dialogTool->setFormula(formula);
    dialogTool->setLineColor(lineColor);
    dialogTool->setSplineId(curveCutId);
    dialogTool->setPointName(point->name());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Create help create tool from GUI.
/// @param dialog dialog.
/// @param scene pointer to scene.
/// @param doc dom document container.
/// @param data container with variables.
//---------------------------------------------------------------------------------------------------------------------
VToolCutSpline* VToolCutSpline::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                       VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(!dialog.isNull())
    QSharedPointer<DialogCutSpline> dialogTool = dialog.objectCast<DialogCutSpline>();
    SCASSERT(!dialogTool.isNull())
    const QString pointName = dialogTool->getPointName();
    QString direction       = dialogTool->getDirection();
    QString formula         = dialogTool->getFormula();
    const QString lineColor = dialogTool->getLineColor();
    const quint32 splineId  = dialogTool->getSplineId();
    VToolCutSpline* point = Create(0, pointName, direction, formula, lineColor, splineId, 5, 10, true,
                                   scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Create help create tool.
/// @param _id tool id, 0 if tool doesn't exist yet.
/// @param pointName point name.
/// @param formula string with formula length first spline.
/// @param lineColor color for tool
/// @param splineId id spline in data container.
/// @param mx label bias x axis.
/// @param my label bias y axis.
/// @param scene pointer to scene.
/// @param doc dom document container.
/// @param data container with variables.
/// @param parse parser file mode.
/// @param typeCreation way we create this tool.
//---------------------------------------------------------------------------------------------------------------------
VToolCutSpline* VToolCutSpline::Create(const quint32 _id, const QString &pointName, QString &direction,
                                       QString &formula, const QString &lineColor, const quint32 &splineId,
                                       qreal mx, qreal my, bool showPointName, VMainGraphicsScene *scene,
                                       VAbstractPattern *doc, VContainer *data, const Document &parse,
                                       const Source &typeCreation)
{
    const auto spl = data->GeometricObject<VAbstractCubicBezier>(splineId);
    SCASSERT(splPath != nullptr)

    qreal splLength = qApp->fromPixel(spl->GetLength());
    QString adjFormula = formula;
    if (direction == "backward")
    {
        adjFormula = QString("%1 - %2").arg(splLength).arg(formula);
    }
    const qreal length = CheckFormula(_id, adjFormula, data);

    QPointF spl1p2, spl1p3, spl2p2, spl2p3;
    QPointF point = spl->CutSpline(qApp->toPixel(length), spl1p2, spl1p3, spl2p2, spl2p3);

    quint32 id = _id;
    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    auto spline1 = QSharedPointer<VAbstractBezier>(new VSpline(spl->GetP1(), spl1p2, spl1p3, *p));
    auto spline2 = QSharedPointer<VAbstractBezier>(new VSpline(*p, spl2p2, spl2p3, spl->GetP4()));

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddSpline(spline1, NULL_ID, id);
        data->AddSpline(spline2, NULL_ID, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddSpline(spline1, NULL_ID, id);
        data->AddSpline(spline2, NULL_ID, id);

        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::CutSpline, doc);
        VToolCutSpline *point = new VToolCutSpline(doc, data, id, direction, formula, lineColor, splineId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(spl->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCutSpline>(show);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief contextMenuEvent handle context menu events.
/// @param event context menu event.
//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCutSpline>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SaveDialog save options into file after change in dialog.
//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::SaveDialog(QDomElement &domElement)
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogCutSpline> dialogTool = m_dialog.objectCast<DialogCutSpline>();
    SCASSERT(!dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,      dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrDirection, dialogTool->getDirection());
    doc->SetAttribute(domElement, AttrLength,    dialogTool->getFormula());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrSpline,    QString().setNum(dialogTool->getSplineId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolCut::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrDirection, m_direction);
    doc->SetAttribute(tag, AttrType,      ToolType);
    doc->SetAttribute(tag, AttrLength,    formula);
    doc->SetAttribute(tag, AttrLineColor, lineColor);
    doc->SetAttribute(tag, AttrSpline,    curveCutId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ReadToolAttributes(const QDomElement &domElement)
{
    m_direction = doc->GetParametrString(domElement, AttrDirection, "forward");
    formula     = doc->GetParametrString(domElement, AttrLength,    "");
    lineColor   = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    curveCutId  = doc->GetParametrUInt(domElement,   AttrSpline,    NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::SetVisualization()
{
    if (!vis.isNull())
    {
        VisToolCutSpline *visual = qobject_cast<VisToolCutSpline *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(curveCutId);
        visual->setDirection(m_direction);
        visual->setLength(qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator()));

        const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(curveCutId);
        visual->setLineStyle(lineTypeToPenStyle(curve->GetPenStyle()));

        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolCutSpline::makeToolTip() const
{
    const auto spl = VAbstractTool::data.GeometricObject<VAbstractCubicBezier>(curveCutId);

    const QString expression = qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator());
    const qreal length = Visualization::FindVal(expression, VAbstractTool::data.DataVariables());

    QPointF spl1p2, spl1p3, spl2p2, spl2p3;
    QPointF point = spl->CutSpline(qApp->toPixel(length), spl1p2, spl1p3, spl2p2, spl2p3);

    VSpline spline1 = VSpline(spl->GetP1(), spl1p2, spl1p3, VPointF(point));
    VSpline spline2 = VSpline(VPointF(point), spl2p2, spl2p3, spl->GetP4());

    const QString curveStr  = tr("Curve");
    const QString lengthStr = tr("length");

    const QString toolTip = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr> <td><b>%6:</b> %7</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%8:</b> %9</td> </tr>"
                                    "<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                                    "</table>")
            .arg(curveStr + QLatin1String("1 ") + lengthStr)
            .arg(qApp->fromPixel(spline1.GetLength()))
            .arg(UnitsToStr(qApp->patternUnit(), true))
            .arg(curveStr + QLatin1String("2 ") + lengthStr)
            .arg(qApp->fromPixel(spline2.GetLength()))
            .arg(curveStr + QLatin1String(" 1") + tr("label"))
            .arg(spline1.name())
            .arg(curveStr + QLatin1String(" 2") + tr("label"))
            .arg(spline2.name());

    return toolTip;
}

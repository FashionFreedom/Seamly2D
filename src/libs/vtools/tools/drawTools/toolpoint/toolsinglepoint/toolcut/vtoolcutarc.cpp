/***************************************************************************
 **  @file   vtoolcutarc.cpp
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
 **  @file   vtoolcutarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2014
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

#include "vtoolcutarc.h"

#include <QPointF>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <new>

#include "../../../../../dialogs/tools/dialogcutarc.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/path/vistoolcutarc.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoolcut.h"

template <class T> class QSharedPointer;

const QString VToolCutArc::ToolType = QStringLiteral("cutArc");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolCutArc constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param formula string with formula length first arc.
 * @param arcId id arc in data container.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolCutArc::VToolCutArc(VAbstractPattern *doc, VContainer *data, const quint32 &id, QString &direction,
                         const QString &formula, const QString &lineColor, const quint32 &arcId, const Source &typeCreation,
                         QGraphicsItem * parent)
    : VToolCut(doc, data, id, direction, formula, lineColor, arcId, parent)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolCutArc::setDialog()
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogCutArc> dialogTool = m_dialog.objectCast<DialogCutArc>();
    SCASSERT(!dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setDirection(m_direction);
    dialogTool->SetFormula(formula);
    dialogTool->setLineColor(lineColor);
    dialogTool->setArcId(curveCutId);
    dialogTool->SetPointName(point->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
VToolCutArc* VToolCutArc::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data)
{
    SCASSERT(!dialog.isNull())
    QSharedPointer<DialogCutArc> dialogTool = dialog.objectCast<DialogCutArc>();
    SCASSERT(!dialogTool.isNull())
    const QString pointName = dialogTool->getPointName();
    QString direction       = dialogTool->getDirection();
    QString formula         = dialogTool->GetFormula();
    const QString lineColor = dialogTool->getLineColor();
    const quint32 arcId     = dialogTool->getArcId();
    VToolCutArc* point = Create(0, pointName, direction, formula, lineColor, arcId, 5, 10, true,
                                scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param pointName point name.
 * @param formula string with formula length first arc.
 * @param lineColor color for tool
 * @param arcId id arc in data container.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolCutArc* VToolCutArc::Create(const quint32 _id, const QString &pointName, QString &direction, QString &formula,
                                 const QString &lineColor, quint32 arcId, qreal mx, qreal my, bool showPointName,
                                 VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data, const Document &parse, const Source &typeCreation)
{
    const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(arcId);
    SCASSERT(arc != nullptr)

    qreal arcLength = qApp->fromPixel(arc->GetLength());
    QString adjFormula = formula;
    if (direction == "backward")
    {
        adjFormula = QString("%1 - %2").arg(arcLength).arg(formula);
    }
    const qreal length = CheckFormula(_id, adjFormula, data);

    VArc arc1;
    VArc arc2;
    QPointF point = arc->CutArc(qApp->toPixel(length), arc1, arc2);

    quint32 id = _id;
    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    auto a1 = QSharedPointer<VArc>(new VArc(arc1));
    auto a2 = QSharedPointer<VArc>(new VArc(arc2));
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        a1->setId(VContainer::getNextId());
        a2->setId(VContainer::getNextId());
        data->AddArc(a1, a1->id(), id);
        data->AddArc(a2, a2->id(), id);
    }
    else
    {
        data->UpdateGObject(id, p);
        a1->setId(id + 1);
        a2->setId(id + 2);
        data->AddArc(a1, a1->id(), id);
        data->AddArc(a2, a2->id(), id);

        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::CutArc, doc);
        VToolCutArc *point = new VToolCutArc(doc, data, id, direction, formula, lineColor, arcId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(arc->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCutArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolCutArc::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCutArc>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolCutArc::SaveDialog(QDomElement &domElement)
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogCutArc> dialogTool = m_dialog.objectCast<DialogCutArc>();
    SCASSERT(!dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,      dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrDirection, dialogTool->getDirection());
    doc->SetAttribute(domElement, AttrLength,    dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrArc,       QString().setNum(dialogTool->getArcId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolCut::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrDirection, m_direction);
    doc->SetAttribute(tag, AttrType,      ToolType);
    doc->SetAttribute(tag, AttrLength,    formula);
    doc->SetAttribute(tag, AttrLineColor, lineColor);
    doc->SetAttribute(tag, AttrArc,       curveCutId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ReadToolAttributes(const QDomElement &domElement)
{
    m_direction = doc->GetParametrString(domElement, AttrDirection, "forward");
    formula     = doc->GetParametrString(domElement, AttrLength,    "");
    lineColor   = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    curveCutId  = doc->GetParametrUInt(domElement,   AttrArc,       NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::SetVisualization()
{
    if (!vis.isNull())
    {
        VisToolCutArc *visual = qobject_cast<VisToolCutArc *>(vis);
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
QString VToolCutArc::makeToolTip() const
{
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(curveCutId);

    const QString expression = qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator());
    const qreal length = Visualization::FindVal(expression, VAbstractTool::data.DataVariables());

    const QString arcStr        = tr("Arc");
    const QString lengthStr     = tr("length");
    const QString startAngleStr = tr("start angle");
    const QString endAngleStr   = tr("end angle");
    const QString radiusStr     = tr("radius");

    VArc ar1;
    VArc ar2;
    arc->CutArc(qApp->toPixel(length), ar1, ar2);

    auto ArcToolTip = [arcStr, lengthStr, startAngleStr, endAngleStr, radiusStr](QString toolTip, const VArc &arc,
            const QString &arcNumber)
    {
        toolTip += QString("<table style=font-size:11pt; font-weight:600>"
                           "<tr> <td><b>%10:</b> %11</td> </tr>"
                           "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                           "<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                           "<tr> <td><b>%6:</b> %7°</td> </tr>"
                           "<tr> <td><b>%8:</b> %9°</td> </tr>"
                           "</table>")
                    .arg(arcStr + arcNumber + QLatin1String(" ") + lengthStr)
                    .arg(qApp->fromPixel(arc.GetLength()))
                    .arg(UnitsToStr(qApp->patternUnit(), true))
                    .arg(arcStr + arcNumber + QLatin1String(" ") + radiusStr)
                    .arg(qApp->fromPixel(arc.GetRadius()))
                    .arg(arcStr + arcNumber + QLatin1String(" ") + startAngleStr)
                    .arg(arc.GetStartAngle())
                    .arg(arcStr + arcNumber + QLatin1String(" ") + endAngleStr)
                    .arg(arc.GetEndAngle())
                    .arg(arcStr + arcNumber + QLatin1String(" ") + tr("label"))
                    .arg(arc.name());
        return toolTip;
    };

    return ArcToolTip(ArcToolTip("<table>", ar1, "1"), ar2, "2") + QLatin1String("</table>");
}

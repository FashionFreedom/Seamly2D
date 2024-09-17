/***************************************************************************
 **  @file   vtoolarc.cpp
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
 **  @file   vtoolarc.cpp
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

#include "vtoolarc.h"

#include <QPen>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogtool.h"
#include "../../../dialogs/tools/dialogarc.h"
#include "../../../visualization/path/vistoolarc.h"
#include "../../../visualization/visualization.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
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

const QString VToolArc::ToolType = QStringLiteral("simple");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolArc constuctor.
 * @param doc dom document container
 * @param data container with variables
 * @param id object id in container
 * @param typeCreation way we create this tool.
 * @param parent parent object
 */
VToolArc::VToolArc(VAbstractPattern *doc, VContainer *data, quint32 id, const Source &typeCreation,
                   QGraphicsItem *parent)
    : VAbstractSpline(doc, data, id, parent)
{
    m_sceneType = SceneObject::Arc;

    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus

    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolArc::setDialog()
{
    SCASSERT(!m_dialog.isNull())
    QSharedPointer<DialogArc> dialogTool = m_dialog.objectCast<DialogArc>();
    SCASSERT(!dialogTool.isNull())
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    dialogTool->setArc(*arc);
    dialogTool->setCenter(arc->GetCenter().id());
    dialogTool->setF1(arc->GetFormulaF1());
    dialogTool->setF2(arc->GetFormulaF2());
    dialogTool->setRadius(arc->GetFormulaRadius());
    dialogTool->setLineColor(arc->getLineColor());
    dialogTool->setLineWeight(arc->getLineWeight());
    dialogTool->setPenStyle(arc->GetPenStyle());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool
 * @param dialog dialog options.
 * @param scene pointer to scene.
 * @param doc dom document container
 * @param data container with variables
 */
VToolArc* VToolArc::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                           VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogArc> dialogTool = dialog.objectCast<DialogArc>();
    SCASSERT(not dialogTool.isNull())
    const quint32 center     = dialogTool->getCenter();
    QString radius           = dialogTool->getRadius();
    QString f1               = dialogTool->getF1();
    QString f2               = dialogTool->getF2();
    const QString color      = dialogTool->getLineColor();
    const QString penStyle   = dialogTool->getPenStyle();
    const QString lineWeight = dialogTool->getLineWeight();
    VToolArc* point = Create(0, center, radius, f1, f2, color, penStyle, lineWeight, scene, doc, data,
                             Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param center id arc center point.
 * @param radius arc radius.
 * @param f1 start angle of arc.
 * @param f2 end angle of arc.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolArc* VToolArc::Create(const quint32 _id, const quint32 &center, QString &radius, QString &f1, QString &f2,
                           const QString &color, const QString &penStyle, const QString &lineWeight,
                           VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                           const Document &parse, const Source &typeCreation)
{
    qreal calcRadius = 0, calcF1 = 0, calcF2 = 0;

    calcRadius = qApp->toPixel(CheckFormula(_id, radius, data));

    calcF1 = CheckFormula(_id, f1, data);
    calcF2 = CheckFormula(_id, f2, data);

    const VPointF c = *data->GeometricObject<VPointF>(center);
    VArc *arc = new VArc(c, calcRadius, radius, calcF1, f1, calcF2, f2 );
    arc->setLineColor(color);
    arc->SetPenStyle(penStyle);
    arc->setLineWeight(lineWeight);
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(arc);
        data->AddArc(data->GeometricObject<VArc>(id), id);
    }
    else
    {
        data->UpdateGObject(id, arc);
        data->AddArc(data->GeometricObject<VArc>(id), id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Arc, doc);
        VToolArc *toolArc = new VToolArc(doc, data, id, typeCreation);
        scene->addItem(toolArc);
        initArcToolConnections(scene, toolArc);
        VAbstractPattern::AddTool(id, toolArc);
        doc->IncrementReferens(c.getIdTool());
        return toolArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolArc::getTagName() const
{
    return VAbstractPattern::TagArc;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolArc::CenterPointName() const
{
    return VAbstractTool::data.GetGObject(getCenter())->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolArc::getCenter() const
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    return arc->GetCenter().id();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::setCenter(const quint32 &value)
{
    if (value != NULL_ID)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);

        QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(value);
        arc->SetCenter(*point.data());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolArc::GetFormulaRadius() const
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula radius(arc->GetFormulaRadius(), getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
            arc->SetFormulaRadius(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolArc::GetFormulaF1() const
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula f1(arc->GetFormulaF1(), getData());
    f1.setCheckZero(false);
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaF1(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);

        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), arc->GetEndAngle()))// Angles can't be equal
        {
            arc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolArc::GetFormulaF2() const
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula f2(arc->GetFormulaF2(), getData());
    f2.setCheckZero(false);
    f2.setToolId(m_id);
    f2.setPostfix(degreeSymbol);
    return f2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaF2(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), arc->GetStartAngle()))// Angles can't be equal
        {
            arc->SetFormulaF2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VArc VToolArc::getArc() const
{
    auto arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    return *arc.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::setArc(const VArc &arc)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VArc> arc2 = qSharedPointerDynamicCast<VArc>(obj);
    *arc2.data() = arc;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showContextMenu handle context menu events.
 * @param event context menu event.
 * @param id tool id
 */
void VToolArc::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)

    try
    {
        ContextMenu<DialogArc>(event);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolArc::RemoveReferens()
{
    const auto arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    doc->DecrementReferens(arc->GetCenter().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolArc::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogArc> dialogTool = m_dialog.objectCast<DialogArc>();
    SCASSERT(!dialogTool.isNull())
    doc->SetAttribute(domElement, AttrType,       ToolType);
    doc->SetAttribute(domElement, AttrCenter,     QString().setNum(dialogTool->getCenter()));
    doc->SetAttribute(domElement, AttrRadius,     dialogTool->getRadius());
    doc->SetAttribute(domElement, AttrAngle1,     dialogTool->getF1());
    doc->SetAttribute(domElement, AttrAngle2,     dialogTool->getF2());
    doc->SetAttribute(domElement, AttrColor,      dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrPenStyle,   dialogTool->getPenStyle());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
    SCASSERT(arc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, arc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius, arc->GetFormulaRadius());
    doc->SetAttribute(tag, AttrAngle1, arc->GetFormulaF1());
    doc->SetAttribute(tag, AttrAngle2, arc->GetFormulaF2());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
        VisToolArc *visual = qobject_cast<VisToolArc *>(vis);
        SCASSERT(visual != nullptr)

        const VTranslateVars *trVars = qApp->translateVariables();
        visual->setObject1Id(arc->GetCenter().id());
        visual->setRadius(trVars->FormulaToUser(arc->GetFormulaRadius(), qApp->Settings()->getOsSeparator()));
        visual->setF1(trVars->FormulaToUser(arc->GetFormulaF1(), qApp->Settings()->getOsSeparator()));
        visual->setF2(trVars->FormulaToUser(arc->GetFormulaF2(), qApp->Settings()->getOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(arc->GetPenStyle()));
        visual->setLineWeight(arc->getLineWeight());
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolArc::makeToolTip() const
{
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);

    const QString toolTip = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr> <td><b>%10:</b> %11</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                                    "<tr> <td><b>%6:</b> %7°</td> </tr>"
                                    "<tr> <td><b>%8:</b> %9°</td> </tr>"
                                    "</table>")
            .arg(tr("     Length"))
            .arg(qApp->fromPixel(arc->GetLength()))
            .arg(UnitsToStr(qApp->patternUnit(), true))
            .arg(tr("     Radius"))
            .arg(qApp->fromPixel(arc->GetRadius()))
            .arg(tr("Start angle"))
            .arg(arc->GetStartAngle())
            .arg(tr("  End angle"))
            .arg(arc->GetEndAngle())
            .arg(tr("      Label"))
            .arg(arc->name());
    return toolTip;
}

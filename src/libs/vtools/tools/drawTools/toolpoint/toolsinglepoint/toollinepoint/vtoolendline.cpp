/***************************************************************************
 **  @file   vtoolendline.cpp
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
 **  @file   vtoolendline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
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

#include "vtoolendline.h"

#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <QString>
#include <new>

#include "../../../../../dialogs/tools/dialogendline.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../../visualization/line/vistoolendline.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

const QString VToolEndLine::ToolType = QStringLiteral("endLine");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolEndLine constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param formulaLength string with formula length of line.
 * @param formulaAngle formula angle of line.
 * @param basePointId id first point of line.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolEndLine::VToolEndLine(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                           const QString &lineType, const QString &lineWeight,
                           const QString &lineColor, const QString &formulaLength, const QString &formulaAngle,
                           const quint32 &basePointId, const Source &typeCreation, QGraphicsItem *parent)
    : VToolLinePoint(doc, data, id, lineType, lineWeight, lineColor, formulaLength, basePointId, 0, parent)
    , formulaAngle(formulaAngle)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolEndLine::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    m_dialog->setModal(true);
    QSharedPointer<DialogEndLine> dialogTool = m_dialog.objectCast<DialogEndLine>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->setLineColor(lineColor);
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
VToolEndLine* VToolEndLine::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                   VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogEndLine> dialogTool = dialog.objectCast<DialogEndLine>();
    SCASSERT(not dialogTool.isNull())
    const QString pointName   = dialogTool->getPointName();
    const QString lineColor   = dialogTool->getLineColor();
    const QString lineType    = dialogTool->getLineType();
    const QString lineWeight  = dialogTool->getLineWeight();
    QString formulaLength     = dialogTool->GetFormula();
    QString formulaAngle      = dialogTool->GetAngle();
    const quint32 basePointId = dialogTool->GetBasePointId();

    VToolEndLine *point = Create(0, pointName, lineType, lineWeight, lineColor, formulaLength, formulaAngle,
                                 basePointId, 5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
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
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param formulaLength string with formula length of line.
 * @param formulaAngle formula angle of line.
 * @param basePointId id first point of line.
 * @param mx label bias x axis.
 * @param my label bias y axis.
 * @param showPointName show/hide point name text
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 * @return the created tool
 */
VToolEndLine* VToolEndLine::Create(const quint32 _id, const QString &pointName,
                                   const QString &lineType, const QString &lineWeight,
                                   const QString &lineColor, QString &formulaLength, QString &formulaAngle,
                                   quint32 basePointId, qreal mx, qreal my, bool showPointName,
                                   VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                   const Document &parse,
                                   const Source &typeCreation)
{
    const QSharedPointer<VPointF> basePoint = data->GeometricObject<VPointF>(basePointId);
    QLineF line = QLineF(static_cast<QPointF>(*basePoint), QPointF(basePoint->x()+100, basePoint->y()));

    line.setAngle(CheckFormula(_id, formulaAngle, data)); //First set angle.
    line.setLength(qApp->toPixel(CheckFormula(_id, formulaLength, data)));
    quint32 id = _id;
    VPointF *p = new VPointF(line.p2(), pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
        data->AddLine(basePointId, id);
    }
    else
    {
        data->UpdateGObject(id, p);
        data->AddLine(basePointId, id);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::EndLine, doc);
        VToolEndLine *point = new VToolEndLine(doc, data, id, lineType, lineWeight, lineColor, formulaLength, formulaAngle,
                                               basePointId, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(basePoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolEndLine::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogEndLine>(event, id);
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
void VToolEndLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogEndLine> dialogTool = m_dialog.objectCast<DialogEndLine>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName,       dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrLineType,   dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight, dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,  dialogTool->getLineColor());
    doc->SetAttribute(domElement, AttrLength,     dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrAngle,      dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->GetBasePointId()));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEndLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEndLine::ReadToolAttributes(const QDomElement &domElement)
{
    m_lineType    = doc->GetParametrString(domElement, AttrLineType, LineTypeSolidLine);
    m_lineWeight  = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor     = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    formulaLength = doc->GetParametrString(domElement, AttrLength, "");
    basePointId   = doc->GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
    formulaAngle  = doc->GetParametrString(domElement, AttrAngle, "");
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEndLine::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolEndLine *visual = qobject_cast<VisToolEndLine *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(basePointId);
        visual->setLength(qApp->translateVariables()->FormulaToUser(formulaLength, qApp->Settings()->getOsSeparator()));
        visual->SetAngle(qApp->translateVariables()->FormulaToUser(formulaAngle, qApp->Settings()->getOsSeparator()));
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula VToolEndLine::GetFormulaAngle() const
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEndLine::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEndLine::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolEndLine>(show);
}

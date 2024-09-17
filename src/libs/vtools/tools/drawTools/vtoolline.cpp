/******************************************************************************
 *   @file   vtoolline.cpp
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
 **
 **  @file   vtoolline.cpp
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

#include "vtoolline.h"

#include <QKeyEvent>
#include <QLineF>
#include <QMessageLogger>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <QtDebug>
#include <new>

#include "../../dialogs/tools/dialogline.h"
#include "../../dialogs/tools/dialogtool.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolline.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vabstracttool.h"
#include "vdrawtool.h"

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolLine constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param firstPoint id first line point.
 * @param secondPoint id second line point.
 * @param lineType line type.
 * @param typeCreation way we create this tool.
 * @param parent parent object.
 */
VToolLine::VToolLine(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 firstPoint, quint32 secondPoint,
                     const QString &lineType, const QString &lineWeight, const QString &lineColor,
                     const Source &typeCreation, QGraphicsItem *parent)
    : VDrawTool(doc, data, id)
    , QGraphicsLineItem(parent)
    , firstPoint(firstPoint)
    , secondPoint(secondPoint)
    , lineColor(lineColor)
    , m_isHovered(false)
{
    this->m_lineType = lineType;
    this->m_lineWeight = lineWeight;
    //Line
    RefreshGeometry();
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus
    this->setAcceptHoverEvents(true);

    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolLine::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogLine> dialogTool = m_dialog.objectCast<DialogLine>();
    SCASSERT(not dialogTool.isNull())
    dialogTool->setFirstPoint(firstPoint);
    dialogTool->setSecondPoint(secondPoint);
    dialogTool->setLineName();
    dialogTool->setLineType(m_lineType);
    dialogTool->setLineWeight(m_lineWeight);
    dialogTool->setLineColor(lineColor);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
VToolLine *VToolLine::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                             VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<DialogLine> dialogTool = dialog.objectCast<DialogLine>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstPoint  = dialogTool->getFirstPoint();
    const quint32 secondPoint = dialogTool->getSecondPoint();
    const QString lineType    = dialogTool->getLineType();
    const QString lineWeight  = dialogTool->getLineWeight();
    const QString lineColor   = dialogTool->getLineColor();

    VToolLine *line = Create(0, firstPoint, secondPoint, lineType, lineWeight, lineColor,  scene, doc, data, Document::FullParse,
                             Source::FromGui);
    if (line != nullptr)
    {
        line->m_dialog = dialogTool;
    }
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param firstPoint id first line point.
 * @param secondPoint id second line point.
 * @param lineType line type.
 * @param lineWeight line weight.
 * @param lineColor line color.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @param parse parser file mode.
 * @param typeCreation way we create this tool.
 */
VToolLine * VToolLine::Create(const quint32 &_id, const quint32 &firstPoint, const quint32 &secondPoint,
                              const QString &lineType, const QString &lineWeight, const QString &lineColor,
                              VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                              const Document &parse, const Source &typeCreation)
{
    SCASSERT(scene != nullptr)
    SCASSERT(doc != nullptr)
    SCASSERT(data != nullptr)
    quint32 id = _id;
    if (typeCreation == Source::FromGui)
    {
        id = VContainer::getNextId();
        data->AddLine(firstPoint, secondPoint);
    }
    else
    {
        VContainer::UpdateId(id);
        data->AddLine(firstPoint, secondPoint);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::Line, doc);
        VToolLine *line = new VToolLine(doc, data, id, firstPoint, secondPoint, lineType, lineWeight, lineColor, typeCreation);
        scene->addItem(line);
        InitDrawToolConnections(scene, line);
        connect(scene, &VMainGraphicsScene::EnableLineItemSelection, line, &VToolLine::AllowSelecting);
        connect(scene, &VMainGraphicsScene::EnableLineItemHover, line, &VToolLine::AllowHover);
        VAbstractPattern::AddTool(id, line);

        const QSharedPointer<VPointF> first = data->GeometricObject<VPointF>(firstPoint);
        const QSharedPointer<VPointF> second = data->GeometricObject<VPointF>(secondPoint);

        doc->IncrementReferens(first->getIdTool());
        doc->IncrementReferens(second->getIdTool());
        return line;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolLine::getTagName() const
{
    return VAbstractPattern::TagLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    //const qreal width = scaleWidth(m_isHovered ? widthMainLine : widthHairLine, sceneScale(scene()));

    const qreal weight = ToPixel(doc->useGroupLineWeight(m_id, m_lineWeight).toDouble(), Unit::Mm);
    const qreal width  = scaleWidth(m_isHovered ? weight + 4 : weight, sceneScale(scene()));

    setPen(QPen(correctColor(this, doc->useGroupColor(m_id, lineColor)),
           width, lineTypeToPenStyle(doc->useGroupLineType(m_id, m_lineType))));

    QGraphicsLineItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolLine::FirstPointName() const
{
    return VAbstractTool::data.GetGObject(firstPoint)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolLine::SecondPointName() const
{
    return VAbstractTool::data.GetGObject(secondPoint)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolLine::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool highlight tool.
 * @param id object id in container
 * @param enable enable or disable highlight.
 */
void VToolLine::ShowTool(quint32 id, bool enable)
{
    ShowItem(this, id, enable);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::Disable(bool disable, const QString &draftBlockName)
{
    const bool enabled = !CorrectDisable(disable, draftBlockName);
    this->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolLine::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogLine>(event);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VToolLine::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    QSharedPointer<VGObject> obj = QSharedPointer<VGObject> ();
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void VToolLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    setToolTip(makeToolTip());
    QGraphicsLineItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void VToolLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (vis.isNull())
    {
        m_isHovered = false;
        QGraphicsLineItem::hoverLeaveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolLine::RemoveReferens()
{
    const auto p1 = VAbstractTool::data.GetGObject(firstPoint);
    const auto p2 = VAbstractTool::data.GetGObject(secondPoint);

    doc->DecrementReferens(p1->getIdTool());
    doc->DecrementReferens(p2->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange handle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
QVariant VToolLine::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return QGraphicsItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key realse events.
 * @param event key realse event.
 */
void VToolLine::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                deleteTool();
            }
            catch(const VExceptionToolWasDeleted &error)
            {
                Q_UNUSED(error)
                return;//Leave this method immediately!!!
            }
            break;
        default:
            break;
    }
    QGraphicsItem::keyReleaseEvent ( event );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<DialogLine> dialogTool = m_dialog.objectCast<DialogLine>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrFirstPoint,  QString().setNum(dialogTool->getFirstPoint()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->getSecondPoint()));
    doc->SetAttribute(domElement, AttrLineType,    dialogTool->getLineType());
    doc->SetAttribute(domElement, AttrLineWeight,  dialogTool->getLineWeight());
    doc->SetAttribute(domElement, AttrLineColor,   dialogTool->getLineColor());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrFirstPoint,  firstPoint);
    doc->SetAttribute(tag, AttrSecondPoint, secondPoint);
    doc->SetAttribute(tag, AttrLineType,    m_lineType);
    doc->SetAttribute(tag, AttrLineWeight,  m_lineWeight);
    doc->SetAttribute(tag, AttrLineColor,   lineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ReadToolAttributes(const QDomElement &domElement)
{
    firstPoint   = doc->GetParametrUInt(domElement,   AttrFirstPoint,  NULL_ID_STR);
    secondPoint  = doc->GetParametrUInt(domElement,   AttrSecondPoint, NULL_ID_STR);
    m_lineType   = doc->GetParametrString(domElement, AttrLineType,    LineTypeSolidLine);
    m_lineWeight = doc->GetParametrString(domElement, AttrLineWeight,  "0.35");
    lineColor    = doc->GetParametrString(domElement, AttrLineColor,   ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetVisualization()
{
    if (not vis.isNull())
    {
        VisToolLine *visual = qobject_cast<VisToolLine *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstPoint);
        visual->setPoint2Id(secondPoint);
        visual->setLineStyle(lineTypeToPenStyle(m_lineType));
        visual->setLineWeight(m_lineWeight);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolLine::makeToolTip() const
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(firstPoint);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(secondPoint);

    const QLineF line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));

    const QString toolTip = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "<tr> <td><b>%4:</b> %5°</td> </tr>"
                                    "</table>")
            .arg(tr("Length"))
            .arg(qApp->fromPixel(line.length()))
            .arg(UnitsToStr(qApp->patternUnit(), true))
            .arg(tr("Angle"))
            .arg(line.angle());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolLine::GetSecondPoint() const
{
    return secondPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetSecondPoint(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPoint = value;

        QSharedPointer<VGObject> obj;//We don't have object for line in data container. Just will send empty object.
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ShowVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<VisToolLine>();
            SetVisualization();
        }
        else
        {
            if (VisToolLine *visual = qobject_cast<VisToolLine *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis;
        hoverLeaveEvent(nullptr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::setLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj;//We don't have object for line in data container. Just will send empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::setLineWeight(const QString &value)
{
    m_lineWeight = value;

    QSharedPointer<VGObject> obj;//We don't have object for line in data container. Just will send empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VToolLine::getLineColor() const
{
    return lineColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::setLineColor(const QString &value)
{
    lineColor = value;

    QSharedPointer<VGObject> obj;//We don't have object for line in data container. Just will send empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VToolLine::GetFirstPoint() const
{
    return firstPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetFirstPoint(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstPoint = value;

        QSharedPointer<VGObject> obj;//We don't have object for line in data container. Just will send empty object.
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RefreshGeometry refresh item on scene.
 */
void VToolLine::RefreshGeometry()
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(firstPoint);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(secondPoint);
    this->setLine(QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)));
}

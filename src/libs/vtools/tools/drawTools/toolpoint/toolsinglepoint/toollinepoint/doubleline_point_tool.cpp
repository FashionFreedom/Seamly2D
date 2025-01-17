/**************************************************************************
 **
 **  @file   doubleline_point_tool.h
 **  @author Douglas S Caskey
 **  @date   7.19.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
 *************************************************************************/

#include "doubleline_point_tool.h"

#include "../vtoolsinglepoint.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "../../../vdrawtool.h"
#include "../../../../vabstracttool.h"

#include <QColor>
#include <QGraphicsLineItem>
#include <QLineF>
#include <QPen>
#include <QPoint>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DoubleLinePointTool constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param typeLine line type.
 * @param lineColor line color.
 * @param formula string with length formula.
 * @param firstPointId id first point.
 * @param secondPointId id second point.
 * @param parent parent object.
 */
DoubleLinePointTool::DoubleLinePointTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                         const QString &typeLine, const QString &lineWeight, const QString &lineColor,
                                         const quint32 &firstPointId, const quint32 &secondPointId,
                                         QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(lineColor), parent)
    , firstPointId(firstPointId)
    , secondPointId(secondPointId)
    , firstLine(nullptr)
    , secondLine(nullptr)
    , lineColor(lineColor)
{
    this->m_lineType   = typeLine;
    this->m_lineWeight = lineWeight;

    setPointColor(lineColor);

    Q_ASSERT_X(firstPointId != 0, Q_FUNC_INFO, "firstPointId == 0"); //-V654 //-V712
    Q_ASSERT_X(secondPointId != 0, Q_FUNC_INFO, "secondPointId == 0"); //-V654 //-V712
    QPointF point1 = static_cast<QPointF>(*data->GeometricObject<VPointF>(firstPointId));
    QPointF point2 = static_cast<QPointF>(*data->GeometricObject<VPointF>(secondPointId));
    QPointF point3 = static_cast<QPointF>(*data->GeometricObject<VPointF>(id));

    firstLine = new VScaledLine(QLineF(point1 - point3, QPointF()), this);
    firstLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    firstLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);

    secondLine = new VScaledLine(QLineF(point2 - point3, QPointF()), this);
    secondLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    secondLine->setFlag(QGraphicsItem::ItemStacksBehindParent, true);
}

//---------------------------------------------------------------------------------------------------------------------
DoubleLinePointTool::~DoubleLinePointTool()
{
    delete firstLine;
    delete secondLine;
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen = firstLine->pen();
    pen.setColor(correctColor(this, lineColor));
    pen.setStyle(lineTypeToPenStyle(m_lineType));
    pen.setWidthF(ToPixel(m_lineWeight.toDouble(), Unit::Mm)); 

    firstLine->setPen(pen);
    secondLine->setPen(pen);

    VToolSinglePoint::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RefreshGeometry  refresh item on scene.
 */
void DoubleLinePointTool::RefreshGeometry()
{
    setPointColor(lineColor);
    VToolSinglePoint::refreshPointGeometry(*VDrawTool::data.GeometricObject<VPointF>(m_id));

    QPointF point1 = static_cast<QPointF>(*VDrawTool::data.GeometricObject<VPointF>(firstPointId));
    QPointF point2 = static_cast<QPointF>(*VDrawTool::data.GeometricObject<VPointF>(secondPointId));
    QPointF point3 = static_cast<QPointF>(*VDrawTool::data.GeometricObject<VPointF>(m_id));

    firstLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    firstLine->setLine(QLineF(point1 - point3, QPointF()));

    secondLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    secondLine->setLine(QLineF(point2 - point3, QPointF()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void DoubleLinePointTool::RemoveReferens()
{
    const auto point1 = VAbstractTool::data.GetGObject(firstPointId);
    const auto point2 = VAbstractTool::data.GetGObject(secondPointId);
    doc->DecrementReferens(point1->getIdTool());
    doc->DecrementReferens(point2->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrLineType,   m_lineType);
    doc->SetAttribute(tag, AttrLineWeight, m_lineWeight);
    doc->SetAttribute(tag, AttrLineColor,  lineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    firstLine->setBasicWidth(ToPixel(m_lineWeight.toDouble()+1, Unit::Mm));
    secondLine->setBasicWidth(ToPixel(m_lineWeight.toDouble()+1, Unit::Mm));
    VToolSinglePoint::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    firstLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    secondLine->setBasicWidth(ToPixel(m_lineWeight.toDouble(), Unit::Mm));
    VToolSinglePoint::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
QString DoubleLinePointTool::makeToolTip() const
{
    const QSharedPointer<VPointF> point1 = VAbstractTool::data.GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> point2 = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> point3 = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF line1(static_cast<QPointF>(*point1), static_cast<QPointF>(*point3));
    const QLineF line2(static_cast<QPointF>(*point2), static_cast<QPointF>(*point3));

    const QString toolTip = QString("<table>"
                                        "<tr>"
                                            "<td align ='right'><b>%9: </b></td>" // Tool name
                                            "<td align ='left'>Intersect XY</td>"
                                        "</tr>"
                                        "<tr>"
                                            "<td align ='right'><b>%1: </b></td>" // Point Name
                                            "<td align ='left'>%5</td>"
                                        "</tr>"
                                        "<tr>"
                                            "<td colspan ='2'><hr></td>"          // Divider
                                        "</tr>"
                                        "<tr>"
                                            "<td align ='right'><b>%1: </b></td>" // Line 1 name
                                            "<td align ='left'>Line_%3_%5</td>"
                                        "</tr>"
                                        "<tr>"
                                            "<td align ='right'><b>%2: </b></td>" // Line 1 length
                                            "<td align ='left'>%6 %8</td>"
                                        "</tr>"
                                        "<tr>"
                                            "<td colspan ='2'><hr></td>"          // Divider
                                        "</tr>"
                                        "<tr>"
                                            "<td align ='right'><b>%1: </b></td>" // Line 2 name
                                            "<td align ='left'>Line_%4_%5</td>"
                                        "</tr>"
                                        "<tr>"
                                            "<td align ='right'><b>%2: </b></td>" // Line 2 length
                                            "<td align ='left'>%7 %8</td>"
                                        "</tr>"
                                    "</table>")
                                    .arg(tr("Name"))                              //1
                                    .arg(tr("Length"))                            //2
                                    .arg(point1->name())                          //3
                                    .arg(point2->name())                          //4
                                    .arg(point3->name())                          //5
                                    .arg(qApp->fromPixel(line1.length()))         //6
                                    .arg(qApp->fromPixel(line2.length()))         //7
                                    .arg(UnitsToStr(qApp->patternUnit(), true))   //8
                                    .arg(tr("Tool"));                             //9

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::Disable(bool disable, const QString &draftBlockName)
{
    VToolSinglePoint::Disable(disable, draftBlockName);
    firstLine->setEnabled(isEnabled());
    secondLine->setEnabled(isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data from file.
 */
void DoubleLinePointTool::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
QString DoubleLinePointTool::getLineColor() const
{
    return lineColor;
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::setLineColor(const QString &value)
{
    lineColor = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DoubleLinePointTool::getFirstPointId() const
{
    return firstPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::setFirstPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DoubleLinePointTool::getSecondPointId() const
{
    return secondPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void DoubleLinePointTool::setSecondPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DoubleLinePointTool::point1Name() const
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString DoubleLinePointTool::point2Name() const
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

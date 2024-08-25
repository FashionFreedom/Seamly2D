//  @file   vabstractspline.cpp
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
 **  @file   vabstractspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 3, 2014
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

#include "vabstractspline.h"

#include <QColor>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QLineF>
#include <QPen>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../../../visualization/line/visline.h"
#include "../../vabstracttool.h"
#include "../vdrawtool.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractSpline::VAbstractSpline(VAbstractPattern *doc, VContainer *data, quint32 id, QGraphicsItem *parent)
    : VDrawTool(doc, data, id)
    , QGraphicsPathItem(parent)
    , m_controlPoints()
    , m_sceneType(SceneObject::Unknown)
    , m_isHovered(false)
    , m_piecesMode(qApp->Settings()->getShowControlPoints())
{
    initDefShape();
    setAcceptHoverEvents(true);
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VAbstractSpline::shape() const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    const QVector<QPointF> points = curve->getPoints();

    QPainterPath path;
    for (qint32 i = 0; i < points.count()-1; ++i)
    {
        path.moveTo(points.at(i));
        path.lineTo(points.at(i+1));
    }

    if (m_isHovered || m_piecesMode)
    {
        path.addPath(VAbstractCurve::ShowDirection(curve->DirectionArrows(),
                                                   scaleWidth(VAbstractCurve::lengthCurveDirectionArrow,
                                                              sceneScale(scene()))));
    }
    path.setFillRule(Qt::WindingFill);
    return itemShapeFromPath(path, pen());
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    const qreal weight = ToPixel(doc->useGroupLineWeight(m_id, curve->getLineWeight()).toDouble(), Unit::Mm);
    const qreal width  = scaleWidth(m_isHovered ? weight + 4 : weight, sceneScale(scene()));

    setPen(QPen(correctColor(this, doc->useGroupColor(m_id, curve->getLineColor())), width,
           lineTypeToPenStyle(doc->useGroupLineType(m_id, curve->GetPenStyle())), Qt::RoundCap));

    auto paintSpline = [this, curve](QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
    {
        if (m_isHovered || m_piecesMode)
        {
            painter->save();

            QPen arrowPen(pen());
            arrowPen.setStyle(Qt::SolidLine);
            painter->setPen(arrowPen);
            painter->setBrush(brush());
            painter->drawPath(VAbstractCurve::ShowDirection(curve->DirectionArrows(),
                                                            scaleWidth(VAbstractCurve::lengthCurveDirectionArrow,
                                                            sceneScale(scene()))));
            painter->restore();
        }

        QGraphicsPathItem::paint(painter, option, widget);
    };

    paintSpline(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractSpline::getTagName() const
{
    return VAbstractPattern::TagSpline;
}

// @brief FullUpdateFromFile update tool data form file.
void VAbstractSpline::FullUpdateFromFile()
{
    ReadAttributes();
    refreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::Disable(bool disable, const QString &draftBlockName)
{
    const bool enabled = !CorrectDisable(disable, draftBlockName);
    this->setEnabled(enabled);
    emit setEnabledPoint(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::piecesMode(bool mode)
{
    m_piecesMode = mode;
    refreshGeometry();
    showHandles(m_piecesMode);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);

    foreach (auto *point, m_controlPoints)
    {
        point->setAcceptHoverEvents(enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);

    foreach (auto *point, m_controlPoints)
    {
        point->setAcceptHoverEvents(enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractSpline::makeToolTip() const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);

    const QString toolTip = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr> <td><b>  %4:</b> %5</td> </tr>"
                                    "<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                                    "</table>")
                                    .arg(tr("Length"))
                                    .arg(qApp->fromPixel(curve->GetLength()))
                                    .arg(UnitsToStr(qApp->patternUnit(), true))
                                    .arg(tr("Name"))
                                    .arg(curve->name());

    return toolTip;
}

// @brief ShowTool highlight tool.
// @param id object id in container
// @param enable enable or disable highlight.
void VAbstractSpline::ShowTool(quint32 id, bool enable)
{
    ShowItem(this, id, enable);
}

// @brief refreshGeometry  refresh item on scene.
void VAbstractSpline::refreshGeometry()
{
    initDefShape();
    refreshCtrlPoints();
    SetVisualization();
}

// @brief hoverEnterEvent handle hover enter events.
// @param event hover enter event.
void VAbstractSpline::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    setToolTip(makeToolTip());
    QGraphicsPathItem::hoverEnterEvent(event);
}

// @brief hoverLeaveEvent handle hover leave events.
// @param event hover leave event.
void VAbstractSpline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    QGraphicsPathItem::hoverLeaveEvent(event);
}

// @brief itemChange hadle item change.
// @param change change.
// @param value value.
// @return value.
QVariant VAbstractSpline::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return QGraphicsPathItem::itemChange(change, value);
}

// @brief keyReleaseEvent handle key release events.
// @param event key release event.
void VAbstractSpline::keyReleaseEvent(QKeyEvent *event)
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
    QGraphicsPathItem::keyReleaseEvent ( event );
}

// @brief mousePressEvent  handle mouse press events.
// @param event mouse release event.
void VAbstractSpline::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for non selectable item first need to call standard mousePressEvent then accept event
    QGraphicsPathItem::mousePressEvent(event);

    // Somehow clicking on non selectable object does not clear previous selections.
    if (!(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    event->accept();// Special for non selectable item first need to call standard mousePressEvent then accept event
}

// @brief mouseReleaseEvent  handle mouse release events.
// @param event mouse release event.
void VAbstractSpline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit chosenTool(m_id, m_sceneType);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::ReadToolAttributes(const QDomElement &domElement)
{
    Q_UNUSED(domElement)
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    const QSharedPointer<VAbstractCurve> curve = qSharedPointerCast<VAbstractCurve>(obj);
    doc->SetAttribute(tag, AttrColor,      curve->getLineColor());
    doc->SetAttribute(tag, AttrPenStyle,   curve->GetPenStyle());
    doc->SetAttribute(tag, AttrLineWeight, curve->getLineWeight());
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::refreshCtrlPoints()
{
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    showContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
VSpline VAbstractSpline::correctedSpline(const VSpline &spline, const SplinePointPosition &position,
                                         const QPointF &pos) const
{
    VSpline spl;
    if (position == SplinePointPosition::FirstPoint)
    {
        QLineF line(static_cast<QPointF>(spline.GetP1()), pos);
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line.setAngle(VisLine::CorrectAngle(line.angle()));
        }

        qreal newAngle1 = line.angle();
        QString newAngle1F = QString().setNum(newAngle1);

        qreal newLength1 = line.length();
        QString newLength1F = QString().setNum(qApp->fromPixel(newLength1));

        if (!qmu::QmuTokenParser::IsSingle(spline.GetStartAngleFormula()))
        {
            newAngle1 = spline.GetStartAngle();
            newAngle1F = spline.GetStartAngleFormula();
        }

        if (!qmu::QmuTokenParser::IsSingle(spline.GetC1LengthFormula()))
        {
            newLength1 = spline.GetC1Length();
            newLength1F = spline.GetC1LengthFormula();
        }

        spl = VSpline(spline.GetP1(), spline.GetP4(), newAngle1, newAngle1F, spline.GetEndAngle(),
                      spline.GetEndAngleFormula(), newLength1, newLength1F, spline.GetC2Length(),
                      spline.GetC2LengthFormula());
    }
    else
    {
        QLineF line(static_cast<QPointF>(spline.GetP4()), pos);
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line.setAngle(VisLine::CorrectAngle(line.angle()));
        }

        qreal newAngle2 = line.angle();
        QString newAngle2F = QString().setNum(newAngle2);

        qreal newLength2 = line.length();
        QString newLength2F = QString().setNum(qApp->fromPixel(newLength2));

        if (!qmu::QmuTokenParser::IsSingle(spline.GetEndAngleFormula()))
        {
            newAngle2 = spline.GetEndAngle();
            newAngle2F = spline.GetEndAngleFormula();
        }

        if (!qmu::QmuTokenParser::IsSingle(spline.GetC2LengthFormula()))
        {
            newLength2 = spline.GetC2Length();
            newLength2F = spline.GetC2LengthFormula();
        }
        spl = VSpline(spline.GetP1(), spline.GetP4(), spline.GetStartAngle(), spline.GetStartAngleFormula(),
                      newAngle2, newAngle2F, spline.GetC1Length(), spline.GetC1LengthFormula(),
                      newLength2, newLength2F);
    }

    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::initDefShape()
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    this->setPath(curve->GetPath());
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::showHandles(bool show)
{
    for (int i = 0; i < m_controlPoints.size(); ++i)
    {
        m_controlPoints.at(i)->setVisible(show);
    }
    update();
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractSpline::getLineColor() const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    return curve->getLineColor();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::setLineColor(const QString &value)
{
    QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->setLineColor(value);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractSpline::GetPenStyle() const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    return curve->GetPenStyle();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::SetPenStyle(const QString &value)
{
    QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetPenStyle(value);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);
    SaveOption(obj);
}

// @brief getLineWeight return line weight of the spline
// @return line weight
QString VAbstractSpline::getLineWeight() const
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    return curve->getLineWeight();
}

// @brief setLineWeight set line weight of the spline
// @param value line weight
void VAbstractSpline::setLineWeight(const QString &value)
{
    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->setLineWeight(value);
    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(curve);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstractSpline::name() const
{
    return ObjectName<VAbstractCurve>(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractSpline::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

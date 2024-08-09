/***************************************************************************
 **  @file   intersect_circles_tool.cpp
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
 **  @file   vtoolpointofintersectioncircles.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
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

#include "intersect_circles_tool.h"

#include "vtoolsinglepoint.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vdrawtool.h"
#include "../../../vabstracttool.h"
#include "../../../../dialogs/tools/intersect_circles_dialog.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/visualization.h"
#include "../../../../visualization/line/intersect_circles_visual.h"

#include <QMessageBox>
#include <QSharedPointer>
#include <QString>
#include <new>

template <class T> class QSharedPointer;

const QString IntersectCirclesTool::ToolType = QStringLiteral("pointOfIntersectionCircles");

//---------------------------------------------------------------------------------------------------------------------
IntersectCirclesTool::IntersectCirclesTool(VAbstractPattern *doc, VContainer *data,
                                                                 const quint32 &id,
                                                                 quint32 firstCircleCenterId,
                                                                 quint32 secondCircleCenterId,
                                                                 const QString &firstCircleRadius,
                                                                 const QString &secondCircleRadius,
                                                                 CrossCirclesPoint crossPoint,
                                                                 const Source &typeCreation, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , firstCircleCenterId(firstCircleCenterId)
    , secondCircleCenterId(secondCircleCenterId)
    , firstCircleRadius(firstCircleRadius)
    , secondCircleRadius(secondCircleRadius)
    , crossPoint(crossPoint)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectCirclesDialog> dialogTool =
            m_dialog.objectCast<IntersectCirclesDialog>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetFirstCircleCenterId(firstCircleCenterId);
    dialogTool->SetSecondCircleCenterId(secondCircleCenterId);
    dialogTool->SetFirstCircleRadius(firstCircleRadius);
    dialogTool->SetSecondCircleRadius(secondCircleRadius);
    dialogTool->setCirclesCrossPoint(crossPoint);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCirclesTool *IntersectCirclesTool::Create(QSharedPointer<DialogTool> dialog,
                                                                         VMainGraphicsScene *scene,
                                                                         VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<IntersectCirclesDialog> dialogTool =
            dialog.objectCast<IntersectCirclesDialog>();
    SCASSERT(not dialogTool.isNull())
    const quint32 firstCircleCenterId = dialogTool->GetFirstCircleCenterId();
    const quint32 secondCircleCenterId = dialogTool->GetSecondCircleCenterId();
    QString firstCircleRadius = dialogTool->GetFirstCircleRadius();
    QString secondCircleRadius = dialogTool->GetSecondCircleRadius();
    const CrossCirclesPoint pType = dialogTool->GetCrossCirclesPoint();
    const QString pointName = dialogTool->getPointName();
    IntersectCirclesTool *point = Create(0, pointName, firstCircleCenterId, secondCircleCenterId,
                                                    firstCircleRadius, secondCircleRadius, pType, 5, 10, true, scene, doc,
                                                    data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCirclesTool *IntersectCirclesTool::Create(const quint32 _id, const QString &pointName,
                                                                         quint32 firstCircleCenterId,
                                                                         quint32 secondCircleCenterId,
                                                                         QString &firstCircleRadius,
                                                                         QString &secondCircleRadius,
                                                                         CrossCirclesPoint crossPoint,
                                                                         qreal mx, qreal my,
                                                                         bool showPointName,
                                                                         VMainGraphicsScene *scene,
                                                                         VAbstractPattern *doc, VContainer *data,
                                                                         const Document &parse,
                                                                         const Source &typeCreation)
{
    const qreal calcC1Radius = qApp->toPixel(CheckFormula(_id, firstCircleRadius, data));
    const qreal calcC2Radius = qApp->toPixel(CheckFormula(_id, secondCircleRadius, data));

    const VPointF c1Point = *data->GeometricObject<VPointF>(firstCircleCenterId);
    const VPointF c2Point = *data->GeometricObject<VPointF>(secondCircleCenterId);

    const QPointF point = FindPoint(static_cast<QPointF>(c1Point), static_cast<QPointF>(c2Point), calcC1Radius,
                                    calcC2Radius, crossPoint);
    if (point == QPointF())
    {
        const QString msg = tr("<b><big>Can't find intersection point %1 of Circles</big></b><br>"
                               "Using origin point as a place holder until pattern is corrected.")
                               .arg(pointName);

        QMessageBox msgBox(qApp->getMainWindow());
        msgBox.setWindowTitle(tr("Point Intersect Circles"));
        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        msgBox.setWindowIcon(QIcon(":/toolicon/32x32/point_of_intersection_circles.png"));
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(msg);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }

    quint32 id = _id;

    VPointF *p = new VPointF(point, pointName, mx, my);
    p->setShowPointName(showPointName);

    if (typeCreation == Source::FromGui)
    {
        id = data->AddGObject(p);
    }
    else
    {
        data->UpdateGObject(id, p);
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }

    if (parse == Document::FullParse)
    {
        VDrawTool::AddRecord(id, Tool::PointOfIntersectionCircles, doc);
        IntersectCirclesTool *point = new IntersectCirclesTool(doc, data, id, firstCircleCenterId,
                                                                                     secondCircleCenterId,
                                                                                     firstCircleRadius,
                                                                                     secondCircleRadius, crossPoint,
                                                                                     typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(c1Point.getIdTool());
        doc->IncrementReferens(c2Point.getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF IntersectCirclesTool::FindPoint(const QPointF &c1Point, const QPointF &c2Point, qreal c1Radius,
                                                   qreal c2Radius, const CrossCirclesPoint crossPoint)
{
    QPointF p1, p2;
    const int res = VGObject::IntersectionCircles(c1Point, c1Radius, c2Point, c2Radius, p1, p2);

    switch(res)
    {
        case 2:
            if (crossPoint == CrossCirclesPoint::FirstPoint)
            {
                return p1;
            }
            else
            {
                return p2;
            }
        case 1:
            return p1;
        case 3:
        case 0:
        default:
            return QPointF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCirclesTool::FirstCircleCenterPointName() const
{
    return VAbstractTool::data.GetGObject(firstCircleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCirclesTool::SecondCircleCenterPointName() const
{
    return VAbstractTool::data.GetGObject(secondCircleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCirclesTool::GetFirstCircleCenterId() const
{
    return firstCircleCenterId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SetFirstCircleCenterId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        firstCircleCenterId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCirclesTool::GetSecondCircleCenterId() const
{
    return secondCircleCenterId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SetSecondCircleCenterId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        secondCircleCenterId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula IntersectCirclesTool::GetFirstCircleRadius() const
{
    VFormula radius(firstCircleRadius, getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SetFirstCircleRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius can't be 0 or negative
        {
            firstCircleRadius = value.GetFormula(FormulaType::FromUser);
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula IntersectCirclesTool::GetSecondCircleRadius() const
{
    VFormula radius(secondCircleRadius, getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SetSecondCircleRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius can't be 0 or negative
        {
            secondCircleRadius = value.GetFormula(FormulaType::FromUser);
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint IntersectCirclesTool::GetCrossCirclesPoint() const
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::setCirclesCrossPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::ShowVisualization(bool show)
{
    ShowToolVisualization<IntersectCirclesVisual>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::RemoveReferens()
{
    const auto firstCircleCenter = VAbstractTool::data.GetGObject(firstCircleCenterId);
    const auto secondCircleCenter = VAbstractTool::data.GetGObject(secondCircleCenterId);

    doc->DecrementReferens(firstCircleCenter->getIdTool());
    doc->DecrementReferens(secondCircleCenter->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<IntersectCirclesDialog>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectCirclesDialog> dialogTool =
            m_dialog.objectCast<IntersectCirclesDialog>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrC1Center, QString().setNum(dialogTool->GetFirstCircleCenterId()));
    doc->SetAttribute(domElement, AttrC2Center, QString().setNum(dialogTool->GetSecondCircleCenterId()));
    doc->SetAttribute(domElement, AttrC1Radius, dialogTool->GetFirstCircleRadius());
    doc->SetAttribute(domElement, AttrC2Radius, dialogTool->GetSecondCircleRadius());
    doc->SetAttribute(domElement, AttrCrossPoint,
                      QString().setNum(static_cast<int>(dialogTool->GetCrossCirclesPoint())));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrC1Center, firstCircleCenterId);
    doc->SetAttribute(tag, AttrC2Center, secondCircleCenterId);
    doc->SetAttribute(tag, AttrC1Radius, firstCircleRadius);
    doc->SetAttribute(tag, AttrC2Radius, secondCircleRadius);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::ReadToolAttributes(const QDomElement &domElement)
{
    firstCircleCenterId = doc->GetParametrUInt(domElement, AttrC1Center, NULL_ID_STR);
    secondCircleCenterId = doc->GetParametrUInt(domElement, AttrC2Center, NULL_ID_STR);
    firstCircleRadius = doc->GetParametrString(domElement, AttrC1Radius);
    secondCircleRadius = doc->GetParametrString(domElement, AttrC2Radius);
    crossPoint = static_cast<CrossCirclesPoint>(doc->GetParametrUInt(domElement, AttrCrossPoint, "1"));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesTool::SetVisualization()
{
    if (not vis.isNull())
    {
        IntersectCirclesVisual *visual = qobject_cast<IntersectCirclesVisual *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(firstCircleCenterId);
        visual->setObject2Id(secondCircleCenterId);
        visual->setC1Radius(firstCircleRadius);
        visual->setC2Radius(secondCircleRadius);
        visual->setCrossPoint(crossPoint);
        visual->RefreshGeometry();
    }
}

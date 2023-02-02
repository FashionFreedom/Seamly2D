/***************************************************************************
 **  @file   intersect_circletangent_tool.cpp
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
 **  @file   vtoolpointfromcircleandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
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

#include "intersect_circletangent_tool.h"

#include "vtoolsinglepoint.h"
#include "../ifc/ifcdef.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../../vdrawtool.h"
#include "../../../vabstracttool.h"
#include "../../../../dialogs/tools/intersect_circletangent_dialog.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/visualization.h"
#include "../../../../visualization/line/intersect_circletangent_visual.h"

#include <QMessageBox>
#include <QSharedPointer>
#include <QString>
#include <new>

template <class T> class QSharedPointer;

const QString IntersectCircleTangentTool::ToolType = QStringLiteral("pointFromCircleAndTangent");

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentTool::IntersectCircleTangentTool(VAbstractPattern *doc, VContainer *data,
                                                               const quint32 &id,
                                                               quint32 circleCenterId, const QString &circleRadius,
                                                               quint32 tangentPointId, CrossCirclesPoint crossPoint,
                                                               const Source &typeCreation, QGraphicsItem *parent)
    : VToolSinglePoint(doc, data, id, QColor(qApp->Settings()->getPointNameColor()), parent)
    , circleCenterId(circleCenterId)
    , tangentPointId(tangentPointId)
    , circleRadius(circleRadius)
    , crossPoint(crossPoint)
{
    ToolCreation(typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::setDialog()
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectCircleTangentDialog> dialogTool = m_dialog.objectCast<IntersectCircleTangentDialog>();
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetCircleCenterId(circleCenterId);
    dialogTool->SetCircleRadius(circleRadius);
    dialogTool->setCirclesCrossPoint(crossPoint);
    dialogTool->SetTangentPointId(tangentPointId);
    dialogTool->SetPointName(p->name());
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentTool *IntersectCircleTangentTool::Create(QSharedPointer<DialogTool> dialog,
                                                                       VMainGraphicsScene *scene,
                                                                       VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<IntersectCircleTangentDialog> dialogTool = dialog.objectCast<IntersectCircleTangentDialog>();
    SCASSERT(not dialogTool.isNull())
    const quint32 circleCenterId = dialogTool->GetCircleCenterId();
    QString circleRadius = dialogTool->GetCircleRadius();
    const quint32 tangentPointId = dialogTool->GetTangentPointId();
    const CrossCirclesPoint pType = dialogTool->GetCrossCirclesPoint();
    const QString pointName = dialogTool->getPointName();
    IntersectCircleTangentTool *point = Create(0, pointName, circleCenterId, circleRadius, tangentPointId, pType,
                                                   5, 10, true, scene, doc, data, Document::FullParse, Source::FromGui);
    if (point != nullptr)
    {
        point->m_dialog = dialogTool;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentTool *IntersectCircleTangentTool::Create(const quint32 _id, const QString &pointName,
                                                                       quint32 circleCenterId, QString &circleRadius,
                                                                       quint32 tangentPointId,
                                                                       CrossCirclesPoint crossPoint,
                                                                       qreal mx, qreal my,
                                                                       bool showPointName,
                                                                       VMainGraphicsScene *scene,
                                                                       VAbstractPattern *doc, VContainer *data,
                                                                       const Document &parse,
                                                                       const Source &typeCreation)
{
    const qreal radius = qApp->toPixel(CheckFormula(_id, circleRadius, data));
    const VPointF cPoint = *data->GeometricObject<VPointF>(circleCenterId);
    const VPointF tPoint = *data->GeometricObject<VPointF>(tangentPointId);

    const QPointF point = IntersectCircleTangentTool::FindPoint(static_cast<QPointF>(tPoint),
                                                                    static_cast<QPointF>(cPoint), radius, crossPoint);

    if (point == QPointF())
    {
        const QString msg = tr("<b><big>Can't find intersection point %1 of</big></b><br>"
                               "<b><big>Circle and Tangent</big></b><br><br>"
                               "Using origin point as a place holder until pattern is corrected.")
                               .arg(pointName);

        QMessageBox msgBox(qApp->getMainWindow());
        msgBox.setWindowTitle(tr("Intersect Circle and Tangent"));
        msgBox.setWindowFlags(msgBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        msgBox.setWindowIcon(QIcon(":/toolicon/32x32/point_from_circle_and_tangent.png"));
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
        VDrawTool::AddRecord(id, Tool::PointFromCircleAndTangent, doc);
        IntersectCircleTangentTool *point = new IntersectCircleTangentTool(doc, data, id, circleCenterId,
                                                                                   circleRadius, tangentPointId,
                                                                                   crossPoint, typeCreation);
        scene->addItem(point);
        InitToolConnections(scene, point);
        VAbstractPattern::AddTool(id, point);
        doc->IncrementReferens(cPoint.getIdTool());
        doc->IncrementReferens(tPoint.getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF IntersectCircleTangentTool::FindPoint(const QPointF &p, const QPointF &center, qreal radius,
                                                  const CrossCirclesPoint crossPoint)
{
    QPointF p1, p2;
    const int res = VGObject::ContactPoints (p, center, radius, p1, p2);

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
QString IntersectCircleTangentTool::TangentPointName() const
{
    return VAbstractTool::data.GetGObject(tangentPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCircleTangentTool::CircleCenterPointName() const
{
    return VAbstractTool::data.GetGObject(circleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCircleTangentTool::GetTangentPointId() const
{
    return tangentPointId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SetTangentPointId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        tangentPointId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCircleTangentTool::GetCircleCenterId() const
{
    return circleCenterId;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SetCircleCenterId(const quint32 &value)
{
    if (value != NULL_ID)
    {
        circleCenterId = value;

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
VFormula IntersectCircleTangentTool::GetCircleRadius() const
{
    VFormula radius(circleRadius, getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(qApp->patternUnit()));
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SetCircleRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0)// Formula don't check this, but radius can't be 0 or negative
        {
            circleRadius = value.GetFormula(FormulaType::FromUser);
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint IntersectCircleTangentTool::GetCrossCirclesPoint() const
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::setCirclesCrossPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::ShowVisualization(bool show)
{
    ShowToolVisualization<IntersectCircleTangentVisual>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::RemoveReferens()
{
    const auto circleCenter = VAbstractTool::data.GetGObject(circleCenterId);
    const auto tangentPoint = VAbstractTool::data.GetGObject(tangentPointId);

    doc->DecrementReferens(circleCenter->getIdTool());
    doc->DecrementReferens(tangentPoint->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<IntersectCircleTangentDialog>(event, id);
    }
    catch(const VExceptionToolWasDeleted &error)
    {
        Q_UNUSED(error)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QSharedPointer<IntersectCircleTangentDialog> dialogTool = m_dialog.objectCast<IntersectCircleTangentDialog>();
    SCASSERT(not dialogTool.isNull())
    doc->SetAttribute(domElement, AttrName, dialogTool->getPointName());
    doc->SetAttribute(domElement, AttrCCenter, QString().setNum(dialogTool->GetCircleCenterId()));
    doc->SetAttribute(domElement, AttrTangent, QString().setNum(dialogTool->GetTangentPointId()));
    doc->SetAttribute(domElement, AttrCRadius, dialogTool->GetCircleRadius());
    doc->SetAttribute(domElement, AttrCrossPoint,
                      QString().setNum(static_cast<int>(dialogTool->GetCrossCirclesPoint())));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCCenter, circleCenterId);
    doc->SetAttribute(tag, AttrTangent, tangentPointId);
    doc->SetAttribute(tag, AttrCRadius, circleRadius);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::ReadToolAttributes(const QDomElement &domElement)
{
    circleCenterId = doc->GetParametrUInt(domElement, AttrCCenter, NULL_ID_STR);
    tangentPointId = doc->GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
    circleRadius = doc->GetParametrString(domElement, AttrCRadius);
    crossPoint = static_cast<CrossCirclesPoint>(doc->GetParametrUInt(domElement, AttrCrossPoint, "1"));
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentTool::SetVisualization()
{
    if (not vis.isNull())
    {
        IntersectCircleTangentVisual *visual = qobject_cast<IntersectCircleTangentVisual *>(vis);
        SCASSERT(visual != nullptr)

        visual->setObject1Id(tangentPointId);
        visual->setObject2Id(circleCenterId);
        visual->setCRadius(circleRadius);
        visual->setCrossPoint(crossPoint);
        visual->RefreshGeometry();
    }
}

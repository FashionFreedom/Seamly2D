/**************************************************************************
 **
 **  @file   intersect_circles_visual.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
 **
 **  @author Douglas S. Caskey
 **  @date   7.16.2022
 **
 **  @copyright
 **  Copyright (C) 2013-2022 Seamly2D project.
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published
 **  by the Free Software Foundation, either version 3 of the License,
 **  or (at your option) any later version.
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

#include "intersect_circles_visual.h"

#include "visline.h"
#include "../../tools/drawTools/toolpoint/toolsinglepoint/intersect_circles_tool.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../visualization.h"

#include <QGraphicsEllipseItem>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

//---------------------------------------------------------------------------------------------------------------------
IntersectCirclesVisual::IntersectCirclesVisual(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
    , object2Id(NULL_ID)
    , c1Radius(0)
    , c2Radius(0)
    , crossPoint(CrossCirclesPoint::FirstPoint)
    , point(nullptr)
    , c1Center(nullptr)
    , c2Center(nullptr)
    , c1Path(nullptr)
    , c2Path(nullptr)
    , m_secondrySupportColor(QColor(qApp->Settings()->getSecondarySupportColor()))
    , m_tertiarySupportColor(QColor(qApp->Settings()->getTertiarySupportColor()))
{
    this->setPen(QPen(Qt::NoPen)); // don't use parent this time

    c1Path   = InitItem<QGraphicsEllipseItem>(m_secondrySupportColor, this);
    c2Path   = InitItem<QGraphicsEllipseItem>(m_tertiarySupportColor, this);
    point    = InitPoint(mainColor, this);
    c1Center = InitPoint(supportColor, this);
    c2Center = InitPoint(supportColor, this);  //-V656
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(c1Center, static_cast<QPointF>(*first), supportColor);

        if (object2Id > NULL_ID)
        {
            const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(object2Id);
            DrawPoint(c2Center, static_cast<QPointF>(*second), supportColor);

            if (c1Radius > 0 && c2Radius > 0)
            {
                c1Path->setRect(pointRect(c1Radius));
                DrawPoint(c1Path, static_cast<QPointF>(*first), m_secondrySupportColor, Qt::DashLine);

                c2Path->setRect(pointRect(c2Radius));
                DrawPoint(c2Path, static_cast<QPointF>(*second), m_tertiarySupportColor, Qt::DashLine);

                const QPointF fPoint = IntersectCirclesTool::FindPoint(static_cast<QPointF>(*first),
                                                                                  static_cast<QPointF>(*second),
                                                                                  c1Radius, c2Radius, crossPoint);
                DrawPoint(point, fPoint, mainColor);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::VisualMode(const quint32 &id)
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
    SCASSERT(scene != nullptr)

    this->object1Id = id;
    Visualization::scenePos = scene->getScenePos();
    RefreshGeometry();

    AddOnScene();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::setObject2Id(const quint32 &value)
{
    object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::setC1Radius(const QString &value)
{
    c1Radius = FindLength(value, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::setC2Radius(const QString &value)
{
    c2Radius = FindLength(value, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesVisual::setCrossPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;
}

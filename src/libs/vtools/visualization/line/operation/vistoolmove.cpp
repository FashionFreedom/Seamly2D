/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 10, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "vistoolmove.h"

#include <limits.h>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGuiApplication>
#include <QLineF>
#include <QPainterPath>
#include <QSharedPointer>
#include <Qt>
#include <QtAlgorithms>
#include <new>

#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/diagnostic.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/varc.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "visoperation.h"
#include "../vmisc/logging.h"

Q_LOGGING_CATEGORY(visTool, "vistool")

//---------------------------------------------------------------------------------------------------------------------
VisToolMove::VisToolMove(const VContainer *data, QGraphicsItem *parent)
    : VisOperation(data, parent)
    , angle(0)
    , length(0)
    , rotationAngle(INT_MIN)
    , originPointItem(nullptr)
    , rotationOriginPointItem(nullptr)
    , rotationFinishPointItem(nullptr)
    , rotationLineItem(nullptr)
{
    originPointItem         = InitPoint(supportColor2, this);
    rotationOriginPointItem = InitPoint(supportColor2, this);
    rotationFinishPointItem = InitPoint(supportColor, this);
    rotationLineItem        = InitItem<VScaledLine>(supportColor3, this);
}

//---------------------------------------------------------------------------------------------------------------------
VisToolMove::~VisToolMove()
{
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::RefreshGeometry()
{
    if (objects.isEmpty())
    {
        return;
    }

    int iPoint = -1;
    int iCurve = -1;

    const QVector<QGraphicsItem *> originObjects = CreateOriginObjects(iPoint, iCurve);

    QPointF origin = GetOriginPoint(originObjects);
    DrawPoint(originPointItem, origin, supportColor2);

    qreal tempAngle = 0;
    qreal tempLength = 0;
    qreal tempRotation = 0;

    QLineF moveLine;
    if (qFuzzyIsNull(length))
    {
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            moveLine = QLineF(origin, Visualization::scenePos);
            moveLine.setAngle(CorrectAngle(moveLine.angle()));    //contrain move line angle to preference setting
        }
        else
        {
            moveLine = QLineF(origin, Visualization::scenePos);
        }

        tempAngle  = moveLine.angle();
        tempLength = moveLine.length();
    }
    else
    {
        moveLine   = VGObject::BuildLine(origin, length, angle);
        tempAngle  = angle;
        tempLength = length;

        if (object1Id != NULL_ID)
        {
            origin = Visualization::data->GeometricObject<VPointF>(object1Id)->toQPointF();
            DrawPoint(rotationOriginPointItem, origin, supportColor2);
        }
        else
        {
            origin = moveLine.p2();
            rotationOriginPointItem->setVisible(false);
        }

        QLineF rotationLine;
        if (VFuzzyComparePossibleNulls(rotationAngle, INT_MIN))
        {
            rotationLine = QLineF(origin, Visualization::scenePos);

            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                rotationLine.setAngle(CorrectAngle(rotationLine.angle())); //contrain rotation line angle to preference 
            }

            qreal cursorLength = rotationLine.length();
            rotationLine.setP2(Ray(origin, rotationLine.angle()));

            qreal minL = scaledRadius(sceneScale(qApp->getCurrentScene())) * 1.5;

            if (cursorLength > minL)
            {
               tempRotation = rotationLine.angle();
            }
            else
            {
                rotationLine.setAngle(0);
            }
        }
        else
        {
            rotationLine = QLineF(origin, Ray(origin, rotationAngle));
            tempRotation = rotationAngle;
        }

        DrawLine(rotationLineItem, rotationLine, supportColor3, Qt::DashLine);
    }
    DrawLine(this, moveLine, supportColor2, Qt::DashLine);
    DrawPoint(rotationFinishPointItem, moveLine.p2(), supportColor3);

    static const QString prefix = UnitsToStr(qApp->patternUnit(), true);
    if (qFuzzyIsNull(length))
    {
        Visualization::toolTip = tr("Length = %1%2, angle = %3°, <b>Shift</b> to constrain angle, "
                                    "<b>Mouse click</b> - finish selecting a position")
                                    .arg(qApp->TrVars()->FormulaToUser(QString::number(qApp->fromPixel(tempLength)),
                                                                       qApp->Settings()->GetOsSeparator()))
                                    .arg(prefix)
                                    .arg(tempAngle);
    }
    else
    {
    Visualization::toolTip = tr("Length = %1%2, angle = %3°, rotation angle = %4° Hold <b>SHIFT</b> to constrain angle,"
                                "<b>CTRL</b> - change rotation origin point, <b>Mouse click</b> - finish creating")
                                .arg(qApp->TrVars()->FormulaToUser(QString::number(qApp->fromPixel(tempLength)),
                                                                   qApp->Settings()->GetOsSeparator()))
                                .arg(prefix)
                                .arg(tempAngle)
                                .arg(tempRotation);
    }
    createRotatedObjects(iPoint, iCurve, tempLength, tempAngle, tempRotation, origin);
}

//---------------------------------------------------------------------------------------------------------------------
QString VisToolMove::Angle() const
{
    return QString::number(line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::SetAngle(const QString &expression)
{
    angle = FindVal(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
QString VisToolMove::Rotation() const
{
    return QString::number(rotationLineItem->line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::setRotation(const QString &expression)
{
    rotationAngle = FindVal(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
QString VisToolMove::Length() const
{
    return QString::number(LengthValue());
}

//---------------------------------------------------------------------------------------------------------------------
qreal VisToolMove::LengthValue() const
{
    return qApp->fromPixel(line().length());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::SetLength(const QString &expression)
{
    length = FindLength(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::setOriginPointId(quint32 value)
{
    object1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
QGraphicsPathItem *VisToolMove::AddOriginCurve(quint32 id, int &i)
{
    const QSharedPointer<Item> curve = Visualization::data->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), supportColor2);
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), supportColor2, Qt::SolidLine, Qt::RoundCap);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
int VisToolMove::AddDestinationCurve(qreal angle, qreal length, quint32 id, int i, qreal rotationAngle,
                                      const QPointF &rotationOrigin)
{
    const QSharedPointer<Item> curve = Visualization::data->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), supportColor);
    const Item moved = curve->Move(length, angle).Rotate(rotationOrigin, rotationAngle);
    DrawPath(path, moved.GetPath(), moved.DirectionArrows(), supportColor, Qt::SolidLine, Qt::RoundCap);

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VisToolMove::GetOriginPoint(const QVector<QGraphicsItem *> &objects)
{
    QRectF boundingRect;
    for (int i=0; i < objects.size(); ++i)
    {
        QGraphicsItem *object = objects.at(i);
        if (object)
        {
            QRectF childrenRect = object->childrenBoundingRect();
            //map to scene coordinate.
            childrenRect.translate(object->scenePos());

            QRectF itemRect = object->boundingRect();
            //map to scene coordinate.
            itemRect.translate(object->scenePos());

            boundingRect = boundingRect.united(itemRect);
            boundingRect = boundingRect.united(childrenRect);
        }
    }

    return boundingRect.center();
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

QVector<QGraphicsItem *> VisToolMove::CreateOriginObjects(int &iPoint, int &iCurve)
{
    QVector<QGraphicsItem *> originObjects;

    qCDebug(visTool, "Create Source Objects");
    for (int i = 0; i < objects.size(); ++i)
    {
        const quint32 id = objects.at(i);
        qCDebug(visTool, "Object:  %d", id);
        const QSharedPointer<VGObject> obj = Visualization::data->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = Visualization::data->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), supportColor2);
                DrawPoint(point, static_cast<QPointF>(*p), supportColor2);
                originObjects.append(point);

                break;
            }
            case GOType::Arc:
                originObjects.append(AddOriginCurve<VArc>(id, iCurve));
                break;
            case GOType::EllipticalArc:
                originObjects.append(AddOriginCurve<VEllipticalArc>(id, iCurve));
                break;
            case GOType::Spline:
                originObjects.append(AddOriginCurve<VSpline>(id, iCurve));
                break;
            case GOType::SplinePath:
                originObjects.append(AddOriginCurve<VSplinePath>(id, iCurve));
                break;
            case GOType::CubicBezier:
                originObjects.append(AddOriginCurve<VCubicBezier>(id, iCurve));
                break;
            case GOType::CubicBezierPath:
                originObjects.append(AddOriginCurve<VCubicBezierPath>(id, iCurve));
                break;
            case GOType::Unknown:
                break;
        }
    }

    return originObjects;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void VisToolMove::createRotatedObjects(int &iPoint, int &iCurve, qreal length, qreal angle, qreal rotationAngle,
                                            const QPointF &rotationOrigin)
{
    qCDebug(visTool, "Create Rotated Objects");
    for (int i = 0; i < objects.size(); ++i)
    {
        const quint32 id = objects.at(i);
        qCDebug(visTool, "Object:  %d", id);
        const QSharedPointer<VGObject> obj = Visualization::data->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects was handled.");

        qCDebug(visTool, "VisToolMove - Rotation Origin");
        qCDebug(visTool, "X:  %f", rotationOrigin.x());
        qCDebug(visTool, "Y:  %f", rotationOrigin.y());

        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = Visualization::data->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), supportColor);
                qCDebug(visTool, "visTool - Point");
                qCDebug(visTool, "length:  %f", length);
                qCDebug(visTool, "angle:  %f", angle);
                qCDebug(visTool, "rotation:  %f\n", rotationAngle);
                DrawPoint(point, static_cast<QPointF>(p->Move(length, angle).Rotate(rotationOrigin, rotationAngle)),
                          supportColor);
                break;
            }
            case GOType::Arc:
                iCurve = AddDestinationCurve<VArc>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::EllipticalArc:
                iCurve = AddDestinationCurve<VEllipticalArc>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::Spline:
                iCurve = AddDestinationCurve<VSpline>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::SplinePath:
                iCurve = AddDestinationCurve<VSplinePath>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::CubicBezier:
                qCDebug(visTool, "visTool - VCubicBezier");
                qCDebug(visTool, "length:  %f", length);
                qCDebug(visTool, "angle:  %f", angle);
                qCDebug(visTool, "rotation:  %f\n", rotationAngle);
                iCurve = AddDestinationCurve<VCubicBezier>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::CubicBezierPath:
                iCurve = AddDestinationCurve<VCubicBezierPath>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::Unknown:
                break;
        }
    }
}

QT_WARNING_POP

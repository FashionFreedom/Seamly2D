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
#include <QPointF>
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
    , moveLineItem(nullptr)
    , rotationLineItem(nullptr)
    , m_origin(QPointF())
    , m_rotationPoint(QPointF())
{
    originPointItem         = InitPoint(supportColor2, this, 110);
    originPointItem->setBrush(QBrush(supportColor2, Qt::SolidPattern));

    rotationOriginPointItem = InitPoint(supportColor2, this, 120);
    rotationOriginPointItem->setBrush(QBrush(supportColor2, Qt::SolidPattern));

    rotationFinishPointItem = InitPoint(supportColor3, this, 130);
    rotationFinishPointItem->setBrush(QBrush(supportColor3, Qt::SolidPattern));

    moveLineItem            = InitItem<ArrowedLineItem>(supportColor2, this);
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

    createOriginObjects(iPoint, iCurve);

    qreal tempAngle = 0;
    qreal tempLength = 0;
    qreal tempRotation = 0;

    QLineF line;
    if (qFuzzyIsNull(length))
    {
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line = QLineF(m_origin, Visualization::scenePos);
            line.setAngle(CorrectAngle(line.angle()));    //contrain move line angle to preference setting
        }
        else
        {
            line = QLineF(m_origin, Visualization::scenePos);
        }

        tempAngle  = line.angle();
        tempLength = line.length();
    }
    else
    {
        tempAngle  = angle;
        tempLength = length;

        if (object1Id != NULL_ID)
        {
            m_origin = Visualization::data->GeometricObject<VPointF>(object1Id)->toQPointF();
            DrawPoint(rotationOriginPointItem, m_origin, supportColor2);
            line   = VGObject::BuildLine(m_origin, length, angle);
            m_rotationPoint = line.p2();
            originPointItem->setVisible(false);
        }
        else
        {
            DrawPoint(originPointItem, m_origin, supportColor2);
            line   = VGObject::BuildLine(m_origin, length, angle);
            m_rotationPoint = line.p2();
            rotationOriginPointItem->setVisible(false);
        }

        QLineF rotationLine;
        if (VFuzzyComparePossibleNulls(rotationAngle, INT_MIN))
        {
            rotationLine = QLineF(m_rotationPoint, Visualization::scenePos);

            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                rotationLine.setAngle(CorrectAngle(rotationLine.angle())); //contrain rotation line angle to preference
            }

            qreal cursorLength = rotationLine.length();
            rotationLine.setP2(Ray(m_rotationPoint, rotationLine.angle()));

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
            rotationLine = QLineF(m_rotationPoint, Ray(m_rotationPoint, rotationAngle));
            tempRotation = rotationAngle;
        }

        DrawLine(rotationLineItem, rotationLine, supportColor3, lineWeight, Qt::DashLine);
    }

    drawArrowedLine(moveLineItem, line, supportColor2, Qt::DashLine);
    DrawPoint(rotationFinishPointItem, line.p2(), supportColor3);

    static const QString prefix = UnitsToStr(qApp->patternUnit(), true);
    if (qFuzzyIsNull(length))
    {
        Visualization::toolTip = tr("Length = %1%2, angle = %3°, <b>Shift</b> to constrain angle, "
                                    "<b>Mouse click</b> - finish selecting a position")
                                    .arg(qApp->translateVariables()->FormulaToUser(QString::number(qApp->fromPixel(tempLength)),
                                                                       qApp->Settings()->getOsSeparator()))
                                    .arg(prefix)
                                    .arg(tempAngle);
    }
    else
    {
    Visualization::toolTip = tr("Length = %1%2, angle = %3°, rotation angle = %4° Hold <b>SHIFT</b> to constrain angle,"
                                "<b>CTRL</b> - change rotation origin point, <b>Mouse click</b> - finish creating")
                                .arg(qApp->translateVariables()->FormulaToUser(QString::number(qApp->fromPixel(tempLength)),
                                                                   qApp->Settings()->getOsSeparator()))
                                .arg(prefix)
                                .arg(tempAngle)
                                .arg(tempRotation);
    }
    createRotatedObjects(iPoint, iCurve, tempLength, tempAngle, tempRotation, m_rotationPoint);
}

//---------------------------------------------------------------------------------------------------------------------
QString VisToolMove::Angle() const
{
    return QString::number(moveLineItem->line().angle());
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
    //return qApp->fromPixel(line().length());
    return qApp->fromPixel(moveLineItem->line().length());
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
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), supportColor2, Qt::SolidLine,
             lineWeight,  Qt::RoundCap);

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
    DrawPath(path, moved.GetPath(), moved.DirectionArrows(), supportColor, Qt::SolidLine,
             lineWeight,  Qt::RoundCap);

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void VisToolMove::createOriginObjects(int &iPoint, int &iCurve)
{
    QPolygonF sourceObjects;

    for (int i = 0; i < objects.size(); ++i)
    {
        const quint32 id = objects.at(i);
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
                sourceObjects.append(p->toQPointF());
                break;
            }
            case GOType::Arc:
                AddOriginCurve<VArc>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::EllipticalArc:
                AddOriginCurve<VEllipticalArc>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::Spline:
                AddOriginCurve<VSpline>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::SplinePath:
                AddOriginCurve<VSplinePath>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::CubicBezier:
                AddOriginCurve<VCubicBezier>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::CubicBezierPath:
                AddOriginCurve<VCubicBezierPath>(id, iCurve);
                sourceObjects.append(Visualization::data->GeometricObject<VAbstractCurve>(id)->getPoints());
                break;
            case GOType::Unknown:
            case GOType::Curve:
            case GOType::Path:
            case GOType::AllCurves:
            default:
                break;
        }
    }

    m_origin = sourceObjects.boundingRect().center();
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void VisToolMove::createRotatedObjects(int &iPoint, int &iCurve, qreal length, qreal angle, qreal rotationAngle,
                                            const QPointF &rotationOrigin)
{
    for (int i = 0; i < objects.size(); ++i)
    {
        const quint32 id = objects.at(i);
        const QSharedPointer<VGObject> obj = Visualization::data->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects was handled.");

        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = Visualization::data->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), supportColor);
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
                iCurve = AddDestinationCurve<VCubicBezier>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::CubicBezierPath:
                iCurve = AddDestinationCurve<VCubicBezierPath>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::Unknown:
            case GOType::Curve:
            case GOType::Path:
            case GOType::AllCurves:
                break;
        }
    }
}

QT_WARNING_POP

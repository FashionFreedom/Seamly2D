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
 **  @date   16 9, 2016
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

#include "vabstractmirror.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractMirror::VAbstractMirror(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &suffix,
                                     const QVector<SourceItem> &source, const QVector<DestinationItem> &destination,
                                     QGraphicsItem *parent)
    : VAbstractOperation(doc, data, id, suffix, source, destination, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMirror::createDestination(Source typeCreation, quint32 &id, QVector<DestinationItem> &dest,
                                          const QVector<SourceItem> &source, const QPointF &fPoint, const QPointF &sPoint,
                                          const QString &suffix, VAbstractPattern *doc, VContainer *data,
                                          const Document &parse)
{
    if (typeCreation == Source::FromGui)
    {
        dest.clear();// Try to avoid mistake, value must be empty

        id = VContainer::getNextId();//Just reserve id for tool

        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            const quint32 objectId = item.id;
            const QSharedPointer<VGObject> obj = data->GetGObject(objectId);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
            switch(static_cast<GOType>(obj->getType()))
            {
                case GOType::Point:
                    dest.append(createPoint(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::Arc:
                    dest.append(createArc<VArc>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::EllipticalArc:
                    dest.append(createArc<VEllipticalArc>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::Spline:
                    dest.append(createCurve<VSpline>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::SplinePath:
                    dest.append(createCurveWithSegments<VSplinePath>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::CubicBezier:
                    dest.append(createCurve<VCubicBezier>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::CubicBezierPath:
                    dest.append(createCurveWithSegments<VCubicBezierPath>(id, objectId, fPoint, sPoint, suffix, data));
                    break;
                case GOType::Unknown:
                    break;
            }
QT_WARNING_POP
        }
    }
    else
    {
        for (int i = 0; i < source.size(); ++i)
        {
            const SourceItem item = source.at(i);
            const quint32 objectId = item.id;
            const QSharedPointer<VGObject> obj = data->GetGObject(objectId);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 7, "Not all objects were handled.");

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
            switch(static_cast<GOType>(obj->getType()))
            {
                case GOType::Point:
                {
                    const DestinationItem &item = dest.at(i);
                    updatePoint(id, objectId, fPoint, sPoint, suffix, data, item);
                        //dest.at(i).id, dest.at(i).mx, dest.at(i).my);
                    break;
                }
                case GOType::Arc:
                    updateArc<VArc>(id, objectId, fPoint, sPoint, suffix, data, dest.at(i).id);
                    break;
                case GOType::EllipticalArc:
                    updateArc<VEllipticalArc>(id, objectId, fPoint, sPoint, suffix, data, dest.at(i).id);
                    break;
                case GOType::Spline:
                    updateCurve<VSpline>(id, objectId, fPoint, sPoint, suffix, data, dest.at(i).id);
                    break;
                case GOType::SplinePath:
                    updateCurveWithSegments<VSplinePath>(id, objectId, fPoint, sPoint, suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezier:
                    updateCurve<VCubicBezier>(id, objectId, fPoint, sPoint, suffix, data, dest.at(i).id);
                    break;
                case GOType::CubicBezierPath:
                    updateCurveWithSegments<VCubicBezierPath>(id, objectId, fPoint, sPoint, suffix, data,
                                                              dest.at(i).id);
                    break;
                case GOType::Unknown:
                    break;
            }
QT_WARNING_POP
        }
        if (parse != Document::FullParse)
        {
            doc->UpdateToolData(id, data);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
DestinationItem VAbstractMirror::createPoint(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                               const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF rotated = point->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);

    DestinationItem item;
    item.mx = rotated.mx();
    item.my = rotated.my();
    item.showPointName = rotated.isShowPointName();
    item.id = data->AddGObject(new VPointF(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractMirror::createArc(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                             const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractMirror::updatePoint(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                    const QPointF &secondPoint, const QString &suffix, VContainer *data,
                                    const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(idItem);
    VPointF rotated = point->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);
    rotated.setMx(item.mx);
    rotated.setMy(item.my);
    rotated.setShowPointName(item.showPointName);
    data->UpdateGObject(item.id, new VPointF(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractMirror::updateArc(quint32 idTool, quint32 idItem, const QPointF &firstPoint, const QPointF &secondPoint,
                                  const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

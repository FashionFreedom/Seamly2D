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

#ifndef VABSTRACTMIRROR_H
#define VABSTRACTMIRROR_H

#include <QtGlobal>

#include "../vabstractoperation.h"

class VAbstractMirror : public VAbstractOperation
{
    Q_OBJECT

public:
    virtual               ~VAbstractMirror() Q_DECL_EQ_DEFAULT;

protected:
                           VAbstractMirror(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &suffix,
                                           const QVector<quint32> &source, const QVector<DestinationItem> &destination,
                                           QGraphicsItem *parent = nullptr);

    static void            createDestination(Source typeCreation, quint32 &id, QVector<DestinationItem> &dest,
                                             const QVector<quint32> &source, const QPointF &fPoint,
                                             const QPointF &sPoint, const QString &suffix, VAbstractPattern *doc,
                                             VContainer *data, const Document &parse);

    static DestinationItem createPoint(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                       const QPointF &secondPoint, const QString &suffix, VContainer *data);

    template <class Item>
    static DestinationItem createItem(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                      const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createArc(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                     const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createCurve(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                       const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                                   const QPointF &secondPoint, const QString &suffix, VContainer *data);

    static void updatePoint(quint32 idTool, quint32 idItem, const QPointF &firstPoint, const QPointF &secondPoint,
                            const QString &suffix, VContainer *data, const DestinationItem &item);
    template <class Item>
    static void updateItem(quint32 idTool, quint32 idItem, const QPointF &firstPoint, const QPointF &secondPoint,
                           const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateArc(quint32 idTool, quint32 idItem, const QPointF &firstPoint, const QPointF &secondPoint,
                          const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateCurve(quint32 idTool, quint32 idItem, const QPointF &firstPoint, const QPointF &secondPoint,
                            const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                        const QPointF &secondPoint, const QString &suffix, VContainer *data,
                                        quint32 id);

private:
    Q_DISABLE_COPY(VAbstractMirror)
};

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractMirror::createItem(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                                const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);

    DestinationItem item;
    item.mx = INT_MAX;
    item.my = INT_MAX;
    item.id = data->AddGObject(new Item(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractMirror::createCurve(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                                 const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractMirror::createCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                                           const QPointF &secondPoint, const QString &suffix,
                                                           VContainer *data)
{
    const DestinationItem item = createItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractMirror::updateItem(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                   const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(idItem);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);
    data->UpdateGObject(id, new Item(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractMirror::updateCurve(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                    const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id)
{
    updateItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractMirror::updateCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &firstPoint,
                                                const QPointF &secondPoint, const QString &suffix, VContainer *data,
                                                quint32 id)
{
    updateItem<Item>(idTool, idItem, firstPoint, secondPoint, suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

#endif // VABSTRACTMIRROR_H

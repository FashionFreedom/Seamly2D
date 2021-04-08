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
 **  @file   vtoolrotation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2016
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

#ifndef VTOOLROTATION_H
#define VTOOLROTATION_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "vabstractoperation.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"

template <class T> class QSharedPointer;
class VFormula;

class VToolRotation : public VAbstractOperation
{
    Q_OBJECT
public:
    virtual               ~VToolRotation() Q_DECL_EQ_DEFAULT;
    virtual void           setDialog() Q_DECL_OVERRIDE;
    static VToolRotation  *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                  VAbstractPattern *doc, VContainer *data);

    static VToolRotation  *Create(const quint32 _id, const quint32 &origin, QString &angle, const QString &suffix,
                                  const QVector<quint32> &source, const QVector<DestinationItem> &destination,
                                  VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                  const Document &parse, const Source &typeCreation);

    static const QString   ToolType;

    virtual int            type() const Q_DECL_OVERRIDE {return Type;}
    enum                   { Type = UserType + static_cast<int>(Tool::Rotation)};

    QString                getOriginPointName() const;

    VFormula               GetFormulaAngle() const;
    void                   SetFormulaAngle(const VFormula &value);

    virtual void           ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void           showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void           SetVisualization() Q_DECL_OVERRIDE;
    virtual void           SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void           ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void           SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual QString        makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolRotation)
    quint32                origPointId;
    QString                formulaAngle;

                           VToolRotation(VAbstractPattern *doc, VContainer *data, quint32 id,
                                         quint32 origPointId, const QString &angle, const QString &suffix,
                                         const QVector<quint32> &source, const QVector<DestinationItem> &destination,
                                         const Source &typeCreation, QGraphicsItem *parent = nullptr);

    static DestinationItem createPoint(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                       const QString &suffix, VContainer *data);

    template <class Item>
    static DestinationItem createItem(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                      const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createArc(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                     const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createCurve(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                       const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem createCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &origin,
                                                   qreal angle, const QString &suffix, VContainer *data);

    static void updatePoint(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data, const DestinationItem &item);
    template <class Item>
    static void updateItem(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                           const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateArc(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                          const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateCurve(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                            const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void updateCurveWithSegments(quint32 idTool, quint32 idItem, const QPointF &origin, qreal angle,
                                        const QString &suffix, VContainer *data, quint32 id);
};

#endif // VTOOLROTATION_H

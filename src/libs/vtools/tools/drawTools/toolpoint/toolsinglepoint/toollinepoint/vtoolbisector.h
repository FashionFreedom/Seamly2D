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
 **  @file   vtoolbisector.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VTOOLBISECTOR_H
#define VTOOLBISECTOR_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolBisector class tool for creation bisector point.
 */
class VToolBisector : public VToolLinePoint
{
    Q_OBJECT
public:
    static qreal   BisectorAngle(const QPointF &firstPoint, const QPointF &secondPoint, const QPointF &thirdPoint);
    static QPointF FindPoint(const QPointF &firstPoint, const QPointF &secondPoint, const QPointF &thirdPoint,
                             const qreal& length);
    virtual void   setDialog() override;

    static VToolBisector *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene, VAbstractPattern *doc,
                                 VContainer *data);
    static VToolBisector *Create(const quint32 _id, QString &formula, quint32 firstPointId, quint32 secondPointId,
                                 quint32 thirdPointId, const QString &lineType, const QString &lineWeight,
                                 const QString &lineColor,
                                 const QString &pointName, quint32 lineId, qreal mx, qreal my,  bool showPointName,
                                 VMainGraphicsScene  *scene, VAbstractPattern *doc, VContainer *data,
                                 const Document &parse, const Source &typeCreation);

    static const QString ToolType;
    virtual int    type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::Bisector)};

    QString        FirstPointName() const;
    QString        ThirdPointName() const;

    quint32        GetFirstPointId() const;
    void           SetFirstPointId(const quint32 &value);

    quint32        GetThirdPointId() const;
    void           SetThirdPointId(const quint32 &value);

    virtual void   ShowVisualization(bool show) override;

protected slots:
    virtual void   showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;

protected:
    virtual void   RemoveReferens() override;
    virtual void   SaveDialog(QDomElement &domElement) override;
    virtual void   SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void   ReadToolAttributes(const QDomElement &domElement) override;
    virtual void   SetVisualization() override;
private:
    Q_DISABLE_COPY(VToolBisector)

    /** @brief firstPointId id first point of angle. */
    quint32        firstPointId;

    /** @brief thirdPointId id third point of angle. */
    quint32        thirdPointId;
    quint32        lineId; /** @brief lineId persisted id of this tool's line. See issue #1678. */

                   VToolBisector(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                 const QString &lineType, const QString &lineWeight,
                                 const QString &lineColor, const QString &formula,
                                 const quint32 &firstPointId, const quint32 &secondPointId,
                                 const quint32 &thirdPointId, const quint32 &lineId, const Source &typeCreation,
                                 QGraphicsItem * parent = nullptr);
};

#endif // VTOOLBISECTOR_H

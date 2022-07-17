/**************************************************************************
 **
 **  @file   intersect_circletangent_tool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
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

#ifndef INTERSECT_CIRCLETANGENT_TOOL_H
#define INTERSECT_CIRCLETANGENT_TOOL_H

#include "vtoolsinglepoint.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

class VFormula;
template <class T> class QSharedPointer;

class IntersectCircleTangentTool : public VToolSinglePoint
{
    Q_OBJECT
public:
    virtual void         setDialog() Q_DECL_OVERRIDE;

    static IntersectCircleTangentTool *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene,
                                                  VAbstractPattern *doc, VContainer *data);
    static IntersectCircleTangentTool *Create(const quint32 _id, const QString &pointName,
                                                  quint32 circleCenterId, QString &circleRadius, quint32 tangentPointId,
                                                  CrossCirclesPoint crossPoint, qreal mx, qreal my, bool showPointName,
                                                  VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                                  const Document &parse, const Source &typeCreation);

    static QPointF       FindPoint(const QPointF &p, const QPointF &center, qreal radius,
                                   const CrossCirclesPoint crossPoint);

    static const QString ToolType;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                {Type = UserType + static_cast<int>(Tool::PointFromCircleAndTangent) };

    QString              TangentPointName() const;
    QString              CircleCenterPointName() const;

    quint32              GetTangentPointId() const;
    void                 SetTangentPointId(const quint32 &value);

    quint32              GetCircleCenterId() const;
    void                 SetCircleCenterId(const quint32 &value);

    VFormula             GetCircleRadius() const;
    void                 SetCircleRadius(const VFormula &value);

    CrossCirclesPoint    GetCrossCirclesPoint() const;
    void                 SetCrossCirclesPoint(const CrossCirclesPoint &value);

    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void         RemoveReferens() Q_DECL_OVERRIDE;
    virtual void         SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void         ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SetVisualization() Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(IntersectCircleTangentTool)

    quint32              circleCenterId;
    quint32              tangentPointId;
    QString              circleRadius;
    CrossCirclesPoint    crossPoint;

                         IntersectCircleTangentTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                        quint32 circleCenterId, const QString &circleRadius,
                                                        quint32 tangentPointId, CrossCirclesPoint crossPoint,
                                                        const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // INTERSECT_CIRCLETANGENT_TOOL_H

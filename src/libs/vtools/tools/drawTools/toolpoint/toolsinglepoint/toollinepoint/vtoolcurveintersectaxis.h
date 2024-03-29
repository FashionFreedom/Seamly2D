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
 **  @file   vtoolcurveintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#ifndef VTOOLCURVEINTERSECTAXIS_H
#define VTOOLCURVEINTERSECTAXIS_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vpatterndb/vformula.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

class VToolCurveIntersectAxis : public VToolLinePoint
{
    Q_OBJECT

public:
    virtual             ~VToolCurveIntersectAxis() Q_DECL_EQ_DEFAULT;
    virtual void         setDialog() Q_DECL_OVERRIDE;

    static VToolCurveIntersectAxis *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                           VAbstractPattern *doc, VContainer *data);
    static VToolCurveIntersectAxis *Create(const quint32 _id, const QString &pointName, const QString &lineType,
                                           const QString &lineWeight,
                                           const QString &lineColor, QString &formulaAngle, quint32 basePointId,
                                           quint32 curveId, qreal mx, qreal my, bool showPointName,
                                           VMainGraphicsScene  *scene, VAbstractPattern *doc, VContainer *data,
                                           const Document &parse, const Source &typeCreation);

    static bool          FindPoint(const QPointF &point, qreal angle, const QSharedPointer<VAbstractCurve> &curve,
                                   QPointF *intersectPoint);

    static const QString ToolType;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CurveIntersectAxis)};

    VFormula             GetFormulaAngle() const;
    void                 SetFormulaAngle(const VFormula &value);

    QString              CurveName() const;

    quint32              getCurveId() const;
    void                 setCurveId(const quint32 &value);

    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void         SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void         ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SetVisualization() Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolCurveIntersectAxis)
    QString              formulaAngle;
    quint32              curveId;

                         VToolCurveIntersectAxis(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                                 const QString &lineType, const QString &lineWeight,
                                                 const QString &lineColor,
                                                 const QString &formulaAngle, const quint32 &basePointId,
                                                 const quint32 &curveId, const Source &typeCreation,
                                                 QGraphicsItem * parent = nullptr);

    template <class Item>
    static void          InitArc(VContainer *data, qreal segLength, const VPointF *p, quint32 curveId);
    static void          InitSegments(const GOType &curveType, qreal segLength, const VPointF *p, quint32 curveId,
                                      VContainer *data);
};

#endif // VTOOLCURVEINTERSECTAXIS_H

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
 **  @file   vtoollineintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#ifndef VTOOLLINEINTERSECTAXIS_H
#define VTOOLLINEINTERSECTAXIS_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/vformula.h"
#include "../vmisc/def.h"
#include "vtoollinepoint.h"

template <class T> class QSharedPointer;

class VToolLineIntersectAxis : public VToolLinePoint
{
    Q_OBJECT
public:
    virtual             ~VToolLineIntersectAxis() Q_DECL_EQ_DEFAULT;
    virtual void         setDialog() override;

    static VToolLineIntersectAxis *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                          VAbstractPattern *doc, VContainer *data);
    static VToolLineIntersectAxis *Create(const quint32 _id, const QString &pointName, const QString &lineType,
                                          const QString &lineWeight, const QString &lineColor,
                                          QString &formulaAngle, quint32 basePointId,
                                          quint32 firstPointId, quint32 secondPointId,
                                          quint32 line1Id, quint32 line2Id, quint32 line3Id,
                                          qreal mx, qreal my, bool showPointName, VMainGraphicsScene  *scene,
                                          VAbstractPattern *doc,
                                          VContainer *data, const Document &parse, const Source &typeCreation);

    static QPointF       FindPoint(const QLineF &axis, const QLineF &line);

    static const QString ToolType;
    virtual int          type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::LineIntersectAxis)};

    QString              FirstLinePoint() const;
    QString              SecondLinePoint() const;

    VFormula             GetFormulaAngle() const;
    void                 SetFormulaAngle(const VFormula &value);

    quint32              GetFirstPointId() const;
    void                 SetFirstPointId(const quint32 &value);

    quint32              GetSecondPointId() const;
    void                 SetSecondPointId(const quint32 &value);

    virtual void         ShowVisualization(bool show) override;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;

protected:
    virtual quint32      GetCopyLengthLineId() const override {return line1Id;}
    virtual quint32      GetCopyAngleLineId() const override {return line1Id;}
    virtual void         SaveDialog(QDomElement &domElement) override;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void         ReadToolAttributes(const QDomElement &domElement) override;
    virtual void         SetVisualization() override;
    virtual QString      makeToolTip() const override;

private:
    Q_DISABLE_COPY(VToolLineIntersectAxis)

    QString              formulaAngle;
    quint32              firstPointId;
    quint32              secondPointId;
    quint32              line1Id; /** @brief line1Id persisted id of the basePoint-to-point line. See issue #1678. */
    quint32              line2Id; /** @brief line2Id persisted id of the firstPoint-to-point line. See issue #1678. */
    quint32              line3Id; /** @brief line3Id persisted id of the point-to-secondPoint line. See issue #1678. */

    VToolLineIntersectAxis(VAbstractPattern *doc, VContainer *data, const quint32 &id, const QString &lineType,
                           const QString &lineWeight, const QString &lineColor, const QString &formulaAngle,
                           const quint32 &basePointId, const quint32 &firstPointId, const quint32 &secondPointId,
                           const quint32 &line1Id, const quint32 &line2Id, const quint32 &line3Id,
                           const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // VTOOLLINEINTERSECTAXIS_H

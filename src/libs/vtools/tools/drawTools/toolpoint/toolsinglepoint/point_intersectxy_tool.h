/**************************************************************************
 **
 **  @file   point_intersectxy_tool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @author Douglas S Caskey
 **  @date   7.21.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#ifndef POINT_INTERSECTXY_TOOL_H
#define POINT_INTERSECTXY_TOOL_H

#include "toollinepoint/doubleline_point_tool.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

template <class T> class QSharedPointer;

/**
 * @brief The PointIntersectXYTool class tool for creation point intersection two lines.
 */
class PointIntersectXYTool : public DoubleLinePointTool
{
    Q_OBJECT
public:
    virtual void         setDialog() Q_DECL_OVERRIDE;

    static PointIntersectXYTool *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene,
                                            VAbstractPattern *doc, VContainer *data);
    static PointIntersectXYTool *Create(const quint32 _id, const QString &pointName, const QString &lineType,
                                            const QString &lineWeight,
                                            const QString &lineColor, const quint32 &firstPointId,
                                            const quint32 &secondPointId, qreal mx, qreal my, bool showPointName,
                                            VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                            const Document &parse, const Source &typeCreation);

    static const QString ToolType;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                 {Type = UserType + static_cast<int>(Tool::PointOfIntersection)};

    QString              firstPointName() const;
    QString              secondPointName() const;

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
                         Q_DISABLE_COPY(PointIntersectXYTool)
                         PointIntersectXYTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                              const QString &lineType, const QString &lineWeight,
                                              const QString &lineColor,
                                              const quint32 &firstPointId, const quint32 &secondPointId,
                                              const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // POINT_INTERSECTXY_TOOL_H

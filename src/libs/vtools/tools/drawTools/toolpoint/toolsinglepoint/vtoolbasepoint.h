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
 **  @file   vtoolsinglepoint.h
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

#ifndef VTOOLBASEPOINT_H
#define VTOOLBASEPOINT_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolBasePoint class tool for creation pattern base point. Only base point can move. All object
 * pattern peace depend on base point.
 */
class VToolBasePoint : public VToolSinglePoint
{
    Q_OBJECT
public:
    virtual               ~VToolBasePoint() =default;
    virtual void           setDialog() override;

    static VToolBasePoint *Create(quint32 _id, const QString &activeDraftBlock, VPointF *point,
                                  VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                  const Document &parse, const Source &typeCreation);

    static const QString   ToolType;
    virtual int            type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::BasePoint)};

    virtual void           ShowVisualization(bool show) override;

    QPointF                GetBasePointPos() const;
    void                   SetBasePointPos(const QPointF &pos);

public slots:
    virtual void           FullUpdateFromFile() override;
    virtual void           EnableToolMove(bool move) override;

signals:
    /**
     * @brief FullUpdateTree handle if need update pattern file.
     */
    void                   LiteUpdateTree();

protected slots:
    virtual void           showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;

protected:
    virtual void           AddToFile() override;
    virtual QVariant       itemChange ( GraphicsItemChange change, const QVariant &value ) override;
    virtual void           deleteTool(bool ask = true) override;
    virtual void           SaveDialog(QDomElement &domElement) override;
    virtual void           hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) override;
    virtual void           hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) override;
    virtual void           mousePressEvent( QGraphicsSceneMouseEvent * event ) override;
    virtual void           mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) override;
    virtual void           SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void           ReadToolAttributes(const QDomElement &domElement) override;
    virtual void           SetVisualization() override {}
    virtual QString        makeToolTip() const override;

private:
    Q_DISABLE_COPY(VToolBasePoint)

    QString                draftBlockName;

                           VToolBasePoint (VAbstractPattern *doc, VContainer *data, quint32 id,
                                           const Source &typeCreation, const QString &draftBlockName,
                                           QGraphicsItem * parent = nullptr );
};

#endif // VTOOLBASEPOINT_H

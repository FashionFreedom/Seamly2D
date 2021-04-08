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
 **  @file   vtoolshoulderpoint.h
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

#ifndef VTOOLSHOULDERPOINT_H
#define VTOOLSHOULDERPOINT_H

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
 * @brief The VToolShoulderPoint class tool for creation point on shoulder. This tool for special situation, when you
 * want find point along line, but have only length from another point (shoulder).
 */
class VToolShoulderPoint : public VToolLinePoint
{
    Q_OBJECT
public:
    virtual void               setDialog() Q_DECL_OVERRIDE;
    static QPointF             FindPoint(const QPointF &p1Line, const QPointF &p2Line, const QPointF &pShoulder,
                                         const qreal &length);

    static VToolShoulderPoint *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene,
                                      VAbstractPattern *doc, VContainer *data);
    static VToolShoulderPoint *Create(const quint32 _id, QString &formula, quint32 p1Line, quint32 p2Line,
                                      quint32 pShoulder, const QString &typeLine, const QString &lineColor,
                                      const QString &pointName, qreal mx, qreal my, bool showPointName,
                                      VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                      const Document &parse,
                                      const Source &typeCreation);
                                      
    static const QString       ToolType;
    virtual int                type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::ShoulderPoint) };

    QString                    SecondPointName() const;
    QString                    ShoulderPointName() const;

    quint32                    GetP2Line() const;
    void                       SetP2Line(const quint32 &value);

    quint32                    getPShoulder() const;
    void                       setPShoulder(const quint32 &value);

    virtual void               ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void               showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;


protected:
    virtual void               RemoveReferens() Q_DECL_OVERRIDE;
    virtual void               SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void               SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void               ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void               SetVisualization() Q_DECL_OVERRIDE;
    virtual QString            makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolShoulderPoint)

    quint32                    p2Line;        /** @brief p2Line id second line point. */
    quint32                    pShoulder;     /** @brief pShoulder id shoulder line point. */

    VToolShoulderPoint(VAbstractPattern *doc, VContainer *data, const quint32 &id, const QString &typeLine,
                       const QString &lineColor, const QString &formula, const quint32 &p1Line, const quint32 &p2Line,
                       const quint32 &pShoulder, const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // VTOOLSHOULDERPOINT_H

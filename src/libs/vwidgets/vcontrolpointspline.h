//  @file   vcontrolpointspline.h
//  @author Douglas S Caskey
//  @date   2 Jun, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

/************************************************************************
 **
 **  @file   vcontrolpointspline.h
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

#ifndef VCONTROLPOINTSPLINE_H
#define VCONTROLPOINTSPLINE_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/def.h"
#include "scene_rect.h"

// @brief The VControlPointSpline class control spline point.
class VControlPointSpline : public QObject, public SceneRect
{
    Q_OBJECT
public:
                        VControlPointSpline(const qint32 &splineIndex, SplinePointPosition position,
                                            QGraphicsItem *parent = nullptr);
                        VControlPointSpline(const qint32 &splineIndex, SplinePointPosition position,
                                            const QPointF &controlPoint, bool freeAngle, bool freeLength,
                                            QGraphicsItem *parent = nullptr);
    virtual            ~VControlPointSpline() = default;

    virtual int         type() const Q_DECL_OVERRIDE {return Type;}
    enum                { Type = UserType + static_cast<int>(Vis::ControlPointSpline)};

    virtual void        paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                              QWidget *widget = nullptr) Q_DECL_OVERRIDE;
signals:
    // @brief controlPointPositionChanged emit when control point change position.
    // @param splineIndex index spline in list.
    // @param position position point in spline.
    // @param pos new posotion.
    void                controlPointPositionChanged(const qint32 &splineIndex, SplinePointPosition position,
                                                   const QPointF &pos);
    // @brief showContextMenu emit when need show tool's context menu.
    // @param event context menu event.
    void                showContextMenu(QGraphicsSceneContextMenuEvent *event);
    void                selected(bool selected);

public slots:
    void                refreshCtrlPoint(const qint32 &splineIndex, SplinePointPosition pos, const QPointF &controlPoint,
                                       const QPointF &splinePoint, bool freeAngle = true, bool freeLength = true);
    void                setEnabledPoint(bool enable);

protected:
    VScaledLine        *m_controlLine; /*!< @brief m_controlLine pointer to line control point. */

    virtual void        hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    virtual void        hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    QVariant            itemChange(GraphicsItemChange change, const QVariant &value) Q_DECL_OVERRIDE;
    virtual void        mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void        mouseReleaseEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void        contextMenuEvent(QGraphicsSceneContextMenuEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VControlPointSpline)
    qint32              m_splineIndex;   /*!< @brief m_splineIndex index of spline in list. */
    SplinePointPosition m_position;      /*!< @brief m_position position of point in spline. */

    bool                m_freeAngle;
    bool                m_freeLength;

    void                init();
    void                setCtrlLine(const QPointF &controlPoint, const QPointF &splinePoint);
};

#endif // VCONTROLPOINTSPLINE_H

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
 **  @date   14 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#ifndef VSCENEPOINT_H
#define VSCENEPOINT_H

#include <QtGlobal>
#include <QGraphicsEllipseItem>

#include "../vmisc/def.h"

class VGraphicsSimpleTextItem;
class VPointF;
class VScaledLine;

class VScenePoint: public QGraphicsEllipseItem
{
public:
    explicit                 VScenePoint(const QColor &lineColor, QGraphicsItem *parent = nullptr);
    virtual                 ~VScenePoint() = default;
    virtual int              type() const Q_DECL_OVERRIDE {return Type;}
                             enum { Type = UserType + static_cast<int>(Vis::ScenePoint)};

    virtual void             paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                   QWidget *widget = nullptr) Q_DECL_OVERRIDE;
    virtual void             refreshPointGeometry(const VPointF &point);

    void                     refreshLeader();

protected:

    VGraphicsSimpleTextItem *m_pointName;  /** @brief namePoint point text. */
    VScaledLine             *m_pointLeader;  /** @brief pointL line that we see if Text is moved too away from point. */
    QColor                   m_pointColor; /** @brief m_pointColor color of point. */
    bool                     m_onlyPoint;
    bool                     m_isHovered;
    bool                     m_showPointName;

    virtual void             hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    virtual void             hoverLeaveEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;

    void                     setOnlyPoint(bool value);
    bool                     isOnlyPoint() const;

    void                     setPointColor(const QString &value);

private:
    Q_DISABLE_COPY(VScenePoint)

    void                     setPointPen(qreal scale);
};

#endif // VSCENEPOINT_H

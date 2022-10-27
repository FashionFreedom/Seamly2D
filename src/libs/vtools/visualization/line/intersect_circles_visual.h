/**************************************************************************
 **
 **  @file   intersect_circles_visual.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
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

#ifndef INTERSECT_CIRCLES_VISUAL_H
#define INTERSECT_CIRCLES_VISUAL_H

#include "visline.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"

#include <QColor>
#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

class IntersectCirclesVisual : public VisLine
{
    Q_OBJECT

public:
    explicit              IntersectCirclesVisual(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual              ~IntersectCirclesVisual() = default;

    virtual void          RefreshGeometry() Q_DECL_OVERRIDE;
    virtual void          VisualMode(const quint32 &id) Q_DECL_OVERRIDE;

    void                  setObject2Id(const quint32 &value);
    void                  setC1Radius(const QString &value);
    void                  setC2Radius(const QString &value);
    void                  setCrossPoint(const CrossCirclesPoint &value);

    virtual int           type() const Q_DECL_OVERRIDE {return Type;}
    enum                  {Type = UserType + static_cast<int>(Vis::ToolPointOfIntersectionCircles)};

private:
                          Q_DISABLE_COPY(IntersectCirclesVisual)

    quint32               object2Id;
    qreal                 c1Radius;
    qreal                 c2Radius;
    CrossCirclesPoint     crossPoint;
    VScaledEllipse       *point;
    VScaledEllipse       *c1Center;
    VScaledEllipse       *c2Center;
    QGraphicsEllipseItem *c1Path;
    QGraphicsEllipseItem *c2Path;
    QColor                m_secondrySupportColor;
    QColor                m_tertiarySupportColor;
};

#endif // INTERSECT_CIRCLES_VISUAL_H

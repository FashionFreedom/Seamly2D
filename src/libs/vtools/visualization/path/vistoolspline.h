/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   vistoolspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 8, 2014
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

#ifndef VISTOOLSPLINE_H
#define VISTOOLSPLINE_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VControlPointSpline;

class VisToolSpline : public VisPath
{
    Q_OBJECT
public:
    explicit VisToolSpline(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual ~VisToolSpline();

    virtual void RefreshGeometry() override;

    void         setObject4Id(const quint32 &value);
    void         SetAngle1(const qreal &value);
    void         SetAngle2(const qreal &value);
    void         SetKAsm1(const qreal &value);
    void         SetKAsm2(const qreal &value);
    void         SetKCurve(const qreal &value);

    QPointF      GetP2() const;
    QPointF      GetP3() const;

    void         setShowPoints(const QPointF &p1, const QPointF &p2,
                               const QPointF &p3, const QPointF &p4);

    virtual int  type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::ToolSpline)};
public slots:
    void MouseLeftPressed();
    void MouseLeftReleased();

protected:
    Q_DISABLE_COPY(VisToolSpline)
    quint32         object4Id;
    VScaledEllipse *point1;
    VScaledEllipse *point4;
    qreal           angle1;
    qreal           angle2;
    qreal           kAsm1;
    qreal           kAsm2;
    qreal           kCurve;

    bool isLeftMousePressed;
    bool p2Selected;
    bool p3Selected;
    bool m_showPointsSet;
    QPointF m_showP1;
    QPointF m_showP2;
    QPointF m_showP3;
    QPointF m_showP4;

    QPointF p2;
    QPointF p3;

    QVector<VControlPointSpline *> controlPoints;
};

#endif // VISTOOLSPLINE_H

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
 **  @file   vistoolcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  @brief  Parametric visualization for the cubic Bézier tool.
 **          Displays two anchor points (P1, P4), two computed handle dots
 **          (P2, P3), and the resulting curve preview.
 *************************************************************************/

#ifndef VISTOOLCUBICBEZIER_H
#define VISTOOLCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCubicBezier : public VisPath
{
    Q_OBJECT
public:
    explicit VisToolCubicBezier(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual ~VisToolCubicBezier() Q_DECL_EQ_DEFAULT;

    virtual void RefreshGeometry() override;

    // IDs for creation-mode point selection (4-click flow)
    void         setObject2Id(const quint32 &value);
    void         setObject3Id(const quint32 &value);
    void         setObject4Id(const quint32 &value);

    // Angle/length for Show mode (after creation, states 2/3/4)
    void         setAngle1(const QString &value);
    void         setAngle2(const QString &value);
    void         setC1Length(const QString &value);
    void         setC2Length(qreal value);   // actual computed c2 in pixels

    virtual int  type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::ToolCubicBezier)};

protected:
    Q_DISABLE_COPY(VisToolCubicBezier)

    quint32         object2Id;     // P2 canvas point (creation mode)
    quint32         object3Id;     // P3 canvas point (creation mode)
    quint32         object4Id;     // P4 endpoint
    QString         m_angle1;      // Show mode: angle from computed P1→P2'
    QString         m_angle2;      // Show mode: angle from computed P4→P3'
    QString         m_c1Length;    // Show mode: c1 length (user units string)
    qreal           m_c2Length;    // Show mode: c2 length (pixels); -1 = use c1 as proxy

    VScaledEllipse *point1;
    VScaledEllipse *point4;
    VScaledEllipse *point2;
    VScaledEllipse *point3;
    VScaledLine    *helpLine1;
    VScaledLine    *helpLine2;
};

#endif // VISTOOLCUBICBEZIER_H

//---------------------------------------------------------------------------------------------------------------------
//  @file   intersect_arc_line_visual.h
//  @author Douglas S Caskey
//  @date   25 Srp, 2026
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vistoolpointofcontact.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   14 Aug, 2014
//
//  @copyright
//  Copyright (C) 2014 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------


#ifndef INTERSECT_ARC_LINE_VISUAL_H
#define INTERSECT_ARC_LINE_VISUAL_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class IntersectArcLineVisual : public VisLine
{
    Q_OBJECT
public:
    explicit              IntersectArcLineVisual(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual              ~IntersectArcLineVisual() = default;

    virtual void          RefreshGeometry() override;
    void                  setLineP2Id(const quint32 &value);
    void                  setRadiusId(const quint32 &value);
    void                  setRadius(const QString &expression);
    virtual int           type() const override {return Type;}
    enum                  {Type = UserType + static_cast<int>(Vis::ToolIntersectArcLine)};

private:
    Q_DISABLE_COPY(IntersectArcLineVisual)
    quint32               m_line_point2_id;
    quint32               m_radius_id;
    VScaledEllipse       *m_point;
    VScaledEllipse       *m_line_point1;
    VScaledEllipse       *m_line_point2;
    VScaledEllipse       *m_arc_point;
    QGraphicsEllipseItem *m_circle;
    qreal                 m_radius;

};

#endif // INTERSECT_ARC_LINE_VISUAL_H

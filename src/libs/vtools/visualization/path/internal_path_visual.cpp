//---------------------------------------------------------------------------------------------------------------------
//  @file   internal_path_visual.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
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
//  @file   vistoolinternalpath.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   22 11, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "internal_path_visual.h"
#include "../vwidgets/vsimplepoint.h"
#include "../vgeometry/vpointf.h"
#include "../vwidgets/scalesceneitems.h"

#include <QGraphicsSceneMouseEvent>

//---------------------------------------------------------------------------------------------------------------------
InternalPathVisual::InternalPathVisual(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , m_points()
    , m_line(nullptr)
    , m_path()
    , m_cutLinePath()
{
    m_line = InitItem<VScaledLine>(supportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathVisual::RefreshGeometry()
{
    hideAllItems();

    if (m_path.nodeCount() > 0)
    {
        DrawPath(this, m_path.PainterPath(Visualization::data, m_cutLinePath), mainColor, m_path.getLineType(),
                 Qt::RoundCap);

        const QVector<VPointF> nodes = m_path.PathNodePoints(Visualization::data);

        for (int i = 0; i < nodes.size(); ++i)
        {
            VSimplePoint *point = getPoint(static_cast<quint32>(i), supportColor);
            point->setOnlyPoint(mode == Mode::Creation);
            point->refreshPointGeometry(nodes.at(i));
            point->setVisible(true);
        }

        if (mode == Mode::Creation)
        {
            const QVector<QPointF> points = m_path.PathPoints(Visualization::data);
            if (points.size() > 0)
            {
                DrawLine(m_line, QLineF(points.last(), Visualization::scenePos), supportColor,
                                        lineWeight, Qt::DashLine);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathVisual::setPath(const VPiecePath &path)
{
    m_path = path;
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathVisual::setCutPath(const QVector<QPointF> &path)
{
    m_cutLinePath = path;
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathVisual::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint *InternalPathVisual::getPoint(quint32 i, const QColor &color)
{
    return VisPath::GetPoint(m_points, i, color);
}

//---------------------------------------------------------------------------------------------------------------------
void InternalPathVisual::hideAllItems()
{
    if (m_line)
    {
        m_line->setVisible(false);
    }

    for (int i=0; i < m_points.size(); ++i)
    {
        if (QGraphicsEllipseItem *item = m_points.at(i))
        {
            item->setVisible(false);
        }
    }
}

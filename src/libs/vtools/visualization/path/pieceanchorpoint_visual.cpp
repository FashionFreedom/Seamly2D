/******************************************************************************
 *   @file   pieceanchorpoint_visual.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Seamly2D project
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
 *****************************************************************************/

#include "pieceanchorpoint_visual.h"
#include "../vwidgets/vsimplepoint.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
PieceAnchorPointVisual::PieceAnchorPointVisual(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , m_points()
    , m_anchors()
{
}

//---------------------------------------------------------------------------------------------------------------------
void PieceAnchorPointVisual::RefreshGeometry()
{
    HideAllItems();

    for (int i = 0; i < m_anchors.size(); ++i)
    {
        VSimplePoint *point = GetPoint(static_cast<quint32>(i), supportColor);
        point->setOnlyPoint(false);
        const QSharedPointer<VPointF> p = Visualization::data->GeometricObject<VPointF>(m_anchors.at(i));
        point->refreshPointGeometry(*p);
        point->setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PieceAnchorPointVisual::setAnchors(const QVector<quint32> &anchors)
{
    m_anchors = anchors;
}

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint *PieceAnchorPointVisual::GetPoint(quint32 i, const QColor &color)
{
    return VisPath::GetPoint(m_points, i, color);
}

//---------------------------------------------------------------------------------------------------------------------
void PieceAnchorPointVisual::HideAllItems()
{
    for (int i=0; i < m_points.size(); ++i)
    {
        if (QGraphicsEllipseItem *item = m_points.at(i))
        {
            item->setVisible(false);
        }
    }
}

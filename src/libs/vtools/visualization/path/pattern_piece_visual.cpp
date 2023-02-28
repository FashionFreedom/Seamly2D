/***************************************************************************
 **  @file   pattern_piece_visual.cpp
 **  @author Douglas S Caskey
 **  @date   Dec 11, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vistoolpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "pattern_piece_visual.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vgeometry/vpointf.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
PatternPieceVisual::PatternPieceVisual(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
    , m_points()
    , m_line1(nullptr)
    , m_line2(nullptr)
    , m_piece()
{
    m_line1 = InitItem<VScaledLine>(supportColor, this);
    m_line2 = InitItem<VScaledLine>(supportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceVisual::RefreshGeometry()
{
    HideAllItems();

    if (m_piece.GetPath().CountNodes() > 0)
    {
        DrawPath(this, m_piece.MainPathPath(Visualization::data), mainColor, Qt::SolidLine, lineWeight, Qt::RoundCap);

        const QVector<VPointF> nodes = m_piece.MainPathNodePoints(Visualization::data);

        for (int i = 0; i < nodes.size(); ++i)
        {
            VScaledEllipse *point = GetPoint(static_cast<quint32>(i), supportColor);
            DrawPoint(point, nodes.at(i).toQPointF(), supportColor);
        }

        if (mode == Mode::Creation)
        {
            const QVector<QPointF> points = m_piece.MainPathPoints(Visualization::data);
            DrawLine(m_line1, QLineF(points.first(), Visualization::scenePos), supportColor, lineWeight, Qt::DashLine);

            if (points.size() > 1)
            {
                DrawLine(m_line2, QLineF(points.last(), Visualization::scenePos), supportColor,
                                         lineWeight, Qt::DashLine);
            }
            VScaledEllipse *point = GetPoint(static_cast<quint32>(i), mainColor);
            point->setBrush(QBrush(mainColor, Qt::SolidPattern));
            DrawPoint(point, nodes.at(i).toQPointF(), mainColor);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceVisual::SetPiece(const VPiece &piece)
{
    m_piece = piece;
}

//---------------------------------------------------------------------------------------------------------------------
VScaledEllipse *PatternPieceVisual::GetPoint(quint32 i, const QColor &color)
{
    return GetPointItem(m_points, i, color, this);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceVisual::HideAllItems()
{
    if (m_line1)
    {
        m_line1->setVisible(false);
    }

    if (m_line2)
    {
        m_line2->setVisible(false);
    }

    for (int i=0; i < m_points.size(); ++i)
    {
        if (QGraphicsEllipseItem *item = m_points.at(i))
        {
            item->setVisible(false);
        }
    }
}

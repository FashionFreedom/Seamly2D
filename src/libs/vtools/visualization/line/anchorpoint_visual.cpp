/************************************************************************
 **
 **  @file   anchorpoint_visual.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
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

#include "anchorpoint_visual.h"
#include "../vwidgets/vsimplepoint.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
AnchorPointVisual::AnchorPointVisual(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
    , m_point()
{
    this->mainColor = Qt::red;
    this->setZValue(2);// Show on top real tool

    m_point = new VSimplePoint(NULL_ID, mainColor);
    m_point->SetPointHighlight(true);
    m_point->setParentItem(this);
    m_point->SetVisualizationMode(true);
    m_point->setVisible(false);
}

//---------------------------------------------------------------------------------------------------------------------
AnchorPointVisual::~AnchorPointVisual()
{
    if (not m_point.isNull())
    {
        delete m_point;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointVisual::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> point = Visualization::data->GeometricObject<VPointF>(object1Id);

        m_point->setOnlyPoint(mode == Mode::Creation);
        m_point->refreshPointGeometry(*point);
        m_point->setVisible(true);
    }
}

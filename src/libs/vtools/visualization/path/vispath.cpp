/**************************************************************************
 **
 **  @file   vispath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
 **
 **  @author Douglas S Caskey
 **  @date   7.23.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#include "vispath.h"

#include "../visualization.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vsimplepoint.h"

#include <QPen>

//---------------------------------------------------------------------------------------------------------------------
VisPath::VisPath(const VContainer *data, QGraphicsItem *parent)
    : Visualization(data)
    , VCurvePathItem(parent)
{
    this->setZValue(1);// Show on top real tool
    initPen();
}

//---------------------------------------------------------------------------------------------------------------------
void VisPath::initPen()
{
    QPen visPen = pen();
    visPen.setColor(mainColor);
    visPen.setStyle(lineStyle);
    visPen.setWidthF(lineWeight);

    this->setPen(visPen);
}

//---------------------------------------------------------------------------------------------------------------------
void VisPath::AddOnScene()
{
    addItem(this);
}

//---------------------------------------------------------------------------------------------------------------------
VSimplePoint *VisPath::GetPoint(QVector<VSimplePoint *> &points, quint32 i, const QColor &color)
{
    if (not points.isEmpty() && static_cast<quint32>(points.size() - 1) >= i)
    {
        return points.at(static_cast<int>(i));
    }
    else
    {
        VSimplePoint *point = new VSimplePoint(NULL_ID, color);
        point->SetPointHighlight(true);
        point->setParentItem(this);
        point->SetVisualizationMode(true);
        points.append(point);

        return point;
    }
    return nullptr;
}

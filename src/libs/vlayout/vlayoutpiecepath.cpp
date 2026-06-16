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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 2, 2017
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

#include "vlayoutpiecepath.h"
#include "vlayoutpiecepath_p.h"
#include "vlayoutdef.h"

#include <QPainterPath>

#ifdef Q_COMPILER_RVALUE_REFS
VLayoutPiecePath &VLayoutPiecePath::operator=(VLayoutPiecePath &&path) noexcept { Swap(path); return *this; }
#endif

void VLayoutPiecePath::Swap(VLayoutPiecePath &path) noexcept
{ std::swap(d, path.d); }

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath()
    : d(new VLayoutPiecePathData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath(const QVector<QPointF> &points, QString color, Qt::PenStyle lineType,
                                   QString lineWeight, bool cut)
    : d(new VLayoutPiecePathData(points, color, lineType, lineWeight, cut))
{
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath(const VLayoutPiecePath &path)
    : d(path.d)
{
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath &VLayoutPiecePath::operator=(const VLayoutPiecePath &path)
{
    if ( &path == this )
    {
        return *this;
    }
    d = path.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::~VLayoutPiecePath()
{
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VLayoutPiecePath::GetPainterPath() const
{
    QPainterPath path;
    if (not d->m_points.isEmpty())
    {
        path.addPolygon(QPolygonF(d->m_points));
        path.setFillRule(Qt::WindingFill);
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VLayoutPiecePath::Points() const
{
    return d->m_points;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::SetPoints(const QVector<QPointF> &points)
{
    d->m_points = points;
}

//---------------------------------------------------------------------------------------------------------------------
QString VLayoutPiecePath::getLineColor() const
{
    return d->m_lineColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::setLineColor(const QString &color)
{
    d->m_lineColor = color;
}

//---------------------------------------------------------------------------------------------------------------------
Qt::PenStyle VLayoutPiecePath::getLineType() const
{
    return d->m_lineType;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::setLineType(const Qt::PenStyle &lineType)
{
    d->m_lineType = lineType;
}

//---------------------------------------------------------------------------------------------------------------------
QString VLayoutPiecePath::getLineWeight() const
{
    return d->m_lineWeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::setLineWeight(const QString &weight)
{
    d->m_lineWeight = weight;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPiecePath::isCutPath() const
{
    return d->m_cut;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::setCutPath(bool cut)
{
    d->m_cut = cut;
}

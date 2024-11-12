//---------------------------------------------------------------------------------------------------------------------
//  @file   vgrainlinedata.cpp
//  @author Douglas S Caskey
//  @date   11 Nov, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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
//  @file   vgrainlinedata.cpp
//  @author Bojan Kverh
//  @date   September 06, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
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

#include <QPointF>

#include "vgrainlinedata.h"
#include "vgrainlinedata_p.h"

#ifdef Q_COMPILER_RVALUE_REFS
VGrainlineData &VGrainlineData::operator=(VGrainlineData &&data) Q_DECL_NOTHROW { Swap(data); return *this; }
#endif

void VGrainlineData::Swap(VGrainlineData &data) Q_DECL_NOTHROW
{ VAbstractFloatItemData::Swap(data); std::swap(d, data.d); }

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::VGrainlineData()
    : VAbstractFloatItemData()
    , d(new VGrainlineDataPrivate())
{}

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::VGrainlineData(const VGrainlineData &data)
    : VAbstractFloatItemData(data)
    , d (data.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData &VGrainlineData::operator=(const VGrainlineData &data)
{
    if ( &data == this )
    {
        return *this;
    }
    VAbstractFloatItemData::operator=(data);
    d = data.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::~VGrainlineData()
{}

//---------------------------------------------------------------------------------------------------------------------
QString VGrainlineData::getLength() const
{
    return d->m_length;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setLength(const QString& length)
{
    d->m_length = length;
}

//---------------------------------------------------------------------------------------------------------------------
QString VGrainlineData::getRotation() const
{
    return d->m_rotation;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setRotation(const QString& rotation)
{
    d->m_rotation = rotation;
}

//---------------------------------------------------------------------------------------------------------------------
ArrowType VGrainlineData::getArrowType() const
{
    return d->m_arrowType;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setArrowType(ArrowType type)
{
    d->m_arrowType = type;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VGrainlineData::centerAnchorPoint() const
{
    return d->m_centerAnchorPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setCenterAnchorPoint(quint32 centerAnchor)
{
    d->m_centerAnchorPoint = centerAnchor;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VGrainlineData::topAnchorPoint() const
{
    return d->m_topAnchorPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setTopAnchorPoint(quint32 topAnchorPoint)
{
    d->m_topAnchorPoint = topAnchorPoint;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VGrainlineData::bottomAnchorPoint() const
{
    return d->m_bottomAnchorPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::setBottomAnchorPoint(quint32 bottomAnchorPoint)
{
    d->m_bottomAnchorPoint = bottomAnchorPoint;
}

/******************************************************************************
*   @file   vfabricsettings.h
**  @author Douglas S Caskey
**  @date   27 Jun, 2026
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2026 Seamly2D project
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

#ifndef VFABRICSETTINGS_H
#define VFABRICSETTINGS_H

#include <QtGlobal>

struct VFabricSettings
{
    qreal fabricWidth      = 0;
    qreal heightRepeat     = 0;
    qreal lengthRepeat     = 0;
    qreal shrinkagePercent = 0;
    qreal stretchPercent   = 0;
    qreal defaultSAWidth   = 0;

    bool operator==(const VFabricSettings &other) const
    {
        return qFuzzyCompare(fabricWidth, other.fabricWidth)
            && qFuzzyCompare(heightRepeat, other.heightRepeat)
            && qFuzzyCompare(lengthRepeat, other.lengthRepeat)
            && qFuzzyCompare(shrinkagePercent, other.shrinkagePercent)
            && qFuzzyCompare(stretchPercent, other.stretchPercent)
            && qFuzzyCompare(defaultSAWidth, other.defaultSAWidth);
    }

    bool operator!=(const VFabricSettings &other) const
    {
        return !(*this == other);
    }
};

#endif // VFABRICSETTINGS_H

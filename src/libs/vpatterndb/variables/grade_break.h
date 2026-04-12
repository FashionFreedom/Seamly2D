//-----------------------------------------------------------------------------
//   @file   grade_break.h
//  @date   29 Mar, 2026
//
//  @brief  GradeBreak struct for piecewise (non-linear) grading.
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
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
//-----------------------------------------------------------------------------

#ifndef GRADE_BREAK_H
#define GRADE_BREAK_H

#include <QtGlobal>

struct GradeBreak
{
    qreal threshold;  // size or height value where this increment takes effect
    qreal increment;  // increment per step from this threshold onward

    GradeBreak() : threshold(0), increment(0) {}
    GradeBreak(qreal threshold, qreal increment) : threshold(threshold), increment(increment) {}

    bool operator<(const GradeBreak &other) const
    {
        return threshold < other.threshold; 
    }
    bool operator==(const GradeBreak &other) const
    {
        return qFuzzyCompare(threshold, other.threshold) && qFuzzyCompare(increment, other.increment);
    }
};

#endif // GRADE_BREAK_H

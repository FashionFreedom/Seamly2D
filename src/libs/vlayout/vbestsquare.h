/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file   vbestsquare.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VBESTSQUARE_H
#define VBESTSQUARE_H

#include <QSizeF>
#include <QTransform>
#include <QtGlobal>

#include "vlayoutdef.h"

class VBestSquare
{
public:
    VBestSquare(const QSizeF &sheetSize, bool saveLength);

    void NewResult(const QSizeF &candidate, int i, int j, const QTransform &transform, bool mirror, BestFrom type);
    void NewResult(const VBestSquare &best);

    QSizeF     BestSize() const;
    int        GContourEdge() const;
    int        DetailEdge() const;
    QTransform Transform() const;
    bool       ValidResult() const;
    bool       isMirror() const;
    BestFrom   Type() const;

    bool IsSaveLength() const;

private:
    // All needed information for best result
    int resI; // Edge of global contour
    int resJ; // Edge of detail
    QTransform resTransform; // Transform for rotation and translation detail
    QSizeF bestSize;
    qreal sheetWidth;
    bool valideResult;
    bool resMirror;
    BestFrom type;
    bool saveLength;

    static qint64 Square(const QSizeF &size);
};

#endif // VBESTSQUARE_H

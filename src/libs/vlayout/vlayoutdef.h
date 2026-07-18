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
 **  @file   vlayoutdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2015
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

#ifndef VLAYOUTDEF_H
#define VLAYOUTDEF_H

#ifdef Q_CC_MSVC
    #include <ciso646>
#endif /* Q_CC_MSVC */

enum class LayoutErrors : char
{
    NoError,
    PrepareLayoutError,
    ProcessStoped,
    EmptyPaperError
};

enum class BestFrom : char
{
    Rotation = 0,
    Combine = 1
};

/**
 * @brief QGraphicsItem::data() keys shared between the layout piece items and the SVG exporter.
 *
 * VLayoutPiece::GetItem() tags every piece component item with these keys so that
 * SvgGenerator can identify each rendered component and write the matching
 * data-* attributes into the exported SVG (see status-docs/new-attributes.csv).
 * The keys live in a namespace so the enumerator names cannot collide with
 * identifiers elsewhere in the codebase (e.g. VDrawTool::ObjectName).
 */
namespace PieceItemData
{
    enum Key : int
    {
        ObjectName  = 0, /**< Piece name on the root item (legacy convention, kept for the SVG group id). */
        ItemType    = 1, /**< SVG data-type string: seamline|cutline|notch|internal_path|cut_path|grainline|piece_label|pattern_label. */
        PieceLetter = 2  /**< Piece letter, exported as the data-letter attribute when set. */
    };
}
#endif // VLAYOUTDEF_H

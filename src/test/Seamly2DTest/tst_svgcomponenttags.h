/******************************************************************************
 **  @file   tst_svgcomponenttags.h
 **  @author slspencer
 **  @date   July 18, 2026
 **
 **  @brief
 **  Unit tests for the SVG component data-type tagging of piece items
 **  (internal_path vs cut_path).
 **
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Seamly2D Project
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

#ifndef TST_SVGCOMPONENTTAGS_H
#define TST_SVGCOMPONENTTAGS_H

#include <QObject>

/**
 * @brief TST_SvgComponentTags tests the SVG data-type tagging contract of the
 * piece component item tree (VLayoutPiece::GetItem()) and of the tagged SVG
 * produced by SvgGenerator, with focus on the internal_path / cut_path split.
 */
class TST_SvgComponentTags : public QObject
{
    Q_OBJECT
public:
    explicit TST_SvgComponentTags(QObject *parent = nullptr);

private slots:
    void CutoutTaggedAsCutPath() const;
    void ExportedSvgTagsCutPathGroups() const;
};

#endif // TST_SVGCOMPONENTTAGS_H

/******************************************************************************
 **  @file   tst_svgtextitem.h
 **  @author slspencer
 **  @date   July 18, 2026
 **
 **  @brief
 **  Unit tests for SvgTextItem (real-text SVG export of labels)
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

#ifndef TST_SVGTEXTITEM_H
#define TST_SVGTEXTITEM_H

#include <QObject>

/**
 * @brief TST_SvgTextItem unit tests for the SvgTextItem graphics item.
 *
 * Verifies the two guarantees the item exists for: SVG exports of the item
 * contain real <text> elements (not glyph-outline paths), and the item is
 * still discovered by the DXF export's QGraphicsSimpleTextItem walk.
 */
class TST_SvgTextItem : public QObject
{
    Q_OBJECT

public:
    explicit TST_SvgTextItem(QObject *parent = nullptr);

private slots:
    void FoundAsSimpleTextItem() const;
    void SvgExportEmitsRealText() const;
    void SvgExportKeepsFontStyle() const;
    void MultiLineTextDrawsEachLine() const;
};

#endif // TST_SVGTEXTITEM_H

/******************************************************************************
 **  @file   svg_text_item.h
 **  @author slspencer
 **  @date   July 18, 2026
 **
 **  @brief
 **  Text graphics item that exports as real text elements
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

#ifndef SVG_TEXT_ITEM_H
#define SVG_TEXT_ITEM_H

#include <QGraphicsSimpleTextItem>

/**
 * @brief SvgTextItem text item whose painting goes through QPainter::drawText().
 *
 * QGraphicsSimpleTextItem paints its text through a QTextLayout with an
 * explicit outline format whenever a pen is set, and Qt converts such text to
 * filled glyph outlines before it reaches the paint engine. As a result
 * exports through QSvgGenerator emit <path> outlines instead of real <text>
 * elements.
 *
 * This subclass keeps all the QGraphicsSimpleTextItem behavior (text, font,
 * brush, boundingRect(), type() — so DXF export's collectTextItems() walk
 * still finds it via qgraphicsitem_cast) but overrides paint() to draw the
 * string with plain QPainter::drawText() calls. Those reach the active paint
 * engine as QPaintEngine::drawTextItem() calls, which QSvgGenerator writes as
 * real <text> elements (and the PDF/DXF engines as their native text records).
 *
 * The text fill color is taken from the item's brush; the inherited pen is
 * intentionally ignored (an outline pen is what forces the glyph-outline
 * conversion this class exists to avoid).
 */
class SvgTextItem : public QGraphicsSimpleTextItem
{
public:
    /**
     * @brief SvgTextItem constructor.
     * @param parent optional parent item taking ownership.
     */
    explicit SvgTextItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief paint draws the text with QPainter::drawText() so paint engines
     * receive real text draw calls.
     * @param painter painter to draw with.
     * @param option unused style options.
     * @param widget unused widget pointer.
     */
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};

#endif // SVG_TEXT_ITEM_H

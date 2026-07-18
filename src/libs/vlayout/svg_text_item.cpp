/******************************************************************************
 **  @file   svg_text_item.cpp
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

#include "svg_text_item.h"

#include <QFontMetricsF>
#include <QPainter>
#include <QPen>
#include <QStringList>

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SvgTextItem constructor.
 * @param parent optional parent item taking ownership.
 */
SvgTextItem::SvgTextItem(QGraphicsItem *parent)
    : QGraphicsSimpleTextItem(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief paint draws the text with QPainter::drawText() so paint engines
 * receive real text draw calls.
 *
 * Workflow: the item's font is applied to the painter, the fill color is
 * applied as a solid pen (paint engines take the text fill from the painter's
 * pen — QSvgGenerator even skips text entirely when the pen is NoPen), and
 * every line of the text is drawn at its baseline with QPainter::drawText().
 * QPainter forwards each call to QPaintEngine::drawTextItem(), which the SVG
 * engine writes as a <text> element, the PDF engine as searchable text, and
 * the DXF engine as a TEXT entity; raster engines (PNG, screen) rasterize the
 * glyphs as before.
 *
 * Geometry matches QGraphicsSimpleTextItem: the first line's top edge sits at
 * y = 0 (drawText() takes a baseline point, so the baseline is offset by the
 * font's ascent) and subsequent lines advance by the font's line spacing.
 *
 * @param painter painter to draw with.
 * @param option unused style options.
 * @param widget unused widget pointer.
 */
void SvgTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setFont(font());
    painter->setPen(QPen(brush().color()));

    const QFontMetricsF metrics(font());
    qreal baseline = metrics.ascent();

    // Label lines are usually single-line strings, but honor embedded
    // newlines the same way the base class does: one drawn line each.
    const QStringList lines = text().split(QLatin1Char('\n'));
    for (int i = 0; i < lines.size(); ++i)
    {
        painter->drawText(QPointF(0.0, baseline), lines.at(i));
        baseline += metrics.lineSpacing();
    }
}

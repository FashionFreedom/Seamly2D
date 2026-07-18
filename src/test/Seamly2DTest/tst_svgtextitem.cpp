/******************************************************************************
 **  @file   tst_svgtextitem.cpp
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

#include "tst_svgtextitem.h"
#include "../vlayout/svg_text_item.h"

#include <QtTest>
#include <QBuffer>
#include <QFont>
#include <QGraphicsScene>
#include <QPainter>
#include <QSvgGenerator>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief renderItemToSvg renders one graphics item through QSvgGenerator.
 *
 * Mirrors the export pipeline of SvgGenerator::renderSceneToDom(): the item is
 * placed in a scene and the scene is painted into an in-memory SVG document,
 * which is returned as markup text for the assertions.
 *
 * @param item heap-allocated item; ownership passes to the scene.
 * @return the generated SVG markup.
 */
QString renderItemToSvg(QGraphicsItem *item)
{
    QGraphicsScene scene;
    scene.addItem(item);

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);
    generator.setSize(QSize(200, 100));
    generator.setViewBox(QRectF(0, 0, 200, 100));
    generator.setResolution(96);

    QPainter painter;
    painter.begin(&generator);
    scene.render(&painter, QRectF(0, 0, 200, 100), QRectF(0, 0, 200, 100), Qt::IgnoreAspectRatio);
    painter.end();
    buffer.close();

    return QString::fromUtf8(byteArray);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TST_SvgTextItem constructor.
 * @param parent optional owning QObject.
 */
TST_SvgTextItem::TST_SvgTextItem(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FoundAsSimpleTextItem checks the DXF export discovery contract.
 *
 * collectTextItems() (mainwindowsnogui.cpp) finds label lines by comparing
 * type() to QGraphicsSimpleTextItem::Type and casting with qgraphicsitem_cast;
 * both must keep working for SvgTextItem.
 */
void TST_SvgTextItem::FoundAsSimpleTextItem() const
{
    SvgTextItem item;
    item.setText(QStringLiteral("Front Bodice"));

    QCOMPARE(item.type(), static_cast<int>(QGraphicsSimpleTextItem::Type));

    QGraphicsItem *asBase = &item;
    QGraphicsSimpleTextItem *cast = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(asBase);
    QVERIFY2(cast != nullptr, "qgraphicsitem_cast to QGraphicsSimpleTextItem must succeed for SvgTextItem");
    QCOMPARE(cast->text(), QStringLiteral("Front Bodice"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SvgExportEmitsRealText checks that the SVG output contains a real
 * <text> element with the item's string, not glyph-outline paths.
 */
void TST_SvgTextItem::SvgExportEmitsRealText() const
{
    SvgTextItem *item = new SvgTextItem();
    item->setFont(QFont(QStringLiteral("Arial"), 12));
    item->setText(QStringLiteral("Piece Label 1"));
    item->setBrush(QBrush(QColor(Qt::black)));

    const QString svg = renderItemToSvg(item);

    QVERIFY2(svg.contains(QLatin1String("<text")), "SVG export must contain a <text> element");
    QVERIFY2(svg.contains(QLatin1String("Piece Label 1")), "SVG export must contain the label string");
    // The label text must not have been converted to glyph outlines: the only
    // scene content is the text item, so no <path> may appear at all.
    QVERIFY2(!svg.contains(QLatin1String("<path")), "SVG export must not contain glyph-outline paths");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SvgExportKeepsFontStyle checks that font family, bold and italic
 * survive into the <text> element's styling attributes.
 */
void TST_SvgTextItem::SvgExportKeepsFontStyle() const
{
    QFont font(QStringLiteral("Arial"), 14);
    font.setBold(true);
    font.setItalic(true);

    SvgTextItem *item = new SvgTextItem();
    item->setFont(font);
    item->setText(QStringLiteral("Styled Label"));
    item->setBrush(QBrush(QColor(QStringLiteral("#0000ff"))));

    const QString svg = renderItemToSvg(item);

    QVERIFY2(svg.contains(QLatin1String("<text")), "SVG export must contain a <text> element");
    QVERIFY2(svg.contains(QLatin1String("Arial")), "font-family must carry the label font");
    QVERIFY2(svg.contains(QLatin1String("font-weight=\"700\"")), "bold must map to font-weight 700");
    QVERIFY2(svg.contains(QLatin1String("font-style=\"italic\"")), "italic must map to font-style italic");
    QVERIFY2(svg.contains(QLatin1String("#0000ff")), "the brush color must be the text fill color");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MultiLineTextDrawsEachLine checks that embedded newlines produce one
 * drawn line (one <text> element) each, like the base class behavior.
 */
void TST_SvgTextItem::MultiLineTextDrawsEachLine() const
{
    SvgTextItem *item = new SvgTextItem();
    item->setFont(QFont(QStringLiteral("Arial"), 10));
    item->setText(QStringLiteral("Line one\nLine two"));
    item->setBrush(QBrush(QColor(Qt::black)));

    const QString svg = renderItemToSvg(item);

    QCOMPARE(svg.count(QLatin1String("<text")), 2);
    QVERIFY(svg.contains(QLatin1String("Line one")));
    QVERIFY(svg.contains(QLatin1String("Line two")));
}

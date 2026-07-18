/******************************************************************************
 **  @file   tst_svgcomponenttags.cpp
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

#include "tst_svgcomponenttags.h"
#include "../vformat/svg_generator.h"
#include "../vlayout/vlayoutdef.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpiecepath.h"

#include <QtTest>
#include <QDomDocument>
#include <QDomElement>
#include <QGraphicsItem>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QScopedPointer>
#include <QTemporaryDir>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief squarePoints builds a closed square contour used as path geometry.
 * @param x    left coordinate.
 * @param y    top coordinate.
 * @param side side length.
 * @return the four corner points of the square.
 */
QVector<QPointF> squarePoints(qreal x, qreal y, qreal side)
{
    QVector<QPointF> points;
    points << QPointF(x, y) << QPointF(x + side, y) << QPointF(x + side, y + side) << QPointF(x, y + side);
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief makePath wraps a point list in a VLayoutPiecePath with test styling.
 * @param points path geometry.
 * @param cut    true to mark the path as a cutout (cut path).
 * @return the layout piece path.
 */
VLayoutPiecePath makePath(const QVector<QPointF> &points, bool cut)
{
    return VLayoutPiecePath(points, QStringLiteral("black"), Qt::SolidLine, QStringLiteral("0.35"), cut);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief makeTestPiece builds a minimal piece: a square contour holding one
 * plain internal path and two cutout paths.
 * @return the layout piece.
 */
VLayoutPiece makeTestPiece()
{
    VLayoutPiece piece;
    piece.SetCountourPoints(squarePoints(10, 10, 180));
    piece.SetName(QStringLiteral("Test Piece"));

    QVector<VLayoutPiecePath> internalPaths;
    internalPaths.append(makePath(squarePoints(20, 20, 30), false));
    piece.setInternalPaths(internalPaths);

    QVector<VLayoutPiecePath> cutoutPaths;
    cutoutPaths.append(makePath(squarePoints(70, 70, 30), true));
    cutoutPaths.append(makePath(squarePoints(120, 120, 30), true));
    piece.setCutoutPaths(cutoutPaths);

    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief groupsOfType collects all <g> elements of the document carrying the
 * given data-type attribute value, in document order.
 * @param doc  parsed SVG document.
 * @param type wanted data-type value.
 * @return the matching group elements.
 */
QVector<QDomElement> groupsOfType(const QDomDocument &doc, const QString &type)
{
    QVector<QDomElement> result;
    const QDomNodeList groups = doc.elementsByTagName(QStringLiteral("g"));
    for (int i = 0; i < groups.size(); ++i)
    {
        const QDomElement group = groups.at(i).toElement();
        if (group.attribute(QStringLiteral("data-type")) == type)
        {
            result.append(group);
        }
    }
    return result;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TST_SvgComponentTags constructor.
 * @param parent optional owning QObject.
 */
TST_SvgComponentTags::TST_SvgComponentTags(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CutoutTaggedAsCutPath checks the item-tree contract: cutout paths get
 * the dedicated "cut_path" item type while plain internal paths keep
 * "internal_path".
 */
void TST_SvgComponentTags::CutoutTaggedAsCutPath() const
{
    const VLayoutPiece piece = makeTestPiece();
    const QScopedPointer<QGraphicsItem> root(piece.GetItem(true));

    int internalCount = 0;
    int cutoutCount = 0;
    const QList<QGraphicsItem *> components = root->childItems();
    for (int i = 0; i < components.size(); ++i)
    {
        const QString type = components.at(i)->data(PieceItemData::ItemType).toString();
        if (type == QLatin1String("internal_path"))
        {
            ++internalCount;
        }
        else if (type == QLatin1String("cut_path"))
        {
            ++cutoutCount;
        }
    }

    QCOMPARE(internalCount, 1);
    QCOMPARE(cutoutCount, 2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ExportedSvgTagsCutPathGroups checks the exported SVG end to end:
 * cutouts become data-type="cut_path" groups with their own per-piece counter
 * and "piece-<n>-cut_path-<m>" ids, plain internal paths keep their
 * independent "internal_path" counter, and every group points back to the
 * piece via data-parent.
 */
void TST_SvgComponentTags::ExportedSvgTagsCutPathGroups() const
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Could not create a temporary directory");
    const QString filePath = tempDir.filePath(QStringLiteral("component_tags.svg"));

    // Render the piece through the real export pipeline into a temp file.
    const VLayoutPiece piece = makeTestPiece();
    QGraphicsScene scene;
    QGraphicsItem *item = piece.GetItem(true);
    scene.addItem(item); // scene takes ownership

    QGraphicsRectItem paper(QRectF(0, 0, 400, 400));
    SvgGenerator generator(&paper, filePath, QStringLiteral("Test Pattern"), QString(), 96);
    generator.addSvgFromScene(&scene, item);
    generator.generate();

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), "Generated SVG file could not be opened");
    QDomDocument doc;
    QVERIFY2(doc.setContent(&file), "Generated SVG could not be parsed");

    // Two cutouts: own counter starting at 1, structured ids, piece as parent.
    const QVector<QDomElement> cutouts = groupsOfType(doc, QStringLiteral("cut_path"));
    QCOMPARE(cutouts.size(), 2);
    for (int i = 0; i < cutouts.size(); ++i)
    {
        const QString number = QString::number(i + 1);
        QCOMPARE(cutouts.at(i).attribute(QStringLiteral("id")), QStringLiteral("piece-1-cut_path-%1").arg(number));
        QCOMPARE(cutouts.at(i).attribute(QStringLiteral("data-type-number")), number);
        QCOMPARE(cutouts.at(i).attribute(QStringLiteral("data-parent")), QStringLiteral("piece-1"));
    }

    // The plain internal path keeps its own counter, unaffected by the cutouts.
    const QVector<QDomElement> internals = groupsOfType(doc, QStringLiteral("internal_path"));
    QCOMPARE(internals.size(), 1);
    QCOMPARE(internals.at(0).attribute(QStringLiteral("id")), QStringLiteral("piece-1-internal_path-1"));
    QCOMPARE(internals.at(0).attribute(QStringLiteral("data-type-number")), QStringLiteral("1"));
    QCOMPARE(internals.at(0).attribute(QStringLiteral("data-parent")), QStringLiteral("piece-1"));
}

//---------------------------------------------------------------------------------------------------------------------
///  @file   global.cpp
///  @author Douglas S Caskey
///  @date   22 Jun, 2024
///
///  @copyright
///  Copyright (C) 2017 - 2026 Seamly, LLC
///  https://github.com/fashionfreedom/seamly2d
///
///  @brief
///  Seamly2D is free software: you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation, either version 3 of the License, or
///  (at your option) any later version.
///
///  Seamly2D is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///  You should have received a copy of the GNU General Public License
///  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
///  @file   global.cpp
///  @author Roman Telezhynskyi <dismine(at)gmail.com>
///  @date   Jun 14, 2015
///
///  @brief
///  @copyright
///  This source code is part of the Valentina project, a pattern making
///  program, whose allow create and modeling patterns of clothing.
///  Copyright (C) 2015 Valentina project
///  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
///
///  Valentina is free software: you can redistribute it and/or modify
///  it under the terms of the GNU General Public License as published by
///  the Free Software Foundation, either version 3 of the License, or
///  (at your option) any later version.
///
///  Valentina is distributed in the hope that it will be useful,
///  but WITHOUT ANY WARRANTY; without even the implied warranty of
///  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
///  GNU General Public License for more details.
///
///  You should have received a copy of the GNU General Public License
///  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "global.h"
#include "../vmisc/def.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QtDebug>

const qreal defPointRadiusPixel = (2.0/*mm*/ / 25.4) * PrintDPI;
const qreal widthMainLine = (1.2/*mm*/ / 25.4) * PrintDPI;
const qreal widthHairLine = widthMainLine/3.0;

// Set from the drawing area background color, see setDarkBackground.
// Cached because correctColor runs on every paint.
static bool dark_background = false;

qreal sceneScale(QGraphicsScene *scene)
{
    qreal scale = 1;

    if (scene)
    {
        const QList<QGraphicsView *> views = scene->views();
        if (!views.isEmpty())
        {
            scale = views.first()->transform().m11();
        }
    }

    return scale;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Adjusts a base color according to the item's enabled state and environment.
///
/// Returns gray if the item is disabled; otherwise, returns the color adapted
/// for the current background/theme configuration.
//---------------------------------------------------------------------------------------------------------------------
 QColor adjustColorForState(const QGraphicsItem *item, const QColor &color)
{
    SCASSERT(item != nullptr)

    if (!item->isEnabled())
    {
        return Qt::gray;
    }

    return adaptColor(color);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief adaptColor color to show on the drawing area.
///
/// On a dark background (and with "Adapt colors to a dark background" on) black is shown as light grey so it stays
/// visible. Every other color is painted exactly as it was chosen, so the drawing area always matches the color
/// picker. This is the same idea as AutoCAD's black/white color, which draws white on a dark model space and prints
/// black. Display only: the pattern keeps the real color for saving, printing and export.
///
/// @param color the color stored in the pattern or settings.
/// @return the color to paint with.
//---------------------------------------------------------------------------------------------------------------------
QColor adaptColor(const QColor &color)
{
    if (!dark_background || color != QColor(Qt::black))
    {
        return color;
    }

    // Black is shown approx 88% of white.
    QColor adapted_color = Qt::black;
    adapted_color.setHsl(0, 0, 225, color.alpha());
    return adapted_color;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setDarkBackground turns the lightness flip in adaptColor on or off.
/// @param dark true when colors should adapt to a dark drawing area background.
//---------------------------------------------------------------------------------------------------------------------
void setDarkBackground(bool dark)
{
    dark_background = dark;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief isDarkBackground whether adaptColor currently flips lightness.
/// @return true when colors adapt to a dark drawing area background.
//---------------------------------------------------------------------------------------------------------------------
bool isDarkBackground()
{
    return dark_background;
}

//---------------------------------------------------------------------------------------------------------------------
// @brief pointRect finds the bounding rect of a point with a given radius.
// @param radius radius of point.
//---------------------------------------------------------------------------------------------------------------------
QRectF pointRect(qreal radius)
{
    QRectF rect = QRectF(0, 0, radius*2, radius*2);
    rect.translate(-rect.center().x(), -rect.center().y());
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Calculates a zoom-independent point radius.
/// Scales down the default pixel radius as the view zooms in, clamping it
/// to a minimum floor so the point remains visible.
///
/// @param scale The current zoom scale factor of the view.
/// @return The calculated, scale-adjusted radius as a qreal.
//---------------------------------------------------------------------------------------------------------------------
qreal scaledRadius(qreal scale)
{
    qDebug() << "scaledRadius() " << scale;
    qreal scaledRadius = defPointRadiusPixel;
    if (scale > 1)
    {
        scaledRadius = qMax(defPointRadiusPixel/96, defPointRadiusPixel/scale);
    }
    return scaledRadius;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Updates the bounding rectangle of an ellipse item based on the zoom scale.
///
/// @param item Pointer to the QGraphicsEllipseItem to be resized (must not be nullptr).
/// @param scale The current zoom scale factor of the view.
//---------------------------------------------------------------------------------------------------------------------
void scaleCircleSize(QGraphicsEllipseItem *item, qreal scale)
{
    SCASSERT(item != nullptr)

    item->setRect(pointRect(scaledRadius(scale)));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Updates the bounding rectangle of a rect item based on the zoom scale.
///
/// @param item Pointer to the QGraphicsRectItem to be resized (must not be nullptr).
/// @param scale The current zoom scale factor of the view.
//---------------------------------------------------------------------------------------------------------------------
void scaleRectSize(QGraphicsRectItem *item, qreal scale)
{
    SCASSERT(item != nullptr)

    item->setRect(pointRect(scaledRadius(scale)));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Scales a line width down as the view zooms in.
/// Prevents lines from becoming overly thick under high zoom while
/// maintaining a minimum hard floor of 0.01 pixels.
///
/// @param width The initial base line width before scaling.
/// @param scale The current zoom scale factor of the view.
/// @return The scale-adjusted line width, clamped to a minimum of 0.01.
//---------------------------------------------------------------------------------------------------------------------
qreal scaleWidth(qreal width, qreal scale)
{
    if (scale > 1)
    {
        width = qMax(0.01, width/scale);
    }
    return width;
}

//---------------------------------------------------------------------------------------------------------------------
///  @brief itemShapeFromPath a new painter path representing the outline of the given path is created based on the
///                           width of the given pen.
///  @param path QPainter path to outline.
///  @param pen QPen
//---------------------------------------------------------------------------------------------------------------------
QPainterPath itemShapeFromPath(const QPainterPath &path, const QPen &pen)
{
    // We unfortunately need this hack as QPainterPathStroker will set a width of 1.0
    // if we pass a value of 0.0 to QPainterPathStroker::setWidth()
    const qreal zeroPenWidth = qreal(0.00000001);

    if (path == QPainterPath() || pen == Qt::NoPen)
    {
        return path;
    }
    QPainterPathStroker stroke;
    stroke.setCapStyle(pen.capStyle());
    if (pen.widthF() <= 0.0)
    {
        stroke.setWidth(zeroPenWidth);
    }
    else
    {
        stroke.setWidth(pen.widthF());
    }
    stroke.setJoinStyle(pen.joinStyle());
    stroke.setMiterLimit(pen.miterLimit());
    QPainterPath newPath = stroke.createStroke(path);
    newPath.addPath(path);
    return newPath;
}

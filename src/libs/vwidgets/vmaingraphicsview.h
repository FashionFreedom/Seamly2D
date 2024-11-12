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
 **  @file   vmaingraphicsview.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VMAINGRAPHICSVIEW_H
#define VMAINGRAPHICSVIEW_H

#include <qcompilerdetection.h>
#include <QGraphicsView>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <Qt>
#include <QtGlobal>
#include <QRubberBand>
#include <QColor>
#include <Qt>
#include <QSharedPointer>

/*!
 * This class adds ability to zoom QGraphicsView using mouse wheel. The point under cursor
 * remains motionless while it's possible.
 *
 * When the user starts scrolling, this class remembers original scene position and
 * keeps it until scrolling is completed. It's better than getting original scene position at
 * each scrolling step because that approach leads to position errors due to before-mentioned
 * positioning restrictions.
 *
 * When zoomed using scroll, this class emits zoomed() signal.
 *
 * Usage:
 *
 *   new Graphics_view_zoom(view);
 *
 * The object will be deleted automatically when the view is deleted.
 *
 * You can set keyboard modifiers used for zooming using set_modified(). Zooming will be
 * performed only on exact match of modifiers combination. The default modifier is Ctrl.
 *
 * You can change zoom velocity by calling set_zoom_factor_base().
 * Zoom coefficient is calculated as zoom_factor_base^angle_delta
 * (see QWheelEvent::angleDelta).
 * The default zoom factor base is 1.0015.
 */

class QTimeLine;
class QTransform;
class QGesture;
class QGestureEvent;
class QPanGesture;
class QPinchGesture;

class GraphicsViewZoom : public QObject
{
    Q_OBJECT
public:
    explicit              GraphicsViewZoom(QGraphicsView* view);
    void                  gentleZoom(qreal factor);
    void                  setModifiers(Qt::KeyboardModifiers modifiers);
    void                  setZoomSpeedFactor(qreal value);
    void                  initScrollAnimations();

signals:
    void                  zoomed();

public slots:
    void                  verticalScrollingTime(qreal x);
    void                  horizontalScrollingTime(qreal x);
    void                  animFinished();

protected:
    virtual bool          eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(GraphicsViewZoom)
    QGraphicsView        *m_view;
    Qt::KeyboardModifiers m_modifiers;
    qreal                 m_zoomSpeedFactor;
    QPointF               targetScenePos;
    QPointF               targetViewPos;
    int                   m_duration;
    int                   m_updateInterval;
    QTimeLine            *verticalScrollAnim;
    /** @brief _numScheduledVerticalScrollings keep number scheduled vertical scrollings. */
    qint32                m_numScheduledVerticalScrollings;
    QTimeLine            *horizontalScrollAnim;
    /** @brief _numScheduledHorizontalScrollings keep number scheduled horizontal scrollings. */
    qint32                m_numScheduledHorizontalScrollings;

    void                  fictiveSceneRect(QGraphicsScene *sc, QGraphicsView *view);

    bool                  startVerticalScrollings(QWheelEvent* wheel_event);
    bool                  startHorizontalScrollings(QWheelEvent* wheel_event);
    bool                  gestureEvent(QGestureEvent* event);
    void                  panTriggered(QPanGesture* gesture);
    void                  pinchTriggered(QPinchGesture* gesture);

    QGesture             *pan;
    QGesture             *pinch;
    qreal                 horizontalOffset;
    qreal                 verticalOffset;
    qreal                 scaleFactor;
    qreal                 currentScaleFactor;
};

/**
 * @brief The VMainGraphicsView class main scene view.
 */
class VMainGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:

    explicit              VMainGraphicsView(QWidget *parent = nullptr);
    void                  setShowToolOptions(bool value);
    void                  allowRubberBand(bool value);
    void                  zoomPanEnabled(bool value);
    void                  zoomToAreaEnabled(bool value);

    static void           NewSceneRect(QGraphicsScene *sc, QGraphicsView *view, QGraphicsItem *item = nullptr);
    static QRectF         SceneVisibleArea(QGraphicsView *view);

    static qreal          MinScale();
    static qreal          MaxScale();

    void                  setRubberBandColor(QRubberBand *band, const QColor &color);

    void                  initScrollBars();

signals:
    /**
     * @brief mouseRelease help catch mouse release event.
     *
     * Usefull when you need show dialog after working with tool visualization.
     */
    void                  mouseRelease();
    void                  itemClicked(QGraphicsItem *item);
    void                  signalZoomScaleChanged(qreal scale);


public slots:
    void                  zoomByScale(qreal scale);
    void                  zoomIn();
    void                  zoomOut();
    void                  zoom100Percent();
    void                  zoomToFit();
    void                  zoomToRect(const QRectF &rect);
    void                  updateView(const QTransform &transform);
    void                  resetScrollBars();
    void                  resetScrollAnimations();

protected:
    virtual void          mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void          mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void          mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void          mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    QSharedPointer<QCursor> curMagnifier;

private:
    Q_DISABLE_COPY(VMainGraphicsView)
    GraphicsViewZoom     *zoom;
    bool                  showToolProperties;
    bool                  showScrollBars;
    bool                  isallowRubberBand;
    bool                  isZoomToAreaActive;
    bool                  isRubberBandActive;
    bool                  isRubberBandColorSet;
    bool                  isZoomPanActive;
    bool                  isPanDragActive;
    QRubberBand          *rubberBand;
    QRect                *rubberBandRect;
    QPoint                startPoint;
    QPoint                endPoint;
    QPoint                m_startPos;
    QPoint                cursorPos;
};

#endif // VMAINGRAPHICSVIEW_H

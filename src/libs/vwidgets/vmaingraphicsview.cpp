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
 **  @file   vmaingraphicsview.cpp
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

#include "vmaingraphicsview.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFlags>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QLineF>
#include <QList>
#include <QMessageLogger>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QPainter>
#include <QPoint>
#include <QScrollBar>
#include <QTimeLine>
#include <QTransform>
#include <QWheelEvent>
#include <QGesture>
#include <QWidget>
#include <QGuiApplication>
#include <QScreen>
#include <QAbstractScrollArea>
#include <QScreen>

#include "../vmisc/logging.h"
#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vabstractapplication.h"
#include "../ifc/ifcdef.h"
#include "vabstractmainwindow.h"
#include "vmaingraphicsscene.h"
#include "vsimplecurve.h"

Q_LOGGING_CATEGORY(vGraphicsViewZoom, "vgraphicsviewzoom")

const qreal maxSceneSize = ((20.0 * 1000.0) / 25.4) * PrintDPI; // 20 meters in pixels

//---------------------------------------------------------------------------------------------------------------------
GraphicsViewZoom::GraphicsViewZoom(QGraphicsView* view)
    : QObject(view)
    , m_view(view)
    , m_modifiers(Qt::ControlModifier)
    , m_zoomSpeedFactor(1.0015)
    , targetScenePos(QPointF())
    , targetViewPos(QPointF())
    , m_duration()
    , m_updateInterval()
    , verticalScrollAnim(new QTimeLine(300, this))
    , m_numScheduledVerticalScrollings(0)
    , horizontalScrollAnim(new QTimeLine(300, this))
    , m_numScheduledHorizontalScrollings(0)
    , pan(nullptr)
    , pinch(nullptr)
    , horizontalOffset(0.0)
    , verticalOffset(0.0)
    , scaleFactor(0.0)
    , currentScaleFactor(0.0)
{
    m_view->viewport()->installEventFilter(this);

    //Enable gestures for the view widget
    m_view->viewport()->grabGesture(Qt::PinchGesture);
    m_view->viewport()->grabGesture(Qt::PanGesture);
    m_view->setMouseTracking(true);

    initScrollAnimations();
    connect(verticalScrollAnim, &QTimeLine::valueChanged, this, &GraphicsViewZoom::verticalScrollingTime);
    connect(verticalScrollAnim, &QTimeLine::finished, this, &GraphicsViewZoom::animFinished);
    connect(horizontalScrollAnim, &QTimeLine::valueChanged, this, &GraphicsViewZoom::horizontalScrollingTime);
    connect(horizontalScrollAnim, &QTimeLine::finished, this, &GraphicsViewZoom::animFinished);
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::gentleZoom(qreal factor)
{
    // We need to check current scale factor because in Windows we have an error when we zoom in or zoom out to much.
    // See issue #532: Unexpected error occurs when zoom out image.
    // factor > 1 for zoomIn and factor < 1 for zoomOut.
    const qreal m11 = m_view->transform().m11();

    if ((factor > 1 && m11 <= VMainGraphicsView::MaxScale()) || (factor < 1 && m11 >= VMainGraphicsView::MinScale()))
    {
        m_view->scale(factor, factor);
        if (factor < 1)
        {
            // Because QGraphicsView centers the picture when it's smaller than the view. And QGraphicsView's scrolls
            // boundaries don't allow to put any picture point at any viewport position we will provide fictive scene
            // size. Temporary and bigger than view, scene size will help position an image under cursor.
            fictiveSceneRect(m_view->scene(), m_view);
        }
        m_view->centerOn(targetScenePos);
        QPointF deltaViewPos = targetViewPos - QPointF(m_view->viewport()->width() / 2.0,
                                                                   m_view->viewport()->height() / 2.0);
        QPointF viewCenter = m_view->mapFromScene(targetScenePos) - deltaViewPos;
        m_view->centerOn(m_view->mapToScene(viewCenter.toPoint()));
        // In the end we just set correct scene size
        VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(m_view->scene());
        SCASSERT(currentScene)
        currentScene->setCurrentTransform(m_view->transform());
        VMainGraphicsView::NewSceneRect(m_view->scene(), m_view);
        emit zoomed();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void GraphicsViewZoom::setModifiers(Qt::KeyboardModifiers modifiers)
{
  m_modifiers = modifiers;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void GraphicsViewZoom::setZoomSpeedFactor(qreal value)
{
    m_zoomSpeedFactor = value;
}

void GraphicsViewZoom::initScrollAnimations()
{
    m_duration = qApp->Settings()->getScrollDuration();
    m_updateInterval = qApp->Settings()->getScrollUpdateInterval();
    verticalScrollAnim->setDuration(m_duration);
    verticalScrollAnim->setUpdateInterval(m_updateInterval);

    horizontalScrollAnim->setDuration(m_duration);
    horizontalScrollAnim->setUpdateInterval(m_updateInterval);
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::verticalScrollingTime(qreal x)
{
    Q_UNUSED(x)
    // Try to adapt scrolling to speed of rotating mouse wheel and scale factor
    // Value of _numScheduledScrollings is too short, so we scale the value

    qreal scroll = static_cast<qreal>(qAbs(m_numScheduledVerticalScrollings))*(10. + 10./m_view->transform().m22())
            /(static_cast<qreal>(m_duration)/static_cast<qreal>(m_updateInterval));

    if (qAbs(scroll) < 1)
    {
        scroll = 1;
    }

    if (m_numScheduledVerticalScrollings > 0)
    {
        scroll = scroll * -1;
    }
    m_view->verticalScrollBar()->setValue(qRound(m_view->verticalScrollBar()->value() + scroll));
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::horizontalScrollingTime(qreal x)
{
    Q_UNUSED(x)
    // Try to adapt scrolling to speed of rotating mouse wheel and scale factor
    // Value of _numScheduledScrollings is too short, so we scale the value

    qreal scroll = static_cast<qreal>(qAbs(m_numScheduledHorizontalScrollings))*(10. + 10./m_view->transform().m11())
            /(static_cast<qreal>(m_duration)/static_cast<qreal>(m_updateInterval));

    if (qAbs(scroll) < 1)
    {
        scroll = 1;
    }

    if (m_numScheduledHorizontalScrollings > 0)
    {
        scroll = scroll * -1;
    }
    m_view->horizontalScrollBar()->setValue(qRound(m_view->horizontalScrollBar()->value() + scroll));
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::animFinished()
{
    m_numScheduledVerticalScrollings = 0;
    verticalScrollAnim->stop();

    /*
     * In moust cases cursor position on view doesn't change, but for scene after scrolling position will be different.
     * We are goint to check changes and save new value.
     * If don't do that we will zoom using old value cursor position on scene. It is not what we expect.
     * Almoust the same we do in method GraphicsViewZoom::eventFilter.
     */
    const QPoint pos = m_view->mapFromGlobal(QCursor::pos());
    const QPointF delta = targetScenePos - m_view->mapToScene(pos);
    if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
    {
        targetViewPos = pos;
        targetScenePos = m_view->mapToScene(pos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool GraphicsViewZoom::eventFilter(QObject *object, QEvent *event)
{
    m_modifiers = (qApp->Settings()->getZoomModKey()) ? Qt::ControlModifier : Qt::NoModifier;
    const qreal  m_zoomSpeedFactor = (qreal(qApp->Settings()->getZoomSpeedFactor())/10000) + 1.0004;

    if (event->type() == QEvent::MouseMove)
    {
        /*
         * Here we are saving cursor position on view and scene.
         * This data need for gentleZoom().
         * Almoust the same we do in method GraphicsViewZoom::animFinished.
         */
        QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
        QPointF delta = targetViewPos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            targetViewPos = mouse_event->pos();
            targetScenePos = m_view->mapToScene(mouse_event->pos());
        }
        return false;
    }
    else if (event->type() == QEvent::Wheel)
    {
        QWheelEvent* wheel_event = static_cast<QWheelEvent*>(event);
        SCASSERT(wheel_event != nullptr)
        if (QApplication::keyboardModifiers() == m_modifiers)
        {
            if (wheel_event->angleDelta().y() != 0)
            {
                const double angle = wheel_event->angleDelta().y();
                const double factor = qPow(m_zoomSpeedFactor, angle);
                qCDebug(vGraphicsViewZoom, "zoom Factor =%f  \n", factor);
                gentleZoom(factor);
                return true;
            }
        }
        else
        {
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                return startHorizontalScrollings(wheel_event);
            }
            else
            {
                return startVerticalScrollings(wheel_event);
            }
        }
    }
    else if (event->type() == QEvent::Gesture)
    {
        return gestureEvent(static_cast<QGestureEvent*>(event));
    }

    return QObject::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
bool GraphicsViewZoom::gestureEvent(QGestureEvent *event)
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
    {
        qCDebug(vGraphicsViewZoom, "pinch gestureEvent");
        pinchTriggered(static_cast<QPinchGesture *>(pinch));
        return true;
    }
    if (QGesture *pan = event->gesture(Qt::PanGesture))
    {
        qCDebug(vGraphicsViewZoom, "pan gestureEvent");
        panTriggered(static_cast<QPanGesture *>(pan));
        return true;
    }

    return false;
}

void GraphicsViewZoom::panTriggered(QPanGesture *gesture)
{
#ifndef QT_NO_CURSOR
    switch (gesture->state())
    {
        case Qt::GestureStarted:
        case Qt::GestureUpdated:
             m_view->viewport()->setCursor(Qt::SizeAllCursor);
             break;
        default:
             m_view->viewport()->setCursor(Qt::ArrowCursor);
    }
#endif
    QPointF delta = gesture->delta();
    qCDebug(vGraphicsViewZoom) << "panTriggered():" << gesture;
    horizontalOffset += delta.x();
    verticalOffset += delta.y();
    //m_view->scrollContentsBy(horizontalOffset, verticalOffset);
}

void GraphicsViewZoom::pinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags flags = gesture->changeFlags();
    if (flags & QPinchGesture::ScaleFactorChanged)
    {
        qreal currentScaleFactor = gesture->lastScaleFactor();
        gentleZoom(currentScaleFactor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::fictiveSceneRect(QGraphicsScene *sc, QGraphicsView *view)
{
    SCASSERT(sc != nullptr)
    SCASSERT(view != nullptr)

    //Calculate view rect
    //to receive the currently visible area, map the widgets bounds to the scene
    const QPointF a = view->mapToScene(0, 0 );
    const QPointF b = view->mapToScene(view->viewport()->width(), view->viewport()->height());
    QRectF viewRect = QRectF( a, b );

    //Scale view
    QLineF topLeftRay(viewRect.center(), viewRect.topLeft());
    topLeftRay.setLength(topLeftRay.length()*2);

    QLineF bottomRightRay(viewRect.center(), viewRect.bottomRight());
    bottomRightRay.setLength(bottomRightRay.length()*2);

    viewRect = QRectF(topLeftRay.p2(), bottomRightRay.p2());

    //Calculate scene rect
    const QRectF sceneRect = sc->sceneRect();

    //Unite two rects
    const QRectF newRect = sceneRect.united(viewRect);

    sc->setSceneRect(newRect);
}

//---------------------------------------------------------------------------------------------------------------------
bool GraphicsViewZoom::startVerticalScrollings(QWheelEvent *wheel_event)
{
    SCASSERT(wheel_event != nullptr)

    const QPoint numPixels = wheel_event->pixelDelta();
    const QPoint numDegrees = wheel_event->angleDelta() / 8;
    int numSteps;

    if (not numPixels.isNull())
    {
        numSteps = numPixels.y();
    }
    else if (not numDegrees.isNull())
    {
        numSteps = numDegrees.y() / 15;
    }
    else
    {
        return true;//Just ignore
    }

    m_numScheduledVerticalScrollings += numSteps;
    if (m_numScheduledVerticalScrollings * numSteps < 0)
    {  // if user moved the wheel in another direction, we reset previously scheduled scalings
        m_numScheduledVerticalScrollings = numSteps;
    }

    m_numScheduledVerticalScrollings *= qint32(qApp->Settings()->getScrollSpeedFactor()/10.0);

    if (verticalScrollAnim->state() != QTimeLine::Running)
    {
        verticalScrollAnim->start();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool GraphicsViewZoom::startHorizontalScrollings(QWheelEvent *wheel_event)
{
    SCASSERT(wheel_event != nullptr)

    const QPoint numPixels = wheel_event->pixelDelta();
    const QPoint numDegrees = wheel_event->angleDelta() / 8;
    int numSteps;

    if (not numPixels.isNull())
    {
        numSteps = numPixels.y();
    }
    else if (not numDegrees.isNull())
    {
        numSteps = numDegrees.y() / 15;
    }
    else
    {
        return true;//Just ignore
    }

    m_numScheduledHorizontalScrollings += numSteps;
    if (m_numScheduledHorizontalScrollings * numSteps < 0)
    {  // if user moved the wheel in another direction, we reset previously scheduled scalings
        m_numScheduledHorizontalScrollings = numSteps;
    }

    m_numScheduledHorizontalScrollings *= qint32(qApp->Settings()->getScrollSpeedFactor()/10.0);

    if (horizontalScrollAnim->state() != QTimeLine::Running)
    {
        horizontalScrollAnim->start();
    }
    return true;
}

Q_LOGGING_CATEGORY(vMainGraphicsView, "vmaingraphicsview")

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMainGraphicsView constructor.
 * @param parent parent object.
 */
VMainGraphicsView::VMainGraphicsView(QWidget *parent)
    : QGraphicsView(parent)
    , curMagnifier(new QCursor(QPixmap(":/cursor/magnifier_cursor.png"), 2, 2))
    , zoom(new GraphicsViewZoom(this))
    , showToolProperties(true)
    , showScrollBars()
    , isallowRubberBand(true)
    , isZoomToAreaActive(false)
    , isRubberBandActive(false)
    , isRubberBandColorSet(false)
    , isZoomPanActive(false)
    , isPanDragActive(false)
    , rubberBand(nullptr)
    , rubberBandRect(nullptr)
    , startPoint(QPoint())
    , endPoint(QPoint())
    , m_ptStartPos(QPoint())
    , cursorPos(QPoint())
{
    initScrollBars();

    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    this->setInteractive(true);

    connect(zoom, &GraphicsViewZoom::zoomed, this, [this](){emit signalZoomScaleChanged(transform().m11());});
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomByScale(qreal scale)
{
    qreal factor = qBound(MinScale(), scale, MaxScale());
    QTransform transform = this->transform();
    transform.setMatrix(factor, transform.m12(), transform.m13(),
                        transform.m21(), factor, transform.m23(),
                        transform.m31(), transform.m32(), transform.m33());
    this->setTransform(transform);
    updateView(transform);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomIn()
{
    // We need to check current scale factor because in Windows we get an error when we zoom in or out too much.
    // See issue #532: Unexpected error occurs when zoom out image.
    if (this->transform().m11() <= MaxScale())
    {
        scale(1.1, 1.1);
        updateView(this->transform());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomOut()
{
    // We need to check current scale factor because in Windows we have an error when we zoom in or zoom out to much.
    // See issue #532: Unexpected error occurs when zoom out image.
    if (this->transform().m11() >= MinScale())
    {
        scale(1.0/1.1, 1.0/1.1);
        updateView(this->transform());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoom100Percent()
{
    QTransform transform = this->transform();
    transform.setMatrix(1.0, transform.m12(), transform.m13(),
                        transform.m21(), 1.0, transform.m23(),
                        transform.m31(), transform.m32(), transform.m33());
    this->setTransform(transform);
    updateView(transform);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomToFit()
{
    VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
    SCASSERT(currentScene)
    currentScene->setOriginsVisible(false);
    const QRectF rect = currentScene->visibleItemsBoundingRect();
    currentScene->setOriginsVisible(true);

    zoomToRect(rect);
}

void VMainGraphicsView::zoomToRect(const QRectF &rect)
{
    if (rect.isEmpty())
    {
        return;
    }

    this->fitInView(rect, Qt::KeepAspectRatio);
    QTransform transform = this->transform();

    qreal factor = transform.m11();
    factor = qMax(factor, MinScale());
    factor = qMin(factor, MaxScale());

    transform.setMatrix(factor, transform.m12(), transform.m13(),
                        transform.m21(), factor, transform.m23(),
                        transform.m31(), transform.m32(), transform.m33());
    this->setTransform(transform);
    updateView(transform);
}

void VMainGraphicsView::updateView(const QTransform &transform)
{
    VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
    SCASSERT(currentScene)
    currentScene->setCurrentTransform(transform);
    VMainGraphicsView::NewSceneRect(this->scene(), this);
    emit signalZoomScaleChanged(transform.m11());
}
//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomToAreaEnabled(bool value)
{
    isZoomToAreaActive = value;
    if (isZoomToAreaActive)
    {
        qCDebug(vMainGraphicsView, "View->Zoom to Selected Area Mode active\n");
        isZoomPanActive = false;
        viewport()->setCursor(*curMagnifier);
    }
    else
    {
        qCDebug(vMainGraphicsView, "View->Zoom to Selected Area Mode not active\n");
        viewport()->setCursor(Qt::ArrowCursor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::zoomPanEnabled(bool value)
{
    isZoomPanActive = value;
    if (isZoomPanActive)
    {
        qCDebug(vMainGraphicsView, "View->Pan Zoom Mode active\n");
        isZoomToAreaActive = false;
        viewport()->setCursor(Qt::OpenHandCursor);
    }
    else
    {
        qCDebug(vMainGraphicsView, "View->Pan Zoom Mode not active\n");
        viewport()->setCursor(Qt::ArrowCursor);
    }
}

void VMainGraphicsView::setRubberBandColor(QRubberBand *band, const QColor &color)
{
    QPalette palette;
    palette.setColor(QPalette::Active, QPalette::Highlight, color);
    band->setPalette(palette);
    isRubberBandColorSet = true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePressEvent handle mouse press events.
 * @param event mouse press event.
 */
void VMainGraphicsView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button())
    {
        case Qt::LeftButton:
        {
            if ( isZoomToAreaActive )
            {
                if (!rubberBand)
                {
                    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);
                }
                isRubberBandActive = true;
                startPoint = event->pos();
                rubberBand->setGeometry(QRect(startPoint,QSize()));
                rubberBand->show();
                qCDebug(vMainGraphicsView, "Select Area...\nstart point X=%d  Y=%d\n", startPoint.x(), startPoint.y());
                break;
            }
            else if ( isZoomPanActive )
            {
                isPanDragActive = true;
                qCDebug(vMainGraphicsView, "Start Pan\n");

                const QList<QGraphicsItem *> list = items(event->pos());
                if (list.size() == 0)
                {// Only when the user clicks on the scene background
                    m_ptStartPos = event->pos();
                    QGraphicsView::setDragMode(QGraphicsView::ScrollHandDrag);
                    event->accept();
                    viewport()->setCursor(Qt::ClosedHandCursor);
                }
                break;
            }
            else
            {
                if ( isallowRubberBand )
                {
                    QGraphicsView::setDragMode(QGraphicsView::RubberBandDrag);
                }
                if ( showToolProperties )
                {
                    QList<QGraphicsItem *> list = items(event->pos());
                    if (list.size() == 0)
                    {
                        emit itemClicked(nullptr);
                        break;
                    }
                    for (int i = 0; i < list.size(); ++i)
                    {
                        if (this->scene()->items().contains(list.at(i)))
                        {
                            if (list.at(i)->type() > QGraphicsItem::UserType && list.at(i)->type() <= VSimpleCurve::Type)
                            {
                                emit itemClicked(list.at(i));
                                break;
                            }
                            else
                            {
                                emit itemClicked(nullptr);
                            }
                        }
                    }
                }
            }
            break;
        }
        case Qt::MiddleButton:
        {
            const QList<QGraphicsItem *> list = items(event->pos());
            if (list.size() == 0)
            {// Only when the user clicks on the scene background
                m_ptStartPos = event->pos();
                QGraphicsView::setDragMode(QGraphicsView::ScrollHandDrag);
                event->accept();
                viewport()->setCursor(Qt::ClosedHandCursor);
            }
            break;
        }
        default:
            break;
    }
    QGraphicsView::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if ( (dragMode() == QGraphicsView::ScrollHandDrag) ||  isPanDragActive  )
    {
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        const QPoint delta = event->pos() - m_ptStartPos;
        hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
        vBar->setValue(vBar->value() - delta.y());
        m_ptStartPos = event->pos();
    }
    else if ( isRubberBandActive )
    {
          endPoint = event->pos();
          if (!isRubberBandColorSet)
          {
              if ( (endPoint.x() < startPoint.x()) | (endPoint.y() < startPoint.y()) )
              {
                setRubberBandColor(rubberBand, qApp->Settings()->getZoomRBNegativeColor());
              }
              else
              {
                setRubberBandColor(rubberBand, qApp->Settings()->getZoomRBPositiveColor());
              }
              isRubberBandColorSet = true;
          }
          rubberBand->setGeometry(QRect(startPoint,endPoint).normalized());
          qCDebug(vMainGraphicsView, "End point  X=%d  Y=%d", endPoint.x(), endPoint.y());
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent handle mouse release events.
 * @param event mouse release event.
 */
void VMainGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent ( event ); // First because need to hide a rubber band
    QGraphicsView::setDragMode( QGraphicsView::NoDrag );
    if ( isPanDragActive )
    {
        isPanDragActive = false;
        viewport()->setCursor(Qt::OpenHandCursor);
        qCDebug(vMainGraphicsView, "Stop Pan\n");
    }
    else if ( isRubberBandActive )
    {
        rubberBand->hide();
        endPoint = event->pos();
        qCDebug( vMainGraphicsView, "End point  X=%d  Y=%d", endPoint.x(), endPoint.y() );

        QRect rubberBandRect = rubberBand->geometry().normalized();
        if (rubberBandRect.width() > 5 && rubberBandRect.height() > 5)
        {
            this->fitInView(QRectF(mapToScene(rubberBandRect.topLeft()),
                            mapToScene(rubberBandRect.bottomRight())), Qt::KeepAspectRatio);
            updateView(this->transform());
        }
        isRubberBandActive = false;
        isRubberBandColorSet = false;
    }
    else if (event->button() == Qt::LeftButton)
    {
        emit mouseRelease();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && qApp->Settings()->isZoomDoubleClick())
    {
        if (VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow()))
        {
            window->zoomToSelected();
        }
    }
    QGraphicsView::mouseDoubleClickEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMainGraphicsView::MinScale()
{
    const QRect screenRect(QGuiApplication::primaryScreen()->availableGeometry());
    const qreal screenSize = qMin(screenRect.width(), screenRect.height());

    return screenSize / maxSceneSize;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMainGraphicsView::MaxScale()
{
    const QRect screenRect(QGuiApplication::primaryScreen()->availableGeometry());
    const qreal screenSize = qMin(screenRect.width(), screenRect.height());

    return maxSceneSize / screenSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::setShowToolOptions(bool value)
{
    showToolProperties = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::allowRubberBand(bool value)
{
    isallowRubberBand = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NewSceneRect calculate scene rect what contains all items and doesn't less that size of scene view.
 * @param sc scene.
 * @param view view.
 */
 void VMainGraphicsView::NewSceneRect(QGraphicsScene *sc, QGraphicsView *view, QGraphicsItem *item)
 {
    SCASSERT(sc != nullptr)
    SCASSERT(view != nullptr)

    if (item == nullptr)
    {
        //Calculate view rect
        const QRectF viewRect = SceneVisibleArea(view);

        //Calculate scene rect
        VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(sc);
        SCASSERT(currentScene)
        const QRectF itemsRect = currentScene->visibleItemsBoundingRect();

        //Unite two rects
        sc->setSceneRect(itemsRect.united(viewRect));
    }
    else if (not sc->sceneRect().contains(item->sceneBoundingRect()))
    {
        sc->setSceneRect(sc->sceneRect().united(item->sceneBoundingRect()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VMainGraphicsView::SceneVisibleArea(QGraphicsView *view)
{
    SCASSERT(view != nullptr)
    //to receive the currently visible area, map the widgets bounds to the scene
    return QRectF(view->mapToScene(0, 0), view->mapToScene(view->width(), view->height()));
}

void VMainGraphicsView::initScrollBars()
{
    showScrollBars = qApp->Settings()->getShowScrollBars();
    if (showScrollBars == true)
    {
        QString horizontal_styleSheet = QString("QScrollBar {height: ");
        horizontal_styleSheet += QString().number(qApp->Settings()->getScrollBarWidth());
        horizontal_styleSheet += QString("px;}");

        QString vertical_styleSheet = QString("QScrollBar {width: ");
        vertical_styleSheet += QString().number(qApp->Settings()->getScrollBarWidth());
        vertical_styleSheet += QString("px;}");

        this->horizontalScrollBar()->setStyleSheet(horizontal_styleSheet);
        this->verticalScrollBar()->setStyleSheet(vertical_styleSheet);
    }
    else
    {
        this->horizontalScrollBar()->setStyleSheet("QScrollBar {height: 0px;}");
        this->verticalScrollBar()->setStyleSheet("QScrollBar {width: 0px;}");
    }

    this->horizontalScrollBar()->setSingleStep(1);
    this->horizontalScrollBar()->update();

    this->verticalScrollBar()->setSingleStep(1);
    this->verticalScrollBar()->update();

}

void VMainGraphicsView::resetScrollBars()
{
    initScrollBars();
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::resetScrollAnimations()
{
    zoom->initScrollAnimations();
}

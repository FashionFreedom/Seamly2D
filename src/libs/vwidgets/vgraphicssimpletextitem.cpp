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
 **  @file   vgraphicssimpletextitem.cpp
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

#include "vgraphicssimpletextitem.h"

#include <QEvent>
#include <QFlags>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QList>
#include <QMessageLogger>
#include <QPalette>
#include <QPoint>
#include <QPolygonF>
#include <QRectF>
#include <Qt>
#include <QtDebug>

#include "vmaingraphicsscene.h"
#include "vmaingraphicsview.h"
#include "vscenepoint.h"
#include "global.h"
#include "../vmisc/vabstractapplication.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VGraphicsSimpleTextItem default constructor.
 * @param parent parent object.
 */
VGraphicsSimpleTextItem::VGraphicsSimpleTextItem(QColor textColor, QGraphicsItem *parent)
    : QGraphicsSimpleTextItem(parent)
    , m_fontSize(qApp->Settings()->getPointNameSize())
    , m_scale(1)
    , m_textColor(textColor)
    , m_isNameHovered(false)
    , selectionType(SelectionType::ByMouseRelease)
    , m_showParentTooltip(true)
{
    initItem();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VGraphicsSimpleTextItem constructor.
 * @param text text.
 * @param parent parent object.
 */
VGraphicsSimpleTextItem::VGraphicsSimpleTextItem(const QString &text, QColor textColor,  QGraphicsItem *parent)
    : QGraphicsSimpleTextItem(text, parent)
    , m_fontSize(qApp->Settings()->getPointNameSize())
    , m_scale(1)
    , m_textColor(textColor)
    , selectionType(SelectionType::ByMouseRelease)
    , m_showParentTooltip(true)
{
    initItem();
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsScene *scene = this->scene();
    const qreal scale = sceneScale(scene);

    if (scale > 1.0 && not VFuzzyComparePossibleNulls(m_scale, scale))
    {
        scalePointName(scale);
    }
    else if (scale <= 1.0 && not VFuzzyComparePossibleNulls(m_scale, 1.0))
    {
        scalePointName(1.0);
    }

    QFont fnt = this->font();
    if (fnt.pointSize() != qApp->Settings()->getPointNameSize() ||
        fnt.family() != qApp->Settings()->getPointNameFont().family())
    {
        fnt.setPointSize(qApp->Settings()->getPointNameSize());
        fnt.setFamily(qApp->Settings()->getPointNameFont().family());
        this->setFont(fnt);
    }

    if (QGraphicsView *view = scene->views().at(0))
    {
        VMainGraphicsView::NewSceneRect(scene, view, this);
    }

    if (m_isNameHovered)
    {
        this->setBrush(QBrush(QColor(qApp->Settings()->getPointNameHoverColor())));
    }
    else
    {
        this->setBrush(QBrush(getTextBrushColor()));
    }

    QGraphicsSimpleTextItem::paint(painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::setEnabled(bool enabled)
{
    QGraphicsSimpleTextItem::setEnabled(enabled);
    this->setBrush(QBrush(getTextBrushColor()));
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::textSelectionType(const SelectionType &type)
{
    selectionType = type;
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::setShowParentTooltip(bool show)
{
    m_showParentTooltip = show;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange handle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
QVariant VGraphicsSimpleTextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
     if (change == ItemPositionChange  && scene())
     {
         // Each time we move something we call recalculation scene rect. In some cases this can cause moving
         // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
         static bool changeFinished = true;

         if (changeFinished)
         {
            changeFinished = false;
            if (scene())
            {
                const QList<QGraphicsView *> viewList = scene()->views();
                if (not viewList.isEmpty())
                {
                    if (QGraphicsView *view = viewList.at(0))
                    {
                        const int xmargin = 50;
                        const int ymargin = 50;

                        const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(view);
                        const QRectF itemRect = mapToScene(boundingRect()).boundingRect();

                        // If item's rect is bigger than view's rect ensureVisible works very unstable.
                        if (itemRect.height() + 2*ymargin < viewRect.height() &&
                            itemRect.width() + 2*xmargin < viewRect.width())
                        {
                             view->ensureVisible(itemRect, xmargin, ymargin);
                        }
                        else
                        {
                            // Ensure visible only small rect around a cursor
                            VMainGraphicsScene *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                            SCASSERT(currentScene)
                            const QPointF cursorPosition = currentScene->getScenePos();
                            view->ensureVisible(QRectF(cursorPosition.x()-5, cursorPosition.y()-5, 10, 10));
                        }
                    }
                }
            }

            m_pointNamePos = value.toPointF();
            const qreal scale = sceneScale(scene());

            if (scale > 1)
            {
                QLineF line(QPointF(), m_pointNamePos);
                line.setLength(line.length() * scale);
                m_pointNamePos = line.p2();
            }
            emit nameChangedPosition(m_pointNamePos + this->parentItem()->pos());

            changeFinished = true;
         }
     }

     if (change == QGraphicsItem::ItemSelectedChange)
     {
         setFlag(QGraphicsItem::ItemIsFocusable, value.toBool());
         emit pointSelected(value.toBool());
     }
     return QGraphicsSimpleTextItem::itemChange(change, value);
     //return QGraphicsItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void VGraphicsSimpleTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isNameHovered = true;
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(qApp->getSceneView()->viewport()->cursor());
    }

    QGraphicsItem *parent = parentItem();
    if(parent && m_showParentTooltip)
    {
        setToolTip(parent->toolTip());
    }

    QGraphicsSimpleTextItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void VGraphicsSimpleTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isNameHovered = false;
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        setCursor(QCursor());
    }

    QGraphicsSimpleTextItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VGraphicsSimpleTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit showContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsSimpleTextItem::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not (flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        }
    }
    if (selectionType == SelectionType::ByMouseRelease)
    {
        event->accept(); // This help for not selectable items still receive mouseReleaseEvent events
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            emit pointChosen();
            event->accept();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
    }

    if (selectionType == SelectionType::ByMouseRelease)
    {
        emit pointChosen();
    }

    QGraphicsSimpleTextItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            emit deleteTool();
            return; //Leave this method immediately after call!!!
        default:
            break;
    }
    QGraphicsSimpleTextItem::keyReleaseEvent (event);
}

QColor VGraphicsSimpleTextItem::getTextBrushColor()
{
    if (qApp->Settings()->getUseToolColor())
    {
        QColor textColor = correctColor(this, m_textColor);
        textColor.setAlpha(224);
        return textColor;
    }
    else
    {
        QColor textColor = correctColor(this, QColor(qApp->Settings()->getPointNameColor()));
        textColor.setAlpha(224);
        return textColor;
    }
}

void VGraphicsSimpleTextItem::setTextColor(const QColor &color)
{
    m_textColor = color;
    this->setBrush(QBrush(getTextBrushColor()));
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::setPosition(QPointF pos)
{
    m_pointNamePos = pos;

    scalePosition();
}

void VGraphicsSimpleTextItem::initItem()
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true);// For keyboard input focus
    this->setAcceptHoverEvents(true);

    QFont fnt = font();
    fnt.setPointSize(m_fontSize);
    fnt.setFamily(qApp->Settings()->getPointNameFont().family());
    setFont(fnt);

    setScale(m_scale);

    setBrush(QBrush(getTextBrushColor()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief scalePointName handle point name scaling to maintain same size when scene scale changes.
 * @param scale parent scene scale.
 */
 void VGraphicsSimpleTextItem::scalePointName(const qreal &scale)
{
    setScale(1/scale);
    scalePosition();
    updateLeader();
    m_scale = scale;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief scalePosition handle point name position scaling to maintain same distance when scene scale changes.
 */
 void VGraphicsSimpleTextItem::scalePosition()
{
    const qreal scale = sceneScale(scene());
    QPointF newPos = m_pointNamePos;

    if (scale > 1)
    {
        QLineF line(QPointF(), m_pointNamePos);
        line.setLength(line.length() / scale);
        newPos = line.p2();
    }

    blockSignals(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setPos(newPos);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::updateLeader()
{
    if (VScenePoint *parent = dynamic_cast<VScenePoint *>(parentItem()))
    {
        parent->refreshLeader();
    }
}

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
 **  @file   vgraphicssimpletextitem.h
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

#ifndef VGRAPHICSSIMPLETEXTITEM_H
#define VGRAPHICSSIMPLETEXTITEM_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>
#include <QColor>

#include "../vmisc/def.h"

/**
 * @brief The VGraphicsSimpleTextItem class pointer label.
 */
class VGraphicsSimpleTextItem : public QObject, public QGraphicsSimpleTextItem
{
    Q_OBJECT
public:
    explicit         VGraphicsSimpleTextItem(QColor color, QGraphicsItem *parent = nullptr);
    explicit         VGraphicsSimpleTextItem( const QString &text, QColor textColor, QGraphicsItem *parent = nullptr );
    virtual         ~VGraphicsSimpleTextItem() =default;

    qint32           BaseFontSize()const;
    virtual int      type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::GraphicsSimpleTextItem)};

    virtual void     paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    void             setEnabled(bool enabled);
    void             textSelectionType(const SelectionType &type);
    void             setShowParentTooltip(bool show);

    QColor           getTextBrushColor();
    void             setTextColor(const QColor &color);

    void             setPosition(QPointF pos);

signals:
    /**
     * @brief nameChangedPosition emit when label change position.
     * @param pos new posotion.
     */
    void             nameChangedPosition(const QPointF &pos);
    /**
     * @brief showContextMenu emit when need show tool context menu.
     * @param event context menu event.
     */
    void             showContextMenu(QGraphicsSceneContextMenuEvent *event);
    void             deleteTool();
    void             pointChosen();
    void             pointSelected(bool selected);

protected:
    virtual QVariant itemChange (GraphicsItemChange change, const QVariant &value ) Q_DECL_OVERRIDE;
    virtual void     hoverEnterEvent (QGraphicsSceneHoverEvent *event ) Q_DECL_OVERRIDE;
    virtual void     hoverLeaveEvent (QGraphicsSceneHoverEvent *event ) Q_DECL_OVERRIDE;
    virtual void     contextMenuEvent (QGraphicsSceneContextMenuEvent *event ) Q_DECL_OVERRIDE;
    virtual void     mousePressEvent(QGraphicsSceneMouseEvent * event ) Q_DECL_OVERRIDE;
    virtual void     mouseReleaseEvent (QGraphicsSceneMouseEvent * event ) Q_DECL_OVERRIDE;
    virtual void     keyReleaseEvent (QKeyEvent * event ) Q_DECL_OVERRIDE;

private:
    /** @brief fontSize label font size. */
    qint32           m_fontSize;
    qreal            m_scale;
    QColor           m_textColor;
    bool             m_isNameHovered;
    SelectionType    selectionType;
    bool             m_showParentTooltip;
    QPointF          m_pointNamePos{};

    void             initItem();
    void             scalePointName(const qreal &scale);
    void             scalePosition();
    void             updateLeader();
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FontSize return label font size.
 * @return font size.
 */
inline qint32 VGraphicsSimpleTextItem::BaseFontSize() const
{
    return m_fontSize;
}

#endif // VGRAPHICSSIMPLETEXTITEM_H

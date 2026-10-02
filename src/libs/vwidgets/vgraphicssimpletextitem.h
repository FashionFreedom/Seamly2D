//---------------------------------------------------------------------------------------------------------------------
//  @file   vgraphicssimpletextitem.h
//  @author Douglas S Caskey
//  @date   20 May, 2026
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vgraphicssimpletextitem.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

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

//---------------------------------------------------------------------------------------------------------------------
/// @brief The VGraphicsSimpleTextItem class pointer label.
//---------------------------------------------------------------------------------------------------------------------
class VGraphicsSimpleTextItem : public QObject, public QGraphicsSimpleTextItem
{
    Q_OBJECT
public:
    explicit         VGraphicsSimpleTextItem(QColor color, QGraphicsItem *parent = nullptr);
    explicit         VGraphicsSimpleTextItem( const QString &text, QColor textColor, QGraphicsItem *parent = nullptr );
    virtual         ~VGraphicsSimpleTextItem() =default;

    qint32           BaseFontSize()const;
    virtual int      type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::GraphicsSimpleTextItem)};

    virtual void     paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget = nullptr) override;

    void             setEnabled(bool enabled);
    void             textSelectionType(const SelectionType &type);
    void             setShowParentTooltip(bool show);

    QColor           getTextBrushColor();
    void             setTextColor(const QColor &color);

    void             setPosition(QPointF pos);

signals:
    /// @brief nameChangedPosition emit when label change position.
    /// @param pos new posotion.
    void             nameChangedPosition(const QPointF &pos);

    /// @brief showContextMenu emit when need show tool context menu.
    /// @param event context menu event.
    void             showContextMenu(QGraphicsSceneContextMenuEvent *event);

    void             deleteTool();
    void             pointChosen();
    void             pointSelected(bool selected);

protected:
    virtual QVariant itemChange (GraphicsItemChange change, const QVariant &value ) override;
    virtual void     hoverEnterEvent (QGraphicsSceneHoverEvent *event ) override;
    virtual void     hoverLeaveEvent (QGraphicsSceneHoverEvent *event ) override;
    virtual void     contextMenuEvent (QGraphicsSceneContextMenuEvent *event ) override;
    virtual void     mousePressEvent(QGraphicsSceneMouseEvent * event ) override;
    virtual void     mouseReleaseEvent (QGraphicsSceneMouseEvent * event ) override;
    virtual void     keyReleaseEvent (QKeyEvent * event ) override;

private:
    qint32           m_fontSize; /// @brief fontSize label font size.
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
/// @brief FontSize return label font size.
/// @return font size.
//---------------------------------------------------------------------------------------------------------------------
inline qint32 VGraphicsSimpleTextItem::BaseFontSize() const
{
    return m_fontSize;
}

#endif // VGRAPHICSSIMPLETEXTITEM_H

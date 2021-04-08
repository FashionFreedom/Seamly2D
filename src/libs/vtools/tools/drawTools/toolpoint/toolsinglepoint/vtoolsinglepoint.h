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
 **  @file   vtoolsinglepoint.h
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

#ifndef VTOOLSINGLEPOINT_H
#define VTOOLSINGLEPOINT_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vabstractpoint.h"
#include "../vmisc/def.h"
#include "../vwidgets/vscenepoint.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolSinglePoint class parent for all tools what create points.
 */
class VToolSinglePoint: public VAbstractPoint, public VScenePoint
{
    Q_OBJECT
public:
                     VToolSinglePoint(VAbstractPattern *doc, VContainer *data, quint32 id,
                                      const QColor &lineColor, QGraphicsItem * parent = nullptr);
    virtual         ~VToolSinglePoint() Q_DECL_EQ_DEFAULT;

    virtual int      type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::SinglePoint)};

    QString          name() const;
    void             setName(const QString &name);

    void             SetEnabled(bool enabled);

    virtual void     GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;
    virtual bool     isPointNameVisible(quint32 id) const Q_DECL_OVERRIDE;
    virtual void     setPointNameVisiblity(quint32 id, bool visible) Q_DECL_OVERRIDE;
    virtual void     setPointNamePosition(quint32 id, const QPointF &pos) Q_DECL_OVERRIDE;

public slots:
    void             pointnameChangedPosition(const QPointF &pos);
    virtual void     Disable(bool disable, const QString &draftBlockName) Q_DECL_OVERRIDE;
    virtual void     EnableToolMove(bool move) Q_DECL_OVERRIDE;
    void             pointChosen();
    void             pointSelected(bool selected);
    virtual void     FullUpdateFromFile() Q_DECL_OVERRIDE;
    virtual void     AllowHover(bool enabled) Q_DECL_OVERRIDE;
    virtual void     AllowSelecting(bool enabled) Q_DECL_OVERRIDE;
    void             allowTextHover(bool enabled);
    void             allowTextSelectable(bool enabled);
    virtual void     ToolSelectionType(const SelectionType &type) Q_DECL_OVERRIDE;

protected:
    virtual void     updatePointNameVisibility(quint32 id, bool visible) Q_DECL_OVERRIDE;
    virtual void     updatePointNamePosition(quint32 id, const QPointF &pos) Q_DECL_OVERRIDE;
    virtual void     mousePressEvent(QGraphicsSceneMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void     mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) Q_DECL_OVERRIDE;
    virtual void     hoverEnterEvent(QGraphicsSceneHoverEvent *event) Q_DECL_OVERRIDE;
    virtual QVariant itemChange ( GraphicsItemChange change, const QVariant &value ) Q_DECL_OVERRIDE;
    virtual void     keyReleaseEvent(QKeyEvent * event) Q_DECL_OVERRIDE;
    virtual void     contextMenuEvent (QGraphicsSceneContextMenuEvent * event ) Q_DECL_OVERRIDE;
    virtual void     SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolSinglePoint)
};

#endif // VTOOLSINGLEPOINT_H

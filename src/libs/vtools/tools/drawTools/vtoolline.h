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
 **  @file   vtoolline.h
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

#ifndef VTOOLLINE_H
#define VTOOLLINE_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vdrawtool.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolLine class tool for creation line.
 */
class VToolLine: public VDrawTool, public QGraphicsLineItem
{
    Q_OBJECT
public:
    virtual void      setDialog() Q_DECL_OVERRIDE;

    static VToolLine *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene, VAbstractPattern *doc,
                             VContainer *data);
    static VToolLine *Create(const quint32 &_id, const quint32 &firstPoint, const quint32 &secondPoint,
                             const QString &typeLine, const QString &lineWeight,
                             const QString &lineColor, VMainGraphicsScene  *scene,
                             VAbstractPattern *doc, VContainer *data, const Document &parse,
                             const Source &typeCreation);

    virtual int       type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::Line)};
    virtual QString   getTagName() const Q_DECL_OVERRIDE;

    virtual void      paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            QWidget *widget = nullptr) Q_DECL_OVERRIDE;

    QString           FirstPointName() const;
    QString           SecondPointName() const;

    quint32           GetFirstPoint() const;
    void              SetFirstPoint(const quint32 &value);

    quint32           GetSecondPoint() const;
    void              SetSecondPoint(const quint32 &value);

    QString           getLineColor() const;
    void              setLineColor(const QString &value);

    virtual void      ShowVisualization(bool show) Q_DECL_OVERRIDE;

    virtual void      setLineType(const QString &value) Q_DECL_OVERRIDE;
    void              setLineWeight(const QString &value) Q_DECL_OVERRIDE;
    
    virtual void      GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;

public slots:
    virtual void      FullUpdateFromFile() Q_DECL_OVERRIDE;
    virtual void      ShowTool(quint32 id, bool enable) Q_DECL_OVERRIDE;
    virtual void      Disable(bool disable, const QString &draftBlockName) Q_DECL_OVERRIDE;
    virtual void      AllowHover(bool enabled) Q_DECL_OVERRIDE;
    virtual void      AllowSelecting(bool enabled) Q_DECL_OVERRIDE;

protected slots:
    virtual void      showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void      contextMenuEvent(QGraphicsSceneContextMenuEvent * event ) override;
    virtual void      AddToFile() Q_DECL_OVERRIDE;
    virtual void      hoverEnterEvent(QGraphicsSceneHoverEvent * event ) Q_DECL_OVERRIDE;
    virtual void      hoverLeaveEvent(QGraphicsSceneHoverEvent * event ) Q_DECL_OVERRIDE;
    virtual void      RemoveReferens() Q_DECL_OVERRIDE;
    virtual QVariant  itemChange(GraphicsItemChange change, const QVariant &value ) Q_DECL_OVERRIDE;
    virtual void      keyReleaseEvent(QKeyEvent * event) Q_DECL_OVERRIDE;
    virtual void      SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void      SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void      ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void      SetVisualization() Q_DECL_OVERRIDE;
    virtual QString   makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolLine)

    /** @brief firstPoint id first line point. */
    quint32           firstPoint;

    /** @brief secondPoint id second line point. */
    quint32           secondPoint;

    /** @brief lineColor color of a line. */
    QString           lineColor;

    bool              m_isHovered;

                      VToolLine(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 firstPoint,
                                quint32 secondPoint, const QString &typeLine, const QString &lineWeight,
                                const QString &lineColor,
                                const Source &typeCreation, QGraphicsItem * parent = nullptr);

    void              RefreshGeometry();
};

#endif // VTOOLLINE_H

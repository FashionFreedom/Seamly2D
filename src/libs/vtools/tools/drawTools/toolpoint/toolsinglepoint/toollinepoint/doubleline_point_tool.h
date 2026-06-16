/**************************************************************************
 **
 **  @file   doubleline_point_tool.h
 **  @author Douglas S Caskey
 **  @date   7.19.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#ifndef DOUBLELINE_POINT_TOOL_H
#define DOUBLELINE_POINT_TOOL_H

#include "../vmisc/def.h"
#include "../vpatterndb/vformula.h"
#include "../vtoolsinglepoint.h"

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

template <class T> class QSharedPointer;

/**
 * @brief The DoubleLinePointTool class parent for all tools what creates a point with 2 lines.
 */
class DoubleLinePointTool : public VToolSinglePoint
{
    Q_OBJECT
public:
                      DoubleLinePointTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                          const QString &typeLine, const QString &lineWeight,
                                          const QString &lineColor,
                                          const quint32 &firstPointId, const quint32 &secondPointId,
                                          QGraphicsItem * parent = nullptr);
    virtual          ~DoubleLinePointTool() override;

    virtual int       type() const override {return Type;}
    enum             {Type = UserType + static_cast<int>(Tool::LinePoint)};

    virtual void      paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            QWidget *widget = nullptr) override;

    QString           point1Name() const;
    QString           point2Name() const;

    quint32           getFirstPointId() const;
    void              setFirstPointId(const quint32 &value);

    quint32           getSecondPointId() const;
    void              setSecondPointId(const quint32 &value);

    QString           getLineColor() const;
    void              setLineColor(const QString &value);

public slots:
    virtual void      Disable(bool disable, const QString &draftBlockName) override;
    virtual void      FullUpdateFromFile() override;

protected:
    quint32           firstPointId;     /** @brief firstPointId id first line point. */
    quint32           secondPointId;    /** @brief secondPointId id second line point. */
    VScaledLine      *firstLine;        /** @brief firstLine first line item. */
    VScaledLine      *secondLine;       /** @brief secondLine second line item. */
    QString           lineColor;        /** @brief lineColor color of a line. */

    virtual void      RefreshGeometry();
    virtual void      RemoveReferens() override;
    virtual void      SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void      hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void      hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual QString   makeToolTip() const override;

private:
    Q_DISABLE_COPY(DoubleLinePointTool)
};

#endif // DOUBLELINE_POINT_TOOL_H

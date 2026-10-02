//---------------------------------------------------------------------------------------------------------------------
//  @file   basepoint_tool.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2023 Seamly, LLC
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
//  @file   vtoolbasepoint.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef BASEPOINT_TOOL_H
#define BASEPOINT_TOOL_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <Qt>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

class BasePointTool : public VToolSinglePoint
{
    Q_OBJECT
public:
    virtual               ~BasePointTool() = default;
    virtual void           setDialog() override;

    static BasePointTool *Create(quint32 _id, const QString &active_draft_block, VPointF *point,
                                  VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                  const Document &parse, const Source &type_creation);

    static const QString   ToolType;
    virtual int            type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::BasePoint)};

    virtual void           ShowVisualization(bool show) override;

    QPointF                GetBasePointPos() const;
    void                   SetBasePointPos(const QPointF &pos);

public slots:
    virtual void           FullUpdateFromFile() override;
    virtual void           EnableToolMove(bool move) override;

signals:
    void                   LiteUpdateTree(); ///@brief FullUpdateTree handle if need update pattern file.

protected slots:
    virtual void           showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;

protected:
    virtual void           AddToFile() override;
    virtual QVariant       itemChange (GraphicsItemChange change, const QVariant &value) override;
    virtual void           deleteTool(bool ask = true) override;
    virtual void           SaveDialog(QDomElement &dom_element) override;
    virtual void           hoverEnterEvent (QGraphicsSceneHoverEvent *event) override;
    virtual void           hoverLeaveEvent (QGraphicsSceneHoverEvent *event) override;
    virtual void           mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void           mouseReleaseEvent (QGraphicsSceneMouseEvent *event) override;
    virtual void           SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void           ReadToolAttributes(const QDomElement &dom_element) override;
    virtual void           SetVisualization() override {}
    virtual QString        makeToolTip() const override;

private:
    Q_DISABLE_COPY(BasePointTool)

    QString                m_draft_block_name;

                           BasePointTool (VAbstractPattern *doc, VContainer *data, quint32 id,
                                           const Source &type_creation, const QString &draft_block_name,
                                           QGraphicsItem * parent = nullptr);
};

#endif // BASEPOINT_TOOL_H

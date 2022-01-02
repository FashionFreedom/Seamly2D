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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#ifndef VTOOLMIRRORBYAXIS_H
#define VTOOLMIRRORBYAXIS_H

#include <QtGlobal>

#include "vabstractmirror.h"

class VToolMirrorByAxis : public VAbstractMirror
{
    Q_OBJECT

public:
    virtual                  ~VToolMirrorByAxis() Q_DECL_EQ_DEFAULT;
    virtual void              setDialog() Q_DECL_OVERRIDE;
    static VToolMirrorByAxis *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data);

    static VToolMirrorByAxis *Create(const quint32 _id, quint32 originPointId, AxisType axisType,
                                     const QString &suffix, const QVector<SourceItem> &source,
                                     const QVector<DestinationItem> &destination, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data, const Document &parse,
                                     const Source &typeCreation);

    static const QString ToolType;

    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                 {Type = UserType + static_cast<int>(Tool::MirrorByAxis)};

    AxisType             getAxisType() const;
    void                 setAxisType(AxisType value);

    QString              getOriginPointName() const;

    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void         SetVisualization() Q_DECL_OVERRIDE;
    virtual void         SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual QString      makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolMirrorByAxis)

    quint32              m_originPointId;
    AxisType             m_axisType;

                         VToolMirrorByAxis(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 originPointId,
                                           AxisType axisType, const QString &suffix, const QVector<SourceItem> &source,
                                           const QVector<DestinationItem> &destination, const Source &typeCreation,
                                           QGraphicsItem *parent = nullptr);
};

#endif // VTOOLMIRRORBYAXIS_H

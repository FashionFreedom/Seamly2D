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
 **  @file   vtoolcutspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 12, 2013
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

#ifndef VTOOLCUTSPLINE_H
#define VTOOLCUTSPLINE_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolcut.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolCutSpline class for tool CutSpline. This tool find point on spline and cut spline on two.
 */
class VToolCutSpline : public VToolCut
{
    Q_OBJECT
public:
    virtual void setDialog() Q_DECL_OVERRIDE;

    static VToolCutSpline *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene, VAbstractPattern *doc,
                                  VContainer *data);

    static VToolCutSpline *Create(const quint32 _id, const QString &pointName, QString &direction,
                                  QString &formula, const quint32 &splineId, qreal mx, qreal my, bool showPointName,
                                  VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                  const Document &parse, const Source &typeCreation);

    static const QString ToolType;
    static const QString AttrSpline;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CutSpline)};
    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void          SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void          SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void          ReadToolAttributes(const QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void          SetVisualization() Q_DECL_OVERRIDE;
    virtual QString       makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolCutSpline)

                          VToolCutSpline(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                         QString &direction, const QString &formula, const quint32 &splineId,
                                         const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // VTOOLCUTSPLINE_H

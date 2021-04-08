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
 **  @file   vtoolarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#ifndef VTOOLARCWITHLENGTH_H
#define VTOOLARCWITHLENGTH_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vabstractspline.h"

class VFormula;
template <class T> class QSharedPointer;

class VToolArcWithLength : public VAbstractSpline
{
    Q_OBJECT
public:
    virtual void         setDialog() Q_DECL_OVERRIDE;

    static VToolArcWithLength *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene  *scene,
                                      VAbstractPattern *doc, VContainer *data);
    static VToolArcWithLength *Create(const quint32 _id, const quint32 &center, QString &radius, QString &f1,
                                      QString &length, const QString &color, const QString &penStyle,
                                      VMainGraphicsScene *scene, VAbstractPattern *doc, VContainer *data,
                                      const Document &parse, const Source &typeCreation);

    static const QString ToolType;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::ArcWithLength)};
    virtual QString      getTagName() const Q_DECL_OVERRIDE;

    QString              CenterPointName() const;

    quint32              getCenter() const;
    void                 setCenter(const quint32 &value);

    VFormula             GetFormulaRadius() const;
    void                 SetFormulaRadius(const VFormula &value);

    VFormula             GetFormulaF1() const;
    void                 SetFormulaF1(const VFormula &value);

    VFormula             GetFormulaLength() const;
    void                 SetFormulaLength(const VFormula &value);

    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;


protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void         RemoveReferens() Q_DECL_OVERRIDE;
    virtual void         SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void         SetVisualization() Q_DECL_OVERRIDE;
    virtual QString      makeToolTip() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolArcWithLength)

                         VToolArcWithLength(VAbstractPattern *doc, VContainer *data, quint32 id,
                                            const Source &typeCreation, QGraphicsItem * parent = nullptr);
};

#endif // VTOOLARCWITHLENGTH_H

/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   movespline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2014
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

#ifndef MOVESPLINE_H
#define MOVESPLINE_H

#include <qcompilerdetection.h>
#include <QHash>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vspline.h"
#include "vundocommand.h"

class QGraphicsScene;
class VContainer;

class MoveSpline : public VUndoCommand
{
    Q_OBJECT
public:
    MoveSpline(VAbstractPattern *doc, const VSpline *oldSpl, const VSpline &newSpl, VContainer *data,
               const quint32 &id, QUndoCommand *parent = nullptr);
    virtual ~MoveSpline() override;
    virtual void undo() override;
    virtual void redo() override;
    virtual bool mergeWith(const QUndoCommand *command) override;
    virtual int  id() const override;
    quint32      getSplineId() const;
    VSpline      getNewSpline() const;
private:
    Q_DISABLE_COPY(MoveSpline)
    VSpline  oldSpline;
    VSpline  newSpline;
    QGraphicsScene *scene;
    // Frozen at construction time, when oldSpline/newSpline's name-form formulas were captured -
    // NOT recomputed in undo()/redo(), which could otherwise run long after an intervening rename
    // desyncs the live container's names from that already-captured text.
    const QHash<QString, QString> nameToIdToken;
    void         Do(const VSpline &spl);
};

//---------------------------------------------------------------------------------------------------------------------
inline quint32 MoveSpline::getSplineId() const
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline VSpline MoveSpline::getNewSpline() const
{
    return newSpline;
}

#endif // MOVESPLINE_H

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
 **  @file   moveoperationlabel.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2016
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

#ifndef MOVE_OPERATION_LABEL_H
#define MOVE_OPERATION_LABEL_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "moveabstractlabel.h"

class MoveOperationLabel : public MoveAbstractLabel
{
public:
                    MoveOperationLabel(quint32 idTool, VAbstractPattern *doc, const QPointF &pos,
                                       quint32 idPoint, QUndoCommand *parent = nullptr);
    virtual        ~MoveOperationLabel()=default;

    virtual bool    mergeWith(const QUndoCommand *command) Q_DECL_OVERRIDE;
    virtual int     id() const Q_DECL_OVERRIDE;

    quint32         GetToolId() const;

protected:
    virtual void    Do(const QPointF &pos) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(MoveOperationLabel)
    quint32         m_idTool;
    //Need for resizing scene rect
    QGraphicsScene *m_scene;

};

//---------------------------------------------------------------------------------------------------------------------
inline quint32 MoveOperationLabel::GetToolId() const
{
    return m_idTool;
}

#endif // MOVE_OPERATION_LABEL_H

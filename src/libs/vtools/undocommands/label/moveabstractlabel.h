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
 **  @file   moveabstractlabel.h
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

#ifndef MOVEABSTRACTLABEL_H
#define MOVEABSTRACTLABEL_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vundocommand.h"

class QGraphicsScene;

class MoveAbstractLabel : public VUndoCommand
{
public:
                    MoveAbstractLabel(VAbstractPattern *doc, quint32 pointId, const QPointF &pos,
                                      QUndoCommand *parent = nullptr);
    virtual        ~MoveAbstractLabel()=default;


    virtual void    undo() Q_DECL_OVERRIDE;
    virtual void    redo() Q_DECL_OVERRIDE;

    quint32         GetPointId() const;
    QPointF         GetNewPos() const;

protected:
    QPointF         m_oldPos;
    QPointF         m_newPos;

    virtual void    Do(const QPointF &pos)=0;
    
private:
    Q_DISABLE_COPY(MoveAbstractLabel)
};

//---------------------------------------------------------------------------------------------------------------------
inline quint32 MoveAbstractLabel::GetPointId() const
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline QPointF MoveAbstractLabel::GetNewPos() const
{
    return m_newPos;
}

#endif // MOVEABSTRACTLABEL_H

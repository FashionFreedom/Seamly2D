/***************************************************************************
 **  @file   set_piece_color.h
 **  @author Douglas S Caskey
 **  @date   Jan 1, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#ifndef SET_PIECE_COLOR_H
#define SET_PIECE_COLOR_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class SetPieceColor : public VUndoCommand
{
    Q_OBJECT
public:
                 SetPieceColor(quint32 id, QString color, VContainer *data,
                               VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    virtual     ~SetPieceColor();
    virtual void undo() Q_DECL_OVERRIDE;
    virtual void redo() Q_DECL_OVERRIDE;
    virtual int  id() const Q_DECL_OVERRIDE;
    quint32      getpieceId() const;
    QString      getColor() const;

signals:
    void         updateList(quint32 m_id);

private:
                 Q_DISABLE_COPY(SetPieceColor)
    quint32      m_id;
    VContainer  *m_data;
    QString      m_oldColor;
    QString      m_newColor;

    void         doCmd(QString color);
};

#endif // SET_PIECE_COLOR_H

/***************************************************************************
 *   @file   showdoublepointname.h                                         *
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
 **************************************************************************/

#ifndef ShowDoublePointName_H
#define ShowDoublePointName_H

#include "../vundocommand.h"

class QGraphicsScene;

enum class ShowDoublePoint: qint8 { FirstPoint, SecondPoint };

class ShowDoublePointName : public VUndoCommand
{
    Q_OBJECT
public:
                     ShowDoublePointName(VAbstractPattern *doc, quint32 toolId, quint32 pointId, bool visible,
                                         ShowDoublePoint type, QUndoCommand *parent = nullptr);
    virtual         ~ShowDoublePointName()=default;

    virtual void     undo() override;
    virtual void     redo() override;

private:
    Q_DISABLE_COPY(ShowDoublePointName)
    bool             m_visible;
    bool             m_oldVisible;
    //Need for resizing scene rect
    QGraphicsScene  *m_scene;
    ShowDoublePoint  m_type;
    quint32          m_idTool;

    void             setVisibility(bool visible);
};

#endif // ShowDoublePointName_H

/***************************************************************************
 *   @file   showoperationpointname.h                                      *
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


 #ifndef SHOW_OPERATION_POINTNAME_H
 #define SHOW_OPERATION_POINTNAME_H

 #include "../vundocommand.h"

 class QGraphicsScene;

 class ShowOperationPointName : public VUndoCommand
 {
 public:
                     ShowOperationPointName(VAbstractPattern *doc, quint32 idTool, quint32 idPoint,
                                            bool visible, QUndoCommand *parent = nullptr);
     virtual        ~ShowOperationPointName()=default;

     virtual void    undo() Q_DECL_OVERRIDE;
     virtual void    redo() Q_DECL_OVERRIDE;

 private:
     Q_DISABLE_COPY(ShowOperationPointName)
     bool            m_visible;
     bool            m_oldVisible;
     QGraphicsScene *m_scene; //Need for resizing scene rect
     quint32         m_idTool;

     void            Do(bool visible);
 };

 #endif // SHOW_OPERATION_POINTNAME_H

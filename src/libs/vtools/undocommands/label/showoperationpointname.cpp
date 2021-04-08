/***************************************************************************
 *   @file   showoperationpointname.cpp                                    *
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

 #include "showoperationpointname.h"

 #include <QDomElement>

 #include "../ifc/xml/vabstractpattern.h"
 #include "../vmisc/logging.h"
 #include "../vwidgets/vmaingraphicsview.h"
 #include "../vmisc/vabstractapplication.h"
 #include "../vtools/tools/drawTools/vdrawtool.h"

 //---------------------------------------------------------------------------------------------------------------------
 ShowOperationPointName::ShowOperationPointName(VAbstractPattern *doc, quint32 idTool, quint32 idPoint, bool visible,
                                                QUndoCommand *parent)
     : VUndoCommand(QDomElement(), doc, parent)
     , m_visible(visible)
     , m_oldVisible(not visible)
     , m_scene(qApp->getCurrentScene())
     , m_idTool(idTool)
 {
     nodeId = idPoint;
     setText(tr("toggle label"));

     const QDomElement element = getDestinationObject(m_idTool, nodeId);
     if (element.isElement())
     {
         m_oldVisible = doc->getParameterBool(element, AttrShowPointName, trueStr);
     }
     else
     {
         qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
     }
 }

 //---------------------------------------------------------------------------------------------------------------------
 void ShowOperationPointName::undo()
 {
     qCDebug(vUndo, "Undo.");

     Do(m_oldVisible);
 }

 //---------------------------------------------------------------------------------------------------------------------
 void ShowOperationPointName::redo()
 {
     qCDebug(vUndo, "Redo.");

     Do(m_visible);
 }

 //---------------------------------------------------------------------------------------------------------------------
 void ShowOperationPointName::Do(bool visible)
 {
     QDomElement domElement = getDestinationObject(m_idTool, nodeId);
     if (not domElement.isNull() && domElement.isElement())
     {
         doc->SetAttribute<bool>(domElement, AttrShowPointName, visible);

         if (VDrawTool *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
         {
             tool->setPointNameVisiblity(nodeId, visible);
         }
         VMainGraphicsView::NewSceneRect(m_scene, qApp->getSceneView());
     }
     else
     {
         qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
     }
 }

/***************************************************************************
 *   @file   showdoublepointname.cpp                                       *
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

#include "showdoublepointname.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/drawTools/vdrawtool.h"

//---------------------------------------------------------------------------------------------------------------------
ShowDoublePointName::ShowDoublePointName(VAbstractPattern *doc, quint32 toolId, quint32 pointId, bool visible,
                                         ShowDoublePoint type, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_visible(visible)
    , m_oldVisible(not visible)
    , m_scene(qApp->getCurrentScene())
    , m_type(type)
    , m_idTool(toolId)
{
    nodeId = pointId;
    qCDebug(vUndo, "Point id %u", nodeId);

    if (type == ShowDoublePoint::FirstPoint)
    {
        setText(tr("toggle the first dart visibility"));
    }
    else
    {
        setText(tr("toggle the second dart visibility"));
    }

    const QDomElement domElement = doc->elementById(m_idTool, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        if (type == ShowDoublePoint::FirstPoint)
        {
            m_oldVisible = doc->getParameterBool(domElement, AttrShowPointName1, trueStr);
        }
        else
        {
            m_oldVisible = doc->getParameterBool(domElement, AttrShowPointName2, trueStr);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", m_idTool);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ShowDoublePointName::undo()
{
    qCDebug(vUndo, "Undo.");

    setVisibility(m_oldVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowDoublePointName::redo()
{
    qCDebug(vUndo, "Redo.");

    setVisibility(m_visible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowDoublePointName::setVisibility(bool visible)
{
    QDomElement domElement = doc->elementById(m_idTool, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        if (m_type == ShowDoublePoint::FirstPoint)
        {
            doc->SetAttribute<bool>(domElement, AttrShowPointName1, visible);
        }
        else
        {
            doc->SetAttribute<bool>(domElement, AttrShowPointName2, visible);
        }

        if (VDrawTool *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
        {
            tool->setPointNameVisiblity(nodeId, visible);
        }
        VMainGraphicsView::NewSceneRect(m_scene, qApp->getSceneView());
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", m_idTool);
    }
}

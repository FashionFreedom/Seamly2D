/***************************************************************************
 *   @file   showpointname.cpp                                             *
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

#include "showpointname.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/logging.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/vabstracttool.h"

//---------------------------------------------------------------------------------------------------------------------
ShowPointName::ShowPointName(VAbstractPattern *doc, quint32 id, bool visible, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_visible(visible)
    , m_oldVisible(true)
    , m_scene(qApp->getCurrentScene())
{
    setText(tr("Toggle point name visibilty"));

    nodeId = id;

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        m_oldVisible = doc->getParameterBool(domElement, AttrShowPointName, trueStr);
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ShowPointName::undo()
{
    qCDebug(vUndo, "Undo.");

    setVisibility(m_oldVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowPointName::redo()
{
    qCDebug(vUndo, "Redo.");

    setVisibility(m_visible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowPointName::setVisibility(bool visible)
{
    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        doc->SetAttribute<bool>(domElement, AttrShowPointName, visible);

        if (VAbstractTool *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->setPointNameVisiblity(nodeId, visible);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

/***************************************************************************
 **  @file   add_groupitem.cpp
 **  @author Douglas S Caskey
 **  @date   Mar 2, 2023
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

#include "add_groupitem.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vtools/tools/vdatatool.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
AddGroupItem::AddGroupItem(const QDomElement &xml, VAbstractPattern *doc, quint32 groupId, QUndoCommand *parent)
    : VUndoCommand(xml, doc, parent)
    , m_activeDraftblockName(doc->getActiveDraftBlockName())
{
    setText(tr("Add item to group"));
    nodeId = groupId;
}

//---------------------------------------------------------------------------------------------------------------------
AddGroupItem::~AddGroupItem()
{
}

//---------------------------------------------------------------------------------------------------------------------
void AddGroupItem::undo()
{
    qCDebug(vUndo, "Undo add group item");

    doc->changeActiveDraftBlock(m_activeDraftblockName);//Without this user will not see this change

    QDomElement group = doc->elementById(nodeId, VAbstractPattern::TagGroup);
    if (group.isElement())
    {
        if (group.removeChild(xml).isNull())
        {
            qCDebug(vUndo, "Can't delete item.");
            return;
        }

        doc->SetModified(true);
        emit qApp->getCurrentDocument()->patternChanged(false);

        // set the item visible. Because if the undo is done when invisibile and it's not in any group after the
        // undo, it stays invisible until the entire drawing is completly rerendered.
        quint32 objectId = doc->GetParametrUInt(xml,QString("object"),NULL_ID_STR);
        quint32 toolId = doc->GetParametrUInt(xml,QString("tool"),NULL_ID_STR);
        VDataTool* tool = doc->getTool(toolId);
        tool->GroupVisibility(objectId,true);

        QDomElement groups = doc->createGroups();
        if (not groups.isNull())
        {
            doc->parseGroups(groups);
        } else
        {
            qCDebug(vUndo, "Can't get tag Groups.");
            return;
        }

        emit updateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get group by id = %u.", nodeId);
        return;
    }

    VMainGraphicsView::NewSceneRect(qApp->getCurrentScene(), qApp->getSceneView());
    emit doc->setCurrentDraftBlock(m_activeDraftblockName);//Return current draft Block after undo
}

//---------------------------------------------------------------------------------------------------------------------
void AddGroupItem::redo()
{
    qCDebug(vUndo, "Redo add group item");
    doc->changeActiveDraftBlock(m_activeDraftblockName);//Without this user will not see this change

    QDomElement group = doc->elementById(nodeId, VAbstractPattern::TagGroup);
    if (group.isElement())
    {
        if (group.appendChild(xml).isNull())
        {
            qCDebug(vUndo, "Can't add item.");
            return;
        }

        doc->SetModified(true);
        emit qApp->getCurrentDocument()->patternChanged(false);

        QDomElement groups = doc->createGroups();
        if (not groups.isNull())
        {
            doc->parseGroups(groups);
        } else
        {
            qCDebug(vUndo, "Can't get tag Groups.");
            return;
        }

        emit updateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get group by id = %u.", nodeId);
        return;
    }

    VMainGraphicsView::NewSceneRect(qApp->getCurrentScene(), qApp->getSceneView());
    emit doc->setCurrentDraftBlock(m_activeDraftblockName);//Return current draft Block after undo
}

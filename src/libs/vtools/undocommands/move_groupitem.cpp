/***************************************************************************
 **  @file   move_groupitem.cpp
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

#include "move_groupitem.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vtools/tools/vdatatool.h"

//---------------------------------------------------------------------------------------------------------------------

MoveGroupItem::MoveGroupItem(const QDomElement &source, const QDomElement &dest, VAbstractPattern *doc,
                             quint32 sourceId, quint32 destinationId, QUndoCommand *parent)
    : VUndoCommand(source, doc, parent)
    , m_source(source)
    , m_dest(dest)
    , m_activeDraftblockName(doc->getActiveDraftBlockName())
    , m_sourceGroupId(sourceId)
    , m_destinationGroupId(destinationId)
{
    setText(tr("Move group item"));
}

//---------------------------------------------------------------------------------------------------------------------
MoveGroupItem::~MoveGroupItem()
{
}

//---------------------------------------------------------------------------------------------------------------------
void MoveGroupItem::undo()
{
    qCDebug(vUndo, "Undo move group item");
    doc->changeActiveDraftBlock(m_activeDraftblockName);//Without this user will not see this change

    QDomElement sourceGroup = doc->elementById(m_sourceGroupId, VAbstractPattern::TagGroup);
    QDomElement destintationGroup = doc->elementById(m_destinationGroupId, VAbstractPattern::TagGroup);
    if (sourceGroup.isElement() && destintationGroup.isElement())
    {
        if (sourceGroup.appendChild(m_source).isNull())
        {
            qCDebug(vUndo, "Can't move the item.");
            return;
        }

        if (destintationGroup.removeChild(m_dest).isNull())
        {
            qCDebug(vUndo, "Can't move item.");
            //sourceGroup.removeChild(m_source);
            return;
        }

        doc->SetModified(false);
        emit qApp->getCurrentDocument()->patternChanged(true);

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
        qCDebug(vUndo, "Can't get group by id = %u.", m_sourceGroupId);
        return;
    }

    VMainGraphicsView::NewSceneRect(qApp->getCurrentScene(), qApp->getSceneView());
    emit doc->setCurrentDraftBlock(m_activeDraftblockName);//Return current draft Block after undo

}

//---------------------------------------------------------------------------------------------------------------------
void MoveGroupItem::redo()
{
    qCDebug(vUndo, "Redo move group item");
    doc->changeActiveDraftBlock(m_activeDraftblockName);//Without this user will not see this change

    QDomElement sourceGroup = doc->elementById(m_sourceGroupId, VAbstractPattern::TagGroup);
    QDomElement destintationGroup = doc->elementById(m_destinationGroupId, VAbstractPattern::TagGroup);
    if (sourceGroup.isElement() && destintationGroup.isElement())
    {
        if (destintationGroup.appendChild(m_dest).isNull())
        {
            qCDebug(vUndo, "Can't move the item.");
            return;
        }

        if (sourceGroup.removeChild(m_source).isNull())
        {
            qCDebug(vUndo, "Can't move item.");
            //destintationGroup.removeChild(m_dest);
            return;
        }

        doc->SetModified(false);
        emit qApp->getCurrentDocument()->patternChanged(true);

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
        qCDebug(vUndo, "Can't get group by id = %u.", m_sourceGroupId);
        return;
    }

    VMainGraphicsView::NewSceneRect(qApp->getCurrentScene(), qApp->getSceneView());
    emit doc->setCurrentDraftBlock(m_activeDraftblockName); //Return current draft Block after undo
}

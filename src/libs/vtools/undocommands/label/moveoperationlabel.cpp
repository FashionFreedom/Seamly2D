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
 **  @file   moveoperationlabel.cpp
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

#include "moveoperationlabel.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vtools/tools/drawTools/vdrawtool.h"
#include "../vundocommand.h"
#include "moveabstractlabel.h"

//---------------------------------------------------------------------------------------------------------------------
MoveOperationLabel::MoveOperationLabel(quint32 idTool, VAbstractPattern *doc, const QPointF &pos, quint32 idPoint,
                                       QUndoCommand *parent)
    : MoveAbstractLabel(doc, idPoint, pos, parent)
    , m_idTool(idTool)
    , m_scene(qApp->getCurrentScene())
{
    setText(tr("move point label"));

    qCDebug(vUndo, "Tool id %u", m_idTool);

    const QDomElement element = getDestinationObject(m_idTool, nodeId);
    if (element.isElement())
    {
        m_oldPos.rx() = qApp->toPixel(doc->GetParametrDouble(element, AttrMx, "0.0"));
        m_oldPos.ry() = qApp->toPixel(doc->GetParametrDouble(element, AttrMy, "0.0"));

        qCDebug(vUndo, "Label old Mx %f", m_oldPos.x());
        qCDebug(vUndo, "Label old My %f", m_oldPos.y());
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MoveOperationLabel::mergeWith(const QUndoCommand *command)
{
    const MoveOperationLabel *moveCommand = static_cast<const MoveOperationLabel *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->GetToolId() != m_idTool && moveCommand->GetPointId() != nodeId)
    {
        return false;
    }

    qCDebug(vUndo, "Merge with undo.");
    m_newPos = moveCommand->GetNewPos();
    qCDebug(vUndo, "Label new Mx %f", m_newPos.x());
    qCDebug(vUndo, "Label new My %f", m_newPos.y());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int MoveOperationLabel::id() const
{
    return static_cast<int>(UndoCommand::RotationMoveLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveOperationLabel::Do(const QPointF &pos)
{
    qCDebug(vUndo, "New mx %f", pos.x());
    qCDebug(vUndo, "New my %f", pos.y());

    QDomElement domElement = getDestinationObject(m_idTool, nodeId);
    if (not domElement.isNull() && domElement.isElement())
    {
        doc->SetAttribute(domElement, AttrMx, QString().setNum(qApp->fromPixel(pos.x())));
        doc->SetAttribute(domElement, AttrMy, QString().setNum(qApp->fromPixel(pos.y())));

        if (VDrawTool *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
        {
            tool->setPointNamePosition(nodeId, pos);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

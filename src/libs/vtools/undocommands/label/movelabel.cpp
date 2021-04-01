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
 **  @file   movelabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#include "movelabel.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vundocommand.h"
#include "moveabstractlabel.h"

//---------------------------------------------------------------------------------------------------------------------
MoveLabel::MoveLabel(VAbstractPattern *doc, const QPointF &pos, const quint32 &id, QUndoCommand *parent)
    : MoveAbstractLabel(doc, id, pos, parent)
    , m_scene(qApp->getCurrentScene())
{
    setText(tr("move point label"));

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        m_oldPos.rx() = qApp->toPixel(doc->GetParametrDouble(domElement, AttrMx, "0.0"));
        m_oldPos.ry() = qApp->toPixel(doc->GetParametrDouble(domElement, AttrMy, "0.0"));
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MoveLabel::mergeWith(const QUndoCommand *command)
{
    const MoveLabel *moveCommand = static_cast<const MoveLabel *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->GetPointId() != nodeId)
    {
        return false;
    }

    m_newPos = moveCommand->GetNewPos();

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int MoveLabel::id() const
{
    return static_cast<int>(UndoCommand::MoveLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveLabel::Do(const QPointF &pos)
{
    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        doc->SetAttribute(domElement, AttrMx, QString().setNum(qApp->fromPixel(pos.x())));
        doc->SetAttribute(domElement, AttrMy, QString().setNum(qApp->fromPixel(pos.y())));

        if (VAbstractTool *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->setPointNamePosition(nodeId, pos);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

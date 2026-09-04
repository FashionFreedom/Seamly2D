/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   movespline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2014
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

#include "movespline.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/formulaidtranslator.h"
#include "../vpatterndb/patternformulatokens.h"
#include "vundocommand.h"

using namespace FormulaIdTranslator;
using namespace PatternFormulaTokens;

//---------------------------------------------------------------------------------------------------------------------
MoveSpline::MoveSpline(VAbstractPattern *doc, const VSpline *oldSpl, const VSpline &newSpl, VContainer *data,
                       const quint32 &id, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      oldSpline(*oldSpl),
      newSpline(newSpl),
      scene(qApp->getCurrentScene()),
      nameToIdToken(nameToIdTokenMap(data))
{
    setText(tr("move spline"));
    nodeId = id;

    SCASSERT(scene != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
MoveSpline::~MoveSpline()
{}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(oldSpline);
    VMainGraphicsView::NewSceneRect(scene, qApp->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(newSpline);
    VMainGraphicsView::NewSceneRect(scene, qApp->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
bool MoveSpline::mergeWith(const QUndoCommand *command)
{
    const MoveSpline *moveCommand = static_cast<const MoveSpline *>(command);
    SCASSERT(moveCommand != nullptr)
    const quint32 id = moveCommand->getSplineId();

    if (id != nodeId)
    {
        return false;
    }

    newSpline = moveCommand->getNewSpline();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int MoveSpline::id() const
{
    return static_cast<int>(UndoCommand::MoveSpline);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::Do(const VSpline &spl)
{
    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagSpline);
    if (domElement.isElement())
    {
        doc->SetAttribute(domElement, AttrPoint1,  spl.GetP1().id());
        doc->SetAttribute(domElement, AttrPoint4,  spl.GetP4().id());
        doc->SetAttribute(domElement, AttrAngle1,  formulaNamesToIds(spl.GetStartAngleFormula(), nameToIdToken));
        doc->SetAttribute(domElement, AttrAngle2,  formulaNamesToIds(spl.GetEndAngleFormula(), nameToIdToken));
        doc->SetAttribute(domElement, AttrLength1, formulaNamesToIds(spl.GetC1LengthFormula(), nameToIdToken));
        doc->SetAttribute(domElement, AttrLength2, formulaNamesToIds(spl.GetC2LengthFormula(), nameToIdToken));

        emit NeedLiteParsing(Document::LiteParse);
    }
    else
    {
        qCDebug(vUndo, "Can't find spline with id = %u.", nodeId);
        return;
    }
}

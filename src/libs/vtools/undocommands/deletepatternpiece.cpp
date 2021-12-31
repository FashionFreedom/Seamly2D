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
 **  @file   deletepatternpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2014
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

#include "deletepatternpiece.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../vmisc/logging.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
DeletePatternPiece::DeletePatternPiece(VAbstractPattern *doc, const QString &draftBlockName, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , draftBlockName(draftBlockName)
    , patternPiece(QDomElement())
    , previousPPName(QString())
{
    setText(tr("delete pattern piece %1").arg(draftBlockName));

    const QDomElement patternP = doc->GetPPElement(draftBlockName);
    patternPiece = patternP.cloneNode().toElement();
    const QDomElement previousPP = patternP.previousSibling().toElement();//find previous pattern piece
    if (not previousPP.isNull() && previousPP.tagName() == VAbstractPattern::TagDraw)
    {
        previousPPName = doc->GetParametrString(previousPP, VAbstractPattern::AttrName, "");
    }
}

//---------------------------------------------------------------------------------------------------------------------
DeletePatternPiece::~DeletePatternPiece()
{}

//---------------------------------------------------------------------------------------------------------------------
void DeletePatternPiece::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement rootElement = doc->documentElement();

    if (not previousPPName.isEmpty())
    { // not first in the list, add after tag draw
        const QDomNode previousPP = doc->GetPPElement(previousPPName);
        rootElement.insertAfter(patternPiece, previousPP);
    }
    else
    { // first in the list, add before tag draw
        const QDomNodeList list = rootElement.elementsByTagName(VAbstractPattern::TagDraw);
        QDomElement draw;

        if (not list.isEmpty())
        {
            draw = list.at(0).toElement();
        }

        Q_ASSERT_X(not draw.isNull(), Q_FUNC_INFO, "Couldn't' find tag draw");
        rootElement.insertBefore(patternPiece, draw);
    }

    emit NeedFullParsing();
    doc->changeActiveDraftBlock(draftBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePatternPiece::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement rootElement = doc->documentElement();
    const QDomElement patternPiece = doc->GetPPElement(draftBlockName);
    rootElement.removeChild(patternPiece);
    emit NeedFullParsing();
}

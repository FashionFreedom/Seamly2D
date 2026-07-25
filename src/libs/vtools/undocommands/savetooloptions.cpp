/***************************************************************************
 **  @file   savetooloptions.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
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

/************************************************************************
 **  @file   savetooloptions.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "savetooloptions.h"

#include <QDomNode>
#include <QStringList>
#include <QTextStream>

#include "../vmisc/def.h"
#include "../vmisc/logging.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/// @brief childElements serialize the child elements of a tool element.
///
/// Some tools keep object references in child elements instead of attributes, for example the points of a
/// spline path. Text nodes are skipped.
///
/// @param element tool element.
/// @return the child elements as one string.
//---------------------------------------------------------------------------------------------------------------------

QString childElements(const QDomElement &element)
{
    QString dump;
    QTextStream stream(&dump);

    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        child.save(stream, 0);
        child = child.nextSiblingElement();
    }
    return dump;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief referencesChanged check if a tool option change replaced a referenced object.
///
/// The reference counters that guard deletion are only set when a tool is created during a full parse.
/// When an option change replaces a referenced object the counters become stale, so the caller has to
/// request a full parse to rebuild them. See issue #1521.
///
/// @param oldXml tool element before the change.
/// @param newXml tool element after the change.
/// @return true if an object reference changed.
//---------------------------------------------------------------------------------------------------------------------

bool referencesChanged(const QDomElement &oldXml, const QDomElement &newXml)
{
    // Attributes of the draw tools that store the id of another object.
    static const QStringList reference_attributes = QStringList()
        << AttrBasePoint << AttrFirstPoint << AttrSecondPoint << AttrThirdPoint
        << AttrCenter << AttrCCenter << AttrC1Center << AttrC2Center
        << AttrArc << AttrFirstArc << AttrSecondArc
        << AttrCurve << AttrCurve1 << AttrCurve2
        << AttrPoint1 << AttrPoint2 << AttrPoint3 << AttrPoint4
        << AttrP1Line << AttrP2Line << AttrP1Line1 << AttrP2Line1 << AttrP1Line2 << AttrP2Line2
        << AttrAxisP1 << AttrAxisP2 << AttrTangent << AttrPShoulder
        << AttrDartP1 << AttrDartP2 << AttrDartP3
        << AttrBaseLineP1 << AttrBaseLineP2;

    for (int i = 0; i < reference_attributes.size(); ++i)
    {
        if (oldXml.attribute(reference_attributes.at(i)) != newXml.attribute(reference_attributes.at(i)))
        {
            return true;
        }
    }

    return childElements(oldXml) != childElements(newXml);
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
SaveToolOptions::SaveToolOptions(const QDomElement &oldXml, const QDomElement &newXml, VAbstractPattern *doc,
                                 const quint32 &id, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent), oldXml(oldXml), newXml(newXml)
{
    setText(tr("save tool option"));
    nodeId = id;
}

//---------------------------------------------------------------------------------------------------------------------
SaveToolOptions::~SaveToolOptions()
{}

//---------------------------------------------------------------------------------------------------------------------
void SaveToolOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = doc->elementById(nodeId);
    if (domElement.isElement())
    {
        domElement.parentNode().replaceChild(oldXml, domElement);

        if (referencesChanged(oldXml, newXml))
        {
            // A referenced object changed. Only a full parse rebuilds the reference counters. See issue #1521.
            emit NeedFullParsing();
        }
        else
        {
            emit NeedLiteParsing(Document::LiteParse);
        }
    }
    else
    {
        qCWarning(vUndo, "Can't find tool with id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SaveToolOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->elementById(nodeId);
    if (domElement.isElement())
    {
        domElement.parentNode().replaceChild(newXml, domElement);

        if (referencesChanged(oldXml, newXml))
        {
            // A referenced object changed. Only a full parse rebuilds the reference counters. See issue #1521.
            emit NeedFullParsing();
        }
        else
        {
            emit NeedLiteParsing(Document::LiteParse);
        }
    }
    else
    {
        qCWarning(vUndo, "Can't find tool with id = %u.", nodeId);
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool SaveToolOptions::mergeWith(const QUndoCommand *command)
{
    const SaveToolOptions *saveCommand = static_cast<const SaveToolOptions *>(command);
    SCASSERT(saveCommand != nullptr)
    const quint32 id = saveCommand->getToolId();

    if (id != nodeId)
    {
        return false;
    }

    newXml = saveCommand->getNewXml();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int SaveToolOptions::id() const
{
    return static_cast<int>(UndoCommand::SaveToolOptions);
}

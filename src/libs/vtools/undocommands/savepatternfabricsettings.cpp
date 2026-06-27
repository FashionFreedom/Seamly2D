/******************************************************************************
*   @file   savepatternfabricsettings.cpp
**  @author Douglas S Caskey
**  @date   27 Jun, 2026
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2026 Seamly2D project
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

#include "savepatternfabricsettings.h"

#include "../ifc/xml/vabstractpattern.h"

//---------------------------------------------------------------------------------------------------------------------
SavePatternFabricSettings::SavePatternFabricSettings(const VFabricSettings &oldSettings,
                                                     const VFabricSettings &newSettings,
                                                     VAbstractPattern *doc, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent)
    , m_oldSettings(oldSettings)
    , m_newSettings(newSettings)
    , m_firstRedo(true)
{
    setText(tr("save fabric settings"));
}

//---------------------------------------------------------------------------------------------------------------------
SavePatternFabricSettings::~SavePatternFabricSettings()
{}

//---------------------------------------------------------------------------------------------------------------------
void SavePatternFabricSettings::undo()
{
    doc->SetFabricSettings(m_oldSettings);
    emit NeedLiteParsing(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void SavePatternFabricSettings::redo()
{
    if (m_firstRedo)
    {
        m_firstRedo = false;
    }
    doc->SetFabricSettings(m_newSettings);
    emit NeedLiteParsing(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
bool SavePatternFabricSettings::mergeWith(const QUndoCommand *command)
{
    if (command->id() != id())
    {
        return false;
    }

    const SavePatternFabricSettings *other = static_cast<const SavePatternFabricSettings *>(command);
    m_newSettings = other->m_newSettings;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int SavePatternFabricSettings::id() const
{
    return static_cast<int>(UndoCommand::SavePatternFabricSettings);
}

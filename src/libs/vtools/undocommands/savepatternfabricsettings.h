/******************************************************************************
*   @file   savepatternfabricsettings.h
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

#ifndef SAVEPATTERNFABRICSETTINGS_H
#define SAVEPATTERNFABRICSETTINGS_H

#include <QtGlobal>

#include "../ifc/xml/vfabricsettings.h"
#include "vundocommand.h"

class SavePatternFabricSettings : public VUndoCommand
{
public:
    SavePatternFabricSettings(const VFabricSettings &oldSettings, const VFabricSettings &newSettings,
                              VAbstractPattern *doc, QUndoCommand *parent = nullptr);

    virtual ~SavePatternFabricSettings();

    virtual void undo() override;
    virtual void redo() override;
    virtual bool mergeWith(const QUndoCommand *command) override;
    virtual int  id() const override;

private:
    Q_DISABLE_COPY(SavePatternFabricSettings)

    const VFabricSettings m_oldSettings;
    VFabricSettings       m_newSettings;
    bool                  m_firstRedo;
};

#endif // SAVEPATTERNFABRICSETTINGS_H

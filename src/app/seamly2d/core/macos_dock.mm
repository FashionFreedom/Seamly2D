//-----------------------------------------------------------------------------
//  @file   macos_dock.mm
//  @author palepal
//  @date   18 Oct, 2026
//
//  @copyright
//  Copyright (C) 2026 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "macos_dock.h"

#import <AppKit/AppKit.h>

//---------------------------------------------------------------------------------------------------------------------
/// @brief showDockIcon make the app a regular Dock app with a menu bar.
///
/// Info.plist sets LSUIElement so console mode (export / test) never gets a Dock icon; GUI mode calls this.
/// Must run before the event loop starts so the menu bar and activation behave as for a regular app.
void showDockIcon()
{
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
}

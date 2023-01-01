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
 **  @file   projectversion.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#ifndef PROJECTVERSION_H
#define PROJECTVERSION_H

class QString;

extern const int MAJOR_VERSION;
extern const int MINOR_VERSION;
extern const int DEBUG_VERSION;
extern const int SUPER_MINOR__VERSION;

extern const QString APP_VERSION_STR;

/*
   APP_VERSION is (major << 16) + (minor << 8) + patch.
*/
#define APP_VERSION ((MAJOR_VERSION << 16) + (MINOR_VERSION << 8) + DEBUG_VERSION)

// Start: Do not edit here, use scripts/version.sh to update
#define VER_FILEVERSION     MAJOR_VERSION,MINOR_VERSION,DEBUG_VERSION,SUPER_MINOR__VERSION
#define VER_FILEVERSION_STR "0.6.0.1"
// End: Do not edit here

#define V_PRERELEASE // Mark prerelease builds

#define VER_PRODUCTVERSION          VER_FILEVERSION
#define VER_PRODUCTVERSION_STR      VER_FILEVERSION_STR

#define VER_INTERNALNAME_2D_STR     "Seamly2D"
#define VER_COMPANYNAME_STR         "Seamly2DTeam"
#define VER_LEGALCOPYRIGHT_STR      "Copyright © 2014-2022 Seamly2D Team"
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_COMPANYDOMAIN_STR       "https://seamly.net"

QString compilerString();
QString buildCompatibilityString();

#endif // PROJECTVERSION_H

//-----------------------------------------------------------------------------
//  @file   vseamlymesettings.cpp
//  @author Douglas S Caskey
//  @date   13 July, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   vseamlymesettings.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 7, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "vseamlymesettings.h"

#include <QString>
#include <QVariant>

const QString settingDataBaseGeometry = QStringLiteral("database/geometry");

const QString settingDefHeight = QStringLiteral("gradation/defHeight");
const QString settingDefSize   = QStringLiteral("gradation/defSize");

//---------------------------------------------------------------------------------------------------------------------
VSeamlyMeSettings::VSeamlyMeSettings(Format format, Scope scope, const QString &organization,
                                     const QString &application, QObject *parent)
    :VCommonSettings(format, scope, organization, application, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
VSeamlyMeSettings::VSeamlyMeSettings(const QString &fileName, QSettings::Format format, QObject *parent)
    : VCommonSettings(fileName, format, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
QByteArray VSeamlyMeSettings::getDataBaseGeometry() const
{
    return value(settingDataBaseGeometry).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VSeamlyMeSettings::setDataBaseGeometry(const QByteArray &value)
{
    setValue(settingDataBaseGeometry, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSeamlyMeSettings::SetDefHeight(int value)
{
    setValue(settingDefHeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VSeamlyMeSettings::GetDefHeight() const
{
    return value(settingDefHeight, 176).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VSeamlyMeSettings::SetDefSize(int value)
{
    setValue(settingDefSize, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VSeamlyMeSettings::GetDefSize() const
{
    return value(settingDefSize, 50).toInt();
}

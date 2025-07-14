//-----------------------------------------------------------------------------
//  @file   vseamlymesettings.h
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
//  @file   vseamlymesettings.h
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

#ifndef VSEAMLYMESETTINGS_H
#define VSEAMLYMESETTINGS_H

#include <QByteArray>
#include <QChar>
#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtGlobal>

#include "vcommonsettings.h"

class VSeamlyMeSettings : public VCommonSettings
{
    Q_OBJECT

public:
               VSeamlyMeSettings(Format format, Scope scope, const QString &organization,
                                 const QString &application = QString(),
                                 QObject *parent = nullptr);

               VSeamlyMeSettings(const QString &fileName, Format format, QObject *parent = nullptr);

    QByteArray getDataBaseGeometry() const;
    void       setDataBaseGeometry(const QByteArray &value);

    void       SetDefHeight(int value);
    int        GetDefHeight() const;

    void       SetDefSize(int value);
    int        GetDefSize() const;

private:
    Q_DISABLE_COPY(VSeamlyMeSettings)
};

#endif // VSEAMLYMESETTINGS_H

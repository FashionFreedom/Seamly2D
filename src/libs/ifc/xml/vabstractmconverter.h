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
 **  @file   vabstractmconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 9, 2015
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

#ifndef VABSTRACTMCONVERTER_H
#define VABSTRACTMCONVERTER_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMultiMap>
#include <QString>
#include <QtGlobal>

#include "vabstractconverter.h"

class VAbstractMConverter : public VAbstractConverter
{
public:
    explicit VAbstractMConverter(const QString &fileName);
    virtual ~VAbstractMConverter() Q_DECL_EQ_DEFAULT;

protected:
    void AddRootComment();
    static QMultiMap<QString, QString> OldNamesToNewNames_InV0_3_0();
    static QMap<QString, QString> OldNamesToNewNames_InV0_3_3();

private:
    Q_DISABLE_COPY(VAbstractMConverter)
};

#endif // VABSTRACTMCONVERTER_H

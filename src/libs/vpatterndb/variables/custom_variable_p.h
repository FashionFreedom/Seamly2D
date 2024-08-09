//  @file   custom_variable_p.h
//  @author Douglas S Caskey
//  @date   2  Apr, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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

/************************************************************************
 **
 **  @file   vincrement_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
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

#ifndef CUSTOM_VARIABLE_P_H
#define CUSTOM_VARIABLE_P_H

#include <QSharedData>

#include "../ifc/ifcdef.h"
#include "../vcontainer.h"
#include "../vmisc/diagnostic.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class CustomVariableData : public QSharedData
{
public:

    CustomVariableData()
        : index(NULL_ID)
        , formula(QString())
        , formulaOk(false)
        , data(VContainer(nullptr, nullptr))
    {}

    CustomVariableData(VContainer *data, quint32 index, const QString &formula, bool ok)
        : index(index)
        , formula(formula)
        , formulaOk(ok)
        , data(*data)
    {}

    CustomVariableData(const CustomVariableData &variable)
        : QSharedData(variable)
        , index(variable.index)
        , formula(variable.formula)
        , formulaOk(variable.formulaOk)
        , data(variable.data)
    {}

    virtual  ~CustomVariableData();

    /** @brief id each variable have unique identificator */
    quint32    index;
    QString    formula;
    bool       formulaOk;
    VContainer data;

private:
    CustomVariableData &operator=(const CustomVariableData &) Q_DECL_EQ_DELETE;
};

CustomVariableData::~CustomVariableData()
{}

QT_WARNING_POP

#endif // CUSTOM_VARIABLE_P_H

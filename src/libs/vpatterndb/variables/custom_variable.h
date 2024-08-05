//  @file   custom_variable.h
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
 **  @file   vincrementtablerow.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef CUSTOM_VARIABLE_H
#define CUSTOM_VARIABLE_H

#include <qcompilerdetection.h>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

#include "vvariable.h"

class CustomVariableData;
class VContainer;

/**
 * @brief The CustomVariable class keep data row of variables table
 */
class CustomVariable :public VVariable
{
public:
                CustomVariable();
                CustomVariable(VContainer *data, const QString &name, quint32 index, qreal base, const QString &formula, bool ok,
                               const QString &description = QString());
                CustomVariable(const CustomVariable &variable);

    virtual    ~CustomVariable() override;

                CustomVariable &operator=(const CustomVariable &variable);

#ifdef Q_COMPILER_RVALUE_REFS
	            CustomVariable &operator=(CustomVariable &&variable) Q_DECL_NOTHROW;
#endif

    void        Swap(CustomVariable &variable) Q_DECL_NOTHROW;
    quint32     getIndex() const;
    QString     GetFormula() const;
    bool        IsFormulaOk() const;
    VContainer *GetData();

private:
    QSharedDataPointer<CustomVariableData> d;
};

Q_DECLARE_TYPEINFO(CustomVariable, Q_MOVABLE_TYPE);

#endif // CUSTOM_VARIABLE_H

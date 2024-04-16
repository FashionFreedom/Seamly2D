//  @file   custom_variable.cpp
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
 **  @file   vincrementtablerow.cpp
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

#include "custom_variable.h"
#include "custom_variable_p.h"

#include "../vmisc/def.h"
#include "vvariable.h"

#ifdef Q_COMPILER_RVALUE_REFS
CustomVariable &CustomVariable::operator=(CustomVariable &&variable) Q_DECL_NOTHROW { Swap(variable); return *this; }
#endif

void CustomVariable::Swap(CustomVariable &variable) Q_DECL_NOTHROW
{ VVariable::Swap(variable); std::swap(d, variable.d); }

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CustomVariable create enpty variable
 */
CustomVariable::CustomVariable()
    : VVariable()
    , d(new CustomVariableData)
{
    SetType(VarType::Variable);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CustomVariableTableRow create variable
 * @param name variable's name
 * @param base value
 * @param description description of variable
 */
CustomVariable::CustomVariable(VContainer *data, const QString &name, quint32 index, qreal base, const QString &formula,
                       bool ok, const QString &description)
    : VVariable(name, description)
    , d(new CustomVariableData(data, index, formula, ok))
{
    SetType(VarType::Variable);
    VInternalVariable::SetValue(base);
}

//---------------------------------------------------------------------------------------------------------------------
CustomVariable::CustomVariable(const CustomVariable &variable)
    : VVariable(variable)
    , d(variable.d)
{}

//---------------------------------------------------------------------------------------------------------------------
CustomVariable &CustomVariable::operator=(const CustomVariable &variable)
{
    if ( &variable == this )
    {
        return *this;
    }
    VVariable::operator=(variable);
    d = variable.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
CustomVariable::~CustomVariable()
{}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getIndex return index of row
 * @return index
 */
quint32 CustomVariable::getIndex() const
{
    return d->index;
}

//---------------------------------------------------------------------------------------------------------------------
QString CustomVariable::GetFormula() const
{
    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
bool CustomVariable::IsFormulaOk() const
{
    return d->formulaOk;
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *CustomVariable::GetData()
{
    return &d->data;
}

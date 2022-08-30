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
 **  @file   calculator.cpp
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

#include "calculator.h"

#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QStringList>

#include "../vmisc/def.h"
#include "../qmuparser/qmuparsererror.h"
#include "variables/vinternalvariable.h"
#include <QSharedPointer>
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Calculator class wrapper for QMuParser. Make easy initialization math parser.
 *
 * This constructor hides initialization variables, operators, character sets.
 * Use this constructor for evaluation formula. All formulas must be converted to internal look.
 * Example:
 *
 * const QString formula = qApp->FormulaFromUser(edit->text());
 * Calculator *cal = new Calculator(data, patternType);
 * const qreal result = cal->EvalFormula(data->PlainVariables(), formula);
 * delete cal;
 *
 */
Calculator::Calculator()
    :QmuFormulaBase()
{
    InitCharSets();
    setAllowSubexpressions(false);//Only one expression per time

    SetSepForEval();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief eval calculate formula.
 *
 * First we try eval expression without adding variables. If it fails, we take tokens from expression and add variables
 * to parser and try again.
 *
 * @param formula string of formula.
 * @return value of formula.
 */
qreal Calculator::EvalFormula(const QHash<QString, QSharedPointer<VInternalVariable>> *vars, const QString &formula)
{
    // Parser doesn't know any variable at this stage. So, we just use variable factory that for each unknown variable
    // set value to 0.
    SetVarFactory(AddVariable, this);
    SetSepForEval();//Reset separators options

    SetExpr(formula);

    qreal result = 0;
    result = Eval();

    QMap<int, QString> tokens = this->GetTokens();

    // Remove "-" from tokens list if exist or else unary minus operation will broken.
    RemoveAll(tokens, QStringLiteral("-"));

    for (int i = 0; i < builInFunctions.size(); ++i)
    {
        if (tokens.isEmpty())
        {
            break;
        }
        RemoveAll(tokens, builInFunctions.at(i));
    }

    if (tokens.isEmpty())
    {
        return result; // We have found only numbers in expression.
    }

    // Add variables to parser because we have deal with expression with variables.
    InitVariables(vars, tokens, formula);
    return Eval();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Calculator::InitVariables add variables to parser.
 *
 * For optimization purpose we try not to add variables that we don't need.
 *
 * @param vars list of variables.
 * @param tokens all tokens (measurements names, variables with lengths) that parser has found in expression.
 * @param formula expression, need for throwing better error message.
 */
void Calculator::InitVariables(const QHash<QString, QSharedPointer<VInternalVariable> > *vars,
                               const QMap<int, QString> &tokens, const QString &formula)
{
    QMap<int, QString>::const_iterator i = tokens.constBegin();
    while (i != tokens.constEnd())
    {
        bool found = false;
        if (vars->contains(i.value()))
        {
            DefineVar(i.value(), vars->value(i.value())->GetValue());
            found = true;
        }

        if (found == false && builInFunctions.contains(i.value()))
        {// We have found built-in function
            found = true;
        }

        if (found == false)
        {
            throw qmu::QmuParserError (qmu::ecUNASSIGNABLE_TOKEN, i.value(), formula, i.key());
        }
        ++i;
    }
}

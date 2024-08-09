/***************************************************************************
 **  @file   vformula.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vformula.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vformula.h"

#include <qnumeric.h>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QtDebug>

#include "../qmuparser/qmuparsererror.h"
#include "../vgeometry/../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "calculator.h"
#include "vcontainer.h"
#include "vtranslatevars.h"

//VFormula
//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula()
    :formula(QString()), value(tr("Error")), checkZero(true), data(nullptr), toolId(NULL_ID),
      postfix(QString()), _error(true), dValue(0)
{}

//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula(const QString &formula, const VContainer *container)
    : formula(qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator())),
      value(tr("Error")),
      checkZero(true),
      data(container),
      toolId(NULL_ID),
      postfix(QString()),
      _error(true),
      dValue(0)
{
    this->formula.replace("\n", " ");// Replace line return with spaces for calc if exist
    Eval();
}

//---------------------------------------------------------------------------------------------------------------------
VFormula &VFormula::operator=(const VFormula &formula)
{
    if ( &formula == this )
    {
        return *this;
    }
    this->formula = formula.GetFormula();
    this->value = formula.getStringValue();
    this->checkZero = formula.getCheckZero();
    this->data = formula.getData();
    this->toolId = formula.getToolId();
    this->postfix = formula.getPostfix();
    this->_error = formula.error();
    this->dValue = formula.getDoubleValue();
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula(const VFormula &formula)
    :formula(formula.GetFormula()), value(formula.getStringValue()), checkZero(formula.getCheckZero()),
      data(formula.getData()), toolId(formula.getToolId()), postfix(formula.getPostfix()), _error(formula.error()),
      dValue(formula.getDoubleValue())
{}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::operator==(const VFormula &formula) const
{
    bool isEqual = false;
    if (this->formula == formula.GetFormula() && this->value == formula.getStringValue() &&
        this->checkZero == formula.getCheckZero() && this->data == formula.getData() &&
        this->toolId == formula.getToolId() && this->postfix == formula.getPostfix() &&
        this->_error == formula.error() && VFuzzyComparePossibleNulls(this->dValue, formula.getDoubleValue()))
    {
        isEqual = true;
    }
    return isEqual;
}

bool VFormula::operator!=(const VFormula &formula) const
{
    return !VFormula::operator==(formula);
}

//---------------------------------------------------------------------------------------------------------------------
QString VFormula::GetFormula(FormulaType type) const
{
    if (type == FormulaType::ToUser)
    {
        return formula;
    }
    else
    {
        return qApp->translateVariables()->TryFormulaFromUser(formula, qApp->Settings()->getOsSeparator());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::SetFormula(const QString &value, FormulaType type)
{
    if (formula != value)
    {
        if (type == FormulaType::ToUser)
        {
            formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
        }
        else
        {
            formula = value;
        }
        formula.replace("\n", " ");// Replace line return with spaces for calc if exist
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VFormula::getStringValue() const
{
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VFormula::getDoubleValue() const
{
    return dValue;
}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::getCheckZero() const
{
    return checkZero;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setCheckZero(bool value)
{
    if (checkZero != value)
    {
        checkZero = value;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
const VContainer *VFormula::getData() const
{
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setData(const VContainer *value)
{
    if (data != value && value != nullptr)
    {
        data = value;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VFormula::getToolId() const
{
    return toolId;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setToolId(const quint32 &value)
{
    toolId = value;
}

//---------------------------------------------------------------------------------------------------------------------
QString VFormula::getPostfix() const
{
    return postfix;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setPostfix(const QString &value)
{
    if (postfix != value)
    {
        postfix = value;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::error() const
{
    return _error;
}

//---------------------------------------------------------------------------------------------------------------------
int VFormula::FormulaTypeId()
{
    return qMetaTypeId<VFormula>();
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::Eval()
{
    if (data == nullptr)
    {
        return;
    }
    if (formula.isEmpty())
    {
        value = tr("Error");
        _error = true;
        dValue = 0;
    }
    else
    {
        try
        {
            QScopedPointer<Calculator> cal(new Calculator());
            QString expression = qApp->translateVariables()->FormulaFromUser(formula, qApp->Settings()->getOsSeparator());
            const qreal result = cal->EvalFormula(data->DataVariables(), expression);

            if (qIsInf(result) || qIsNaN(result))
            {
                value = tr("Error");
                _error = true;
                dValue = 0;
            }
            else
            {
                //if result equal 0
                if (checkZero && qFuzzyIsNull(result))
                {
                    value = QString("0");
                    _error = true;
                    dValue = 0;
                }
                else
                {
                    dValue = result;
                    value = QString(qApp->LocaleToString(result) + " " + postfix);
                    _error = false;
                }
            }
        }
        catch (qmu::QmuParserError &error)
        {
            value = tr("Error");
            _error = true;
            dValue = 0;
            qDebug() << "\nMath parser error:\n"
                       << "--------------------------------------\n"
                       << "Message:     " << error.GetMsg()  << "\n"
                       << "Expression:  " << error.GetExpr() << "\n"
                       << "--------------------------------------";
        }
    }
}

// @file   vformula.cpp
// @author Douglas S Caskey
// @date   14 May, 2026
//
// @copyright
// Copyright (C) 2017 - 2026 Seamly, LLC
// https://github.com/fashionfreedom/seamly2d
//
// @brief
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

//-----------------------------------------------------------------------------
//
//  @file   vformula.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   28 Aug, 2014
//
//  @copyright
//  Copyright (C) 2014 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

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
    : m_formula(QString())
    , m_valueStr(tr("Error"))
    , m_checkZero(true)
    , m_data(nullptr)
    , m_toolId(NULL_ID)
    , m_postfix(QString())
    , m_error(true)
    , m_value(0)
{}

//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula(const QString &formula, const VContainer *container)
    : m_formula(qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator()))
    , m_valueStr(tr("Error"))
    , m_checkZero(true)
    , m_data(container)
    , m_toolId(NULL_ID)
    , m_postfix(QString())
    , m_error(true)
    , m_value(0)
{
    m_formula.replace("\n", " ");// Replace line return with spaces for calc if exist
    Eval();
}

//---------------------------------------------------------------------------------------------------------------------
VFormula &VFormula::operator=(const VFormula &formula)
{
    if ( &formula == this )
    {
        return *this;
    }
    m_formula   = formula.GetFormula();
    m_valueStr  = formula.getStringValue();
    m_checkZero = formula.getCheckZero();
    m_data      = formula.getData();
    m_toolId    = formula.getToolId();
    m_postfix   = formula.getPostfix();
    m_error     = formula.error();
    m_value     = formula.getDoubleValue();
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VFormula::VFormula(const VFormula &formula)
    : m_formula(formula.GetFormula())
    , m_valueStr(formula.getStringValue())
    , m_checkZero(formula.getCheckZero())
    , m_data(formula.getData())
    , m_toolId(formula.getToolId())
    , m_postfix(formula.getPostfix())
    , m_error(formula.error())
    , m_value(formula.getDoubleValue())
{}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::operator==(const VFormula &formula) const
{
    bool isEqual = false;
    if (m_formula == formula.GetFormula() && m_valueStr == formula.getStringValue() &&
        m_checkZero == formula.getCheckZero() && m_data == formula.getData() &&
        m_toolId == formula.getToolId() && m_postfix == formula.getPostfix() &&
        m_error == formula.error() && VFuzzyComparePossibleNulls(m_value, formula.getDoubleValue()))
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
        return m_formula;
    }
    else
    {
        return qApp->translateVariables()->TryFormulaFromUser(m_formula, qApp->Settings()->getOsSeparator());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::SetFormula(const QString &text, FormulaType type)
{
    if (m_formula != text)
    {
        if (type == FormulaType::ToUser)
        {
            m_formula = qApp->translateVariables()->FormulaToUser(text, qApp->Settings()->getOsSeparator());
        }
        else
        {
            m_formula = text;
        }
        m_formula.replace("\n", " ");// Replace line return with spaces for calc if exist
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VFormula::getStringValue() const
{
    return m_valueStr;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VFormula::getDoubleValue() const
{
    return m_value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::getCheckZero() const
{
    return m_checkZero;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setCheckZero(bool value)
{
    if (m_checkZero != value)
    {
        m_checkZero = value;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
const VContainer *VFormula::getData() const
{
    return m_data;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setData(const VContainer *value)
{
    if (m_data != value && value != nullptr)
    {
        m_data = value;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VFormula::getToolId() const
{
    return m_toolId;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setToolId(const quint32 &value)
{
    m_toolId = value;
}

//---------------------------------------------------------------------------------------------------------------------
QString VFormula::getPostfix() const
{
    return m_postfix;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::setPostfix(const QString &text)
{
    if (m_postfix != text)
    {
        m_postfix = text;
        Eval();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VFormula::error() const
{
    return m_error;
}

//---------------------------------------------------------------------------------------------------------------------
int VFormula::FormulaTypeId()
{
    return qMetaTypeId<VFormula>();
}

//---------------------------------------------------------------------------------------------------------------------
void VFormula::Eval()
{
    if (m_data == nullptr)
    {
        return;
    }
    if (m_formula.isEmpty())
    {
        m_valueStr = tr("Error");
        m_error = true;
        m_value = 0;
    }
    else
    {
        try
        {
            QScopedPointer<Calculator> cal(new Calculator());
            QString expression = qApp->translateVariables()->FormulaFromUser(m_formula, qApp->Settings()->getOsSeparator());
            qreal result = cal->EvalFormula(m_data->DataVariables(), expression);

            if (qIsInf(result) || qIsNaN(result))
            {
                m_valueStr = tr("Error");
                m_error = true;
                m_value = 0;
            }
            else
            {
                //if result equal 0
                if (m_checkZero && qFuzzyIsNull(result))
                {
                    m_valueStr = QString("0");
                    m_error = true;
                    m_value = 0;
                }
                else
                {
                    if (m_postfix == degreeSymbol)
                    {
                        result = normalize(result, 0.0, 360.0);
                    }
                    m_value = result;
                    m_valueStr = QString(qApp->LocaleToString(result) + " " + m_postfix);
                    m_error = false;
                }
            }
        }
        catch (qmu::QmuParserError &error)
        {
            m_valueStr = tr("Error");
            m_error = true;
            m_value = 0;
            qDebug() << "\nMath parser error:\n"
                       << "--------------------------------------\n"
                       << "Message:     " << error.GetMsg()  << "\n"
                       << "Expression:  " << error.GetExpr() << "\n"
                       << "--------------------------------------";
        }
    }
}

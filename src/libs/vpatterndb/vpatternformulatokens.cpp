/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
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
 **************************************************************************/

#include "vpatternformulatokens.h"

#include "vcontainer.h"
#include "vformulaidtranslator.h"
#include "vcompositevariabletokens.h"

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> VPatternFormulaTokens::NameToIdTokenMap(const VContainer *data)
{
    QHash<QString, QString> nameToIdToken = VFormulaIdTranslator::NameToIdTokenMap(*data->DataGObjects());

    const QHash<QString, QString> composite =
        VCompositeVariableTokens::NameToIdTokenMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        nameToIdToken.insert(i.key(), i.value());
        ++i;
    }
    return nameToIdToken;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> VPatternFormulaTokens::IdTokenToNameMap(const VContainer *data)
{
    QHash<QString, QString> idTokenToName = VFormulaIdTranslator::IdTokenToNameMap(*data->DataGObjects());

    const QHash<QString, QString> composite =
        VCompositeVariableTokens::IdTokenToNameMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        idTokenToName.insert(i.key(), i.value());
        ++i;
    }
    return idTokenToName;
}

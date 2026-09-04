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

#include "patternformulatokens.h"

#include "vcontainer.h"
#include "formulaidtranslator.h"
#include "compositevariabletokens.h"

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> PatternFormulaTokens::nameToIdTokenMap(const VContainer *data)
{
    QHash<QString, QString> nameToIdToken = FormulaIdTranslator::nameToIdTokenMap(*data->DataGObjects());

    const QHash<QString, QString> composite = CompositeVariableTokens::nameToIdTokenMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        nameToIdToken.insert(i.key(), i.value());
        ++i;
    }
    return nameToIdToken;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> PatternFormulaTokens::idTokenToNameMap(const VContainer *data)
{
    QHash<QString, QString> idTokenToName = FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects());

    const QHash<QString, QString> composite = CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        idTokenToName.insert(i.key(), i.value());
        ++i;
    }
    return idTokenToName;
}

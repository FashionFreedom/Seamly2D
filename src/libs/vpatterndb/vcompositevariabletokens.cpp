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

#include "vcompositevariabletokens.h"

#include "vformulaidtranslator.h"
#include "variables/vinternalvariable.h"
#include "variables/vlinelength.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> VCompositeVariableTokens::NameToIdTokenMap(
    const QHash<QString, QSharedPointer<VInternalVariable>> &variables)
{
    QHash<QString, QString> nameToIdToken;

    QHash<QString, QSharedPointer<VInternalVariable>>::const_iterator i = variables.constBegin();
    while (i != variables.constEnd())
    {
        if (i.value()->GetType() == VarType::LineLength)
        {
            const QSharedPointer<VLengthLine> length = i.value().staticCast<VLengthLine>();
            nameToIdToken.insert(i.key(), line_ + VFormulaIdTranslator::IdToken(length->GetP1Id()) +
                                               QLatin1Char('_') + VFormulaIdTranslator::IdToken(length->GetP2Id()));
        }
        ++i;
    }
    return nameToIdToken;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> VCompositeVariableTokens::IdTokenToNameMap(
    const QHash<QString, QSharedPointer<VInternalVariable>> &variables)
{
    QHash<QString, QString> idTokenToName;

    const QHash<QString, QString> nameToIdToken = NameToIdTokenMap(variables);
    QHash<QString, QString>::const_iterator i = nameToIdToken.constBegin();
    while (i != nameToIdToken.constEnd())
    {
        idTokenToName.insert(i.value(), i.key());
        ++i;
    }
    return idTokenToName;
}

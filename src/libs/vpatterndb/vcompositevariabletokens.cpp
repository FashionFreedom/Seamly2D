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
#include "variables/vlineangle.h"
#include "variables/varcradius.h"
#include "variables/vcurveangle.h"
#include "variables/vcurveclength.h"
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
        switch (i.value()->GetType())
        {
            case VarType::LineLength:
            {
                const QSharedPointer<VLengthLine> length = i.value().staticCast<VLengthLine>();
                nameToIdToken.insert(i.key(), line_ + VFormulaIdTranslator::IdToken(length->GetP1Id()) +
                                                   QLatin1Char('_') +
                                                   VFormulaIdTranslator::IdToken(length->GetP2Id()));
                break;
            }
            case VarType::LineAngle:
            {
                const QSharedPointer<VLineAngle> angle = i.value().staticCast<VLineAngle>();
                nameToIdToken.insert(i.key(), angleLine_ + VFormulaIdTranslator::IdToken(angle->GetP1Id()) +
                                                   QLatin1Char('_') +
                                                   VFormulaIdTranslator::IdToken(angle->GetP2Id()));
                break;
            }
            case VarType::ArcRadius:
            {
                const QSharedPointer<VArcRadius> radius = i.value().staticCast<VArcRadius>();
                nameToIdToken.insert(i.key(), radius_V + QString::number(radius->GetNumberRadius()) +
                                                   VFormulaIdTranslator::IdToken(radius->GetId()));
                break;
            }
            case VarType::CurveAngle:
            {
                const QSharedPointer<VCurveAngle> angle = i.value().staticCast<VCurveAngle>();
                const QString &prefix = (angle->GetAngle() == CurveAngle::StartAngle) ? angle1_V : angle2_V;
                nameToIdToken.insert(i.key(), prefix + VFormulaIdTranslator::IdToken(angle->GetId()));
                break;
            }
            case VarType::CurveCLength:
            {
                const QSharedPointer<VCurveCLength> cLength = i.value().staticCast<VCurveCLength>();
                const QString &prefix = (cLength->GetCType() == CurveCLength::C1) ? c1Length_V : c2Length_V;
                nameToIdToken.insert(i.key(), prefix + VFormulaIdTranslator::IdToken(cLength->GetId()));
                break;
            }
            case VarType::CurveLength:
            case VarType::Measurement:
            case VarType::Variable:
            case VarType::Unknown:
            default:
                // CurveLength's plain (non-segmented) form is just the curve's own name - already covered by
                // VFormulaIdTranslator::NameToIdTokenMap() since a curve is a VGObject with its own id, same as
                // a point. Measurement/Variable are user-assigned identities, not derived from object names, so
                // they're intentionally left alone. Segment-suffixed composite names (multi-segment curve paths)
                // aren't covered yet - the segment index isn't currently exposed by these classes.
                break;
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

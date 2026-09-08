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

#include "compositevariabletokens.h"

#include "formulaidtranslator.h"
#include "variables/vinternalvariable.h"
#include "variables/vlinelength.h"
#include "variables/vlineangle.h"
#include "variables/varcradius.h"
#include "variables/vcurveangle.h"
#include "variables/vcurveclength.h"
#include "variables/vcurvelength.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"

using namespace FormulaIdTranslator;

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> CompositeVariableTokens::nameToIdTokenMap(
    const QHash<QString, QSharedPointer<VInternalVariable>> &variables)
{
    QHash<QString, QString> name_to_id_token;

    QHash<QString, QSharedPointer<VInternalVariable>>::const_iterator i = variables.constBegin();
    while (i != variables.constEnd())
    {
        switch (i.value()->GetType())
        {
            case VarType::LineLength:
            {
                const QSharedPointer<VLengthLine> length = i.value().staticCast<VLengthLine>();
                name_to_id_token.insert(i.key(), line_ + idToken(length->getLineId()));
                break;
            }
            case VarType::LineAngle:
            {
                const QSharedPointer<VLineAngle> angle = i.value().staticCast<VLineAngle>();
                name_to_id_token.insert(i.key(), angleLine_ + idToken(angle->getLineId()));
                break;
            }
            case VarType::ArcRadius:
            {
                const QSharedPointer<VArcRadius> radius = i.value().staticCast<VArcRadius>();
                name_to_id_token.insert(i.key(), radius_V + QString::number(radius->getNumberRadius()) +
                                                   idToken(radius->GetId()));
                break;
            }
            case VarType::CurveAngle:
            {
                const QSharedPointer<VCurveAngle> angle = i.value().staticCast<VCurveAngle>();
                const QString &prefix = (angle->getAngle() == CurveAngle::StartAngle) ? angle1_V : angle2_V;
                QString token = prefix + idToken(angle->GetId());
                if (angle->getSegment() > 0)
                {
                    token += QLatin1Char('_') + seg_ + QString::number(angle->getSegment());
                }
                name_to_id_token.insert(i.key(), token);
                break;
            }
            case VarType::CurveCLength:
            {
                const QSharedPointer<VCurveCLength> c_length = i.value().staticCast<VCurveCLength>();
                const QString &prefix = (c_length->getCType() == CurveCLength::C1) ? c1Length_V : c2Length_V;
                QString token = prefix + idToken(c_length->GetId());
                if (c_length->getSegment() > 0)
                {
                    token += QLatin1Char('_') + seg_ + QString::number(c_length->getSegment());
                }
                name_to_id_token.insert(i.key(), token);
                break;
            }
            case VarType::CurveLength:
            {
                const QSharedPointer<VCurveLength> length = i.value().staticCast<VCurveLength>();
                if (length->getSegment() > 0)
                {
                    name_to_id_token.insert(i.key(), idToken(length->GetId()) +
                                                       QLatin1Char('_') + seg_ +
                                                       QString::number(length->getSegment()));
                }
                else
                {
                    // Plain (non-segmented) form uses the same bare "id<n>" token a curve's own
                    // VGObject entry would produce, and is deliberately added here too rather than
                    // left to FormulaIdTranslator::nameToIdTokenMap(data->DataGObjects()) alone:
                    // a later draft block referencing an earlier block's curve can be translated
                    // before that curve's VGObject is visible in the currently active block's
                    // DataGObjects(), while the pattern-wide variable table (this map's source) is
                    // already populated - see the referencing point's own broken-formula symptom
                    // this was fixed for.
                    name_to_id_token.insert(i.key(), idToken(length->GetId()));
                }
                break;
            }
            case VarType::Measurement:
            case VarType::Variable:
            case VarType::Unknown:
            default:
                // Measurement/Variable are user-assigned identities, not derived from object names, so
                // they're intentionally left alone.
                break;
        }
        ++i;
    }
    return name_to_id_token;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> CompositeVariableTokens::idTokenToNameMap(
    const QHash<QString, QSharedPointer<VInternalVariable>> &variables)
{
    QHash<QString, QString> id_token_to_name;

    const QHash<QString, QString> name_to_id_token = nameToIdTokenMap(variables);
    QHash<QString, QString>::const_iterator i = name_to_id_token.constBegin();
    while (i != name_to_id_token.constEnd())
    {
        id_token_to_name.insert(i.value(), i.key());
        ++i;
    }
    return id_token_to_name;
}

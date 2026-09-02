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
    QHash<QString, QString> nameToIdToken;

    QHash<QString, QSharedPointer<VInternalVariable>>::const_iterator i = variables.constBegin();
    while (i != variables.constEnd())
    {
        switch (i.value()->GetType())
        {
            case VarType::LineLength:
            {
                const QSharedPointer<VLengthLine> length = i.value().staticCast<VLengthLine>();
                nameToIdToken.insert(i.key(), line_ + idToken(length->GetLineId()));
                break;
            }
            case VarType::LineAngle:
            {
                const QSharedPointer<VLineAngle> angle = i.value().staticCast<VLineAngle>();
                nameToIdToken.insert(i.key(), angleLine_ + idToken(angle->GetLineId()));
                break;
            }
            case VarType::ArcRadius:
            {
                const QSharedPointer<VArcRadius> radius = i.value().staticCast<VArcRadius>();
                nameToIdToken.insert(i.key(), radius_V + QString::number(radius->GetNumberRadius()) +
                                                   idToken(radius->GetId()));
                break;
            }
            case VarType::CurveAngle:
            {
                const QSharedPointer<VCurveAngle> angle = i.value().staticCast<VCurveAngle>();
                const QString &prefix = (angle->GetAngle() == CurveAngle::StartAngle) ? angle1_V : angle2_V;
                QString token = prefix + idToken(angle->GetId());
                if (angle->GetSegment() > 0)
                {
                    token += QLatin1Char('_') + seg_ + QString::number(angle->GetSegment());
                }
                nameToIdToken.insert(i.key(), token);
                break;
            }
            case VarType::CurveCLength:
            {
                const QSharedPointer<VCurveCLength> cLength = i.value().staticCast<VCurveCLength>();
                const QString &prefix = (cLength->GetCType() == CurveCLength::C1) ? c1Length_V : c2Length_V;
                QString token = prefix + idToken(cLength->GetId());
                if (cLength->GetSegment() > 0)
                {
                    token += QLatin1Char('_') + seg_ + QString::number(cLength->GetSegment());
                }
                nameToIdToken.insert(i.key(), token);
                break;
            }
            case VarType::CurveLength:
            {
                const QSharedPointer<VCurveLength> length = i.value().staticCast<VCurveLength>();
                if (length->GetSegment() > 0)
                {
                    nameToIdToken.insert(i.key(), idToken(length->GetId()) +
                                                       QLatin1Char('_') + seg_ +
                                                       QString::number(length->GetSegment()));
                }
                // Plain (non-segmented) form is just the curve's own name - already covered by
                // FormulaIdTranslator::nameToIdTokenMap() since a curve is a VGObject with its own id,
                // same as a point. Nothing to add here for that case.
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
    return nameToIdToken;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> CompositeVariableTokens::idTokenToNameMap(
    const QHash<QString, QSharedPointer<VInternalVariable>> &variables)
{
    QHash<QString, QString> idTokenToName;

    const QHash<QString, QString> nameToIdToken = nameToIdTokenMap(variables);
    QHash<QString, QString>::const_iterator i = nameToIdToken.constBegin();
    while (i != nameToIdToken.constEnd())
    {
        idTokenToName.insert(i.value(), i.key());
        ++i;
    }
    return idTokenToName;
}

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

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief isDisambiguatedVariantOf reports whether @p candidate is exactly what
 * VContainer::UniqueCompositeVariableName() would produce for @p base once @p base is already taken -
 * "<base>_2", "<base>_3", and so on.
 *
 * Two independently drawn arcs/curves (each with its own DataGObjects() identity) can legitimately
 * generate the identical raw display name (e.g. two literal splines connecting the same two points) -
 * there is no way to give both the plain name, so one of them keeps a disambiguated variant as ITS
 * OWN composite variable name, which then intentionally differs from the plain name DataGObjects()
 * reports for it. That is expected, not a bug - only a genuine mismatch (a composite name that isn't
 * simply a disambiguated variant of the base name) is worth a warning.
 */
bool isDisambiguatedVariantOf(const QString &candidate, const QString &base)
{
    const QString prefix = base + QLatin1Char('_');
    if (!candidate.startsWith(prefix))
    {
        return false;
    }

    const QString suffix = candidate.mid(prefix.length());
    if (suffix.isEmpty())
    {
        return false;
    }

    for (const QChar &c : suffix)
    {
        if (!c.isDigit())
        {
            return false;
        }
    }
    return true;
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> PatternFormulaTokens::nameToIdTokenMap(const VContainer *data)
{
    QHash<QString, QString> name_to_id_token = FormulaIdTranslator::nameToIdTokenMap(*data->DataGObjects());

    const QHash<QString, QString> composite = CompositeVariableTokens::nameToIdTokenMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        name_to_id_token.insert(i.key(), i.value());
        ++i;
    }
    return name_to_id_token;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> PatternFormulaTokens::idTokenToNameMap(const VContainer *data)
{
    QHash<QString, QString> id_token_to_name = FormulaIdTranslator::idTokenToNameMap(*data->DataGObjects());

    const QHash<QString, QString> composite = CompositeVariableTokens::idTokenToNameMap(*data->DataVariables());
    QHash<QString, QString>::const_iterator i = composite.constBegin();
    while (i != composite.constEnd())
    {
        // Two different composite variables should never resolve to the same internal id-token. If they do,
        // one of them is silently going to display the wrong name. This should be unreachable after the
        // id self-healing fix in VPattern::PrepareForParse(), but is kept as a loud safety net rather than
        // a hard assertion because a wrong displayed name is recoverable, a crash is not.
        //
        // Exception: the composite name legitimately differing from DataGObjects()'s plain name by
        // exactly the "_<N>" suffix VContainer::UniqueCompositeVariableName() appends is not a bug - it
        // is what disambiguating two independently drawn arcs/curves that generate the identical raw
        // name looks like (see issue #1678). Only a mismatch that ISN'T such a disambiguated variant
        // indicates two genuinely different objects sharing one internal id.
        const QHash<QString, QString>::const_iterator existing = id_token_to_name.constFind(i.key());
        if (existing != id_token_to_name.constEnd() && existing.value() != i.value() &&
            !isDisambiguatedVariantOf(i.value(), existing.value()))
        {
            qCWarning(vCon, "Id-token collision while merging composite variable names: token '%s' already maps "
                             "to '%s', ignoring conflicting name '%s'. This points at two objects sharing one "
                             "internal id.",
                      qUtf8Printable(i.key()), qUtf8Printable(existing.value()), qUtf8Printable(i.value()));
        }
        id_token_to_name.insert(i.key(), i.value());
        ++i;
    }
    return id_token_to_name;
}

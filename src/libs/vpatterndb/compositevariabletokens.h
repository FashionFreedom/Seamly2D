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

#ifndef COMPOSITEVARIABLETOKENS_H
#define COMPOSITEVARIABLETOKENS_H

#include <QHash>
#include <QSharedPointer>
#include <QString>

class VInternalVariable;

/**
 * @brief Builds the display-name <-> id-token pairs for derived/composite formula variables
 * (line length, angle, curve length, radius, ...), whose display name is glued together from
 * the names of the points/curves they're computed from.
 *
 * See FormulaIdTranslator for why this hands over whole-string pairs instead of asking the
 * translator to split a glued name apart itself - a constituent point's own name can contain
 * an underscore, so the glued string can't be reliably split back into its parts.
 *
 * Covers all six formula-visible variable types: line length, line angle, arc radius (plain and
 * elliptical), curve start/end angle, curve control-point length, and curve length - including
 * the segment-suffixed variants of the latter three that appear on multi-segment curve paths.
 * Plain (non-segmented) curve length needs no entry here - it's just the curve's own name,
 * already covered by FormulaIdTranslator::nameToIdTokenMap() since a curve is a VGObject with
 * its own id, same as a point. A variable type this namespace doesn't recognize (measurements,
 * user-defined custom variables) is simply left out of the map, which just means
 * FormulaIdTranslator leaves its formula tokens untouched.
 */
namespace CompositeVariableTokens
{
    QHash<QString, QString> nameToIdTokenMap(const QHash<QString, QSharedPointer<VInternalVariable>> &variables);
    QHash<QString, QString> idTokenToNameMap(const QHash<QString, QSharedPointer<VInternalVariable>> &variables);
}

#endif // COMPOSITEVARIABLETOKENS_H

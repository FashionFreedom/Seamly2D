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

#ifndef VCOMPOSITEVARIABLETOKENS_H
#define VCOMPOSITEVARIABLETOKENS_H

#include <QHash>
#include <QSharedPointer>
#include <QString>

class VInternalVariable;

/**
 * @brief Builds the display-name <-> id-token pairs for derived/composite formula variables
 * (line length, angle, curve length, radius, ...), whose display name is glued together from
 * the names of the points/curves they're computed from.
 *
 * See VFormulaIdTranslator for why this hands over whole-string pairs instead of asking the
 * translator to split a glued name apart itself - a constituent point's own name can contain
 * an underscore, so the glued string can't be reliably split back into its parts.
 *
 * Covers line length, line angle, arc radius (plain and elliptical), curve start/end angle and
 * curve control-point length. Plain curve length needs no entry here - it's just the curve's own
 * name, already covered by VFormulaIdTranslator::NameToIdTokenMap() since a curve is a VGObject
 * with its own id, same as a point. Not yet covered: the segment-suffixed variants of curve
 * length/angle/control-point-length that appear on multi-segment curve paths - those classes
 * don't currently expose the segment index needed to build an unambiguous token. A variable type
 * this class doesn't (yet) recognize is simply left out of the map, which just means
 * VFormulaIdTranslator leaves its formula tokens untouched for now.
 */
class VCompositeVariableTokens
{
public:
    static QHash<QString, QString> NameToIdTokenMap(
        const QHash<QString, QSharedPointer<VInternalVariable>> &variables);
    static QHash<QString, QString> IdTokenToNameMap(
        const QHash<QString, QSharedPointer<VInternalVariable>> &variables);
};

#endif // VCOMPOSITEVARIABLETOKENS_H

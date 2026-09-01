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

#ifndef VPATTERNFORMULATOKENS_H
#define VPATTERNFORMULATOKENS_H

#include <QHash>
#include <QString>

class VContainer;

/**
 * @brief Convenience entry point for translating formula text against a real, currently-loaded
 * pattern: merges VFormulaIdTranslator's plain-object map (points, curves, ...) with
 * VCompositeVariableTokens' derived-variable map (line length, angle, ...) so a caller reading or
 * writing a tool's formula only needs one map, built from whatever the container knows about at
 * that point in parsing.
 *
 * Deliberately built fresh from the container every time, not cached anywhere - the container
 * itself is the only source of truth for current names, so there's nothing to keep in sync.
 */
class VPatternFormulaTokens
{
public:
    static QHash<QString, QString> NameToIdTokenMap(const VContainer *data);
    static QHash<QString, QString> IdTokenToNameMap(const VContainer *data);
};

#endif // VPATTERNFORMULATOKENS_H

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

#ifndef VLINEVARIABLELOOKUP_H
#define VLINEVARIABLELOOKUP_H

#include <QSharedPointer>
#include <QtGlobal>

class VContainer;
class VLengthLine;
class VLineAngle;

/**
 * @brief Finds the VLengthLine/VLineAngle registered for a line's persisted id.
 *
 * A tool that implicitly registers a line (see VContainer::AddLine(), issue #1678) only ever
 * knows that line's own stable id, not the composite variable's current display name - the name
 * changes with the connected points, the id doesn't. These are the shared lookups other code
 * (e.g. the "Copy Length"/"Copy Angle" context menu actions) should use to go from that id to the
 * real object, rather than re-deriving a display name by hand and risking it drift out of sync
 * with how CompositeVariableTokens builds names.
 *
 * lineLengthsData()/lineAnglesData() are keyed by name, not by id, so this is a linear scan - the
 * same cost class as the id-token translation that already scans the whole variable set on every
 * formula load/save.
 *
 * @param data container with variables.
 * @param line_id the line's persisted id.
 * @return the matching object, or a null pointer if no line with that id is registered.
 */
QSharedPointer<VLengthLine> findLineLength(const VContainer &data, quint32 line_id);
QSharedPointer<VLineAngle>  findLineAngle(const VContainer &data, quint32 line_id);

/**
 * @brief Resolves a line's persisted id to the actual name it is registered under in the
 * container's variable table (i.e. the key lineLengthsData()/lineAnglesData() returns it keyed
 * by), NOT VLengthLine::GetName()/VLineAngle::GetName().
 *
 * VContainer::AddLine() may register a line under a disambiguated name that differs from the
 * plain "Line_<p1>_<p2>" the object was constructed with, when another unrelated line's endpoints
 * happen to produce the identical display name (see VContainer::UniqueCompositeVariableName(),
 * issue #1678 - point display names are not enforced globally unique). The object itself is never
 * told about that disambiguation: its own GetName() keeps returning the original, undisambiguated
 * string, which in that situation is a real but DIFFERENT line's name - pasting it into a formula
 * would silently resolve to the wrong line.
 *
 * @param data container with variables.
 * @param line_id the line's persisted id.
 * @return the name line_id is actually registered under, or an empty string if no line with that
 * id is registered.
 */
QString findLineLengthName(const VContainer &data, quint32 line_id);
QString findLineAngleName(const VContainer &data, quint32 line_id);

#endif // VLINEVARIABLELOOKUP_H

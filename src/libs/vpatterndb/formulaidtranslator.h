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

#ifndef FORMULAIDTRANSLATOR_H
#define FORMULAIDTRANSLATOR_H

#include <QHash>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vgobject.h"

/**
 * @brief Translates the tokens inside a formula to/from a stable, id-based form, so formulas
 * can be stored keyed by object id while the user only ever sees and edits names.
 *
 * This does not look anything up and does not know anything about how a token is built - the
 * caller supplies the full display-token <-> stored-token pairs, built from whatever objects
 * (points, but also derived values like a line length or angle, whose displayed name is built
 * by gluing constituent point names together) are relevant at the time. Deliberately whole-token
 * lookup only, never split apart: a glued-together display name like "Line_A1_A2" can't be
 * reliably split back into its parts by string position once a point name can itself contain an
 * underscore (e.g. "Halsloch_hinten"), so the caller must hand over the exact pairing instead of
 * this namespace trying to reconstruct it.
 */
namespace FormulaIdTranslator
{
    QString idToken(quint32 id);

    QString formulaNamesToIds(const QString &formula, const QHash<QString, QString> &nameToIdToken);
    QString formulaIdsToNames(const QString &formula, const QHash<QString, QString> &idTokenToName);

    // Builds the lookup straight off VContainer::DataGObjects() - no separate name/id table
    // is kept anywhere, this just reads each object's own id and current name. Covers plain
    // object names only; composite/derived-variable names are the caller's responsibility to
    // add to the map (there is no persisted object to enumerate them from).
    QHash<QString, QString> nameToIdTokenMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects);
    QHash<QString, QString> idTokenToNameMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects);
}

#endif // FORMULAIDTRANSLATOR_H

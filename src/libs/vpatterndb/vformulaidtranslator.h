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

#ifndef VFORMULAIDTRANSLATOR_H
#define VFORMULAIDTRANSLATOR_H

#include <QHash>
#include <QMap>
#include <QSharedPointer>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vgobject.h"

/**
 * @brief Translates the object names inside a formula to/from the stable numeric
 * object ids they refer to, so formulas can be stored keyed by id while the user
 * only ever sees and edits names.
 *
 * This does not look anything up itself - the caller supplies the name/id lookup,
 * built from whatever objects (points, lines, ...) are relevant at the time.
 */
class VFormulaIdTranslator
{
public:
    static QString IdToken(quint32 id);

    static QString FormulaNamesToIds(const QString &formula, const QHash<QString, quint32> &nameToId);
    static QString FormulaIdsToNames(const QString &formula, const QHash<quint32, QString> &idToName);

    // Builds the lookups straight off VContainer::DataGObjects() - no separate name/id table
    // is kept anywhere, this just reads each object's own id and current name.
    static QHash<QString, quint32> NameToIdMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects);
    static QHash<quint32, QString> IdToNameMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects);

private:
    static void Replace(QString &formula, const QString &newToken, int position, const QString &token, int &bias);
    static void CorrectionsPositions(int position, int bias, QMap<int, QString> &tokens);
};

#endif // VFORMULAIDTRANSLATOR_H

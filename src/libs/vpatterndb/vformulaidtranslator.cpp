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

#include "vformulaidtranslator.h"

#include <QList>
#include <QScopedPointer>

#include "../qmuparser/qmutokenparser.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IdToken builds the internal placeholder a formula stores in place of an object's name.
 *
 * Must start with a letter, not a digit - QmuParserBase::CheckName() rejects a leading digit,
 * so a bare number can't stand in for a variable reference.
 */
QString VFormulaIdTranslator::IdToken(quint32 id)
{
    return QStringLiteral("id%1").arg(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FormulaNamesToIds replaces every object-name token in formula with its id token.
 * @param formula formula as typed/displayed to the user, referencing object names
 * @param nameToId lookup from object name to its stable numeric id
 * @return formula with each known name replaced by IdToken(id); unknown tokens are left untouched
 */
QString VFormulaIdTranslator::FormulaNamesToIds(const QString &formula, const QHash<QString, quint32> &nameToId)
{
    if (formula.isEmpty())
    {
        return formula;
    }

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false));
    QMap<int, QString> tokens = cal->GetTokens();
    delete cal.take();

    QList<int> tKeys = tokens.keys();
    QList<QString> tValues = tokens.values();

    QString newFormula = formula;
    for (int i = 0; i < tValues.size(); ++i)
    {
        if (not nameToId.contains(tValues.at(i)))
        {
            continue;
        }

        int bias = 0;
        Replace(newFormula, IdToken(nameToId.value(tValues.at(i))), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        {
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FormulaIdsToNames replaces every id token in formula with the object's current name.
 * @param formula formula as stored internally, referencing IdToken() placeholders
 * @param idToName lookup from object id to its current name
 * @return formula with each known id token replaced by the object's current name; unknown tokens are left untouched
 */
QString VFormulaIdTranslator::FormulaIdsToNames(const QString &formula, const QHash<quint32, QString> &idToName)
{
    if (formula.isEmpty())
    {
        return formula;
    }

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false));
    QMap<int, QString> tokens = cal->GetTokens();
    delete cal.take();

    QList<int> tKeys = tokens.keys();
    QList<QString> tValues = tokens.values();

    QString newFormula = formula;
    for (int i = 0; i < tValues.size(); ++i)
    {
        const QString &token = tValues.at(i);
        if (not token.startsWith(QLatin1String("id")))
        {
            continue;
        }

        bool ok = false;
        const quint32 id = token.mid(2).toUInt(&ok);
        if (not ok || not idToName.contains(id))
        {
            continue;
        }

        int bias = 0;
        Replace(newFormula, idToName.value(id), tKeys.at(i), token, bias);
        if (bias != 0)
        {
            CorrectionsPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, quint32> VFormulaIdTranslator::NameToIdMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects)
{
    QHash<QString, quint32> nameToId;
    nameToId.reserve(gObjects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = gObjects.constBegin();
    while (i != gObjects.constEnd())
    {
        nameToId.insert(i.value()->name(), i.key());
        ++i;
    }
    return nameToId;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<quint32, QString> VFormulaIdTranslator::IdToNameMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects)
{
    QHash<quint32, QString> idToName;
    idToName.reserve(gObjects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = gObjects.constBegin();
    while (i != gObjects.constEnd())
    {
        idToName.insert(i.key(), i.value()->name());
        ++i;
    }
    return idToName;
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaIdTranslator::Replace(QString &formula, const QString &newToken, int position, const QString &token,
                                    int &bias)
{
    formula.replace(position, token.length(), newToken);
    bias = token.length() - newToken.length();
}

//---------------------------------------------------------------------------------------------------------------------
void VFormulaIdTranslator::CorrectionsPositions(int position, int bias, QMap<int, QString> &tokens)
{
    if (bias == 0)
    {
        return;
    }

    QMap<int, QString> newTokens;
    QMap<int, QString>::const_iterator i = tokens.constBegin();
    while (i != tokens.constEnd())
    {
        if (i.key() <= position)
        {
            newTokens.insert(i.key(), i.value());
        }
        else
        {
            newTokens.insert(i.key() - bias, i.value());
        }
        ++i;
    }
    tokens = newTokens;
}

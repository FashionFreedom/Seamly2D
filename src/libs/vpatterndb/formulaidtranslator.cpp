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

#include "formulaidtranslator.h"

#include <QList>
#include <QScopedPointer>

#include "../qmuparser/qmutokenparser.h"

using namespace FormulaIdTranslator;

namespace
{
    void replaceToken(QString &formula, const QString &newToken, int position, const QString &token, int &bias);
    void correctPositions(int position, int bias, QMap<int, QString> &tokens);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief idToken builds the internal placeholder a formula stores in place of an object's name.
 *
 * Must start with a letter, not a digit - QmuParserBase::CheckName() rejects a leading digit,
 * so a bare number can't stand in for a variable reference.
 */
QString FormulaIdTranslator::idToken(quint32 id)
{
    return QStringLiteral("id%1").arg(id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief formulaNamesToIds replaces every known display-name token in formula with its stored id token.
 * @param formula formula as typed/displayed to the user, referencing object/derived-variable names
 * @param nameToIdToken lookup from a whole display token (e.g. "A1" or "Line_A1_A2") to its stored id token
 * @return formula with each known token replaced; unknown tokens are left untouched
 */
QString FormulaIdTranslator::formulaNamesToIds(const QString &formula, const QHash<QString, QString> &nameToIdToken)
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
        if (not nameToIdToken.contains(tValues.at(i)))
        {
            continue;
        }

        int bias = 0;
        replaceToken(newFormula, nameToIdToken.value(tValues.at(i)), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        {
            correctPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief formulaIdsToNames replaces every known stored id token in formula with its current display name.
 * @param formula formula as stored internally, referencing id tokens
 * @param idTokenToName lookup from a whole stored id token (e.g. "id42" or "Line_id42_id17") to its current
 * display name
 * @return formula with each known token replaced; unknown tokens are left untouched
 */
QString FormulaIdTranslator::formulaIdsToNames(const QString &formula, const QHash<QString, QString> &idTokenToName)
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
        if (not idTokenToName.contains(tValues.at(i)))
        {
            continue;
        }

        int bias = 0;
        replaceToken(newFormula, idTokenToName.value(tValues.at(i)), tKeys.at(i), tValues.at(i), bias);
        if (bias != 0)
        {
            correctPositions(tKeys.at(i), bias, tokens);
            tKeys = tokens.keys();
            tValues = tokens.values();
        }
    }
    return newFormula;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> FormulaIdTranslator::nameToIdTokenMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects)
{
    QHash<QString, QString> nameToIdToken;
    nameToIdToken.reserve(gObjects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = gObjects.constBegin();
    while (i != gObjects.constEnd())
    {
        nameToIdToken.insert(i.value()->name(), idToken(i.key()));
        ++i;
    }
    return nameToIdToken;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> FormulaIdTranslator::idTokenToNameMap(const QHash<quint32, QSharedPointer<VGObject>> &gObjects)
{
    QHash<QString, QString> idTokenToName;
    idTokenToName.reserve(gObjects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = gObjects.constBegin();
    while (i != gObjects.constEnd())
    {
        idTokenToName.insert(idToken(i.key()), i.value()->name());
        ++i;
    }
    return idTokenToName;
}

//---------------------------------------------------------------------------------------------------------------------
namespace
{
    void replaceToken(QString &formula, const QString &newToken, int position, const QString &token, int &bias)
    {
        formula.replace(position, token.length(), newToken);
        bias = token.length() - newToken.length();
    }

    //-----------------------------------------------------------------------------------------------------------------
    void correctPositions(int position, int bias, QMap<int, QString> &tokens)
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
}

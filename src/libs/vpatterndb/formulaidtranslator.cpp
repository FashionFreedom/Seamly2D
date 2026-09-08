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
#include <QRegularExpression>
#include <QScopedPointer>

#include "../qmuparser/qmutokenparser.h"

using namespace FormulaIdTranslator;

namespace
{
    void replaceToken(QString &formula, const QString &new_token, int position, const QString &token, int &bias);
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
bool FormulaIdTranslator::isIdToken(const QString &token)
{
    static const QRegularExpression idTokenRx(
        QStringLiteral("^(Line_|AngleLine_|Radius\\d+|Angle1|Angle2|C1Length|C2Length)?"
                       "id\\d+(_Seg_\\d+)?$"));
    return idTokenRx.match(token).hasMatch();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief formulaNamesToIds replaces every known display-name token in formula with its stored id token.
 * @param formula formula as typed/displayed to the user, referencing object/derived-variable names
 * @param nameToIdToken lookup from a whole display token (e.g. "A1" or "Line_A1_A2") to its stored id token
 * @return formula with each known token replaced; unknown tokens are left untouched
 */
QString FormulaIdTranslator::formulaNamesToIds(const QString &formula, const QHash<QString, QString> &name_to_id_token)
{
    if (formula.isEmpty())
    {
        return formula;
    }

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false));
    QMap<int, QString> tokens = cal->GetTokens();
    delete cal.take();

    QList<int> t_keys = tokens.keys();
    QList<QString> t_values = tokens.values();

    QString new_formula = formula;
    for (int i = 0; i < t_values.size(); ++i)
    {
        if (not name_to_id_token.contains(t_values.at(i)))
        {
            continue;
        }

        int bias = 0;
        replaceToken(new_formula, name_to_id_token.value(t_values.at(i)), t_keys.at(i), t_values.at(i), bias);
        if (bias != 0)
        {
            correctPositions(t_keys.at(i), bias, tokens);
            t_keys = tokens.keys();
            t_values = tokens.values();
        }
    }
    return new_formula;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief formulaIdsToNames replaces every known stored id token in formula with its current display name.
 * @param formula formula as stored internally, referencing id tokens
 * @param idTokenToName lookup from a whole stored id token (e.g. "id42" or "Line_id42_id17") to its current
 * display name
 * @return formula with each known token replaced; unknown tokens are left untouched
 */
QString FormulaIdTranslator::formulaIdsToNames(const QString &formula, const QHash<QString, QString> &id_token_to_name)
{
    if (formula.isEmpty())
    {
        return formula;
    }

    QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(formula, false, false));
    QMap<int, QString> tokens = cal->GetTokens();
    delete cal.take();

    QList<int> t_keys = tokens.keys();
    QList<QString> t_values = tokens.values();

    QString new_formula = formula;
    for (int i = 0; i < t_values.size(); ++i)
    {
        if (not id_token_to_name.contains(t_values.at(i)))
        {
            continue;
        }

        int bias = 0;
        replaceToken(new_formula, id_token_to_name.value(t_values.at(i)), t_keys.at(i), t_values.at(i), bias);
        if (bias != 0)
        {
            correctPositions(t_keys.at(i), bias, tokens);
            t_keys = tokens.keys();
            t_values = tokens.values();
        }
    }
    return new_formula;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> FormulaIdTranslator::nameToIdTokenMap(const QHash<quint32, QSharedPointer<VGObject>> &g_objects)
{
    QHash<QString, QString> name_to_id_token;
    name_to_id_token.reserve(g_objects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = g_objects.constBegin();
    while (i != g_objects.constEnd())
    {
        name_to_id_token.insert(i.value()->name(), idToken(i.key()));
        ++i;
    }
    return name_to_id_token;
}

//---------------------------------------------------------------------------------------------------------------------
QHash<QString, QString> FormulaIdTranslator::idTokenToNameMap(const QHash<quint32, QSharedPointer<VGObject>> &g_objects)
{
    QHash<QString, QString> id_token_to_name;
    id_token_to_name.reserve(g_objects.size());

    QHash<quint32, QSharedPointer<VGObject>>::const_iterator i = g_objects.constBegin();
    while (i != g_objects.constEnd())
    {
        id_token_to_name.insert(idToken(i.key()), i.value()->name());
        ++i;
    }
    return id_token_to_name;
}

//---------------------------------------------------------------------------------------------------------------------
namespace
{
    void replaceToken(QString &formula, const QString &new_token, int position, const QString &token, int &bias)
    {
        formula.replace(position, token.length(), new_token);
        bias = token.length() - new_token.length();
    }

    //-----------------------------------------------------------------------------------------------------------------
    void correctPositions(int position, int bias, QMap<int, QString> &tokens)
    {
        if (bias == 0)
        {
            return;
        }

        QMap<int, QString> new_tokens;
        QMap<int, QString>::const_iterator i = tokens.constBegin();
        while (i != tokens.constEnd())
        {
            if (i.key() <= position)
            {
                new_tokens.insert(i.key(), i.value());
            }
            else
            {
                new_tokens.insert(i.key() - bias, i.value());
            }
            ++i;
        }
        tokens = new_tokens;
    }
}

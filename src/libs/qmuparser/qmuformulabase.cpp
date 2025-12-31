//-------------------------------------------------------------------------------------------------
//  @file   qmuformulabase.cpp
//  @author Douglas S Caskey
//  @date   20 Jul, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2025 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//   Copyright (C) 2015 Roman Telezhynskyi
//
//   Permission is hereby granted, free of charge, to any person obtaining a copy of this
//   software and associated documentation files (the "Software"), to deal in the Software
//   without restriction, including without limitation the rights to use, copy, modify,
//   merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//   permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in all copies or
//   substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//   DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------------------------------

#include "qmuformulabase.h"

#include <QChar>
#include <QList>
#include <QLocale>
#include <QMap>
#include <QString>
#include <QStringList>

namespace qmu
{

//---------------------------------------------------------------------------------------------------------------------
QmuFormulaBase::QmuFormulaBase()
    :QmuParser()
{
}

//---------------------------------------------------------------------------------------------------------------------
QmuFormulaBase::~QmuFormulaBase()
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief InitCharSets init character set for parser.
 *
 * QMuParser require setting character set for legal characters. Because we try make our expresion language independent
 * we set all posible unique characters from all alphabets.
 *
 */
void QmuFormulaBase::InitCharSets()
{
    //String with all unique symbols for supported alphabets.
    //See script alphabets.py for generation and more information.
    //Note. MSVC doesn't support normal string concatenation for long string. That's why we use QStringList in this
    //case.
    const QStringList symbols = QStringList()
        << "ցЀĆЈVӧĎАғΕĖӅИқΝĞơРңњΥĦШҫ̆έجگĮаҳѕεشԶиһνԾрÃυلՆӝшËύՎїPÓՖXӛӟŞãզhëҔծpóӞնxßվāŁЃֆĉЋΊCŬđҐГ"
        << "ΒęҘЛΚŘġҠУGاհЫΪŪدԱҰгβطԹõлκKՁÀуςهՉÈыvیՑÐSOřӘћաőcӐթèkàѓżűðsķչøӥӔĀփӣІĈΏЎґĐΗЖҙĘȚΟОҡĠآΧ"
        << "ЦتίЮұİزηжԸغẽοоÁՀقχцÉՈيюÑՐђӋіәťӆўáŠĺѐfөըnñŰӤӨӹոľЁրăЉŭċΌБӸēłΔҖЙŤěΜӜDСձģΤӰЩīņحάҮбưԳص"
        << "δHйԻŇμӴсՃمτƠщՋόєLQŹՓŕÖYśÞaգĽæiŽիӓîqճöyջþĂօЄӦΉĊЌΑĒДҗјΙȘĚМΡéĵĢФūӚΩبĪЬүќαذԲдҷιظԺмρՂф"
        << "ÇωوՊьÏՒTŚĻJբdçժlïӪղtպӫAւąЇΆčŃЏΎĕӯЗΖEțŮĝПΞأĥĹЧΦثÆӳЯήIسŲԵзζԽпξكՅÄчφNMՍӌяώӢӲՕÔWÎŝÜџё"
        << "źեägխoӒյôwĶBžսüЂĄև̈ЊČƏљΓВҕĔӮΛКĜΣТҥĤکΫЪƯخγвŅԴϊضλкԼĴσтÅՄنϋъÍՌRӕՔZÝŜbåդﻩjíլļrӵմӱzýռپê"
        << "ЅքćچΈЍďΐҒЕůėژșΘØҚНğńءΠFҢХħΨҪЭųįҶرΰҲеԷňعθҺнԿفπÂхՇψÊэšՏÒUəÚѝŻşҤӑâeէŐımկòuշÕúտŔ°";

    INIT_LOCALE_VARIABLES(m_locale);
    Q_UNUSED(expUpper)
    Q_UNUSED(expLower)
    Q_UNUSED(decimalPoint)
    Q_UNUSED(groupSeparator)

    // Defining identifier character sets
    const QString nameChars = QString() + char0 + char1 + char2 + char3 + char4 + char5 + char6 + char7 + char8 +
            char9 + QLatin1String("_@#'") + symbols.join("");
    DefineNameChars(nameChars);

    const QString oprtChars = symbols.join("") + positiveSign + negativeSign + QLatin1String("*^/?<>=!$%&|~'_");
    DefineOprtChars(oprtChars);

    const QString infixOprtChars = QString() + positiveSign + negativeSign + QLatin1String("*^/?<>=!$%&|~'_");
    DefineInfixOprtChars(infixOprtChars);
}

//---------------------------------------------------------------------------------------------------------------------
// Factory function for creating new parser variables
// This could as well be a function performing database queries.
qreal *QmuFormulaBase::AddVariable(const QString &a_szName, void *a_pUserData)
{
    Q_UNUSED(a_szName)
    Q_UNUSED(a_pUserData)

    static qreal value = 0;
    return &value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSepForTr set separators for translation expression.
 * @param fromUser true if expression come from user (from dialog).
 */
void QmuFormulaBase::SetSepForTr(bool osSeparator, bool fromUser)
{
    if (fromUser)
    {
        const QLocale loc = QLocale();
        setLocale(loc);
        SetArgSep(';');
        if (osSeparator)
        {
            setDecimalPoint(localeDecimalPoint(loc));
            setThousandsSeparator(localeGroupSeparator(loc));
            return;
        }
    }

    SetSepForEval();//Same separators (internal) as for eval.
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSepForEval set separators for eval. Each expression eval in internal (C) locale.
 */
void QmuFormulaBase::SetSepForEval()
{
    SetArgSep(';');
    setThousandsSeparator(',');
    setDecimalPoint('.');
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveAll remove token from token list.
 *
 * Standard Qt class QMap doesn't have method RemoveAll.
 * Example: remove "-" from tokens list if exist. If don't do that unary minus operation will broken.
 *
 * @param map map with tokens
 * @param val token that need delete
 */
void QmuFormulaBase::RemoveAll(QMap<int, QString> &map, const QString &val)
{
    const QList<int> listKeys = map.keys(val);//Take all keys that contain token.
    if (listKeys.size() > 0)
    {
        for (int i = 0; i < listKeys.size(); ++i)
        {
            map.remove(listKeys.at(i));
        }
    }
}

}// namespace qmu

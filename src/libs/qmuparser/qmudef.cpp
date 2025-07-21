//-------------------------------------------------------------------------------------------------
//  @file   qmudef.cpp
//  @author Douglas S Caskey
//  @date   14 Jul, 2025
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
//  Copyright (C) 2015 Roman Telezhynskyi <dismine(at)gmail.com>
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this
//  software and associated documentation files (the "Software"), to deal in the Software
//  without restriction, including without limitation the rights to use, copy, modify,
//  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------------------------------

#include "qmudef.h"

#include <QLocale>
#include <QSet>

enum State
{
    Init     = 0,
    Sign     = 1,
    Thousand = 2,
    Mantissa = 3,
    Dot	     = 4,
    Abscissa = 5,
    ExpMark	 = 6,
    ExpSign	 = 7,
    Exponent = 8,
    Done	 = 9
};

enum InputToken
{
    InputSign       = 1,
    InputThousand   = 2,
    InputDigit      = 3,
    InputDot        = 4,
    InputExp        = 5
};

static const QChar QmuEOF = QChar(static_cast<ushort>(0xffff)); //guaranteed not to be a character.

//---------------------------------------------------------------------------------------------------------------------
static QChar GetChar(const QString &formula, int &index)
{
    if (index >= formula.size())
    {
        return QmuEOF;
    }

    return formula.at(index++);
}

//---------------------------------------------------------------------------------------------------------------------
static QChar EatWhiteSpace(const QString &formula, int &index)
{
    QChar c;
    do
    {
        c = GetChar(formula, index);
    }
    while ( c != QmuEOF && c.isSpace() );

    return c;
}

//---------------------------------------------------------------------------------------------------------------------
static int CheckChar(QChar &c, const QLocale &locale, const QChar &decimal, const QChar &thousand)
{
    INIT_LOCALE_VARIABLES(locale);
    Q_UNUSED(decimalPoint)
    Q_UNUSED(groupSeparator)

    if (c == positiveSign)
    {
        c = '+';
        return InputToken::InputSign;
    }
    else if (c == negativeSign)
    {
        c = '-';
        return InputToken::InputSign;
    }
    else if (c == char0)
    {
        c = '0';
        return InputToken::InputDigit;
    }
    else if (c == char1)
    {
        c = '1';
        return InputToken::InputDigit;
    }
    else if (c == char2)
    {
        c = '2';
        return InputToken::InputDigit;
    }
    else if (c == char3)
    {
        c = '3';
        return InputToken::InputDigit;
    }
    else if (c == char4)
    {
        c = '4';
        return InputToken::InputDigit;
    }
    else if (c == char5)
    {
        c = '5';
        return InputToken::InputDigit;
    }
    else if (c == char6)
    {
        c = '6';
        return InputToken::InputDigit;
    }
    else if (c == char7)
    {
        c = '7';
        return InputToken::InputDigit;
    }
    else if (c == char8)
    {
        c = '8';
        return InputToken::InputDigit;
    }
    else if (c == char9)
    {
        c = '9';
        return InputToken::InputDigit;
    }
    else if (c == decimal)
    {
        return InputToken::InputDot;
    }
    else if (c == thousand)
    {
        return InputToken::InputThousand;
    }
    else if (c == expLower)
    {
        c = 'e';
        return InputToken::InputExp;
    }
    else if (c == expUpper)
    {
        c = 'E';
        return InputToken::InputExp;
    }
    else
    {
        return 0;
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
int ReadVal(const QString &formula, qreal &val, const QLocale &locale, const QChar &decimal, const QChar &thousand)
{
    // Must not be equal
    if (decimal == thousand || formula.isEmpty())
    {
        val = 0;
        return -1;
    }

    INIT_LOCALE_VARIABLES(locale);
    Q_UNUSED(decimalPoint)
    Q_UNUSED(groupSeparator)

    QSet<QChar> reserved;
    reserved << positiveSign
             << negativeSign
             << char0
             << char1
             << char2
             << char3
             << char4
             << char5
             << char6
             << char7
             << char8
             << char9
             << expUpper
             << expLower;

    if (reserved.contains(decimal) || reserved.contains(thousand))
    {
        val = 0;
        return -1;
    }

    // row - current state, column - new state
    static uchar table[9][6] =
    {
        /*    None	     InputSign     InputThousand      InputDigit     InputDot      InputExp */
        { 0,	       State::Sign,    0,               State::Mantissa, State::Dot,  0,	         }, // Init
        { 0,	       0,	           0,               State::Mantissa, State::Dot,  0,	         }, // Sign
        { 0,	       0,	           0,               State::Mantissa, 0,	          0,	         }, // Thousand
        { State::Done, State::Done,    State::Thousand, State::Mantissa, State::Dot,  State::ExpMark,}, // Mantissa
        { 0,	       0,	           0,               State::Abscissa, 0,	          0,	         }, // Dot
        { State::Done, State::Done,    0,               State::Abscissa, 0,           State::ExpMark,}, // Abscissa
        { 0,	       State::ExpSign, 0,               State::Exponent, 0,	          0,	         }, // ExpMark
        { 0,	       0,	           0,               State::Exponent, 0,	          0,	         }, // ExpSign
        { State::Done, 0,              0,               State::Exponent, 0,           State::Done	 }  // Exponent
    };

    int state = State::Init;	// parse state
    QString buf;

    int index = 0; // start position
    QChar c = EatWhiteSpace(formula, index);

    while ( true )
    {
        const int input = CheckChar(c, locale, decimal, thousand);// input token

        state = table[state][input];

        if (state == 0)
        {
            val = 0;
            return -1;
        }
        else if (state == Done)
        {
            // Convert to C locale
            QLocale cLocale(QLocale::C);
            const QChar cDecimal = localeDecimalPoint(cLocale);
            const QChar cThousand = localeGroupSeparator(cLocale);
            if (locale != cLocale && (cDecimal != decimal || cThousand != thousand))
            {
                if (decimal == cThousand)
                {// Handle reverse to C locale case: thousand '.', decimal ','
                    //const QChar tmpThousand = '@';
                    const QChar tmpThousand = QLatin1Char('@');
                    buf.replace(thousand, tmpThousand);
                    buf.replace(decimal, cDecimal);
                    buf.replace(tmpThousand, cThousand);
                }
                else
                {
                    buf.replace(thousand, cThousand);
                    buf.replace(decimal, cDecimal);
                }
            }

            bool ok = false;
            const double d = cLocale.toDouble(buf, &ok);
            if (ok)
            {
                val = d;
                return buf.size();
            }
            else
            {
                val = 0;
                return -1;
            }
        }

        buf.append(c);
        c = GetChar(formula, index);
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
QString NameRegExp()
{
    static QString regex;

    if (regex.isEmpty())
    {
        const QList<QLocale> allLocales = QLocale::matchingLocales(QLocale::AnyLanguage,
                                                                   QLocale::AnyScript,
                                                                   QLocale::AnyCountry);

        QString positiveSigns;
        QString negativeSigns;
        QString decimalPoints;
        QString groupSeparators;
        for(int i = 0; i < allLocales.size(); ++i)
        {
            if (!positiveSigns.contains(localePositiveSign(allLocales.at(i))))
            {
                positiveSigns.append(localePositiveSign(allLocales.at(i)));
            }

            if (!negativeSigns.contains(localeNegativeSign(allLocales.at(i))))
            {
                negativeSigns.append(localeNegativeSign(allLocales.at(i)));
            }

            if (!decimalPoints.contains(localeDecimalPoint(allLocales.at(i))))
            {
                decimalPoints.append(localeDecimalPoint(allLocales.at(i)));
            }

            if (!groupSeparators.contains(localeGroupSeparator(allLocales.at(i))))
            {
                groupSeparators.append(localeGroupSeparator(allLocales.at(i)));
            }
        }

        negativeSigns.replace('-', "\\-");
        groupSeparators.remove('\'');

        //Same regexp in pattern.xsd schema file. Don't forget to synchronize.
        // \p{Nd} - \p{Decimal_Digit_Number}
        // \p{Zs} - \p{Space_Separator}
        regex = QString("^([^\\p{Nd}\\p{Zs}*/&|!<>^\\()%1%2%3%4=?:;'\"]){1,1}"
                        "([^\\p{Zs}*/&|!<>^\\()%1%2%3%4=?:;\"]){0,}$")
                .arg(negativeSigns).arg(positiveSigns).arg(decimalPoints).arg(groupSeparators);
    }

    return regex;
}

//---------------------------------------------------------------------------------------------------------------------
bool isLocaleSupported(const QLocale &locale)
{
    return locale.name() != QStringLiteral("tok_001") &&
           locale.positiveSign().size() == 1 &&
           locale.negativeSign().size() == 1 &&
           locale.toString(0).size() == 1 &&
           locale.toString(1).size() == 1 &&
           locale.toString(2).size() == 1 &&
           locale.toString(3).size() == 1 &&
           locale.toString(4).size() == 1 &&
           locale.toString(5).size() == 1 &&
           locale.toString(6).size() == 1 &&
           locale.toString(7).size() == 1 &&
           locale.toString(8).size() == 1 &&
           locale.toString(9).size() == 1 &&
           locale.exponential().size() == 1 &&
           locale.decimalPoint().size() == 1 &&
           locale.groupSeparator().size() == 1;
}

//----------------------------------------------------------------------------------------------
QChar localePositiveSign(const QLocale &locale)
{
    const QString string = locale.positiveSign();
    QChar result = (string.size() == 1) ? string.front() : QLocale::c().positiveSign().front();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeNegativeSign(const QLocale &locale)
{
    const QString string = locale.negativeSign();
    QChar result = (string.size() == 1) ? string.front() : QLocale::c().negativeSign().front();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeCharacter(const QLocale &locale, int number)
{
    const QString string = locale.toString(number);
    char character = number + '0';
    QChar result = (string.size() == 1) ? string.front() : character;
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeExpUpper(const QLocale &locale)
{
    const QString string = locale.exponential();
    QChar result = (string.size() == 1) ? string.front().toUpper() : QLocale::c().exponential().front().toUpper();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeExpLower(const QLocale &locale)
{
    const QString string = locale.exponential();
    QChar result = (string.size() == 1) ? string.front().toLower() : QLocale::c().exponential().front().toLower();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeDecimalPoint(const QLocale &locale)
{
    const QString string = locale.decimalPoint();
    QChar result = (string.size() == 1) ? string.front() : QLocale::c().decimalPoint().front();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
QChar localeGroupSeparator(const QLocale &locale)
{
    const QString string = locale.groupSeparator();
    QChar result = (string.size() == 1) ? string.front() : QLocale::c().groupSeparator().front();
    return result;
}

//-------------------------------------------------------------------------------------------------
//  @file   qmudef.h
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

#ifndef QMUDEF_H
#define QMUDEF_H

#include <qcompilerdetection.h>
#include <QtGlobal>
#include <QChar>
#include <QString>
#include <QLocale>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")

#ifdef Q_CC_MSVC
    #include <ciso646>
#endif /* Q_CC_MSVC */

#define INIT_LOCALE_VARIABLES(locale)                           \
const QChar positiveSign   = localePositiveSign((locale));      \
const QChar negativeSign   = localeNegativeSign((locale));      \
const QChar char0          = localeCharacter((locale), 0);      \
const QChar char1          = localeCharacter((locale), 1);      \
const QChar char2          = localeCharacter((locale), 2);      \
const QChar char3          = localeCharacter((locale), 3);      \
const QChar char4          = localeCharacter((locale), 4);      \
const QChar char5          = localeCharacter((locale), 5);      \
const QChar char6          = localeCharacter((locale), 6);      \
const QChar char7          = localeCharacter((locale), 7);      \
const QChar char8          = localeCharacter((locale), 8);      \
const QChar char9          = localeCharacter((locale), 9);      \
const QChar expUpper       = localeExpUpper((locale));          \
const QChar expLower       = localeExpLower((locale));          \
const QChar decimalPoint   = localeDecimalPoint((locale));      \
const QChar groupSeparator = localeGroupSeparator((locale));    \

QString NameRegExp();
bool    isLocaleSupported(const QLocale &locale);
QChar   localePositiveSign(const QLocale &locale);
QChar   localeNegativeSign(const QLocale &locale);
QChar   localeCharacter(const QLocale &locale, int number);
QChar   localeExpUpper(const QLocale &locale);
QChar   localeExpLower(const QLocale &locale);
QChar   localeDecimalPoint(const QLocale &locale);
QChar   localeGroupSeparator(const QLocale &locale);

QT_WARNING_POP

Q_REQUIRED_RESULT static inline bool QmuFuzzyComparePossibleNulls(double p1, double p2);
static inline bool QmuFuzzyComparePossibleNulls(double p1, double p2)
{
    if(qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }
    else if(qFuzzyIsNull(p2))
    {
        return false;
    }
    else
    {
        return qFuzzyCompare(p1, p2);
    }
}

int ReadVal(const QString &formula, qreal &val, const QLocale &locale, const QChar &decimal, const QChar &thousand);

#endif // QMUDEF_H

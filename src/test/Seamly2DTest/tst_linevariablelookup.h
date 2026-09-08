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

#ifndef TST_LINEVARIABLELOOKUP_H
#define TST_LINEVARIABLELOOKUP_H

#include <QObject>

class TST_LineVariableLookup : public QObject
{
    Q_OBJECT
public:
    explicit TST_LineVariableLookup(QObject *parent = nullptr);

private slots:
    void TestFindLineLengthReturnsMatchingLine();
    void TestFindLineAngleReturnsMatchingLine();
    void TestFindLineLengthReturnsNullForUnknownId();
    void TestFindLineAngleReturnsNullForUnknownId();

private:
    Q_DISABLE_COPY(TST_LineVariableLookup)
};

#endif // TST_LINEVARIABLELOOKUP_H

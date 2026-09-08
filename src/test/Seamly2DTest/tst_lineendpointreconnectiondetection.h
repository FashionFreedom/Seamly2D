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

#ifndef TST_LINEENDPOINTRECONNECTIONDETECTION_H
#define TST_LINEENDPOINTRECONNECTIONDETECTION_H

#include <QObject>

class TST_LineEndpointReconnectionDetection : public QObject
{
    Q_OBJECT
public:
    explicit TST_LineEndpointReconnectionDetection(QObject *parent = nullptr);

private slots:
    void TestReconnectedLineLeavesDependentFormulaUnresolved();
    void TestReconnectedLineFormulaFailsEvaluation();

private:
    Q_DISABLE_COPY(TST_LineEndpointReconnectionDetection)
};

#endif // TST_LINEENDPOINTRECONNECTIONDETECTION_H

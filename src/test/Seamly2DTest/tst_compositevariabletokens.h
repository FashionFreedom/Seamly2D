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

#ifndef TST_COMPOSITEVARIABLETOKENS_H
#define TST_COMPOSITEVARIABLETOKENS_H

#include <QObject>

class TST_CompositeVariableTokens : public QObject
{
    Q_OBJECT
public:
    explicit TST_CompositeVariableTokens(QObject *parent = nullptr);

private slots:
    void TestLineLengthMapping();
    void TestLineLengthSurvivesRename();
    void TestUnknownVariableTypeLeftOut();
    void TestLineAngleMapping();
    void TestArcRadiusMapping();
    void TestEllipticalArcRadiusMapping();
    void TestCurveAngleMapping();
    void TestCurveCLengthMapping();
    void TestPlainCurveLengthLeftOutOfCompositeMap();
    void TestCurveLengthSegmentMapping();
    void TestCurveAngleSegmentMapping();
    void TestCurveCLengthSegmentMapping();

private:
    Q_DISABLE_COPY(TST_CompositeVariableTokens)
};

#endif // TST_COMPOSITEVARIABLETOKENS_H

//-----------------------------------------------------------------------------
//  @file   tst_vmeasurements.h
//  @author Douglas S Caskey
//  @date   14 Jul, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  @file   tst_vmeasurements.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   16 Oct, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#ifndef TST_VMEASUREMENTS_H
#define TST_VMEASUREMENTS_H

#include <QObject>

class TST_Measurements : public QObject
{
    Q_OBJECT
public:
    explicit TST_Measurements(QObject *parent = nullptr);

private slots:
    void CreateEmptyMultisizeFile();
    void CreateEmptyIndividualFile();

    void ValidPMCodesMultisizeFile();
    void ValidPMCodesIndividualFile();

    void SizeRangeDefaults();
    void setSizeRange();
    void setSizeRangeRejectsInvalidRange();
    void HeightRangeDefaults();
    void setHeightRange();
    void setHeightRangeRejectsInvalidRange();
    void activeSizesFiltered();
    void activeHeightsFiltered();
    void SizeRangeRoundTripThroughFile();
    void MultisizeFileWithRangeValidatesSchema();

    void testSizeAliasesRoundTrip();
    void testSizeDisplayAliasFallback();
};

#endif // TST_VMEASUREMENTS_H

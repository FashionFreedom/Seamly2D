/******************************************************************************
 *   @file   multi_size_converter.h
 **  @author Douglas S Caskey
 **  @date   25 Jan, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2024 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
 *************************************************************************/

 /************************************************************************
 **
 **  @file   vmeasurementconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef MULTISIZE_CONVERTER_H
#define MULTISIZE_CONVERTER_H

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QString>
#include <QtGlobal>

#include "abstract_m_converter.h"
#include "abstract_converter.h"

class QDomElement;

class MultiSizeConverter : public AbstractMConverter
{
    Q_DECLARE_TR_FUNCTIONS(MultiSizeConverter)
public:
    explicit MultiSizeConverter(const QString &fileName);
    virtual ~MultiSizeConverter() Q_DECL_EQ_DEFAULT;

    static const QString MeasurementMaxVerStr;
    static const QString CurrentSchema;
// GCC 4.6 doesn't allow constexpr and const together
#if !defined(__INTEL_COMPILER) && !defined(__clang__) && defined(__GNUC__) && (__GNUC__ * 100 + __GNUC_MINOR__) <= 406
    static Q_DECL_CONSTEXPR int MeasurementMinVer = CONVERTER_VERSION_CHECK(0, 3, 0);
    static Q_DECL_CONSTEXPR int MeasurementMaxVer = CONVERTER_VERSION_CHECK(0, 4, 5);
#else
    static Q_DECL_CONSTEXPR const int MeasurementMinVer = CONVERTER_VERSION_CHECK(0, 3, 0);
    static Q_DECL_CONSTEXPR const int MeasurementMaxVer = CONVERTER_VERSION_CHECK(0, 4, 5);
#endif

protected:
    virtual int     minVer() const Q_DECL_OVERRIDE;
    virtual int     maxVer() const Q_DECL_OVERRIDE;

    virtual QString minVerStr() const Q_DECL_OVERRIDE;
    virtual QString maxVerStr() const Q_DECL_OVERRIDE;

    virtual QString getSchema(int ver) const Q_DECL_OVERRIDE;
    virtual void    applyPatches() Q_DECL_OVERRIDE;
    virtual void    downgradeToCurrentMaxVersion() Q_DECL_OVERRIDE;
    virtual bool    isReadOnly() const Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(MultiSizeConverter)
    static const QString MeasurementMinVerStr;

    void addNewTagsForVer0_4_0();
    void removeTagsForVer0_4_0();
    void convertMeasurementsToV0_4_0();
    QDomElement addMeasurementsVer0_4_0(const QString &name, qreal value, qreal sizeIncrease, qreal heightIncrease);
    void convertPmSystemToVer0_4_1();
    void convertMeasurementsToV0_4_2();

    void convertToVer0_4_0();
    void convertToVer0_4_1();
    void convertToVer0_4_2();
    void convertToVer0_4_3();
    void convertToVer0_4_4();
    void convertToVer0_4_5();
};

//---------------------------------------------------------------------------------------------------------------------
inline int MultiSizeConverter::minVer() const
{
    return MeasurementMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline int MultiSizeConverter::maxVer() const
{
    return MeasurementMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString MultiSizeConverter::minVerStr() const
{
    return MeasurementMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString MultiSizeConverter::maxVerStr() const
{
    return MeasurementMaxVerStr;
}

#endif // MULTISIZE_CONVERTER_H

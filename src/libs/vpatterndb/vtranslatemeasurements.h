/******************************************************************************
*   @file   vtranslatemeasurements.h
**  @author DSCaskey <dscaskey@gmail.com>
**  @date  5 Jun, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
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
 **  @file   vtranslatemeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 8, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VTRANSLATEMEASUREMENTS_H
#define VTRANSLATEMEASUREMENTS_H

#include <QMap>
#include <QString>
#include <QtGlobal>

#include "../qmuparser/qmutranslation.h"

/**
 * @class VTranslateMeasurements
 * @brief Manages the translation of measurement-related strings.
 * 
 * The VTranslateMeasurements class provides methods for translating and manipulating measurement-related strings.
 * It includes functionality for converting measurements from user input, retrieving measurement names, numbers, formulas,
 * GUI text, and descriptions, as well as re-translating measurements.
 * 
 * @details
 * - The class stores various translations and mappings related to measurements.
 * - It includes methods for initializing measurement groups and individual measurements.
 * 
 * @public
 * - VTranslateMeasurements();
 * - virtual ~VTranslateMeasurements();
 * - bool MeasurementsFromUser(QString &newFormula, int position, const QString &token, int &bias) const;
 * - QString MToUser(const QString &measurement) const;
 * - QString MNumber(const QString &measurement) const;
 * - QString MFormula(const QString &measurement) const;
 * - QString guiText(const QString &measurement) const;
 * - QString Description(const QString &measurement) const;
 * - virtual void Retranslate();
 * 
 * @protected
 * - QMap<QString, qmu::QmuTranslation> m_measurements;
 * 
 * @private
 * - Q_DISABLE_COPY(VTranslateMeasurements)
 * - QMap<QString, qmu::QmuTranslation> m_guiTexts;
 * - QMap<QString, qmu::QmuTranslation> m_descriptions;
 * - QMap<QString, QString> m_numbers;
 * - QMap<QString, QString> m_formulas;
 * - void InitGroupA(); // Direct Height
 * - void InitGroupB(); // Direct Width
 * - void InitGroupC(); // Indentation
 * - void InitGroupD(); // Circumference and Arc
 * - void InitGroupE(); // Vertical
 * - void InitGroupF(); // Horizontal
 * - void InitGroupG(); // Bust
 * - void InitGroupH(); // Balance
 * - void InitGroupI(); // Arm
 * - void InitGroupJ(); // Leg
 * - void InitGroupK(); // Crotch and Rise
 * - void InitGroupL(); // Hand
 * - void InitGroupM(); // Foot
 * - void InitGroupN(); // Head
 * - void InitGroupO(); // Men & Tailoring
 * - void InitGroupP(); // Historical & Specialty
 * - void InitGroupQ(); // Patternmaking measurements
 * - void InitMeasurements();
 * - void InitMeasurement(const QString &name, const qmu::QmuTranslation &m, const qmu::QmuTranslation &g,
 *                        const qmu::QmuTranslation &d, const QString &number, const QString &formula = QString());
 */
class VTranslateMeasurements
{
public:
    VTranslateMeasurements();
    virtual ~VTranslateMeasurements();

    bool MeasurementsFromUser(QString &newFormula, int position, const QString &token, int &bias) const;

    QString MToUser(const QString &measurement) const;
    QString MNumber(const QString &measurement) const;
    QString MFormula(const QString &measurement) const;
    QString guiText(const QString &measurement) const;
    QString Description(const QString &measurement) const;

    virtual void Retranslate();

protected:
    QMap<QString, qmu::QmuTranslation> m_measurements;

private:
    Q_DISABLE_COPY(VTranslateMeasurements)
    QMap<QString, qmu::QmuTranslation> m_guiTexts;
    QMap<QString, qmu::QmuTranslation> m_descriptions;
    QMap<QString, QString>             m_numbers;
    QMap<QString, QString>             m_formulas;

    void InitGroupA(); // Direct Height
    void InitGroupB(); // Direct Width
    void InitGroupC(); // Indentation
    void InitGroupD(); // Circumference and Arc
    void InitGroupE(); // Vertical
    void InitGroupF(); // Horizontal
    void InitGroupG(); // Bust
    void InitGroupH(); // Balance
    void InitGroupI(); // Arm
    void InitGroupJ(); // Leg
    void InitGroupK(); // Crotch and Rise
    void InitGroupL(); // Hand
    void InitGroupM(); // Foot
    void InitGroupN(); // Head
    void InitGroupO(); // Men & Tailoring
    void InitGroupP(); // Historical & Specialty
    void InitGroupQ(); // Patternmaking measurements

    void InitMeasurements();

    void InitMeasurement(const QString &name, const qmu::QmuTranslation &m, const qmu::QmuTranslation &g,
                         const qmu::QmuTranslation &d, const QString &number, const QString &formula = QString());
};

#endif // VTRANSLATEMEASUREMENTS_H

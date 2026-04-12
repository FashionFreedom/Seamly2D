//-----------------------------------------------------------------------------
//  @file   measurement_variable.cpp
//  @author Douglas S Caskey
//  @date   16 Jul, 2023
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
//  @file   vstandardtablecell.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013 Valentina project
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

#include "measurement_variable.h"

#include <QMap>
#include <QMessageLogger>
#include <QtDebug>
#include <algorithm>

#include "../ifc/ifcdef.h"
#include "vvariable.h"
#include "measurement_variable_p.h"

#ifdef Q_COMPILER_RVALUE_REFS
MeasurementVariable &MeasurementVariable::operator=(MeasurementVariable &&m) noexcept
{ Swap(m); return *this; }
#endif

void MeasurementVariable::Swap(MeasurementVariable &m) noexcept
{ VVariable::Swap(m); std::swap(d, m.d); }

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MeasurementVariable create measurement for multisize table
 * @param name measurement's name
 * @param base value in base size and height
 * @param ksize increment in sizes
 * @param kheight increment in heights
 * @param gui_text shor tooltip for user
 * @param description measurement full description
 * @param tagName measurement's tag name in file
 */
MeasurementVariable::MeasurementVariable(quint32 index, const QString &name, qreal baseSize, qreal baseHeight, const qreal &base,
                           const qreal &ksize, const qreal &kheight, const QString &gui_text,
                           const QString &description, const QString &tagName)
    : VVariable(name, description)
    , d(new MeasurementVariableData(index, gui_text, tagName, baseSize, baseHeight, base, ksize, kheight))
{
    SetType(VarType::Measurement);
    VInternalVariable::SetValue(d->base);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MeasurementVariable create measurement for individual table
 * @param name measurement's name
 * @param base value in base size and height
 * @param gui_text shor tooltip for user
 * @param description measurement full description
 * @param tagName measurement's tag name in file
 */
MeasurementVariable::MeasurementVariable(VContainer *data, quint32 index, const QString &name, const qreal &base,
                           const QString &formula, bool ok, const QString &gui_text, const QString &description,
                           const QString &tagName)
    : VVariable(name, description)
    , d(new MeasurementVariableData(data, index, formula, ok, gui_text, tagName, base))
{
    SetType(VarType::Measurement);
    VInternalVariable::SetValue(base);
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementVariable::MeasurementVariable(const MeasurementVariable &m)
    : VVariable(m)
    , d(m.d)
{}

//---------------------------------------------------------------------------------------------------------------------
MeasurementVariable &MeasurementVariable::operator=(const MeasurementVariable &m)
{
    if ( &m == this )
    {
        return *this;
    }
    VVariable::operator=(m);
    d = m.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementVariable::~MeasurementVariable()
{}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::patternHeightsList(QMap<GHeights, bool> heights, Unit patternUnit)
{
    QStringList list;

    QMap<GHeights, bool>::const_iterator i = heights.constBegin();
    while (i != heights.constEnd())
    {
        if (i.value() && i.key() != GHeights::ALL)
        {
            ListValue(list, static_cast<int>(i.key()), patternUnit);
        }
        ++i;
    }

    if (list.isEmpty())
    {
        list = MeasurementVariable::allHeightsList(patternUnit);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::patternSizesList(QMap<GSizes, bool> sizes, Unit patternUnit)
{
    QStringList list;

    QMap<GSizes, bool>::const_iterator i = sizes.constBegin();
    while (i != sizes.constEnd())
    {
        if (i.value() && i.key() != GSizes::ALL)
        {
            ListValue(list, static_cast<int>(i.key()), patternUnit);
        }
        ++i;
    }

    if (list.isEmpty())
    {
        list = MeasurementVariable::allSizesList(patternUnit);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::allHeightsList(Unit patternUnit)
{
    QStringList list;

    for (int i = static_cast<int>(GHeights::H50); i<= static_cast<int>(GHeights::H200); i = i+heightStep)
    {
        ListValue(list, i, patternUnit);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::allSizesList(Unit patternUnit)
{
    QStringList list;

    for (int i = static_cast<int>(GSizes::S22); i<= static_cast<int>(GSizes::S72); i = i+sizeStep)
    {
       ListValue(list, i, patternUnit);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::IsGradationSizeValid(const QString &size)
{
    if (not size.isEmpty())
    {
        const QStringList sizes = MeasurementVariable::allSizesList(Unit::Cm);
        return sizes.contains(size);
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::IsGradationHeightValid(const QString &height)
{
    if (not height.isEmpty())
    {
        const QStringList heights = MeasurementVariable::allHeightsList(Unit::Cm);
        return heights.contains(height);
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal MeasurementVariable::CalcValue() const
{
    if (d->currentUnit == nullptr || d->currentSize == nullptr || d->currentHeight == nullptr)
    {
        return VInternalVariable::GetValue();
    }

    if (*d->currentUnit == Unit::Inch)
    {
        qWarning("Gradation doesn't support inches");
        return 0;
    }

    const qreal sizeStep = UnitConvertor(2.0, Unit::Cm, *d->currentUnit);
    const qreal heightStep = UnitConvertor(6.0, Unit::Cm, *d->currentUnit);

    qreal sizeOffset = 0;
    qreal heightOffset = 0;

    if (d->sizeBreaks.isEmpty())
    {
        // Original linear formula for size dimension
        const qreal k_size = (*d->currentSize - d->baseSize) / sizeStep;
        sizeOffset = k_size * d->ksize;
    }
    else
    {
        sizeOffset = calcPiecewiseOffset(*d->currentSize, d->baseSize, sizeStep,
                                         d->ksize, d->sizeBreaks);
    }

    if (d->heightBreaks.isEmpty())
    {
        // Original linear formula for height dimension
        const qreal k_height = (*d->currentHeight - d->baseHeight) / heightStep;
        heightOffset = k_height * d->kheight;
    }
    else
    {
        heightOffset = calcPiecewiseOffset(*d->currentHeight, d->baseHeight, heightStep,
                                           d->kheight, d->heightBreaks);
    }

    return d->base + sizeOffset + heightOffset;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::ListValue(QStringList &list, qreal value, Unit patternUnit)
{
    const qreal val = UnitConvertor(value, Unit::Cm, patternUnit);
    const QString strVal = QString("%1").arg(val);
    list.append(strVal);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getGuiText measurement name for tooltip
 * @return measurement name
 */
QString MeasurementVariable::getGuiText() const
{
    return d->gui_text;
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementVariable::TagName() const
{
    return d->_tagName;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::setTagName(const QString &tagName)
{
    d->_tagName = tagName;
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementVariable::GetFormula() const
{
    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::isCustom() const
{
    return GetName().indexOf(CustomMSign) == 0;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementVariable::Index() const
{
    return static_cast<int>(d->index);
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::IsFormulaOk() const
{
    return d->formulaOk;
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::isNotUsed() const
{
    return qFuzzyIsNull(d->base) && qFuzzyIsNull(d->ksize) && qFuzzyIsNull(d->kheight)
           && d->sizeBreaks.isEmpty() && d->heightBreaks.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
qreal MeasurementVariable::GetValue() const
{
    return CalcValue();
}

//---------------------------------------------------------------------------------------------------------------------
qreal *MeasurementVariable::GetValue()
{
    VInternalVariable::SetValue(CalcValue());
    return VInternalVariable::GetValue();
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *MeasurementVariable::GetData()
{
    return &d->data;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::setSize(qreal *size)
{
    d->currentSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::setHeight(qreal *height)
{
    d->currentHeight = height;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::SetUnit(const Unit *unit)
{
    d->currentUnit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBase return value in base size and height
 * @return value
 */
qreal MeasurementVariable::GetBase() const
{
    return d->base;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::SetBase(const qreal &value)
{
    d->base = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKsize return increment in sizes
 * @return increment
 */
qreal MeasurementVariable::GetKsize() const
{
    return d->ksize;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void MeasurementVariable::SetKsize(const qreal &value)
{
    d->ksize = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKheight return increment in heights
 * @return increment
 */
qreal MeasurementVariable::GetKheight() const
{
    return d->kheight;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void MeasurementVariable::SetKheight(const qreal &value)
{
    d->kheight = value;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<GradeBreak> MeasurementVariable::GetSizeBreaks() const
{
    return d->sizeBreaks;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::SetSizeBreaks(const QVector<GradeBreak> &breaks)
{
    d->sizeBreaks = breaks;
    std::sort(d->sizeBreaks.begin(), d->sizeBreaks.end());
}

//---------------------------------------------------------------------------------------------------------------------
QVector<GradeBreak> MeasurementVariable::GetHeightBreaks() const
{
    return d->heightBreaks;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementVariable::SetHeightBreaks(const QVector<GradeBreak> &breaks)
{
    d->heightBreaks = breaks;
    std::sort(d->heightBreaks.begin(), d->heightBreaks.end());
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementVariable::HasGradeBreaks() const
{
    return !d->sizeBreaks.isEmpty() || !d->heightBreaks.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief calcPiecewiseOffset walks from base to current in step-sized increments,
 * looking up the applicable increment from the breaks vector at each step.
 * @param current current size/height value
 * @param base base size/height value
 * @param step step size (e.g. 2.0 for sizes, 6.0 for heights) in current unit
 * @param defaultIncrement the uniform ksize/kheight value
 * @param breaks sorted vector of grade breaks
 * @return total offset from base
 */
qreal MeasurementVariable::calcPiecewiseOffset(qreal current, qreal base, qreal step,
                                                qreal defaultIncrement,
                                                const QVector<GradeBreak> &breaks)
{
    if (qFuzzyCompare(current, base) || qFuzzyIsNull(step))
    {
        return 0;
    }

    const bool gradingUp = (current > base);
    const int numSteps = qRound(qAbs(current - base) / step);
    qreal totalOffset = 0;

    for (int i = 0; i < numSteps; ++i)
    {
        // The size/height at the start of this step
        const qreal pos = gradingUp ? (base + i * step) : (base - i * step);

        // Find the applicable increment for this position
        qreal increment = defaultIncrement;
        for (int b = breaks.size() - 1; b >= 0; --b)
        {
            if (pos >= breaks[b].threshold - 0.001) // small epsilon for floating point
            {
                increment = breaks[b].increment;
                break;
            }
        }

        if (gradingUp)
        {
            totalOffset += increment;
        }
        else
        {
            totalOffset -= increment;
        }
    }

    return totalOffset;
}

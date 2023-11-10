/******************************************************************************
 *   @file   measurement_variable.cpp
 **  @author Douglas S Caskey
 **  @date   16 Jul, 2023
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
 **  @file   vstandardtablecell.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "measurement_variable.h"

#include <QMap>
#include <QMessageLogger>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "vvariable.h"
#include "measurement_variable_p.h"

#ifdef Q_COMPILER_RVALUE_REFS
MeasurementVariable &MeasurementVariable::operator=(MeasurementVariable &&m) Q_DECL_NOTHROW
{ Swap(m); return *this; }
#endif

void MeasurementVariable::Swap(MeasurementVariable &m) Q_DECL_NOTHROW
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
QStringList MeasurementVariable::ListHeights(QMap<GHeights, bool> heights, Unit patternUnit)
{
    QStringList list;
    if (patternUnit == Unit::Inch)
    {
        qWarning() << "Multisize table doesn't support inches.";
        return list;
    }

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
        list = MeasurementVariable::WholeListHeights(patternUnit);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::ListSizes(QMap<GSizes, bool> sizes, Unit patternUnit)
{
    QStringList list;
    if (patternUnit == Unit::Inch)
    {
        qWarning() << "Multisize table doesn't support inches.";
        return list;
    }

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
        list = MeasurementVariable::WholeListSizes(patternUnit);
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::WholeListHeights(Unit patternUnit)
{
    QStringList list;
    if (patternUnit == Unit::Inch)
    {
        qWarning() << "Multisize table doesn't support inches.";
        return list;
    }

    for (int i = static_cast<int>(GHeights::H50); i<= static_cast<int>(GHeights::H200); i = i+heightStep)
    {
        ListValue(list, i, patternUnit);
    }

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementVariable::WholeListSizes(Unit patternUnit)
{
    QStringList list;
    if (patternUnit == Unit::Inch)
    {
        qWarning() << "Multisize table doesn't support inches.";
        return list;
    }

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
        const QStringList sizes = MeasurementVariable::WholeListSizes(Unit::Cm);
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
        const QStringList heights = MeasurementVariable::WholeListHeights(Unit::Cm);
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

    const qreal sizeIncrement = UnitConvertor(2.0, Unit::Cm, *d->currentUnit);
    const qreal heightIncrement = UnitConvertor(6.0, Unit::Cm, *d->currentUnit);

    // Formula for calculation gradation
    const qreal k_size    = ( *d->currentSize - d->baseSize ) / sizeIncrement;
    const qreal k_height  = ( *d->currentHeight - d->baseHeight ) / heightIncrement;
    return d->base + k_size * d->ksize + k_height * d->kheight;
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
    return qFuzzyIsNull(d->base) && qFuzzyIsNull(d->ksize) && qFuzzyIsNull(d->kheight);
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

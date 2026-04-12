//-----------------------------------------------------------------------------
//  @file   measurements.cpp
//  @author Douglas S Caskey
//  @date   25 Jan, 2024
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
//  @file   vmeasurements.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
// @date   14 Jul, 2015
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

#include "measurements.h"

#include <qnumeric.h>
#include <QDate>
#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QLatin1Char>
#include <QMessageLogger>
#include <QScopedPointer>
#include <QSet>
#include <QString>
#include <QtDebug>

#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/xml/individual_size_converter.h"
#include "../ifc/xml/multi_size_converter.h"
#include "../ifc/ifcdef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../vpatterndb/variables/grade_break.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/measurements_def.h"
#include "../vpatterndb/pmsystems.h"
#include "../vmisc/projectversion.h"

const QString MeasurementDoc::TagVST              = QStringLiteral("vst");
const QString MeasurementDoc::TagVIT              = QStringLiteral("vit");
const QString MeasurementDoc::TagSMMS             = QStringLiteral("smms");
const QString MeasurementDoc::TagSMIS             = QStringLiteral("smis");
const QString MeasurementDoc::TagBodyMeasurements = QStringLiteral("body-measurements");
const QString MeasurementDoc::TagNotes            = QStringLiteral("notes");
const QString MeasurementDoc::TagSize             = QStringLiteral("size");
const QString MeasurementDoc::TagHeight           = QStringLiteral("height");
const QString MeasurementDoc::TagPersonal         = QStringLiteral("personal");
const QString MeasurementDoc::TagFamilyName       = QStringLiteral("family-name");
const QString MeasurementDoc::TagGivenName        = QStringLiteral("given-name");
const QString MeasurementDoc::TagBirthDate        = QStringLiteral("birth-date");
const QString MeasurementDoc::TagGender           = QStringLiteral("gender");
const QString MeasurementDoc::TagPMSystem         = QStringLiteral("pm_system");
const QString MeasurementDoc::TagEmail            = QStringLiteral("email");
const QString MeasurementDoc::TagReadOnly         = QStringLiteral("read-only");
const QString MeasurementDoc::TagMeasurement      = QStringLiteral("m");

const QString MeasurementDoc::AttrBase            = QStringLiteral("base");

const QString MeasurementDoc::AttrSizeIncrease    = QStringLiteral("size_increase");
const QString MeasurementDoc::AttrHeightIncrease  = QStringLiteral("height_increase");
const QString MeasurementDoc::AttrDescription     = QStringLiteral("description");
const QString MeasurementDoc::AttrName            = QStringLiteral("name");
const QString MeasurementDoc::AttrFullName        = QStringLiteral("full_name");

const QString MeasurementDoc::AttrMin              = QStringLiteral("min");
const QString MeasurementDoc::AttrMax              = QStringLiteral("max");
const QString MeasurementDoc::AttrStep             = QStringLiteral("step");
const QString MeasurementDoc::TagSizeBreaks        = QStringLiteral("sizeRules");
const QString MeasurementDoc::TagHeightBreaks      = QStringLiteral("heightRules");
const QString MeasurementDoc::TagBreak             = QStringLiteral("break");
const QString MeasurementDoc::AttrThreshold        = QStringLiteral("threshold");
const QString MeasurementDoc::AttrIncrement        = QStringLiteral("increment");
const QString MeasurementDoc::AttrAliasValue       = QStringLiteral("value");
const QString MeasurementDoc::AttrAliasText        = QStringLiteral("alias");
const QString MeasurementDoc::AttrColor             = QStringLiteral("color");

const QString MeasurementDoc::TagSizes             = QStringLiteral("sizes");
const QString MeasurementDoc::TagHeights           = QStringLiteral("heights");
const QString MeasurementDoc::TagAlias             = QStringLiteral("alias");

const QString MeasurementDoc::GenderMale          = QStringLiteral("male");
const QString MeasurementDoc::GenderFemale        = QStringLiteral("female");
const QString MeasurementDoc::GenderUnknown       = QStringLiteral("unknown");

const QString defBirthDate                        = QStringLiteral("1800-01-01");

namespace
{
//---------------------------------------------------------------------------------------------------------------------
QString FileComment()
{
    return QString("Measurements created with SeamlyMe v%1 (https://seamly.io/).").arg(APP_VERSION_STR);
}
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDoc::MeasurementDoc(VContainer *data)
    :VDomDocument()
    , data(data)
    , type(MeasurementsType::Unknown)
    , m_currentSize(nullptr)
    , m_currentHeight(nullptr)
{
    SCASSERT(data != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDoc::MeasurementDoc(Unit unit, VContainer *data)
    :VDomDocument()
    , data(data)
    , type(MeasurementsType::Individual)
    , m_currentSize(nullptr)
    , m_currentHeight(nullptr)
{
    SCASSERT(data != nullptr)

    CreateEmptyIndividualFile(unit);
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDoc::MeasurementDoc(Unit unit, int baseSize, int baseHeight, VContainer *data)
    :VDomDocument()
    , data(data)
    , type(MeasurementsType::Multisize)
    , m_currentSize(nullptr)
    , m_currentHeight(nullptr)
{
    SCASSERT(data != nullptr)

    CreateEmptyMultisizeFile(unit, baseSize, baseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setXMLContent(const QString &fileName)
{
    VDomDocument::setXMLContent(fileName);
    type = ReadType();
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementDoc::SaveDocument(const QString &fileName, QString &error)
{
    // Update comment with Seamly2D version
    QDomNode commentNode = documentElement().firstChild();
    if (commentNode.isComment())
    {
        QDomComment comment = commentNode.toComment();
        comment.setData(FileComment());
    }

    return VDomDocument::SaveDocument(fileName, error);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::addEmpty(const QString &name, const QString &formula)
{
    const QDomElement element = MakeEmpty(name, formula);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::AddEmptyAfter(const QString &after, const QString &name, const QString &formula)
{
    const QDomElement element = MakeEmpty(name, formula);
    const QDomElement sibling = FindM(after);

    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);

    if (sibling.isNull())
    {
        list.at(0).appendChild(element);
    }
    else
    {
        list.at(0).insertAfter(element, sibling);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::Remove(const QString &name)
{
    const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
    list.at(0).removeChild(FindM(name));
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::MoveTop(const QString &name)
{
    const QDomElement node = FindM(name);
    if (!node.isNull())
    {
        const QDomNodeList mList = elementsByTagName(TagMeasurement);
        if (mList.size() >= 2)
        {
            const QDomNode top = mList.at(0);
            if (!top.isNull())
            {
                const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
                list.at(0).insertBefore(node, top);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::MoveUp(const QString &name)
{
    const QDomElement node = FindM(name);
    if (!node.isNull())
    {
        const QDomElement prSibling = node.previousSiblingElement(TagMeasurement);
        if (!prSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertBefore(node, prSibling);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::MoveDown(const QString &name)
{
    const QDomElement node = FindM(name);
    if (!node.isNull())
    {
        const QDomElement nextSibling = node.nextSiblingElement(TagMeasurement);
        if (!nextSibling.isNull())
        {
            const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
            list.at(0).insertAfter(node, nextSibling);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::MoveBottom(const QString &name)
{
    const QDomElement node = FindM(name);
    if (!node.isNull())
    {
        const QDomNodeList mList = elementsByTagName(TagMeasurement);
        if (mList.size() >= 2)
        {
            const QDomNode bottom = mList.at(mList.size()-1);
            if (!bottom.isNull())
            {
                const QDomNodeList list = elementsByTagName(TagBodyMeasurements);
                list.at(0).insertAfter(node, bottom);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::readMeasurements() const
{
    // For conversion values we must first calculate all data in measurement file's unit.
    // That's why we need two containers: one for converted values, second for real data.

    // Container for values in measurement file's unit
    QScopedPointer<VContainer> tempData(new VContainer(data->getTranslateVariables(), data->GetPatternUnit()));

    const QDomNodeList list = elementsByTagName(TagMeasurement);
    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement dom = list.at(i).toElement();

        const QString name = GetParametrString(dom, AttrName);

        QString description;
        try
        {
            description = GetParametrString(dom, AttrDescription);
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        QString fullName;
        try
        {
            fullName = GetParametrString(dom, AttrFullName);
        }
        catch (VExceptionEmptyParameter &error)
        {
            Q_UNUSED(error)
        }

        QSharedPointer<MeasurementVariable> meash;
        QSharedPointer<MeasurementVariable> tempMeash;
        if (type == MeasurementsType::Multisize)
        {
            qreal base = GetParametrDouble(dom, AttrBase, "0");
            qreal ksize = GetParametrDouble(dom, AttrSizeIncrease, "0");
            qreal kheight = GetParametrDouble(dom, AttrHeightIncrease, "0");

            // Read optional grade breaks
            QVector<GradeBreak> sizeBreaks;
            QVector<GradeBreak> heightBreaks;

            const QDomElement sizeBreaksElem = dom.firstChildElement(TagSizeBreaks);
            if (!sizeBreaksElem.isNull())
            {
                QDomElement brk = sizeBreaksElem.firstChildElement(TagBreak);
                while (!brk.isNull())
                {
                    const qreal threshold = GetParametrDouble(brk, AttrThreshold, "0");
                    const qreal increment = GetParametrDouble(brk, AttrIncrement, "0");
                    sizeBreaks.append(GradeBreak(threshold, increment));
                    brk = brk.nextSiblingElement(TagBreak);
                }
            }

            const QDomElement heightBreaksElem = dom.firstChildElement(TagHeightBreaks);
            if (!heightBreaksElem.isNull())
            {
                QDomElement brk = heightBreaksElem.firstChildElement(TagBreak);
                while (!brk.isNull())
                {
                    const qreal threshold = GetParametrDouble(brk, AttrThreshold, "0");
                    const qreal increment = GetParametrDouble(brk, AttrIncrement, "0");
                    heightBreaks.append(GradeBreak(threshold, increment));
                    brk = brk.nextSiblingElement(TagBreak);
                }
            }

            tempMeash = QSharedPointer<MeasurementVariable>(new MeasurementVariable(static_cast<quint32>(i), name,
                                                            getBaseSize(), getBaseHeight(), base, ksize, kheight));
            tempMeash->SetSizeBreaks(sizeBreaks);
            tempMeash->SetHeightBreaks(heightBreaks);
            tempMeash->setSize(m_currentSize);
            tempMeash->setHeight(m_currentHeight);
            tempMeash->SetUnit(data->GetPatternUnit());

            base = UnitConvertor(base, measurementUnits(), *data->GetPatternUnit());
            ksize = UnitConvertor(ksize, measurementUnits(), *data->GetPatternUnit());
            kheight = UnitConvertor(kheight, measurementUnits(), *data->GetPatternUnit());

            // Convert grade break thresholds and increments to pattern units
            QVector<GradeBreak> convSizeBreaks;
            for (const GradeBreak &gb : sizeBreaks)
            {
                convSizeBreaks.append(GradeBreak(
                    UnitConvertor(gb.threshold, measurementUnits(), *data->GetPatternUnit()),
                    UnitConvertor(gb.increment, measurementUnits(), *data->GetPatternUnit())));
            }

            QVector<GradeBreak> convHeightBreaks;
            for (const GradeBreak &gb : heightBreaks)
            {
                convHeightBreaks.append(GradeBreak(
                    UnitConvertor(gb.threshold, measurementUnits(), *data->GetPatternUnit()),
                    UnitConvertor(gb.increment, measurementUnits(), *data->GetPatternUnit())));
            }

            const qreal baseSize = UnitConvertor(getBaseSize(), measurementUnits(), *data->GetPatternUnit());
            const qreal baseHeight = UnitConvertor(getBaseHeight(), measurementUnits(), *data->GetPatternUnit());

            meash = QSharedPointer<MeasurementVariable>(new MeasurementVariable(static_cast<quint32>(i), name, baseSize, baseHeight,
                                                                  base, ksize, kheight, fullName, description));
            meash->SetSizeBreaks(convSizeBreaks);
            meash->SetHeightBreaks(convHeightBreaks);
            meash->setSize(m_currentSize);
            meash->setHeight(m_currentHeight);
            meash->SetUnit(data->GetPatternUnit());
        }
        else
        {
            const QString formula = GetParametrString(dom, AttrAliasValue, "0");
            bool ok = false;
            qreal value = EvalFormula(tempData.data(), formula, &ok);

            tempMeash = QSharedPointer<MeasurementVariable>(new MeasurementVariable(tempData.data(), static_cast<quint32>(i), name,
                                                                      value, formula, ok));

            value = UnitConvertor(value, measurementUnits(), *data->GetPatternUnit());
            meash = QSharedPointer<MeasurementVariable>(new MeasurementVariable(data, static_cast<quint32>(i), name, value, formula,
                                                                  ok, fullName, description));
        }
        tempData->AddVariable(name, tempMeash);
        data->AddVariable(name, meash);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::ClearForExport()
{
    const QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            if (qmu::QmuTokenParser::IsSingle(domElement.attribute(AttrAliasValue)))
            {
                SetAttribute(domElement, AttrAliasValue, QString("0"));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType MeasurementDoc::Type() const
{
    return type;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::getBaseSize() const
{
    if (type == MeasurementsType::Multisize)
    {
        return static_cast<int>(UniqueTagAttr(TagSize, AttrBase, 50));
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::getBaseHeight() const
{
    if (type == MeasurementsType::Multisize)
    {
        return static_cast<int>(UniqueTagAttr(TagHeight, AttrBase, 176));
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::minSize() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagSize);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrMin))
            {
                return element.attribute(AttrMin).toInt();
            }
        }
        return static_cast<int>(GSizes::S22);
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::maxSize() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagSize);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrMax))
            {
                return element.attribute(AttrMax).toInt();
            }
        }
        return static_cast<int>(GSizes::S72);
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::sizeStep() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagSize);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrStep))
            {
                return element.attribute(AttrStep).toInt();
            }
        }
    }
    return 2;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setSizeRange(int min, int max, int step)
{
    if (isReadOnly())
    {
        return;
    }

    const QDomNodeList nodeList = this->elementsByTagName(TagSize);
    if (nodeList.isEmpty())
    {
        return;
    }

    QDomElement element = nodeList.at(0).toElement();
    if (element.isNull())
    {
        return;
    }

    const int base = getBaseSize();
    if (min > base || max < base)
    {
        return;
    }

    SetAttribute(element, AttrMin, min);
    SetAttribute(element, AttrMax, max);
    SetAttribute(element, AttrStep, step);
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::minHeight() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagHeight);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrMin))
            {
                return element.attribute(AttrMin).toInt();
            }
        }
        return static_cast<int>(GHeights::H50);
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::maxHeight() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagHeight);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrMax))
            {
                return element.attribute(AttrMax).toInt();
            }
        }
        return static_cast<int>(GHeights::H200);
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
int MeasurementDoc::heightStep() const
{
    if (type == MeasurementsType::Multisize)
    {
        const QDomNodeList nodeList = this->elementsByTagName(TagHeight);
        if (!nodeList.isEmpty())
        {
            const QDomElement element = nodeList.at(0).toElement();
            if (element.hasAttribute(AttrStep))
            {
                return element.attribute(AttrStep).toInt();
            }
        }
    }
    return 6;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setHeightRange(int min, int max, int step)
{
    if (isReadOnly())
    {
        return;
    }

    const QDomNodeList nodeList = this->elementsByTagName(TagHeight);
    if (nodeList.isEmpty())
    {
        return;
    }

    QDomElement element = nodeList.at(0).toElement();
    if (element.isNull())
    {
        return;
    }

    const int base = getBaseHeight();
    if (min > base || max < base)
    {
        return;
    }

    SetAttribute(element, AttrMin, min);
    SetAttribute(element, AttrMax, max);
    SetAttribute(element, AttrStep, step);
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> MeasurementDoc::sizeAliases() const
{
    QMap<int, QString> map;
    const QDomNodeList nodeList = this->elementsByTagName(TagSizes);
    if (nodeList.isEmpty())
    {
        return map;
    }

    const QDomElement container = nodeList.at(0).toElement();
    QDomElement element = container.firstChildElement(TagSize);
    while (!element.isNull())
    {
        const int value = element.attribute(AttrAliasValue).toInt();
        const QString text = element.attribute(AttrAliasText);
        if (value > 0 && !text.isEmpty())
        {
            map.insert(value, text);
        }
        element = element.nextSiblingElement(TagSize);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setSizes(const QMap<int, QString> &aliases, const QMap<int, QString> &colors)
{
    if (isReadOnly())
    {
        return;
    }

    // Remove existing size-aliases element if any
    const QDomNodeList nodeList = this->elementsByTagName(TagSizes);
    if (!nodeList.isEmpty())
    {
        documentElement().removeChild(nodeList.at(0));
    }

    if (!aliases.isEmpty())
    {
        QDomElement container = createElement(TagSizes);
        for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it)
        {
            QString color = colors.value(it.key(), QString("#000000"));
            QDomElement element = createElement(TagSize);
            SetAttribute(element, AttrAliasValue, it.key());
            SetAttribute(element, AttrAliasText, it.value());
            SetAttribute(element, AttrColor, color);
            container.appendChild(element);
        }

        // Insert before body-measurements
        const QDomNodeList bmList = elementsByTagName(TagBodyMeasurements);
        if (!bmList.isEmpty())
        {
            documentElement().insertBefore(container, bmList.at(0));
        }
        else
        {
            documentElement().appendChild(container);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> MeasurementDoc::heightAliases() const
{
    QMap<int, QString> map;
    const QDomNodeList nodeList = this->elementsByTagName(TagHeights);
    if (nodeList.isEmpty())
    {
        return map;
    }

    const QDomElement container = nodeList.at(0).toElement();
    QDomElement element = container.firstChildElement(TagHeight);
    while (!element.isNull())
    {
        const int value = element.attribute(AttrAliasValue).toInt();
        const QString text = element.attribute(AttrAliasText);
        if (value > 0 && !text.isEmpty())
        {
            map.insert(value, text);
        }
        element = element.nextSiblingElement(TagHeight);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setHeightAliases(const QMap<int, QString> &aliases)
{
    if (isReadOnly())
    {
        return;
    }

    // Remove existing height-aliases element if any
    const QDomNodeList nodeList = this->elementsByTagName(TagHeights);
    if (!nodeList.isEmpty())
    {
        documentElement().removeChild(nodeList.at(0));
    }

    if (!aliases.isEmpty())
    {
        QDomElement container = createElement(TagHeights);
        for (auto it = aliases.constBegin(); it != aliases.constEnd(); ++it)
        {
            QDomElement element = createElement(TagHeight);
            SetAttribute(element, AttrAliasValue, it.key());
            SetAttribute(element, AttrAliasText, it.value());
            container.appendChild(element);
        }

        // Insert before body-measurements
        const QDomNodeList bmList = elementsByTagName(TagBodyMeasurements);
        if (!bmList.isEmpty())
        {
            documentElement().insertBefore(container, bmList.at(0));
        }
        else
        {
            documentElement().appendChild(container);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> MeasurementDoc::sizeColors() const
{
    QMap<int, QString> map;
    const QDomNodeList nodeList = this->elementsByTagName(TagSizes);
    if (nodeList.isEmpty())
    {
        return map;
    }

    const QDomElement container = nodeList.at(0).toElement();
    QDomElement element = container.firstChildElement(TagSize);
    while (!element.isNull())
    {
        const int value = element.attribute(AttrAliasValue).toInt();
        const QString color =  GetParametrString(element, AttrColor, "#000000");
        if (value > 0 && !color.isEmpty())
        {
            map.insert(value, color);
        }
        element = element.nextSiblingElement(TagSize);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::sizeDisplayAlias(int valueInCm, const QMap<int, QString> &aliases)
{
    return aliases.value(valueInCm, QString::number(valueInCm));
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDoc::activeSizes(Unit patternUnit) const
{
    const QStringList all = MeasurementVariable::allSizesList(patternUnit);
    const int min = minSize();
    const int max = maxSize();

    if (min == static_cast<int>(GSizes::S22) && max == static_cast<int>(GSizes::S72))
    {
        return all;
    }

    QStringList filtered;
    for (const QString &string : all)
    {
        const int val = string.toInt();
        if (val >= min && val <= max)
        {
            filtered.append(string);
        }
    }
    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDoc::activeHeights(Unit patternUnit) const
{
    const QStringList all = MeasurementVariable::allHeightsList(patternUnit);
    const int min = minHeight();
    const int max = maxHeight();

    if (min == static_cast<int>(GHeights::H50) && max == static_cast<int>(GHeights::H200))
    {
        return all;
    }

    QStringList filtered;
    for (const QString &string : all)
    {
        const int val = string.toInt();
        if (val >= min && val <= max)
        {
            filtered.append(string);
        }
    }
    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::Notes() const
{
    return UniqueTagText(TagNotes, "");
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetNotes(const QString &text)
{
    if (!isReadOnly())
    {
        setTagText(TagNotes, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::FamilyName() const
{
    return UniqueTagText(TagFamilyName, "");
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetFamilyName(const QString &text)
{
    if (!isReadOnly())
    {
        setTagText(TagFamilyName, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::GivenName() const
{
    return UniqueTagText(TagGivenName, "");
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetGivenName(const QString &text)
{
    if (!isReadOnly())
    {
        setTagText(TagGivenName, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDate MeasurementDoc::BirthDate() const
{
    return QDate::fromString(UniqueTagText(TagBirthDate, defBirthDate), "yyyy-MM-dd");
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetBirthDate(const QDate &date)
{
    if (!isReadOnly())
    {
        setTagText(TagBirthDate, date.toString("yyyy-MM-dd"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
GenderType MeasurementDoc::Gender() const
{
    return StrToGender(UniqueTagText(TagGender, GenderUnknown));
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetGender(const GenderType &gender)
{
    if (!isReadOnly())
    {
        setTagText(TagGender, GenderToStr(gender));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::PMSystem() const
{
    return UniqueTagText(TagPMSystem, ClearPMCode(p998_S));
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetPMSystem(const QString &system)
{
    if (!isReadOnly())
    {
        setTagText(TagPMSystem, ClearPMCode(system));
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::Email() const
{
    return UniqueTagText(TagEmail, "");
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetEmail(const QString &text)
{
    if (!isReadOnly())
    {
        setTagText(TagEmail, text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementDoc::isReadOnly() const
{
    return UniqueTagText(TagReadOnly, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetReadOnly(bool ro)
{
    if (ro)
    {
        setTagText(TagReadOnly, trueStr);
    }
    else
    {
        setTagText(TagReadOnly, falseStr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setSize(qreal *size)
{
    m_currentSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::setHeight(qreal *height)
{
    m_currentHeight = height;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMValue(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrAliasValue, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMBaseValue(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrBase, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMSizeIncrease(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrSizeIncrease, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMHeightIncrease(const QString &name, double value)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrHeightIncrease, value);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMDescription(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrDescription, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMFullName(const QString &name, const QString &text)
{
    QDomElement node = FindM(name);
    if (!node.isNull())
    {
        SetAttribute(node, AttrFullName, text);
    }
    else
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMSizeBreaks(const QString &name, const QVector<GradeBreak> &breaks)
{
    QDomElement node = FindM(name);
    if (node.isNull())
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
        return;
    }

    // Remove existing size_breaks element if any
    QDomElement existing = node.firstChildElement(TagSizeBreaks);
    if (!existing.isNull())
    {
        node.removeChild(existing);
    }

    if (!breaks.isEmpty())
    {
        QDomElement sizeBreaksElem = createElement(TagSizeBreaks);
        for (const GradeBreak &gb : breaks)
        {
            QDomElement brkElem = createElement(TagBreak);
            SetAttribute(brkElem, AttrThreshold, gb.threshold);
            SetAttribute(brkElem, AttrIncrement, gb.increment);
            sizeBreaksElem.appendChild(brkElem);
        }
        node.insertBefore(sizeBreaksElem, node.firstChildElement());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::SetMHeightBreaks(const QString &name, const QVector<GradeBreak> &breaks)
{
    QDomElement node = FindM(name);
    if (node.isNull())
    {
        qWarning() << tr("Can't find measurement '%1'").arg(name);
        return;
    }

    // Remove existing height_breaks element if any
    QDomElement existing = node.firstChildElement(TagHeightBreaks);
    if (!existing.isNull())
    {
        node.removeChild(existing);
    }

    if (!breaks.isEmpty())
    {
        QDomElement heightBreaksElem = createElement(TagHeightBreaks);
        for (const GradeBreak &gb : breaks)
        {
            QDomElement brkElem = createElement(TagBreak);
            SetAttribute(brkElem, AttrThreshold, gb.threshold);
            SetAttribute(brkElem, AttrIncrement, gb.increment);
            heightBreaksElem.appendChild(brkElem);
        }
        // Insert after size_breaks if it exists, otherwise as first child
        QDomElement sizeBreaksElem = node.firstChildElement(TagSizeBreaks);
        if (!sizeBreaksElem.isNull())
        {
            node.insertAfter(heightBreaksElem, sizeBreaksElem);
        }
        else
        {
            node.insertBefore(heightBreaksElem, node.firstChildElement());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::GenderToStr(const GenderType &sex)
{
    switch (sex)
    {
        case GenderType::Male:
            return GenderMale;
        case GenderType::Female:
            return GenderFemale;
        case GenderType::Unknown:
        default:
            return GenderUnknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
GenderType MeasurementDoc::StrToGender(const QString &sex)
{
    const QStringList genders = QStringList() << GenderMale << GenderFemale << GenderUnknown;
    switch (genders.indexOf(sex))
    {
        case 0: // GenderMale
            return GenderType::Male;
        case 1: // GenderFemale
            return GenderType::Female;
        case 2: // GenderUnknown
        default:
            return GenderType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDoc::ListAll() const
{
    QStringList listNames;
    const QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            listNames.append(domElement.attribute(AttrName));
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDoc::listKnown() const
{
    QStringList listNames;
    const QStringList list = ListAll();
    for (int i=0; i < list.size(); ++i)
    {
        if (list.at(i).indexOf(CustomMSign) != 0)
        {
            listNames.append(list.at(i));
        }
    }

    return listNames;
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementDoc::eachKnownNameIsValid() const
{
    QStringList names = AllGroupNames();

    QSet<QString> set;
    foreach (const QString &var, names)
    {
        set.insert(var);
    }

    names = listKnown();
    foreach (const QString &var, names)
    {
        if (!set.contains(var))
        {
            return false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *MeasurementDoc::GetData() const
{
    return data;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::CreateEmptyMultisizeFile(Unit unit, int baseSize, int baseHeight)
{
    this->clear();
    QDomElement mElement = this->createElement(TagSMMS);

    mElement.appendChild(createComment(FileComment()));

    QDomElement version = createElement(TagVersion);
    const QDomText newNodeText = createTextNode(MultiSizeConverter::MeasurementMaxVerStr);
    version.appendChild(newNodeText);
    mElement.appendChild(version);

    QDomElement ro = createElement(TagReadOnly);
    const QDomText roNodeText = createTextNode("false");
    ro.appendChild(roNodeText);
    mElement.appendChild(ro);

    mElement.appendChild(createElement(TagNotes));

    QDomElement mUnit = createElement(TagUnit);
    const QDomText unitText = createTextNode(UnitsToStr(unit));
    mUnit.appendChild(unitText);
    mElement.appendChild(mUnit);

    QDomElement system = createElement(TagPMSystem);
    system.appendChild(createTextNode(ClearPMCode(p998_S)));
    mElement.appendChild(system);

    QDomElement size = createElement(TagSize);
    SetAttribute(size, AttrBase, QString().setNum(baseSize));
    mElement.appendChild(size);

    QDomElement height = createElement(TagHeight);
    SetAttribute(height, AttrBase, QString().setNum(baseHeight));
    mElement.appendChild(height);

    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDoc::CreateEmptyIndividualFile(Unit unit)
{
    this->clear();
    QDomElement mElement = this->createElement(TagSMIS);

    mElement.appendChild(createComment(FileComment()));

    QDomElement version = createElement(TagVersion);
    const QDomText newNodeText = createTextNode(IndividualSizeConverter::MeasurementMaxVerStr);
    version.appendChild(newNodeText);
    mElement.appendChild(version);

    QDomElement ro = createElement(TagReadOnly);
    const QDomText roNodeText = createTextNode("false");
    ro.appendChild(roNodeText);
    mElement.appendChild(ro);

    mElement.appendChild(createElement(TagNotes));

    QDomElement mUnit = createElement(TagUnit);
    mUnit.appendChild(createTextNode(UnitsToStr(unit)));
    mElement.appendChild(mUnit);

    QDomElement system = createElement(TagPMSystem);
    system.appendChild(createTextNode(ClearPMCode(p998_S)));
    mElement.appendChild(system);

    QDomElement personal = createElement(TagPersonal);
    personal.appendChild(createElement(TagFamilyName));
    personal.appendChild(createElement(TagGivenName));

    QDomElement date = createElement(TagBirthDate);
    date.appendChild(createTextNode(defBirthDate));
    personal.appendChild(date);

    QDomElement gender = createElement(TagGender);
    gender.appendChild(createTextNode(GenderToStr(GenderType::Unknown)));
    personal.appendChild(gender);

    personal.appendChild(createElement(TagEmail));
    mElement.appendChild(personal);

    mElement.appendChild(createElement(TagBodyMeasurements));

    this->appendChild(mElement);
    insertBefore(createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""), this->firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
qreal MeasurementDoc::UniqueTagAttr(const QString &tag, const QString &attr, qreal defValue) const
{
    const qreal defVal = UnitConvertor(defValue, Unit::Cm, measurementUnits());

    const QDomNodeList nodeList = this->elementsByTagName(tag);
    if (nodeList.isEmpty())
    {
        return defVal;
    }
    else
    {
        const QDomNode domNode = nodeList.at(0);
        if (domNode.isNull() == false && domNode.isElement())
        {
            const QDomElement domElement = domNode.toElement();
            if (domElement.isNull() == false)
            {
                return GetParametrDouble(domElement, attr, QString("%1").arg(defVal));
            }
        }
    }
    return defVal;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement MeasurementDoc::MakeEmpty(const QString &name, const QString &formula)
{
    QDomElement element = createElement(TagMeasurement);

    SetAttribute(element, AttrName, name);

    if (type == MeasurementsType::Multisize)
    {
        SetAttribute(element, AttrBase, QString("0"));
        SetAttribute(element, AttrSizeIncrease, QString("0"));
        SetAttribute(element, AttrHeightIncrease, QString("0"));
    }
    else
    {
        if (formula.isEmpty())
        {
            SetAttribute(element, AttrAliasValue, QString("0"));
        }
        else
        {
            SetAttribute(element, AttrAliasValue, formula);
        }
    }

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement MeasurementDoc::FindM(const QString &name) const
{
    if (name.isEmpty())
    {
        qWarning() << tr("The measurement name is empty!");
        return QDomElement();
    }

    QDomNodeList list = elementsByTagName(TagMeasurement);

    for (int i=0; i < list.size(); ++i)
    {
        const QDomElement domElement = list.at(i).toElement();
        if (domElement.isNull() == false)
        {
            const QString parameter = domElement.attribute(AttrName);
            if (parameter == name)
            {
                return domElement;
            }
        }
    }

    return QDomElement();
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType MeasurementDoc::ReadType() const
{
    QDomElement root = documentElement();
    if ((root.tagName() == TagVST) || (root.tagName() == TagSMMS))
    {
        return MeasurementsType::Multisize;
    }
    else if ((root.tagName() == TagVIT) || (root.tagName() == TagSMIS))
    {
        return MeasurementsType::Individual;
    }
    else
    {
        return MeasurementsType::Unknown;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal MeasurementDoc::EvalFormula(VContainer *data, const QString &formula, bool *ok) const
{
    if (formula.isEmpty())
    {
        *ok = true;
        return 0;
    }
    else
    {
        try
        {
            // Replace line return character with spaces for calc if exist
            QString f = formula;
            f.replace("\n", " ");
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(data->DataVariables(), f);

            (qIsInf(result) || qIsNaN(result)) ? *ok = false : *ok = true;
            return result;
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            *ok = false;
            return 0;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDoc::ClearPMCode(const QString &code) const
{
    QString clear = code;
    const int index = clear.indexOf(QLatin1Char('p'));
    if (index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}

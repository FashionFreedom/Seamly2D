/******************************************************************************
 *   @file   individual_size_converter.cpp
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
 **  @file   vvitconverter.cpp
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

#include "individual_size_converter.h"

#include <QDomNode>
#include <QDomNodeList>
#include <QDomText>
#include <QFile>
#include <QLatin1String>
#include <QList>
#include <QMap>
#include <QMultiMap>
#include <QString>

#include "../exception/vexception.h"
#include "../vmisc/def.h"
#include "abstract_m_converter.h"

/*
 * Version rules:
 * 1. Version have three parts "major.minor.patch";
 * 2. major part only for stable releases;
 * 3. minor - 10 or more patch changes, or one big change;
 * 4. patch - little change.
 */

const QString IndividualSizeConverter::MeasurementMinVerStr = QStringLiteral("0.2.0");
const QString IndividualSizeConverter::MeasurementMaxVerStr = QStringLiteral("0.3.4");
const QString IndividualSizeConverter::CurrentSchema        = QStringLiteral("://schema/individual_size_measurements/v0.3.4.xsd");

//IndividualSizeConverter::MeasurementMinVer; // <== DON'T FORGET TO UPDATE TOO!!!!
//IndividualSizeConverter::MeasurementMaxVer; // <== DON'T FORGET TO UPDATE TOO!!!!

static const QString strTagRead_Only = QStringLiteral("read-only");

//---------------------------------------------------------------------------------------------------------------------
IndividualSizeConverter::IndividualSizeConverter(const QString &fileName)
    :AbstractMConverter(fileName)
{
    ValidateInputFile(CurrentSchema);
}

//---------------------------------------------------------------------------------------------------------------------
QString IndividualSizeConverter::getSchema(int ver) const
{
    switch (ver)
    {
        case (0x000200):
            return QStringLiteral("://schema/individual_size_measurements/v0.2.0.xsd");
        case (0x000300):
            return QStringLiteral("://schema/individual_size_measurements/v0.3.0.xsd");
        case (0x000301):
            return QStringLiteral("://schema/individual_size_measurements/v0.3.1.xsd");
        case (0x000302):
            return QStringLiteral("://schema/individual_size_measurements/v0.3.2.xsd");
        case (0x000303):
            return QStringLiteral("://schema/individual_size_measurements/v0.3.3.xsd");
        case (0x000304):
            return CurrentSchema;
        default:
            InvalidVersion(ver);
            break;
    }
    return QString();//unreachable code
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::applyPatches()
{
    switch (m_ver)
    {
        case (0x000200):
            convertToVer0_3_0();
            ValidateXML(getSchema(0x000300), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000300):
            convertToVer0_3_1();
            ValidateXML(getSchema(0x000301), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000301):
            convertToVer0_3_2();
            ValidateXML(getSchema(0x000302), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000302):
            convertToVer0_3_3();
            ValidateXML(getSchema(0x000303), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000303):
            convertToVer0_3_4();
            ValidateXML(getSchema(0x000304), m_convertedFileName);
            V_FALLTHROUGH
        case (0x000304):
            break;
        default:
            InvalidVersion(m_ver);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::downgradeToCurrentMaxVersion()
{
    setVersion(MeasurementMaxVerStr);
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
bool IndividualSizeConverter::isReadOnly() const
{
    // Check if attribute read-only was not changed in file format
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMaxVer == CONVERTER_VERSION_CHECK(0, 3, 4),
                      "Check attribute read-only.");

    // Possibly in future attribute read-only will change position etc.
    // For now position is the same for all supported format versions.
    // But don't forget to keep all versions of attribute until we support that format versions

    return UniqueTagText(strTagRead_Only, falseStr) == trueStr;
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::addNewTagsForVer0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    QDomElement rootElement = this->documentElement();
    QDomNode refChild = rootElement.firstChildElement("version");

    QDomElement ro = createElement(QStringLiteral("read-only"));
    const QDomText roNodeText = createTextNode("false");
    ro.appendChild(roNodeText);
    refChild = rootElement.insertAfter(ro, refChild);

    refChild = rootElement.insertAfter(createElement(QStringLiteral("notes")), refChild);

    QDomElement unit = createElement("unit");
    unit.appendChild(createTextNode(convertUnitsToVer0_2_0()));
    rootElement.insertAfter(unit, refChild);
}

//---------------------------------------------------------------------------------------------------------------------
QString IndividualSizeConverter::convertUnitsToVer0_2_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    return UniqueTagText(QStringLiteral("unit"), QStringLiteral("cm"));
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertMeasurementsToV0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    const QString tagBM = QStringLiteral("body-measurements");

    QDomElement bm = createElement(tagBM);

    const QMultiMap<QString, QString> names = OldNamesToNewNames_InV0_3_0();
    const QList<QString> keys = names.uniqueKeys();
    for (int i = 0; i < keys.size(); ++i)
    {
        qreal resValue = 0;

        // This has the same effect as a .values(), just isn't as elegant
        const QList<QString> list = names.values( keys.at(i) );
        foreach(const QString &val, list )
        {
            const QDomNodeList nodeList = this->elementsByTagName(val);
            if (nodeList.isEmpty())
            {
                continue;
            }

            const qreal value = GetParametrDouble(nodeList.at(0).toElement(), QStringLiteral("value"), "0.0");

            if (not qFuzzyIsNull(value))
            {
                resValue = value;
            }
        }

        bm.appendChild(addMeasurementsVer0_3_0(keys.at(i), resValue));
    }

    QDomElement rootElement = this->documentElement();
    const QDomNodeList listBM = elementsByTagName(tagBM);
    rootElement.replaceChild(bm, listBM.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement IndividualSizeConverter::addMeasurementsVer0_3_0(const QString &name, qreal value)
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    QDomElement element = createElement(QStringLiteral("m"));

    SetAttribute(element, QStringLiteral("name"), name);
    SetAttribute(element, QStringLiteral("value"), QString().setNum(value));
    SetAttribute(element, QStringLiteral("description"), QString(""));
    SetAttribute(element, QStringLiteral("full_name"), QString(""));

    return element;
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertGenderToVer0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 1),
                      "Time to refactor the code.");

    const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("sex"));
    QDomElement sex = nodeList.at(0).toElement();

    QDomElement gender = createElement(QStringLiteral("gender"));
    gender.appendChild(createTextNode(sex.text()));

    QDomElement parent = sex.parentNode().toElement();
    parent.replaceChild(gender, sex);
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertPmSystemToVer0_3_2()
{
    // TODO. Delete if minimal supported version is 0.3.2
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 2),
                      "Time to refactor the code.");

    QDomElement pm_system = createElement(QStringLiteral("pm_system"));
    pm_system.appendChild(createTextNode(QStringLiteral("998")));

    const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("personal"));
    QDomElement personal = nodeList.at(0).toElement();

    QDomElement parent = personal.parentNode().toElement();
    parent.insertBefore(pm_system, personal);
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertMeasurementsToV0_3_3()
{
    // TODO. Delete if minimal supported version is 0.3.3
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 3),
                      "Time to refactor the code.");

    const QMap<QString, QString> names = OldNamesToNewNames_InV0_3_3();
    auto i = names.constBegin();
    while (i != names.constEnd())
    {
        const QDomNodeList nodeList = this->elementsByTagName(QStringLiteral("m"));
        if (nodeList.isEmpty())
        {
            ++i;
            continue;
        }

        for (int ii = 0; ii < nodeList.size(); ++ii)
        {
            const QString attrName = QStringLiteral("name");
            QDomElement element = nodeList.at(ii).toElement();
            const QString name = GetParametrString(element, attrName);
            if (name == i.value())
            {
                SetAttribute(element, attrName, i.key());
            }
        }

        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertToVer0_3_0()
{
    // TODO. Delete if minimal supported version is 0.3.0
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 0),
                      "Time to refactor the code.");

    AddRootComment();
    setVersion(QStringLiteral("0.3.0"));
    addNewTagsForVer0_3_0();
    convertMeasurementsToV0_3_0();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertToVer0_3_1()
{
    // TODO. Delete if minimal supported version is 0.3.1
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 1),
                      "Time to refactor the code.");

    setVersion(QStringLiteral("0.3.1"));
    convertGenderToVer0_3_1();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertToVer0_3_2()
{
    // TODO. Delete if minimal supported version is 0.3.2
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 2),
                      "Time to refactor the code.");

    setVersion(QStringLiteral("0.3.2"));
    convertPmSystemToVer0_3_2();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertToVer0_3_3()
{
    // TODO. Delete if minimal supported version is 0.3.3
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 3),
                      "Time to refactor the code.");

    setVersion(QStringLiteral("0.3.3"));
    convertMeasurementsToV0_3_3();
    Save();
}

//---------------------------------------------------------------------------------------------------------------------
void IndividualSizeConverter::convertToVer0_3_4()
{
    // TODO. Delete if minimal supported version is 0.3.4
    Q_STATIC_ASSERT_X(IndividualSizeConverter::MeasurementMinVer < CONVERTER_VERSION_CHECK(0, 3, 4),
                      "Time to refactor the code.");

    const QDomNodeList list = elementsByTagName("vit");
    for (int i=0; i < list.size(); ++i)
    {
        QDomElement element = list.at(i).toElement();
        if (!element.isNull())
        {
            element.setTagName("smis");
        }
    }

    setVersion(QStringLiteral("0.3.4"));
    Save();
}

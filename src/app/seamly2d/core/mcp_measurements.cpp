// SPDX-License-Identifier: GPL-3.0-or-later
#include "mcp_measurements.h"
#include "../vformat/measurements.h"
#include "../ifc/xml/individual_size_converter.h"
#include "../ifc/xml/multi_size_converter.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QRegularExpression>
#include <QSet>
#include <QTemporaryDir>
#include <cmath>
#include <memory>
#include <stdexcept>

namespace
{
void check(bool ok, const char *message) { if (!ok) throw std::runtime_error(message); }
QDomDocument parse(const QByteArray &bytes)
{
    check(bytes.size() <= 4 * 1024 * 1024, "Measurement file exceeds 4 MiB.");
    check(!bytes.toUpper().contains("<!DOCTYPE") && !bytes.toUpper().contains("<!ENTITY"), "DTD/entities are not accepted.");
    QDomDocument xml;
    check(static_cast<bool>(xml.setContent(bytes)), "Invalid measurement XML.");
    return xml;
}
}

QString McpMeasurements::token(const QString &path, const QByteArray &bytes)
{
    return QString::fromLatin1(QCryptographicHash::hash(QFileInfo(path).absoluteFilePath().toUtf8() + '\0' + bytes,
                                                       QCryptographicHash::Sha256).toHex());
}
QByteArray McpMeasurements::readBytes(const QString &path)
{
    QFile file(path);
    check(file.open(QIODevice::ReadOnly), "Could not read measurement file.");
    check(file.size() <= 4 * 1024 * 1024, "Measurement file exceeds 4 MiB.");
    return file.readAll();
}
McpMeasurements::Table McpMeasurements::inspect(const QByteArray &bytes, double size, double height)
{
    auto xml = parse(bytes);
    const auto root = xml.documentElement().tagName();
    const bool multi = root == "smms" || root == "vst";
    check(multi || root == "smis" || root == "vit", "Expected an individual or multisize measurement file.");
    QTemporaryDir directory; check(directory.isValid(), "Could not create measurement validation workspace.");
    const auto path = directory.filePath(multi ? "measurements.smms" : "measurements.smis");
    QFile copy(path); check(copy.open(QIODevice::WriteOnly) && copy.write(bytes) == bytes.size(), "Could not stage measurements.");
    copy.close();
    QString converted;
    if (multi) { MultiSizeConverter converter(path); converted = converter.Convert(); }
    else { IndividualSizeConverter converter(path); converted = converter.Convert(); }
    xml = parse(readBytes(converted));
    const Unit unit = StrToUnits(xml.documentElement().firstChildElement("unit").text());
    check(unit == Unit::Cm || unit == Unit::Mm || (!multi && unit == Unit::Inch), "Unsupported measurement units.");
    VContainer data(nullptr, &unit);
    MeasurementDoc document(&data); document.setXMLContent(converted);
    check(document.eachKnownNameIsValid(), "Unknown standard measurement name; custom names start with @.");
    qreal currentSize = size < 0 ? document.BaseSize() : size;
    qreal currentHeight = height < 0 ? document.BaseHeight() : height;
    document.setSize(&currentSize); document.setHeight(&currentHeight);
    document.readMeasurements();
    QJsonArray measurements;
    const auto nodes = document.elementsByTagName("m");
    for (int i = 0; i < nodes.size(); ++i)
    {
        const auto element = nodes.at(i).toElement();
        const auto name = element.attribute("name");
        const auto variable = qSharedPointerDynamicCast<MeasurementVariable>(data.DataVariables()->value(name));
        check(variable && variable->IsFormulaOk(), "Invalid measurement formula.");
        const auto value = static_cast<const MeasurementVariable &>(*variable).GetValue();
        check(std::isfinite(value), "Measurements must evaluate to finite values.");
        QJsonObject row{{"name", name}, {"value", value}, {"description", element.attribute("description")},
                        {"full_name", element.attribute("full_name")}};
        if (multi)
        {
            row.insert("base", element.attribute("base", "0").toDouble());
            row.insert("size_increase", element.attribute("size_increase", "0").toDouble());
            row.insert("height_increase", element.attribute("height_increase", "0").toDouble());
        }
        else row.insert("formula", element.attribute("value"));
        measurements.append(row);
    }
    return {document.toByteArray(), {{"type", multi ? "multisize" : "individual"}, {"unit", UnitsToStr(unit)},
        {"read_only", document.isReadOnly()}, {"base_size", document.BaseSize()}, {"base_height", document.BaseHeight()},
        {"size", currentSize}, {"height", currentHeight}, {"measurements", measurements}}};
}
McpMeasurements::Table McpMeasurements::prepare(const QByteArray &original, const QJsonObject &args)
{
    QByteArray bytes = original;
    if (bytes.isEmpty())
    {
        const auto type = args.value("type").toString();
        check(type == "individual" || type == "multisize", "New files require type individual or multisize.");
        check(args.contains("unit"), "New measurement files require unit.");
        const auto unit = StrToUnits(args.value("unit").toString());
        check(unit == Unit::Cm || unit == Unit::Mm || (type == "individual" && unit == Unit::Inch), "Invalid measurement units.");
        VContainer data(nullptr, &unit);
        std::unique_ptr<MeasurementDoc> document;
        if (type == "multisize")
        {
            check(args.contains("base_size") && args.contains("base_height"), "Multisize files require base_size and base_height in file units.");
            document.reset(new MeasurementDoc(unit, args.value("base_size").toInt(), args.value("base_height").toInt(), &data));
        }
        else document.reset(new MeasurementDoc(unit, &data));
        bytes = document->toByteArray();
    }
    const auto table = inspect(bytes);
    check(!table.info.value("read_only").toBool(), "Measurement file is read-only.");
    if (args.contains("type")) check(args.value("type") == table.info.value("type"), "Cannot change an existing file's type.");
    if (args.contains("unit")) check(args.value("unit") == table.info.value("unit"), "Cannot change an existing file's unit.");
    auto xml = parse(table.xml);
    auto body = xml.documentElement().firstChildElement("body-measurements");
    const bool multi = table.info.value("type") == QJsonValue("multisize");
    QSet<QString> changed;
    for (const auto &entry : args.value("measurements").toArray())
    {
        const auto row = entry.toObject();
        const auto name = row.value("name").toString();
        check(QRegularExpression(NameRegExp()).match(name.startsWith('@') ? name.mid(1) : name).hasMatch(), "Invalid measurement name.");
        check(!changed.contains(name), "Duplicate measurement update."); changed.insert(name);
        QDomElement element;
        for (auto node = body.firstChildElement("m"); !node.isNull(); node = node.nextSiblingElement("m"))
            if (node.attribute("name") == name) element = node;
        const bool added = element.isNull();
        if (added) { element = xml.createElement("m"); element.setAttribute("name", name); body.appendChild(element); }
        if (multi)
        {
            check(!row.contains("formula"), "Multisize entries use base and increments, not formulas.");
            check(!added || row.contains("base"), "New multisize entries require base.");
            for (const auto &key : {"base", "size_increase", "height_increase"})
                if (row.contains(key) || added) element.setAttribute(key, row.value(key).toDouble());
        }
        else
        {
            check(!row.contains("base") && !row.contains("size_increase") && !row.contains("height_increase"), "Individual entries use formula.");
            check(!added || row.contains("formula"), "New individual entries require formula.");
            if (row.contains("formula")) element.setAttribute("value", row.value("formula").toString());
        }
        for (const auto &key : {"description", "full_name"}) if (row.contains(key)) element.setAttribute(key, row.value(key).toString());
    }
    for (const auto &entry : args.value("remove").toArray())
    {
        const auto name = entry.toString();
        check(!changed.contains(name), "Cannot update and remove the same measurement."); changed.insert(name);
        QDomElement element;
        for (auto node = body.firstChildElement("m"); !node.isNull(); node = node.nextSiblingElement("m"))
            if (node.attribute("name") == name) element = node;
        check(!element.isNull(), "Unknown measurement to remove."); body.removeChild(element);
    }
    return inspect(xml.toByteArray());
}

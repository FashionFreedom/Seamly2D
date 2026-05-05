//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogvariablesimport.cpp
//  @brief  Helpers for importing custom variables from CSV-like text.
//---------------------------------------------------------------------------------------------------------------------

#include "dialogvariablesimport.h"

#include "../ifc/ifcdef.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/variables/custom_variable.h"

#include <QFile>
#include <QRegularExpression>
#include <QScopedPointer>
#include <QtNumeric>

namespace DialogVariablesImport
{
QString unitConvertVariableName(Unit from, Unit to)
{
    return QStringLiteral("%1%2to%3").arg(CustomIncrSign, UnitsToStr(from), UnitsToStr(to));
}

bool parseUnit(const QString &text, Unit *unit)
{
    const QString normalized = text.trimmed().toLower();
    if (normalized == QLatin1String("mm") || normalized == QObject::tr("millimeters").toLower())
    {
        *unit = Unit::Mm;
        return true;
    }
    if (normalized == QLatin1String("cm") || normalized == QObject::tr("centimeters").toLower())
    {
        *unit = Unit::Cm;
        return true;
    }
    if (normalized == QLatin1String("in") || normalized == QLatin1String("inch") || normalized == QLatin1String("inches"))
    {
        *unit = Unit::Inch;
        return true;
    }
    return false;
}

namespace
{
QChar detectCsvSeparator(const QString &text)
{
    const QString firstLine = text.section(QLatin1Char('\n'), 0, 0);
    QChar separator = QLatin1Char(',');
    int bestCount = firstLine.count(separator);

    const QList<QChar> candidates = QList<QChar>() << QLatin1Char(';') << QLatin1Char('\t') << QLatin1Char(':');
    for (const QChar candidate : candidates)
    {
        const int count = firstLine.count(candidate);
        if (count > bestCount)
        {
            separator = candidate;
            bestCount = count;
        }
    }

    return separator;
}

QList<QStringList> parseSeparatedText(const QString &text, QChar separator)
{
    QList<QStringList> rows;
    QStringList row;
    QString field;
    QChar quote;
    bool inQuote = false;

    for (int i = 0; i < text.size(); ++i)
    {
        const QChar ch = text.at(i);
        if (inQuote)
        {
            if (ch == quote)
            {
                if (i + 1 < text.size() && text.at(i + 1) == quote)
                {
                    field.append(ch);
                    ++i;
                }
                else
                {
                    inQuote = false;
                }
            }
            else
            {
                field.append(ch);
            }
            continue;
        }

        if ((ch == QLatin1Char('"') || ch == QLatin1Char('\'')) && field.trimmed().isEmpty())
        {
            quote = ch;
            inQuote = true;
            continue;
        }

        if (ch == separator)
        {
            row.append(field.trimmed());
            field.clear();
            continue;
        }

        if (ch == QLatin1Char('\n') || ch == QLatin1Char('\r'))
        {
            if (ch == QLatin1Char('\r') && i + 1 < text.size() && text.at(i + 1) == QLatin1Char('\n'))
            {
                ++i;
            }

            row.append(field.trimmed());
            field.clear();
            if (!row.isEmpty() && !(row.size() == 1 && row.constFirst().isEmpty()))
            {
                rows.append(row);
            }
            row.clear();
            continue;
        }

        field.append(ch);
    }

    row.append(field.trimmed());
    if (!row.isEmpty() && !(row.size() == 1 && row.constFirst().isEmpty()))
    {
        rows.append(row);
    }

    return rows;
}

QString cellText(const QList<QStringList> &rows, int row, int column)
{
    if (row < 0 || row >= rows.size() || column < 0 || column >= rows.at(row).size())
    {
        return QString();
    }

    return rows.at(row).at(column);
}

int columnCount(const QList<QStringList> &rows)
{
    int count = 0;
    for (const QStringList &row : rows)
    {
        count = qMax(count, row.size());
    }

    return count;
}

bool parseNumber(const QString &text, qreal *value)
{
    bool ok = false;
    *value = QLocale().toDouble(text.trimmed(), &ok);
    if (!ok)
    {
        *value = QLocale::c().toDouble(text.trimmed(), &ok);
    }
    return ok;
}

bool splitValueAndUnit(const QString &text, qreal *value, Unit *unit)
{
    static const QRegularExpression rx(QStringLiteral("^\\s*([+-]?(?:\\d+(?:[\\.,]\\d*)?|[\\.,]\\d+))\\s*([A-Za-z]+)\\s*$"));
    const QRegularExpressionMatch match = rx.match(text);
    if (!match.hasMatch())
    {
        return false;
    }
    return parseNumber(match.captured(1), value) && parseUnit(match.captured(2), unit);
}

bool hasHeader(const QList<QStringList> &rows)
{
    return !rows.isEmpty() && (cellText(rows, 0, 0).trimmed().toLower() == QLatin1String("name")
                               || cellText(rows, 0, 0).trimmed().toLower() == QObject::tr("name").toLower());
}

bool looksLikeExportedTable(const QList<QStringList> &rows, int firstDataRow)
{
    if (columnCount(rows) != 3)
    {
        return false;
    }

    for (int row = firstDataRow; row < rows.size(); ++row)
    {
        qreal value = 0;
        if (!cellText(rows, row, 0).trimmed().startsWith(CustomIncrSign)
            || !parseNumber(cellText(rows, row, 1), &value)
            || cellText(rows, row, 2).trimmed().isEmpty())
        {
            return false;
        }
    }

    return rows.size() > firstDataRow;
}

QSet<QString> importNames(const QList<QStringList> &rows, int firstDataRow, int nameColumn)
{
    QSet<QString> names;
    for (int row = firstDataRow; row < rows.size(); ++row)
    {
        const QString displayName = cellText(rows, row, nameColumn).trimmed();
        const QString cleanName = displayName.startsWith(CustomIncrSign) ? displayName.mid(1) : displayName;
        if (!cleanName.isEmpty() && QRegularExpression(NameRegExp()).match(cleanName).hasMatch())
        {
            names.insert(CustomIncrSign + cleanName);
        }
    }

    return names;
}

bool validateFormula(const QString &formula, VContainer *data, const QSet<QString> &availableImportNames,
                     QString *status)
{
    if (data == nullptr)
    {
        return true;
    }

    QHash<QString, QSharedPointer<VInternalVariable>> variables = *data->DataVariables();
    for (const QString &name : availableImportNames)
    {
        if (!variables.contains(name))
        {
            variables.insert(name, QSharedPointer<VInternalVariable>(new CustomVariable(data, name, 0, 0, QStringLiteral("0"), true)));
        }
    }

    try
    {
        QScopedPointer<Calculator> cal(new Calculator());
        const qreal result = cal->EvalFormula(&variables, formula);
        if (qIsInf(result) || qIsNaN(result))
        {
            *status = QObject::tr("Invalid result");
            return false;
        }
    }
    catch (qmu::QmuParserError &error)
    {
        *status = QObject::tr("Parser error: %1").arg(error.GetMsg());
        return false;
    }

    return true;
}
}

bool looksLikeHeader(const QList<QStringList> &rows)
{
    return hasHeader(rows);
}

int columnByHeader(const QList<QStringList> &rows, const QStringList &needles, int fallback)
{
    if (!looksLikeHeader(rows))
    {
        return fallback;
    }

    for (int column = 0; column < columnCount(rows); ++column)
    {
        const QString header = cellText(rows, 0, column).trimmed().toLower();
        for (const QString &needle : needles)
        {
            if (header.contains(needle))
            {
                return column;
            }
        }
    }

    return fallback;
}

QString cNumber(qreal value)
{
    return QLocale::c().toString(value, 'g', 15);
}

bool readImportTextFile(const QString &fileName, QString *text, QString *errorString)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (errorString != nullptr)
        {
            *errorString = file.errorString();
        }
        return false;
    }

    *text = QString::fromUtf8(file.readAll());
    return true;
}

QList<ImportedVariable> parseImportedVariables(const QString &text, VContainer *data, Unit targetUnit,
                                               QSet<QString> *convertVariables)
{
    QList<ImportedVariable> result;
    convertVariables->clear();

    const QList<QStringList> rows = parseSeparatedText(text, detectCsvSeparator(text));
    const int firstDataRow = looksLikeHeader(rows) ? 1 : 0;
    const bool header = looksLikeHeader(rows);
    const int columns = columnCount(rows);
    const int nameColumn = columnByHeader(rows, QStringList() << QStringLiteral("name"), 0);
    const bool exportedTable = !header && looksLikeExportedTable(rows, firstDataRow);
    const int valueColumn = columnByHeader(rows, QStringList() << QStringLiteral("value") << QStringLiteral("calculated"),
                                           exportedTable ? 1 : -1);
    const int formulaColumn = columnByHeader(rows, QStringList() << QStringLiteral("formula"),
                                             exportedTable ? 2 : !header && columns > 1 ? 1 : -1);
    const int descriptionColumn = columnByHeader(rows, QStringList() << QStringLiteral("description"),
                                                 !exportedTable && !header && columns > 2 ? 2 : -1);
    const QMap<QString, QSharedPointer<CustomVariable>> existing = data != nullptr
            ? data->variablesData()
            : QMap<QString, QSharedPointer<CustomVariable>>();
    const QSet<QString> availableImportNames = importNames(rows, firstDataRow, nameColumn);
    QSet<QString> seen;

    for (int row = firstDataRow; row < rows.size(); ++row)
    {
        ImportedVariable imported;
        const QString displayName = cellText(rows, row, nameColumn).trimmed();
        const QString cleanName = displayName.startsWith(CustomIncrSign) ? displayName.mid(1) : displayName;
        imported.name = CustomIncrSign + cleanName;
        imported.description = descriptionColumn >= 0 ? cellText(rows, row, descriptionColumn).trimmed() : QString();

        if (cleanName.isEmpty())
        {
            imported.status = QObject::tr("Missing name");
        }
        else if (!QRegularExpression(NameRegExp()).match(cleanName).hasMatch())
        {
            imported.status = QObject::tr("Invalid name");
        }
        else if (seen.contains(imported.name))
        {
            imported.status = QObject::tr("Duplicate in CSV");
        }
        else if (data != nullptr && !existing.contains(imported.name) && !data->IsUnique(imported.name))
        {
            imported.status = QObject::tr("Name is already used");
        }
        else
        {
            const QString rawFormula = formulaColumn >= 0 ? cellText(rows, row, formulaColumn).trimmed() : QString();
            Unit valueUnit = targetUnit;
            qreal value = 0;
            QString formulaToValidate;

            if (rawFormula.isEmpty() && valueColumn >= 0)
            {
                imported.status = QObject::tr("Missing formula");
            }

            if (!imported.status.isEmpty())
            {
                imported.valid = false;
            }
            else if (!rawFormula.isEmpty() && splitValueAndUnit(rawFormula, &value, &valueUnit))
            {
                const QString valueText = cNumber(value);
                formulaToValidate = valueText;
                if (valueUnit == targetUnit)
                {
                    imported.formula = valueText;
                }
                else
                {
                    const QString convertName = unitConvertVariableName(valueUnit, targetUnit);
                    convertVariables->insert(convertName);
                    imported.formula = QStringLiteral("%1 * %2").arg(valueText, convertName);
                }
            }
            else if (!rawFormula.isEmpty() && valueUnit != targetUnit)
            {
                formulaToValidate = rawFormula;
                const QString convertName = unitConvertVariableName(valueUnit, targetUnit);
                convertVariables->insert(convertName);
                imported.formula = QStringLiteral("(%1) * %2").arg(rawFormula, convertName);
            }
            else if (!rawFormula.isEmpty())
            {
                formulaToValidate = rawFormula;
                imported.formula = rawFormula;
            }
            else
            {
                imported.status = QObject::tr("Missing formula");
            }

            if (imported.status.isEmpty() && !validateFormula(formulaToValidate, data, availableImportNames, &imported.status))
            {
                imported.valid = false;
            }

            if (imported.status.isEmpty())
            {
                imported.valid = true;
                imported.exists = existing.contains(imported.name);
                imported.status = imported.exists ? QObject::tr("Update") : QObject::tr("Add");
                seen.insert(imported.name);
            }
        }

        result.append(imported);
    }

    return result;
}
}

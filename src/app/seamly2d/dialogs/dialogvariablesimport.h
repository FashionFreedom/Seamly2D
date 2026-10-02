//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogvariablesimport.h
//  @brief  Helpers for importing custom variables from CSV-like text.
//---------------------------------------------------------------------------------------------------------------------

#ifndef DIALOG_VARIABLES_IMPORT_H
#define DIALOG_VARIABLES_IMPORT_H

#include "../vmisc/def.h"

#include <QList>
#include <QSet>
#include <QString>

class VContainer;

namespace DialogVariablesImport
{
struct ImportedVariable
{
    QString name;
    QString formula;
    QString description;
    QString status;
    bool valid = false;
    bool exists = false;
};

QString unitConvertVariableName(Unit from, Unit to);
bool parseUnit(const QString &text, Unit *unit);
QString cNumber(qreal value);
bool readImportTextFile(const QString &fileName, QString *text, QString *errorString = nullptr);
QList<ImportedVariable> parseImportedVariables(const QString &text, VContainer *data, Unit targetUnit,
                                               QSet<QString> *convertVariables);
}

#endif // DIALOG_VARIABLES_IMPORT_H

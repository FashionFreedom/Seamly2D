// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY_MCP_MEASUREMENTS_H
#define SEAMLY_MCP_MEASUREMENTS_H
#include <QByteArray>
#include <QJsonObject>
#include <QString>

namespace McpMeasurements
{
struct Table { QByteArray xml; QJsonObject info; };
QString token(const QString &path, const QByteArray &bytes);
QByteArray readBytes(const QString &path);
Table inspect(const QByteArray &bytes, double size = -1, double height = -1);
Table prepare(const QByteArray &original, const QJsonObject &arguments);
}
#endif

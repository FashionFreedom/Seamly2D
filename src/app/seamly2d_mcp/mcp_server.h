// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY_MCP_SERVER_H
#define SEAMLY_MCP_SERVER_H

#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <functional>

class McpServer
{
public:
    using BridgeCall = std::function<QJsonObject(const QString &, const QString &, const QJsonObject &)>;

    explicit McpServer(const QString &socketPath, BridgeCall bridge = callBridge);
    // Undefined means a notification: no response may be written to stdout.
    QJsonValue handle(const QJsonValue &request);
    static QJsonObject error(const QJsonValue &id, int code, const QString &message);
    static QJsonObject callBridge(const QString &socketPath, const QString &method, const QJsonObject &arguments);
    static QJsonArray tools();

private:
    enum class Phase { New, Initializing, Ready };
    QString m_socketPath;
    BridgeCall m_bridge;
    Phase m_phase = Phase::New;
    QString m_protocol = QStringLiteral("2025-06-18");
    QJsonObject m_legacyCapabilities;
};

// Bounded newline framing over UTF-8 stdio; logs/help never use this channel.
int serveMcp(McpServer &server, QFile &input, QFile &output);

#endif

// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef SEAMLY_MCP_BRIDGE_H
#define SEAMLY_MCP_BRIDGE_H

#include <QJsonObject>
#include <QLocalServer>
#include <QObject>

class MainWindow;

// Private, newline-delimited JSON IPC. The separate stdio process speaks MCP.
// This object and all document operations live on the application's GUI thread.
class McpBridge final : public QObject
{
public:
    explicit McpBridge(MainWindow *window);
    bool start(const QString &socketPath);
    QString errorString() const;

private:
    MainWindow *m_window;
    QLocalServer m_server;
    QString m_error;
    QByteArray m_session;
    int m_connections = 0;
    quint64 m_revision = 0;
    bool m_executing = false;

    void acceptConnections();
    QJsonObject execute(const QString &method, const QJsonObject &arguments);
    QString stateToken() const;
    QString measurementsPath() const;
    QByteArray validateDocument(const QByteArray &xml, const QString &origin, bool allowConversion = false) const;
    void restoreDocument(const QByteArray &xml, const QString &activeBlock);
    void commitDocument(const QByteArray &xml, const QString &label);
};

#endif

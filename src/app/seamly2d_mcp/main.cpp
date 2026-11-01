// SPDX-License-Identifier: GPL-3.0-or-later
#include "mcp_server.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDir>
#include <cstdio>
#ifdef Q_OS_UNIX
#include <csignal>
#endif

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("seamly2d-mcp"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.4.0"));
    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Local Seamly2D MCP stdio adapter. Requires the enabled application bridge."));
    parser.addHelpOption();
    parser.addVersionOption();
    const QCommandLineOption socketOption(QStringLiteral("socket"),
        QStringLiteral("Absolute path used by the app's SEAMLY_MCP_SOCKET (defaults to that environment variable)."),
        QStringLiteral("path"), qEnvironmentVariable("SEAMLY_MCP_SOCKET", QDir::homePath() + QStringLiteral("/.seamly2d/mcp/app.sock")));
    parser.addOption(socketOption);
    parser.process(app);
    const QString socketPath = parser.value(socketOption);
    if (!parser.positionalArguments().isEmpty() || socketPath.isEmpty() || !QDir::isAbsolutePath(socketPath))
    {
        std::fputs("seamly2d-mcp: --socket (or SEAMLY_MCP_SOCKET) must be an absolute path; no positional arguments accepted.\n", stderr);
        return 2;
    }
#ifndef Q_OS_UNIX
    std::fputs("seamly2d-mcp: this version supports macOS and Linux local sockets.\n", stderr);
    return 2;
#else
    // The client may close its read end while a request is completing.
    std::signal(SIGPIPE, SIG_IGN);
    QFile input;
    QFile output;
    if (!input.open(stdin, QIODevice::ReadOnly) || !output.open(stdout, QIODevice::WriteOnly))
    {
        std::fputs("seamly2d-mcp: cannot open stdin/stdout.\n", stderr);
        return 1;
    }
    McpServer server(socketPath);
    return serveMcp(server, input, output);
#endif
}

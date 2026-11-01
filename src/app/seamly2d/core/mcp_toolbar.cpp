// SPDX-License-Identifier: GPL-3.0-or-later
#include "../mainwindow.h"
#include "ui_mainwindow.h"
#include "mcp_bridge.h"
#include "application_2d.h"
#include "../vmisc/vsettings.h"
#include <QAction>
#include <QClipboard>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QMenu>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStatusBar>
#include <QStyle>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QToolTip>

void MainWindow::setMcpBridge(McpBridge *bridge)
{
    m_mcpBridge = bridge;
    updateMcpStatus();
}

void MainWindow::initializeMcpToolBar()
{
    auto *menu = new QMenu(this);
    m_mcpEnabledAction = menu->addAction(tr("Enable MCP"));
    m_mcpEnabledAction->setCheckable(true);
    connect(m_mcpEnabledAction, &QAction::triggered, this, [this](bool enabled)
    {
        if (!m_mcpBridge) return;
        m_mcpProbeError.clear(); m_mcpLastCheck.clear();
        if (enabled) m_mcpBridge->startDefault(); else m_mcpBridge->stop();
        updateMcpStatus();
    });
    menu->addAction(tr("Verify connection"), this, &MainWindow::verifyMcpConnection);
    auto *copy = menu->addAction(tr("Copy socket path"));
    connect(copy, &QAction::triggered, this, [this]()
    {
        if (m_mcpBridge && m_mcpBridge->isListening()) qApp->clipboard()->setText(m_mcpBridge->socketPath());
    });
    connect(menu, &QMenu::aboutToShow, this, [this, copy]() { copy->setEnabled(m_mcpBridge && m_mcpBridge->isListening()); });
    m_mcpButton = new QToolButton(this);
    m_mcpButton->setObjectName("mcpConnectionButton");
    m_mcpButton->setIcon(style()->standardIcon(QStyle::SP_DriveNetIcon));
    m_mcpButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    m_mcpButton->setPopupMode(QToolButton::MenuButtonPopup);
    m_mcpButton->setMenu(menu);
    m_mcpButton->setFocusPolicy(Qt::StrongFocus);
    connect(m_mcpButton, &QToolButton::clicked, this, &MainWindow::verifyMcpConnection);
    ui->file_ToolBar->addSeparator();
    ui->file_ToolBar->addWidget(m_mcpButton);
    updateMcpStatus();
}

void MainWindow::updateMcpStatus()
{
    if (!m_mcpButton) return;
    const bool enabled = m_mcpBridge && m_mcpBridge->isListening();
    const int clients = m_mcpBridge ? m_mcpBridge->clientCount() : 0;
    const auto error = !m_mcpProbeError.isEmpty() ? m_mcpProbeError : m_mcpBridge ? m_mcpBridge->errorString() : QString{};
    QString label, description;
    if (m_mcpProbe) { label = tr("MCP: Checking..."); description = tr("Checking the MCP executable and the application bridge."); }
    else if (!error.isEmpty()) { label = tr("MCP: Error"); description = error; }
    else if (!enabled) { label = tr("MCP: Off"); description = tr("Click to enable MCP and check the connection."); }
    else if (clients) { label = tr("MCP: Connected"); description = tr("Connected MCP clients: %1").arg(clients); }
    else { label = tr("MCP: Waiting"); description = tr("MCP is active. Waiting for a client to connect."); }
    if (!m_mcpLastCheck.isEmpty()) description += "\n" + m_mcpLastCheck;
    if (enabled) description += "\n" + tr("Click to verify the connection.");
    m_mcpButton->setText(label);
    m_mcpButton->setAccessibleName(label);
    m_mcpButton->setAccessibleDescription(description);
    m_mcpButton->setToolTip(description);
    m_mcpButton->setEnabled(!m_mcpProbe);
    const QSignalBlocker blocker(m_mcpEnabledAction);
    m_mcpEnabledAction->setChecked(enabled);
}

void MainWindow::verifyMcpConnection()
{
    if (!m_mcpBridge || m_mcpProbe) return;
    m_mcpProbeError.clear();
    m_mcpLastCheck.clear();
    if (!m_mcpBridge->isListening() && !m_mcpBridge->startDefault()) { updateMcpStatus(); return; }
    const auto directory = QCoreApplication::applicationDirPath();
    QString executable = directory + "/seamly2d-mcp";
    if (!QFileInfo(executable).isExecutable()) executable = QDir(directory).absoluteFilePath("../../seamly2d_mcp/bin/seamly2d-mcp");
    if (!QFileInfo(executable).isExecutable()) executable = QStandardPaths::findExecutable("seamly2d-mcp");
    if (executable.isEmpty() || !QFileInfo(executable).isExecutable())
    {
        m_mcpProbeError = tr("The MCP executable was not found. Rebuild or reinstall Seamly2D."); updateMcpStatus(); return;
    }
    auto *process = new QProcess(this); m_mcpProbe = process;
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert("SEAMLY_MCP_PROBE", "1");
    process->setProcessEnvironment(environment);
    const auto finish = [this, process](const QString &error, int count)
    {
        if (m_mcpProbe != process) return;
        m_mcpProbe = nullptr; m_mcpProbeError = error;
        m_mcpLastCheck = error.isEmpty() ? tr("Connection verified: %1 tools available.").arg(count) : QString{};
        updateMcpStatus();
        const auto message = error.isEmpty() ? m_mcpLastCheck + " " + (m_mcpBridge->clientCount()
            ? tr("Connected MCP clients: %1").arg(m_mcpBridge->clientCount())
            : tr("MCP is active. Waiting for a client to connect.")) : error;
        ui->statusBar->showMessage(message, 8000);
        QToolTip::showText(m_mcpButton->mapToGlobal(QPoint(0, m_mcpButton->height())), message, m_mcpButton);
        process->deleteLater();
    };
    connect(process, &QProcess::started, this, [process]()
    {
        const auto send = [process](const QJsonObject &message) { process->write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n'); };
        send({{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}, {"params", QJsonObject{{"protocolVersion", "2025-06-18"},
            {"capabilities", QJsonObject{}}, {"clientInfo", QJsonObject{{"name", "seamly2d-ui-check"}, {"version", "1"}}}}}});
        send({{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}});
        send({{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}});
        send({{"jsonrpc", "2.0"}, {"id", 3}, {"method", "tools/call"}, {"params", QJsonObject{{"name", "get_pattern_summary"}, {"arguments", QJsonObject{}}}}});
        process->closeWriteChannel();
    });
    connect(process, &QProcess::errorOccurred, this, [finish](QProcess::ProcessError error)
    { if (error == QProcess::FailedToStart) finish(tr("Could not start the MCP connection check."), 0); });
    connect(process, qOverload<int, QProcess::ExitStatus>(&QProcess::finished), this, [process, finish](int code, QProcess::ExitStatus status)
    {
        QJsonObject catalog, summary;
        for (const auto &line : process->readAllStandardOutput().split('\n'))
        {
            const auto reply = QJsonDocument::fromJson(line).object();
            if (reply.value("id") == QJsonValue(2)) catalog = reply.value("result").toObject();
            if (reply.value("id") == QJsonValue(3)) summary = reply.value("result").toObject();
        }
        const int count = catalog.value("tools").toArray().size();
        if (code == 0 && status == QProcess::NormalExit && count > 0 && summary.value("isError") == QJsonValue(false)
            && summary.value("structuredContent").toObject().value("state_token").toString().size() == 64) finish({}, count);
        else
        {
            QString detail;
            for (const auto &entry : summary.value("content").toArray()) detail += entry.toObject().value("text").toString();
            finish(process->property("mcpTimeout").toBool() ? tr("The MCP connection check timed out. Try again.")
                : tr("MCP verification failed. %1").arg(detail.isEmpty() ? process->errorString() : detail), 0);
        }
    });
    QTimer::singleShot(5000, process, [process]()
    { if (process->state() != QProcess::NotRunning) { process->setProperty("mcpTimeout", true); process->kill(); } });
    updateMcpStatus();
    process->start(executable, {"--socket", m_mcpBridge->socketPath()});
}

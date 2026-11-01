// SPDX-License-Identifier: GPL-3.0-or-later
#include "../../app/seamly2d_mcp/mcp_server.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QLibraryInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QProcess>
#include <QProcessEnvironment>
#include <QTemporaryDir>
#include <QTest>
#include <memory>
#include <stdexcept>

namespace
{
QJsonObject request(const QString &method, const QJsonObject &params = {}, const QJsonValue &id = 1)
{
    return {{"jsonrpc", "2.0"}, {"id", id}, {"method", method}, {"params", params}};
}

QJsonObject initializeParams(const QString &version = QStringLiteral("2025-06-18"))
{
    return {{"protocolVersion", version}, {"capabilities", QJsonObject{}},
            {"clientInfo", QJsonObject{{"name", "test"}, {"version", "1"}}}};
}

McpServer readyServer(McpServer::BridgeCall bridge = [](const QString &, const QString &, const QJsonObject &)
    { return QJsonObject{{"state_token", QString(64, 'a')}}; })
{
    McpServer server(QStringLiteral("/unused.sock"), std::move(bridge));
    server.handle(request(QStringLiteral("initialize"), initializeParams()));
    server.handle(QJsonObject{{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}});
    return server;
}

class Client
{
public:
    QProcess process;
    int nextId = 0;

    ~Client()
    {
        if (process.state() != QProcess::NotRunning)
        {
            process.kill();
            process.waitForFinished(3000);
        }
    }

    bool start(const QString &path)
    {
        QString program = qEnvironmentVariable("SEAMLY_MCP_TEST_SERVER");
        if (program.isEmpty())
        {
            program = QDir(QCoreApplication::applicationDirPath()).absoluteFilePath(
                QStringLiteral("../../../app/seamly2d_mcp/bin/seamly2d-mcp"));
        }
        process.start(program, {QStringLiteral("--socket"), path});
        return process.waitForStarted(3000);
    }

    void send(const QJsonObject &message)
    {
        process.write(QJsonDocument(message).toJson(QJsonDocument::Compact) + '\n');
        process.waitForBytesWritten(3000);
    }

    QJsonObject receive()
    {
        QElapsedTimer timer;
        timer.start();
        while (!process.canReadLine())
        {
            if (timer.elapsed() > 5000 || !process.waitForReadyRead(5000 - static_cast<int>(timer.elapsed())))
            {
                return {{"test_error", QString::fromUtf8(process.readAllStandardError()) + process.errorString()}};
            }
        }
        const auto line = process.readLine();
        QJsonParseError error;
        const auto document = QJsonDocument::fromJson(line, &error);
        if (!document.isObject() || error.error != QJsonParseError::NoError)
        {
            return {{"test_error", QString::fromUtf8(line)}};
        }
        return document.object();
    }

    QJsonObject exchange(const QString &method, const QJsonObject &params = {})
    {
        send(request(method, params, ++nextId));
        return receive();
    }

    bool initialize()
    {
        const auto reply = exchange(QStringLiteral("initialize"), initializeParams());
        send({{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}});
        return reply.contains("result");
    }

    QJsonObject call(const QString &name, const QJsonObject &arguments = {})
    {
        const auto reply = exchange(QStringLiteral("tools/call"), {{"name", name}, {"arguments", arguments}});
        if (!reply.contains("result"))
        {
            return reply;
        }
        return reply.value("result").toObject();
    }
};

const QByteArray fixture = R"(<?xml version="1.0" encoding="UTF-8"?>
<pattern>
  <version>0.7.6</version><unit>cm</unit><description/><notes/><measurements/>
  <variables><variable name="#testLength" formula="10" description="MCP test"/></variables>
  <draftBlock name="MCP test">
    <calculation><point id="1" type="single" name="A" x="10" y="20" mx="5" my="10"/></calculation>
    <modeling/><pieces/><groups/><images/>
  </draftBlock>
</pattern>
)";
}

class McpTest : public QObject
{
    Q_OBJECT
private slots:
    void lifecycle();
    void legacyProtocol();
    void invalidArguments_data();
    void invalidArguments();
    void requestsAndNotifications();
    void toolResults();
    void interactiveStdio();
    void framingAndLimits();
    void bridgeTransport();
    void applicationIntegration();
    void modernProtocol();
    void skillsAndResources();
};

void McpTest::lifecycle()
{
    McpServer server(QStringLiteral("/unused.sock"));
    QCOMPARE(server.handle(request("tools/list")).toObject().value("error").toObject().value("code").toInt(), -32600);
    QCOMPARE(server.handle(request("ping")).toObject().value("result").toObject(), QJsonObject{});
    auto result = server.handle(request("initialize", initializeParams("future"))).toObject().value("result").toObject();
    QCOMPARE(result.value("protocolVersion").toString(), QStringLiteral("2025-06-18"));
    QVERIFY(result.value("capabilities").toObject().contains("tools"));
    QCOMPARE(server.handle(request("tools/list")).toObject().value("error").toObject().value("code").toInt(), -32600);
    QVERIFY(server.handle(QJsonObject{{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}}).isUndefined());
    const auto tools = server.handle(request("tools/list")).toObject().value("result").toObject().value("tools").toArray();
    QCOMPARE(tools.size(), McpServer::tools().size());
    QVERIFY(tools.size() >= 30);
    for (const auto &tool : tools) if (tool.toObject().value("name") == QJsonValue("create_point")) QVERIFY(!tool.toObject().value("annotations").toObject().value("readOnlyHint").toBool());
    QCOMPARE(server.handle(request("initialize", initializeParams())).toObject().value("error").toObject().value("code").toInt(), -32600);
}

void McpTest::legacyProtocol()
{
    McpServer server(QStringLiteral("/unused.sock"), [](const QString &, const QString &, const QJsonObject &)
        { return QJsonObject{{"ok", true}}; });
    const auto reply = server.handle(request("initialize", initializeParams("2024-11-05"))).toObject();
    QCOMPARE(reply.value("result").toObject().value("protocolVersion").toString(), QStringLiteral("2024-11-05"));
    server.handle(QJsonObject{{"jsonrpc", "2.0"}, {"method", "notifications/initialized"}});
    const auto result = server.handle(request("tools/call", {{"name", "get_selection"}})).toObject().value("result").toObject();
    QVERIFY(result.contains("content"));
    QVERIFY(!result.contains("structuredContent"));
}

void McpTest::invalidArguments_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QJsonValue>("arguments");
    QTest::newRow("missing") << QString("create_point") << QJsonValue(QJsonObject{});
    QTest::newRow("boolean") << QString("list_objects") << QJsonValue(QJsonObject{{"limit", true}});
    QTest::newRow("fraction") << QString("list_objects") << QJsonValue(QJsonObject{{"limit", 1.5}});
    QTest::newRow("large") << QString("list_objects") << QJsonValue(QJsonObject{{"limit", 201}});
    QTest::newRow("negative") << QString("list_objects") << QJsonValue(QJsonObject{{"offset", -1}});
    QTest::newRow("extra") << QString("get_selection") << QJsonValue(QJsonObject{{"extra", 1}});
    QTest::newRow("unknown") << QString("missing") << QJsonValue(QJsonObject{});
    QTest::newRow("short-token") << QString("undo") << QJsonValue(QJsonObject{{"state_token", "short"}});
    QTest::newRow("array") << QString("get_selection") << QJsonValue(QJsonArray{});
}

void McpTest::invalidArguments()
{
    QFETCH(QString, name);
    QFETCH(QJsonValue, arguments);
    bool called = false;
    auto server = readyServer([&called](const QString &, const QString &, const QJsonObject &)
        { called = true; return QJsonObject{}; });
    const auto reply = server.handle(request("tools/call", {{"name", name}, {"arguments", arguments}})).toObject();
    QCOMPARE(reply.value("error").toObject().value("code").toInt(), -32602);
    QVERIFY(!called);
}

void McpTest::requestsAndNotifications()
{
    auto server = readyServer();
    QCOMPARE(server.handle(QJsonArray{}).toObject().value("error").toObject().value("code").toInt(), -32600);
    QCOMPARE(server.handle(request("missing")).toObject().value("error").toObject().value("code").toInt(), -32601);
    QCOMPARE(server.handle(request("ping", {}, true)).toObject().value("error").toObject().value("code").toInt(), -32600);
    QVERIFY(server.handle(QJsonObject{{"jsonrpc", "2.0"}, {"method", "notifications/cancelled"}}).isUndefined());
    QCOMPARE(server.handle(request("ping", {}, "text-id")).toObject().value("id").toString(), QStringLiteral("text-id"));
}

void McpTest::toolResults()
{
    auto server = readyServer();
    auto result = server.handle(request("tools/call", {{"name", "get_pattern_summary"}})).toObject().value("result").toObject();
    QCOMPARE(result.value("isError"), QJsonValue(false));
    const auto text = result.value("content").toArray().at(0).toObject().value("text").toString();
    QCOMPARE(QJsonDocument::fromJson(text.toUtf8()).object(), result.value("structuredContent").toObject());
    auto failing = readyServer([](const QString &, const QString &, const QJsonObject &) -> QJsonObject
        { throw std::runtime_error("Document state changed"); });
    result = failing.handle(request("tools/call", {{"name", "get_selection"}})).toObject().value("result").toObject();
    QCOMPARE(result.value("isError"), QJsonValue(true));
}

void McpTest::interactiveStdio()
{
    Client client;
    QVERIFY2(client.start(QStringLiteral("/unused.sock")), qPrintable(client.process.errorString()));
    // Keep stdin open: replies must not depend on EOF or fill a stdio buffer.
    QVERIFY(client.initialize());
    const auto reply = client.exchange(QStringLiteral("tools/list"));
    QCOMPARE(reply.value("result").toObject().value("tools").toArray().size(), McpServer::tools().size());
    QCOMPARE(client.exchange(QStringLiteral("ping")).value("result").toObject(), QJsonObject{});
    client.process.closeWriteChannel();
    QVERIFY(client.process.waitForFinished(3000));
    QCOMPARE(client.process.exitCode(), 0);
    QVERIFY(client.process.readAllStandardOutput().isEmpty());
    QVERIFY(client.process.readAllStandardError().isEmpty());
}

void McpTest::framingAndLimits()
{
    Client client;
    QVERIFY(client.start(QStringLiteral("/unused.sock")));
    client.process.write("not json\n[]\nnull\ntrue\n42\n{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\"}\n");
    client.process.waitForBytesWritten(1000);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32700);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32600);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32600);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32600);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32600);
    client.send(request("ping"));
    QVERIFY(client.receive().contains("result"));
    client.process.write(QByteArray(8 * 1024 * 1024 + 1, 'x'));
    client.process.waitForBytesWritten(3000);
    QCOMPARE(client.receive().value("error").toObject().value("code").toInt(), -32700);
    QVERIFY(client.process.waitForFinished(3000) || client.process.state() == QProcess::NotRunning);
    QCOMPARE(client.process.exitCode(), 0);
}

void McpTest::bridgeTransport()
{
    QTemporaryDir directory(QStringLiteral("/tmp/smcp-XXXXXX"));
    QVERIFY(directory.isValid());
    const QString path = directory.filePath(QStringLiteral("app.sock"));
    QLocalServer listener;
    QVERIFY2(listener.listen(path), qPrintable(listener.errorString()));
    Client client;
    QVERIFY(client.start(path));
    QVERIFY(client.initialize());
    const QList<QByteArray> replies{
        "{\"id\":1,\"result\":{\"ok\":true}}\n",
        "{\"id\":1,\"error\":\"Document state changed\"}\n",
        "{\"id\":99,\"result\":{}}\n",
        "not-json\n"
    };
    for (int i = 0; i < replies.size(); ++i)
    {
        client.send(request("tools/call", {{"name", "get_selection"}}, i + 10));
        QVERIFY(listener.waitForNewConnection(3000));
        std::unique_ptr<QLocalSocket> connection(listener.nextPendingConnection());
        QVERIFY(connection->canReadLine() || connection->waitForReadyRead(3000));
        const auto forwarded = QJsonDocument::fromJson(connection->readLine()).object();
        QCOMPARE(forwarded.value("method").toString(), QStringLiteral("get_selection"));
        const auto bytes = replies.at(i);
        connection->write(bytes.left(bytes.size() / 2));
        connection->flush();
        QTest::qWait(5);
        connection->write(bytes.mid(bytes.size() / 2));
        connection->flush();
        const auto result = client.receive().value("result").toObject();
        QCOMPARE(result.value("isError"), QJsonValue(i != 0));
        if (i == 0)
        {
            QCOMPARE(result.value("structuredContent").toObject(), (QJsonObject{{"ok", true}}));
        }
    }
    listener.close();
    const auto failed = client.call(QStringLiteral("get_selection"));
    QCOMPARE(failed.value("isError"), QJsonValue(true));
    QVERIFY(failed.value("content").toArray().at(0).toObject().value("text").toString().contains("not retried"));
}


namespace
{
QJsonObject modernRequest(const QString &method, QJsonObject params = {}, QJsonObject extensions = {})
{
    params.insert("_meta", QJsonObject{{"io.modelcontextprotocol/protocolVersion", "2026-07-28"},
        {"io.modelcontextprotocol/clientCapabilities", QJsonObject{{"extensions", extensions}}}});
    return request(method, params);
}
}
void McpTest::modernProtocol()
{
    McpServer server("/unused.sock");
    auto reply = server.handle(modernRequest("server/discover")).toObject();
    auto result = reply.value("result").toObject();
    QCOMPARE(result.value("resultType"), QJsonValue("complete"));
    QVERIFY(result.value("supportedVersions").toArray().contains("2026-07-28"));
    QVERIFY(result.value("capabilities").toObject().value("extensions").toObject().contains("io.modelcontextprotocol/skills"));
    QVERIFY(result.value("_meta").toObject().contains("io.modelcontextprotocol/serverInfo"));
    QVERIFY(result.contains("ttlMs"));
    // Modern requests work without initialize and never change legacy state.
    reply = server.handle(modernRequest("tools/list")).toObject();
    QCOMPARE(reply.value("result").toObject().value("tools").toArray().size(), McpServer::tools().size());
    QCOMPARE(server.handle(request("tools/list")).toObject().value("error").toObject().value("code").toInt(), -32600);
    auto unsupported = modernRequest("tools/list"); auto params = unsupported.value("params").toObject();
    auto meta = params.value("_meta").toObject(); meta.insert("io.modelcontextprotocol/protocolVersion", "future"); params.insert("_meta", meta); unsupported.insert("params", params);
    auto detail = server.handle(unsupported).toObject().value("error").toObject();
    QCOMPARE(detail.value("code").toInt(), -32022);
    QVERIFY(detail.value("data").toObject().value("supported").toArray().contains("2026-07-28"));
    meta.insert("io.modelcontextprotocol/protocolVersion", "2026-07-28"); meta.remove("io.modelcontextprotocol/clientCapabilities"); params.insert("_meta", meta); unsupported.insert("params", params);
    QCOMPARE(server.handle(unsupported).toObject().value("error").toObject().value("code").toInt(), -32602);
    QCOMPARE(server.handle(modernRequest("ping")).toObject().value("error").toObject().value("code").toInt(), -32601);
}
void McpTest::skillsAndResources()
{
    McpServer server("/unused.sock");
    QCOMPARE(server.handle(modernRequest("skills/list")).toObject().value("error").toObject().value("code").toInt(), -32021);
    QJsonObject extensions{{"io.modelcontextprotocol/skills", QJsonObject{}}};
    auto result = server.handle(modernRequest("skills/list", {}, extensions)).toObject().value("result").toObject();
    const auto skill = result.value("skills").toArray().at(0).toObject();
    QCOMPARE(skill.value("frontmatter").toObject().value("name"), QJsonValue("seamly2d-pattern"));
    QCOMPARE(server.handle(modernRequest("skills/get", {{"uri", skill.value("uri")}}, extensions)).toObject().value("result").toObject().value("skill"), QJsonValue(skill));
    for (const auto &file : skill.value("resources").toArray())
    {
        const auto entry = file.toObject();
        result = server.handle(modernRequest("resources/read", {{"uri", entry.value("uri")}}, extensions)).toObject().value("result").toObject();
        const auto content = result.value("contents").toArray().at(0).toObject().value("text").toString().toUtf8();
        QCOMPARE(content.size(), entry.value("size").toInt());
        QCOMPARE("sha256:" + QString::fromLatin1(QCryptographicHash::hash(content, QCryptographicHash::Sha256).toHex()), entry.value("digest").toString());
    }
    const auto discovery = server.handle(modernRequest("server/discover")).toObject().value("result").toObject();
    QVERIFY(!discovery.value("capabilities").toObject().value("extensions").toObject().contains("io.modelcontextprotocol/ui"));
    const auto resources = server.handle(modernRequest("resources/list", {}, extensions)).toObject().value("result").toObject().value("resources").toArray();
    QCOMPARE(resources.size(), 2);
    for (const auto &resource : resources) QCOMPARE(resource.toObject().value("mimeType"), QJsonValue("text/markdown"));
    auto wrong = server.handle(modernRequest("resources/read", {{"uri", "skill://seamly2d-pattern/../private"}}, extensions)).toObject();
    QCOMPARE(wrong.value("error").toObject().value("code").toInt(), -32602);
}

void McpTest::applicationIntegration()
{
    const QString app = qEnvironmentVariable("SEAMLY_MCP_TEST_APP");
    if (app.isEmpty())
    {
        QSKIP("Set SEAMLY_MCP_TEST_APP to run against the built Qt app.");
    }
    QTemporaryDir directory(QStringLiteral("/tmp/smcp-XXXXXX"));
    QVERIFY(directory.isValid());
    const QString patternPath = directory.filePath(QStringLiteral("test.sm2d"));
    QFile pattern(patternPath);
    QVERIFY(pattern.open(QIODevice::WriteOnly));
    QCOMPARE(pattern.write(fixture), fixture.size());
    pattern.close();
    QVERIFY(QDir(directory.path()).mkpath(QStringLiteral("state/settings/Seamly2DTeam")));
    QFile settings(directory.filePath(QStringLiteral("state/settings/Seamly2DTeam/qt6_seamly2d.ini")));
    QVERIFY(settings.open(QIODevice::WriteOnly));
    settings.write("[configuration]\nshowWelcome=true\nlocale=en_US\nautosave=false\n");
    settings.close();
    const QString path = directory.filePath(QStringLiteral("app.sock"));
    auto environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("SEAMLY_DATA_DIR"), directory.filePath(QStringLiteral("state")));
    environment.insert(QStringLiteral("SEAMLY_MCP_SOCKET"), path);
    environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("offscreen"));
    environment.insert(QStringLiteral("QT_LOGGING_RULES"), QStringLiteral("*.warning=false"));
#ifdef Q_OS_MAC
    // Deployed bundles usually contain only Cocoa. Use this test build's Qt
    // installation for the offscreen GUI process, without changing the native
    // MCP process's environment (so its bundled framework paths are tested).
    if (!environment.contains(QStringLiteral("DYLD_FRAMEWORK_PATH")))
    {
        environment.insert(QStringLiteral("DYLD_FRAMEWORK_PATH"), QLibraryInfo::path(QLibraryInfo::LibrariesPath));
    }
    if (!environment.contains(QStringLiteral("QT_QPA_PLATFORM_PLUGIN_PATH")))
    {
        environment.insert(QStringLiteral("QT_QPA_PLATFORM_PLUGIN_PATH"),
                           QLibraryInfo::path(QLibraryInfo::PluginsPath) + QStringLiteral("/platforms"));
    }
#endif
    Client application;
    application.process.setProcessEnvironment(environment);
    application.process.start(app, {patternPath});
    QVERIFY(application.process.waitForStarted(3000));
    QTRY_VERIFY_WITH_TIMEOUT(QFile::exists(path) || application.process.state() == QProcess::NotRunning, 15000);
    QVERIFY2(QFile::exists(path), application.process.readAllStandardError().constData());
    Client client;
    QVERIFY(client.start(path));
    QVERIFY(client.initialize());
    const auto data = [](const QJsonObject &reply) { return reply.value("structuredContent").toObject(); };
    auto summary = data(client.call(QStringLiteral("get_pattern_summary")));
    QCOMPARE(summary.value("active_draft_block").toString(), QStringLiteral("MCP test"));
    QCOMPARE(summary.value("unit").toString(), QStringLiteral("cm"));
    QCOMPARE(summary.value("object_count").toInt(), 1);
    const auto original = data(client.call(QStringLiteral("list_objects"))).value("objects").toArray();
    QCOMPARE(original.at(0).toObject().value("x").toDouble(), 10.0);
    QCOMPARE(original.at(0).toObject().value("y").toDouble(), 20.0);
    QCOMPARE(data(client.call(QStringLiteral("get_selection"))).value("selection").toArray(), QJsonArray{});
    QJsonObject arguments{{"state_token", summary.value("state_token")}, {"name", "McpB"},
                          {"base_point_id", 1}, {"length", "#testLength"}, {"angle", "90"}};
    const QList<QJsonObject> invalid{
        {{"length", "undefined_name"}}, {{"length", "-1"}}, {{"length", "1/0"}}, {{"name", "A"}}, {{"base_point_id", 9999}}
    };
    for (const auto &change : invalid)
    {
        auto badArguments = arguments;
        for (auto entry = change.begin(); entry != change.end(); ++entry)
        {
            badArguments.insert(entry.key(), entry.value());
        }
        QCOMPARE(client.call(QStringLiteral("create_point"), badArguments).value("isError"), QJsonValue(true));
        QCOMPARE(data(client.call(QStringLiteral("get_pattern_summary"))).value("state_token"), summary.value("state_token"));
    }
    const auto createdReply = client.call(QStringLiteral("create_point"), arguments);
    QCOMPARE(createdReply.value("isError"), QJsonValue(false));
    QCOMPARE(data(client.call("get_pattern_summary")).value("modified"), QJsonValue(true));
    const auto created = data(createdReply);
    const auto point = created.value("object").toObject();
    QCOMPARE(point.value("name").toString(), QStringLiteral("McpB"));
    QCOMPARE(point.value("x").toDouble(), 10.0);
    QCOMPARE(point.value("y").toDouble(), 10.0);
    QCOMPARE(data(client.call(QStringLiteral("get_pattern_summary"))).value("object_count").toInt(), 2);
    arguments.insert("name", "McpC");
    QCOMPARE(client.call(QStringLiteral("create_point"), arguments).value("isError"), QJsonValue(true));
    QCOMPARE(client.call(QStringLiteral("undo"), {{"state_token", summary.value("state_token")}}).value("isError"), QJsonValue(true));
    QCOMPARE(client.call(QStringLiteral("undo"), {{"state_token", created.value("state_token")}}).value("isError"), QJsonValue(false));
    const auto after = data(client.call(QStringLiteral("get_pattern_summary")));
    QCOMPARE(after.value("object_count").toInt(), 1);
    QVERIFY(after.value("state_token") != summary.value("state_token"));
    QCOMPARE(data(client.call(QStringLiteral("list_objects"))).value("objects").toArray(), original);

    auto currentToken = [&]() { return data(client.call("get_pattern_summary")).value("state_token"); };
    auto edit = [&](const QString &name, QJsonObject args = {}) { args.insert("state_token", currentToken()); return client.call(name, args); };
    auto expectSuccess = [](const QJsonObject &reply) { return reply.value("isError") == QJsonValue(false); };
    QVERIFY(expectSuccess(client.call("evaluate_formula", {{"formula", "#testLength*2"}})));
    QCOMPARE(data(client.call("evaluate_formula", {{"formula", "#testLength*2"}})).value("value").toDouble(), 20.0);
    const auto variableReply = edit("set_variable", {{"name", "#extra"}, {"formula", "#testLength/2"}});
    QVERIFY2(expectSuccess(variableReply), qPrintable(QString::fromUtf8(QJsonDocument(variableReply).toJson())));
    QCOMPARE(data(client.call("evaluate_formula", {{"formula", "#extra"}})).value("value").toDouble(), 5.0);
    const auto beforeInvalid = currentToken();
    QCOMPARE(edit("set_variable", {{"name", "#extra"}, {"formula", "missing_variable"}}).value("isError"), QJsonValue(true));
    QCOMPARE(currentToken(), beforeInvalid);
    QVERIFY(expectSuccess(edit("undo")));
    QCOMPARE(client.call("evaluate_formula", {{"formula", "#extra"}}).value("isError"), QJsonValue(true));
    QVERIFY(expectSuccess(edit("redo")));
    auto b = edit("create_point", {{"name", "B"}, {"base_point_id", 1}, {"length", "#extra"}, {"angle", "0"}});
    QVERIFY2(expectSuccess(b), qPrintable(QString::fromUtf8(QJsonDocument(b).toJson())));
    const int bId = data(b).value("object").toObject().value("id").toInt();
    QVERIFY(expectSuccess(edit("create_line", {{"first_point_id", 1}, {"second_point_id", bId}})));
    auto curve = edit("create_curve", {{"first_point_id", 1}, {"second_point_id", bId}, {"angle1", "0"}, {"angle2", "180"}, {"length1", "2"}, {"length2", "2"}});
    QVERIFY2(expectSuccess(curve), qPrintable(QString::fromUtf8(QJsonDocument(curve).toJson())));
    auto arc = edit("create_arc", {{"center_id", 1}, {"radius", "3"}, {"angle1", "0"}, {"angle2", "90"}});
    QVERIFY2(expectSuccess(arc), qPrintable(QString::fromUtf8(QJsonDocument(arc).toJson())));
    QVERIFY(expectSuccess(client.call("get_object", {{"id", bId}})));
    QCOMPARE(edit("delete_variable", {{"name", "#extra"}}).value("isError"), QJsonValue(true));
    QCOMPARE(edit("delete_object", {{"id", 1}}).value("isError"), QJsonValue(true));
    QVERIFY(expectSuccess(edit("update_object", {{"id", bId}, {"attributes", QJsonObject{{"length", "8"}}}})));
    QCOMPARE(data(client.call("get_object", {{"id", bId}})).value("geometry").toObject().value("x").toDouble(), 18.0);
    QVERIFY(expectSuccess(edit("delete_variable", {{"name", "#extra"}})));
    auto generic = edit("create_tool", {{"section", "calculation"}, {"xml", "<point type=\"endLine\" name=\"GenericPoint\" basePoint=\"1\" length=\"3\" angle=\"180\" lineType=\"solidLine\" lineWeight=\"0.35\" lineColor=\"black\" mx=\"5\" my=\"10\"/>"}});
    QVERIFY2(expectSuccess(generic), qPrintable(QString::fromUtf8(QJsonDocument(generic).toJson())));
    int genericId = 0;
    for (const auto &object : data(client.call("list_objects")).value("objects").toArray()) if (object.toObject().value("name") == QJsonValue("GenericPoint")) genericId = object.toObject().value("id").toInt();
    QVERIFY(genericId > 0);
    QVERIFY(expectSuccess(edit("delete_object", {{"id", genericId}})));
    QVERIFY(expectSuccess(edit("add_draft_block", {{"name", "Second"}, {"point_name", "C"}, {"x", 1}, {"y", 2}})));
    QCOMPARE(data(client.call("get_pattern_summary")).value("active_draft_block"), QJsonValue("Second"));
    QVERIFY(expectSuccess(edit("rename_draft_block", {{"name", "Second"}, {"new_name", "Renamed"}})));
    QVERIFY(expectSuccess(edit("set_active_draft_block", {{"name", "MCP test"}})));
    QVERIFY(expectSuccess(edit("set_selection", {{"ids", QJsonArray{bId}}})));
    QVERIFY(!data(client.call("get_selection")).value("selection").toArray().isEmpty());
    QVERIFY(expectSuccess(edit("set_view", {{"fit", true}, {"scale", 1.0}})));
    const auto preview = data(client.call("get_preview"));
    QVERIFY(QByteArray::fromBase64(preview.value("svg_base64").toString().toLatin1()).contains("<svg"));
    for (const auto &format : {"svg", "png", "pdf"})
    {
        const QString out = directory.filePath(QString("drawing.") + format);
        auto exported = edit("export_pattern", {{"path", out}, {"format", format}});
        QVERIFY2(expectSuccess(exported), qPrintable(QString::fromUtf8(QJsonDocument(exported).toJson())));
        QVERIFY(QFileInfo(out).size() > 20);
        QCOMPARE(edit("export_pattern", {{"path", out}, {"format", format}}).value("isError"), QJsonValue(true));
    }
    auto xml = data(client.call("get_pattern_xml")).value("xml").toString();
    const auto unchanged = currentToken();
    QCOMPARE(edit("apply_pattern_xml", {{"xml", "<!DOCTYPE pattern><pattern/>"}}).value("isError"), QJsonValue(true));
    QCOMPARE(currentToken(), unchanged);
    xml.replace("<notes/>", "<notes>MCP batch</notes>");
    QVERIFY(expectSuccess(edit("apply_pattern_xml", {{"xml", xml}, {"label", "MCP notes"}})));
    QVERIFY(data(client.call("get_export_formats")).value("formats").toArray().size() > 10);
    QCOMPARE(data(client.call("list_pieces")).value("pieces").toArray().size(), 0);
    QCOMPARE(edit("export_layout", {{"output_directory", directory.filePath("layout")}, {"basename", "test"}, {"format", 0}}).value("isError"), QJsonValue(true));
    const int finalCount = data(client.call("get_pattern_summary")).value("object_count").toInt();
    const int appliedCount = data(client.call("get_history")).value("index").toInt();
    for (int i = 0; i < appliedCount; ++i) QVERIFY(expectSuccess(edit("undo")));
    QCOMPARE(data(client.call("get_pattern_summary")).value("object_count").toInt(), 1);
    for (int i = 0; i < appliedCount; ++i) QVERIFY(expectSuccess(edit("redo")));
    QCOMPARE(data(client.call("get_pattern_summary")).value("object_count").toInt(), finalCount);
    const auto saved = directory.filePath("saved.sm2d");
    QVERIFY(expectSuccess(edit("save_pattern", {{"path", saved}})));
    QVERIFY(QFileInfo::exists(saved));
    QCOMPARE(data(client.call("get_pattern_summary")).value("modified"), QJsonValue(false));
    QVERIFY(expectSuccess(edit("close_pattern")));
    const auto reopened = edit("open_pattern", {{"path", saved}});
    QVERIFY2(expectSuccess(reopened), qPrintable(QString::fromUtf8(QJsonDocument(reopened).toJson())));
    QVERIFY(data(client.call("get_pattern_summary")).value("draft_blocks").toArray().contains("Renamed"));
    auto fresh = edit("new_pattern", {{"draft_block", "Fresh"}, {"unit", "mm"}});
    QVERIFY2(expectSuccess(fresh), qPrintable(QString::fromUtf8(QJsonDocument(fresh).toJson())));
    QCOMPARE(data(fresh).value("unit"), QJsonValue("mm"));
    QCOMPARE(data(fresh).value("object_count").toInt(), 1);
    QCOMPARE(edit("close_pattern").value("isError"), QJsonValue(true));
    QVERIFY(expectSuccess(edit("close_pattern", {{"discard_changes", true}})));

    const QString sample = QFINDTESTDATA("../CollectionTest/share/all_tools_pattern/alltools_pattern.sm2d");
    QVERIFY(!sample.isEmpty());
    const auto sampleReply = edit("open_pattern", {{"path", sample}});
    QVERIFY2(expectSuccess(sampleReply), qPrintable(QString::fromUtf8(QJsonDocument(sampleReply).toJson())));
    QVERIFY(data(client.call("list_pieces")).value("pieces").toArray().size() > 0);
    const auto layout = edit("export_layout", {{"output_directory", directory.filePath("pieces-svg")}, {"basename", "shirt"}, {"format", 0}, {"pieces_only", true}});
    QVERIFY2(expectSuccess(layout), qPrintable(QString::fromUtf8(QJsonDocument(layout).toJson())));
    QVERIFY(!data(layout).value("files").toArray().isEmpty());
    const auto dxf = edit("export_layout", {{"output_directory", directory.filePath("pieces-dxf")}, {"basename", "shirt"}, {"format", 18}, {"pieces_only", true}});
    QVERIFY2(expectSuccess(dxf), qPrintable(QString::fromUtf8(QJsonDocument(dxf).toJson())));
    QVERIFY(!data(dxf).value("files").toArray().isEmpty());
    const auto nested = edit("export_layout", {{"output_directory", directory.filePath("nested-svg")}, {"basename", "layout"}, {"format", 0},
        {"options", QJsonObject{{"pagew", "2000"}, {"pageh", "2000"}, {"pageunits", "mm"}, {"ignoremargins", "true"}, {"crop", "true"}}}});
    QVERIFY2(expectSuccess(nested), qPrintable(QString::fromUtf8(QJsonDocument(nested).toJson())));
    QVERIFY(!data(nested).value("files").toArray().isEmpty());
    QVERIFY(pattern.open(QIODevice::ReadOnly));
    QCOMPARE(pattern.readAll(), fixture);

}

QTEST_GUILESS_MAIN(McpTest)
#include "tst_mcp.moc"

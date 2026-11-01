// SPDX-License-Identifier: GPL-3.0-or-later
#include "mcp_server.h"
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalSocket>
#include <QStringList>
#include <cmath>
#include <stdexcept>
#include <utility>

namespace
{
constexpr qint64 maxMessageBytes = 8 * 1024 * 1024;
constexpr qint64 maxReplyBytes = 16 * 1024 * 1024;
constexpr int bridgeTimeoutMs = 60000;
const QString modernVersion = QStringLiteral("2026-07-28");
const QString skillExtension = QStringLiteral("io.modelcontextprotocol/skills");
const QString skillUri = QStringLiteral("skill://seamly2d-pattern/SKILL.md");
const QString instructions = QStringLiteral("Control the explicitly enabled local Seamly2D app. Read summary before edits and pass its latest state_token. Never blindly retry an edit after timeout: read state. Undo/redo includes manual edits. Use dedicated tools for common geometry; use validated create_tool/apply_pattern_xml for other persisted pattern features. Read the pattern skill for advanced edits. Save/export only to intended paths.");
struct RpcError { int code; QString message; QJsonObject data; };
struct ApplicationError : std::runtime_error { using std::runtime_error::runtime_error; };
QJsonObject serverInfo() { return {{"name", "seamly2d"}, {"version", "0.4.0"}}; }
QJsonArray versions() { return {"2026-07-28", "2025-11-25", "2025-06-18", "2025-03-26", "2024-11-05"}; }
QByteArray asset(const QString &name)
{
    QFile file(QStringLiteral(":/mcp/") + name);
    if (!file.open(QIODevice::ReadOnly)) throw std::runtime_error("Missing bundled MCP resource.");
    return file.readAll();
}
QJsonObject capabilities()
{
    return {{"tools", QJsonObject{}}, {"resources", QJsonObject{}}, {"prompts", QJsonObject{}},
            {"extensions", QJsonObject{{skillExtension, QJsonObject{}}}}};
}
QJsonObject frontmatter()
{
    return {{"name", "seamly2d-pattern"},
            {"description", "Create, inspect, edit and export sewing patterns in the local Seamly2D desktop app using its MCP tools. Use for cartamodelli and parametric garment drafting, rather than ordinary image drawing."}};
}
QJsonObject skillEntry()
{
    QJsonArray files;
    const QList<QPair<QString, QString>> names{{skillUri, "SKILL.md"},
        {"skill://seamly2d-pattern/references/advanced.md", "advanced.md"}};
    for (const auto &entry : names)
    {
        const auto bytes = asset(entry.second);
        files.append(QJsonObject{{"uri", entry.first}, {"size", static_cast<double>(bytes.size())},
            {"digest", "sha256:" + QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex())}});
    }
    return {{"uri", skillUri}, {"frontmatter", frontmatter()}, {"resources", files}};
}
void validateValue(const QJsonObject &schema, const QJsonValue &value, const QString &path, int depth = 0)
{
    if (depth > 12) throw RpcError{-32602, "Arguments exceed maximum nesting depth.", {}};
    bool valid = true;
    const auto type = schema.value("type").toString();
    if (type == "object")
    {
        valid = value.isObject();
        if (valid)
        {
            const auto obj = value.toObject();
            const auto props = schema.value("properties").toObject();
            valid = obj.size() <= schema.value("maxProperties").toInt(100);
            for (const auto &required : schema.value("required").toArray())
                if (!obj.contains(required.toString())) throw RpcError{-32602, "Missing argument: " + path + required.toString(), {}};
            for (auto it = obj.begin(); it != obj.end(); ++it)
            {
                if (props.contains(it.key())) validateValue(props.value(it.key()).toObject(), it.value(), path + it.key() + ".", depth + 1);
                else if (schema.value("additionalProperties").isObject()) validateValue(schema.value("additionalProperties").toObject(), it.value(), path + it.key(), depth + 1);
                else if (schema.value("additionalProperties") == QJsonValue(false)) throw RpcError{-32602, "Unknown argument: " + path + it.key(), {}};
            }
        }
    }
    else if (type == "array")
    {
        valid = value.isArray() && value.toArray().size() <= schema.value("maxItems").toInt(1000);
        if (valid) for (const auto &item : value.toArray()) validateValue(schema.value("items").toObject(), item, path, depth + 1);
    }
    else if (type == "string") valid = value.isString() && value.toString().size() >= schema.value("minLength").toInt()
        && value.toString().size() <= schema.value("maxLength").toInt(4 * 1024 * 1024);
    else if (type == "boolean") valid = value.isBool();
    else if (type == "number" || type == "integer")
    {
        const double n = value.toDouble();
        valid = value.isDouble() && std::isfinite(n) && n >= schema.value("minimum").toDouble(-1e12)
            && n <= schema.value("maximum").toDouble(1e12) && (type != "integer" || std::floor(n) == n);
    }
    if (schema.contains("enum") && !schema.value("enum").toArray().contains(value)) valid = false;
    if (!valid) throw RpcError{-32602, "Invalid argument: " + path, {}};
}
QJsonObject toolResult(const QString &text, bool isError)
{
    return {{"content", QJsonArray{QJsonObject{{"type", "text"}, {"text", text}}}}, {"isError", isError}};
}
void bridgeRequire(bool condition, const QString &message)
{
    if (!condition) throw std::runtime_error(message.toStdString());
}
QJsonObject cached(QJsonObject result)
{
    result.insert("ttlMs", 300000); result.insert("cacheScope", "public"); return result;
}
}

McpServer::McpServer(const QString &socketPath, BridgeCall bridge)
    : m_socketPath(socketPath), m_bridge(std::move(bridge)) {}
QJsonArray McpServer::tools() { return QJsonDocument::fromJson(asset("tools.json")).array(); }
QJsonObject McpServer::error(const QJsonValue &id, int code, const QString &message)
{
    return {{"jsonrpc", "2.0"}, {"id", id.isUndefined() ? QJsonValue(QJsonValue::Null) : id},
            {"error", QJsonObject{{"code", code}, {"message", message}}}};
}
QJsonValue McpServer::handle(const QJsonValue &request)
{
    const auto message = request.toObject();
    if (!request.isObject() || message.value("jsonrpc") != QJsonValue("2.0") || !message.value("method").isString())
        return error(QJsonValue::Null, -32600, "Invalid JSON-RPC request.");
    const bool notification = !message.contains("id");
    const auto id = message.value("id");
    if (!notification && !(id.isString() || (id.isDouble() && std::isfinite(id.toDouble()) && std::floor(id.toDouble()) == id.toDouble())))
        return error(QJsonValue::Null, -32600, "Request id must be a string or integer.");
    const auto method = message.value("method").toString();
    try
    {
        if (message.contains("params") && !message.value("params").isObject()) throw RpcError{-32602, "params must be an object.", {}};
        const auto params = message.value("params").toObject();
        const auto meta = params.value("_meta").toObject();
        const auto versionValue = meta.value("io.modelcontextprotocol/protocolVersion");
        const bool modern = !versionValue.isUndefined();
        QJsonObject clientCapabilities = modern ? meta.value("io.modelcontextprotocol/clientCapabilities").toObject() : m_legacyCapabilities;
        if (modern)
        {
            if (!versionValue.isString() || !meta.value("io.modelcontextprotocol/clientCapabilities").isObject())
                throw RpcError{-32602, "Every modern request requires protocolVersion and clientCapabilities in params._meta.", {}};
            if (versionValue.toString() != modernVersion)
                throw RpcError{-32022, "Unsupported protocol version", {{"supported", versions()}, {"requested", versionValue}}};
        }
        if (notification)
        {
            if (!modern && method == "notifications/initialized" && m_phase == Phase::Initializing) m_phase = Phase::Ready;
            return QJsonValue(QJsonValue::Undefined);
        }
        const auto extensions = clientCapabilities.value("extensions").toObject();
        const auto requireExtension = [&](const QString &extension)
        {
            if (!extensions.value(extension).isObject()) throw RpcError{-32021, "Client extension not declared.",
                {{"requiredCapabilities", QJsonObject{{"extensions", QJsonObject{{extension, QJsonObject{}}}}}}}};
        };
        QJsonObject result;
        if (method == "server/discover")
        {
            if (!modern) throw RpcError{-32602, "Discovery requires modern per-request metadata.", {}};
            result = cached({{"supportedVersions", versions()}, {"capabilities", capabilities()}, {"instructions", instructions}});
        }
        else if (!modern && method == "initialize")
        {
            if (m_phase != Phase::New) throw RpcError{-32600, "Already initialized.", {}};
            if (!params.value("protocolVersion").isString() || !params.value("capabilities").isObject() || !params.value("clientInfo").isObject())
                throw RpcError{-32602, "Expected protocolVersion, capabilities and clientInfo.", {}};
            const auto requested = params.value("protocolVersion").toString();
            if (requested != modernVersion && versions().contains(requested)) m_protocol = requested;
            m_legacyCapabilities = params.value("capabilities").toObject();
            auto legacyCapabilities = capabilities();
            auto legacyExtensions = legacyCapabilities.value("extensions").toObject();
            legacyExtensions.remove(skillExtension);
            legacyCapabilities.insert("extensions", legacyExtensions);
            result = {{"protocolVersion", m_protocol}, {"capabilities", legacyCapabilities}, {"serverInfo", serverInfo()}, {"instructions", instructions}};
            m_phase = Phase::Initializing;
        }
        else if (!modern && method == "ping") {}
        else if (!modern && m_phase != Phase::Ready) throw RpcError{-32600, "Complete initialization first.", {}};
        else if (method == "tools/list")
        {
            if (params.contains("cursor") && params.value("cursor") != QJsonValue("")) throw RpcError{-32602, "No additional catalog pages.", {}};
            result = cached({{"tools", tools()}});
        }
        else if (method == "skills/list" || method == "skills/get")
        {
            if (!modern) throw RpcError{-32601, "Skills over MCP requires the modern protocol; use bundled plugin skills with legacy clients.", {}};
            requireExtension(skillExtension);
            if (method == "skills/get")
            {
                if (params.value("uri") != QJsonValue(skillUri)) throw RpcError{-32602, "Unknown skill URI.", {}};
                result = cached({{"skill", skillEntry()}});
            }
            else
            {
                if (params.contains("cursor")) throw RpcError{-32602, "No additional skill pages.", {}};
                result = cached({{"skills", QJsonArray{skillEntry()}}});
            }
        }
        else if (method == "resources/list")
        {
            QJsonArray resources{QJsonObject{{"uri", skillUri}, {"name", "seamly2d-pattern"}, {"mimeType", "text/markdown"}},
                QJsonObject{{"uri", "skill://seamly2d-pattern/references/advanced.md"}, {"name", "advanced.md"}, {"mimeType", "text/markdown"}}};
            result = cached({{"resources", resources}});
        }
        else if (method == "resources/templates/list") result = cached({{"resourceTemplates", QJsonArray{}}});
        else if (method == "resources/read")
        {
            const auto uri = params.value("uri").toString();
            QString file, mime = "text/markdown";
            if (uri == skillUri) file = "SKILL.md";
            else if (uri == "skill://seamly2d-pattern/references/advanced.md") file = "advanced.md";
            else throw RpcError{-32602, "Unknown resource URI.", {}};
            QJsonObject content{{"uri", uri}, {"mimeType", mime}, {"text", QString::fromUtf8(asset(file))}};
            result = cached({{"contents", QJsonArray{content}}});
        }
        else if (method == "prompts/list") result = cached({{"prompts", QJsonArray{QJsonObject{{"name", "seamly2d-pattern"}, {"description", "Work on the open sewing pattern using verified MCP operations."}}}}});
        else if (method == "prompts/get")
        {
            if (params.value("name") != QJsonValue("seamly2d-pattern")) throw RpcError{-32602, "Unknown prompt.", {}};
            result = {{"messages", QJsonArray{QJsonObject{{"role", "user"}, {"content", QJsonObject{{"type", "text"}, {"text", QString::fromUtf8(asset("SKILL.md"))}}}}}}};
        }
        else if (method == "tools/call")
        {
            QJsonObject definition;
            for (const auto &entry : tools()) if (entry.toObject().value("name") == params.value("name")) { definition = entry.toObject(); break; }
            if (definition.isEmpty()) throw RpcError{-32602, "Unknown tool.", {}};
            const auto arguments = params.contains("arguments") ? params.value("arguments") : QJsonValue(QJsonObject{});
            validateValue(definition.value("inputSchema").toObject(), arguments, "");
            try
            {
                const auto name = params.value("name").toString();
                QJsonObject data;
                if (name == "get_pattern_schema") data = {{"version", "0.7.6"}, {"xsd", QString::fromUtf8(asset("pattern.xsd"))}};
                else data = m_bridge(m_socketPath, name, arguments.toObject());
                result = toolResult(QString::fromUtf8(QJsonDocument(data).toJson(QJsonDocument::Compact)), false);
                if (modern || m_protocol >= "2025-06-18") result.insert("structuredContent", data);
            }
            catch (const std::exception &exception) { result = toolResult(QString::fromUtf8(exception.what()), true); }
        }
        else throw RpcError{-32601, "Method not found.", {}};
        if (modern)
        {
            result.insert("resultType", "complete");
            auto resultMeta = result.value("_meta").toObject(); resultMeta.insert("io.modelcontextprotocol/serverInfo", serverInfo()); result.insert("_meta", resultMeta);
        }
        return QJsonObject{{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
    }
    catch (const RpcError &exception)
    {
        if (notification) return QJsonValue(QJsonValue::Undefined);
        auto reply = error(id, exception.code, exception.message);
        if (!exception.data.isEmpty()) { auto detail = reply.value("error").toObject(); detail.insert("data", exception.data); reply.insert("error", detail); }
        return reply;
    }
}

QJsonObject McpServer::callBridge(const QString &socketPath, const QString &method, const QJsonObject &arguments)
{
    try
    {
        QElapsedTimer timer;
        timer.start();
        const auto remaining = [&timer]() { return qMax(0, bridgeTimeoutMs - static_cast<int>(timer.elapsed())); };
        QLocalSocket socket;
        socket.setReadBufferSize(maxReplyBytes + 1);
        socket.connectToServer(socketPath);
        bridgeRequire(socket.waitForConnected(remaining()), socket.errorString());
        const auto request = QJsonDocument(QJsonObject{{"id", 1}, {"method", method}, {"params", arguments}})
                                 .toJson(QJsonDocument::Compact) + '\n';
        bridgeRequire(socket.write(request) == request.size(), socket.errorString());
        while (socket.bytesToWrite() > 0)
        {
            bridgeRequire(socket.waitForBytesWritten(remaining()), socket.errorString());
        }
        while (!socket.canReadLine())
        {
            bridgeRequire(socket.bytesAvailable() <= maxReplyBytes, QStringLiteral("Oversized bridge reply."));
            const bool ready = socket.waitForReadyRead(remaining());
            bridgeRequire(ready || socket.canReadLine(), QStringLiteral("Incomplete bridge reply: ") + socket.errorString());
        }
        bridgeRequire(socket.bytesAvailable() <= maxReplyBytes, QStringLiteral("Oversized bridge reply."));
        const auto line = socket.readLine(maxReplyBytes + 1);
        QJsonParseError parseError;
        const auto document = QJsonDocument::fromJson(line, &parseError);
        bridgeRequire(parseError.error == QJsonParseError::NoError && document.isObject(), QStringLiteral("Invalid bridge JSON."));
        const auto reply = document.object();
        bridgeRequire(reply.value("id") == QJsonValue(1), QStringLiteral("Invalid bridge reply id."));
        if (reply.contains("error")) throw ApplicationError(reply.value("error").toString(QStringLiteral("Application error.")).toStdString());
        bridgeRequire(reply.value("result").isObject(), QStringLiteral("Bridge result must be an object."));
        return reply.value("result").toObject();
    }
    catch (const ApplicationError &) { throw; }
    catch (const std::exception &exception)
    {
        throw std::runtime_error((QStringLiteral("Seamly2D bridge: ") + QString::fromUtf8(exception.what())
            + QStringLiteral(". Ensure the app is running with the same SEAMLY_MCP_SOCKET. The request was not retried. After a timeout/disconnect, read the current state before issuing another edit.")).toStdString());
    }
}

int serveMcp(McpServer &server, QFile &input, QFile &output)
{
    while (true)
    {
        const auto line = input.readLine(maxMessageBytes + 2);
        if (line.isEmpty())
        {
            return input.error() == QFileDevice::NoError ? 0 : 1;
        }
        const bool oversized = line.size() > maxMessageBytes;
        QJsonValue response;
        if (oversized)
        {
            response = McpServer::error(QJsonValue::Null, -32700, QStringLiteral("Message too large; closing transport."));
        }
        else
        {
            QJsonParseError parseError;
            const auto document = QJsonDocument::fromJson(line, &parseError);
            if (parseError.error != QJsonParseError::NoError)
            {
                // QJsonDocument only accepts objects/arrays at the root. A
                // syntactically valid scalar is an invalid RPC request instead.
                const auto wrapped = QJsonDocument::fromJson('[' + line + ']');
                response = wrapped.isArray() && wrapped.array().size() == 1
                    ? server.handle(wrapped.array().first())
                    : QJsonValue(McpServer::error(QJsonValue::Null, -32700, QStringLiteral("Invalid JSON.")));
            }
            else
            {
                response = server.handle(document.isObject() ? QJsonValue(document.object()) : QJsonValue(document.array()));
            }
        }
        if (!response.isUndefined())
        {
            const auto bytes = QJsonDocument(response.toObject()).toJson(QJsonDocument::Compact) + '\n';
            if (output.write(bytes) != bytes.size() || !output.flush())
            {
                return 1;
            }
        }
        if (oversized)
        {
            return 0;
        }
    }
}

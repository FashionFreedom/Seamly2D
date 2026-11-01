// SPDX-License-Identifier: GPL-3.0-or-later
#include "mcp_bridge.h"
#include "mcp_measurements.h"

#include "../mainwindow.h"
#include "../xml/vpattern.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/calculator.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/vabstracttool.h"
#include "application_2d.h"
#include "../vmisc/vsettings.h"

#include <QCryptographicHash>
#include <QBuffer>
#include <QComboBox>
#include <QDomDocument>
#include <QPainter>
#include <QPolygonF>
#include <QPdfWriter>
#include <QProcess>
#include <QProcessEnvironment>
#include <QSaveFile>
#include <QSvgGenerator>
#include "../vwidgets/export_format_combobox.h"
#include <QTemporaryDir>
#include <QUndoCommand>
#include "../vpatterndb/variables/custom_variable.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/exception/vexception.h"
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QLocalSocket>
#include <QRegularExpression>
#include <QScopedValueRollback>
#include <QSet>
#include <QTimer>
#include <QUndoStack>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#ifdef Q_OS_UNIX
#include <unistd.h>
#endif

namespace
{
constexpr qint64 maxRequestBytes = 8 * 1024 * 1024;

void require(bool condition, const char *message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

QString objectType(GOType type)
{
    switch (type)
    {
        case GOType::Point: return QStringLiteral("point");
        case GOType::Arc: return QStringLiteral("arc");
        case GOType::EllipticalArc: return QStringLiteral("elliptical_arc");
        case GOType::Spline: return QStringLiteral("spline");
        case GOType::SplinePath: return QStringLiteral("spline_path");
        case GOType::CubicBezier: return QStringLiteral("cubic_bezier");
        case GOType::CubicBezierPath: return QStringLiteral("cubic_bezier_path");
        default: return QStringLiteral("unknown");
    }
}

QJsonObject objectJson(quint32 id, const QSharedPointer<VGObject> &object)
{
    QJsonObject result{{"id", static_cast<double>(id)}, {"name", object->name()},
                       {"type", objectType(object->getType())},
                       {"tool_id", static_cast<double>(object->getIdTool())}};
    if (object->getType() == GOType::Point)
    {
        const auto point = qSharedPointerDynamicCast<VPointF>(object);
        if (point)
        {
            result.insert("x", qApp->fromPixel(point->x()));
            result.insert("y", qApp->fromPixel(point->y()));
        }
    }
    return result;
}

int integerArgument(const QJsonObject &args, const QString &key, int fallback, int maximum)
{
    if (!args.contains(key))
    {
        return fallback;
    }
    const auto value = args.value(key);
    const double number = value.toDouble(-1);
    require(value.isDouble() && number >= 0 && number <= maximum && std::floor(number) == number,
            "Invalid integer argument.");
    return static_cast<int>(number);
}
QDomElement elementWithId(const QDomNode &parent, quint32 id)
{
    for (auto child = parent.firstChild(); !child.isNull(); child = child.nextSibling())
    {
        const auto element = child.toElement();
        if (!element.isNull() && element.hasAttribute("id") && element.attribute("id").toUInt() == id) return element;
        const auto found = elementWithId(child, id);
        if (!found.isNull()) return found;
    }
    return {};
}
quint32 freshId(const QDomNode &parent)
{
    quint32 maximum = 0;
    std::function<void(const QDomNode &)> visit = [&](const QDomNode &node)
    {
        const auto element = node.toElement();
        // Operation outputs use idObject; double-point outputs use point1/point2.
        // These IDs share the geometry namespace with ordinary tool IDs.
        for (const auto &attribute : {"id", "idObject", "point1", "point2"})
            maximum = qMax(maximum, element.attribute(attribute).toUInt());
        for (auto child = node.firstChild(); !child.isNull(); child = child.nextSibling()) visit(child);
    };
    visit(parent);
    require(maximum < 2147483647, "Pattern exhausted the supported ID range.");
    return maximum + 1;
}
QDomDocument parseXml(const QByteArray &xml)
{
    require(!xml.toUpper().contains("<!DOCTYPE") && !xml.toUpper().contains("<!ENTITY"), "DTD and entity declarations are not accepted.");
    QDomDocument document;
    require(static_cast<bool>(document.setContent(xml)), "Invalid XML.");
    return document;
}
QJsonObject attributesJson(const QDomElement &element)
{
    QJsonObject attributes;
    const auto map = element.attributes();
    for (int i = 0; i < map.size(); ++i) attributes.insert(map.item(i).nodeName(), map.item(i).nodeValue());
    return attributes;
}
class DocumentCommand final : public QUndoCommand
{
public:
    DocumentCommand(QByteArray before, QByteArray after, std::function<void(const QByteArray &)> restore, const QString &label)
        : QUndoCommand(label), m_before(std::move(before)), m_after(std::move(after)), m_restore(std::move(restore)) {}
    void undo() override { m_restore(m_before); }
    void redo() override { if (m_first) m_first = false; else m_restore(m_after); }
private:
    QByteArray m_before, m_after;
    std::function<void(const QByteArray &)> m_restore;
    bool m_first = true;
};

class PauseMeasurementWatch
{
public:
    explicit PauseMeasurementWatch(QFileSystemWatcher *watcher) : m_watcher(watcher)
    {
        if (m_watcher) { m_paths = m_watcher->files(); if (!m_paths.isEmpty()) m_watcher->removePaths(m_paths); }
    }
    ~PauseMeasurementWatch()
    {
        if (m_watcher) for (const auto &path : m_paths)
            if (QFileInfo::exists(path) && !m_watcher->files().contains(path)) m_watcher->addPath(path);
    }
private:
    QFileSystemWatcher *m_watcher;
    QStringList m_paths;
};

}

McpBridge::McpBridge(MainWindow *window)
    : QObject(window), m_window(window), m_session(QUuid::createUuid().toByteArray())
{
    connect(&m_server, &QLocalServer::newConnection, this, &McpBridge::acceptConnections);
    connect(qApp->getUndoStack(), &QUndoStack::indexChanged, this, [this](int) { ++m_revision; });
}

McpBridge::~McpBridge() { m_window = nullptr; stop(); }
void McpBridge::notifyStatus() { if (m_window) m_window->updateMcpStatus(); }
bool McpBridge::startDefault()
{
    const auto path = qEnvironmentVariable("SEAMLY_MCP_SOCKET", qApp->Seamly2DSettings()->value("mcp/socketPath",
        QDir::homePath() + "/.seamly2d/mcp/app.sock").toString());
    const auto directory = QFileInfo(path).absolutePath();
    if (!QFileInfo::exists(directory))
    {
        if (!QDir().mkpath(directory) || !QFile::setPermissions(directory, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner))
        {
            m_error = QStringLiteral("Could not create the private MCP socket directory."); notifyStatus(); return false;
        }
    }
    return start(path);
}
void McpBridge::stop()
{
    m_server.close();
    m_clients.clear();
    for (auto *socket : m_server.findChildren<QLocalSocket *>()) socket->abort();
    m_error.clear(); notifyStatus();
}

bool McpBridge::start(const QString &socketPath)
{
    if (m_server.isListening()) return true;
    m_error.clear();
#ifndef Q_OS_UNIX
    Q_UNUSED(socketPath)
    m_error = QStringLiteral("This first version supports macOS and Linux local sockets.");
    notifyStatus();
    return false;
#else
    const QFileInfo directory(QFileInfo(socketPath).absolutePath());
    const auto publicPermissions = QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup
                                 | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
    if (!QDir::isAbsolutePath(socketPath) || !directory.isDir() || directory.isSymLink()
        || directory.ownerId() != static_cast<uint>(getuid()) || (directory.permissions() & publicPermissions))
    {
        m_error = QStringLiteral("SEAMLY_MCP_SOCKET requires an absolute path inside an owned, private directory (0700).");
        notifyStatus();
        return false;
    }
    m_server.setSocketOptions(QLocalServer::UserAccessOption);
    m_server.setMaxPendingConnections(4);
    // Never remove an existing socket: it may belong to another running instance.
    if (!m_server.listen(socketPath))
    {
        m_error = m_server.errorString();
        notifyStatus();
        return false;
    }
    qInfo() << "Local MCP bridge listening at" << socketPath;
    qApp->Seamly2DSettings()->setValue("mcp/socketPath", socketPath);
    notifyStatus();
    return true;
#endif
}

QString McpBridge::errorString() const
{
    return m_error;
}

void McpBridge::acceptConnections()
{
    while (m_server.hasPendingConnections())
    {
        auto *socket = m_server.nextPendingConnection();
        if (m_connections >= 8)
        {
            socket->abort();
            socket->deleteLater();
            continue;
        }
        ++m_connections;
        socket->setReadBufferSize(maxRequestBytes + 1);
        auto *timer = new QTimer(socket);
        timer->setSingleShot(true);
        timer->start(5000);
        connect(timer, &QTimer::timeout, socket, &QLocalSocket::abort);
        connect(socket, &QLocalSocket::disconnected, this, [this, socket]()
        {
            --m_connections;
            if (m_clients.remove(socket)) notifyStatus();
            socket->deleteLater();
        });
        auto handled = std::make_shared<bool>(false);
        const auto receive = [this, socket, timer, handled]()
        {
            if (*handled)
            {
                // A presence channel stays open, but never accepts more requests.
                if (m_clients.contains(socket) && socket->bytesAvailable()) socket->abort();
                return;
            }
            if (socket->bytesAvailable() > maxRequestBytes)
            {
                socket->abort();
                return;
            }
            if (!socket->canReadLine())
            {
                return;
            }
            *handled = true;
            timer->stop();
            const QByteArray line = socket->readLine();
            QJsonParseError parseError;
            const auto request = QJsonDocument::fromJson(line, &parseError);
            QJsonObject response;
            try
            {
                require(parseError.error == QJsonParseError::NoError && request.isObject(), "Invalid JSON request.");
                const auto message = request.object();
                response.insert("id", message.value("id"));
                require(message.value("method").isString() && message.value("params").isObject(),
                        "Expected method and params.");
                const auto method = message.value("method").toString();
                if (method == "_mcp_session")
                {
                    require(m_clients.size() < 4, "Too many MCP clients.");
                    m_clients.insert(socket); notifyStatus();
                    response.insert("result", QJsonObject{{"session", true}});
                }
                else if (method == "_mcp_status")
                    response.insert("result", QJsonObject{{"listening", m_server.isListening()}, {"clients", m_clients.size()}});
                else
                {
                    require(!m_executing, "Another bridge operation is in progress.");
                    QScopedValueRollback<bool> executing(m_executing, true);
                    response.insert("result", execute(method, message.value("params").toObject()));
                }
            }
            catch (const VException &error)
            {
                response.insert("error", error.ErrorMessage() + QStringLiteral("\n") + error.DetailedInformation());
            }
            catch (const qmu::QmuParserError &error)
            {
                response.insert("error", error.GetMsg());
            }
            catch (const std::exception &error)
            {
                response.insert("error", QString::fromUtf8(error.what()));
            }
            catch (...)
            {
                response.insert("error", QStringLiteral("The application could not complete the operation. Read state before continuing."));
            }
            socket->write(QJsonDocument(response).toJson(QJsonDocument::Compact) + '\n');
            if (m_clients.contains(socket)) socket->flush();
            else socket->disconnectFromServer();
        };
        connect(socket, &QLocalSocket::readyRead, this, receive);
        receive();
    }
}

QString McpBridge::stateToken() const
{
    QByteArray state = m_session + m_window->doc->toByteArray(-1);
    state += qApp->getFilePath().toUtf8();
    state += m_window->doc->getActiveDraftBlockName().toUtf8();
    state += QByteArray::number(qApp->getUndoStack()->index());
    state += QByteArray::number(m_revision);
    state += QByteArray::number(m_window->doc->getCursorId());
    state += QByteArray::number(VContainer::size(), 'g', 17) + ':' + QByteArray::number(VContainer::height(), 'g', 17);
    if (!m_window->doc->MPath().isEmpty())
    {
        QFile measurements(measurementsPath());
        state += measurements.open(QIODevice::ReadOnly)
            ? QCryptographicHash::hash(measurements.read(4 * 1024 * 1024 + 1), QCryptographicHash::Sha256)
            : QByteArray("measurements-unavailable");
    }
    return QString::fromLatin1(QCryptographicHash::hash(state, QCryptographicHash::Sha256).toHex());
}

QString McpBridge::measurementsPath() const
{
    const auto path = m_window->doc->MPath();
    if (path.isEmpty() || QDir::isAbsolutePath(path)) return path;
    return QFileInfo(qApp->getFilePath()).dir().absoluteFilePath(path);
}

QByteArray McpBridge::validateDocument(const QByteArray &xml, const QString &origin, bool allowConversion) const
{
    require(xml.size() <= 4 * 1024 * 1024, "Pattern XML exceeds the 4 MiB edit limit.");
    auto candidate = parseXml(xml);
    require(candidate.documentElement().tagName() == "pattern", "Expected a pattern document.");
    QTemporaryDir directory;
    require(directory.isValid(), "Could not create validation workspace.");
    auto measurement = candidate.documentElement().firstChildElement("measurements");
    QString measurementSource = measurement.text();
    bool multisize = false;
    if (!measurementSource.isEmpty())
    {
        if (!QDir::isAbsolutePath(measurementSource))
        {
            require(!origin.isEmpty(), "A new document must use an absolute measurements path.");
            measurementSource = QFileInfo(origin).dir().absoluteFilePath(measurementSource);
        }
        const auto table = McpMeasurements::inspect(McpMeasurements::readBytes(measurementSource));
        multisize = table.info.value("type") == QJsonValue("multisize");
        const auto copyPath = directory.filePath(table.info.value("type") == QJsonValue("multisize") ? "measurements.smms" : "measurements.smis");
        QFile copy(copyPath);
        require(copy.open(QIODevice::WriteOnly) && copy.write(table.xml) == table.xml.size(), "Could not stage linked measurements.");
        copy.close();
        VDomDocument::RemoveAllChildren(measurement);
        measurement.appendChild(candidate.createTextNode(copyPath));
    }
    // Keep image references valid when validating the private copy elsewhere.
    const auto images = candidate.elementsByTagName("image");
    for (int i = 0; i < images.size(); ++i)
    {
        auto image = images.at(i).toElement();
        for (const auto &attribute : {"filename", "fileName", "path"})
        {
            const auto value = image.attribute(attribute);
            if (!value.isEmpty() && !QDir::isAbsolutePath(value) && !origin.isEmpty())
                image.setAttribute(attribute, QFileInfo(origin).dir().absoluteFilePath(value));
        }
    }
    const auto path = directory.filePath("candidate.sm2d");
    QFile file(path);
    require(file.open(QIODevice::WriteOnly), "Could not write validation copy.");
    const auto bytes = candidate.toByteArray();
    require(file.write(bytes) == bytes.size(), "Could not write validation copy.");
    file.close();
    if (!allowConversion) require(candidate.documentElement().firstChildElement("version").text() == "0.7.6", "Document edits require the current pattern version.");
    auto env = QProcessEnvironment::systemEnvironment();
    env.remove("SEAMLY_MCP_SOCKET");
    env.insert("SEAMLY_DATA_DIR", directory.filePath("state"));
    env.insert("SEAMLY_MCP_VALIDATE", "1");
    const auto output = directory.filePath("normalized.sm2d");
    env.insert("SEAMLY_MCP_NORMALIZE_OUTPUT", output);
    QProcess process;
    process.setProcessEnvironment(env);
    process.setProcessChannelMode(QProcess::MergedChannels);
    QStringList args{"--test"};
    const auto gradation = candidate.documentElement().firstChildElement("gradation");
    if (multisize && gradation.attribute("custom") == "true")
    {
        if (gradation.hasAttribute("defSize")) args << "--gsize" << gradation.attribute("defSize");
        if (gradation.hasAttribute("defHeight")) args << "--gheight" << gradation.attribute("defHeight");
    }
    args << path;
    process.start(QCoreApplication::applicationFilePath(), args);
    require(process.waitForStarted(5000), "Could not start isolated pattern validation.");
    if (!process.waitForFinished(20000))
    {
        process.kill(); process.waitForFinished(3000);
        throw std::runtime_error("Pattern validation timed out; the live document was not changed.");
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0)
        throw std::runtime_error((QStringLiteral("Pattern validation failed: ") + QString::fromUtf8(process.readAll()).left(6000)).toStdString());
    QFile normalized(output);
    require(normalized.open(QIODevice::ReadOnly), "Validator produced no normalized pattern.");
    auto result = parseXml(normalized.readAll());
    if (!measurementSource.isEmpty())
    {
        auto node = result.documentElement().firstChildElement("measurements");
        VDomDocument::RemoveAllChildren(node); node.appendChild(result.createTextNode(measurementSource));
    }
    return result.toByteArray();
}

void McpBridge::restoreDocument(const QByteArray &xml, const QString &activeBlock)
{
    if (!m_window->m_watcher->files().isEmpty()) m_window->m_watcher->removePaths(m_window->m_watcher->files());
    QTemporaryDir directory;
    QFile file(directory.filePath("snapshot.sm2d"));
    require(directory.isValid() && file.open(QIODevice::WriteOnly), "Could not restore pattern snapshot.");
    require(file.write(xml) == xml.size(), "Could not write pattern snapshot.");
    file.close();
    m_window->doc->setXMLContent(file.fileName());
    qApp->setPatternUnit(m_window->doc->measurementUnits());
    const auto measurements = m_window->doc->MPath();
    if (!measurements.isEmpty())
    {
        const auto path = QDir::isAbsolutePath(measurements) ? measurements : QFileInfo(qApp->getFilePath()).dir().absoluteFilePath(measurements);
        const auto table = McpMeasurements::inspect(McpMeasurements::readBytes(path));
        const auto copyPath = directory.filePath(table.info.value("type") == QJsonValue("multisize") ? "measurements.smms" : "measurements.smis");
        QFile copy(copyPath);
        require(copy.open(QIODevice::WriteOnly) && copy.write(table.xml) == table.xml.size(), "Could not stage linked measurements."); copy.close();
        require(m_window->loadMeasurements(copyPath), "Could not load validated measurements.");
        m_window->m_watcher->addPath(path);
    }
    else
    {
        m_window->pattern->ClearVariables(VarType::Measurement);
        qApp->setPatternType(MeasurementsType::Unknown);
    }
    m_window->fullParseFile();
    require(m_window->guiEnabled, "Failed to load validated pattern.");
    m_window->patternReadOnly = m_window->doc->isReadOnly();
    m_window->setWidgetsEnabled(true);
    const auto blocks = m_window->doc->getPatternPieces();
    if (!blocks.isEmpty()) m_window->changeDraftBlockGlobally(blocks.contains(activeBlock) ? activeBlock : blocks.first());
    m_window->doc->SetModified(true);
    m_window->doc->haveLiteChange();
    m_window->m_changes = false;
    m_window->updateWindowTitle();
    ++m_revision;
}

void McpBridge::commitDocument(const QByteArray &xml, const QString &label)
{
    const auto before = m_window->doc->toByteArray();
    const auto active = m_window->doc->getActiveDraftBlockName();
    const auto after = validateDocument(xml, qApp->getFilePath());
    try { restoreDocument(after, active); }
    catch (...) { restoreDocument(before, active); throw; }
    qApp->getUndoStack()->push(new DocumentCommand(before, after,
        [this, active](const QByteArray &snapshot) { restoreDocument(snapshot, active); }, label));
}

QJsonObject McpBridge::execute(const QString &method, const QJsonObject &arguments)
{
    require(!QApplication::activeModalWidget() && !QApplication::activePopupWidget(), "Close the active dialog or menu before using the bridge.");
    auto *doc = m_window->doc;
    auto *data = m_window->pattern;
    auto *stack = qApp->getUndoStack();
    const QString token = stateToken();
    const auto objects = data->DataGObjects();
    const bool modified = doc->IsModified() || m_window->isWindowModified() || !stack->isClean();
    const auto summary = [this]() { return execute(QStringLiteral("get_pattern_summary"), {}); };
    if (method == "get_pattern_summary")
    {
        const QString unit = qApp->patternUnit() == Unit::Cm ? "cm" : qApp->patternUnit() == Unit::Mm ? "mm" : qApp->patternUnit() == Unit::Inch ? "inch" : "px";
        return {{"state_token", token}, {"name", doc->GetPatternName()}, {"file", qApp->getFilePath()},
                {"active_draft_block", doc->getActiveDraftBlockName()}, {"draft_blocks", QJsonArray::fromStringList(doc->getPatternPieces())},
                {"unit", unit}, {"coordinates", "x right, y down; document units; counterclockwise degrees"},
                {"modified", modified}, {"read_only", doc->isReadOnly()}, {"object_count", static_cast<int>(objects->size())},
                {"piece_count", static_cast<int>(data->DataPieces()->size())}, {"measurement_names", QJsonArray::fromStringList(doc->ListMeasurements())},
                {"can_undo", stack->canUndo()}, {"undo_text", stack->undoText()}, {"can_redo", stack->canRedo()}, {"redo_text", stack->redoText()},
                {"mode", qApp->getSceneView()->scene() == m_window->draftScene ? "draft" : "piece"}};
    }

    if (method == "get_export_formats")
    {
        ExportFormatCombobox formats; QJsonArray result;
        for (int i = 0; i < formats.count(); ++i) result.append(QJsonObject{{"id", formats.itemData(i).toInt()}, {"name", formats.itemText(i)}});
        return {{"formats", result}, {"value_options", QJsonArray{"pageformat", "pagew", "pageh", "pageunits", "lmargin", "rmargin", "tmargin", "bmargin", "rotate", "shiftlen", "layounits", "gapwidth", "groups", "gsize", "gheight"}},
                {"flag_options", QJsonArray{"ignoremargins", "crop", "unite", "savelen", "bdxf", "text2paths"}}};
    }
    if (method == "list_pieces")
    {
        QJsonArray pieces; const auto nodes = doc->elementsByTagName("piece");
        for (int i = 0; i < nodes.size(); ++i)
        {
            const auto element = nodes.at(i).toElement();
            if (element.parentNode().nodeName() == "pieces") pieces.append(QJsonObject{{"id", element.attribute("id").toInt()}, {"name", element.attribute("name")}, {"attributes", attributesJson(element)}});
        }
        return {{"state_token", token}, {"pieces", pieces}};
    }
    if (method == "get_measurements_file" || method == "get_measurements_status")
    {
        const bool attached = method == "get_measurements_status";
        const auto path = attached ? measurementsPath() : arguments.value("path").toString();
        QJsonObject result{{"state_token", token}, {"path", path}, {"attached", !measurementsPath().isEmpty()}, {"needs_sync", m_window->m_changes}};
        if (attached && path.isEmpty()) return result;
        require(QDir::isAbsolutePath(path), "Measurements require an absolute file path.");
        const auto bytes = McpMeasurements::readBytes(path);
        auto table = McpMeasurements::inspect(bytes, arguments.value("size").toDouble(-1), arguments.value("height").toDouble(-1));
        if (attached && table.info.value("type") == QJsonValue("multisize"))
        {
            const auto unit = StrToUnits(table.info.value("unit").toString());
            table = McpMeasurements::inspect(bytes, UnitConvertor(VContainer::size(), qApp->patternUnit(), unit),
                UnitConvertor(VContainer::height(), qApp->patternUnit(), unit));
            result.insert("gradation_unit", "cm");
            result.insert("active_size", UnitConvertor(VContainer::size(), qApp->patternUnit(), Unit::Cm));
            result.insert("active_height", UnitConvertor(VContainer::height(), qApp->patternUnit(), Unit::Cm));
            result.insert("allowed_sizes", QJsonArray::fromStringList(MeasurementVariable::ListSizes(doc->GetGradationSizes(), Unit::Cm)));
            result.insert("allowed_heights", QJsonArray::fromStringList(MeasurementVariable::ListHeights(doc->GetGradationHeights(), Unit::Cm)));
        }
        for (auto it = table.info.begin(); it != table.info.end(); ++it) result.insert(it.key(), it.value());
        result.insert("file_token", McpMeasurements::token(path, bytes));
        return result;
    }
    if (method == "get_piece")
    {
        const auto id = static_cast<quint32>(arguments.value("id").toInt());
        require(data->DataPieces()->contains(id), "Unknown piece ID.");
        const auto element = elementWithId(*doc, id);
        const auto piece = data->DataPieces()->value(id);
        VContainer geometryData(*data);
        geometryData.AddVariable(currentSeamAllowance, new CustomVariable(&geometryData, currentSeamAllowance, 0,
            piece.GetSAWidth(), QString::number(piece.GetSAWidth(), 'g', 17), true));
        QJsonArray nodes;
        for (auto node = element.firstChildElement("nodes").firstChildElement("node"); !node.isNull(); node = node.nextSiblingElement("node"))
        {
            auto row = attributesJson(node);
            const auto model = elementWithId(*doc, node.attribute("idObject").toUInt());
            row.insert("node_id", node.attribute("idObject").toInt()); row.insert("object_id", model.attribute("idObject").toInt());
            nodes.append(row);
        }
        const auto outline = [](const QVector<QPointF> &points)
        {
            require(points.size() <= 10000, "Piece outline exceeds the read limit.");
            QJsonArray result; qreal area = 0;
            for (int i = 0; i < points.size(); ++i)
            {
                const auto a = points.at(i), b = points.at((i + 1) % points.size());
                area += a.x() * b.y() - b.x() * a.y();
                result.append(QJsonObject{{"x", qApp->fromPixel(a.x())}, {"y", qApp->fromPixel(a.y())}});
            }
            const auto bounds = QPolygonF(points).boundingRect();
            const auto factor = qApp->fromPixel(1);
            return QJsonObject{{"points", result}, {"area", std::abs(area) * factor * factor / 2},
                {"bounds", QJsonObject{{"x", qApp->fromPixel(bounds.x())}, {"y", qApp->fromPixel(bounds.y())},
                    {"width", qApp->fromPixel(bounds.width())}, {"height", qApp->fromPixel(bounds.height())}}}};
        };
        return {{"state_token", token}, {"id", static_cast<double>(id)}, {"attributes", attributesJson(element)},
            {"nodes", nodes}, {"main_path", outline(piece.mainPathPoints(data))},
            {"seam_allowance_path", outline(piece.seamAllowancePoints(&geometryData))}};
    }
    if (method == "get_pattern_xml") return {{"state_token", token}, {"xml", doc->toString()}};
    if (method == "get_history")
    {
        QJsonArray commands;
        for (int i = 0; i < stack->count(); ++i) commands.append(QJsonObject{{"index", i}, {"text", stack->text(i)}, {"applied", i < stack->index()}});
        return {{"state_token", token}, {"index", stack->index()}, {"commands", commands}};
    }
    if (method == "list_variables")
    {
        QJsonArray variables;
        const auto values = data->DataVariables();
        auto names = values->keys(); std::sort(names.begin(), names.end());
        for (const auto &name : names)
        {
            const auto value = values->value(name);
            QJsonObject entry{{"name", name}, {"value", static_cast<const VInternalVariable &>(*value).GetValue()}, {"type", static_cast<int>(value->GetType())}};
            if (const auto custom = qSharedPointerDynamicCast<CustomVariable>(value)) entry.insert("formula", custom->GetFormula());
            variables.append(entry);
        }
        return {{"state_token", token}, {"variables", variables}};
    }
    if (method == "evaluate_formula")
    {
        Calculator calculator;
        const auto result = calculator.EvalFormula(data->DataVariables(), arguments.value("formula").toString());
        require(std::isfinite(result), "Formula must evaluate to a finite number.");
        return {{"state_token", token}, {"value", result}};
    }
    if (method == "get_object")
    {
        const auto id = static_cast<quint32>(integerArgument(arguments, "id", 0, 2147483647));
        auto element = elementWithId(*doc, id);
        if (element.isNull() && objects->contains(id))
            element = elementWithId(*doc, objects->value(id)->getIdTool());
        require(!element.isNull(), "Unknown document element ID.");
        QDomDocument fragment; fragment.appendChild(fragment.importNode(element, true));
        QJsonObject result{{"state_token", token}, {"id", static_cast<double>(id)},
                           {"element_id", element.attribute("id").toInt()}, {"tag", element.tagName()},
                           {"attributes", attributesJson(element)}, {"xml", fragment.toString()}};
        if (objects->contains(id)) result.insert("geometry", objectJson(id, objects->value(id)));
        return result;
    }
    if (method == "list_objects")
    {
        const int offset = integerArgument(arguments, "offset", 0, 10000000);
        const int limit = integerArgument(arguments, "limit", 100, 200);
        require(limit > 0, "limit must be positive.");
        auto ids = objects->keys(); std::sort(ids.begin(), ids.end());
        QJsonArray result;
        for (int i = offset; i < ids.size() && i < offset + limit; ++i)
        {
            const auto object = objects->value(ids.at(i));
            auto entry = objectJson(ids.at(i), object);
            entry.insert("draft_block", doc->getToolDraftBlockName(object->getIdTool()));
            result.append(entry);
        }
        return {{"state_token", token}, {"objects", result}, {"total", static_cast<int>(ids.size())},
                {"next_offset", offset + result.size() < ids.size() ? QJsonValue(offset + result.size()) : QJsonValue()}};
    }
    if (method == "get_selection")
    {
        QSet<quint32> selected;
        if (auto *scene = qApp->getSceneView()->scene()) for (auto *item : scene->selectedItems())
            for (auto *parent = item; parent; parent = parent->parentItem())
                if (auto *tool = dynamic_cast<VAbstractTool *>(parent)) { selected.insert(tool->getId()); break; }
        auto ids = selected.values(); std::sort(ids.begin(), ids.end()); QJsonArray result;
        for (auto id : ids) result.append(objects->contains(id) ? objectJson(id, objects->value(id)) : QJsonObject{{"tool_id", static_cast<double>(id)}});
        return {{"state_token", token}, {"selection", result}};
    }
    const auto renderSvg = [this]()
    {
        auto *scene = qApp->getSceneView()->scene();
        require(scene != nullptr, "No scene is open.");
        const auto bounds = scene->itemsBoundingRect().adjusted(-10, -10, 10, 10);
        QByteArray bytes; QBuffer buffer(&bytes); buffer.open(QIODevice::WriteOnly);
        QSvgGenerator svg; svg.setOutputDevice(&buffer); svg.setSize(QSize(1200, 900)); svg.setViewBox(QRect(0, 0, 1200, 900));
        QPainter painter(&svg); scene->render(&painter, QRectF(0, 0, 1200, 900), bounds, Qt::KeepAspectRatio); painter.end();
        return bytes;
    };
    if (method == "get_preview") return {{"state_token", token}, {"svg_base64", QString::fromLatin1(renderSvg().toBase64())}};

    require(arguments.value("state_token").isString() && arguments.value("state_token").toString() == token,
            "Document state changed. Read get_pattern_summary and review the current state before retrying.");
    require(m_window->currentTool == Tool::Arrow && m_window->dialogTool.isNull(), "Finish the active drawing tool before modifying the pattern.");
    if (method == "write_measurements_file")
    {
        const auto path = arguments.value("path").toString();
        require(QDir::isAbsolutePath(path) && !QFileInfo(path).isSymLink(), "Use an absolute measurement path, not a symlink.");
        const bool exists = QFileInfo::exists(path);
        require(!exists || QFileInfo(path).isWritable(), "Measurement file is not writable.");
        VLockGuard<char> lock(path); require(lock.IsLocked(), "Measurement file is locked; close it in SeamlyMe before editing.");
        const auto original = exists ? McpMeasurements::readBytes(path) : QByteArray{};
        require(!exists || (arguments.value("overwrite").toBool() && arguments.value("file_token") == QJsonValue(McpMeasurements::token(path, original))),
            "Existing measurements require overwrite=true and the latest file_token.");
        require(exists || !arguments.contains("file_token"), "Measurement file disappeared; read state before creating it again.");
        const auto table = McpMeasurements::prepare(original, arguments);
        require(QFileInfo(path).suffix() == (table.info.value("type") == QJsonValue("multisize") ? "smms" : "smis"), "File extension must match the measurement type.");
        const bool active = !measurementsPath().isEmpty() && QFileInfo(measurementsPath()).canonicalFilePath() == QFileInfo(path).canonicalFilePath();
        const auto snapshot = doc->toByteArray(); const auto block = doc->getActiveDraftBlockName();
        if (active)
        {
            require(!m_window->patternReadOnly && !doc->isReadOnly(), "The attached pattern is read-only.");
            QTemporaryDir directory; require(directory.isValid(), "Could not stage measurement edit.");
            QFile copy(directory.filePath(QFileInfo(path).fileName()));
            require(copy.open(QIODevice::WriteOnly) && copy.write(table.xml) == table.xml.size(), "Could not stage measurement edit."); copy.close();
            auto candidate = parseXml(snapshot); auto node = candidate.documentElement().firstChildElement("measurements");
            VDomDocument::RemoveAllChildren(node); node.appendChild(candidate.createTextNode(copy.fileName()));
            validateDocument(candidate.toByteArray(), qApp->getFilePath());
        }
        const auto save = [&](const QByteArray &bytes)
        {
            QSaveFile file(path);
            require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size() && file.commit(), "Could not atomically save measurements.");
        };
        require(!exists || McpMeasurements::readBytes(path) == original, "Measurements changed during validation; read again.");
        PauseMeasurementWatch watch(active ? m_window->m_watcher : nullptr);
        save(table.xml);
        if (active)
        {
            try { restoreDocument(snapshot, block); }
            catch (...) { save(original); restoreDocument(snapshot, block); throw; }
        }
        ++m_revision;
        auto result = table.info; result.insert("state_token", stateToken()); result.insert("path", path);
        result.insert("file_token", McpMeasurements::token(path, table.xml)); result.insert("undoable", false);
        return result;
    }
    if (method == "new_pattern" || method == "open_pattern" || method == "close_pattern")
    {
        require(!modified || arguments.value("discard_changes").toBool(), "Unsaved changes exist. Save first or explicitly set discard_changes=true.");
        QByteArray xml; QString path;
        std::shared_ptr<VLockGuard<char>> nextLock;
        if (method == "open_pattern")
        {
            path = arguments.value("path").toString();
            require(QDir::isAbsolutePath(path) && QFileInfo(path).isFile(), "Open requires an existing absolute file path.");
            require(QFileInfo(path).suffix().compare("sm2d", Qt::CaseInsensitive) == 0, "Open requires a .sm2d pattern.");
            if (path != qApp->getFilePath()) { nextLock = std::make_shared<VLockGuard<char>>(path); require(nextLock->IsLocked(), "Pattern is already locked by another instance."); }
            else nextLock = m_window->m_lock;
            QFile file(path); require(file.open(QIODevice::ReadOnly), "Could not read pattern.");
            require(file.size() <= 4 * 1024 * 1024, "Pattern exceeds the 4 MiB limit.");
            xml = validateDocument(file.readAll(), path, true);
        }
        else if (method == "new_pattern")
        {
            QDomDocument candidate; auto root = candidate.createElement("pattern"); candidate.appendChild(root);
            for (const auto &tag : {"version", "unit", "description", "notes", "measurements", "variables"})
            {
                auto element = candidate.createElement(tag);
                if (QString(tag) == "version") element.appendChild(candidate.createTextNode("0.7.6"));
                if (QString(tag) == "unit") element.appendChild(candidate.createTextNode(arguments.value("unit").toString()));
                root.appendChild(element);
            }
            auto block = candidate.createElement("draftBlock"); block.setAttribute("name", arguments.value("draft_block").toString()); root.appendChild(block);
            auto calc = candidate.createElement("calculation"); auto point = candidate.createElement("point");
            point.setAttribute("id", "1"); point.setAttribute("type", "single"); point.setAttribute("name", "A"); point.setAttribute("x", "0"); point.setAttribute("y", "0");
            point.setAttribute("mx", "5"); point.setAttribute("my", "10"); calc.appendChild(point); block.appendChild(calc);
            for (const auto &tag : {"modeling", "pieces", "groups", "images"}) block.appendChild(candidate.createElement(tag));
            xml = validateDocument(candidate.toByteArray(), {});
        }
        m_window->Clear(); stack->clear();
        doc->Parse(Document::FullParse);
        doc->SetModified(false);
        m_window->setWindowModified(false);
        if (method != "close_pattern")
        {
            m_window->setCurrentFile(path); m_window->m_lock = nextLock;
            restoreDocument(xml, {});
            if (method == "open_pattern") { doc->SetModified(false); m_window->patternChangesWereSaved(true); stack->setClean(); }
        }
        ++m_revision;
        return summary();
    }
    require(doc->draftBlockCount() > 0, "Open or create a pattern first.");
    if (method == "set_view")
    {
        if (arguments.value("mode") == QJsonValue("draft")) m_window->showDraftMode(true);
        if (arguments.value("mode") == QJsonValue("piece")) m_window->showPieceMode(true);
        auto *view = qApp->getSceneView();
        if (arguments.contains("scale")) { view->resetTransform(); view->scale(arguments.value("scale").toDouble(), arguments.value("scale").toDouble()); }
        if (arguments.value("fit").toBool()) view->fitInView(view->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
        if (arguments.contains("x") && arguments.contains("y")) view->centerOn(qApp->toPixel(arguments.value("x").toDouble()), qApp->toPixel(arguments.value("y").toDouble()));
        return summary();
    }
    if (method == "set_active_draft_block")
    {
        const auto name = arguments.value("name").toString(); require(doc->getPatternPieces().contains(name), "Unknown draft block.");
        m_window->changeDraftBlockGlobally(name); return summary();
    }
    if (method == "set_selection")
    {
        auto *scene = qApp->getSceneView()->scene(); QSet<quint32> requested;
        for (const auto &value : arguments.value("ids").toArray()) requested.insert(static_cast<quint32>(value.toInt()));
        QHash<quint32, QGraphicsItem *> targets;
        for (auto *item : scene->items()) if (auto *tool = dynamic_cast<VAbstractTool *>(item))
        {
            if (!requested.contains(tool->getId())) continue;
            if (item->flags() & QGraphicsItem::ItemIsSelectable) targets.insert(tool->getId(), item);
            else for (auto *child : item->childItems()) if (child->flags() & QGraphicsItem::ItemIsSelectable) { targets.insert(tool->getId(), child); break; }
        }
        require(targets.size() == requested.size(), "Some IDs are not selectable in the current scene.");
        scene->clearSelection(); for (auto *item : targets) item->setSelected(true);
        return execute("get_selection", {});
    }

    if (method == "export_layout")
    {
        require(!data->DataPieces()->isEmpty(), "Create pattern pieces before generating a layout.");
        const auto outputPath = arguments.value("output_directory").toString();
        const auto basename = arguments.value("basename").toString();
        require(QDir::isAbsolutePath(outputPath), "Layout output directory must be absolute.");
        require(!QFileInfo::exists(outputPath) || (QFileInfo(outputPath).isDir() && QDir(outputPath).entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Hidden).isEmpty()), "Layout output directory must be empty.");
        require(QRegularExpression("^[A-Za-z0-9][A-Za-z0-9_-]{0,63}$").match(basename).hasMatch(), "Use a plain filename stem containing letters, digits, underscore or hyphen.");
        QTemporaryDir directory; require(directory.isValid(), "Could not create layout workspace.");
        const auto source = directory.filePath("pattern.sm2d"); QFile file(source);
        require(file.open(QIODevice::WriteOnly), "Could not write layout snapshot.");
        const auto xml = validateDocument(doc->toByteArray(), qApp->getFilePath()); file.write(xml); file.close();
        const auto staged = directory.filePath("output"); QDir().mkpath(staged);
        QStringList args{"--basename", basename, "--destination", staged, "--format", QString::number(arguments.value("format").toInt())};
        if (arguments.value("pieces_only").toBool()) args.append("--exportOnlyDetails");
        const auto supported = execute("get_export_formats", {});
        const auto values = supported.value("value_options").toArray(), flags = supported.value("flag_options").toArray();
        auto options = arguments.value("options").toObject();
        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            if (!options.contains("gsize")) options.insert("gsize", QString::number(qRound(UnitConvertor(VContainer::size(), qApp->patternUnit(), Unit::Cm))));
            if (!options.contains("gheight")) options.insert("gheight", QString::number(qRound(UnitConvertor(VContainer::height(), qApp->patternUnit(), Unit::Cm))));
        }
        // The legacy CLI requires all four explicit margins when pageunits is
        // specified, even with ignoremargins. Supply deterministic zero defaults.
        if (options.contains("pageunits")) for (const auto &margin : {"lmargin", "rmargin", "tmargin", "bmargin"})
            if (!options.contains(margin)) options.insert(margin, "0");
        for (auto option = options.begin(); option != options.end(); ++option)
        {
            const auto value = option.value().toString();
            if (flags.contains(option.key())) { require(value == "true" || value == "false", "Flag values must be true or false strings."); if (value == "true") args.append("--" + option.key()); }
            else { require(values.contains(option.key()), "Unsupported layout option."); args.append("--" + option.key() + "=" + value); }
        }
        args.append(source);
        auto env = QProcessEnvironment::systemEnvironment(); env.remove("SEAMLY_MCP_SOCKET"); env.remove("SEAMLY_MCP_NORMALIZE_OUTPUT");
        env.insert("SEAMLY_DATA_DIR", directory.filePath("state")); env.insert("SEAMLY_MCP_VALIDATE", "1");
        QProcess process; process.setProcessEnvironment(env); process.setProcessChannelMode(QProcess::MergedChannels);
        process.start(QCoreApplication::applicationFilePath(), args);
        require(process.waitForStarted(5000), "Could not start layout engine.");
        if (!process.waitForFinished(20000)) { process.kill(); process.waitForFinished(3000); throw std::runtime_error("Layout generation timed out; no staged files were published."); }
        if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) throw std::runtime_error((QStringLiteral("Layout export failed: ") + QString::fromUtf8(process.readAll()).left(6000)).toStdString());
        const auto files = QDir(staged).entryList(QDir::Files); require(!files.isEmpty(), "Layout engine produced no files.");
        require(QDir().mkpath(outputPath), "Could not create output directory."); QJsonArray written;
        for (const auto &name : files)
        {
            const auto target = QDir(outputPath).filePath(name); require(!QFileInfo::exists(target), "A destination file appeared during export; refusing to overwrite it.");
            QFile input(QDir(staged).filePath(name)); QSaveFile output(target); require(input.open(QIODevice::ReadOnly) && output.open(QIODevice::WriteOnly), "Could not publish layout file.");
            const auto bytes = input.readAll(); require(output.write(bytes) == bytes.size() && output.commit(), "Could not save layout file."); written.append(target);
        }
        return {{"state_token", token}, {"files", written}};
    }
    if (method == "export_pattern")
    {
        const auto path = arguments.value("path").toString();
        require(QDir::isAbsolutePath(path), "Export requires an absolute path.");
        require(!QFileInfo::exists(path) || arguments.value("overwrite").toBool(), "Export destination exists; set overwrite=true explicitly.");
        QSaveFile file(path); require(file.open(QIODevice::WriteOnly), "Could not open export destination.");
        const auto format = arguments.value("format").toString();
        if (format == "svg") { const auto bytes = renderSvg(); require(file.write(bytes) == bytes.size(), "SVG export failed."); }
        else
        {
            auto *scene = qApp->getSceneView()->scene(); const auto bounds = scene->itemsBoundingRect().adjusted(-10, -10, 10, 10);
            if (format == "png")
            {
                QImage image(1600, 1200, QImage::Format_ARGB32_Premultiplied); image.fill(Qt::white); QPainter painter(&image);
                scene->render(&painter, QRectF(0, 0, 1600, 1200), bounds, Qt::KeepAspectRatio); painter.end(); require(image.save(&file, "PNG"), "PNG export failed.");
            }
            else
            {
                require(format == "pdf", "Unsupported export format.");
                QPdfWriter pdf(&file); QPainter painter(&pdf); scene->render(&painter, {}, bounds, Qt::KeepAspectRatio); painter.end();
            }
        }
        require(file.commit(), "Could not commit export file.");
        return {{"state_token", token}, {"path", path}, {"format", format}};
    }
    require(!m_window->patternReadOnly && !doc->isReadOnly(), "The pattern is read-only.");
    if (method == "save_pattern")
    {
        const auto path = arguments.value("path").toString(qApp->getFilePath());
        require(!path.isEmpty() && QDir::isAbsolutePath(path), "Saving an untitled pattern requires an absolute path.");
        require(QFileInfo(path).suffix().compare("sm2d", Qt::CaseInsensitive) == 0, "Save requires a .sm2d extension.");
        std::shared_ptr<VLockGuard<char>> nextLock = m_window->m_lock;
        if (path != qApp->getFilePath())
        {
            require(!QFileInfo::exists(path) || arguments.value("overwrite").toBool(), "Destination exists; set overwrite=true explicitly.");
            nextLock = std::make_shared<VLockGuard<char>>(path); require(nextLock->IsLocked(), "Destination is locked by another instance.");
        }
        QString error; require(m_window->SavePattern(path, error), "Could not save pattern.");
        m_window->m_lock = nextLock; stack->setClean(); ++m_revision; return summary();
    }
    require(doc->getCursorId() == NULL_ID, "Move the history cursor to the end before modifying the pattern.");
    if (method == "undo" || method == "redo")
    {
        require(method == "undo" ? stack->canUndo() : stack->canRedo(), "No operation available.");
        if (method == "undo") stack->undo(); else stack->redo();
        return summary();
    }
    const bool pieceEdit = method == "create_piece" || method == "update_piece" || method == "delete_piece" || method == "set_seam_allowance";
    const bool measurementEdit = method == "attach_measurements" || method == "detach_measurements" || method == "set_gradation";
    require(!m_window->m_changes || measurementEdit, "Measurements changed externally; read and reattach the table to refresh the pattern first.");
    require(pieceEdit || measurementEdit || qApp->getSceneView()->scene() == m_window->draftScene, "Switch to Draft mode first.");
    if (method != "create_point")
    {
        auto candidate = parseXml(doc->toByteArray()); auto root = candidate.documentElement();
        QDomElement active;
        for (auto block = root.firstChildElement("draftBlock"); !block.isNull(); block = block.nextSiblingElement("draftBlock"))
            if (block.attribute("name") == doc->getActiveDraftBlockName()) active = block;
        QString nextActive;
        quint32 editedPiece = 0;
        if (measurementEdit)
        {
            auto measurement = root.firstChildElement("measurements");
            const auto setGradation = [&](int size, int height)
            {
                auto gradation = root.firstChildElement("gradation");
                if (gradation.isNull())
                {
                    gradation = candidate.createElement("gradation");
                    QDomElement previous;
                    for (auto node = root.firstChildElement(); !node.isNull(); node = node.nextSiblingElement())
                    {
                        if (QStringList{"version", "unit", "image", "description", "notes"}.contains(node.tagName())) previous = node;
                        else break;
                    }
                    root.insertAfter(gradation, previous);
                    for (const auto &tag : {"heights", "sizes"}) { auto child = candidate.createElement(tag); child.setAttribute("all", "true"); gradation.appendChild(child); }
                }
                gradation.setAttribute("custom", "true"); gradation.setAttribute("defSize", size); gradation.setAttribute("defHeight", height);
            };
            if (method == "attach_measurements")
            {
                const auto path = arguments.value("path").toString(); require(QDir::isAbsolutePath(path), "Use an absolute measurements path.");
                const auto bytes = McpMeasurements::readBytes(path);
                require(arguments.value("file_token") == QJsonValue(McpMeasurements::token(path, bytes)), "Measurement file changed; read it again.");
                const auto table = McpMeasurements::inspect(bytes);
                VDomDocument::RemoveAllChildren(measurement); measurement.appendChild(candidate.createTextNode(path));
                if (table.info.value("type") == QJsonValue("multisize"))
                {
                    const auto unit = StrToUnits(table.info.value("unit").toString());
                    const bool sameTable = qApp->patternType() == MeasurementsType::Multisize && !measurementsPath().isEmpty()
                        && QFileInfo(measurementsPath()).canonicalFilePath() == QFileInfo(path).canonicalFilePath();
                    setGradation(qRound(sameTable ? UnitConvertor(VContainer::size(), qApp->patternUnit(), Unit::Cm)
                        : UnitConvertor(table.info.value("base_size").toDouble(), unit, Unit::Cm)),
                        qRound(sameTable ? UnitConvertor(VContainer::height(), qApp->patternUnit(), Unit::Cm)
                        : UnitConvertor(table.info.value("base_height").toDouble(), unit, Unit::Cm)));
                }
                else root.removeChild(root.firstChildElement("gradation"));
            }
            else if (method == "detach_measurements")
            {
                VDomDocument::RemoveAllChildren(measurement); root.removeChild(root.firstChildElement("gradation"));
            }
            else
            {
                require(qApp->patternType() == MeasurementsType::Multisize, "Attach multisize measurements before selecting gradation.");
                const int size = arguments.value("size").toInt(), height = arguments.value("height").toInt();
                require(MeasurementVariable::ListSizes(doc->GetGradationSizes(), Unit::Cm).contains(QString::number(size))
                    && MeasurementVariable::ListHeights(doc->GetGradationHeights(), Unit::Cm).contains(QString::number(height)), "Size/height is not enabled for this pattern.");
                setGradation(size, height);
            }
        }
        else if (pieceEdit)
        {
            QDomElement piece;
            if (method == "create_piece")
            {
                piece = candidate.createElement("piece"); piece.setAttribute("id", freshId(candidate)); piece.setAttribute("version", "2");
                piece.setAttribute("name", arguments.value("name").toString()); piece.setAttribute("mx", "0"); piece.setAttribute("my", "0");
                piece.setAttribute("inLayout", arguments.value("in_layout").toBool(true) ? "true" : "false");
                piece.setAttribute("seamAllowance", arguments.value("seam_allowance").toBool() ? "true" : "false");
                piece.setAttribute("width", arguments.value("width").toString("0"));
                active.firstChildElement("pieces").appendChild(piece);
            }
            else
            {
                piece = elementWithId(candidate, static_cast<quint32>(arguments.value("id").toInt()));
                require(!piece.isNull() && piece.tagName() == "piece" && piece.parentNode().nodeName() == "pieces", "Unknown piece ID.");
                active = piece.parentNode().parentNode().toElement();
            }
            editedPiece = piece.attribute("id").toUInt();
            QSet<quint32> oldNodes;
            if (method == "delete_piece" || arguments.contains("nodes") && method == "update_piece")
                for (auto node = piece.firstChildElement("nodes").firstChildElement("node"); !node.isNull(); node = node.nextSiblingElement("node")) oldNodes.insert(node.attribute("idObject").toUInt());
            if (method == "delete_piece") piece.parentNode().removeChild(piece);
            else if (method == "set_seam_allowance")
            {
                if (arguments.contains("enabled")) piece.setAttribute("seamAllowance", arguments.value("enabled").toBool() ? "true" : "false");
                if (arguments.contains("built_in")) piece.setAttribute("seamAllowanceBuiltIn", arguments.value("built_in").toBool() ? "true" : "false");
                if (arguments.contains("width")) piece.setAttribute("width", arguments.value("width").toString());
                Calculator widthCalculator;
                const auto currentWidth = widthCalculator.EvalFormula(data->DataVariables(), piece.attribute("width", "0"));
                require(std::isfinite(currentWidth) && currentWidth >= 0, "Seam allowance width must be finite and non-negative.");
                VContainer formulaData(*data);
                formulaData.AddVariable(currentSeamAllowance, new CustomVariable(&formulaData, currentSeamAllowance, 0,
                    currentWidth, QString::number(currentWidth, 'g', 17), true));
                for (const auto &entry : arguments.value("nodes").toArray())
                {
                    const auto row = entry.toObject(); QDomElement target;
                    for (auto node = piece.firstChildElement("nodes").firstChildElement("node"); !node.isNull(); node = node.nextSiblingElement("node"))
                        if (node.attribute("idObject").toInt() == row.value("node_id").toInt()) target = node;
                    require(!target.isNull(), "Seam-allowance node does not belong to this piece.");
                    for (const auto &key : {"before", "after"}) if (row.contains(key))
                    {
                        Calculator calculator; const auto value = calculator.EvalFormula(formulaData.DataVariables(), row.value(key).toString());
                        require(std::isfinite(value) && value >= 0, "Node seam allowance must be finite and non-negative.");
                        target.setAttribute(key, row.value(key).toString());
                    }
                }
            }
            else
            {
                for (const auto &key : {"name", "in_layout", "forbid_flipping", "locked"}) if (arguments.contains(key))
                {
                    const QString attribute = QString(key) == "in_layout" ? "inLayout" : QString(key) == "forbid_flipping" ? "forbidFlipping" : key;
                    piece.setAttribute(attribute, QString(key) == "name" ? arguments.value(key).toString() : arguments.value(key).toBool() ? "true" : "false");
                }
                if (arguments.contains("nodes"))
                {
                    const auto input = arguments.value("nodes").toArray(); require(input.size() >= 2, "A piece needs an ordered contour.");
                    auto nodes = candidate.createElement("nodes"); quint32 next = freshId(candidate); QSet<quint32> points; bool curves = false;
                    for (const auto &entry : input)
                    {
                        const auto row = entry.toObject(); const auto id = static_cast<quint32>(row.value("object_id").toInt());
                        require(objects->contains(id), "Unknown contour object."); const auto object = objects->value(id);
                        require(object->getMode() == Draw::Calculation && doc->getToolDraftBlockName(object->getIdTool()) == active.attribute("name"), "Contour objects must belong to this draft block's construction.");
                        QString tag, type, nodeType;
                        switch (object->getType())
                        {
                            case GOType::Point: tag = "point"; type = "modeling"; nodeType = "NodePoint"; points.insert(id); break;
                            case GOType::Arc: tag = "arc"; type = "modeling"; nodeType = "NodeArc"; break;
                            case GOType::EllipticalArc: tag = "elArc"; type = "modeling"; nodeType = "NodeElArc"; break;
                            case GOType::Spline: case GOType::CubicBezier: tag = "spline"; type = "modelingSpline"; nodeType = "NodeSpline"; break;
                            case GOType::SplinePath: case GOType::CubicBezierPath: tag = "spline"; type = "modelingPath"; nodeType = "NodeSplinePath"; break;
                            default: throw std::runtime_error("Unsupported piece contour object.");
                        }
                        curves |= object->getType() != GOType::Point;
                        require(next < 2147483647, "Pattern exhausted the supported ID range.");
                        auto model = candidate.createElement(tag); model.setAttribute("id", next); model.setAttribute("idObject", id);
                        model.setAttribute("type", type); model.setAttribute("inUse", "true");
                        if (object->getType() == GOType::Point) { model.setAttribute("mx", "0"); model.setAttribute("my", "0"); }
                        active.firstChildElement("modeling").appendChild(model);
                        auto node = candidate.createElement("node"); node.setAttribute("idObject", next++); node.setAttribute("type", nodeType);
                        node.setAttribute("reverse", row.value("reverse").toBool() ? "1" : "0"); node.setAttribute("excluded", row.value("excluded").toBool() ? "true" : "false"); nodes.appendChild(node);
                    }
                    require(curves || points.size() >= 3, "A point-only contour needs at least three distinct points.");
                    const auto old = piece.firstChildElement("nodes");
                    if (old.isNull()) piece.appendChild(nodes); else piece.replaceChild(nodes, old);
                }
            }
            if (method != "delete_piece")
            {
                Calculator calculator; const auto width = calculator.EvalFormula(data->DataVariables(), piece.attribute("width", "0"));
                require(std::isfinite(width) && width >= 0, "Seam allowance width must be finite and non-negative.");
            }
            // Retain shared nodes; remove only nodes with no other XML references.
            for (auto id : oldNodes)
            {
                auto model = elementWithId(candidate, id); if (model.parentNode().nodeName() != "modeling") continue;
                bool referenced = false;
                std::function<void(const QDomNode &)> scan = [&](const QDomNode &node)
                {
                    const auto element = node.toElement(); const auto attrs = element.attributes();
                    for (int i = 0; i < attrs.size(); ++i) if (attrs.item(i).nodeName() != "id" && attrs.item(i).nodeValue().toUInt() == id) referenced = true;
                    if (element.tagName() == "record" && element.text().toUInt() == id) referenced = true;
                    for (auto child = node.firstChild(); !child.isNull() && !referenced; child = child.nextSibling()) scan(child);
                };
                scan(candidate); if (!referenced) model.parentNode().removeChild(model);
            }
        }
        else if (method == "apply_pattern_xml") candidate = parseXml(arguments.value("xml").toString().toUtf8());
        else if (method == "set_variable" || method == "delete_variable")
        {
            const auto name = arguments.value("name").toString(); require(name.startsWith('#') && QRegularExpression(NameRegExp()).match(name.mid(1)).hasMatch(), "Custom variable names begin with # and use valid identifiers.");
            auto variables = root.firstChildElement("variables");
            if (variables.isNull()) { variables = candidate.createElement("variables"); root.insertBefore(variables, root.firstChildElement("draftBlock")); }
            QDomElement variable;
            for (auto node = variables.firstChildElement("variable"); !node.isNull(); node = node.nextSiblingElement("variable")) if (node.attribute("name") == name) variable = node;
            if (method == "delete_variable") { require(!variable.isNull(), "Unknown custom variable."); variables.removeChild(variable); }
            else
            {
                if (variable.isNull()) { variable = candidate.createElement("variable"); variable.setAttribute("name", name); variable.setAttribute("description", ""); variables.appendChild(variable); }
                variable.setAttribute("formula", arguments.value("formula").toString());
                if (arguments.contains("description")) variable.setAttribute("description", arguments.value("description").toString());
            }
        }
        else if (method == "update_object" || method == "delete_object")
        {
            auto element = elementWithId(candidate, static_cast<quint32>(arguments.value("id").toInt())); require(!element.isNull(), "Unknown element ID.");
            if (method == "delete_object") element.parentNode().removeChild(element);
            else
            {
                const auto attrs = arguments.value("attributes").toObject(); require(!attrs.contains("id"), "Use a complete document transaction to change IDs.");
                for (auto entry = attrs.begin(); entry != attrs.end(); ++entry) element.setAttribute(entry.key(), entry.value().toString());
            }
        }
        else if (method == "add_draft_block")
        {
            const auto name = arguments.value("name").toString(); require(!doc->getPatternPieces().contains(name), "Draft block name already exists.");
            auto block = candidate.createElement("draftBlock"); block.setAttribute("name", name); root.appendChild(block);
            auto calculation = candidate.createElement("calculation"); auto point = candidate.createElement("point"); point.setAttribute("id", freshId(candidate));
            point.setAttribute("type", "single"); point.setAttribute("name", arguments.value("point_name").toString());
            point.setAttribute("x", arguments.value("x").toDouble()); point.setAttribute("y", arguments.value("y").toDouble()); point.setAttribute("mx", "5"); point.setAttribute("my", "10");
            calculation.appendChild(point); block.appendChild(calculation);
            for (const auto &tag : {"modeling", "pieces", "groups", "images"}) block.appendChild(candidate.createElement(tag));
            nextActive = name;
        }
        else if (method == "rename_draft_block")
        {
            const auto name = arguments.value("name").toString(); const auto newName = arguments.value("new_name").toString();
            require(doc->getPatternPieces().contains(name) && !doc->getPatternPieces().contains(newName), "Draft block must exist and new name must be unique.");
            for (auto block = root.firstChildElement("draftBlock"); !block.isNull(); block = block.nextSiblingElement("draftBlock")) if (block.attribute("name") == name) block.setAttribute("name", newName);
            if (name == doc->getActiveDraftBlockName()) nextActive = newName;
        }
        else if (method == "create_tool")
        {
            auto fragment = parseXml(arguments.value("xml").toString().toUtf8()); auto element = candidate.importNode(fragment.documentElement(), true).toElement();
            if (!element.hasAttribute("id")) element.setAttribute("id", freshId(candidate));
            auto section = active.firstChildElement(arguments.value("section").toString()); require(!section.isNull(), "Unknown or absent draft-block section."); section.appendChild(element);
        }
        else if (method == "create_line" || method == "create_curve" || method == "create_arc")
        {
            const auto requirePoint = [&](const QString &key)
            {
                const auto id = static_cast<quint32>(arguments.value(key).toInt());
                require(objects->contains(id) && objects->value(id)->getType() == GOType::Point && doc->getToolDraftBlockName(objects->value(id)->getIdTool()) == doc->getActiveDraftBlockName(), "Point must belong to the active draft block."); return id;
            };
            auto element = candidate.createElement(method == "create_line" ? "line" : method == "create_curve" ? "spline" : "arc");
            element.setAttribute("id", freshId(candidate)); element.setAttribute("lineWeight", QString::number(doc->getDefaultLineWeight()));
            if (method == "create_line")
            {
                const auto first = requirePoint("first_point_id"), second = requirePoint("second_point_id"); require(first != second, "Line endpoints must differ.");
                element.setAttribute("firstPoint", first); element.setAttribute("secondPoint", second); element.setAttribute("lineType", "solidLine"); element.setAttribute("lineColor", doc->getDefaultLineColor());
            }
            else
            {
                element.setAttribute("color", doc->getDefaultLineColor()); element.setAttribute("penStyle", "solidLine");
                element.setAttribute("angle1", arguments.value("angle1").toString()); element.setAttribute("angle2", arguments.value("angle2").toString());
                if (method == "create_curve")
                {
                    const auto first = requirePoint("first_point_id"), second = requirePoint("second_point_id"); require(first != second, "Curve endpoints must differ.");
                    element.setAttribute("type", "simpleInteractive"); element.setAttribute("point1", first); element.setAttribute("point4", second);
                    element.setAttribute("length1", arguments.value("length1").toString()); element.setAttribute("length2", arguments.value("length2").toString());
                }
                else { element.setAttribute("type", "simple"); element.setAttribute("center", requirePoint("center_id")); element.setAttribute("radius", arguments.value("radius").toString()); }
            }
            active.firstChildElement("calculation").appendChild(element);
        }
        else throw std::runtime_error("Unknown bridge method.");
        const auto label = arguments.value("label").toString(QStringLiteral("MCP: ") + method);
        commitDocument(candidate.toByteArray(), label);
        if (!nextActive.isEmpty()) m_window->changeDraftBlockGlobally(nextActive);
        if (editedPiece && method != "delete_piece") return execute("get_piece", {{"id", static_cast<double>(editedPiece)}});
        if (measurementEdit) return execute("get_measurements_status", {});
        return summary();
    }
    const auto nameValue = arguments.value("name");
    const QString name = nameValue.toString();
    require(nameValue.isString() && name.size() <= 128 && !name.isEmpty()
            && QRegularExpression(NameRegExp()).match(name).hasMatch() && VContainer::IsUnique(name),
            "Point name must be valid and unique.");
    const int baseId = integerArgument(arguments, QStringLiteral("base_point_id"), 0, 2147483647);
    require(baseId > 0 && objects->contains(static_cast<quint32>(baseId)), "Unknown base point.");
    const auto base = objects->value(static_cast<quint32>(baseId));
    require(base->getType() == GOType::Point && base->getMode() == Draw::Calculation
            && doc->getToolDraftBlockName(base->getIdTool()) == doc->getActiveDraftBlockName(),
            "Base point must belong to the active draft block.");
    require(arguments.value("length").isString() && arguments.value("angle").isString(),
            "length and angle must be formula strings (internal English syntax, decimal dot).");
    QString length = arguments.value("length").toString();
    QString angle = arguments.value("angle").toString();
    require(!length.isEmpty() && !angle.isEmpty() && length.size() <= 1024 && angle.size() <= 1024,
            "Formulas must contain 1 to 1024 characters.");
    // Preflight without CheckFormula's interactive error/fix dialogs.
    {
        Calculator calculator;
        const qreal lengthValue = calculator.EvalFormula(data->DataVariables(), length);
        const qreal angleValue = calculator.EvalFormula(data->DataVariables(), angle);
        require(std::isfinite(lengthValue) && lengthValue > 0 && std::isfinite(angleValue)
                && std::isfinite(qApp->toPixel(lengthValue)), "Length must be positive and formulas must evaluate to finite numbers.");
    }
    auto *point = VToolEndLine::Create(0, name, doc->getDefaultLineType(),
                                      QString::number(doc->getDefaultLineWeight()), doc->getDefaultLineColor(),
                                      length, angle, static_cast<quint32>(baseId), 5, 10, true,
                                      m_window->draftScene, doc, data, Document::FullParse, Source::FromGui);
    require(point != nullptr, "Point creation failed. Read state before continuing.");
    connect(point, &VToolEndLine::ToolTip, m_window, &MainWindow::setStatusMessage);
    const quint32 id = point->getId();
    m_window->setStatusMessage(QStringLiteral("MCP: created point %1").arg(name));
    return {{"state_token", stateToken()}, {"object", objectJson(id, data->GetGObject(id))},
            {"undo_text", stack->undoText()}};
}

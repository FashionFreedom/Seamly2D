 //  @file   vapplication.cpp
 //  @author Douglas S Caskey
 //  @date   7 Mar, 2024
 //
 //  @brief
 //  @copyright
 //  This source code is part of the Seamly2D project, a pattern making
 //  program to create and model patterns of clothing.
 //  Copyright (C) 2017-2024 Seamly2D project
 //  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 //
 //  Seamly2D is free software: you can redistribute it and/or modify
 //  it under the terms of the GNU General Public License as published by
 //  the Free Software Foundation, either version 3 of the License, or
 //  (at your option) any later version.
 //
 //  Seamly2D is distributed in the hope that it will be useful,
 //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 //  GNU General Public License for more details.
 //
 //  You should have received a copy of the GNU General Public License
 //  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.

/************************************************************************
 **
 **  @file   vapplication.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "application_2d.h"

#include "../mainwindow.h"
#include "../version.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../vmisc/logging.h"
#include "../vmisc/vmath.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vwidgets/vmaingraphicsview.h"

#include <Qt>
#include <QtDebug>
#include <QDir>
#include <QProcess>
#include <QTemporaryFile>
#include <QUndoStack>
#include <QTemporaryFile>
#include <QFile>
#include <QStandardPaths>
#include <QMessageBox>
#include <QThread>
#include <QDateTime>
#include <QtXmlPatterns>
#include <QIcon>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vApp, "v.application")

QT_WARNING_POP

Q_DECL_CONSTEXPR auto DAYS_TO_KEEP_LOGS = 3;

//---------------------------------------------------------------------------------------------------------------------
inline void noisyFailureMsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    // Why on earth didn't Qt want to make failed signal/slot connections qWarning?
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("::connect")))
    {
        type = QtWarningMsg;
    }

#if defined(V_NO_ASSERT)
    // I have decided to hide this annoing message for release builds.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QSslSocket: cannot resolve")))
    {
        type = QtDebugMsg;
    }

    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("setGeometry: Unable to set geometry")))
    {
        type = QtDebugMsg;
    }
#endif //defined(V_NO_ASSERT)

#if defined(Q_OS_MAC)
    // Hide Qt bug 'Assertion when reading an icns file'
    // https://bugreports.qt.io/browse/QTBUG-45537
    // Remove after Qt fix will be released
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QICNSHandler::read()")))
    {
        type = QtDebugMsg;
    }

    // See issue #568
    if (msg.contains(QStringLiteral("Error receiving trust for a CA certificate")))
    {
        type = QtDebugMsg;
    }
#endif

    // This is another one that doesn't make sense as just a debug message.  pretty serious
    // sign of a problem
    // http://www.developer.nokia.com/Community/Wiki/QPainter::begin:Paint_device_returned_engine_%3D%3D_0_(Known_Issue)
    if ((type == QtDebugMsg) && msg.contains(QStringLiteral("QPainter::begin"))
            && msg.contains(QStringLiteral("Paint device returned engine")))
    {
        type = QtWarningMsg;
    }

    // This qWarning about "Cowardly refusing to send clipboard message to hung application..."
    // is something that can easily happen if you are debugging and the application is paused.
    // As it is so common, not worth popping up a dialog.
    if ((type == QtWarningMsg) && msg.contains(QStringLiteral("QClipboard::event"))
            && msg.contains(QStringLiteral("Cowardly refusing")))
    {
        type = QtDebugMsg;
    }

    // Only the GUI thread should display message boxes.  If you are
    // writing a multithreaded application and the error happens on
    // a non-GUI thread, you'll have to queue the message to the GUI
    QCoreApplication *instance = QCoreApplication::instance();
    const bool isGuiThread = instance && (QThread::currentThread() == instance->thread());

    {
        QString debugdate = "[" + QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss"));

        switch (type)
        {
            case QtDebugMsg:
                debugdate += QString(":DEBUG:%1(%2)] %3: %4: %5").arg(context.file).arg(context.line)
                             .arg(context.function).arg(context.category).arg(msg);
                vStdOut()  <<  QApplication::translate("vNoisyHandler", "DEBUG:")  <<  msg  <<  "\n";
                break;
            case QtWarningMsg:
                debugdate += QString(":WARNING:%1(%2)] %3: %4: %5").arg(context.file).arg(context.line)
                             .arg(context.function).arg(context.category).arg(msg);
                vStdErr()  <<  QApplication::translate("vNoisyHandler", "WARNING:")  <<  msg  <<  "\n";
                break;
            case QtCriticalMsg:
                debugdate += QString(":CRITICAL:%1(%2)] %3: %4: %5").arg(context.file).arg(context.line)
                             .arg(context.function).arg(context.category).arg(msg);
                vStdErr()  <<  QApplication::translate("vNoisyHandler", "CRITICAL:")  <<  msg  <<  "\n";
                break;
            case QtFatalMsg:
                debugdate += QString(":FATAL:%1(%2)] %3: %4: %5").arg(context.file).arg(context.line)
                             .arg(context.function).arg(context.category).arg(msg);
                vStdErr()  <<  QApplication::translate("vNoisyHandler", "FATAL:")  <<  msg  <<  "\n";
                break;
            #if QT_VERSION > QT_VERSION_CHECK(5, 4, 2)
            case QtInfoMsg:
                debugdate += QString(":INFO:%1(%2)] %3: %4: %5").arg(context.file).arg(context.line)
                             .arg(context.function).arg(context.category).arg(msg);
                vStdOut()  <<  QApplication::translate("vNoisyHandler", "INFO:")  <<  msg  <<  "\n";
                break;
            #endif
            default:
                break;
        }

        (*qApp->logFile())  <<  debugdate  <<  Qt::endl;
    }

    if (isGuiThread)
    {
        // fixme: trying to make sure that no save/load dialogs are opened, because an error message
        // during them will lead to a crash
        const bool topWinAllowsPop = (QApplication::activeModalWidget() == nullptr) ||
                !QApplication::activeModalWidget()->inherits("QFileDialog");

        QMessageBox messageBox;

        switch (type)
        {
            case QtWarningMsg:
                messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Warning"));
                messageBox.setIcon(QMessageBox::Warning);
                break;
            case QtCriticalMsg:
                messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Critical Error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
            case QtFatalMsg:
                messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Fatal Error"));
                messageBox.setIcon(QMessageBox::Critical);
                break;
            #if QT_VERSION > QT_VERSION_CHECK(5, 4, 2)
            case QtInfoMsg:
                messageBox.setWindowTitle(QApplication::translate("vNoisyHandler", "Information"));
                messageBox.setIcon(QMessageBox::Information);
                break;
            #endif
            case QtDebugMsg:
            default:
                break;
        }

        if (type == QtWarningMsg || type == QtCriticalMsg || type == QtFatalMsg)
        {
            if (Application2D::isGUIMode())
            {
                if (topWinAllowsPop)
                {
                    messageBox.setText(msg);
                    messageBox.setStandardButtons(QMessageBox::Ok);
                    messageBox.setWindowModality(Qt::ApplicationModal);
                    messageBox.setModal(true);
                #ifndef QT_NO_CURSOR
                    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
                #endif
                    messageBox.setWindowFlags(messageBox.windowFlags() & ~Qt::WindowContextHelpButtonHint);
                    messageBox.exec();
                #ifndef QT_NO_CURSOR
                    QGuiApplication::restoreOverrideCursor();
                #endif
                }
            }
        }

        if (QtFatalMsg == type)
        {
            abort();
        }
    }
    else
    {
        if( QtDebugMsg != type && QtWarningMsg != type )
        {
            abort(); // be NOISY unless overridden!
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------

#define DefWidth 1.2//mm

// @brief Application2D constructor.
// @param argc number arguments.
// @param argv command line.
 Application2D::Application2D(int &argc, char **argv)
    : VAbstractApplication(argc, argv)
    , m_trVars(nullptr)
    , m_autoSaveTimer(nullptr)
    , m_lockLog()
    , m_out(nullptr)
{
    //setApplicationDisplayName(VER_PRODUCTNAME_STR);
    setApplicationName(VER_INTERNALNAME_STR);
    setOrganizationName(VER_COMPANYNAME_STR);
    setOrganizationDomain(VER_COMPANYDOMAIN_STR);
    // Setting the Application version
    setApplicationVersion(APP_VERSION_STR);

    openSettings();

    // making sure will create new instance...just in case we will ever do 2 objects of Application2D
    VCommandLine::Reset();
    loadTranslations(QLocale().name());// By default the console version uses system locale
    VCommandLine::Get(*this);
    undoStack = new QUndoStack(this);
}

//---------------------------------------------------------------------------------------------------------------------
Application2D::~Application2D()
{
    qCDebug(vApp, "Application closing.");
    qInstallMessageHandler(nullptr); // Restore the message handler
    delete m_trVars;
    VCommandLine::Reset();
}

// @brief startNewSeamly2D start Seamly2D in new process, send path to pattern file in argument.
// @param fileName path to pattern file.
void Application2D::startNewSeamly2D(const QString &fileName)
{
    qCDebug(vApp, "Open new detached process.");
    if (fileName.isEmpty())
    {
        qCDebug(vApp, "New process without arguments. program = %s",
                qUtf8Printable(QCoreApplication::applicationFilePath()));
        // Path can contain spaces.
        if (QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList()))
        {
            qCDebug(vApp, "The process was started successfully.");
        }
        else
        {
            qCWarning(vApp, "Could not run process. The operation timed out or an error occurred.");
        }
    }
    else
    {
        const QString run = QString("\"%1\" \"%2\"").arg(QCoreApplication::applicationFilePath()).arg(fileName);
        qCDebug(vApp, "New process with arguments. program = %s", qUtf8Printable(run));

        if (QProcess::startDetached(QCoreApplication::applicationFilePath(), QStringList{fileName}))
        {
            qCDebug(vApp, "The process was started successfully.");
        }
        else
        {
            qCWarning(vApp, "Could not run process. The operation timed out or an error occurred.");
        }
    }
}

// @brief notify Reimplemented from QApplication::notify().
// @param receiver receiver.
// @param event event.
// @return value that is returned from the receiver's event handler.
// reimplemented from QApplication so we can throw exceptions in slots
bool Application2D::notify(QObject *receiver, QEvent *event)
{
    try
    {
        return QApplication::notify(receiver, event);
    }
    catch (const VExceptionObjectError &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file. Program will be terminated.")), //-V807
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionBadId &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error bad id. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionConversionError &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionEmptyParameter &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionWrongId &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id. Program will be terminated.")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VExceptionToolWasDeleted &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s",
                   qUtf8Printable("Unhadled deleting tool. Continue use object after deleting"),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        exit(V_EX_DATAERR);
    }
    catch (const VException &error)
    {
        qCCritical(vApp, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Something's wrong!!")),
                   qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        return true;
    }
    // These last two cases are special. I found that we can't show a modal dialog here with an error message.
    // Somehow the program doesn't wait until an error dialog is closed, but if the exception is ignored
    // the program will hang.
    catch (const qmu::QmuParserError &error)
    {
        qCCritical(vApp, "%s", qUtf8Printable(tr("Parser error: %1. Program will be terminated.").arg(error.GetMsg())));
        exit(V_EX_DATAERR);
    }
    catch (std::exception &error)
    {
        qCCritical(vApp, "%s", qUtf8Printable(tr("Exception thrown: %1. Program will be terminated.").arg(error.what())));
        exit(V_EX_SOFTWARE);
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
QString Application2D::seamlyMeFilePath() const
{
    const QString seamlyme = QStringLiteral("seamlyme");
#ifdef Q_OS_WIN
    QFileInfo seamlymeFile(QCoreApplication::applicationDirPath() + "/" + seamlyme + ".exe");
    if (seamlymeFile.exists())
    {
        return seamlymeFile.absoluteFilePath();
    }
    else
    {
        return QCoreApplication::applicationDirPath() + "/../../seamlyme/bin/" + seamlyme + ".exe";
    }
#elif defined(Q_OS_MAC)
    QFileInfo seamlymeFile(QCoreApplication::applicationDirPath() + "/" + seamlyme);
    if (seamlymeFile.exists())
    {
        return seamlymeFile.absoluteFilePath();
    }
    else
    {
        QFileInfo file(QCoreApplication::applicationDirPath() + "/../../seamlyme/bin/" + seamlyme);
        if (file.exists())
        {
            return file.absoluteFilePath();
        }
        else
        {
            return seamlyme;
        }
    }
#else // Unix
    QFileInfo file(QCoreApplication::applicationDirPath() + "/../../seamlyme/bin/" + seamlyme);
    if (file.exists())
    {
        return file.absoluteFilePath();
    }
    else
    {
        QFileInfo seamlymeFile(QCoreApplication::applicationDirPath() + "/" + seamlyme);
        if (seamlymeFile.exists())
        {
            return seamlymeFile.absoluteFilePath();
        }
        else
        {
            return seamlyme;
        }
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
QString Application2D::logDirPath() const
{
#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    const QString logDirPath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(),
                                                      QStandardPaths::LocateDirectory) + "Seamly2D";
#else
    const QString logDirPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, QString(),
                                                      QStandardPaths::LocateDirectory)
            + QCoreApplication::organizationName();
#endif
    return logDirPath;
}

//---------------------------------------------------------------------------------------------------------------------
QString Application2D::logPath() const
{
    return QString("%1/seamly2d-pid%2.log").arg(logDirPath()).arg(applicationPid());
}

//---------------------------------------------------------------------------------------------------------------------
bool Application2D::createLogDir() const
{
    QDir logDir(logDirPath());
    if (logDir.exists() == false)
    {
        return logDir.mkpath("."); // Create directory for log if need
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void Application2D::beginLogging()
{
    VlpCreateLock(m_lockLog, logPath(), [this](){return new QFile(logPath());});

    if (m_lockLog->IsLocked())
    {
        if (m_lockLog->GetProtected()->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        {
            m_out.reset(new QTextStream(m_lockLog->GetProtected().get()));
            qInstallMessageHandler(noisyFailureMsgHandler);
            qCInfo(vApp, "Log file %s was locked.", qUtf8Printable(logPath()));
        }
        else
        {
            qCWarning(vApp, "Error opening log file \'%s\'. All debug output redirected to console.",
                    qUtf8Printable(logPath()));
        }
    }
    else
    {
        qCWarning(vApp, "Failed to lock %s", qUtf8Printable(logPath()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void Application2D::clearOldLogs() const
{
    QDir logsDir(logDirPath());
    logsDir.setNameFilters(QStringList("*.log"));
    logsDir.setCurrent(logDirPath());

    const QStringList allFiles = logsDir.entryList(QDir::NoDotAndDotDot | QDir::Files);
    if (allFiles.isEmpty() == false)
    {
        qCDebug(vApp, "Clearing old logs");
        for (int i = 0, sz = allFiles.size(); i < sz; ++i)
        {
            auto fn = allFiles.at(i);
            QFileInfo info(fn);
            if (info.birthTime().daysTo(QDateTime::currentDateTime()) >= DAYS_TO_KEEP_LOGS)
            {
                VLockGuard<QFile> tmp(info.absoluteFilePath(), [&fn](){return new QFile(fn);});
                if (tmp.GetProtected() != nullptr)
                {
                    if (tmp.GetProtected()->remove())
                    {
                        qCDebug(vApp, "Deleted %s", qUtf8Printable(info.absoluteFilePath()));
                    }
                    else
                    {
                        qCWarning(vApp, "Could not delete %s", qUtf8Printable(info.absoluteFilePath()));
                    }
                }
                else
                {
                    qCWarning(vApp, "Failed to lock %s", qUtf8Printable(info.absoluteFilePath()));
                }
            }
        }
    }
    else
    {
        qCDebug(vApp, "There are no old logs.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void Application2D::initOptions()
{
    // Run creation log after sending crash report
    startLogging();

    qInfo() << "Version:" << APP_VERSION_STR;
    qInfo() << "Build revision:" << BUILD_REVISION;
    qInfo() << buildCompatibilityString();
    qInfo() << "Built on" << __DATE__ << "at" << __TIME__;
    qInfo() << "Command-line arguments:" << arguments();
    qInfo() << "Process ID:" << applicationPid();

    if (Application2D::isGUIMode())// By default console version uses system locale
    {
        loadTranslations(Seamly2DSettings()->getLocale());
    }

    static const char * GENERIC_ICON_TO_CHECK = "document-open";
    if (QIcon::hasThemeIcon(GENERIC_ICON_TO_CHECK) == false)
    {
        //If there is no default working icon theme then we should
        //use an icon theme that we provide via a .qrc file
        //This case happens under Windows and Mac OS X
        //This does not happen under GNOME or KDE
        QIcon::setThemeName("win.icon.theme");
    }

    openSettings();
    VSettings *settings = Seamly2DSettings();
    QDir().mkpath(settings->getDefaultLayoutPath());
    QDir().mkpath(settings->getDefaultPatternPath());
    QDir().mkpath(settings->getDefaultIndividualSizePath());
    QDir().mkpath(settings->getDefaultMultisizePath());
    QDir().mkpath(settings->getDefaultTemplatePath());
    QDir().mkpath(settings->getDefaultLabelTemplatePath());
    QDir().mkpath(settings->getDefaultBackupFilePath());
}

//---------------------------------------------------------------------------------------------------------------------
QStringList Application2D::pointNameLanguages()
{
    QStringList list = QStringList()  <<  "de" // German
                                      <<  "en" // English
                                      <<  "fr" // French
                                      <<  "ru" // Russian
                                      <<  "uk" // Ukrainian
                                      <<  "hr" // Croatian
                                      <<  "sr" // Serbian
                                      <<  "bs"; // Bosnian
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
void Application2D::startLogging()
{
    if (createLogDir())
    {
        beginLogging();
        clearOldLogs();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QTextStream *Application2D::logFile()
{
    return m_out.get();
}

//---------------------------------------------------------------------------------------------------------------------
const VTranslateVars *Application2D::translateVariables()
{
    return m_trVars;
}

//---------------------------------------------------------------------------------------------------------------------
void Application2D::initTranslateVariables()
{
    if (m_trVars == nullptr)
    {
        m_trVars = new VTranslateVars();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool Application2D::event(QEvent *event)
{
    switch(event->type())
    {
        // In Mac OS X the QFileOpenEvent event is generated when user performs "Open With" from Finder (this event is
        // Mac specific).
        case QEvent::FileOpen:
        {
            QFileOpenEvent *fileOpenEvent = static_cast<QFileOpenEvent *>(event);
            const QString macFileOpen = fileOpenEvent->file();
            if(!macFileOpen.isEmpty())
            {
                MainWindow *window = qobject_cast<MainWindow*>(mainWindow);
                if (window)
                {
                    window->LoadPattern(macFileOpen);  // open file in existing window
                }
                return true;
            }
            break;
        }
#if defined(Q_OS_MAC)
        case QEvent::ApplicationActivate:
        {
            if (mainWindow && not mainWindow->isMinimized())
            {
                mainWindow->show();
            }
            return true;
        }
#endif //defined(Q_OS_MAC)
        default:
            return VAbstractApplication::event(event);
    }
    return VAbstractApplication::event(event);
}


// @brief openSettings get acsses to application settings.
// Because we can create object in constructor we open file separately.
void Application2D::openSettings()
{
    settings = new VSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                             QCoreApplication::applicationName(), this);
}

//---------------------------------------------------------------------------------------------------------------------
VSettings *Application2D::Seamly2DSettings()
{
    SCASSERT(settings != nullptr)
    return qobject_cast<VSettings *>(settings);
}

//---------------------------------------------------------------------------------------------------------------------
bool Application2D::isGUIMode()
{
    return (VCommandLine::instance != nullptr) && VCommandLine::instance->IsGuiEnabled();
}

// @brief isAppInGUIMode little hack that allows to have access to application state from VAbstractApplication class.
bool Application2D::isAppInGUIMode() const
{
    return isGUIMode();
}

//---------------------------------------------------------------------------------------------------------------------
const VCommandLinePtr Application2D::commandLine() const
{
    return VCommandLine::instance;
}
//---------------------------------------------------------------------------------------------------------------------

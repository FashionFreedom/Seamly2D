//---------------------------------------------------------------------------------------------------------------------
//   @file   fvupdater.cpp
//   @author Douglas S Caskey
//   @date   17 Sep, 2023
//
//   @copyright
//   Copyright (C) 2017 - 2025 Seamly, LLC
//   https://github.com/fashionfreedom/seamly2d
//
//   @brief
//   Seamly2D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Seamly2D is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
// ---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//   @file   fvupdater.cpp
//   @copyright
//   Copyright (c) 2012 Linas Valiukas and others.
//
//   @brief
//   Permission is hereby granted, free of charge, to any person obtaining a copy of this
//   software and associated documentation files (the "Software"), to deal in the Software
//   without restriction, including without limitation the rights to use, copy, modify,
//   merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//   permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//   The above copyright notice and this permission notice shall be included in all copies or
//   substantial portions of the Software.
//
//   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//   NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//   DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------

#include "fvupdater.h"

#include <qsystemdetection.h>
#include <qxmlstream.h>
#include <QApplication>
#include <QByteArray>
#include <QDate>
#include <QDesktopServices>
#include <QLatin1String>
#include <QMessageBox>
#include <QMessageLogger>
#include <QMutex>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QDir>
#include <QProcess>
#include <QtConcurrent/QtConcurrent>
#include "../ifc/exception/vexception.h"
#include "../ifc/xml/abstract_converter.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

const QString defaultFeedURL = QStringLiteral("https://api.github.com/repos/FashionFreedom/Seamly2D/releases/latest");

QPointer<FvUpdater> FvUpdater::m_Instance;

//---------------------------------------------------------------------------------------------------------------------
FvUpdater *FvUpdater::sharedUpdater()
{
	static QMutex mutex;
	if (!m_Instance)
    {
        mutex.lock();
		m_Instance = new FvUpdater;
		mutex.unlock();
	}

	return m_Instance.data();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::drop()
{
	static QMutex mutex;
    mutex.lock();
	delete m_Instance;
    m_Instance = nullptr;
	mutex.unlock();
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::FvUpdater()
	: QObject(nullptr)
    , m_silentMode(true)
    , m_feedURL()
    , m_networkAccessManager()
    , m_reply(nullptr)
    , m_httpRequestAborted(false)
    , m_dropOnFinish(true)
{
	// no op
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::~FvUpdater()
{
	delete m_reply;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::setFeedURL(const QUrl &feedURL)
{
    m_feedURL = feedURL;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::setFeedURL(const QString &feedURL)
{
	setFeedURL(QUrl(feedURL));
}

//---------------------------------------------------------------------------------------------------------------------
QString FvUpdater::getFeedURL() const
{
    return m_feedURL.toString();
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::isDropOnFinish() const
{
    return m_dropOnFinish;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::setDropOnFinish(bool value)
{
    m_dropOnFinish = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::checkForUpdates(bool silentMode)
{
	if (m_feedURL.isEmpty())
    {
		qCritical() << "Please set feed URL via setFeedURL() before calling checkForUpdates().";
		return false;
	}

	m_silentMode = silentMode;

	// Check if application's organization name and domain are set, fail
	// otherwise (nowhere to store QSettings to)
	if (QCoreApplication::organizationName().isEmpty())
    {
		qCritical() << "QApplication::organizationName is not set. Please do that.";
		return false;
	}
	if (QCoreApplication::organizationDomain().isEmpty())
    {
		qCritical() << "QApplication::organizationDomain is not set. Please do that.";
		return false;
	}

	// Set application name / version is not set yet
	if (QCoreApplication::applicationName().isEmpty())
    {
		qCritical() << "QApplication::applicationName is not set. Please do that.";
		return false;
	}

	if (QCoreApplication::applicationVersion().isEmpty())
    {
		qCritical() << "QApplication::applicationVersion is not set. Please do that.";
		return false;
	}

	cancelDownloadFeed();
	m_httpRequestAborted = false;
	startDownloadFeed(m_feedURL);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::checkForUpdatesSilent()
{
	if (qApp->Settings()->GetDateOfLastRemind().daysTo(QDate::currentDate()) >= 1)
    {
		const bool success = checkForUpdates(true);
		if (m_dropOnFinish && !success)
        {
			drop();
		}
		return success;
	}
    else
    {
		if (m_dropOnFinish)
        {
			drop();
		}
		return true;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::checkForUpdatesNotSilent()
{
	const bool success = checkForUpdates(false);
	if (m_dropOnFinish && !success)
    {
		drop();
	}
	return success;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::httpFeedDownloadFinished()
{
	if (m_httpRequestAborted)
    {
		m_reply->deleteLater();
		return;
	}

	const QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

    if (m_reply->error() != QNetworkReply::NoError)
    {
		// Error.
        showMessageBox(QMessageBox::Critical, tr("Error"),
                       tr("Feed download failed: %1.").arg(m_reply->errorString()), false);
	}
    else if (!redirectionTarget.isNull())
    {
		const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());

		m_feedURL = newUrl;
		m_reply->deleteLater();

		startDownloadFeed(m_feedURL);
		return;
	}
    else
    {
		auto jsonDoc = QJsonDocument::fromJson(m_reply->readAll());
        qDebug() << "Response is a JSON object:" << jsonDoc.isObject();
        if (jsonDoc.isObject())
        {
            auto tag = jsonDoc.object()["tag_name"].toString();
            qDebug() << "Found the following tag" << tag;

            if (!releaseIsNewer(tag))
            {
                showMessageBox(QMessageBox::Information, tr("Information"), tr("No new releases available."), false);
                return;
            }
            if (showConfirmationDialog(tr("A new release %1 is available.\nDo you want to download it?").arg(tag), true))
                getPLatformSpecificInstaller(jsonDoc.object()["assets"].toArray());
            return;
		}
	}

	m_reply->deleteLater();

	if (m_dropOnFinish)
    {
		drop();
	}
}

void FvUpdater::networkError(QNetworkReply::NetworkError)
{
	emit setProgress(100);
	m_fileSize = 0;
    showMessageBox(QMessageBox::Critical, tr("Error"), m_reply->errorString(), false);
}

void FvUpdater::getFileSize()
{
	auto fileSizeHeader = m_reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	if (m_fileSize == 0 && fileSizeHeader > 1000000)
    {
		m_fileSize = fileSizeHeader;
	}
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::startDownloadFeed(const QUrl &url)
{
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_networkAccessManager.get(request);

	connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesRead, qint64 totalBytes)
    {
		Q_UNUSED(bytesRead)
		Q_UNUSED(totalBytes)

		if (m_httpRequestAborted)
        {
			return;
		}
	});

	connect(m_reply, &QNetworkReply::finished, this, &FvUpdater::httpFeedDownloadFinished);
}

void FvUpdater::startDownloadFile(QUrl url, QString name)
{
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_networkAccessManager.get(request);

	connect(m_reply, &QNetworkReply::metaDataChanged, this, &FvUpdater::getFileSize);

	QDir downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
	downloadDir.mkdir(m_releaseName);
	downloadDir.cd(m_releaseName);

	auto downloadedFile = new QFile(downloadDir.filePath(name), this);
	if(downloadedFile->exists() && !downloadedFile->remove())
    {
        showMessageBox(QMessageBox::Critical, tr("Error"),
		               tr("Unable to get exclusive access to file\n%1\nPossibly the file is already being downloaded.").arg(downloadDir.filePath(name)), false);
		return;
	}

	bool isOpen = downloadedFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (!isOpen)
    {
        showMessageBox(QMessageBox::Critical, tr("Error"),
		               tr("Unable to open file\n%1\nfor writing").arg(downloadDir.filePath(name)), false);
		return;
	}

	connect(m_reply, &QNetworkReply::readyRead, this, [this, downloadedFile]()
    {
		// this slot gets called every time the QNetworkReply has new data.
		// We read all of its new data and write it into the file.
		// That way we use less RAM than when reading it at the finished()
		// signal of the QNetworkReply
		downloadedFile->write(m_reply->readAll());
		int progress = int(downloadedFile->size() * 100 / m_fileSize);
		emit setProgress(progress);
	});

    connect(m_reply, &QNetworkReply::errorOccurred, this, &FvUpdater::networkError);

	connect(m_reply, &QNetworkReply::downloadProgress, this, [this]()
    {
		if (m_httpRequestAborted)
        {
			return;
		}
	});

	connect(m_reply, &QNetworkReply::finished, this, [=]()
    {
		fileDownloadFinished(downloadedFile, name);
	});
}

void FvUpdater::fileDownloadFinished(QFile *downloadedFile, QString name)
{
	if (m_httpRequestAborted)
    {
		m_reply->deleteLater();
		return;
	}

	const QVariant redirectionTarget = m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

	if (m_reply->error() != QNetworkReply::NoError)
    {
		// Show error.
        showMessageBox(QMessageBox::Critical, tr("Error"),
                       tr("File download failed: %1.").arg(m_reply->errorString()), false);
	}
    else if (!redirectionTarget.isNull())
    {
		downloadedFile->close();
		const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());
		m_reply->deleteLater();
		startDownloadFile(newUrl, name);
		return;
	}
    else
    {
		emit setProgress(100); //just in case
		m_fileSize = 0;
		downloadedFile->write(m_reply->readAll());
		downloadedFile->close();
		auto fileInfo = QFileInfo(*downloadedFile);

#ifdef Q_OS_LINUX
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.dir().absolutePath()));
		QProcess proc;
		auto	 res = proc.startDetached(QDir::toNativeSeparators(fileInfo.absoluteFilePath()), QStringList());
		auto	 err = proc.error();
		qDebug() << res << " " << err;
#else
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absoluteFilePath()));
#endif
		downloadedFile->deleteLater();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::cancelDownloadFeed()
{
	if (m_reply)
    {
		m_httpRequestAborted = true;
		m_reply->abort();
	}
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showConfirmationDialog show a confirmatiom dialog.
///
/// This method displays a dialog a confirmatiom message box and returns a yes or no answer.
///
/// @param message This property holds the informative text that provides a fuller description for the message
/// @param showEvenInSilentMode Sets the given option to show dialog or not.
///     - true always show dialog.
///     - false donlt show dialog in silent mode.
///
/// @return bool yes or no.
//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::showConfirmationDialog(const QString &message, bool showEvenInSilentMode)
{
    if (m_silentMode && !showEvenInSilentMode)
    {
		// Don't show confirmation dialog in the silent mode
        return false;
	}

	QMessageBox confirmationMsgBox;
	confirmationMsgBox.setIcon(QMessageBox::Information);
	confirmationMsgBox.setText(tr("Information"));
	confirmationMsgBox.setInformativeText(message);
	confirmationMsgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

	return QMessageBox::Yes == confirmationMsgBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showMessageBox Show a message box.
///
/// This method displays a message box with an icon, text and informative text based on a boolean option.
///
/// @param icon QMessageBox This property holds the message box's icon.
/// @param text This property holds the message box text to be displayed.
/// @param message This property holds the informative text that provides a fuller description for the message
/// @param showEvenInSilentMode Sets the given option to show message box or not.
///     - true always show message box.
///     - false don't show message box in silent mode.
//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showMessageBox(QMessageBox::Icon icon, const QString &text,
                               const QString &message, bool showEvenInSilentMode)
{
    if (m_silentMode && !showEvenInSilentMode)
    {
        // Don't show MessageBox in the silent mode
        return;
    }

    QMessageBox messageBox;
	messageBox.setIcon(icon);
	messageBox.setText(text);
	messageBox.setInformativeText(message);
	messageBox.exec();
}

bool FvUpdater::releaseIsNewer(const QString &releaseTag)
{
	const auto releaseVersion = releaseTag.mid(1).split('.');
	const auto currentVersion = QCoreApplication::applicationVersion().split('.');

	for (int i = 0; i < releaseVersion.length(); i++)
    {
		if (releaseVersion[i].toInt() > currentVersion[i].toInt())
        {
			return (m_releaseName = releaseTag), true;
        }
	}
	return false;
}

void FvUpdater::getPLatformSpecificInstaller(QJsonArray assets)
{
	qDebug() << "current application version" << QCoreApplication::applicationVersion();

    QString searchPattern = "Unknown";
#if defined(Q_OS_LINUX)   // Defined on Linux
	searchPattern = "AppImage";
#elif defined(Q_OS_MAC)   // Defined on macOS
    searchPattern = "macos";
#elif defined(Q_OS_WIN) && defined(Q_PROCESSOR_ARM_64)  // Defined on Windows ARM 64-bit only.
	// The artifact does not include the full name "windows" so that is does not accidentally
	// match what the old intel versions search for.
	searchPattern = "win-arm64";
#elif defined(Q_OS_WIN) // Defined on Windows 64-bit only.
	searchPattern = "windows";
#endif

	for (const QJsonValueRef asset : assets)
    {
		auto name = asset.toObject()["name"].toString();
		qDebug() << "Checking" << searchPattern << "against" << name;
		if (name.contains(searchPattern, Qt::CaseInsensitive))
        {
			QUrl downloadableUrl = asset.toObject()["browser_download_url"].toString();
			startDownloadFile(downloadableUrl, name);
            showMessageBox(QMessageBox::Information, tr("Information"),
                           tr("Download has started, the installer will open once it's finished downloading"), false);
		}
	}
}

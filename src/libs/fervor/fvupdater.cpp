/***************************************************************************
 **  @file   fvupdater.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/***************************************************************************************************
 **  @file   fvupdater.cpp
 **
 **  @brief
 **  @copyright
 **  Copyright (c) 2012 Linas Valiukas and others.
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

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
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QStringList>
#include <QStringRef>
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
FvUpdater *FvUpdater::sharedUpdater() {
	static QMutex mutex;
	if (m_Instance.isNull()) {
        mutex.lock();
		m_Instance = new FvUpdater;
		mutex.unlock();
	}

	return m_Instance.data();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::drop() {
	static QMutex mutex;
    mutex.lock();
	delete m_Instance;
	mutex.unlock();
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::FvUpdater()
	: QObject(nullptr),
	  m_silentAsMuchAsItCouldGet(true), m_feedURL(), m_qnam(), m_reply(nullptr),
	  m_httpRequestAborted(false), m_dropOnFinish(true) {
	// noop
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::~FvUpdater() {
	delete m_reply;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetFeedURL(const QUrl &feedURL) { m_feedURL = feedURL; }

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetFeedURL(const QString &feedURL) {
	SetFeedURL(QUrl(feedURL));
}

//---------------------------------------------------------------------------------------------------------------------
QString FvUpdater::GetFeedURL() const { return m_feedURL.toString(); }

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::IsDropOnFinish() const { return m_dropOnFinish; }

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetDropOnFinish(bool value) { m_dropOnFinish = value; }

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::CheckForUpdates(bool silentAsMuchAsItCouldGet) {
	if (m_feedURL.isEmpty()) {
		qCritical()
			<< "Please set feed URL via setFeedURL() before calling CheckForUpdates().";
		return false;
	}

	m_silentAsMuchAsItCouldGet = silentAsMuchAsItCouldGet;

	// Check if application's organization name and domain are set, fail
	// otherwise (nowhere to store QSettings to)
	if (QCoreApplication::organizationName().isEmpty()) {
		qCritical()
			<< "QApplication::organizationName is not set. Please do that.";
		return false;
	}
	if (QCoreApplication::organizationDomain().isEmpty()) {
		qCritical()
			<< "QApplication::organizationDomain is not set. Please do that.";
		return false;
	}

	// Set application name / version is not set yet
	if (QCoreApplication::applicationName().isEmpty()) {
		qCritical()
			<< "QApplication::applicationName is not set. Please do that.";
		return false;
	}

	if (QCoreApplication::applicationVersion().isEmpty()) {
		qCritical()
			<< "QApplication::applicationVersion is not set. Please do that.";
		return false;
	}

	cancelDownloadFeed();
	m_httpRequestAborted = false;
	startDownloadFeed(m_feedURL);

	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::CheckForUpdatesSilent() {
	if (qApp->Settings()->GetDateOfLastRemind().daysTo(QDate::currentDate())
		>= 1) {
		const bool success = CheckForUpdates(true);
		if (m_dropOnFinish && not success) {
			drop();
		}
		return success;
	} else {
		if (m_dropOnFinish) {
			drop();
		}
		return true;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::CheckForUpdatesNotSilent() {
	const bool success = CheckForUpdates(false);
	if (m_dropOnFinish && not success) {
		drop();
	}
	return success;
}

void FvUpdater::getFileSize() {
	auto fileSizeHeader = m_reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	if (m_fileSize == 0 && fileSizeHeader > 1000000) {
		m_fileSize = fileSizeHeader;
	}
}

void FvUpdater::startDownloadFile(QUrl url, QString name) {
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader,
					  QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
					  QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_qnam.get(request);
	connect(m_reply, SIGNAL(metaDataChanged()), this, SLOT(getFileSize()));
	QDir downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
	downloadDir.mkdir(m_releaseName);
	downloadDir.cd(m_releaseName);
	auto downloadedFile = new QFile(downloadDir.filePath(name), this);
	if(downloadedFile->exists() && !downloadedFile->remove()){
		showErrorDialog(tr("Unable to get exclusive access to file \n%1\nPossibly the file is already being downloaded.").arg(downloadDir.filePath(name)), false);
		return;
	}
	bool isOpen = downloadedFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (!isOpen) {
		showErrorDialog(tr("Unable to open file\n%1\nfor writing").arg(downloadDir.filePath(name)), false);
		return;
	}
	connect(m_reply.data(), &QNetworkReply::readyRead, this, [this, downloadedFile]() {
		// this slot gets called every time the QNetworkReply has new data.
		// We read all of its new data and write it into the file.
		// That way we use less RAM than when reading it at the finished()
		// signal of the QNetworkReply
		downloadedFile->write(m_reply->readAll());
		int progress = int(downloadedFile->size() * 100 / m_fileSize);
		setProgress(progress);
	});

	connect(m_reply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(networkError(QNetworkReply::NetworkError)));


	connect(m_reply.data(), &QNetworkReply::downloadProgress, this,
			[this](qint64 bytesRead, qint64 totalBytes) {
				Q_UNUSED(bytesRead)
				Q_UNUSED(totalBytes)

				if (m_httpRequestAborted) {
					return;
				}
			});
	connect(m_reply.data(), &QNetworkReply::finished, this, [=]() {
		fileDownloadFinished(downloadedFile, name);
	});
}
void FvUpdater::fileDownloadFinished(QFile *downloadedFile, QString name) {
	if (m_httpRequestAborted) {
		m_reply->deleteLater();
		return;
	}

	const QVariant redirectionTarget =
		m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (m_reply->error() != QNetworkReply::NoError) {
		// Error.
		showErrorDialog(
			tr("File download failed: %1.").arg(m_reply->errorString()), false);
	} else if (not redirectionTarget.isNull()) {
		downloadedFile->close();
		const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());
		m_reply->deleteLater();
		showInformationDialog(tr("Download has started, the installer will open once it's finished downloading"), false);
		startDownloadFile(newUrl, name);
		return;
	} else {
		setProgress(100); //just in case
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
void FvUpdater::startDownloadFeed(const QUrl &url) {
	// m_xml.clear();

	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader,
					  QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
					  QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_qnam.get(request);

	connect(m_reply.data(), &QNetworkReply::downloadProgress, this,
			[this](qint64 bytesRead, qint64 totalBytes) {
				Q_UNUSED(bytesRead)
				Q_UNUSED(totalBytes)

				if (m_httpRequestAborted) {
					return;
				}
			});
	connect(m_reply.data(), &QNetworkReply::finished, this,
			&FvUpdater::httpFeedDownloadFinished);
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::cancelDownloadFeed() {
	if (m_reply) {
		m_httpRequestAborted = true;
		m_reply->abort();
	}
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::httpFeedDownloadFinished() {
	if (m_httpRequestAborted) {
		m_reply->deleteLater();
		return;
	}

	const QVariant redirectionTarget =
		m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
	if (m_reply->error() != QNetworkReply::NoError) {
		// Error.
		showErrorDialog(
			tr("Feed download failed: %1.").arg(m_reply->errorString()), false);
	} else if (not redirectionTarget.isNull()) {
		const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());

		m_feedURL = newUrl;
		m_reply->deleteLater();

		startDownloadFeed(m_feedURL);
		return;
	} else {
		auto jsonDoc = QJsonDocument::fromJson(m_reply->readAll());
        qDebug() << "Response is a JSON object:" << jsonDoc.isObject();
        if (jsonDoc.isObject()) {
            auto tag = jsonDoc.object()["tag_name"].toString();
            qDebug() << "Found the following tag" << tag;

            if (!releaseIsNewer(tag)) {
                showInformationDialog(tr("No new releases available."));
                return;
            }
            if (showConfirmationDialog(tr("A new release %1 is available.\nDo you want to download it?").arg(tag), true))
                getPLatformSpecificInstaller(jsonDoc.object()["assets"].toArray());
            return;
		}
	}

	m_reply->deleteLater();

	if (m_dropOnFinish) {
		drop();
	}
}

void FvUpdater::getPLatformSpecificInstaller(QJsonArray assets) {
	qDebug() << "current application version" << QCoreApplication::applicationVersion();

#ifdef Q_OS_LINUX // Defined on Linux.
	auto searchPattern = "AppImage";
#else
	#ifdef Q_OS_MACOS // Defined on macOS
		auto searchPattern = "macos";
	#else
		#ifdef Q_OS_WIN64 // Defined on Windows 64-bit only.
			auto searchPattern = "windows";
		#else // Only windows 32-bit left
			auto searchPattern = "win32";
		#endif
	#endif
#endif

	for (const QJsonValueRef asset : assets) {
		auto name = asset.toObject()["name"].toString();
		qDebug() << "Checking" << searchPattern << "against" << name;
		if (name.contains(searchPattern,
						  Qt::CaseInsensitive)) {
			QUrl downloadableUrl =
				asset.toObject()["browser_download_url"].toString();
			startDownloadFile(downloadableUrl, name);
		}
	}
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showErrorDialog(const QString &message,
								bool		   showEvenInSilentMode) {
	if (m_silentAsMuchAsItCouldGet) {
		if (not showEvenInSilentMode) {
			// Don't show errors in the silent mode
			return;
		}
	}

	QMessageBox dlFailedMsgBox;
	dlFailedMsgBox.setIcon(QMessageBox::Critical);
	dlFailedMsgBox.setText(tr("Error"));
	dlFailedMsgBox.setInformativeText(message);
	dlFailedMsgBox.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showInformationDialog(const QString &message,
									  bool			 showEvenInSilentMode) {
	if (m_silentAsMuchAsItCouldGet) {
		if (not showEvenInSilentMode) {
			// Don't show information dialogs in the silent mode
			return;
		}
	}

	QMessageBox dlInformationMsgBox;
	dlInformationMsgBox.setIcon(QMessageBox::Information);
	dlInformationMsgBox.setText(tr("Information"));
	dlInformationMsgBox.setInformativeText(message);
	dlInformationMsgBox.exec();
}

bool FvUpdater::showConfirmationDialog(const QString &message,
									   bool			  showEvenInSilentMode) {
	if (m_silentAsMuchAsItCouldGet) {
		if (not showEvenInSilentMode) {
			// Don't show information dialogs in the silent mode
			return false;
		}
	}

	QMessageBox dlInformationMsgBox;
	dlInformationMsgBox.setIcon(QMessageBox::Information);
	dlInformationMsgBox.setText(tr("Information"));
	dlInformationMsgBox.setInformativeText(message);
	dlInformationMsgBox.setStandardButtons(QMessageBox::Yes
										   | QMessageBox::No);

	return QMessageBox::Yes == dlInformationMsgBox.exec();
}


bool FvUpdater::releaseIsNewer(const QString &releaseTag) {
	const auto releaseVersion = releaseTag.mid(1).split('.');
	const auto currentVersion = QCoreApplication::applicationVersion().split('.');
	for (int i = 0; i < releaseVersion.length(); i++) {
		if (releaseVersion[i].toInt() > currentVersion[i].toInt())
			return (m_releaseName = releaseTag), true;
	}
	return false;
}

void FvUpdater::networkError(QNetworkReply::NetworkError) {
	setProgress(100);
	m_fileSize = 0;
	showErrorDialog(m_reply->errorString(), false);
}

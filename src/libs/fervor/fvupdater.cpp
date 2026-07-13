//-------------------------------------------------------------------------------------------------
//  @file   fvupdater_dialog.cpp
//  @author Douglas S Caskey
//  @date   31 May, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
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
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//  @file   fvupdater.cpp
//
//  @brief
//  @copyright
//  Copyright (c) 2012 Linas Valiukas and others.
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this
//  software and associated documentation files (the "Software"), to deal in the Software
//  without restriction, including without limitation the rights to use, copy, modify,
//  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or
//  substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
//  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
//  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//-------------------------------------------------------------------------------------------------

#include "fvupdater.h"
#include "updater_dialog.h"
#include "fvignoredversions.h"
#include "fvavailableupdate.h"

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

#define SILENT true
#define NOT_SILENT false

QPointer<FvUpdater> FvUpdater::m_Instance;

//---------------------------------------------------------------------------------------------------------------------
FvUpdater *FvUpdater::sharedUpdater()
{
	static QMutex mutex;
	if (m_Instance.isNull())
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
	mutex.unlock();
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::FvUpdater()
	: QObject(nullptr)
	, m_isSilentMode(true)
    , m_feedURL()
    , m_qnam()
    , m_reply(nullptr)
    , m_httpRequestAborted(false)
    , m_dropOnFinish(false)
    , m_updateVersion(QString(""))
{
	// noop
}

//---------------------------------------------------------------------------------------------------------------------
FvUpdater::~FvUpdater()
{
	delete m_reply;

    hideUpdaterDialog();
}


void FvUpdater::hideUpdaterDialog()
{
	if (m_updaterDialog)
    {
		if (!m_updaterDialog->close())
        {
			qWarning() << "Update window didn't close, leaking memory from now on";
		}

		// not deleting because of Qt::WA_DeleteOnClose

		m_updaterDialog = nullptr;
	}
}

void FvUpdater::updaterDialogWasClosed()
{
	// (Re-)nullify a pointer to a destroyed QWidget or you're going to have a bad time.
	m_updaterDialog = nullptr;
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
void FvUpdater::setParent(QWidget *parent)
{
    m_parent = parent;
}

void FvUpdater::downloadUpdate()
{
	qDebug() << "Download update";

}

void FvUpdater::skipUpdate()
{
	qDebug() << "Skip update";

	if (m_updateVersion.isEmpty())
    {
		qWarning() << "Proposed update is Empty (shouldn't be at this point)";
		return;
	}

	// Start ignoring this particular version
    qApp->Settings()->setLatestSkippedVersion(m_updateVersion);

	hideUpdaterDialog();
}

void FvUpdater::remindLater()
{
	qDebug() << "Remind me later";

	hideUpdaterDialog();
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::dropOnFinish() const
{
    return m_dropOnFinish;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::setDropOnFinish(bool value)
{
    m_dropOnFinish = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::checkForUpdates(bool isSilentMode)
{
	if (m_feedURL.isEmpty())
    {
		qCritical()
			<< "Please set feed URL via setFeedURL() before calling checkForUpdates().";
		return false;
	}

	m_isSilentMode = isSilentMode;

	// Check if application's organization name and domain are set, fail
	// otherwise (nowhere to store QSettings to)
	if (QCoreApplication::organizationName().isEmpty())
    {
		qCritical()
			<< "QApplication::organizationName is not set. Please do that.";
		return false;
	}
	if (QCoreApplication::organizationDomain().isEmpty())
    {
		qCritical()
			<< "QApplication::organizationDomain is not set. Please do that.";
		return false;
	}

	// Set application name / version is not set yet
	if (QCoreApplication::applicationName().isEmpty())
    {
		qCritical()
			<< "QApplication::applicationName is not set. Please do that.";
		return false;
	}

	if (QCoreApplication::applicationVersion().isEmpty())
    {
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
bool FvUpdater::checkForUpdatesSilent()
{
	if (qApp->Settings()->getDateOfLastRemind().daysTo(QDate::currentDate()) >= 1)
    {
		const bool success = checkForUpdates(SILENT);
		if (m_dropOnFinish && !success)
        {
			drop();
		}
		return success;
	}

    if (m_dropOnFinish)
    {
		drop();
	}
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::checkForUpdatesNotSilent()
{
	const bool success = checkForUpdates(NOT_SILENT);
	if (m_dropOnFinish && !success)
    {
		drop();
	}
	return success;
}

void FvUpdater::getFileSize()
{
	auto fileSizeHeader = m_reply->header(QNetworkRequest::ContentLengthHeader).toInt();
	if (m_fileSize == 0 && fileSizeHeader > 1000000)
    {
		m_fileSize = fileSizeHeader;
	}
}

void FvUpdater::parseJson()
{
    QJsonDocument jsonDoc = QJsonDocument::fromJson(m_reply->readAll());
    qDebug() << "Response is a JSON object:" << jsonDoc.isObject();

    if (jsonDoc.isObject())
    {
        m_updateVersion = jsonDoc.object()["tag_name"].toString();
        qDebug() << "Found the following version" << m_updateVersion;

        if (!releaseIsNewer(m_updateVersion))
        {
            showInformationDialog(tr("No new releases available."));
            return;
        }
        QString message = tr("%1 %2 is now available - you have %3.").arg(QGuiApplication::applicationDisplayName(),
                                                                          m_updateVersion,
                                                                          QCoreApplication::applicationVersion());

        if (showUpdaterDialog(message))
        {
            getPLatformSpecificInstaller(jsonDoc.object()["assets"].toArray());
        }
        return;
    }
}

void FvUpdater::startDownloadFile(QUrl url, QString name)
{
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader,
					  QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader,
					  QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_qnam.get(request);

	connect(m_reply, &QNetworkReply::metaDataChanged, this, &FvUpdater::getFileSize);

    QDir downloadDir = m_updaterDialog->getDownloadDirectory();
	downloadDir.mkdir(m_releaseName);
	downloadDir.cd(m_releaseName);
	auto downloadedFile = new QFile(downloadDir.filePath(name), this);

    if(downloadedFile->exists() && !downloadedFile->remove())
    {
		showErrorDialog(tr("Unable to get exclusive access to file\n%1\nPossibly the file is already being downloaded.").arg(downloadDir.filePath(name)));
		return;
	}
	bool isOpen = downloadedFile->open(QIODevice::WriteOnly | QIODevice::Truncate);

    if (!isOpen)
    {
		showErrorDialog(tr("Unable to open file\n%1\nfor writing").arg(downloadDir.filePath(name)));
		return;
	}
	connect(m_reply.data(), &QNetworkReply::readyRead, this, [this, downloadedFile]()
    {
		// this slot gets called every time the QNetworkReply has new data.
		// We read all of its new data and write it into the file.
		// That way we use less RAM than when reading it at the finished()
		// signal of the QNetworkReply
		downloadedFile->write(m_reply->readAll());
		int progress = int(downloadedFile->size() * 100 / m_fileSize);
		emit setProgress(progress);
	});

	connect(m_reply.data(), &QNetworkReply::errorOccurred, this, &FvUpdater::networkError);

	connect(m_reply.data(), &QNetworkReply::downloadProgress, this, [this](qint64 bytesRead, qint64 totalBytes)
    {
		Q_UNUSED(bytesRead)
		Q_UNUSED(totalBytes)

		if (m_httpRequestAborted)
        {
			return;
		}
	});

	connect(m_reply.data(), &QNetworkReply::finished, this, [=]()
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
		// Error.
		showErrorDialog(tr("File download failed: %1.").arg(m_reply->errorString()));
	}
    else if (!redirectionTarget.isNull())
    {
		downloadedFile->close();
		const QUrl newUrl = m_feedURL.resolved(redirectionTarget.toUrl());
		m_reply->deleteLater();
		showInformationDialog(tr("Download has started, the installer will open once it's finished downloading"));
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
void FvUpdater::startDownloadFeed(const QUrl &url)
{
	QNetworkRequest request;
	request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/text"));
	request.setHeader(QNetworkRequest::UserAgentHeader, QCoreApplication::applicationName());
	request.setUrl(url);
	request.setSslConfiguration(QSslConfiguration::defaultConfiguration());

	m_reply = m_qnam.get(request);

	connect(m_reply.data(), &QNetworkReply::downloadProgress, this, [this](qint64 bytesRead, qint64 totalBytes)
    {
		Q_UNUSED(bytesRead)
		Q_UNUSED(totalBytes)

		if (m_httpRequestAborted)
        {
			return;
		}
	});
	connect(m_reply.data(), &QNetworkReply::finished, this, &FvUpdater::httpFeedDownloadFinished);
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
		showErrorDialog(
			tr("Feed download failed: %1.").arg(m_reply->errorString()));
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
        parseJson();
	}

	m_reply->deleteLater();

	if (m_dropOnFinish)
    {
		drop();
	}
}

void FvUpdater::getPLatformSpecificInstaller(QJsonArray assets)
{
	qDebug() << "current application version" << QCoreApplication::applicationVersion();
    QString osVersion = getOsVersion();

	for (const QJsonValueRef asset : assets)
    {
		auto name = asset.toObject()["name"].toString();
		qDebug() << "Checking" << osVersion << "against" << name;
		if (name.contains(osVersion, Qt::CaseInsensitive))
        {
			QUrl downloadableUrl =
				asset.toObject()["browser_download_url"].toString();
			startDownloadFile(downloadableUrl, name);
		}
	}
}

QString FvUpdater::getOsVersion()
{
    #ifdef Q_OS_LINUX // Defined on Linux.
    	QString osVersion = "AppImage";
    #else
    	#ifdef Q_OS_MACOS // Defined on macOS
    		QString osVersion = "macOS";
    	#else
    		#ifdef Q_OS_WIN64 // Defined on Windows 64-bit only.
    			QString osVersion = "Win64";
    		#else // Only windows 32-bit left
    			QString osVersion = "Win32";
    		#endif
    	#endif
    #endif
    return osVersion;
}



//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showErrorDialog(const QString &message)
{
	if (!m_isSilentMode)
    {
	    QMessageBox dlFailedMsgBox;
	    dlFailedMsgBox.setIcon(QMessageBox::Critical);
	    dlFailedMsgBox.setText(tr("Error"));
	    dlFailedMsgBox.setInformativeText(message);
	    dlFailedMsgBox.exec();
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::showInformationDialog(const QString &message)
{
	if (!m_isSilentMode)
    {
        QMessageBox dlInformationMsgBox;
        dlInformationMsgBox.setIcon(QMessageBox::Information);
        dlInformationMsgBox.setText(tr("Information"));
        dlInformationMsgBox.setInformativeText(message);
        dlInformationMsgBox.exec();
    }
}

bool FvUpdater::showUpdaterDialog(const QString &message)
{
	//if (!m_isSilentMode)
    {
        m_updaterDialog = new UpdaterDialog(m_parent);
        m_updaterDialog->updateDialog(getOsVersion(), message);
        m_updaterDialog->show();
        return true;
    }
    return false;
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

void FvUpdater::networkError()
{
	emit setProgress(100);
	m_fileSize = 0;
	showErrorDialog(m_reply->errorString());
}

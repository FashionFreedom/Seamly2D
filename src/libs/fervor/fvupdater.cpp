/***************************************************************************************************
 **
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

#include "../ifc/exception/vexception.h"
#include "../ifc/xml/vabstractconverter.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

const QString defaultFeedURL = QStringLiteral(
	"https://api.github.com/repos/FashionFreedom/Seamly2D/releases/latest");

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
	: QObject(nullptr), m_proposedUpdate(nullptr),
	  m_silentAsMuchAsItCouldGet(true), m_feedURL(), m_qnam(), m_reply(nullptr),
	  m_httpRequestAborted(false), m_dropOnFinnish(true) {
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
bool FvUpdater::IsDropOnFinnish() const { return m_dropOnFinnish; }

//---------------------------------------------------------------------------------------------------------------------
void FvUpdater::SetDropOnFinnish(bool value) { m_dropOnFinnish = value; }

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
		if (m_dropOnFinnish && not success) {
			drop();
		}
		return success;
	} else {
		if (m_dropOnFinnish) {
			drop();
		}
		return true;
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool FvUpdater::CheckForUpdatesNotSilent() {
	const bool success = CheckForUpdates(false);
	if (m_dropOnFinnish && not success) {
		drop();
	}
	return success;
}

void FvUpdater::startDownloadFile(const QUrl &url) {
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
		auto jsonRespArray = QJsonDocument::fromJson(m_reply->readAll()).array();
		for (const auto &jsonResp : jsonRespArray) {
			auto tag = jsonResp.toObject()["tag_name"].toString();

			QRegularExpression regexp{"v\\d+\\.\\d+\\.\\d+\\.\\d+"};
			// QRegularExpression regexp{"weekly"};
			auto matcher = regexp.match(tag);
			qDebug() << matcher.captured();

			if (matcher.hasMatch()) {
				if (!releaseIsNewer(matcher.captured())) {
					showInformationDialog(tr("No new releases available."));
					return;
				}
				if (showConfirmationDialog(tr("A new release %1 is available").arg(matcher.captured())))
					getPLatformSpecificInstaller(jsonResp.toObject()["assets"].toArray());
				return;
			}
		}
	}

	m_reply->deleteLater();

	if (m_dropOnFinnish) {
		drop();
	}
}

void FvUpdater::getPLatformSpecificInstaller(QJsonArray assets) {
	qDebug() << QCoreApplication::applicationVersion();

#ifdef Q_OS_LINUX // Defined on Linux.
	auto searchPattern = "AppImage";
#elseif Q_OS_MAC // Defined on MAC OS (synonym for Darwin).
	auto searchPattern = "dmg";
#else
	auto searchPattern = ".exe";
#endif

	for (const auto &asset : assets) {
		// qDebug() << asset;
		qDebug() << asset.toObject()["name"].toString();
		if (asset.toObject()["name"].toString().contains(searchPattern,
														 Qt::CaseInsensitive)) {
			QUrl downloadableUrl =
				asset.toObject()["browser_download_url"].toString();
			startDownloadFile(downloadableUrl);
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
	dlInformationMsgBox.setStandardButtons(QMessageBox::Ok
										   | QMessageBox::Cancel);

	return QMessageBox::Ok == dlInformationMsgBox.exec();
}


bool FvUpdater::releaseIsNewer(const QString &releaseTag) const {
}

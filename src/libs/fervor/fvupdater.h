//---------------------------------------------------------------------------------------------------------------------
//  @file   fvupdater.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//   @file   fvupdater.h
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

#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <QMetaObject>
#include <QMessageBox>
#include <QMutex>
#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QUrl>
#include <QNetworkReply>
#include <QtGlobal>

extern const QString defaultFeedURL;
class QFile;
class FvUpdater : public QObject
{
	Q_OBJECT

public:
	// Singleton
	static FvUpdater       *sharedUpdater();
	static void		        drop();

	// Set / get feed URL
	void	                setFeedURL(const QUrl &feedURL);
	void	                setFeedURL(const QString &feedURL);
	QString                 getFeedURL() const;
	bool                    isDropOnFinish() const;
	void                    setDropOnFinish(bool value);

	// Check for updates
	bool                    checkForUpdates(bool silentMode = true);
	bool                    checkForUpdatesSilent();
	bool                    checkForUpdatesNotSilent();

private slots:
    void                    httpFeedDownloadFinished();
    void                    networkError(QNetworkReply::NetworkError);
    void                    getFileSize();

signals:
    void                    setProgress(int);

protected:
	friend class            FvUpdateWindow; /// Uses GetProposedUpdate() and others

private:
	//
	// Singleton business
	//
	Q_DISABLE_COPY(FvUpdater)
	                        FvUpdater();		  // Hide main constructor
	virtual                ~FvUpdater();          // Hide main destructor

	static QPointer<FvUpdater> m_Instance;        // Singleton instance

	// If true, don't show the error dialogs and the "no updates." dialog
	// (silentMode from checkForUpdates() goes here)
	// Useful for automatic update checking upon application startup.
	bool                    m_silentMode;

	//
	// HTTP feed fetcher infrastructure
	//
	QUrl					m_feedURL;            // Feed URL that will be fetched
	QNetworkAccessManager	m_networkAccessManager;
	QPointer<QNetworkReply> m_reply;
	bool					m_httpRequestAborted;
	bool					m_dropOnFinish;
	int						m_fileSize{};
	QString					m_releaseName{};

	void                    startDownloadFeed(const QUrl &url);		   // Start downloading feed
	void                    startDownloadFile(QUrl url, QString name); // Start downloading file
	void                    fileDownloadFinished(QFile *downloadedFile, QString name);
	void                    cancelDownloadFeed();                      // Stop downloading the current feed

	// Dialogs (notifications)
    bool                    showConfirmationDialog(const QString &message, bool showEvenInSilentMode = false);
    void                    showMessageBox(QMessageBox::Icon icon, const QString &text,
                                           const QString &message, bool showEvenInSilentMode);

    bool                    releaseIsNewer(const QString &releaseTag);
	void                    getPLatformSpecificInstaller(QJsonArray assets);
};

#endif // FVUPDATER_H

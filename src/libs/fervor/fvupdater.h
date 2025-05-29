//-------------------------------------------------------------------------------------------------
//  @file   fvupdater_dialog.h
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
//  @file   fvupdater.h
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

#ifndef FVUPDATER_H
#define FVUPDATER_H

#include <QMetaObject>
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
class UpdaterDialog;
class FvAvailableUpdate;

class FvUpdater : public QObject
{
	Q_OBJECT

public:
	// Singleton
    static FvUpdater  *sharedUpdater();
	static void		   drop();

	// Set / get feed URL
	void	           setFeedURL(const QUrl &feedURL);
	void	           setFeedURL(const QString &feedURL);
	QString            getFeedURL() const;

    void               setParent(QWidget *parent);

	bool               dropOnFinish() const;
	void               setDropOnFinish(bool value);

signals:
	void               setProgress(int value);

public slots:
	// Check for updates
	bool               checkForUpdates(bool isSilentMode = true);

	// Aliases
	bool               checkForUpdatesSilent();
	bool               checkForUpdatesNotSilent();

protected:
	friend class       UpdaterDialog;
    FvAvailableUpdate *getProposedUpdate();

protected slots:
    // Update window button slots
    void               downloadUpdate();
    void               skipUpdate();
    void               remindLater();

private slots:
	void               httpFeedDownloadFinished();
	void               networkError();
    void               getFileSize();

private:
	//
	// Singleton business
	//
	Q_DISABLE_COPY(FvUpdater)
                       FvUpdater();		              // Hide main constructor
    virtual           ~FvUpdater();                   // Hide main destructor
    //FvUpdater(const FvUpdater&);			           // Hide copy constructor
	//FvUpdater& operator=(const FvUpdater&);	       // Hide assign op

	static QPointer<FvUpdater> m_Instance;            // Singleton instance


	// If true, don't show the error dialogs and the "no updates." dialog
	// (isSilentMode from checkForUpdates() goes here)
	// Useful for automatic update checking upon application startup.
	bool                    m_isSilentMode;

	//
	// HTTP feed fetcher infrastructure
	//
	QUrl					m_feedURL; // Feed URL that will be fetched
	QNetworkAccessManager	m_qnam;
	QPointer<QNetworkReply> m_reply;
	bool					m_httpRequestAborted;
	bool					m_dropOnFinish;
	int						m_fileSize{};
	QString					m_releaseName{};
    QString                 m_updateVersion;
    QWidget                *m_parent{nullptr};

    void            startDownloadFeed(const QUrl &url);		       // Start downloading feed
    void            startDownloadFile(QUrl url, QString name);     // Start downloading file
    void            fileDownloadFinished(QFile *downloadedFile, QString name);
    void            cancelDownloadFeed();                          // Stop downloading the current feed

    // Dialogs
    UpdaterDialog  *m_updaterDialog{nullptr};                     // Updater window (NULL if not shown)
    bool            showUpdaterDialog(const QString &message);    // Updater dialog
    void            hideUpdaterDialog();		                  // Hide + destroy m_updaterDialog
    void            updaterDialogWasClosed();

	// Dialogs (notifications)
    void            showErrorDialog(const QString &message);       // Show an error message
    void            showInformationDialog(const QString &message); // Show an informational message

    void            parseJson(); // Parse json
	bool            releaseIsNewer(const QString &releaseTag);
	void            getPLatformSpecificInstaller(QJsonArray assets);
    QString         getOsVersion();
};

#endif // FVUPDATER_H

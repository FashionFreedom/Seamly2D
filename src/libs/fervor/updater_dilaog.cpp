// @file   updater_dialog.cpp
// @author Douglas S Caskey
// @date   31 May, 2025
//
// @brief
// @copyright
// This source code is part of the Seamly2D project, a pattern making
// program to create and model patterns of clothing.
// Copyright (C) 2017-2025 Seamly2D project
// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.

#include "updater_dialog.h"

#include <QApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QLabel>
#include <QPointer>
#include <QPushButton>

#include "fvavailableupdate.h"
#include "fvupdater.h"
#include "../vmisc/def.h"
#include "ui_updater_dialog.h"

//---------------------------------------------------------------------------------------------------------------------
UpdaterDialog::UpdaterDialog(QWidget *parent)
  : QDialog(parent)
  , m_ui(new Ui::UpdaterDialog)
  , m_downloadDir()
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QDir m_downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_ui->downloadFolder_LineEdit->setText(m_downloadDir.path());
    connect(m_ui->browse_PushButton, &QPushButton::clicked, this, &UpdaterDialog::setDownloadDirectory);

    // Delete on close
    setAttribute(Qt::WA_DeleteOnClose, true);

    // Set the "new version is available" string
    const QString newVersString = m_ui->version_Label->text().arg(QGuiApplication::applicationDisplayName());
    m_ui->version_Label->setText(newVersString);

    m_ui->cancel_PushButton->hide();
    m_ui->download_ProgressBar->hide();
	m_ui->download_ProgressBar->setValue(0);
	connect(FvUpdater::sharedUpdater(), &FvUpdater::setProgress, this, &UpdaterDialog::setProgressValue);

    // Connect buttons
    connect(m_ui->intsallUpdate_PushButton, &QPushButton::clicked, this, &UpdaterDialog::downloadUpdate);
    connect(m_ui->intsallUpdate_PushButton, &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::downloadUpdate);
    connect(m_ui->skipVersion_PushButton,   &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::skipUpdate);
    connect(m_ui->remind_PushButton,        &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::remindLater);
    connect(m_ui->cancel_PushButton,        &QPushButton::clicked, FvUpdater::sharedUpdater(), &FvUpdater::cancelDownloadFeed);
}

//---------------------------------------------------------------------------------------------------------------------
UpdaterDialog::~UpdaterDialog()
{
    delete m_ui;
}

// Update the current update proposal from FvUpdater
void UpdaterDialog::updateDialog(const QString &osVersion, const QString &message)
{
    QString versionAvailable = tr("A new version of Seamly for %1 is available!").arg(osVersion);
    m_ui->newVersionIsAvailable_Label->setText(versionAvailable);
    m_ui->version_Label->setText(message);
    return;
}

QDir UpdaterDialog::getDownloadDirectory()
{
    return m_downloadDir;
}


void UpdaterDialog::downloadUpdate()
{
    m_ui->newVersionIsAvailable_Label->setText(tr("Retrieving the file"));
    m_ui->version_Label->setText("");
    m_ui->downloadNow_label->setText("");

    m_ui->download_ProgressBar->show();
    m_ui->cancel_PushButton->show();

    m_ui->downloadFolder_Label->hide();
    m_ui->downloadFolder_LineEdit->hide();
    m_ui->browse_PushButton->hide();

    m_ui->intsallUpdate_PushButton->hide();
    m_ui->skipVersion_PushButton->hide();
    m_ui->remind_PushButton->hide();
    return;
}

void UpdaterDialog::setProgressValue(int value)
{
	if (!m_ui->download_ProgressBar->isVisible())
    {
		m_ui->download_ProgressBar->show();
		//m_ui->cancel_PushButton->setDisabled(true);
	}

	m_ui->download_ProgressBar->setValue(value);

	if (value == 100){
		m_ui->download_ProgressBar->hide();
		m_ui->download_ProgressBar->setValue(0);
		//m_ui->cancel_PushButton->setDisabled(false);
	}
}

void UpdaterDialog::setDownloadDirectory()
{

    const QString filename = fileDialog(this, tr("Download Path"),
                                        m_downloadDir.dirName(),
                                        QString(""), nullptr,
                                        QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

    if (!filename.isEmpty())
    {
        m_downloadDir = QFileInfo(filename).dir();
    }
}

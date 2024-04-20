// @file   about2d_dialog.cpp
// @author Douglas S Caskey
// @date   18 Apr, 2024
//
// @brief
// @copyright
// This source code is part of the Seamly2D project, a pattern making
// program to create and model patterns of clothing.
// Copyright (C) 2017-2024 Seamly2D project
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

/************************************************************************
 **
 **  @file   dialogaboutapp.cpp
 **  @author Patrick Proy <patrick(at)proy.org>
 **  @date   6 5, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
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

#include "about2d_dialog.h"
#include "ui_about2d_dialog.h"
#include "../version.h"

#include <QClipboard>
#include <QDate>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QSound>
#include <QSysInfo>
#include <QtDebug>
#include <QTextCodec>

#include "../options.h"
#include "../core/application_2d.h"
#include "../fervor/fvupdater.h"
#include "../vmisc/vcommonsettings.h"

//---------------------------------------------------------------------------------------------------------------------
About2DAppDialog::About2DAppDialog(QWidget *parent)
    : QDialog(parent)
	, ui(new Ui::About2DAppDialog)
	, m_isInitialized(false)
    , m_beep(new QSound(qApp->Settings()->getSelectionSound()))
{
	ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	qApp->Seamly2DSettings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    //About Tab
    connect(ui->copyToClipbaord_toolButton, &QToolButton::clicked, this, &About2DAppDialog::copyToClipboard);

    QString revision = BUILD_REVISION;
    if (revision == QString("unknown"))
    {
        revision = tr("unknown");
    }
	ui->version_value->setText(QString("Seamly2D %1").arg(APP_VERSION_STR));
	ui->revision_value->setText(revision);

	QDate date = QLocale::c().toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy"));
	ui->buildDate_value->setText(tr("Built on %1 at %2").arg(date.toString()).arg(__TIME__));
	ui->qtVersion_value->setText(QLatin1String(qVersion()));
    ui->cpu_value->setText(QSysInfo::buildCpuArchitecture());
    ui->compiler_value->setText(QString("%1 %2 bit").arg(compilerString(), QString::number(QSysInfo::WordSize)));

	ui->legalStuff_label->setText(QApplication::translate("InternalStrings",
									  "Seamly2D is an application for computer-aided design of garment patterns.\n\n"
                                      "Seamly2D is a free (open source) software.\n\n"
                                      "All brand or product names are trademarks or registered trademarks of their respective holders.\n\n"
                                      "© 2017-2024 Seamly2D Project.\n\n"
                                      "Portions of this software © 2008-2024 The Qt Company Ltd.\n\n"
                                      "The program is provided AS IS with NO WARRANTY OF ANY "
									  "KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY "
									  "AND FITNESS FOR A PARTICULAR PURPOSE."));


	ui->pushButton_Web_Site->setText(tr("Web site : %1").arg(VER_COMPANYDOMAIN_STR));
	connect(ui->pushButton_Web_Site, &QPushButton::clicked, this, [this]()
	{
		if ( QDesktopServices::openUrl(QUrl(VER_COMPANYDOMAIN_STR)) == false)
		{
			qWarning() << tr("Cannot open your default browser");
		}
	});

	connect(ui->pushButtonCheckUpdate, &QPushButton::clicked, []()
	{
		// Set feed URL before doing anything else
		FvUpdater::sharedUpdater()->SetFeedURL(defaultFeedURL);
		FvUpdater::sharedUpdater()->CheckForUpdatesNotSilent();
	});

	ui->downloadProgress->hide();
	ui->downloadProgress->setValue(0);
	connect(FvUpdater::sharedUpdater(), SIGNAL(setProgress(int)), this, SLOT(setProgressValue(int)));

    //System Tab
    ui->sysVersion_value->setText(QString("Seamly2D %1").arg(APP_VERSION_STR));
    ui->sysRevision_value->setText(revision);
    ui->sysBuildDate_value->setText(tr("Built on %1 at %2").arg(date.toString()).arg(__TIME__));
    ui->sysCPU_value->setText(QSysInfo::buildCpuArchitecture());
    ui->sysCompiler_value->setText(QString("%1 %2 bit").arg(compilerString(), QString::number(QSysInfo::WordSize)));

    ui->sysArchCPU_value->setText(QSysInfo::buildCpuArchitecture());

    QString os = "Unknown";
    #if defined(Q_OS_LINUX)
        os = "Linux";
    #elif defined(Q_OS_MAC)
        os = "macOS";
    #elif defined(Q_OS_WIN)
        os = "Windows";
    #endif

    ui->sysOS_value->setText(os);
    ui->sysOSVersion_value->setText(QSysInfo::productVersion());

    QLocale sysloc = QLocale();
    ui->sysLocale_value->setText(sysloc.name());
    ui->sysLocaleCountry_value->setText(QLocale::countryToString(sysloc.country()));
    ui->sysLocalLang_value->setText(QLocale::languageToString(sysloc.language()));

    ui->sysLocalScriptName_vale->setText(QLocale::scriptToString(sysloc.script()));
    ui->sysLocalDecimal_value->setText(sysloc.decimalPoint());
    ui->sysLocalNegative_value->setText(sysloc.negativeSign());
    ui->sysLocalPositive_value->setText(sysloc.positiveSign());

    QString direction;
    switch(sysloc.textDirection())
    {
        case Qt::LeftToRight:
    	   direction = tr("Left to right");
        case Qt::RightToLeft:
           direction = tr("Right to left");
        case Qt::LayoutDirectionAuto:
        default:
           direction = tr("Automatic");
    }

    ui->sysLocalDirection_value->setText(direction);
    ui->sysCodec_value->setText(QTextCodec::codecForLocale()->name());
    ui->sysArguments_value->setText(QCoreApplication::arguments().join(","));
}

//---------------------------------------------------------------------------------------------------------------------
About2DAppDialog::~About2DAppDialog()
{
	delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void About2DAppDialog::showEvent(QShowEvent *event)
{
	QDialog::showEvent( event );
	if ( event->spontaneous() )
	{
		return;
	}

	if (m_isInitialized)
	{
		return;
	}

	m_isInitialized = true;//first show windows are held
}

void About2DAppDialog::setProgressValue(int val) {
	if (!ui->downloadProgress->isVisible()){
		ui->downloadProgress->show();
		ui->pushButtonCheckUpdate->setDisabled(true);
	}
	ui->downloadProgress->setValue(val);
	if (val == 100){
		ui->downloadProgress->hide();
		ui->downloadProgress->setValue(0);
		ui->pushButtonCheckUpdate->setDisabled(false);
	}
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Copies the text content of the text browser widget to the clipboard.
 */
void About2DAppDialog::copyToClipboard()
{
    m_beep->play();
    QClipboard *clipboard = QApplication::clipboard();

    QString text ="";
    text += QString("Seamly2D version: %1").arg(ui->version_value->text());
    text += QString("\nBuild revision: %1").arg(ui->revision_value->text());
    text += QString("\nBuild date: %1").arg(ui->buildDate_value->text());
    text += QString("\nQt Version: %1").arg(ui->qtVersion_value->text());
    text += QString("\nCPU: %1").arg(ui->cpu_value->text());
    text += QString("\nCompiler: %1").arg(ui->compiler_value->text());
    text += QString("\nCPU: %1").arg(ui->sysArchCPU_value->text());
    text += QString("\nOS: %1").arg(ui->sysOS_value->text());
    text += QString("\nOS Version: %1").arg(ui->sysOSVersion_value->text());
    text += QString("\nLocale: %1").arg(ui->sysLocale_value->text());
    text += QString("\nCountry: %1").arg(ui->sysLocaleCountry_value->text());
    text += QString("\nLanguage: %1").arg(ui->sysLocalLang_value->text());
    text += QString("\nScript Name: %1").arg(ui->sysLocalScriptName_vale->text());
    text += QString("\nDecimal Point: %1").arg(ui->sysLocalDecimal_value->text());
    text += QString("\nNegative Sign: %1").arg(ui->sysLocalNegative_value->text());
    text += QString("\nPositive Sign: %1").arg(ui->sysLocalPositive_value->text());
    text += QString("\nDirection: %1").arg(ui->sysLocalDirection_value->text());
    text += QString("\nSystem Codec: %1").arg(ui->sysCodec_value->text());
    text += QString("\nArguments: %1").arg(ui->sysArguments_value->text());
    clipboard->setText(text);
}

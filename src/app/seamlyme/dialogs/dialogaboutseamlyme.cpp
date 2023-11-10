/******************************************************************************
*   @file  dialogaboutseamlyme.cpp
**  @author Douglas S Caskey
**  @date   3 Sep, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
**  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
**
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
**  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

/************************************************************************
 **
 **  @file   dialogaboutseamlyme.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "dialogaboutseamlyme.h"
#include "ui_dialogaboutseamlyme.h"
#include "../version.h"
#include "../vmisc/def.h"
#include "../fervor/fvupdater.h"

#include <QDate>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QMessageBox>
#include <QScreen>
#include <QShowEvent>
#include <QUrl>
#include <QtDebug>

//---------------------------------------------------------------------------------------------------------------------
DialogAboutSeamlyMe::DialogAboutSeamlyMe(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogAboutSeamlyMe)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    //mApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    RetranslateUi();
    connect(ui->pushButton_Web_Site, &QPushButton::clicked, this, [this]()
    {
        if ( QDesktopServices::openUrl(QUrl(VER_COMPANYDOMAIN_STR)) == false)
        {
            qWarning() << tr("Cannot open your default browser");
        }
    });
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &DialogAboutSeamlyMe::close);
    connect(ui->pushButtonCheckUpdate, &QPushButton::clicked, []()
    {
        // Set feed URL before doing anything else
        FvUpdater::sharedUpdater()->SetFeedURL(defaultFeedURL);
        FvUpdater::sharedUpdater()->CheckForUpdatesNotSilent();
    });

    // By default on Windows font point size 8 points we need 11 like on Linux.
    FontPointSize(ui->label_Legal_Stuff, 11);
    FontPointSize(ui->label_SeamlyMe_Built, 11);
    FontPointSize(ui->label_QT_Version, 11);

	ui->downloadProgress->hide();
	ui->downloadProgress->setValue(0);
	connect(FvUpdater::sharedUpdater(), SIGNAL(setProgress(int)), this, SLOT(setProgressValue(int)));
}

//---------------------------------------------------------------------------------------------------------------------
DialogAboutSeamlyMe::~DialogAboutSeamlyMe()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutSeamlyMe::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        RetranslateUi();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutSeamlyMe::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutSeamlyMe::FontPointSize(QWidget *w, int pointSize)
{
    SCASSERT(w != nullptr)

    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogAboutSeamlyMe::RetranslateUi()
{
    QString revision = BUILD_REVISION;
    if (BUILD_REVISION == "unknown")
    {
        revision = tr("unknown");
    }
    ui->label_SeamlyMe_Version->setText(QString("SeamlyMe %1").arg(APP_VERSION_STR));
    ui->labelBuildRevision->setText(tr("Build revision: %1").arg(revision));
    ui->label_QT_Version->setText(buildCompatibilityString());

    const QDate date = QLocale::c().toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy"));
    ui->label_SeamlyMe_Built->setText(tr("Built on %1 at %2").arg(date.toString()).arg(__TIME__));

    ui->label_Legal_Stuff->setText(QApplication::translate("InternalStrings",
                                                           "The program is provided AS IS with NO WARRANTY OF ANY "
                                                           "KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY "
                                                           "AND FITNESS FOR A PARTICULAR PURPOSE."));

    ui->pushButton_Web_Site->setText(tr("Web site : %1").arg(VER_COMPANYDOMAIN_STR));
}

void DialogAboutSeamlyMe::setProgressValue(int val) {
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

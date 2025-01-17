/***************************************************************************
 **  @file   groups_widget.cpp
 **  @author Douglas S Caskey
 **  @date   12 Nov, 2023
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
 **************************************************************************

 /************************************************************************
 **
 **  @file   dialognewpattern.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 2, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2014 Seamly2D project
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

#include "dialognewpattern.h"
#include "ui_dialognewpattern.h"
#include "../core/vapplication.h"
#include "../vmisc/vsettings.h"
#include "../vpatterndb/vcontainer.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QGuiApplication>
#include <QScreen>

//---------------------------------------------------------------------------------------------------------------------
DialogNewPattern::DialogNewPattern(VContainer *data, const QString &patternPieceName, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogNewPattern)
    , data(data)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEditName->setClearButtonEnabled(true);

    qApp->Seamly2DSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QRect position = this->frameGeometry();
    position.moveCenter(QGuiApplication::primaryScreen()->availableGeometry().center());
    move(position.topLeft());

    ui->lineEditName->setText(patternPieceName);

    InitUnits();
    CheckState();
    connect(ui->lineEditName, &QLineEdit::textChanged, this, &DialogNewPattern::CheckState);
}

//---------------------------------------------------------------------------------------------------------------------
DialogNewPattern::~DialogNewPattern()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
Unit DialogNewPattern::PatternUnit() const
{
    const qint32 index = ui->comboBoxUnits->currentIndex();
    return StrToUnits(ui->comboBoxUnits->itemData(index).toString());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewPattern::CheckState()
{
    bool flagName = false;
    if (ui->lineEditName->text().isEmpty() == false)
    {
        flagName = true;
    }

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagName);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNewPattern::showEvent(QShowEvent *event)
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
void DialogNewPattern::InitUnits()
{
    ui->comboBoxUnits->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxUnits->addItem(tr("Millimeters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxUnits->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));

    // set default unit
    const qint32 indexUnit = ui->comboBoxUnits->findData(qApp->Seamly2DSettings()->GetUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxUnits->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogNewPattern::name() const
{
    return ui->lineEditName->text();
}

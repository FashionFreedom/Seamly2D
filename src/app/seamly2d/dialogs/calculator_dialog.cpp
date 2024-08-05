/************************************************************************
 **
 **  @file   decimalchart_dialog.h
 **  @author DSCaskey <dscaskey@gmail.com>
 **  @date   12 26, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  All Rights Reserved.
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

#include <QDate>
#include <QDesktopServices>
#include <QMessageBox>
#include <QtDebug>
#include <QWidget>
#include <QDialog>
#include "../options.h"
#include "../core/application_2d.h"
#include "../fervor/fvupdater.h"
#include "../vwidgets/calculator/calculator.h"
#include "calculator_dialog.h"
#include "ui_calculator_dialog.h"
#include "../version.h"

//---------------------------------------------------------------------------------------------------------------------
CalculatorDialog::CalculatorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CalculatorDialog)
    , isInitialized(false)
    , calc(new CalculatorUtil(this))
{

    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon("://icon/32x32/calculator.png"));

    ui->calculatorLayout->addWidget(calc);
}

//---------------------------------------------------------------------------------------------------------------------
CalculatorDialog::~CalculatorDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void CalculatorDialog::showEvent(QShowEvent *event)
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

    isInitialized = true;//first show windows are held
}

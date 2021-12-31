/************************************************************************
 **
 **  @file   calculator_dialog.h
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

#ifndef DIALOGCALCULATOR_H
#define DIALOGCALCULATOR_H

#include <QDialog>
#include <QWidget>

#include "../vwidgets/calculator/calculator.h"

namespace Ui
{
    class CalculatorDialog;
}

class QWidget;
class Calculator;

class CalculatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalculatorDialog(QWidget *parent = nullptr);
    virtual ~CalculatorDialog();

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private:
    Ui::CalculatorDialog *ui;
    bool isInitialized;
    Q_DISABLE_COPY(CalculatorDialog)

    void setFontPointSize(QWidget *w, int pointSize);

    Calculator* calc;
};

#endif // DIALOGCALCULATOR_H

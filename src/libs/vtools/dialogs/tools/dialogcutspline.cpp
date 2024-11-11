//-----------------------------------------------------------------------------
//  @file   dialogcutspline.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   dialogcutspline.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Dec 2013
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

#include "dialogcutspline.h"

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QToolButton>

#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/path/vistoolcutspline.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogcutspline.h"

//---------------------------------------------------------------------------------------------------------------------
/// @brief DialogCutSpline create dialog.
/// @param data container with data
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
DialogCutSpline::DialogCutSpline(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCutSpline)
    , formula(QString())
    , formulaBaseHeight(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/spline_cut_point.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    initializeFormulaUi(ui);
    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;
    this->formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    initializeOkCancelApply(ui);
    flagFormula = false;
    DialogTool::CheckState();

    FillComboBoxSplines(ui->comboBoxSpline);

    int index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (index != -1)
    {
        ui->lineColor_ComboBox->setCurrentIndex(index);
    }

    ui->direction_ComboBox->addItem(tr("Forward (from start point)"), "forward");
    ui->direction_ComboBox->addItem(tr("Backward (from end point)"), "backward");

    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogCutSpline::FXLength);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, &DialogCutSpline::NamePointChanged);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogCutSpline::FormulaChanged);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogCutSpline::DeployFormulaTextEdit);

    vis = new VisToolCutSpline(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCutSpline::~DialogCutSpline()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setPointName set name of point
/// @param value name
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::setPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setDirection set the direction
/// @param value name
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::setDirection(const QString &value)
{
    ChangeCurrentData(ui->direction_ComboBox, value);
    VisToolCutSpline *path = qobject_cast<VisToolCutSpline *>(vis);
    SCASSERT(path != nullptr)
    path->setDirection(value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getDirection get the direction of the spline
/// @return QString direction
//---------------------------------------------------------------------------------------------------------------------

QString DialogCutSpline::getDirection() const
{
    return GetComboBoxCurrentData(ui->direction_ComboBox, "forward");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setFormula set string of formula
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::setFormula(const QString &value)
{
    formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formula);

    VisToolCutSpline *path = qobject_cast<VisToolCutSpline *>(vis);
    SCASSERT(path != nullptr)
    path->setLength(formula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setSplineId set id spline
/// @param value id
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::setSplineId(const quint32 &value)
{
    setCurrentSplineId(ui->comboBoxSpline, value);

    VisToolCutSpline *path = qobject_cast<VisToolCutSpline *>(vis);
    SCASSERT(path != nullptr)
    path->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getLineColor get the color of line
/// @return QString name of color
//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSpline::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLineColor set color of the line
/// @param value type
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
/// @param id id of point or detail
/// @param type type of object
//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Spline)
        {
            if (SetObject(id, ui->comboBoxSpline, ""))
            {
                vis->VisualMode(id);
                prepare = true;
                this->setModal(true);
                this->show();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formula = ui->plainTextEditFormula->toPlainText();
    formula.replace("\n", " ");

    VisToolCutSpline *path = qobject_cast<VisToolCutSpline *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(getSplineId());
    path->setDirection(getDirection());
    path->setLength(formula);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::DeployFormulaTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::FXLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(getFormula());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setFormula(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutSpline::ShowVisualization()
{
    AddVisualization<VisToolCutSpline>();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getFormula return string of formula
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogCutSpline::getFormula() const
{
    return qApp->translateVariables()->TryFormulaFromUser(formula, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getSplineId return id base point of line
/// @return id
//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCutSpline::getSplineId() const
{
    return getCurrentObjectId(ui->comboBoxSpline);
}

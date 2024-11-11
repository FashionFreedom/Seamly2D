//-----------------------------------------------------------------------------
//  @file   dialogcutarc.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2023
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
//  @file   dialogcutarc.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   7 Jan, 2013
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

#include "dialogcutarc.h"

#include <QDialog>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QToolButton>

#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/path/vistoolcutarc.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogcutarc.h"

//---------------------------------------------------------------------------------------------------------------------
/// @brief DialogCutArc create dialog.
/// @param data container with data
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
DialogCutArc::DialogCutArc(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCutArc)
    , formula(QString())
    , formulaBaseHeight(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/arc_cut.png"));

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

    FillComboBoxArcs(ui->comboBoxArc);

    int index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (index != -1)
    {
        ui->lineColor_ComboBox->setCurrentIndex(index);
    }

    ui->direction_ComboBox->addItem(tr("Forward (from start point)"), "forward");
    ui->direction_ComboBox->addItem(tr("Backward (from end point)"), "backward");

    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogCutArc::FXLength);
    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, &DialogCutArc::NamePointChanged);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogCutArc::FormulaTextChanged);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogCutArc::DeployFormulaTextEdit);

    vis = new VisToolCutArc(data);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::FormulaTextChanged()
{
    this->FormulaChangedPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::FXLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(GetFormula());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormula(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::ShowVisualization()
{
    AddVisualization<VisToolCutArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::DeployFormulaTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCutArc::~DialogCutArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
/// @param id id of point or detail
/// @param type type of object
//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Arc)
        {
            if (SetObject(id, ui->comboBoxArc, ""))
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
void DialogCutArc::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formula = ui->plainTextEditFormula->toPlainText();
    formula.replace("\n", " ");

    VisToolCutArc *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(getArcId());
    path->setDirection(getDirection());
    path->setLength(formula);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setArcId set id of arc
/// @param value id
//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::setArcId(const quint32 &value)
{
    setCurrentArcId(ui->comboBoxArc, value);

    VisToolCutArc *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)
    path->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetFormula set string with formula length
/// @param value string with formula
//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SetFormula(const QString &value)
{
    formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formula);

    VisToolCutArc *path = qobject_cast<VisToolCutArc *>(vis);
    SCASSERT(path != nullptr)
    path->setLength(formula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetPointName set name point on arc
/// @param value name
//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

// @brief setDirection set the direction
// @param value name
void DialogCutArc::setDirection(const QString &value)
{
    ChangeCurrentData(ui->direction_ComboBox, value);
}

QString DialogCutArc::getDirection() const
{
    return GetComboBoxCurrentData(ui->direction_ComboBox, "forward");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetFormula return string with formula length
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogCutArc::GetFormula() const
{
    return qApp->translateVariables()->TryFormulaFromUser(formula, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getArcId return id of arc
/// @return id
//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCutArc::getArcId() const
{
    return getCurrentObjectId(ui->comboBoxArc);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getLineColor get the color of line
/// @return QString name of color
//---------------------------------------------------------------------------------------------------------------------
QString DialogCutArc::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLineColor set color of the line
/// @param value type
//---------------------------------------------------------------------------------------------------------------------
void DialogCutArc::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

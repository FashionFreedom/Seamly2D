/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialognormal.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#include "dialognormal.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QToolButton>

#include "../vpatterndb/vtranslatevars.h"
#include "../../tools/vabstracttool.h"
#include "../../visualization/line/vistoolnormal.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialognormal.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogNormal create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogNormal::DialogNormal(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent), ui(new Ui::DialogNormal)
    , formula(QString())
    , angle(0)
    , formulaBaseHeight(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/normal.png"));

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    initializeFormulaUi(ui);
    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;
    this->formulaBaseHeight = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    initializeOkCancelApply(ui);
    flagFormula = false;
    DialogTool::CheckState();

    FillComboBoxPoints(ui->comboBoxFirstPoint);
    FillComboBoxPoints(ui->comboBoxSecondPoint);

    int index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (index != -1)
    {
        ui->lineColor_ComboBox->setCurrentIndex(index);
    }

    index = ui->lineWeight_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineWeight());
    if (index != -1)
    {
        ui->lineWeight_ComboBox->setCurrentIndex(index);
    }

    index = ui->lineType_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineType());
    if (index != -1)
    {
        ui->lineType_ComboBox->setCurrentIndex(index);
    }

    connect(ui->toolButtonExprLength, &QPushButton::clicked,          this, &DialogNormal::FXLength);
    connect(ui->lineEditNamePoint,    &QLineEdit::textChanged,        this, &DialogNormal::NamePointChanged);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged,   this, &DialogNormal::FormulaTextChanged);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked,          this, &DialogNormal::DeployFormulaTextEdit);
    connect(ui->comboBoxFirstPoint,   &QComboBox::currentTextChanged, this, &DialogNormal::PointNameChanged);
    connect(ui->comboBoxSecondPoint,  &QComboBox::currentTextChanged, this, &DialogNormal::PointNameChanged);

    vis = new VisToolNormal(data);

    // Call after initialization vis!!!!
    setLineType(LineTypeDashLine);
    setLineWeight("0.35");
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::FormulaTextChanged()
{
    this->FormulaChangedPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::PointNameChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxFirstPoint) == getCurrentObjectId(ui->comboBoxSecondPoint))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelFirstPoint, color);
    ChangeColor(ui->labelSecondPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::FXLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
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
void DialogNormal::ShowVisualization()
{
    AddVisualization<VisToolNormal>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::DeployFormulaTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
DialogNormal::~DialogNormal()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogNormal::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
            SCASSERT(line != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxFirstPoint, tr("Select second point of line")))
                    {
                        number++;
                        line->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxFirstPoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxSecondPoint, ""))
                        {
                            line->setObject2Id(id);
                            line->RefreshGeometry();
                            prepare = true;
                            this->setModal(true);
                            this->show();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formula = ui->plainTextEditFormula->toPlainText();
    formula.replace("\n", " ");
    angle = ui->doubleSpinBoxAngle->value();

    VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
    SCASSERT(line != nullptr)

    line->setObject1Id(GetFirstPointId());
    line->setObject2Id(GetSecondPointId());
    line->setLength(formula);
    line->SetAngle(angle);
    line->setLineStyle(lineTypeToPenStyle(getLineType()));
    line->setLineWeight(getLineWeight());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetSecondPointId set id of second point
 * @param value id
 */
void DialogNormal::SetSecondPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxSecondPoint, value);

    VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
    SCASSERT(line != nullptr)
    line->setObject2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFirstPointId set id of first point
 * @param value id
 */
void DialogNormal::SetFirstPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxFirstPoint, value);

    VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
    SCASSERT(line != nullptr)
    line->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetAngle set aditional angle of normal
 * @param value angle in degree
 */
void DialogNormal::SetAngle(const qreal &value)
{
    angle = value;
    ui->doubleSpinBoxAngle->setValue(angle);

    VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
    SCASSERT(line != nullptr)
    line->SetAngle(angle);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFormula set string of formula
 * @param value formula
 */
void DialogNormal::SetFormula(const QString &value)
{
    formula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formula);

    VisToolNormal *line = qobject_cast<VisToolNormal *>(vis);
    SCASSERT(line != nullptr)
    line->setLength(formula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineType return type of line
 * @return type
 */
QString DialogNormal::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineType set type of line
 * @param value type
 */
void DialogNormal::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogNormal::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogNormal::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogNormal::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogNormal::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogNormal::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormula return string of formula
 * @return formula
 */
QString DialogNormal::GetFormula() const
{
    return qApp->TrVars()->TryFormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetAngle return aditional angle of normal
 * @return angle in degree
 */
qreal DialogNormal::GetAngle() const
{
    return angle;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFirstPointId return id of first point
 * @return id
 */
quint32 DialogNormal::GetFirstPointId() const
{
    return getCurrentObjectId(ui->comboBoxFirstPoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetSecondPointId return id of second point
 * @return id
 */
quint32 DialogNormal::GetSecondPointId() const
{
    return getCurrentObjectId(ui->comboBoxSecondPoint);
}

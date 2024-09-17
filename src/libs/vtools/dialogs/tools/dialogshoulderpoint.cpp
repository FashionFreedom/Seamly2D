//-----------------------------------------------------------------------------
//  @file   dialogshoulderpoint.cpp
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
//  @file   dialogshoulderpoint.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
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

#include "dialogshoulderpoint.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QToolButton>

#include "../vpatterndb/vtranslatevars.h"
#include "../../tools/vabstracttool.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolshoulderpoint.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogshoulderpoint.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogShoulderPoint create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogShoulderPoint::DialogShoulderPoint(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogShoulderPoint)
    , formula(QString())
    , formulaBaseHeight(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/shoulder.png"));

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

    fillComboBoxPoints(ui->comboBoxP1Line);
    fillComboBoxPoints(ui->comboBoxP2Line);
    fillComboBoxPoints(ui->comboBoxP3);

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

    connect(ui->toolButtonExprLength, &QPushButton::clicked,        this, &DialogShoulderPoint::FXLength);
    connect(ui->lineEditNamePoint,    &QLineEdit::textChanged,      this, &DialogShoulderPoint::NamePointChanged);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogShoulderPoint::FormulaTextChanged);
    connect(ui->pushButtonGrowLength, &QPushButton::clicked,        this, &DialogShoulderPoint::DeployFormulaTextEdit);
    connect(ui->comboBoxP1Line,       &QComboBox::currentTextChanged, this, &DialogShoulderPoint::PointNameChanged);
    connect(ui->comboBoxP2Line,       &QComboBox::currentTextChanged, this, &DialogShoulderPoint::PointNameChanged);
    connect(ui->comboBoxP3,           &QComboBox::currentTextChanged, this, &DialogShoulderPoint::PointNameChanged);

    vis = new VisToolShoulderPoint(data);

    // Call after visual initialized.
    // If true current pen overides the default tool pen
    if(!qApp->Settings()->useCurrentPen())
    {
        setLineType(LineTypeDashLine);
        setLineWeight("0.35");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::FormulaTextChanged()
{
    this->FormulaChangedPlainText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxP1Line));
    set.insert(getCurrentObjectId(ui->comboBoxP2Line));
    set.insert(getCurrentObjectId(ui->comboBoxP3));

    QColor color = okColor;
    if (set.size() != 3)
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
    ChangeColor(ui->labelThirdPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::FXLength()
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
void DialogShoulderPoint::ShowVisualization()
{
    AddVisualization<VisToolShoulderPoint>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::DeployFormulaTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
DialogShoulderPoint::~DialogShoulderPoint()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogShoulderPoint::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)
    {
        if (type == SceneObject::Point)
        {
            VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
            SCASSERT(line != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxP3, tr("Select first point of line")))
                    {
                        number++;
                        line->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxP3) != id)
                    {
                        if (SetObject(id, ui->comboBoxP1Line, tr("Select second point of line")))
                        {
                            number++;
                            line->setLineP1Id(id);
                            line->RefreshGeometry();
                        }
                    }
                    break;
                case 2:
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxP3));
                    set.insert(getCurrentObjectId(ui->comboBoxP1Line));
                    set.insert(id);

                    if (set.size() == 3)
                    {
                        if (SetObject(id, ui->comboBoxP2Line, ""))
                        {
                            line->setLineP2Id(id);
                            line->RefreshGeometry();
                            prepare = true;
                            this->setModal(true);
                            this->show();
                        }
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
void DialogShoulderPoint::SaveData()
{
    pointName = ui->lineEditNamePoint->text();
    formula = ui->plainTextEditFormula->toPlainText();
    formula.replace("\n", " ");

    VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
    SCASSERT(line != nullptr)

    line->setObject1Id(GetP3());
    line->setLineP1Id(GetP1Line());
    line->setLineP2Id(GetP2Line());
    line->setLength(formula);
    line->setLineStyle(lineTypeToPenStyle(getLineType()));
    line->setLineWeight(getLineWeight());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setPShoulder set id shoulder point
 * @param value id
 */
void DialogShoulderPoint::SetP3(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP3, value);

    VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
    SCASSERT(line != nullptr)
    line->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogShoulderPoint::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogShoulderPoint::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP2Line set id second point of line
 * @param value id
 */
void DialogShoulderPoint::SetP2Line(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP2Line, value);

    VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
    SCASSERT(line != nullptr)
    line->setLineP2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP1Line set id first point of line
 * @param value id
 */
void DialogShoulderPoint::SetP1Line(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP1Line, value);

    VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
    SCASSERT(line != nullptr)
    line->setLineP1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetFormula set string of formula
 * @param value formula
 */
void DialogShoulderPoint::SetFormula(const QString &value)
{
    formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formula);

    VisToolShoulderPoint *line = qobject_cast<VisToolShoulderPoint *>(vis);
    SCASSERT(line != nullptr)
    line->setLength(formula);
    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineType return type of line
 * @return type
 */
QString DialogShoulderPoint::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineType set type of line
 * @param value type
 */
void DialogShoulderPoint::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogShoulderPoint::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogShoulderPoint::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogShoulderPoint::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormula return string of formula
 * @return formula
 */
QString DialogShoulderPoint::GetFormula() const
{
    return qApp->translateVariables()->TryFormulaFromUser(formula, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1Line return id first point of line
 * @return id
 */
quint32 DialogShoulderPoint::GetP1Line() const
{
    return getCurrentObjectId(ui->comboBoxP1Line);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2Line return id second point of line
 * @return id
 */
quint32 DialogShoulderPoint::GetP2Line() const
{
    return getCurrentObjectId(ui->comboBoxP2Line);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPShoulder return id shoulder point
 * @return id
 */
quint32 DialogShoulderPoint::GetP3() const
{
    return getCurrentObjectId(ui->comboBoxP3);
}

/******************************************************************************
 *   @file   dialogarcwithlength.cpp
 **  @author Douglas S Caskey
 **  @date   21 Mar, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

/************************************************************************
 **
 **  @file   dialogarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#include "dialogarcwithlength.h"

#include <limits.h>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/visualization.h"
#include "../../visualization/path/vistoolarcwithlength.h"

#include "ui_dialogarcwithlength.h"

//---------------------------------------------------------------------------------------------------------------------
DialogArcWithLength::DialogArcWithLength(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogArcWithLength)
    , flagRadius(false)
    , flagF1(false)
    , flagLength(false)
    , timerRadius(nullptr)
    , timerF1(nullptr)
    , timerLength(nullptr)
    , radius(QString())
    , f1(QString())
    , length(QString())
    , formulaBaseHeightRadius(0)
    , formulaBaseHeightF1(0)
    , formulaBaseHeightLength(0)
    , angleF1(INT_MIN)
    , m_arc()
    , m_Id()
    , newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/arc_with_length.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    m_Id  = data->getId() + 1;

    plainTextEditFormula = ui->plainTextEditRadius;
    this->formulaBaseHeightLength = ui->plainTextEditRadius->height();
    this->formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->formulaBaseHeightLength = ui->plainTextEditLength->height();

    ui->plainTextEditRadius->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditLength->installEventFilter(this);

    timerRadius = new QTimer(this);
    connect(timerRadius, &QTimer::timeout, this, &DialogArcWithLength::Radius);

    timerF1 = new QTimer(this);
    connect(timerF1, &QTimer::timeout, this, &DialogArcWithLength::EvalF);

    timerLength = new QTimer(this);
    connect(timerLength, &QTimer::timeout, this, &DialogArcWithLength::Length);

    initializeOkCancelApply(ui);

    fillComboBoxPoints(ui->centerPoint_ComboBox);

    int index = ui->lineType_ComboBox->findData(LineTypeNone);
    if (index != -1)
    {
        ui->lineType_ComboBox->removeItem(index);
    }

    index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
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

    CheckState();

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArcWithLength::FXRadius);
    connect(ui->toolButtonExprF1,     &QPushButton::clicked, this, &DialogArcWithLength::FXF1);
    connect(ui->toolButtonExprLength, &QPushButton::clicked, this, &DialogArcWithLength::FXLength);

    connect(ui->plainTextEditRadius, &QPlainTextEdit::textChanged, this, &DialogArcWithLength::RadiusChanged);
    connect(ui->plainTextEditF1,     &QPlainTextEdit::textChanged, this, &DialogArcWithLength::F1Changed);
    connect(ui->plainTextEditLength, &QPlainTextEdit::textChanged, this, &DialogArcWithLength::LengthChanged);

    connect(ui->pushButtonGrowLengthRadius,    &QPushButton::clicked, this, &DialogArcWithLength::DeployRadiusTextEdit);
    connect(ui->pushButtonGrowLengthF1,        &QPushButton::clicked, this, &DialogArcWithLength::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthArcLength, &QPushButton::clicked, this, &DialogArcWithLength::DeployLengthTextEdit);

    connect(ui->centerPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogArcWithLength::pointNameChanged);

    vis = new VisToolArcWithLength(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogArcWithLength::~DialogArcWithLength()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
VArc DialogArcWithLength::getArc() const
{
    return m_arc;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::setArc(const VArc &arc)
{
    m_arc = arc;
    ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(m_arc.name()));
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogArcWithLength::GetCenter() const
{
    return getCurrentObjectId(ui->centerPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetCenter(const quint32 &value)
{
    ChangeCurrentData(ui->centerPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArcWithLength::GetRadius() const
{
    return qApp->translateVariables()->TryFormulaFromUser(radius, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetRadius(const QString &value)
{
    radius = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (radius.length() > 80)
    {
        this->DeployRadiusTextEdit();
    }
    ui->plainTextEditRadius->setPlainText(radius);

    VisToolArcWithLength *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius(radius);

    MoveCursorToEnd(ui->plainTextEditRadius);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArcWithLength::GetF1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(f1, qApp->Settings()->getOsSeparator());
}

void DialogArcWithLength::SetF1(const QString &value)
{
    f1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(f1);

    VisToolArcWithLength *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArcWithLength::GetLength() const
{
    return qApp->translateVariables()->TryFormulaFromUser(length, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SetLength(const QString &value)
{
    length = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (length.length() > 80)
    {
        this->DeployLengthTextEdit();
    }
    ui->plainTextEditLength->setPlainText(length);

    VisToolArcWithLength *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)
    path->setLength(radius);

    MoveCursorToEnd(ui->plainTextEditLength);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArcWithLength::getPenStyle() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::setPenStyle(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogArcWithLength::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogArcWithLength::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArcWithLength::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->centerPoint_ComboBox, ""))
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
void DialogArcWithLength::DeployRadiusTextEdit()
{
    DeployFormula(ui->plainTextEditRadius, ui->pushButtonGrowLengthArcLength, formulaBaseHeightRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployF1TextEdit()
{
    DeployFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::DeployLengthTextEdit()
{
    DeployFormula(ui->plainTextEditLength, ui->pushButtonGrowLengthArcLength, formulaBaseHeightLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::RadiusChanged()
{
    labelEditFormula = ui->labelEditRadius;
    labelResultCalculation = ui->labelResultRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagRadius, ui->plainTextEditRadius, timerRadius, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::F1Changed()
{
    labelEditFormula = ui->labelEditF1;
    labelResultCalculation = ui->labelResultF1;
    ValFormulaChanged(flagF1, ui->plainTextEditF1, timerF1, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::LengthChanged()
{
    labelEditFormula = ui->labelEditLength;
    labelResultCalculation = ui->labelResultLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagLength, ui->plainTextEditLength, timerLength, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FXRadius()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit radius"));
    dialog->SetFormula(GetRadius());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FXF1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit the first angle"));
    dialog->SetFormula(GetF1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::FXLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit the arc length"));
    dialog->SetFormula(GetLength());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetLength(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::pointNameChanged()
{
    QColor color = okColor;

    flagError = true;
    color = okColor;

    if (getCurrentObjectId(ui->centerPoint_ComboBox) == m_arc.GetCenter().id())
    {
        newDuplicate = -1;
        ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(m_arc.name()));
    }
    else
    {
        VArc arc(*data->GeometricObject<VPointF>(getCurrentObjectId(ui->centerPoint_ComboBox)),
             GetRadius().toDouble(),
             GetF1().toDouble(),
             GetLength().toDouble());

        if (!data->IsUnique(arc.name()))
        {
            newDuplicate = static_cast<qint32>(DNumber(arc.name()));
            arc.SetDuplicate(static_cast<quint32>(newDuplicate));
        }
        ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(arc.name() + "_" + QString().setNum(m_Id)));
    }

    ChangeColor(ui->name_Label, color);
    ChangeColor(ui->centerPoint_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagRadius && flagF1 && flagLength);
    // In case dialog does not have an apply button
    if (apply_Button != nullptr)
    {
        apply_Button->setEnabled(ok_Button->isEnabled());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::ShowVisualization()
{
    AddVisualization<VisToolArcWithLength>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::SaveData()
{
    radius = ui->plainTextEditRadius->toPlainText();
    radius.replace("\n", " ");

    f1 = ui->plainTextEditF1->toPlainText();
    f1.replace("\n", " ");

    length = ui->plainTextEditLength->toPlainText();
    length.replace("\n", " ");

    VisToolArcWithLength *path = qobject_cast<VisToolArcWithLength *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(GetCenter());
    path->setRadius(radius);
    path->setF1(f1);
    path->setLength(length);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::Radius()
{
    labelEditFormula = ui->labelEditRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditRadius->toPlainText(), flagRadius, ui->labelResultRadius, postfix);

    if (radius < 0)
    {
        flagRadius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius->setText(tr("Error"));
        ui->labelResultRadius->setToolTip(tr("Radius can't be negative"));

        DialogArcWithLength::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::Length()
{
    labelEditFormula = ui->labelEditLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal length = Eval(ui->plainTextEditLength->toPlainText(), flagLength, ui->labelResultLength, postfix);

    if (qFuzzyIsNull(length))
    {
        flagLength = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultLength->setText(tr("Error"));
        ui->labelResultLength->setToolTip(tr("Length can't be equal 0"));

        DialogArcWithLength::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArcWithLength::EvalF()
{
    labelEditFormula = ui->labelEditF1;
    angleF1 = Eval(ui->plainTextEditF1->toPlainText(), flagF1, ui->labelResultF1, degreeSymbol, false);
}

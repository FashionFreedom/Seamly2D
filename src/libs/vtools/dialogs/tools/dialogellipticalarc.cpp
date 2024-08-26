//-----------------------------------------------------------------------------
//  @file   dialogellipticalarc.cpp
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
//  @file   dialogellipticalarc.cpp
//  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
//  @date   15 Sep, 2013
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

#include "dialogellipticalarc.h"

#include <limits.h>
#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

#include "../../tools/vabstracttool.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/path/vistoolellipticalarc.h"
#include "../../visualization/visualization.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogellipticalarc.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogEllipticalArc create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogEllipticalArc::DialogEllipticalArc(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogEllipticalArc)
    , flagRadius1(false)
    , flagRadius2(false)
    , flagF1(false)
    , flagF2(false)
    , flagRotationAngle(false)
    , timerRadius1(nullptr)
    , timerRadius2(nullptr)
    , timerF1(nullptr)
    , timerF2(nullptr)
    , timerRotationAngle(nullptr)
    , radius1()
    , radius2()
    , f1()
    , f2()
    , rotationAngle()
    , formulaBaseHeightRadius1(0)
    , formulaBaseHeightRadius2(0)
    , formulaBaseHeightF1(0)
    , formulaBaseHeightF2(0)
    , formulaBaseHeightRotationAngle(0)
    , angleF1(INT_MIN)
    , angleF2(INT_MIN)
    , angleRotation(INT_MIN)
    , m_arc()
    , m_Id()
    , newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/el_arc.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    m_Id  = data->getId() + 1;

    this->formulaBaseHeightRadius1 = ui->plainTextEditRadius1->height();
    this->formulaBaseHeightRadius2 = ui->plainTextEditRadius2->height();
    this->formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->formulaBaseHeightF2 = ui->plainTextEditF2->height();
    this->formulaBaseHeightRotationAngle = ui->plainTextEditRotationAngle->height();

    ui->plainTextEditRadius1->installEventFilter(this);
    ui->plainTextEditRadius2->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF2->installEventFilter(this);
    ui->plainTextEditRotationAngle->installEventFilter(this);

    timerRadius1 = new QTimer(this);
    connect(timerRadius1, &QTimer::timeout, this, &DialogEllipticalArc::EvalRadiuses);

    timerRadius2 = new QTimer(this);
    connect(timerRadius2, &QTimer::timeout, this, &DialogEllipticalArc::EvalRadiuses);

    timerF1 = new QTimer(this);
    connect(timerF1, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

    timerF2 = new QTimer(this);
    connect(timerF2, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

    timerRotationAngle = new QTimer(this);
    connect(timerRotationAngle, &QTimer::timeout, this, &DialogEllipticalArc::EvalAngles);

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

    connect(ui->toolButtonExprRadius1, &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius1);
    connect(ui->toolButtonExprRadius2, &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius2);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogEllipticalArc::FXF1);
    connect(ui->toolButtonExprF2, &QPushButton::clicked, this, &DialogEllipticalArc::FXF2);
    connect(ui->toolButtonExprRotationAngle, &QPushButton::clicked, this, &DialogEllipticalArc::FXRotationAngle);

    connect(ui->plainTextEditRadius1, &QPlainTextEdit::textChanged, this, &DialogEllipticalArc::Radius1Changed);
    connect(ui->plainTextEditRadius2, &QPlainTextEdit::textChanged, this, &DialogEllipticalArc::Radius2Changed);
    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this, &DialogEllipticalArc::F1Changed);
    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this, &DialogEllipticalArc::F2Changed);
    connect(ui->plainTextEditRotationAngle, &QPlainTextEdit::textChanged,
            this, &DialogEllipticalArc::RotationAngleChanged);

    connect(ui->pushButtonGrowLengthRadius1, &QPushButton::clicked, this, &DialogEllipticalArc::DeployRadius1TextEdit);
    connect(ui->pushButtonGrowLengthRadius2, &QPushButton::clicked, this, &DialogEllipticalArc::DeployRadius2TextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogEllipticalArc::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthF2, &QPushButton::clicked, this, &DialogEllipticalArc::DeployF2TextEdit);
    connect(ui->pushButtonGrowLengthRotationAngle, &QPushButton::clicked,
            this, &DialogEllipticalArc::DeployRotationAngleTextEdit);

    connect(ui->centerPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogEllipticalArc::pointNameChanged);

    vis = new VisToolEllipticalArc(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogEllipticalArc::~DialogEllipticalArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc DialogEllipticalArc::getArc() const
{
    return m_arc;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::setArc(const VEllipticalArc &arc)
{
    m_arc = arc;
    ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(m_arc.name()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCenter return id of center point
 * @return id id
 */
quint32 DialogEllipticalArc::GetCenter() const
{
    return getCurrentObjectId(ui->centerPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetCenter set id of center point
 * @param value id
 */
void DialogEllipticalArc::SetCenter(const quint32 &value)
{
    ChangeCurrentData(ui->centerPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius1 return formula of radius1
 * @return formula
 */
QString DialogEllipticalArc::GetRadius1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(radius1, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRadius1 set formula of radius1
 * @param value formula
 */
void DialogEllipticalArc::SetRadius1(const QString &value)
{
    radius1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (radius1.length() > 80)
    {
        this->DeployRadius1TextEdit();
    }
    ui->plainTextEditRadius1->setPlainText(radius1);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius1(radius1);

    MoveCursorToEnd(ui->plainTextEditRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius2 return formula of radius2
 * @return formula
 */
QString DialogEllipticalArc::GetRadius2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(radius2, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRadius2 set formula of radius2
 * @param value formula
 */
void DialogEllipticalArc::SetRadius2(const QString &value)
{
    radius2 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (radius2.length() > 80)
    {
        this->DeployRadius2TextEdit();
    }
    ui->plainTextEditRadius2->setPlainText(radius2);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius2(radius2);

    MoveCursorToEnd(ui->plainTextEditRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF1 return formula first angle of elliptical arc
 * @return formula
 */
QString DialogEllipticalArc::GetF1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(f1, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF1 set formula first angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetF1(const QString &value)
{
    f1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(f1);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetF2 return formula second angle of elliptical arc
 * @return formula
 */
QString DialogEllipticalArc::GetF2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(f2, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetF2 set formula second angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetF2(const QString &value)
{
    f2 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (f2.length() > 80)
    {
        this->DeployF2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(f2);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF2(f2);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRotationAngle return formula rotation angle of elliptical arc
 * @return formula
 */
QString DialogEllipticalArc::GetRotationAngle() const
{
    return qApp->translateVariables()->TryFormulaFromUser(rotationAngle, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetRotationAngle set formula rotation angle of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::SetRotationAngle(const QString &value)
{
    rotationAngle = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (rotationAngle.length() > 80)
    {
        this->DeployRotationAngleTextEdit();
    }
    ui->plainTextEditRotationAngle->setPlainText(rotationAngle);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRotationAngle(rotationAngle);

    MoveCursorToEnd(ui->plainTextEditRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::getPenStyle() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::setPenStyle(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogEllipticalArc::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogEllipticalArc::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetColor return color of elliptical arc
 * @return formula
 */
QString DialogEllipticalArc::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineColor set color of elliptical arc
 * @param value formula
 */
void DialogEllipticalArc::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalRadiuses calculate value of radiuses
 */
void DialogEllipticalArc::EvalRadiuses()
{
    labelEditFormula = ui->labelEditRadius1;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius_1 = Eval(ui->plainTextEditRadius1->toPlainText(), flagRadius1, ui->labelResultRadius1, postfix);

    if (radius_1 < 0)
    {
        flagRadius1 = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius1->setText(tr("Error"));
        ui->labelResultRadius1->setToolTip(tr("Radius can't be negative"));

        DialogEllipticalArc::CheckState();
    }

    labelEditFormula = ui->labelEditRadius2;
    const qreal radius_2 = Eval(ui->plainTextEditRadius2->toPlainText(), flagRadius2, ui->labelResultRadius2, postfix);
    if (radius_2 < 0)
    {
        flagRadius2 = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius2->setText(tr("Error"));
        ui->labelResultRadius2->setToolTip(tr("Radius can't be negative"));

        DialogEllipticalArc::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalAngles calculate value of angles
 */
void DialogEllipticalArc::EvalAngles()
{
    labelEditFormula = ui->labelEditF1;
    angleF1 = Eval(ui->plainTextEditF1->toPlainText(), flagF1, ui->labelResultF1, degreeSymbol, false);

    labelEditFormula = ui->labelEditF2;
    angleF2 = Eval(ui->plainTextEditF2->toPlainText(), flagF2, ui->labelResultF2, degreeSymbol, false);

    labelEditFormula = ui->labelEditRotationAngle;
    angleRotation = Eval(ui->plainTextEditRotationAngle->toPlainText(), flagRotationAngle,
                         ui->labelResultRotationAngle, degreeSymbol, false);

    CheckAngles();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::CheckAngles()
{
    if (static_cast<int>(angleF1) == INT_MIN || static_cast<int>(angleF2) == INT_MIN)
    {
        return;
    }

    if (VFuzzyComparePossibleNulls(angleF1, angleF2))
    {
        flagF1 = false;
        ChangeColor(ui->labelEditF1, Qt::red);
        ui->labelResultF1->setText(tr("Error"));
        ui->labelResultF1->setToolTip(tr("Angles equal"));

        flagF2 = false;
        ChangeColor(ui->labelEditF2, Qt::red);
        ui->labelResultF2->setText(tr("Error"));
        ui->labelResultF2->setToolTip(tr("Angles equal"));
    }

    DialogEllipticalArc::CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXRadius1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit radius1"));
    dialog->SetFormula(GetRadius1());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXRadius2()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit radius2"));
    dialog->SetFormula(GetRadius2());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRadius2(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXF1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit first angle"));
    dialog->SetFormula(GetF1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXF2()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit second angle"));
    dialog->SetFormula(GetF2());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetF2(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::FXRotationAngle()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit rotation angle"));
    dialog->SetFormula(GetRotationAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRotationAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Radius1Changed after change formula of radius1 calculate value and show result
 */
void DialogEllipticalArc::Radius1Changed()
{
    labelEditFormula = ui->labelEditRadius1;
    labelResultCalculation = ui->labelResultRadius1;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagRadius1, ui->plainTextEditRadius1, timerRadius1, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Radius2Changed after change formula of radius2 calculate value and show result
 */
void DialogEllipticalArc::Radius2Changed()
{
    labelEditFormula = ui->labelEditRadius2;
    labelResultCalculation = ui->labelResultRadius2;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagRadius2, ui->plainTextEditRadius2, timerRadius2, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief F1Changed after change formula of first angle calculate value and show result
 */
void DialogEllipticalArc::F1Changed()
{
    labelEditFormula = ui->labelEditF1;
    labelResultCalculation = ui->labelResultF1;
    ValFormulaChanged(flagF1, ui->plainTextEditF1, timerF1, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief F2Changed after change formula of second angle calculate value and show result
 */
void DialogEllipticalArc::F2Changed()
{
    labelEditFormula = ui->labelEditF2;
    labelResultCalculation = ui->labelResultF2;
    ValFormulaChanged(flagF2, ui->plainTextEditF2, timerF2, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RotationAngleChanged after change formula of rotation angle calculate value and show result
 */
void DialogEllipticalArc::RotationAngleChanged()
{
    labelEditFormula = ui->labelEditRotationAngle;
    labelResultCalculation = ui->labelResultF2;
    ValFormulaChanged(flagRotationAngle, ui->plainTextEditRotationAngle, timerRotationAngle, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployRadius1TextEdit()
{
    DeployFormula(ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, formulaBaseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, formulaBaseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
    collapseFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployRadius2TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, formulaBaseHeightRadius1);
    DeployFormula(ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, formulaBaseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
    collapseFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployF1TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, formulaBaseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, formulaBaseHeightRadius2);
    DeployFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
    collapseFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployF2TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, formulaBaseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, formulaBaseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
    DeployFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::DeployRotationAngleTextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->pushButtonGrowLengthRadius1, formulaBaseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->pushButtonGrowLengthRadius2, formulaBaseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
    collapseFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
    DeployFormula(ui->plainTextEditRotationAngle, ui->pushButtonGrowLengthRotationAngle,formulaBaseHeightRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogEllipticalArc::ChosenObject(quint32 id, const SceneObject &type)
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
void DialogEllipticalArc::pointNameChanged()
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
        VEllipticalArc arc(*data->GeometricObject<VPointF>(getCurrentObjectId(ui->centerPoint_ComboBox)),
                           GetRadius1().toDouble(),
                           GetRadius2().toDouble(),
                           GetF1().toDouble(),
                           GetF2().toDouble(),
                           GetRotationAngle().toDouble());

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
/**
 * @brief CheckState if all is right enable button ok
 */
void DialogEllipticalArc::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagRadius1 && flagRadius2 && flagF1 && flagF2 && flagRotationAngle);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(flagRadius1 && flagRadius2 && flagF1 && flagF2 && flagRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::ShowVisualization()
{
    AddVisualization<VisToolEllipticalArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SaveData()
{
    radius1 = ui->plainTextEditRadius1->toPlainText();
    radius1.replace("\n", " ");
    radius2 = ui->plainTextEditRadius2->toPlainText();
    radius2.replace("\n", " ");
    f1 = ui->plainTextEditF1->toPlainText();
    f1.replace("\n", " ");
    f2 = ui->plainTextEditF2->toPlainText();
    f2.replace("\n", " ");
    rotationAngle = ui->plainTextEditRotationAngle->toPlainText();
    rotationAngle.replace("\n", " ");

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(GetCenter());
    path->setRadius1(radius1);
    path->setRadius2(radius2);
    path->setF1(f1);
    path->setF2(f2);
    path->setRotationAngle(rotationAngle);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius1->blockSignals(true);
    ui->plainTextEditRadius2->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditF2->blockSignals(true);
    ui->plainTextEditRotationAngle->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::collapseFormula(QPlainTextEdit *textEdit, QPushButton *pushButton, int height)
{
    SCASSERT(textEdit != nullptr)
    SCASSERT(pushButton != nullptr)

    const QTextCursor cursor = textEdit->textCursor();

    setMaximumWidth(260);
    textEdit->setFixedHeight(height);
    pushButton->setIcon(QIcon::fromTheme("go-down", QIcon(":/icons/win.icon.theme/16x16/actions/go-down.png")));
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);
    textEdit->setFocus();
    textEdit->setTextCursor(cursor);
}

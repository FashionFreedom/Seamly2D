//-----------------------------------------------------------------------------
//  @file   dialogarc.cpp
//  @author Douglas S Caskey
//  @date   21 Mar, 2023
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
//  @file   dialogarc.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
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


#include "dialogarc.h"

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
#include "../vgeometry/vpointf.h"

#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/path/vistoolarc.h"
#include "../../visualization/visualization.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogarc.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogArc create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogArc::DialogArc(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogArc)
    , flagRadius(false)
    , flagF1(false)
    , flagF2(false)
    , timerRadius(nullptr)
    , timerF1(nullptr)
    , timerF2(nullptr)
    , radius(QString())
    , f1(QString())
    , f2(QString())
    , formulaBaseHeight(0)
    , formulaBaseHeightF1(0)
    , formulaBaseHeightF2(0)
    , angleF1(INT_MIN)
    , angleF2(INT_MIN)
    , m_arc()
    , m_Id()
    , newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/arc.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    m_Id  = data->getId();
    plainTextEditFormula = ui->plainTextEditFormula;
    this->formulaBaseHeight = ui->plainTextEditFormula->height();
    this->formulaBaseHeightF1 = ui->plainTextEditF1->height();
    this->formulaBaseHeightF2 = ui->plainTextEditF2->height();

    ui->plainTextEditFormula->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF2->installEventFilter(this);

    timerRadius = new QTimer(this);
    connect(timerRadius, &QTimer::timeout, this, &DialogArc::EvalRadius);

    timerF1 = new QTimer(this);
    connect(timerF1, &QTimer::timeout, this, &DialogArc::EvalF);

    timerF2 = new QTimer(this);
    connect(timerF2, &QTimer::timeout, this, &DialogArc::EvalF);

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

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArc::FXRadius);
    connect(ui->toolButtonExprF1, &QPushButton::clicked, this, &DialogArc::FXF1);
    connect(ui->toolButtonExprF2, &QPushButton::clicked, this, &DialogArc::FXF2);

    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogArc::RadiusChanged);
    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this, &DialogArc::F1Changed);
    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this, &DialogArc::F2Changed);

    connect(ui->pushButtonGrowLength, &QPushButton::clicked, this, &DialogArc::DeployFormulaTextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogArc::DeployF1TextEdit);
    connect(ui->pushButtonGrowLengthF2, &QPushButton::clicked, this, &DialogArc::DeployF2TextEdit);

    connect(ui->centerPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogArc::pointNameChanged);

    vis = new VisToolArc(data);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployFormulaTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, formulaBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployF1TextEdit()
{
    DeployFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, formulaBaseHeightF1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::DeployF2TextEdit()
{
    DeployFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, formulaBaseHeightF2);
}

//---------------------------------------------------------------------------------------------------------------------
DialogArc::~DialogArc()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
VArc DialogArc::getArc() const
{
    return m_arc;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setArc(const VArc &arc)
{
    m_arc = arc;
    ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(m_arc.name()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCenter set id of center point
 * @param value id
 */
void DialogArc::setCenter(const quint32 &value)
{
    ChangeCurrentData(ui->centerPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setF2 set formula second angle of arc
 * @param value formula
 */
void DialogArc::setF2(const QString &value)
{
    f2 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (f2.length() > 80)
    {
        this->DeployF2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(f2);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF2(f2);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getPenStyle() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setPenStyle(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogArc::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogArc::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setF1 set formula first angle of arc
 * @param value formula
 */
void DialogArc::setF1(const QString &value)
{
    f1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (f1.length() > 80)
    {
        this->DeployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(f1);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(f1);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setRadius set formula of radius
 * @param value formula
 */
void DialogArc::setRadius(const QString &value)
{
    radius = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (radius.length() > 80)
    {
        this->DeployFormulaTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(radius);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius(radius);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogArc::ChosenObject(quint32 id, const SceneObject &type)
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
void DialogArc::ShowVisualization()
{
    AddVisualization<VisToolArc>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::SaveData()
{
    radius = ui->plainTextEditFormula->toPlainText();
    radius.replace("\n", " ");
    f1 = ui->plainTextEditF1->toPlainText();
    f1.replace("\n", " ");
    f2 = ui->plainTextEditF2->toPlainText();
    f2.replace("\n", " ");

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(getCenter());
    path->setRadius(radius);
    path->setF1(f1);
    path->setF2(f2);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    ui->plainTextEditF1->blockSignals(true);
    ui->plainTextEditF2->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RadiusChanged after change formula of radius calculate value and show result
 */
void DialogArc::RadiusChanged()
{
    labelEditFormula = ui->labelEditRadius;
    labelResultCalculation = ui->labelResultRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagRadius, ui->plainTextEditFormula, timerRadius, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief F1Changed after change formula of first angle calculate value and show result
 */
void DialogArc::F1Changed()
{
    labelEditFormula = ui->labelEditF1;
    labelResultCalculation = ui->labelResultF1;
    ValFormulaChanged(flagF1, ui->plainTextEditF1, timerF1, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief F2Changed after change formula of second angle calculate value and show result
 */
void DialogArc::F2Changed()
{
    labelEditFormula = ui->labelEditF2;
    labelResultCalculation = ui->labelResultF2;
    ValFormulaChanged(flagF2, ui->plainTextEditF2, timerF2, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXRadius()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius"));
    dialog->SetFormula(getRadius());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXF1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit first angle"));
    dialog->SetFormula(getF1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        setF1(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::FXF2()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit second angle"));
    dialog->SetFormula(getF2());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        setF2(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::pointNameChanged()
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
             getRadius().toDouble(),
             getF1().toDouble(),
             getF2().toDouble());

        if (!data->IsUnique(arc.name()))
        {
            newDuplicate = static_cast<qint32>(DNumber(arc.name()));
            arc.SetDuplicate(static_cast<quint32>(newDuplicate));
        }
        if (m_arc.id() == NULL_ID)
        {
            ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(arc.name() + "_" + QString().setNum(m_Id + 1)));
        }
        else
        {
            ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(arc.name() + "_" + QString().setNum(m_arc.id())));
        }
    }

    ChangeColor(ui->name_Label, color);
    ChangeColor(ui->centerPoint_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckState if all is right enable button ok
 */
void DialogArc::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagRadius && flagF1 && flagF2);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(flagRadius && flagF1 && flagF2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalRadius calculate value of radius
 */
void DialogArc::EvalRadius()
{
    labelEditFormula = ui->labelEditRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditFormula->toPlainText(), flagRadius, ui->labelResultRadius, postfix);

    if (radius < 0)
    {
        flagRadius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius->setText(tr("Error"));
        ui->labelResultRadius->setToolTip(tr("Radius can't be negative"));

        DialogArc::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalF1 calculate value of angle
 */
void DialogArc::EvalF()
{
    labelEditFormula = ui->labelEditF1;
    angleF1 = Eval(ui->plainTextEditF1->toPlainText(), flagF1, ui->labelResultF1, degreeSymbol, false);

    labelEditFormula = ui->labelEditF2;
    angleF2 = Eval(ui->plainTextEditF2->toPlainText(), flagF2, ui->labelResultF2, degreeSymbol, false);

    CheckAngles();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::CheckAngles()
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

    DialogArc::CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getCenter return id of center point
 * @return id id
 */
quint32 DialogArc::getCenter() const
{
    return getCurrentObjectId(ui->centerPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getRadius return formula of radius
 * @return formula
 */
QString DialogArc::getRadius() const
{
    return qApp->translateVariables()->TryFormulaFromUser(radius, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getF1 return formula first angle of arc
 * @return formula
 */
QString DialogArc::getF1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(f1, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getF2 return formula second angle of arc
 * @return formula
 */
QString DialogArc::getF2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(f2, qApp->Settings()->getOsSeparator());
}

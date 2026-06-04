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
/// @brief DialogEllipticalArc create dialog
/// @param data container with data
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
DialogEllipticalArc::DialogEllipticalArc(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogEllipticalArc)
    , m_flagRadius1(false)
    , m_flagRadius2(false)
    , m_flagAngle1(false)
    , m_flagAngle2(false)
    , m_flagRotation(false)
    , m_timerRadius1(nullptr)
    , m_timerRadius2(nullptr)
    , m_timerAngle1(nullptr)
    , m_timerAngle2(nullptr)
    , m_timerRotation(nullptr)
    , m_radius1Fx()
    , m_radius2Fx()
    , m_angle1Fx()
    , m_angle2Fx()
    , m_rotationFx()
    , m_baseHeightRadius1(0)
    , m_baseHeightRadius2(0)
    , m_baseHeightAngle1(0)
    , m_baseHeightAngle2(0)
    , m_baseHeightRotation(0)
    , m_angleF1(INT_MIN)
    , m_angleF2(INT_MIN)
    , m_rotationAngle(INT_MIN)
    , m_arc()
    , m_Id()
    , m_newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/el_arc.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    m_Id  = data->getId() + 1;

    this->m_baseHeightRadius1 = ui->plainTextEditRadius1->height();
    this->m_baseHeightRadius2 = ui->plainTextEditRadius2->height();
    this->m_baseHeightAngle1 = ui->plainTextEditF1->height();
    this->m_baseHeightAngle2 = ui->plainTextEditF2->height();
    this->m_baseHeightRotation = ui->plainTextEditRotationAngle->height();

    ui->plainTextEditRadius1->installEventFilter(this);
    ui->plainTextEditRadius2->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF1->setToolTip(makeAngleTooltip());
    ui->plainTextEditF2->installEventFilter(this);
    ui->plainTextEditF2->setToolTip(makeAngleTooltip());
    ui->plainTextEditRotationAngle->installEventFilter(this);
    ui->plainTextEditRotationAngle->setToolTip(makeAngleTooltip());

    m_timerRadius1 = new QTimer(this);
    connect(m_timerRadius1, &QTimer::timeout, this, &DialogEllipticalArc::evalRadiuses);

    m_timerRadius2 = new QTimer(this);
    connect(m_timerRadius2, &QTimer::timeout, this, &DialogEllipticalArc::evalRadiuses);

    m_timerAngle1 = new QTimer(this);
    connect(m_timerAngle1, &QTimer::timeout, this, &DialogEllipticalArc::evalAngles);

    m_timerAngle2 = new QTimer(this);
    connect(m_timerAngle2, &QTimer::timeout, this, &DialogEllipticalArc::evalAngles);

    m_timerRotation = new QTimer(this);
    connect(m_timerRotation, &QTimer::timeout, this, &DialogEllipticalArc::evalAngles);

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

    connect(ui->toolButtonExprRadius1,       &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius1);
    connect(ui->toolButtonExprRadius2,       &QPushButton::clicked, this, &DialogEllipticalArc::FXRadius2);
    connect(ui->toolButtonExprF1,            &QPushButton::clicked, this, &DialogEllipticalArc::FXF1);
    connect(ui->toolButtonExprF2,            &QPushButton::clicked, this, &DialogEllipticalArc::FXF2);
    connect(ui->toolButtonExprRotationAngle, &QPushButton::clicked, this, &DialogEllipticalArc::FXRotationAngle);

    connect(ui->plainTextEditRadius1, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerRadius1->start(std::chrono::milliseconds(300));});
    connect(ui->plainTextEditRadius2, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerRadius2->start(std::chrono::milliseconds(300));});
    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerAngle1->start(std::chrono::milliseconds(300));});
    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerAngle2->start(std::chrono::milliseconds(300));});
    connect(ui->plainTextEditRotationAngle, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerRotation->start(std::chrono::milliseconds(300));});

    connect(ui->radius1_PushButton,  &QPushButton::clicked, this, &DialogEllipticalArc::deployRadius1TextEdit);
    connect(ui->radius2_PushButton,  &QPushButton::clicked, this, &DialogEllipticalArc::deployRadius2TextEdit);
    connect(ui->angle1_PushButton,   &QPushButton::clicked, this, &DialogEllipticalArc::deployF1TextEdit);
    connect(ui->angle2_PushButton,   &QPushButton::clicked, this, &DialogEllipticalArc::deployF2TextEdit);
    connect(ui->rotation_PushButton, &QPushButton::clicked, this, &DialogEllipticalArc::deployRotationAngleTextEdit);

    connect(ui->centerPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogEllipticalArc::pointNameChanged);

    // Set default values angles
    ui->plainTextEditF1->setPlainText("0");
    ui->plainTextEditF2->setPlainText("360");
    ui->plainTextEditRotationAngle->setPlainText("0");
    ui->plainTextEditRadius1->setFocus();

    CheckState();

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
/// @brief GetCenter return id of center point
/// @return id id
//---------------------------------------------------------------------------------------------------------------------
quint32 DialogEllipticalArc::GetCenter() const
{
    return getCurrentObjectId(ui->centerPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetCenter set id of center point
/// @param value id
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetCenter(const quint32 &value)
{
    changeCurrentData(ui->centerPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetRadius1 return formula of m_radius1Fx
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::GetRadius1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_radius1Fx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRadius1 set formula of m_radius1Fx
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetRadius1(const QString &value)
{
    m_radius1Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_radius1Fx.length() > 80)
    {
        this->deployRadius1TextEdit();
    }
    ui->plainTextEditRadius1->setPlainText(m_radius1Fx);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius1(m_radius1Fx);

    MoveCursorToEnd(ui->plainTextEditRadius1);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetRadius2 return formula of m_radius2Fx
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::GetRadius2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_radius2Fx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRadius2 set formula of m_radius2Fx
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetRadius2(const QString &value)
{
    m_radius2Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_radius2Fx.length() > 80)
    {
        this->deployRadius2TextEdit();
    }
    ui->plainTextEditRadius2->setPlainText(m_radius2Fx);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius2(m_radius2Fx);

    MoveCursorToEnd(ui->plainTextEditRadius2);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetF1 return formula first angle of elliptical arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::GetF1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle1Fx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetF1 set formula first angle of elliptical arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetF1(const QString &value)
{
    m_angle1Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_angle1Fx.length() > 80)
    {
        this->deployF1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(m_angle1Fx);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(m_angle1Fx);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief GetF2 return formula second angle of elliptical arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::GetF2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle2Fx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetF2 set formula second angle of elliptical arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetF2(const QString &value)
{
    m_angle2Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_angle2Fx.length() > 80)
    {
        this->deployF2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(m_angle2Fx);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF2(m_angle2Fx);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getRotationAngle return formula rotation angle of elliptical arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::getRotationAngle() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_rotationFx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SetRotationAngle set formula rotation angle of elliptical arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::SetRotationAngle(const QString &value)
{
    m_rotationFx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_rotationFx.length() > 80)
    {
        this->deployRotationAngleTextEdit();
    }
    ui->plainTextEditRotationAngle->setPlainText(m_rotationFx);

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRotationAngle(m_rotationFx);

    MoveCursorToEnd(ui->plainTextEditRotationAngle);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::getPenStyle() const
{
    return getComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::setPenStyle(const QString &value)
{
    changeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getLineWeight return weight of the lines
/// @return type
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::getLineWeight() const
{
        return getComboBoxCurrentData(ui->lineWeight_ComboBox, DefaultLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLineWeight set weight of the lines
/// @param value type
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::setLineWeight(const QString &value)
{
    changeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}
//---------------------------------------------------------------------------------------------------------------------
/// @brief GetColor return color of elliptical arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogEllipticalArc::getLineColor() const
{
    return getComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLineColor set color of elliptical arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::setLineColor(const QString &value)
{
    changeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief evalRadiuses calculate value of radiuses
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::evalRadiuses()
{
    labelEditFormula = ui->labelEditRadius1;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius_1 = Eval(ui->plainTextEditRadius1->toPlainText(), m_flagRadius1, ui->labelResultRadius1, postfix);

    if (radius_1 < 0)
    {
        m_flagRadius1 = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius1->setText(tr("Error"));
        ui->labelResultRadius1->setToolTip(tr("Radius can't be negative"));

        DialogEllipticalArc::CheckState();
    }

    labelEditFormula = ui->labelEditRadius2;
    const qreal radius_2 = Eval(ui->plainTextEditRadius2->toPlainText(), m_flagRadius2, ui->labelResultRadius2, postfix);
    if (radius_2 < 0)
    {
        m_flagRadius2 = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius2->setText(tr("Error"));
        ui->labelResultRadius2->setToolTip(tr("Radius can't be negative"));

        DialogEllipticalArc::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief evalAngles calculate value of angles
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::evalAngles()
{
    labelEditFormula = ui->labelEditF1;
    m_angleF1 = Eval(ui->plainTextEditF1->toPlainText(), m_flagAngle1, ui->labelResultF1, degreeSymbol, false);

    labelEditFormula = ui->labelEditF2;
    m_angleF2 = Eval(ui->plainTextEditF2->toPlainText(), m_flagAngle2, ui->labelResultF2, degreeSymbol, false);

    labelEditFormula = ui->labelEditRotationAngle;
    m_rotationAngle = Eval(ui->plainTextEditRotationAngle->toPlainText(), m_flagRotation,
                           ui->labelResultRotationAngle, degreeSymbol, false);

    checkAngles();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::checkAngles()
{
    if (static_cast<int>(m_angleF1) == INT_MIN || static_cast<int>(m_angleF2) == INT_MIN)
    {
        return;
    }

    if (VFuzzyComparePossibleNulls(m_angleF1, m_angleF2))
    {
        m_flagAngle1 = false;
        ChangeColor(ui->labelEditF1, Qt::red);
        ui->labelResultF1->setText(tr("Error"));
        ui->labelResultF1->setToolTip(tr("Angles equal"));

        m_flagAngle2 = false;
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
    dialog->SetFormula(getRotationAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetRotationAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief deployRadius1TextEdit grow or shrink formula input
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::deployRadius1TextEdit()
{
    DeployFormula(ui->plainTextEditRadius1, ui->radius1_PushButton, m_baseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->radius2_PushButton, m_baseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->angle1_PushButton, m_baseHeightAngle1);
    collapseFormula(ui->plainTextEditF2, ui->angle2_PushButton, m_baseHeightAngle2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->rotation_PushButton,m_baseHeightRotation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief deployRadius1TextEdit grow or shrink formula input
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::deployRadius2TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->radius1_PushButton, m_baseHeightRadius1);
    DeployFormula(ui->plainTextEditRadius2, ui->radius2_PushButton, m_baseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->angle1_PushButton, m_baseHeightAngle1);
    collapseFormula(ui->plainTextEditF2, ui->angle2_PushButton, m_baseHeightAngle2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->rotation_PushButton,m_baseHeightRotation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief deployF1TextEdit grow or shrink formula input
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::deployF1TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->radius1_PushButton, m_baseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->radius2_PushButton, m_baseHeightRadius2);
    DeployFormula(ui->plainTextEditF1, ui->angle1_PushButton, m_baseHeightAngle1);
    collapseFormula(ui->plainTextEditF2, ui->angle2_PushButton, m_baseHeightAngle2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->rotation_PushButton,m_baseHeightRotation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief deployF2TextEdit grow or shrink formula input
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::deployF2TextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->radius1_PushButton, m_baseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->radius2_PushButton, m_baseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->angle1_PushButton, m_baseHeightAngle1);
    DeployFormula(ui->plainTextEditF2, ui->angle2_PushButton, m_baseHeightAngle2);
    collapseFormula(ui->plainTextEditRotationAngle, ui->rotation_PushButton,m_baseHeightRotation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief deployRotationAngleTextEdit grow or shrink formula input
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::deployRotationAngleTextEdit()
{
    collapseFormula(ui->plainTextEditRadius1, ui->radius1_PushButton, m_baseHeightRadius1);
    collapseFormula(ui->plainTextEditRadius2, ui->radius2_PushButton, m_baseHeightRadius2);
    collapseFormula(ui->plainTextEditF1, ui->angle1_PushButton, m_baseHeightAngle1);
    collapseFormula(ui->plainTextEditF2, ui->angle2_PushButton, m_baseHeightAngle2);
    DeployFormula(ui->plainTextEditRotationAngle, ui->rotation_PushButton,m_baseHeightRotation);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
/// @param id id of point or detail
/// @param type type of object
//---------------------------------------------------------------------------------------------------------------------
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
        m_newDuplicate = -1;
        ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(m_arc.name()));
    }
    else
    {
        VEllipticalArc arc(*data->GeometricObject<VPointF>(getCurrentObjectId(ui->centerPoint_ComboBox)),
                           GetRadius1().toDouble(),
                           GetRadius2().toDouble(),
                           GetF1().toDouble(),
                           GetF2().toDouble(),
                           getRotationAngle().toDouble());

        if (!data->IsUnique(arc.name()))
        {
            m_newDuplicate = static_cast<qint32>(DNumber(arc.name()));
            arc.SetDuplicate(static_cast<quint32>(m_newDuplicate));
        }
        ui->name_LineEdit->setText(qApp->translateVariables()->VarToUser(arc.name() + "_" + QString().setNum(m_Id)));
    }

    ChangeColor(ui->name_Label, color);
    ChangeColor(ui->centerPoint_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief CheckState if all is right enable button ok
//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(m_flagRadius1 && m_flagRadius2 && m_flagAngle1 && m_flagAngle2 && m_flagRotation);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(m_flagRadius1 && m_flagRadius2 && m_flagAngle1 && m_flagAngle2 && m_flagRotation);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogEllipticalArc::ShowVisualization()
{
    AddVisualization<VisToolEllipticalArc>();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief SaveData Put dialog data in local variables
//---------------------------------------------------------------------------------------------------------------------
 void DialogEllipticalArc::SaveData()
{
    m_radius1Fx = ui->plainTextEditRadius1->toPlainText();
    m_radius1Fx.replace("\n", " ");
    m_radius2Fx = ui->plainTextEditRadius2->toPlainText();
    m_radius2Fx.replace("\n", " ");
    m_angle1Fx = ui->plainTextEditF1->toPlainText();
    m_angle1Fx.replace("\n", " ");
    m_angle2Fx = ui->plainTextEditF2->toPlainText();
    m_angle2Fx.replace("\n", " ");
    m_rotationFx = ui->plainTextEditRotationAngle->toPlainText();
    m_rotationFx.replace("\n", " ");

    VisToolEllipticalArc *path = qobject_cast<VisToolEllipticalArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(GetCenter());
    path->setRadius1(m_radius1Fx);
    path->setRadius2(m_radius2Fx);
    path->setF1(m_angle1Fx);
    path->setF2(m_angle2Fx);
    path->setRotationAngle(m_rotationFx);
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

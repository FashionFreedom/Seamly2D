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
#include <chrono>
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
/// @brief DialogArc create dialog
/// @param data container with data
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
DialogArc::DialogArc(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogArc)
    , m_flagRadius(false)
    , m_flagAngle1(false)
    , m_flagAngle2(false)
    , m_timerRadius(nullptr)
    , m_timerAngle1(nullptr)
    , m_timerAngle2(nullptr)
    , m_radiusFx(QString())
    , m_angle1Fx(QString())
    , m_angle2Fx(QString())
    , m_baseHeightRadius(0)
    , m_baseHeightAngle1(0)
    , m_baseHeightAngle2(0)
    , m_angle1(INT_MIN)
    , m_angle2(INT_MIN)
    , m_arc()
    , m_Id()
    , m_newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/arc.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    m_Id  = data->getId();
    plainTextEditFormula = ui->plainTextEditFormula;
    this->m_baseHeightRadius = ui->plainTextEditFormula->height();
    this->m_baseHeightAngle1 = ui->plainTextEditF1->height();
    this->m_baseHeightAngle2 = ui->plainTextEditF2->height();

    ui->plainTextEditFormula->installEventFilter(this);
    ui->plainTextEditF1->installEventFilter(this);
    ui->plainTextEditF1->setToolTip(makeAngleTooltip());
    ui->plainTextEditF2->installEventFilter(this);
    ui->plainTextEditF2->setToolTip(makeAngleTooltip());

    m_timerRadius = new QTimer(this);
    connect(m_timerRadius, &QTimer::timeout, this, &DialogArc::evalRadius);

    m_timerAngle1 = new QTimer(this);
    connect(m_timerAngle1, &QTimer::timeout, this, &DialogArc::evalAngles);

    m_timerAngle2 = new QTimer(this);
    connect(m_timerAngle2, &QTimer::timeout, this, &DialogArc::evalAngles);

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

    connect(ui->toolButtonExprRadius, &QPushButton::clicked, this, &DialogArc::radiusFX);
    connect(ui->toolButtonExprF1,     &QPushButton::clicked, this, &DialogArc::angle1FX);
    connect(ui->toolButtonExprF2,     &QPushButton::clicked, this, &DialogArc::angle2FX);

    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerRadius->start(std::chrono::milliseconds(300));});

    connect(ui->plainTextEditF1, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerAngle1->start(std::chrono::milliseconds(300));});

    connect(ui->plainTextEditF2, &QPlainTextEdit::textChanged, this,
            [this]() {m_timerAngle2->start(std::chrono::milliseconds(300));});

    connect(ui->pushButtonGrowLength,   &QPushButton::clicked, this, &DialogArc::deployRadiusTextEdit);
    connect(ui->pushButtonGrowLengthF1, &QPushButton::clicked, this, &DialogArc::deployAngle1TextEdit);
    connect(ui->pushButtonGrowLengthF2, &QPushButton::clicked, this, &DialogArc::deployAngle2TextEdit);

    connect(ui->centerPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogArc::pointNameChanged);

    // Set default values for angles
    ui->plainTextEditF1->setPlainText("0");
    ui->plainTextEditF2->setPlainText("360");
    ui->plainTextEditFormula->setFocus();

    CheckState();

    vis = new VisToolArc(data);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::deployRadiusTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLength, m_baseHeightRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::deployAngle1TextEdit()
{
    DeployFormula(ui->plainTextEditF1, ui->pushButtonGrowLengthF1, m_baseHeightAngle1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::deployAngle2TextEdit()
{
    DeployFormula(ui->plainTextEditF2, ui->pushButtonGrowLengthF2, m_baseHeightAngle2);
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
/// @brief setCenter set id of center point
/// @param value id
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setCenter(const quint32 &value)
{
    changeCurrentData(ui->centerPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setF2 set formula second angle of arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setF2(const QString &value)
{
    m_angle2Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_angle2Fx.length() > 80)
    {
        this->deployAngle2TextEdit();
    }
    ui->plainTextEditF2->setPlainText(m_angle2Fx);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF2(m_angle2Fx);

    MoveCursorToEnd(ui->plainTextEditF2);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getPenStyle() const
{
    return getComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setPenStyle(const QString &value)
{
    changeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getLineWeight return weight of the lines
/// @return type
//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getLineWeight() const
{
        return getComboBoxCurrentData(ui->lineWeight_ComboBox, DefaultLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setLineWeight set weight of the lines
/// @param value type
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setLineWeight(const QString &value)
{
    changeCurrentData(ui->lineWeight_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getLineColor() const
{
    return getComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setLineColor(const QString &value)
{
    changeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setF1 set formula first angle of arc
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setF1(const QString &value)
{
    m_angle1Fx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_angle1Fx.length() > 80)
    {
        this->deployAngle1TextEdit();
    }
    ui->plainTextEditF1->setPlainText(m_angle1Fx);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setF1(m_angle1Fx);

    MoveCursorToEnd(ui->plainTextEditF1);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setRadius set formula of radius
/// @param value formula
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::setRadius(const QString &value)
{
    m_radiusFx = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (m_radiusFx.length() > 80)
    {
        this->deployRadiusTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(m_radiusFx);

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)
    path->setRadius(m_radiusFx);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
/// @param id id of point or detail
/// @param type type of object
//---------------------------------------------------------------------------------------------------------------------
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
/// @brief SaveData Put dialog data in local variables
//---------------------------------------------------------------------------------------------------------------------
 void DialogArc::SaveData()
{
    m_radiusFx = ui->plainTextEditFormula->toPlainText();
    m_radiusFx.replace("\n", " ");
    m_angle1Fx = ui->plainTextEditF1->toPlainText();
    m_angle1Fx.replace("\n", " ");
    m_angle2Fx = ui->plainTextEditF2->toPlainText();
    m_angle2Fx.replace("\n", " ");

    VisToolArc *path = qobject_cast<VisToolArc *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(getCenter());
    path->setRadius(m_radiusFx);
    path->setF1(m_angle1Fx);
    path->setF2(m_angle2Fx);
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
void DialogArc::radiusFX()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
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
void DialogArc::angle1FX()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
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
void DialogArc::angle2FX()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
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
        m_newDuplicate = -1;
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
            m_newDuplicate = static_cast<qint32>(DNumber(arc.name()));
            arc.SetDuplicate(static_cast<quint32>(m_newDuplicate));
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
/// @brief CheckState if all is right enable button ok
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(m_flagRadius && m_flagAngle1 && m_flagAngle2);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(m_flagRadius && m_flagAngle1 && m_flagAngle2);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief evalRadius calculate value of radius
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::evalRadius()
{
    labelEditFormula = ui->labelEditRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditFormula->toPlainText(), m_flagRadius, ui->labelResultRadius, postfix);

    if (radius < 0)
    {
        m_flagRadius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultRadius->setText(tr("Error"));
        ui->labelResultRadius->setToolTip(tr("Radius can't be negative"));

        DialogArc::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief evalAngles1 calculate value of angle
//---------------------------------------------------------------------------------------------------------------------
void DialogArc::evalAngles()
{
    labelEditFormula = ui->labelEditF1;
    m_angle1 = Eval(ui->plainTextEditF1->toPlainText(), m_flagAngle1, ui->labelResultF1, degreeSymbol, false);

    labelEditFormula = ui->labelEditF2;
    m_angle2 = Eval(ui->plainTextEditF2->toPlainText(), m_flagAngle2, ui->labelResultF2, degreeSymbol, false);

    checkAngles();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogArc::checkAngles()
{
    if (static_cast<int>(m_angle1) == INT_MIN || static_cast<int>(m_angle2) == INT_MIN)
    {
        return;
    }

    if (VFuzzyComparePossibleNulls(m_angle1, m_angle2))
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

    DialogArc::CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getCenter return id of center point
/// @return id id
//---------------------------------------------------------------------------------------------------------------------
quint32 DialogArc::getCenter() const
{
    return getCurrentObjectId(ui->centerPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getRadius return formula of radius
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getRadius() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_radiusFx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getF1 return formula first angle of arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getF1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle1Fx, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getF2 return formula second angle of arc
/// @return formula
//---------------------------------------------------------------------------------------------------------------------
QString DialogArc::getF2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle2Fx, qApp->Settings()->getOsSeparator());
}

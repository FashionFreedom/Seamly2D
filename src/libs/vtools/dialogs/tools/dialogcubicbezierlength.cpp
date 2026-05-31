/***************************************************************************
 **  @file   dialogcubicbezierlength.cpp
 **  @author Seamly2D contributors
 **
 **  @brief  Dialog for the cubic Bézier curve with matched length tool.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#include "dialogcubicbezierlength.h"

#include <QDialog>
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
#include "../../visualization/path/vistoolcubicbezierlength.h"

#include "ui_dialogcubicbezierlength.h"

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezierLength::DialogCubicBezierLength(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCubicBezierLength)
    , flagAngle1(false)
    , flagC1Length(false)
    , flagAngle2(false)
    , flagTargetLength(false)
    , timerAngle1(nullptr)
    , timerC1Length(nullptr)
    , timerAngle2(nullptr)
    , timerTargetLength(nullptr)
    , m_angle1(QString())
    , m_c1Length(QString())
    , m_angle2(QString())
    , m_targetLength(QString())
    , formulaBaseHeightAngle1(0)
    , formulaBaseHeightC1Length(0)
    , formulaBaseHeightAngle2(0)
    , formulaBaseHeightTargetLength(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/cubic_bezier_length.png"));
    setDialogPosition();

    plainTextEditFormula = ui->plainTextEditAngle1;
    formulaBaseHeightAngle1        = ui->plainTextEditAngle1->height();
    formulaBaseHeightC1Length      = ui->plainTextEditC1Length->height();
    formulaBaseHeightAngle2        = ui->plainTextEditAngle2->height();
    formulaBaseHeightTargetLength  = ui->plainTextEditTargetLength->height();

    ui->plainTextEditAngle1->installEventFilter(this);
    ui->plainTextEditC1Length->installEventFilter(this);
    ui->plainTextEditAngle2->installEventFilter(this);
    ui->plainTextEditTargetLength->installEventFilter(this);

    timerAngle1       = new QTimer(this);
    timerC1Length     = new QTimer(this);
    timerAngle2       = new QTimer(this);
    timerTargetLength = new QTimer(this);
    connect(timerAngle1,       &QTimer::timeout, this, &DialogCubicBezierLength::EvalAngle1);
    connect(timerC1Length,     &QTimer::timeout, this, &DialogCubicBezierLength::EvalC1Length);
    connect(timerAngle2,       &QTimer::timeout, this, &DialogCubicBezierLength::EvalAngle2);
    connect(timerTargetLength, &QTimer::timeout, this, &DialogCubicBezierLength::EvalTargetLength);

    initializeOkCancelApply(ui);

    fillComboBoxPoints(ui->startPoint_ComboBox);
    fillComboBoxPoints(ui->endPoint_ComboBox);

    // Remove "no line" option from linetype
    int idx = ui->lineType_ComboBox->findData(LineTypeNone);
    if (idx != -1) ui->lineType_ComboBox->removeItem(idx);

    // Apply document defaults
    idx = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (idx != -1) ui->lineColor_ComboBox->setCurrentIndex(idx);

    idx = ui->lineWeight_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineWeight());
    if (idx != -1) ui->lineWeight_ComboBox->setCurrentIndex(idx);

    idx = ui->lineType_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineType());
    if (idx != -1) ui->lineType_ComboBox->setCurrentIndex(idx);

    CheckState();

    // FX buttons
    connect(ui->toolButtonExprAngle1,       &QToolButton::clicked, this, &DialogCubicBezierLength::FXAngle1);
    connect(ui->toolButtonExprC1Length,     &QToolButton::clicked, this, &DialogCubicBezierLength::FXC1Length);
    connect(ui->toolButtonExprAngle2,       &QToolButton::clicked, this, &DialogCubicBezierLength::FXAngle2);
    connect(ui->toolButtonExprTargetLength, &QToolButton::clicked, this, &DialogCubicBezierLength::FXTargetLength);

    // Text-changed signals
    connect(ui->plainTextEditAngle1,       &QPlainTextEdit::textChanged, this, &DialogCubicBezierLength::Angle1Changed);
    connect(ui->plainTextEditC1Length,     &QPlainTextEdit::textChanged, this, &DialogCubicBezierLength::C1LengthChanged);
    connect(ui->plainTextEditAngle2,       &QPlainTextEdit::textChanged, this, &DialogCubicBezierLength::Angle2Changed);
    connect(ui->plainTextEditTargetLength, &QPlainTextEdit::textChanged, this, &DialogCubicBezierLength::TargetLengthChanged);

    // Grow buttons
    connect(ui->pushButtonGrowAngle1,       &QPushButton::clicked, this, &DialogCubicBezierLength::DeployAngle1TextEdit);
    connect(ui->pushButtonGrowC1Length,     &QPushButton::clicked, this, &DialogCubicBezierLength::DeployC1LengthTextEdit);
    connect(ui->pushButtonGrowAngle2,       &QPushButton::clicked, this, &DialogCubicBezierLength::DeployAngle2TextEdit);
    connect(ui->pushButtonGrowTargetLength, &QPushButton::clicked, this, &DialogCubicBezierLength::DeployTargetLengthTextEdit);

    // Point combo changes
    connect(ui->startPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogCubicBezierLength::pointNameChanged);
    connect(ui->endPoint_ComboBox,   &QComboBox::currentTextChanged, this, &DialogCubicBezierLength::pointNameChanged);

    vis = new VisToolCubicBezierLength(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezierLength::~DialogCubicBezierLength()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezierLength::GetPoint1() const
{
    return getCurrentObjectId(ui->startPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetPoint1(const quint32 &value)
{
    ChangeCurrentData(ui->startPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezierLength::GetPoint4() const
{
    return getCurrentObjectId(ui->endPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetPoint4(const quint32 &value)
{
    ChangeCurrentData(ui->endPoint_ComboBox, value);
    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual) visual->setObject4Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::GetAngle1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle1, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetAngle1(const QString &value)
{
    m_angle1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_angle1.length() > 80) DeployAngle1TextEdit();
    ui->plainTextEditAngle1->setPlainText(m_angle1);
    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual) visual->setAngle1(m_angle1);
    MoveCursorToEnd(ui->plainTextEditAngle1);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::GetAngle2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle2, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetAngle2(const QString &value)
{
    m_angle2 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_angle2.length() > 80) DeployAngle2TextEdit();
    ui->plainTextEditAngle2->setPlainText(m_angle2);
    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual) visual->setAngle2(m_angle2);
    MoveCursorToEnd(ui->plainTextEditAngle2);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::GetC1Length() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_c1Length, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetC1Length(const QString &value)
{
    m_c1Length = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_c1Length.length() > 80) DeployC1LengthTextEdit();
    ui->plainTextEditC1Length->setPlainText(m_c1Length);
    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual) visual->setC1Length(m_c1Length);
    MoveCursorToEnd(ui->plainTextEditC1Length);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::GetTargetLength() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_targetLength, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SetTargetLength(const QString &value)
{
    m_targetLength = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_targetLength.length() > 80) DeployTargetLengthTextEdit();
    ui->plainTextEditTargetLength->setPlainText(m_targetLength);
    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual) visual->setTargetLength(m_targetLength);
    MoveCursorToEnd(ui->plainTextEditTargetLength);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::getPenStyle() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

void DialogCubicBezierLength::setPenStyle(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::getLineWeight() const
{
    return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

void DialogCubicBezierLength::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezierLength::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

void DialogCubicBezierLength::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare)
        return;

    if (type != SceneObject::Point)
        return;

    // First click: start point P1
    if (getCurrentObjectId(ui->startPoint_ComboBox) == NULL_ID ||
        !vis->isVisible())
    {
        if (SetObject(id, ui->startPoint_ComboBox, tr("Select end point")))
        {
            vis->VisualMode(id);
            auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
            if (visual) visual->setObject1Id(id);
        }
        return;
    }

    // Second click: end point P4
    if (getCurrentObjectId(ui->startPoint_ComboBox) != id)
    {
        if (SetObject(id, ui->endPoint_ComboBox, QString()))
        {
            auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
            if (visual) visual->setObject4Id(id);
            prepare = true;
            this->setModal(true);
            this->show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::DeployAngle1TextEdit()
{
    DeployFormula(ui->plainTextEditAngle1, ui->pushButtonGrowAngle1, formulaBaseHeightAngle1);
}

void DialogCubicBezierLength::DeployC1LengthTextEdit()
{
    DeployFormula(ui->plainTextEditC1Length, ui->pushButtonGrowC1Length, formulaBaseHeightC1Length);
}

void DialogCubicBezierLength::DeployAngle2TextEdit()
{
    DeployFormula(ui->plainTextEditAngle2, ui->pushButtonGrowAngle2, formulaBaseHeightAngle2);
}

void DialogCubicBezierLength::DeployTargetLengthTextEdit()
{
    DeployFormula(ui->plainTextEditTargetLength, ui->pushButtonGrowTargetLength, formulaBaseHeightTargetLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::Angle1Changed()
{
    labelEditFormula = ui->labelEditAngle1;
    labelResultCalculation = ui->labelResultAngle1;
    ValFormulaChanged(flagAngle1, ui->plainTextEditAngle1, timerAngle1, degreeSymbol);
}

void DialogCubicBezierLength::C1LengthChanged()
{
    labelEditFormula = ui->labelEditC1Length;
    labelResultCalculation = ui->labelResultC1Length;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagC1Length, ui->plainTextEditC1Length, timerC1Length, postfix);
}

void DialogCubicBezierLength::Angle2Changed()
{
    labelEditFormula = ui->labelEditAngle2;
    labelResultCalculation = ui->labelResultAngle2;
    ValFormulaChanged(flagAngle2, ui->plainTextEditAngle2, timerAngle2, degreeSymbol);
}

void DialogCubicBezierLength::TargetLengthChanged()
{
    labelEditFormula = ui->labelEditTargetLength;
    labelResultCalculation = ui->labelResultTargetLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagTargetLength, ui->plainTextEditTargetLength, timerTargetLength, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::FXAngle1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit start handle angle"));
    dialog->SetFormula(GetAngle1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted) SetAngle1(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezierLength::FXC1Length()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit handle 1 length"));
    dialog->SetFormula(GetC1Length());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted) SetC1Length(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezierLength::FXAngle2()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit end handle angle"));
    dialog->SetFormula(GetAngle2());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted) SetAngle2(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezierLength::FXTargetLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit target curve length"));
    dialog->SetFormula(GetTargetLength());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted) SetTargetLength(dialog->GetFormula());
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::pointNameChanged()
{
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagAngle1 && flagC1Length && flagAngle2 && flagTargetLength);
    if (apply_Button != nullptr)
        apply_Button->setEnabled(ok_Button->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::ShowVisualization()
{
    AddVisualization<VisToolCubicBezierLength>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::SaveData()
{
    m_angle1 = ui->plainTextEditAngle1->toPlainText();
    m_angle1.replace("\n", " ");

    m_c1Length = ui->plainTextEditC1Length->toPlainText();
    m_c1Length.replace("\n", " ");

    m_angle2 = ui->plainTextEditAngle2->toPlainText();
    m_angle2.replace("\n", " ");

    m_targetLength = ui->plainTextEditTargetLength->toPlainText();
    m_targetLength.replace("\n", " ");

    auto visual = qobject_cast<VisToolCubicBezierLength *>(vis);
    if (visual)
    {
        visual->setObject1Id(GetPoint1());
        visual->setObject4Id(GetPoint4());
        visual->setAngle1(m_angle1);
        visual->setC1Length(m_c1Length);
        visual->setAngle2(m_angle2);
        visual->setTargetLength(m_targetLength);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditAngle1->blockSignals(true);
    ui->plainTextEditC1Length->blockSignals(true);
    ui->plainTextEditAngle2->blockSignals(true);
    ui->plainTextEditTargetLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezierLength::EvalAngle1()
{
    labelEditFormula = ui->labelEditAngle1;
    Eval(ui->plainTextEditAngle1->toPlainText(), flagAngle1, ui->labelResultAngle1, degreeSymbol, false);
}

void DialogCubicBezierLength::EvalAngle2()
{
    labelEditFormula = ui->labelEditAngle2;
    Eval(ui->plainTextEditAngle2->toPlainText(), flagAngle2, ui->labelResultAngle2, degreeSymbol, false);
}

void DialogCubicBezierLength::EvalC1Length()
{
    labelEditFormula = ui->labelEditC1Length;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal val = Eval(ui->plainTextEditC1Length->toPlainText(), flagC1Length, ui->labelResultC1Length, postfix);
    if (val <= 0.0)
    {
        flagC1Length = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultC1Length->setText(tr("Error"));
        ui->labelResultC1Length->setToolTip(tr("Handle length must be positive"));
        CheckState();
    }
}

void DialogCubicBezierLength::EvalTargetLength()
{
    labelEditFormula = ui->labelEditTargetLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal val = Eval(ui->plainTextEditTargetLength->toPlainText(), flagTargetLength,
                           ui->labelResultTargetLength, postfix);
    if (val <= 0.0)
    {
        flagTargetLength = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultTargetLength->setText(tr("Error"));
        ui->labelResultTargetLength->setToolTip(tr("Target length must be positive"));
        CheckState();
    }
}

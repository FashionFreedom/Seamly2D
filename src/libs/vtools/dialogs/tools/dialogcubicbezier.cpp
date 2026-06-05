/***************************************************************************
 *  @file   dialogcubicbezier.cpp
 *  @author Douglas S Caskey / Seamly2D contributors
 *
 *  @brief  Dialog for the parametric cubic Bézier tool (merged from
 *          DialogCubicBezierLength + two optional checkboxes).
 *
 *  Seamly2D is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 ***************************************************************************/

#include "dialogcubicbezier.h"

#include <QCheckBox>
#include <QDialog>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/visualization.h"
#include "../../visualization/path/vistoolcubicbezier.h"

#include "ui_dialogcubicbezier.h"

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezier::DialogCubicBezier(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCubicBezier)
    , flagAngle1(false)
    , flagC1Length(false)
    , flagAngle2(false)
    , flagC2Length(false)
    , flagTargetLength(false)
    , timerAngle1(nullptr)
    , timerC1Length(nullptr)
    , timerAngle2(nullptr)
    , timerC2Length(nullptr)
    , timerTargetLength(nullptr)
    , m_angle1(QString())
    , m_c1Length(QString())
    , m_angle2(QString())
    , m_c2Length(QString())
    , m_targetLength(QString())
    , formulaBaseHeightAngle1(0)
    , formulaBaseHeightC1Length(0)
    , formulaBaseHeightAngle2(0)
    , formulaBaseHeightC2Length(0)
    , formulaBaseHeightTargetLength(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/cubic_bezier.png"));
    setDialogPosition();

    plainTextEditFormula = ui->plainTextEditAngle1;
    formulaBaseHeightAngle1       = ui->plainTextEditAngle1->height();
    formulaBaseHeightC1Length     = ui->plainTextEditC1Length->height();
    formulaBaseHeightAngle2       = ui->plainTextEditAngle2->height();
    formulaBaseHeightC2Length     = ui->plainTextEditC2Length->height();
    formulaBaseHeightTargetLength = ui->plainTextEditTargetLength->height();

    ui->plainTextEditAngle1->installEventFilter(this);
    ui->plainTextEditC1Length->installEventFilter(this);
    ui->plainTextEditAngle2->installEventFilter(this);
    ui->plainTextEditC2Length->installEventFilter(this);
    ui->plainTextEditTargetLength->installEventFilter(this);

    timerAngle1       = new QTimer(this);
    timerC1Length     = new QTimer(this);
    timerAngle2       = new QTimer(this);
    timerC2Length     = new QTimer(this);
    timerTargetLength = new QTimer(this);
    connect(timerAngle1,       &QTimer::timeout, this, &DialogCubicBezier::EvalAngle1);
    connect(timerC1Length,     &QTimer::timeout, this, &DialogCubicBezier::EvalC1Length);
    connect(timerAngle2,       &QTimer::timeout, this, &DialogCubicBezier::EvalAngle2);
    connect(timerC2Length,     &QTimer::timeout, this, &DialogCubicBezier::EvalC2Length);
    connect(timerTargetLength, &QTimer::timeout, this, &DialogCubicBezier::EvalTargetLength);

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
    connect(ui->toolButtonExprAngle1,       &QToolButton::clicked, this, &DialogCubicBezier::FXAngle1);
    connect(ui->toolButtonExprC1Length,     &QToolButton::clicked, this, &DialogCubicBezier::FXC1Length);
    connect(ui->toolButtonExprAngle2,       &QToolButton::clicked, this, &DialogCubicBezier::FXAngle2);
    connect(ui->toolButtonExprC2Length,     &QToolButton::clicked, this, &DialogCubicBezier::FXC2Length);
    connect(ui->toolButtonExprTargetLength, &QToolButton::clicked, this, &DialogCubicBezier::FXTargetLength);

    // Text-changed signals
    connect(ui->plainTextEditAngle1,       &QPlainTextEdit::textChanged, this, &DialogCubicBezier::Angle1Changed);
    connect(ui->plainTextEditC1Length,     &QPlainTextEdit::textChanged, this, &DialogCubicBezier::C1LengthChanged);
    connect(ui->plainTextEditAngle2,       &QPlainTextEdit::textChanged, this, &DialogCubicBezier::Angle2Changed);
    connect(ui->plainTextEditC2Length,     &QPlainTextEdit::textChanged, this, &DialogCubicBezier::C2LengthChanged);
    connect(ui->plainTextEditTargetLength, &QPlainTextEdit::textChanged, this, &DialogCubicBezier::TargetLengthChanged);

    // Grow buttons
    connect(ui->pushButtonGrowAngle1,       &QPushButton::clicked, this, &DialogCubicBezier::DeployAngle1TextEdit);
    connect(ui->pushButtonGrowC1Length,     &QPushButton::clicked, this, &DialogCubicBezier::DeployC1LengthTextEdit);
    connect(ui->pushButtonGrowAngle2,       &QPushButton::clicked, this, &DialogCubicBezier::DeployAngle2TextEdit);
    connect(ui->pushButtonGrowC2Length,     &QPushButton::clicked, this, &DialogCubicBezier::DeployC2LengthTextEdit);
    connect(ui->pushButtonGrowTargetLength, &QPushButton::clicked, this, &DialogCubicBezier::DeployTargetLengthTextEdit);

    // Checkboxes
    connect(ui->checkBoxTargetLength, &QCheckBox::toggled, this, &DialogCubicBezier::OnTargetLengthToggled);
    connect(ui->checkBoxAutoSmooth,   &QCheckBox::toggled, this, &DialogCubicBezier::OnAutoSmoothToggled);

    // Point combo changes
    connect(ui->startPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogCubicBezier::pointNameChanged);
    connect(ui->endPoint_ComboBox,   &QComboBox::currentTextChanged, this, &DialogCubicBezier::pointNameChanged);

    vis = new VisToolCubicBezier(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezier::~DialogCubicBezier()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetPoint1() const
{
    return getCurrentObjectId(ui->startPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetPoint1(const quint32 &value)
{
    changeCurrentData(ui->startPoint_ComboBox, value);
    vis->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetPoint4() const
{
    return getCurrentObjectId(ui->endPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetPoint4(const quint32 &value)
{
    changeCurrentData(ui->endPoint_ComboBox, value);
    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
    if (visual) visual->setObject4Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetAngle1() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle1, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetAngle1(const QString &value)
{
    m_angle1 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_angle1.length() > 80) DeployAngle1TextEdit();
    ui->plainTextEditAngle1->setPlainText(m_angle1);
    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
    if (visual) visual->setAngle1(m_angle1);
    MoveCursorToEnd(ui->plainTextEditAngle1);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetAngle2() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_angle2, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetAngle2(const QString &value)
{
    m_angle2 = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_angle2.length() > 80) DeployAngle2TextEdit();
    ui->plainTextEditAngle2->setPlainText(m_angle2);
    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
    if (visual) visual->setAngle2(m_angle2);
    MoveCursorToEnd(ui->plainTextEditAngle2);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetC1Length() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_c1Length, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetC1Length(const QString &value)
{
    m_c1Length = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_c1Length.length() > 80) DeployC1LengthTextEdit();
    ui->plainTextEditC1Length->setPlainText(m_c1Length);
    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
    if (visual) visual->setC1Length(m_c1Length);
    MoveCursorToEnd(ui->plainTextEditC1Length);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetC2Length() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_c2Length, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetC2Length(const QString &value)
{
    m_c2Length = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_c2Length.length() > 80) DeployC2LengthTextEdit();
    ui->plainTextEditC2Length->setPlainText(m_c2Length);
    MoveCursorToEnd(ui->plainTextEditC2Length);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetTargetLength() const
{
    if (!ui->checkBoxTargetLength->isChecked())
        return QString();
    return qApp->translateVariables()->TryFormulaFromUser(m_targetLength, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetTargetLength(const QString &value)
{
    m_targetLength = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    if (m_targetLength.length() > 80) DeployTargetLengthTextEdit();
    ui->plainTextEditTargetLength->setPlainText(m_targetLength);
    MoveCursorToEnd(ui->plainTextEditTargetLength);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogCubicBezier::GetAutoSmooth() const
{
    return ui->checkBoxAutoSmooth->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SetAutoSmooth(bool value)
{
    ui->checkBoxAutoSmooth->setChecked(value);
    // OnAutoSmoothToggled is called automatically via the toggled signal
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::getPenStyle() const
{
    return getComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::setPenStyle(const QString &value)
{
    changeCurrentData(ui->lineType_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::getLineWeight() const
{
    return getComboBoxCurrentData(ui->lineWeight_ComboBox, DefaultLineWeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::setLineWeight(const QString &value)
{
    changeCurrentData(ui->lineWeight_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::getLineColor() const
{
    return getComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::setLineColor(const QString &value)
{
    changeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare)
        return;

    if (type != SceneObject::Point)
        return;

    switch (number)
    {
        case 0:
            if (SetObject(id, ui->startPoint_ComboBox, tr("Select end point")))
            {
                ++number;
                vis->VisualMode(id);
                auto visual = qobject_cast<VisToolCubicBezier *>(vis);
                if (visual) visual->setObject1Id(id);
            }
            break;
        case 1:
            if (getCurrentObjectId(ui->startPoint_ComboBox) != id)
            {
                if (SetObject(id, ui->endPoint_ComboBox, QString()))
                {
                    ++number;
                    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
                    if (visual) visual->setObject4Id(id);
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

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::DeployAngle1TextEdit()
{
    DeployFormula(ui->plainTextEditAngle1, ui->pushButtonGrowAngle1, formulaBaseHeightAngle1);
}

void DialogCubicBezier::DeployC1LengthTextEdit()
{
    DeployFormula(ui->plainTextEditC1Length, ui->pushButtonGrowC1Length, formulaBaseHeightC1Length);
}

void DialogCubicBezier::DeployAngle2TextEdit()
{
    DeployFormula(ui->plainTextEditAngle2, ui->pushButtonGrowAngle2, formulaBaseHeightAngle2);
}

void DialogCubicBezier::DeployC2LengthTextEdit()
{
    DeployFormula(ui->plainTextEditC2Length, ui->pushButtonGrowC2Length, formulaBaseHeightC2Length);
}

void DialogCubicBezier::DeployTargetLengthTextEdit()
{
    DeployFormula(ui->plainTextEditTargetLength, ui->pushButtonGrowTargetLength, formulaBaseHeightTargetLength);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::Angle1Changed()
{
    labelEditFormula = ui->labelEditAngle1;
    labelResultCalculation = ui->labelResultAngle1;
    formulaValueChanged(flagAngle1, ui->plainTextEditAngle1, timerAngle1, degreeSymbol);
}

void DialogCubicBezier::C1LengthChanged()
{
    labelEditFormula = ui->labelEditC1Length;
    labelResultCalculation = ui->labelResultC1Length;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    formulaValueChanged(flagC1Length, ui->plainTextEditC1Length, timerC1Length, postfix);
}

void DialogCubicBezier::Angle2Changed()
{
    labelEditFormula = ui->labelEditAngle2;
    labelResultCalculation = ui->labelResultAngle2;
    formulaValueChanged(flagAngle2, ui->plainTextEditAngle2, timerAngle2, degreeSymbol);
}

void DialogCubicBezier::C2LengthChanged()
{
    labelEditFormula = ui->labelEditC2Length;
    labelResultCalculation = ui->labelResultC2Length;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    formulaValueChanged(flagC2Length, ui->plainTextEditC2Length, timerC2Length, postfix);
}

void DialogCubicBezier::TargetLengthChanged()
{
    labelEditFormula = ui->labelEditTargetLength;
    labelResultCalculation = ui->labelResultTargetLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    formulaValueChanged(flagTargetLength, ui->plainTextEditTargetLength, timerTargetLength, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::FXAngle1()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit start handle angle"));
    dialog->SetFormula(GetAngle1());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted) SetAngle1(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezier::FXC1Length()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit handle 1 length"));
    dialog->SetFormula(GetC1Length());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted) SetC1Length(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezier::FXAngle2()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit end handle angle"));
    dialog->SetFormula(GetAngle2());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted) SetAngle2(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezier::FXC2Length()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit handle 2 length"));
    dialog->SetFormula(GetC2Length());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted) SetC2Length(dialog->GetFormula());
    delete dialog;
}

void DialogCubicBezier::FXTargetLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit target curve length"));
    dialog->SetFormula(GetTargetLength());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted) SetTargetLength(dialog->GetFormula());
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::OnTargetLengthToggled(bool checked)
{
    updateTargetLengthVisible(checked);
    updateC2LengthEnabled();
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::OnAutoSmoothToggled(bool checked)
{
    // Disable manual handle length fields when Hobby computes them
    ui->plainTextEditC1Length->setEnabled(!checked);
    ui->pushButtonGrowC1Length->setEnabled(!checked);
    ui->toolButtonExprC1Length->setEnabled(!checked);
    ui->labelEditC1Length->setEnabled(!checked);

    updateC2LengthEnabled();
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::pointNameChanged()
{
    // Update spline name display
    const quint32 p1Id = getCurrentObjectId(ui->startPoint_ComboBox);
    const quint32 p4Id = getCurrentObjectId(ui->endPoint_ComboBox);
    if (p1Id != NULL_ID && p4Id != NULL_ID && p1Id != p4Id)
    {
        const auto p1 = data->GeometricObject<VPointF>(p1Id);
        const auto p4 = data->GeometricObject<VPointF>(p4Id);
        ui->lineEditSplineName->setText(
            qApp->translateVariables()->VarToUser(
                QString("Spl_%1_%2").arg(p1->name(), p4->name())));
    }
    else
    {
        ui->lineEditSplineName->setText(tr("Invalid"));
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::CheckState()
{
    SCASSERT(ok_Button != nullptr)

    const bool twoPoints = (getCurrentObjectId(ui->startPoint_ComboBox) != NULL_ID) &&
                           (getCurrentObjectId(ui->endPoint_ComboBox)   != NULL_ID) &&
                           (getCurrentObjectId(ui->startPoint_ComboBox) !=
                            getCurrentObjectId(ui->endPoint_ComboBox));

    const bool autoSmooth  = ui->checkBoxAutoSmooth->isChecked();
    const bool useTarget   = ui->checkBoxTargetLength->isChecked();

    // Angles are always required
    const bool anglesOk = flagAngle1 && flagAngle2;

    bool lengthsOk = false;
    if (autoSmooth && useTarget)
        lengthsOk = flagTargetLength;          // State 4: only target needed
    else if (autoSmooth)
        lengthsOk = true;                      // State 3: no manual lengths
    else if (useTarget)
        lengthsOk = flagC1Length && flagTargetLength;  // State 2
    else
        lengthsOk = flagC1Length && flagC2Length;      // State 1

    ok_Button->setEnabled(twoPoints && anglesOk && lengthsOk);
    if (apply_Button != nullptr)
        apply_Button->setEnabled(ok_Button->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::ShowVisualization()
{
    AddVisualization<VisToolCubicBezier>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::SaveData()
{
    m_angle1 = ui->plainTextEditAngle1->toPlainText();
    m_angle1.replace("\n", " ");

    m_c1Length = ui->plainTextEditC1Length->toPlainText();
    m_c1Length.replace("\n", " ");

    m_angle2 = ui->plainTextEditAngle2->toPlainText();
    m_angle2.replace("\n", " ");

    m_c2Length = ui->plainTextEditC2Length->toPlainText();
    m_c2Length.replace("\n", " ");

    m_targetLength = ui->plainTextEditTargetLength->toPlainText();
    m_targetLength.replace("\n", " ");

    auto visual = qobject_cast<VisToolCubicBezier *>(vis);
    if (visual)
    {
        visual->setObject1Id(GetPoint1());
        visual->setObject4Id(GetPoint4());
        visual->setAngle1(m_angle1);
        visual->setC1Length(m_c1Length);
        visual->setAngle2(m_angle2);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditAngle1->blockSignals(true);
    ui->plainTextEditC1Length->blockSignals(true);
    ui->plainTextEditAngle2->blockSignals(true);
    ui->plainTextEditC2Length->blockSignals(true);
    ui->plainTextEditTargetLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::EvalAngle1()
{
    labelEditFormula = ui->labelEditAngle1;
    Eval(ui->plainTextEditAngle1->toPlainText(), flagAngle1, ui->labelResultAngle1, degreeSymbol, false);
}

void DialogCubicBezier::EvalAngle2()
{
    labelEditFormula = ui->labelEditAngle2;
    Eval(ui->plainTextEditAngle2->toPlainText(), flagAngle2, ui->labelResultAngle2, degreeSymbol, false);
}

void DialogCubicBezier::EvalC1Length()
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

void DialogCubicBezier::EvalC2Length()
{
    labelEditFormula = ui->labelEditC2Length;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal val = Eval(ui->plainTextEditC2Length->toPlainText(), flagC2Length, ui->labelResultC2Length, postfix);
    if (val <= 0.0)
    {
        flagC2Length = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultC2Length->setText(tr("Error"));
        ui->labelResultC2Length->setToolTip(tr("Handle length must be positive"));
        CheckState();
    }
}

void DialogCubicBezier::EvalTargetLength()
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

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::updateC2LengthEnabled()
{
    // c2 is only editable in State 1 (both checkboxes off)
    const bool c2Editable = !ui->checkBoxTargetLength->isChecked() &&
                            !ui->checkBoxAutoSmooth->isChecked();
    ui->plainTextEditC2Length->setEnabled(c2Editable);
    ui->pushButtonGrowC2Length->setEnabled(c2Editable);
    ui->toolButtonExprC2Length->setEnabled(c2Editable);
    ui->labelEditC2Length->setEnabled(c2Editable);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::updateTargetLengthVisible(bool visible)
{
    ui->labelEditTargetLength->setVisible(visible);
    ui->labelResultTargetLength->setVisible(visible);
    ui->toolButtonExprTargetLength->setVisible(visible);
    ui->pushButtonGrowTargetLength->setVisible(visible);
    ui->plainTextEditTargetLength->setVisible(visible);
}

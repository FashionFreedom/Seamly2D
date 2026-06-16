/***************************************************************************
 **  @file   dialogcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com> / Seamly2D contributors
 **
 **  Dialog for "Kurve Fixiert" — 4-point canvas selection plus two optional
 **  checkboxes (target length / Hobby auto-smooth).
 ***************************************************************************/

#include "dialogcubicbezier.h"

#include <QCheckBox>
#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <new>

#include "../../tools/vabstracttool.h"
#include "../../visualization/path/vistoolcubicbezier.h"
#include "../../visualization/visualization.h"
#include "../support/edit_formula_dialog.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "dialogtool.h"
#include "ui_dialogcubicbezier.h"

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezier::DialogCubicBezier(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCubicBezier)
    , flagTargetLength(false)
    , flagError(true)
    , timerTargetLength(nullptr)
    , m_targetLength(QString())
    , formulaBaseHeightTargetLength(0)
    , newDuplicate(-1)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/cubic_bezier.png"));
    setDialogPosition();

    initializeOkCancelApply(ui);

    fillComboBoxPoints(ui->comboBoxP1);
    fillComboBoxPoints(ui->comboBoxP2);
    fillComboBoxPoints(ui->comboBoxP3);
    fillComboBoxPoints(ui->comboBoxP4);

    formulaBaseHeightTargetLength = ui->plainTextEditTargetLength->height();
    ui->plainTextEditTargetLength->installEventFilter(this);

    timerTargetLength = new QTimer(this);
    connect(timerTargetLength, &QTimer::timeout, this, &DialogCubicBezier::EvalTargetLength);

    int index = ui->lineType_ComboBox->findData(LineTypeNone);
    if (index != -1) ui->lineType_ComboBox->removeItem(index);

    index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (index != -1) ui->lineColor_ComboBox->setCurrentIndex(index);

    index = ui->lineWeight_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineWeight());
    if (index != -1) ui->lineWeight_ComboBox->setCurrentIndex(index);

    index = ui->lineType_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineType());
    if (index != -1) ui->lineType_ComboBox->setCurrentIndex(index);

    // Checkboxes
    connect(ui->checkBoxTargetLength, &QCheckBox::toggled,
            this, &DialogCubicBezier::OnTargetLengthToggled);
    connect(ui->checkBoxAutoSmooth, &QCheckBox::toggled,
            this, &DialogCubicBezier::OnAutoSmoothToggled);

    // Target length formula
    connect(ui->plainTextEditTargetLength, &QPlainTextEdit::textChanged,
            this, &DialogCubicBezier::TargetLengthChanged);
    connect(ui->toolButtonExprTargetLength, &QToolButton::clicked,
            this, &DialogCubicBezier::FXTargetLength);
    connect(ui->pushButtonGrowTargetLength, &QPushButton::clicked,
            this, &DialogCubicBezier::DeployTargetLengthTextEdit);

    // Point selection
    connect(ui->comboBoxP1, &QComboBox::currentTextChanged,
            this, &DialogCubicBezier::PointNameChanged);
    connect(ui->comboBoxP2, &QComboBox::currentTextChanged,
            this, &DialogCubicBezier::PointNameChanged);
    connect(ui->comboBoxP3, &QComboBox::currentTextChanged,
            this, &DialogCubicBezier::PointNameChanged);
    connect(ui->comboBoxP4, &QComboBox::currentTextChanged,
            this, &DialogCubicBezier::PointNameChanged);

    DialogTool::CheckState();

    vis = new VisToolCubicBezier(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCubicBezier::~DialogCubicBezier()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetP1Id() const
{
    return getCurrentObjectId(ui->comboBoxP1);
}

void DialogCubicBezier::SetP1Id(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP1, value);
    auto path = qobject_cast<VisToolCubicBezier *>(vis);
    if (path) path->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetP2Id() const
{
    return getCurrentObjectId(ui->comboBoxP2);
}

void DialogCubicBezier::SetP2Id(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP2, value);
    auto path = qobject_cast<VisToolCubicBezier *>(vis);
    if (path) path->setObject2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetP3Id() const
{
    return getCurrentObjectId(ui->comboBoxP3);
}

void DialogCubicBezier::SetP3Id(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP3, value);
    auto path = qobject_cast<VisToolCubicBezier *>(vis);
    if (path) path->setObject3Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCubicBezier::GetP4Id() const
{
    return getCurrentObjectId(ui->comboBoxP4);
}

void DialogCubicBezier::SetP4Id(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP4, value);
    auto path = qobject_cast<VisToolCubicBezier *>(vis);
    if (path) path->setObject4Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::GetTargetLength() const
{
    if (!ui->checkBoxTargetLength->isChecked())
        return QString();
    return qApp->translateVariables()->TryFormulaFromUser(m_targetLength,
                                                          qApp->Settings()->getOsSeparator());
}

void DialogCubicBezier::SetTargetLength(const QString &value)
{
    m_targetLength = qApp->translateVariables()->FormulaToUser(value,
                                                               qApp->Settings()->getOsSeparator());
    if (!value.isEmpty())
    {
        ui->checkBoxTargetLength->setChecked(true);  // activates the field
        updateTargetLengthVisible(true);
    }
    if (m_targetLength.length() > 80)
        DeployTargetLengthTextEdit();
    ui->plainTextEditTargetLength->setPlainText(m_targetLength);
    MoveCursorToEnd(ui->plainTextEditTargetLength);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogCubicBezier::GetAutoSmooth() const
{
    return ui->checkBoxAutoSmooth->isChecked();
}

void DialogCubicBezier::SetAutoSmooth(bool value)
{
    ui->checkBoxAutoSmooth->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCubicBezier::getPenStyle() const
{
    return getComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

void DialogCubicBezier::setPenStyle(const QString &value)
{
    changeCurrentData(ui->lineType_ComboBox, value);
}

QString DialogCubicBezier::getLineWeight() const
{
    return getComboBoxCurrentData(ui->lineWeight_ComboBox, DefaultLineWeight);
}

void DialogCubicBezier::setLineWeight(const QString &value)
{
    changeCurrentData(ui->lineWeight_ComboBox, value);
}

QString DialogCubicBezier::getLineColor() const
{
    return getComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

void DialogCubicBezier::setLineColor(const QString &value)
{
    changeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)
    {
        if (type == SceneObject::Point)
        {
            auto *path = qobject_cast<VisToolCubicBezier *>(vis);
            SCASSERT(path != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxP1, tr("Select the second point of curve")))
                    {
                        ++number;
                        path->VisualMode(id);
                    }
                    break;
                case 1:
                    if (SetObject(id, ui->comboBoxP2, tr("Select the third point of curve")))
                    {
                        ++number;
                        path->setObject2Id(id);
                        path->RefreshGeometry();
                    }
                    break;
                case 2:
                    if (SetObject(id, ui->comboBoxP3, tr("Select the fourth point of curve")))
                    {
                        ++number;
                        path->setObject3Id(id);
                        path->RefreshGeometry();
                    }
                    break;
                case 3:
                    if (getCurrentObjectId(ui->comboBoxP1) != id)
                    {
                        if (SetObject(id, ui->comboBoxP4, QString()))
                        {
                            ++number;
                            path->setObject4Id(id);
                            path->RefreshGeometry();
                            prepare = true;
                            DialogAccepted();
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
void DialogCubicBezier::DeployTargetLengthTextEdit()
{
    DeployFormula(ui->plainTextEditTargetLength, ui->pushButtonGrowTargetLength,
                  formulaBaseHeightTargetLength);
}

void DialogCubicBezier::TargetLengthChanged()
{
    labelEditFormula        = ui->labelEditTargetLength;
    labelResultCalculation  = ui->labelResultTargetLength;
    const QString postfix   = UnitsToStr(qApp->patternUnit(), true);
    formulaValueChanged(flagTargetLength, ui->plainTextEditTargetLength,
                        timerTargetLength, postfix);
}

void DialogCubicBezier::FXTargetLength()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit target curve length"));
    dialog->SetFormula(GetTargetLength());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
        SetTargetLength(dialog->GetFormula());
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::OnTargetLengthToggled(bool checked)
{
    updateTargetLengthVisible(checked);
    CheckState();
}

void DialogCubicBezier::OnAutoSmoothToggled(bool /*checked*/)
{
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::PointNameChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxP1) == getCurrentObjectId(ui->comboBoxP4))
    {
        flagError = false;
        color = errorColor;
        ui->lineEditSplineName->setText(tr("Invalid spline"));
    }
    else
    {
        flagError = true;
        color = okColor;

        const quint32 p1 = getCurrentObjectId(ui->comboBoxP1);
        const quint32 p2 = getCurrentObjectId(ui->comboBoxP2);
        const quint32 p3 = getCurrentObjectId(ui->comboBoxP3);
        const quint32 p4 = getCurrentObjectId(ui->comboBoxP4);
        if (p1 != NULL_ID && p4 != NULL_ID)
        {
            try
            {
                const auto pt1 = data->GeometricObject<VPointF>(p1);
                const auto pt4 = data->GeometricObject<VPointF>(p4);
                const QString name = QString("Spl_%1_%2").arg(pt1->name(), pt4->name());
                ui->lineEditSplineName->setText(qApp->translateVariables()->VarToUser(name));
            }
            catch (...) {}
        }
        Q_UNUSED(p2) Q_UNUSED(p3)
    }
    ChangeColor(ui->labelName,       color);
    ChangeColor(ui->labelFirstPoint, color);
    ChangeColor(ui->labelForthPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::CheckState()
{
    SCASSERT(ok_Button != nullptr)

    const bool fourPointsOk = (getCurrentObjectId(ui->comboBoxP1) != NULL_ID) &&
                              (getCurrentObjectId(ui->comboBoxP2) != NULL_ID) &&
                              (getCurrentObjectId(ui->comboBoxP3) != NULL_ID) &&
                              (getCurrentObjectId(ui->comboBoxP4) != NULL_ID) &&
                              flagError;

    const bool targetOk = !ui->checkBoxTargetLength->isChecked() || flagTargetLength;

    ok_Button->setEnabled(fourPointsOk && targetOk);
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
    m_targetLength = ui->plainTextEditTargetLength->toPlainText();
    m_targetLength.replace("\n", " ");

    auto path = qobject_cast<VisToolCubicBezier *>(vis);
    SCASSERT(path != nullptr)

    path->setObject1Id(GetP1Id());
    path->setObject2Id(GetP2Id());
    path->setObject3Id(GetP3Id());
    path->setObject4Id(GetP4Id());
    path->SetMode(Mode::Show);
    path->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditTargetLength->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCubicBezier::EvalTargetLength()
{
    labelEditFormula = ui->labelEditTargetLength;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal val = Eval(ui->plainTextEditTargetLength->toPlainText(),
                           flagTargetLength, ui->labelResultTargetLength, postfix);
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
void DialogCubicBezier::updateTargetLengthVisible(bool visible)
{
    ui->labelEditTargetLength->setVisible(visible);
    ui->labelResultTargetLength->setVisible(visible);
    ui->toolButtonExprTargetLength->setVisible(visible);
    ui->pushButtonGrowTargetLength->setVisible(visible);
    ui->plainTextEditTargetLength->setVisible(visible);
}

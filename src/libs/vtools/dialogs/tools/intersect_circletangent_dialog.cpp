/**************************************************************************
 **
 **  @file   intersect_circletangent_dialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
 **
 **  @author Douglas S. Caskey
 **  @date   7.16.2022
 **
 **  @copyright
 **  Copyright (C) 2013-2022 Seamly2D project.
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published
 **  by the Free Software Foundation, either version 3 of the License,
 **  or (at your option) any later version.
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

#include "intersect_circletangent_dialog.h"
#include "ui_intersect_circletangent_dialog.h"

#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/intersect_circletangent_visual.h"

#include <limits.h>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPointer>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <Qt>

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentDialog::IntersectCircleTangentDialog(const VContainer *data, const quint32 &toolId,
                                                           QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::IntersectCircleTangentDialog)
    , flagCircleRadius(false)
    , timerCircleRadius(nullptr)
    , circleRadius()
    , formulaBaseHeightCircleRadius(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_from_circle_and_tangent.png"));

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;

    plainTextEditFormula = ui->plainTextEditRadius;
    this->formulaBaseHeightCircleRadius = ui->plainTextEditRadius->height();

    ui->plainTextEditRadius->installEventFilter(this);

    timerCircleRadius = new QTimer(this);
    connect(timerCircleRadius, &QTimer::timeout, this, &IntersectCircleTangentDialog::EvalCircleRadius);

    initializeOkCancelApply(ui);
    CheckState();

    FillComboBoxPoints(ui->comboBoxCircleCenter);
    FillComboBoxPoints(ui->comboBoxTangentPoint);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged,
            this, &IntersectCircleTangentDialog::NamePointChanged);

    connect(ui->comboBoxCircleCenter, &QComboBox::currentTextChanged,
            this, &IntersectCircleTangentDialog::PointChanged);

    connect(ui->toolButtonExprRadius, &QPushButton::clicked,
            this, &IntersectCircleTangentDialog::FXCircleRadius);

    connect(ui->plainTextEditRadius, &QPlainTextEdit::textChanged, this,
            &IntersectCircleTangentDialog::CircleRadiusChanged);

    connect(ui->pushButtonGrowRadius, &QPushButton::clicked, this,
            &IntersectCircleTangentDialog::DeployCircleRadiusTextEdit);

    vis = new IntersectCircleTangentVisual(data);
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCircleTangentDialog::~IntersectCircleTangentDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCircleTangentDialog::GetCircleCenterId() const
{
    return getCurrentObjectId(ui->comboBoxCircleCenter);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::SetCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircleCenter, value);

    IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setObject2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCircleTangentDialog::GetCircleRadius() const
{
    return qApp->TrVars()->TryFormulaFromUser(ui->plainTextEditRadius->toPlainText(),
                                              qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::SetCircleRadius(const QString &value)
{
    const QString formula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployCircleRadiusTextEdit();
    }
    ui->plainTextEditRadius->setPlainText(formula);

    IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setCRadius(formula);

    MoveCursorToEnd(ui->plainTextEditRadius);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCircleTangentDialog::GetTangentPointId() const
{
    return getCurrentObjectId(ui->comboBoxTangentPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::SetTangentPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxTangentPoint, value);

    IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint IntersectCircleTangentDialog::GetCrossCirclesPoint() const
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::setCirclesCrossPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
        SCASSERT(point != nullptr)
        point->setCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
            SCASSERT(point != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxTangentPoint, tr("Select a circle center")))
                    {
                        number++;
                        point->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxTangentPoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxCircleCenter, ""))
                        {
                            number = 0;
                            point->setObject2Id(id);
                            point->RefreshGeometry();
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
void IntersectCircleTangentDialog::PointChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxCircleCenter) == getCurrentObjectId(ui->comboBoxTangentPoint))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelCircleCenter, color);
    ChangeColor(ui->labelTangentPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::DeployCircleRadiusTextEdit()
{
    DeployFormula(ui->plainTextEditRadius, ui->pushButtonGrowRadius, formulaBaseHeightCircleRadius);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::CircleRadiusChanged()
{
    labelEditFormula = ui->labelEditRadius;
    labelResultCalculation = ui->labelResultCircleRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagCircleRadius, ui->plainTextEditRadius, timerCircleRadius, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::FXCircleRadius()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit radius"));
    dialog->SetFormula(GetCircleRadius());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::EvalCircleRadius()
{
    labelEditFormula = ui->labelEditRadius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditRadius->toPlainText(), flagCircleRadius,
                              ui->labelResultCircleRadius, postfix);

    if (radius < 0)
    {
        flagCircleRadius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultCircleRadius->setText(tr("Error"));
        ui->labelResultCircleRadius->setToolTip(tr("Radius can't be negative"));

        IntersectCircleTangentDialog::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::ShowVisualization()
{
    AddVisualization<IntersectCircleTangentVisual>();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    QString radius = ui->plainTextEditRadius->toPlainText();
    radius.replace("\n", " ");

    IntersectCircleTangentVisual *point = qobject_cast<IntersectCircleTangentVisual *>(vis);
    SCASSERT(point != nullptr)

    point->setObject1Id(GetTangentPointId());
    point->setObject2Id(GetCircleCenterId());
    point->setCRadius(radius);
    point->setCrossPoint(GetCrossCirclesPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditRadius->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCircleTangentDialog::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagFormula && flagName && flagError && flagCircleRadius);
    // In case dialog hasn't apply button
    if (apply_Button != nullptr)
    {
        apply_Button->setEnabled(ok_Button->isEnabled());
    }
}

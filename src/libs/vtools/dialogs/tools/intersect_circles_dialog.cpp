//-----------------------------------------------------------------------------
//  @file   intersect_circles_tool.cpp
//  @author Douglas S Caskey
//  @date   16 Jul, 2022
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
//  @file   intersect_circles_tool.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   29 May, 2015
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

#include "intersect_circles_dialog.h"
#include "ui_intersect_circles_dialog.h"

#include "../ifc/xml/vdomdocument.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/intersect_circles_visual.h"

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
IntersectCirclesDialog::IntersectCirclesDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::IntersectCirclesDialog)
    , flagCircle1Radius(false)
    , flagCircle2Radius(false)
    , timerCircle1Radius(nullptr)
    , timerCircle2Radius(nullptr)
    , circle1Radius()
    , circle2Radius()
    , formulaBaseHeightCircle1Radius(0)
    , formulaBaseHeightCircle2Radius(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_of_intersection_circles.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;

    plainTextEditFormula = ui->plainTextEditCircle1Radius;
    this->formulaBaseHeightCircle1Radius = ui->plainTextEditCircle1Radius->height();
    this->formulaBaseHeightCircle2Radius = ui->plainTextEditCircle2Radius->height();

    ui->plainTextEditCircle1Radius->installEventFilter(this);
    ui->plainTextEditCircle2Radius->installEventFilter(this);

    timerCircle1Radius = new QTimer(this);
    connect(timerCircle1Radius, &QTimer::timeout, this, &IntersectCirclesDialog::EvalCircle1Radius);

    timerCircle2Radius = new QTimer(this);
    connect(timerCircle2Radius, &QTimer::timeout, this, &IntersectCirclesDialog::EvalCircle2Radius);

    initializeOkCancelApply(ui);
    CheckState();

    fillComboBoxPoints(ui->comboBoxCircle1Center);
    fillComboBoxPoints(ui->comboBoxCircle2Center);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged,
            this, &IntersectCirclesDialog::NamePointChanged);

    connect(ui->comboBoxCircle1Center, &QComboBox::currentTextChanged,
            this, &IntersectCirclesDialog::PointChanged);

    connect(ui->comboBoxCircle2Center, &QComboBox::currentTextChanged,
            this, &IntersectCirclesDialog::PointChanged);

    connect(ui->toolButtonExprCircle1Radius, &QPushButton::clicked, this,
            &IntersectCirclesDialog::FXCircle1Radius);

    connect(ui->toolButtonExprCircle2Radius, &QPushButton::clicked, this,
            &IntersectCirclesDialog::FXCircle2Radius);

    connect(ui->plainTextEditCircle1Radius, &QPlainTextEdit::textChanged, this,
            &IntersectCirclesDialog::Circle1RadiusChanged);

    connect(ui->plainTextEditCircle2Radius, &QPlainTextEdit::textChanged, this,
            &IntersectCirclesDialog::Circle2RadiusChanged);

    vis = new IntersectCirclesVisual(data);
}

//---------------------------------------------------------------------------------------------------------------------
IntersectCirclesDialog::~IntersectCirclesDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCirclesDialog::GetFirstCircleCenterId() const
{
    return getCurrentObjectId(ui->comboBoxCircle1Center);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SetFirstCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircle1Center, value);

    IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 IntersectCirclesDialog::GetSecondCircleCenterId() const
{
    return getCurrentObjectId(ui->comboBoxCircle2Center);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SetSecondCircleCenterId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxCircle2Center, value);

    IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setObject2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCirclesDialog::GetFirstCircleRadius() const
{
    return qApp->translateVariables()->TryFormulaFromUser(ui->plainTextEditCircle1Radius->toPlainText(),
                                              qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SetFirstCircleRadius(const QString &value)
{
    const QString formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    ui->plainTextEditCircle1Radius->setPlainText(formula);

    IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setC1Radius(formula);

    MoveCursorToEnd(ui->plainTextEditCircle1Radius);
}

//---------------------------------------------------------------------------------------------------------------------
QString IntersectCirclesDialog::GetSecondCircleRadius() const
{
    return qApp->translateVariables()->TryFormulaFromUser(ui->plainTextEditCircle2Radius->toPlainText(),
                                              qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SetSecondCircleRadius(const QString &value)
{
    const QString formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    ui->plainTextEditCircle2Radius->setPlainText(formula);

    IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setC2Radius(formula);

    MoveCursorToEnd(ui->plainTextEditCircle2Radius);
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint IntersectCirclesDialog::GetCrossCirclesPoint() const
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::setCirclesCrossPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
        SCASSERT(point != nullptr)
        point->setCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
            SCASSERT(point != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxCircle1Center, tr("Select second circle center")))
                    {
                        number++;
                        point->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxCircle1Center) != id)
                    {
                        if (SetObject(id, ui->comboBoxCircle2Center, ""))
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
void IntersectCirclesDialog::PointChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxCircle1Center) == getCurrentObjectId(ui->comboBoxCircle2Center))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelCircle1Center, color);
    ChangeColor(ui->labelCircle1Center, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::Circle1RadiusChanged()
{
    labelEditFormula = ui->labelEditCircle1Radius;
    labelResultCalculation = ui->labelResultCircle1Radius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagCircle1Radius, ui->plainTextEditCircle1Radius, timerCircle1Radius, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::Circle2RadiusChanged()
{
    labelEditFormula = ui->labelEditCircle2Radius;
    labelResultCalculation = ui->labelResultCircle2Radius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagCircle2Radius, ui->plainTextEditCircle2Radius, timerCircle2Radius, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::FXCircle1Radius()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit first circle radius"));
    dialog->SetFormula(GetFirstCircleRadius());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFirstCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::FXCircle2Radius()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit second circle radius"));
    dialog->SetFormula(GetSecondCircleRadius());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetSecondCircleRadius(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::EvalCircle1Radius()
{
    labelEditFormula = ui->labelEditCircle1Radius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditCircle1Radius->toPlainText(), flagCircle1Radius,
                              ui->labelResultCircle1Radius, postfix);

    if (radius < 0)
    {
        flagCircle2Radius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultCircle1Radius->setText(tr("Error"));
        ui->labelResultCircle1Radius->setToolTip(tr("Radius can't be negative"));

        IntersectCirclesDialog::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::EvalCircle2Radius()
{
    labelEditFormula = ui->labelEditCircle2Radius;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const qreal radius = Eval(ui->plainTextEditCircle2Radius->toPlainText(), flagCircle2Radius,
                              ui->labelResultCircle2Radius, postfix);

    if (radius < 0)
    {
        flagCircle2Radius = false;
        ChangeColor(labelEditFormula, Qt::red);
        ui->labelResultCircle2Radius->setText(tr("Error"));
        ui->labelResultCircle2Radius->setToolTip(tr("Radius can't be negative"));

        IntersectCirclesDialog::CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::ShowVisualization()
{
    AddVisualization<IntersectCirclesVisual>();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    QString c1Radius = ui->plainTextEditCircle2Radius->toPlainText();
    c1Radius.replace("\n", " ");

    QString c2Radius = ui->plainTextEditCircle2Radius->toPlainText();
    c2Radius.replace("\n", " ");

    IntersectCirclesVisual *point = qobject_cast<IntersectCirclesVisual *>(vis);
    SCASSERT(point != nullptr)

    point->setObject1Id(GetFirstCircleCenterId());
    point->setObject2Id(GetSecondCircleCenterId());
    point->setC1Radius(c1Radius);
    point->setC2Radius(c2Radius);
    point->setCrossPoint(GetCrossCirclesPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditCircle1Radius->blockSignals(true);
    ui->plainTextEditCircle2Radius->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void IntersectCirclesDialog::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagFormula && flagName && flagError && flagCircle1Radius && flagCircle2Radius);
    // In case dialog does not have an apply button
    if (apply_Button != nullptr)
    {
        apply_Button->setEnabled(ok_Button->isEnabled());
    }
}

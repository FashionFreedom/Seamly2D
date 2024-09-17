//-----------------------------------------------------------------------------
//  @file   dialogcurveintersectaxis.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
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
//  @file   dialogcurveintersectaxis.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   21 Nov, 2014
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

#include "dialogcurveintersectaxis.h"

#include <QDialog>
#include <QLineEdit>
#include <QLineF>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QSharedPointer>
#include <QTimer>
#include <QToolButton>
#include <new>

#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../../visualization/line/vistoolcurveintersectaxis.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialogcurveintersectaxis.h"

//---------------------------------------------------------------------------------------------------------------------
DialogCurveIntersectAxis::DialogCurveIntersectAxis(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogCurveIntersectAxis)
    , formulaAngle()
    , formulaBaseHeightAngle(0)
    , m_firstRelease(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/curve_intersect_axis.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->pointName_LineEdit->setClearButtonEnabled(true);

    initializeFormulaUi(ui);
    ui->pointName_LineEdit->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->pointName_Label;
    this->formulaBaseHeightAngle = plainTextEditFormula->height();
    plainTextEditFormula->installEventFilter(this);

    initializeOkCancelApply(ui);
    flagFormula = false;
    DialogTool::CheckState();

    fillComboBoxPoints(ui->axisPoint_ComboBox);
    FillComboBoxCurves(ui->curve_ComboBox);

    int index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
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

    connect(ui->exprAngle_ToolButton, &QPushButton::clicked, this, &DialogCurveIntersectAxis::FXAngle);
    connect(ui->pointName_LineEdit, &QLineEdit::textChanged, this, &DialogCurveIntersectAxis::NamePointChanged);
    connect(plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogCurveIntersectAxis::AngleTextChanged);
    connect(ui->growLengthAngle_PushButton, &QPushButton::clicked, this, &DialogCurveIntersectAxis::DeployAngleTextEdit);
    connect(timerFormula, &QTimer::timeout, this, &DialogCurveIntersectAxis::EvalAngle);

    vis = new VisToolCurveIntersectAxis(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogCurveIntersectAxis::~DialogCurveIntersectAxis()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetPointName(const QString &value)
{
    pointName = value;
    ui->pointName_LineEdit->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCurveIntersectAxis::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogCurveIntersectAxis::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogCurveIntersectAxis::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogCurveIntersectAxis::GetAngle() const
{
    return qApp->translateVariables()->TryFormulaFromUser(formulaAngle, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetAngle(const QString &value)
{
    formulaAngle = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formulaAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    plainTextEditFormula->setPlainText(formulaAngle);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetAngle(formulaAngle);

    MoveCursorToEnd(plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCurveIntersectAxis::GetBasePointId() const
{
    return getCurrentObjectId(ui->axisPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SetBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->axisPoint_ComboBox, value);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setAxisPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogCurveIntersectAxis::getCurveId() const
{
    return getCurrentObjectId(ui->curve_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::setCurveId(const quint32 &value)
{
    setCurrentCurveId(ui->curve_ComboBox, value);

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineColor get the color of line
 * @param value type
 */
 QString DialogCurveIntersectAxis::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
/* @brief setLineColor set color of the line
 * @param value type
 */
void DialogCurveIntersectAxis::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ShowDialog(bool click)
{
    if (prepare)
    {
        if (click)
        {
            // The check need to ignore first release of mouse button.
            // User can select point by clicking on a label.
            if (not m_firstRelease)
            {
                m_firstRelease = true;
                return;
            }

            /*We will ignore click if poinet is in point circle*/
            VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
            SCASSERT(scene != nullptr)
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(GetBasePointId());
            QLineF line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            //Radius of point circle, but little bigger. Need handle with hover sizes.
            if (line.length() <= defPointRadiusPixel*1.5)
            {
                return;
            }
        }

        VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(line != nullptr)

        this->SetAngle(line->Angle());//Show in dialog angle what user choose
        emit ToolTip("");

        DialogAccepted();// Just set default values and don't show dialog
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(line != nullptr)

        switch (number)
        {
            case (0):
                if (type == SceneObject::Spline
                        || type == SceneObject::Arc
                        || type == SceneObject::ElArc
                        || type == SceneObject::SplinePath)
                {
                    if (SetObject(id, ui->curve_ComboBox, tr("Select axis point")))
                    {
                        number++;
                        line->VisualMode(id);
                        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(line, &VisToolCurveIntersectAxis::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                    }
                }
                break;
            case (1):
                if (type == SceneObject::Point)
                {
                    if (SetObject(id, ui->axisPoint_ComboBox, ""))
                    {
                        line->setAxisPointId(id);
                        line->RefreshGeometry();
                        prepare = true;
                    }
                }
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::EvalAngle()
{
    Eval(ui->plainTextEditFormula->toPlainText(), flagError, ui->labelResultCalculation, degreeSymbol, false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::AngleTextChanged()
{
    ValFormulaChanged(flagError, plainTextEditFormula, timerFormula, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::DeployAngleTextEdit()
{
    DeployFormula(plainTextEditFormula, ui->growLengthAngle_PushButton, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::FXAngle()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit angle"));
    dialog->SetFormula(GetAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::ShowVisualization()
{
    AddVisualization<VisToolCurveIntersectAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::SaveData()
{
    pointName = ui->pointName_LineEdit->text();

    formulaAngle = plainTextEditFormula->toPlainText();
    formulaAngle.replace("\n", " ");

    VisToolCurveIntersectAxis *line = qobject_cast<VisToolCurveIntersectAxis *>(vis);
    SCASSERT(line != nullptr)

    line->setObject1Id(getCurveId());
    line->setAxisPointId(GetBasePointId());
    line->SetAngle(formulaAngle);
    line->setLineStyle(lineTypeToPenStyle(getLineType()));
    line->setLineWeight(getLineWeight());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogCurveIntersectAxis::closeEvent(QCloseEvent *event)
{
    plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

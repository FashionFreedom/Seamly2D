/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialoglineintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
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

#include "dialoglineintersectaxis.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QSet>
#include <QSharedPointer>
#include <QTimer>
#include <QToolButton>
#include <new>

#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../../tools/vabstracttool.h"
#include "../../visualization/line/vistoollineintersectaxis.h"
#include "../../visualization/visualization.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../support/edit_formula_dialog.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "ui_dialoglineintersectaxis.h"

//---------------------------------------------------------------------------------------------------------------------
DialogLineIntersectAxis::DialogLineIntersectAxis(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogLineIntersectAxis),
      formulaAngle(),
      formulaBaseHeightAngle(0),
      m_firstRelease(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/line_intersect_axis.png"));

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    initializeFormulaUi(ui);
    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;
    this->formulaBaseHeightAngle = ui->plainTextEditFormula->height();
    ui->plainTextEditFormula->installEventFilter(this);

    initializeOkCancelApply(ui);
    flagFormula = false;
    DialogTool::CheckState();

    FillComboBoxPoints(ui->comboBoxAxisPoint);
    FillComboBoxPoints(ui->comboBoxFirstLinePoint);
    FillComboBoxPoints(ui->comboBoxSecondLinePoint);

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

    connect(ui->toolButtonExprAngle,  &QPushButton::clicked,        this, &DialogLineIntersectAxis::FXAngle);
    connect(ui->lineEditNamePoint,    &QLineEdit::textChanged,      this, &DialogLineIntersectAxis::NamePointChanged);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged, this, &DialogLineIntersectAxis::AngleTextChanged);
    connect(ui->pushButtonGrowLengthAngle, &QPushButton::clicked, this, &DialogLineIntersectAxis::DeployAngleTextEdit);
    connect(timerFormula, &QTimer::timeout, this, &DialogLineIntersectAxis::EvalAngle);
    connect(ui->comboBoxFirstLinePoint, &QComboBox::currentTextChanged,
            this, &DialogLineIntersectAxis::PointNameChanged);
    connect(ui->comboBoxSecondLinePoint, &QComboBox::currentTextChanged,
            this, &DialogLineIntersectAxis::PointNameChanged);
    connect(ui->comboBoxAxisPoint, &QComboBox::currentTextChanged,
            this, &DialogLineIntersectAxis::PointNameChanged);

    vis = new VisToolLineIntersectAxis(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogLineIntersectAxis::~DialogLineIntersectAxis()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLineIntersectAxis::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogLineIntersectAxis::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogLineIntersectAxis::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLineIntersectAxis::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogLineIntersectAxis::GetAngle() const
{
    return qApp->translateVariables()->TryFormulaFromUser(formulaAngle, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetAngle(const QString &value)
{
    formulaAngle = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    // increase height if needed. TODO : see if I can get the max number of caracters in one line
    // of this PlainTextEdit to change 80 to this value
    if (formulaAngle.length() > 80)
    {
        this->DeployAngleTextEdit();
    }
    ui->plainTextEditFormula->setPlainText(formulaAngle);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->SetAngle(formulaAngle);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogLineIntersectAxis::GetBasePointId() const
{
    return getCurrentObjectId(ui->comboBoxAxisPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxAxisPoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setAxisPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogLineIntersectAxis::GetFirstPointId() const
{
    return getCurrentObjectId(ui->comboBoxFirstLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetFirstPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxFirstLinePoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogLineIntersectAxis::GetSecondPointId() const
{
    return getCurrentObjectId(ui->comboBoxSecondLinePoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SetSecondPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxSecondLinePoint, value);

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)
    line->setPoint2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::ShowDialog(bool click)
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

        VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
        SCASSERT(line != nullptr)

        this->SetAngle(line->Angle());//Show in dialog angle what user choose
        emit ToolTip("");

        DialogAccepted();// Just set default values and don't show dialog
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
            SCASSERT(line != nullptr)

            switch (number)
            {
                case (0):
                    if (SetObject(id, ui->comboBoxFirstLinePoint, tr("Select second point of line")))
                    {
                        number++;
                        line->VisualMode(id);
                        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
                        SCASSERT(window != nullptr)
                        connect(line, &VisToolLineIntersectAxis::ToolTip, window, &VAbstractMainWindow::ShowToolTip);
                    }
                    break;
                case (1):
                    if (getCurrentObjectId(ui->comboBoxFirstLinePoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxSecondLinePoint, tr("Select axis point")))
                        {
                            number++;
                            line->setPoint2Id(id);
                            line->RefreshGeometry();
                        }
                    }
                    break;
                case (2):
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxFirstLinePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxSecondLinePoint));
                    set.insert(id);

                    if (set.size() == 3)
                    {
                        if (SetObject(id, ui->comboBoxAxisPoint, ""))
                        {
                            line->setAxisPointId(id);
                            line->RefreshGeometry();
                            prepare = true;
                        }
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
void DialogLineIntersectAxis::EvalAngle()
{
    Eval(ui->plainTextEditFormula->toPlainText(), flagError, ui->labelResultCalculation, degreeSymbol, false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::AngleTextChanged()
{
    ValFormulaChanged(flagError, ui->plainTextEditFormula, timerFormula, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::DeployAngleTextEdit()
{
    DeployFormula(ui->plainTextEditFormula, ui->pushButtonGrowLengthAngle, formulaBaseHeightAngle);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::PointNameChanged()
{
    QSet<quint32> set;
    set.insert(getCurrentObjectId(ui->comboBoxFirstLinePoint));
    set.insert(getCurrentObjectId(ui->comboBoxSecondLinePoint));
    set.insert(getCurrentObjectId(ui->comboBoxAxisPoint));

    QColor color = okColor;
    if (set.size() != 3)
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelFirstLinePoint, color);
    ChangeColor(ui->labelSecondLinePoint, color);
    ChangeColor(ui->labelAxisPoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::FXAngle()
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
void DialogLineIntersectAxis::ShowVisualization()
{
    AddVisualization<VisToolLineIntersectAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    formulaAngle = ui->plainTextEditFormula->toPlainText();
    formulaAngle.replace("\n", " ");

    VisToolLineIntersectAxis *line = qobject_cast<VisToolLineIntersectAxis *>(vis);
    SCASSERT(line != nullptr)

    line->setObject1Id(GetFirstPointId());
    line->setPoint2Id(GetSecondPointId());
    line->setAxisPointId(GetBasePointId());
    line->SetAngle(formulaAngle);
    line->setLineStyle(lineTypeToPenStyle(getLineType()));
    line->setLineWeight(getLineWeight());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogLineIntersectAxis::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

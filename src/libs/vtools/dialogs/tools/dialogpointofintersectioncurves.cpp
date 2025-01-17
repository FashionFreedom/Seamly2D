/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   dialogpointofintersectioncurves.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 1, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "dialogpointofintersectioncurves.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>

#include "../../visualization/path/vistoolpointofintersectioncurves.h"
#include "../../visualization/visualization.h"
#include "../vmisc/vabstractapplication.h"
#include "dialogtool.h"
#include "ui_dialogpointofintersectioncurves.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCurves::DialogPointOfIntersectionCurves(const VContainer *data, const quint32 &toolId,
                                                                 QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogPointOfIntersectionCurves)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/intersection_curves.png"));

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;

    initializeOkCancelApply(ui);
    CheckState();

    FillComboBoxCurves(ui->comboBoxCurve1);
    FillComboBoxCurves(ui->comboBoxCurve2);
    FillComboBoxVCrossCurvesPoint(ui->verticalTake_ComboBox);
    FillComboBoxHCrossCurvesPoint(ui->horizontalTake_ComboBox);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, &DialogPointOfIntersectionCurves::NamePointChanged);
    connect(ui->comboBoxCurve1, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);
    connect(ui->comboBoxCurve2, &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionCurves::CurveChanged);

    vis = new VisToolPointOfIntersectionCurves(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionCurves::~DialogPointOfIntersectionCurves()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointOfIntersectionCurves::GetFirstCurveId() const
{
    return getCurrentObjectId(ui->comboBoxCurve1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetFirstCurveId(const quint32 &value)
{
    setCurrentCurveId(ui->comboBoxCurve1, value);

    auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)
    point->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointOfIntersectionCurves::GetSecondCurveId() const
{
    return getCurrentObjectId(ui->comboBoxCurve2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetSecondCurveId(const quint32 &value)
{
    setCurrentCurveId(ui->comboBoxCurve2, value);

    auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)
    point->setObject2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
VCrossCurvesPoint DialogPointOfIntersectionCurves::GetVCrossPoint() const
{
    return getCurrentCrossPoint<VCrossCurvesPoint>(ui->verticalTake_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetVCrossPoint(const VCrossCurvesPoint &vP)
{
    auto index = ui->verticalTake_ComboBox->findData(static_cast<int>(vP));
    if (index != -1)
    {
        ui->verticalTake_ComboBox->setCurrentIndex(index);

        auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(point != nullptr)
        point->setVCrossPoint(vP);
    }
}

//---------------------------------------------------------------------------------------------------------------------
HCrossCurvesPoint DialogPointOfIntersectionCurves::GetHCrossPoint() const
{
    return getCurrentCrossPoint<HCrossCurvesPoint>(ui->horizontalTake_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SetHCrossPoint(const HCrossCurvesPoint &hP)
{
    auto index = ui->horizontalTake_ComboBox->findData(static_cast<int>(hP));
    if (index != -1)
    {
        ui->horizontalTake_ComboBox->setCurrentIndex(index);

        auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(point != nullptr)
        point->setHCrossPoint(hP);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Spline
                || type == SceneObject::Arc
                || type == SceneObject::ElArc
                || type == SceneObject::SplinePath)
        {
            auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
            SCASSERT(point != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->comboBoxCurve1, tr("Select second curve")))
                    {
                        number++;
                        point->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxCurve1) != id)
                    {
                        if (SetObject(id, ui->comboBoxCurve2, ""))
                        {
                            number = 0;
                            point->setObject2Id(id);
                            point->RefreshGeometry();
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
void DialogPointOfIntersectionCurves::ShowVisualization()
{
    AddVisualization<VisToolPointOfIntersectionCurves>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    auto point = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
    SCASSERT(point != nullptr)

    point->setObject1Id(GetFirstCurveId());
    point->setObject2Id(GetSecondCurveId());
    point->setVCrossPoint(GetVCrossPoint());
    point->setHCrossPoint(GetHCrossPoint());
    point->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagName && flagError);
    // In case dialog hasn't apply button
    if (apply_Button != nullptr)
    {
        apply_Button->setEnabled(ok_Button->isEnabled());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionCurves::CurveChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxCurve1) == getCurrentObjectId(ui->comboBoxCurve2))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelCurve1, color);
    ChangeColor(ui->labelCurve2, color);
    CheckState();
}

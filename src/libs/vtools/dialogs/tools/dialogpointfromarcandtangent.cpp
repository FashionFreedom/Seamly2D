//-----------------------------------------------------------------------------
//  @file   dialogpointfromarcandtangent.cpp
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
//  @file   dialogpointfromarcandtangent.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   5 Jun, 2015
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

#include "dialogpointfromarcandtangent.h"

#include <QComboBox>
#include <QLineEdit>
#include <QPointer>

#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolpointfromarcandtangent.h"
#include "../vmisc/vabstractapplication.h"
#include "dialogs/tools/dialogtool.h"
#include "ui_dialogpointfromarcandtangent.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointFromArcAndTangent::DialogPointFromArcAndTangent(const VContainer *data, const quint32 &toolId,
                                                           QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogPointFromArcAndTangent)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_from_arc_and_tangent.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;

    initializeOkCancelApply(ui);
    DialogTool::CheckState();

    fillComboBoxPoints(ui->comboBoxTangentPoint);
    FillComboBoxArcs(ui->comboBoxArc);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged, this, &DialogPointFromArcAndTangent::NamePointChanged);

    vis = new VisToolPointFromArcAndTangent(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointFromArcAndTangent::~DialogPointFromArcAndTangent()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointFromArcAndTangent::GetArcId() const
{
    return getCurrentObjectId(ui->comboBoxArc);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::SetArcId(const quint32 &value)
{
    setCurrentArcId(ui->comboBoxArc, value);

    VisToolPointFromArcAndTangent *point = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
    SCASSERT(point != nullptr)
    point->setArcId(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointFromArcAndTangent::GetTangentPointId() const
{
    return getCurrentObjectId(ui->comboBoxTangentPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::SetTangentPointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxTangentPoint, value);

    VisToolPointFromArcAndTangent *point = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
    SCASSERT(point != nullptr)
    point->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint DialogPointFromArcAndTangent::GetCrossCirclesPoint() const
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::setCirclesCrossPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        VisToolPointFromArcAndTangent *point = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
        SCASSERT(point != nullptr)
        point->setCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point || type == SceneObject::Arc)
        {
            VisToolPointFromArcAndTangent *point = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
            SCASSERT(point != nullptr)

            switch (number)
            {
                case 0:
                    if (type == SceneObject::Point)
                    {
                        if (SetObject(id, ui->comboBoxTangentPoint, tr("Select an arc")))
                        {
                            number++;
                            point->VisualMode(id);
                        }
                    }
                    break;
                case 1:
                    if (type == SceneObject::Arc)
                    {
                        if (SetObject(id, ui->comboBoxArc, ""))
                        {
                            number = 0;
                            point->setArcId(id);
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
void DialogPointFromArcAndTangent::ShowVisualization()
{
    AddVisualization<VisToolPointFromArcAndTangent>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointFromArcAndTangent::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    VisToolPointFromArcAndTangent *point = qobject_cast<VisToolPointFromArcAndTangent *>(vis);
    SCASSERT(point != nullptr)

    point->setObject1Id(GetTangentPointId());
    point->setArcId(GetArcId());
    point->setCrossPoint(GetCrossCirclesPoint());
    point->RefreshGeometry();
}

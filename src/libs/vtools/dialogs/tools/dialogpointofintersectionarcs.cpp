//-----------------------------------------------------------------------------
//  @file   dialogpointofintersectionarcs.cpp
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
//  @file   dialogpointofintersectionarcs.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   25 May, 2015
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

#include "dialogpointofintersectionarcs.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>

#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolpointofintersectionarcs.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogs/tools/dialogtool.h"
#include "ui_dialogpointofintersectionarcs.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionArcs::DialogPointOfIntersectionArcs(const VContainer *data, const quint32 &toolId,
                                                             QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogPointOfIntersectionArcs)
    , firstArcId(NULL_ID)
    , secondArcId(NULL_ID)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_of_intersection_arcs.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;

    initializeOkCancelApply(ui);
    DialogTool::CheckState();

    fillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged,  this, &DialogPointOfIntersectionArcs::NamePointChanged);

    vis = new VisToolPointOfIntersectionArcs(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionArcs::~DialogPointOfIntersectionArcs()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointOfIntersectionArcs::GetFirstArcId() const
{
    return firstArcId;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetFirstArcId(const quint32 &value)
{
    firstArcId = value;
    ui->arc1Name_Label->setText(data->GetGObject(value)->name());

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)
    point->setArc1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointOfIntersectionArcs::GetSecondArcId() const
{
    return secondArcId;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetSecondArcId(const quint32 &value)
{
    secondArcId = value;
    ui->arc2Name_Label->setText(data->GetGObject(value)->name());

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)
    point->setArc2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
CrossCirclesPoint DialogPointOfIntersectionArcs::GetCrossArcPoint() const
{
    return getCurrentCrossPoint<CrossCirclesPoint>(ui->comboBoxResult);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetCrossArcPoint(const CrossCirclesPoint &p)
{
    const qint32 index = ui->comboBoxResult->findData(static_cast<int>(p));
    if (index != -1)
    {
        ui->comboBoxResult->setCurrentIndex(index);

        VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(point != nullptr)
        point->setCrossPoint(p);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Arc)
        {
            VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
            SCASSERT(point != nullptr)

            switch (number)
            {
                case 0:
                    firstArcId = id;
                    ui->arc1Name_Label->setText(data->GetGObject(id)->name());
                    number++;
                    point->VisualMode(id);
                    emit ToolTip(tr("Select second an arc"));
                    break;
                case 1:
                    if (firstArcId != id)
                    {
                        secondArcId = id;
                        ui->arc2Name_Label->setText(data->GetGObject(id)->name());
                        number = 0;
                        point->setArc2Id(id);
                        point->RefreshGeometry();
                        prepare = true;
                        emit ToolTip("");
                        DialogAccepted();
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::ShowVisualization()
{
    AddVisualization<VisToolPointOfIntersectionArcs>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)

    point->setArc1Id(GetFirstArcId());
    point->setArc2Id(GetSecondArcId());
    point->setCrossPoint(GetCrossArcPoint());
    point->RefreshGeometry();
}

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

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>

#include "../../visualization/visualization.h"
#include "../../visualization/line/vistoolpointofintersectionarcs.h"
#include "../vmisc/vabstractapplication.h"
#include "dialogs/tools/dialogtool.h"
#include "ui_dialogpointofintersectionarcs.h"

//---------------------------------------------------------------------------------------------------------------------
DialogPointOfIntersectionArcs::DialogPointOfIntersectionArcs(const VContainer *data, const quint32 &toolId,
                                                             QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogPointOfIntersectionArcs)
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

    FillComboBoxArcs(ui->comboBoxArc1);
    FillComboBoxArcs(ui->comboBoxArc2);
    FillComboBoxCrossCirclesPoints(ui->comboBoxResult);

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged,  this, &DialogPointOfIntersectionArcs::NamePointChanged);
    connect(ui->comboBoxArc1,      &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionArcs::ArcChanged);
    connect(ui->comboBoxArc1,      &QComboBox::currentTextChanged, this, &DialogPointOfIntersectionArcs::ArcChanged);

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
    return getCurrentObjectId(ui->comboBoxArc1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetFirstArcId(const quint32 &value)
{
    setCurrentArcId(ui->comboBoxArc1, value);

    VisToolPointOfIntersectionArcs *point = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
    SCASSERT(point != nullptr)
    point->setArc1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogPointOfIntersectionArcs::GetSecondArcId() const
{
    return getCurrentObjectId(ui->comboBoxArc2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogPointOfIntersectionArcs::SetSecondArcId(const quint32 &value)
{
    setCurrentArcId(ui->comboBoxArc2, value);

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
                    if (SetObject(id, ui->comboBoxArc1, tr("Select second an arc")))
                    {
                        number++;
                        point->VisualMode(id);
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->comboBoxArc1) != id)
                    {
                        if (SetObject(id, ui->comboBoxArc2, ""))
                        {
                            number = 0;
                            point->setArc2Id(id);
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
void DialogPointOfIntersectionArcs::ArcChanged()
{
    QColor color = okColor;
    if (getCurrentObjectId(ui->comboBoxArc1) == getCurrentObjectId(ui->comboBoxArc2))
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelArc1, color);
    ChangeColor(ui->labelArc2, color);
    CheckState();
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

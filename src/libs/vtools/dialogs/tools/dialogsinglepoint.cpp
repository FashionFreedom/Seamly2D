//-----------------------------------------------------------------------------
//  @file   dialogsinglepoint.cpp
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
//  @file   dialogsinglepoint.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
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

#include "dialogsinglepoint.h"

#include <QDoubleSpinBox>
#include <QLineEdit>

#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "dialogtool.h"
#include "ui_dialogsinglepoint.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogSinglePoint create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogSinglePoint::DialogSinglePoint(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogSinglePoint), point(QPointF())
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_basepoint_icon.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditName->setClearButtonEnabled(true);

    ui->doubleSpinBoxX->setRange(0, qApp->fromPixel(SceneSize));
    ui->doubleSpinBoxY->setRange(0, qApp->fromPixel(SceneSize));
    labelEditNamePoint = ui->labelEditName;
    initializeOkCancel(ui);

    flagName = true;
    DialogTool::CheckState();

    connect(ui->lineEditName, &QLineEdit::textChanged, this, &DialogTool::NamePointChanged);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePress get mouse position
 * @param scenePos position of cursor
 */
void DialogSinglePoint::mousePress(const QPointF &scenePos)
{
    if (isInitialized == false)
    {
        ui->doubleSpinBoxX->setValue(qApp->fromPixel(scenePos.x()));
        ui->doubleSpinBoxY->setValue(qApp->fromPixel(scenePos.y()));
        this->show();
    }
    else
    {
        ui->doubleSpinBoxX->setValue(qApp->fromPixel(scenePos.x()));
        ui->doubleSpinBoxY->setValue(qApp->fromPixel(scenePos.y()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSinglePoint::SaveData()
{
    point = QPointF(qApp->toPixel(ui->doubleSpinBoxX->value()), qApp->toPixel(ui->doubleSpinBoxY->value()));
    pointName = ui->lineEditName->text();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setData set name and point
 * @param name name of point
 * @param point data for point
 */
void DialogSinglePoint::SetData(const QString &name, const QPointF &point)
{
    pointName = name;
    this->point = point;
    isInitialized = true;
    ui->lineEditName->setText(name);
    ui->doubleSpinBoxX->setValue(qApp->fromPixel(point.x()));
    ui->doubleSpinBoxY->setValue(qApp->fromPixel(point.y()));
}

//---------------------------------------------------------------------------------------------------------------------
DialogSinglePoint::~DialogSinglePoint()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPoint return point
 * @return point
 */
QPointF DialogSinglePoint::GetPoint() const
{
    return point;
}

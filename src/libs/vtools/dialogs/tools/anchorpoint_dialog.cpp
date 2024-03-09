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
 **  @file   dialogpin.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#include "anchorpoint_dialog.h"
#include "ui_anchorpoint_dialog.h"
#include "visualization/line/anchorpoint_visual.h"
#include "../../tools/vabstracttool.h"
#include "../../tools/pattern_piece_tool.h"

//---------------------------------------------------------------------------------------------------------------------
AnchorPointDialog::AnchorPointDialog(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::AnchorPointDialog)
    , m_showMode(false)
    , m_flagPoint(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/anchor_point.png"));

    initializeOkCancel(ui);

    FillComboBoxPoints(ui->comboBoxPoint);

    flagError = false;
    CheckState();

    connect(ui->comboBoxPiece, &QComboBox::currentTextChanged, this, [this](){ CheckPieces(); });

    vis = new AnchorPointVisual(data);
}

//---------------------------------------------------------------------------------------------------------------------
AnchorPointDialog::~AnchorPointDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::EnbleShowMode(bool disable)
{
    m_showMode = disable;
    ui->comboBoxPiece->setDisabled(m_showMode);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AnchorPointDialog::GetPieceId() const
{
    return getCurrentObjectId(ui->comboBoxPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::SetPieceId(quint32 id)
{
    if (ui->comboBoxPiece->count() <= 0)
    {
        ui->comboBoxPiece->addItem(data->GetPiece(id).GetName(), id);
    }
    else
    {
        const qint32 index = ui->comboBoxPiece->findData(id);
        if (index != -1)
        {
            ui->comboBoxPiece->setCurrentIndex(index);
        }
        else
        {
            ui->comboBoxPiece->setCurrentIndex(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AnchorPointDialog::GetPointId() const
{
    return getCurrentObjectId(ui->comboBoxPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::SetPointId(quint32 id)
{
    setCurrentPointId(ui->comboBoxPoint, id);

    AnchorPointVisual *point = qobject_cast<AnchorPointVisual *>(vis);
    SCASSERT(point != nullptr)
    point->setObject1Id(id);

    CheckPoint();
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->comboBoxPiece, list);
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)
    {
        if (type == SceneObject::Point)
        {
            if (SetObject(id, ui->comboBoxPoint, ""))
            {
                vis->VisualMode(id);
                CheckPoint();
                prepare = true;
                this->setModal(true);
                this->show();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::CheckState()
{
    SCASSERT(ok_Button != nullptr);
    ok_Button->setEnabled(m_flagPoint && flagError);
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::ShowVisualization()
{
    AddVisualization<AnchorPointVisual>();

    if (m_showMode)
    {
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(GetPieceId()));
        SCASSERT(tool != nullptr);
        auto visPoint = qobject_cast<AnchorPointVisual *>(vis);
        SCASSERT(visPoint != nullptr);
        visPoint->setParentItem(tool);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::CheckPieces()
{
    if (not m_showMode)
    {
        QColor color = okColor;
        if (ui->comboBoxPiece->count() <= 0 || ui->comboBoxPiece->currentIndex() == -1)
        {
            flagError = false;
            color = errorColor;
        }
        else
        {
            flagError = true;
            color = okColor;
        }
        ChangeColor(ui->labelPiece, color);
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AnchorPointDialog::CheckPoint()
{
    QColor color = okColor;
    if (ui->comboBoxPoint->currentIndex() != -1)
    {
        m_flagPoint = true;
        color = okColor;
    }
    else
    {
        m_flagPoint = false;
        color = errorColor;
    }
    ChangeColor(ui->labelPoint, color);
    CheckState();
}

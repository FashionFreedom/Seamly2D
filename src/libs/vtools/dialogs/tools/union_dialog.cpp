//-----------------------------------------------------------------------------
//  @file   union_dialog.cpp
//  @author Douglas S Caskey
//  @date   Dec 27, 2022
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
//  @file   dialogunion.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   23 Dec, 2013
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

#include "union_dialog.h"

#include <QCheckBox>
#include <QVector>

#include "../ifc/ifcdef.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "dialogtool.h"
#include "ui_union_dialog.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnionDialog create dialog
 * @param data container with data
 * @param parent parent widget
 */
UnionDialog::UnionDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::UnionDialog)
    , piece1_Index(0)
    , piece2_Index(0)
    , d1(NULL_ID)
    , d2(NULL_ID)
    , numberD(0)
    , numberP(0)
    , p1(NULL_ID)
    , p2(NULL_ID)
    , m_beep(new QSound(qApp->Settings()->getSelectionSound()))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/union.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    initializeOkCancel(ui);
}

//---------------------------------------------------------------------------------------------------------------------
UnionDialog::~UnionDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChosenObject gets id and type of selected object. Save correct data and ignore wrong.
 * @param id id of point or piece
 * @param type type of object
 */
void UnionDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (numberD == 0)
    {
        chosenPiece(id, type, d1, piece1_Index);
    }
    else
    {
        chosenPiece(id, type, d2, piece2_Index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckObject check if piece contains this id
 * @param id id of item
 * @param pieceId piece id
 * @return true if contain
 */
bool UnionDialog::CheckObject(const quint32 &id, const quint32 &pieceId) const
{
    if (pieceId == NULL_ID)
    {
        return false;
    }
    const VPiece piece = data->GetPiece(pieceId);
    return piece.GetPath().Contains(id);
}

//---------------------------------------------------------------------------------------------------------------------
bool UnionDialog::isPieceValid(const quint32 &pieceId) const
{
    if (pieceId == NULL_ID)
    {
        return false;
    }
    const VPiece piece = data->GetPiece(pieceId);
    if (piece.GetPath().CountNodes() >= 3 && piece.GetPath().ListNodePoint().size() >= 2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief chosenPiece help save information about piece and points on piece
 * @param id id selected object
 * @param type type selected object
 * @param pieceId id of piece
 * @param index index of edge
 */
void UnionDialog::chosenPiece(const quint32 &id, const SceneObject &type, quint32 &pieceId,
                                       int &index)
{
    if (pieceId == NULL_ID)
    {
        if (type == SceneObject::Piece)
        {
            m_beep->play();
            if (isPieceValid(id))
            {
                pieceId = id;
                emit ToolTip(tr("Select the first point"));
                return;
            }
            else
            {
                emit ToolTip(tr("Pattern piece should have at least two points and three objects"));
                return;
            }
        }
    }
    if (CheckObject(id, pieceId) == false)
    {
        return;
    }
    if (type == SceneObject::Point)
    {
        m_beep->play();
        if (numberP == 0)
        {
            p1 = id;
            ++numberP;
            emit ToolTip(tr("Select a second point"));
            return;
        }
        if (numberP == 1)
        {
            if (id == p1)
            {
                emit ToolTip(tr("Select a unique point"));
                return;
            }
            VPiece piece = data->GetPiece(pieceId);
            if (piece.GetPath().OnEdge(p1, id))
            {
                p2 = id;
                index = piece.GetPath().Edge(p1, p2);
                ++numberD;
                if (numberD > 1)
                {
                    ++numberP;
                    emit ToolTip("");
                    this->setModal(true);
                    this->show();
                    return;
                }
                else
                {
                    numberP = 0;
                    p1 = 0;
                    p2 = 0;
                    emit ToolTip(tr("Select a piece"));
                    return;
                }
            }
            else
            {
                emit ToolTip(tr("Select a point on edge"));
                return;
            }
        }
    }
}

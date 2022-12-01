/***************************************************************************
 **  @file   piece_tablewidgetitem.cpp
 **  @author Douglas S Caskey
 **  @date   Jan 7, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "piece_tablewidgetitem.h"

#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"

PieceTableWidgetItem::PieceTableWidgetItem(VContainer *data)
    : QTableWidgetItem()
    , m_data(data)
    {}

//---------------------------------------------------------------------------------------------------------------------
bool PieceTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    if (other.column() == 0)
    {
        VPiece thisPiece = m_data->GetPiece(this->data(Qt::UserRole).toUInt());
        VPiece otherPiece = m_data->GetPiece(other.data(Qt::UserRole).toUInt());

        return int(thisPiece.isInLayout()) < int(otherPiece.isInLayout());
    }
    else if (other.column() == 1)
    {
        VPiece thisPiece = m_data->GetPiece(this->data(Qt::UserRole).toUInt());
        VPiece otherPiece = m_data->GetPiece(other.data(Qt::UserRole).toUInt());

        return int(thisPiece.isLocked()) < int(otherPiece.isLocked());
    }
    else if (other.column() == 2)
    {
        return this->background().color().name() < other.background().color().name();
    }
    return false;
}

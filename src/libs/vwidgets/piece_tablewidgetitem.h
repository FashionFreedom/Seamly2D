/***************************************************************************
 **  @file   piece_tablewidgetitem.c
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

#ifndef PIECE_TABLEWIDGETITEM_H
#define PIECE_TABLEWIDGETITEM_H

#include <QTableWidgetItem>

class VContainer;

class PieceTableWidgetItem : public QTableWidgetItem
{

public:
    explicit     PieceTableWidgetItem(VContainer *data);
    virtual bool operator<(const QTableWidgetItem &other) const Q_DECL_OVERRIDE;

private:
    VContainer  *m_data;
};

#endif // PIECE_TABLEWIDGETITEM_H

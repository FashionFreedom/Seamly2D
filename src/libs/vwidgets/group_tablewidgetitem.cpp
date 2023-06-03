/***************************************************************************
 **  @file   group_tablewidgetitem.cpp
 **  @author Douglas S Caskey
 **  @date   11 Jun, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

#include "group_tablewidgetitem.h"

#include "../ifc/xml/vabstractpattern.h"

GroupTableWidgetItem::GroupTableWidgetItem(VAbstractPattern *doc)
    : QTableWidgetItem()
    , m_doc(doc)
    {}

//---------------------------------------------------------------------------------------------------------------------
bool GroupTableWidgetItem::operator<(const QTableWidgetItem &other) const
{
    if (other.column() == 0)
    {
        bool thisVisible  = m_doc->getGroupVisibility(this->data(Qt::UserRole).toUInt());
        bool otherVisible = m_doc->getGroupVisibility(other.data(Qt::UserRole).toUInt());
        return int(thisVisible) < int(otherVisible);
    }
    else if (other.column() == 1)
    {
        bool thisLocked  = m_doc->getGroupLock(this->data(Qt::UserRole).toUInt());
        bool otherLocked = m_doc->getGroupLock(other.data(Qt::UserRole).toUInt());
        return int(thisLocked) < int(otherLocked);
    }
    else if (other.column() == 2)
    {
        bool thisEmpty  = m_doc->isGroupEmpty(this->data(Qt::UserRole).toUInt());
        bool otherEmpty = m_doc->isGroupEmpty(other.data(Qt::UserRole).toUInt());
        return int(!thisEmpty) < int(!otherEmpty);
    }
    else if (other.column() == 3)
    {
        return this->data(Qt::UserRole).toString() < other.data(Qt::UserRole).toString();
    }

    return false;
}

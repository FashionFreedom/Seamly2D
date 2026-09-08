//---------------------------------------------------------------------------------------------------------------------
//  @file   tst_vcontainer.h
//  @author Odrnorn
//  @date   8 Sep, 2026
//
//  @copyright
//  Copyright (C)  2026 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

#ifndef TST_VCONTAINER_H
#define TST_VCONTAINER_H

#include <QObject>

class TST_VContainer : public QObject
{
    Q_OBJECT
public:
    explicit TST_VContainer(QObject *parent = nullptr);

private slots:
    void reparseDoesNotClearOtherBlocks() const;
    void unknownToolIsNeverCleared() const;

private:
    Q_DISABLE_COPY(TST_VContainer)
};

#endif // TST_VCONTAINER_H

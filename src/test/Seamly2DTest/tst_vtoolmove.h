//---------------------------------------------------------------------------------------------------------------------
//  @file   tst_vtoolmove.h
//  @author Andrea Gorletta
//  @date   8 Aug, 2026
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

#ifndef TST_VTOOLMOVE_H
#define TST_VTOOLMOVE_H

#include <QObject>

class TST_VToolMove : public QObject
{
    Q_OBJECT
public:
    explicit TST_VToolMove(QObject *parent = nullptr);

private slots:
    void explicitOriginIsNotTranslatedByTheMove();
    void explicitOriginIncludedInTheMoveFollowsTheMove();
    void moveWithRotationMatchesRotationAfterMove();
    void implicitOriginStillFollowsTheMove();

private:
    Q_DISABLE_COPY(TST_VToolMove)
};

#endif // TST_VTOOLMOVE_H

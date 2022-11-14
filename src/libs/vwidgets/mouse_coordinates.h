/******************************************************************************
 *   @file   mouse_coordinates.h
 **  @author DS Caskey
 **  @date   Nov 6, 2022
 **
 **  @brief
 **  @copyright
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
 *****************************************************************************/
#ifndef MOUSE_COORDINATES_H
#define MOUSE_COORDINATES_H

#include <QPointF>
#include <QWidget>

#include "../vmisc/def.h"

namespace Ui
{
    class MouseCoordinates;
}

class MouseCoordinates: public QWidget
{
    Q_OBJECT

public:
                         MouseCoordinates(const Unit &units, QWidget *parent = nullptr);
    virtual             ~MouseCoordinates();

public slots:
    void                 updateCoordinates(const QPointF &scenePos);

private:
    Ui::MouseCoordinates *ui;
    Unit                  m_units;
};

#endif

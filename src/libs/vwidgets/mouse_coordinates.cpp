 /******************************************************************************
  *   @file   mouse_coordinates.cpp
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
 #include "mouse_coordinates.h"
 #include "ui_mouse_coordinates.h"

#include <QLabel>
#include <QPointF>
#include <QString>
#include <Qt>

#include "../vmisc/vabstractapplication.h"

//---------------------------------------------------------------------------------------------------------------------
/*
 * A widget for displaying the mouse coordinates in pattern units.
 *
 * Constructor.
 */
MouseCoordinates::MouseCoordinates(const Unit &units, QWidget *parent)
		: QWidget(parent)
        , ui(new Ui::MouseCoordinates)
      	, m_units(units)
{
    ui->setupUi(this);
    updateCoordinates(QPointF());
    ui->unitsText_Label->setText(UnitsToStr(m_units));
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * Destructor
 */
MouseCoordinates::~MouseCoordinates() {}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Show user updated mouse oordinates. Converts from pixels to pattern units. 
 * @param scenePos mouse position.
 */
void MouseCoordinates::updateCoordinates(const QPointF &scenePos)
{
    ui->xposText_Label->setText(QString(QString::number(qApp->fromPixel(scenePos.x()), 'f', 2 )));
    ui->yposText_Label->setText(QString(QString::number(qApp->fromPixel(scenePos.y()), 'f', 2 )));
}

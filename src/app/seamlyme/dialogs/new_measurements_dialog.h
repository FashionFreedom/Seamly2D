//  @file   new_measurements_dialog.h
//  @author Douglas S Caskey
//  @date   3 Jan, 2024
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2024 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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

/************************************************************************
 **
 **  @file   dialognewmeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#ifndef NEW_MEASUREMENTS_DIALOG_H
#define NEW_MEASUREMENTS_DIALOG_H

#include <QDialog>

#include "../vmisc/def.h"

namespace Ui
{
    class NewMeasurementsDialog;
}

class NewMeasurementsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit                   NewMeasurementsDialog(QWidget *parent = nullptr);
                              ~NewMeasurementsDialog();

    MeasurementsType           type() const;
    Unit                       measurementUnits() const;
    int                        baseSize() const;
    int                        baseHeight() const;

protected:
    virtual void               changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    virtual void               showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

private slots:
    void                       currentTypeChanged(int index);
    void                       currentUnitChanged(int index);

private:
                               Q_DISABLE_COPY(NewMeasurementsDialog)
    Ui::NewMeasurementsDialog *ui;
    bool                       isInitialized;

    void                       initializeMeasurementTypes();
    void                       initializeHeightsList();
    void                       initializeSizesList();
    void                       initializeUnits(const MeasurementsType &type);
};

#endif // NEW_MEASUREMENTS_DIALOG_H

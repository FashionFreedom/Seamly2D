//  @file   dialognewpattern.h
//  @author Douglas S Caskey
//  @date   12 Nov, 2023
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.

/************************************************************************
 **
 **  @file   dialogindividualmeasurements.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 2, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef DIALOG_NEWPATTERN_H
#define DIALOG_NEWPATTERN_H

#include <QDialog>

#include "../vmisc/def.h"

class VContainer;

namespace Ui
{
    class DialogNewPattern;
}

class DialogNewPattern : public QDialog
{
    Q_OBJECT
public:
                          DialogNewPattern(VContainer *data, const QString &patternPieceName,
                                           QWidget *parent = nullptr);
    virtual              ~DialogNewPattern();
    QString               name() const;
    Unit                  patternUnit() const;

protected:
    virtual void          showEvent(QShowEvent *event) override;

private slots:
    void                  CheckState();

private:
                          Q_DISABLE_COPY(DialogNewPattern)
    Ui::DialogNewPattern *ui;
    VContainer           *data;
    bool                  isInitialized;
    void                  initUnits();
};

#endif // DIALOG_NEWPATTERN_H

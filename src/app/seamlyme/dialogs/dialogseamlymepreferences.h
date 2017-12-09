/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialogseamlymepreferences.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#ifndef DIALOGSEAMLYMEPREFERENCES_H
#define DIALOGSEAMLYMEPREFERENCES_H

#include <QDialog>

namespace Ui
{
    class DialogSeamlyMePreferences;
}

class SeamlyMePreferencesConfigurationPage;
class SeamlyMePreferencesPathPage;
class QListWidgetItem;

class DialogSeamlyMePreferences : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSeamlyMePreferences(QWidget *parent = nullptr);
    virtual ~DialogSeamlyMePreferences();
signals:
    void UpdateProperties();
protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
private slots:
    void Apply();
    void Ok();
    void PageChanged(QListWidgetItem *current, QListWidgetItem *previous);
private:
    Q_DISABLE_COPY(DialogSeamlyMePreferences)
    Ui::DialogSeamlyMePreferences *ui;
    bool m_isInitialized;
    SeamlyMePreferencesConfigurationPage *m_configurationPage;
    SeamlyMePreferencesPathPage          *m_pathPage;
};

#endif // DIALOGSEAMLYMEPREFERENCES_H

/************************************************************************
 **
 **  @file   PreferencesGraphicsViewPage.h
 **  @author DS Caskey
 **  @date   10.17.2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
 **  All Rights Reserved.
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

#ifndef PREFERENCES_GRAPHICSVIEWPAGE_H
#define PREFERENCES_GRAPHICSVIEWPAGE_H

#include <QWidget>

namespace Ui
{
    class PreferencesGraphicsViewPage ;
}

class PreferencesGraphicsViewPage  : public QWidget
{
    Q_OBJECT

public:
    explicit                          PreferencesGraphicsViewPage (QWidget *parent = nullptr);
    virtual                          ~PreferencesGraphicsViewPage ();

    void                              Apply();

protected:

private:
    Q_DISABLE_COPY(PreferencesGraphicsViewPage )
    Ui::PreferencesGraphicsViewPage  *ui;
    bool                              m_zrbPositiveColorChanged;
    bool                              m_zrbNegativeColorChanged;
    bool                              m_pointNameColorChanged;
    bool                              m_pointNameHoverColorChanged;
    bool                              m_orginAxisColorChanged;
    bool                              m_primarySupportColorChanged;
    bool                              m_secondarySupportColorChanged;
    bool                              m_tertiarySupportColorChanged;
};

#endif // PREFERENCES_GRAPHICSVIEWPAGE_H

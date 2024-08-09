//  @file   preferencesconfigurationpage.h
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
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
 **  @file   preferencesconfigurationpage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  if not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef PREFERENCESCONFIGURATIONPAGE_H
#define PREFERENCESCONFIGURATIONPAGE_H

#include <QWidget>

namespace Ui
{
    class PreferencesConfigurationPage;
}

class PreferencesConfigurationPage : public QWidget
{
    Q_OBJECT

public:
    explicit     PreferencesConfigurationPage(QWidget *parent = nullptr);
    virtual     ~PreferencesConfigurationPage();

    void         apply();

protected:
    virtual void changeEvent(QEvent* event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(PreferencesConfigurationPage)
    Ui::PreferencesConfigurationPage *ui;
    bool m_langChanged;
    bool m_unitChanged;
    bool m_labelLangChanged;
    bool m_selectionSoundChanged;
    bool m_moveSuffixChanged;
    bool m_rotateSuffixChanged;
    bool m_mirrorByAxisSuffixChanged;
    bool m_mirrorByLineSuffixChanged;
    bool m_defaultExportFormatChanged;

    void setPointNameComboBox(const QStringList &list);
    void initUnits();
    void adjustTextColor();
};

#endif // PREFERENCESCONFIGURATIONPAGE_H

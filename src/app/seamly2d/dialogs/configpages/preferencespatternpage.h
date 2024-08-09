/******************************************************************************
*   @file   preferencespatternpage.h
**  @author Douglas S Caskey
**  @date   26 Oct, 2023
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

/************************************************************************
 **
 **  @file   preferencespatternpage.h
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

#ifndef PREFERENCESPATTERNPAGE_H
#define PREFERENCESPATTERNPAGE_H

#include <QWidget>

namespace Ui
{
    class PreferencesPatternPage;
}

class QComboBox;

class PreferencesPatternPage : public QWidget
{
    Q_OBJECT

public:
    explicit      PreferencesPatternPage(QWidget *parent = nullptr);
    virtual      ~PreferencesPatternPage();

    void          Apply();
    void          initDefaultSeamAllowance();

protected:
    virtual void  changeEvent(QEvent* event) Q_DECL_OVERRIDE;

private slots:
    void          editDateTimeFormats();
    void          setDefaultTemplate();

private:
    Q_DISABLE_COPY(PreferencesPatternPage)
    Ui::PreferencesPatternPage *ui;

    void          initializeLabelsTab();
    void          initNotches();
    void          initGrainlines();
    void          initComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat);

    template <typename T>
    void          callDateTimeFormatEditor(const T &type, const QStringList &predefinedFormats,
                                           const QStringList &userDefinedFormats, QComboBox *box);
};

#endif // PREFERENCESPATTERNPAGE_H

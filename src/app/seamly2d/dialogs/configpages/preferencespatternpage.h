//---------------------------------------------------------------------------------------------------------------------
//  @file   preferencespatternpage.h
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
//
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017 - 2025 Seamly, LLC
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   preferencespatternpage.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   12 4, 2017
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2017 Seamly2D project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef PREFERENCESPATTERNPAGE_H
#define PREFERENCESPATTERNPAGE_H

#include <QWidget>

namespace Ui
{
    class PreferencesPatternPage;
}

class QComboBox;
class QDoubleSpinBox;

class PreferencesPatternPage : public QWidget
{
    Q_OBJECT

public:
    explicit      PreferencesPatternPage(QWidget *parent = nullptr);
    virtual      ~PreferencesPatternPage();

    void          Apply();
    void          initDefaultSeamAllowance();

protected:
    virtual void  changeEvent(QEvent* event) override;

private slots:
    void          editDateTimeFormats();
    void          setDefaultTemplate();

private:
    Q_DISABLE_COPY(PreferencesPatternPage)
    Ui::PreferencesPatternPage *ui;

    void          initializeLabelsTab();
    void          initNotches();
    void          initGrainlines();
    void          arrowLengthChanged();
    void          grainlineLengthChanged();
    void          initComboBoxFormats(QComboBox *box, const QStringList &items, const QString &currentFormat);
    void          changeColor(QWidget *widget, const QColor &color);


    template <typename T>
    void          callDateTimeFormatEditor(const T &type, const QStringList &predefinedFormats,
                                           const QStringList &userDefinedFormats, QComboBox *box);

    void          setMaxByUnits(QDoubleSpinBox *box, const qreal &value);
};

#endif // PREFERENCESPATTERNPAGE_H

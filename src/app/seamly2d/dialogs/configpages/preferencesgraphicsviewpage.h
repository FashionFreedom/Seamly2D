//-----------------------------------------------------------------------------
//  @file   PreferencesGraphicsViewPage.h
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
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
//-----------------------------------------------------------------------------

#ifndef PREFERENCES_GRAPHICSVIEWPAGE_H
#define PREFERENCES_GRAPHICSVIEWPAGE_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>

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
    void                              enableOffsets();
    virtual void                      changeEvent(QEvent *event) override;

private:
    Q_DISABLE_COPY(PreferencesGraphicsViewPage )
    Ui::PreferencesGraphicsViewPage  *ui;

    void                              setIndex(QComboBox *box, const QString &text);
    void                              setupFontComboBox(QComboBox *combo_box, QLabel *label, const QFont &initial_font,
                                                        QComboBox *size_combo_box = nullptr, int fixed_size = 0);

    int                               getTargetSize(QComboBox * size_combo_box, int fixed_size, int fallback_size);

};

#endif // PREFERENCES_GRAPHICSVIEWPAGE_H

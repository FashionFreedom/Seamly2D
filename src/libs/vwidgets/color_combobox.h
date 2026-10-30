//---------------------------------------------------------------------------------------------------------------------
///  @file   color_comboox.cpp
///  @author Douglas S Caskey
///  @date   11 Apr, 2025
///
/// @copyright
/// Copyright (C) 2017 - 2026 Seamly, LLC
/// https://github.com/fashionfreedom/seamly2d
///
/// @brief
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef COLOR_COMBOBOX_H
#define COLOR_COMBOBOX_H

#include <QComboBox>
#include <QString>
#include <QWidget>
#include <memory>

class ColorComboBox: public QComboBox
{
    Q_OBJECT

public:
             ColorComboBox(QWidget *parent = nullptr , const char *name = nullptr );
             ColorComboBox(int width, int height, QWidget *parent = nullptr , const char *name = nullptr );
    virtual ~ColorComboBox();

    QString  getColor() const;
    void     setColor(const QString &color);
    void     setItems(QMap<QString, QString> map);
    int      getIconWidth();
    int      getIconHeight();

private slots:
    void     colorChanged(int index);

signals:
    void     colorChangedSignal(const QString &color);

private:
    void     init();
    QString  m_current_color;
    int      m_icon_width;
    int      m_icon_height;
};

#endif

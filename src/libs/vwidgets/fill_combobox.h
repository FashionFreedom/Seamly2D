/***************************************************************************
 **  @file   fill_combobox.h
 **  @author Douglas S Caskey
 **  @date   Dec 28, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#ifndef FILL_COMBOBOX_H
#define FILL_COMBOBOX_H

#include <QComboBox>
#include <QString>
#include <memory>

/**
 * A comboBox for choosing a fill pattern.
 */
class PieceFillComboBox: public QComboBox
{
    Q_OBJECT

public:
              PieceFillComboBox(QWidget *parent = nullptr, const char *name = nullptr);
              PieceFillComboBox(int width, int height, QWidget *parent = nullptr, const char *name = nullptr);
    virtual  ~PieceFillComboBox();



    QString   getFill() const;
    void      setFill(const QString &fill);
    QPixmap   createFillIcon(const int w, const int h, Qt::BrushStyle style);
    int       getIconWidth();
    int       getIconHeight();

private slots:
    void      fillChanged(int index);

signals:
    void      fillChangedSignal(const QString &fill);

private:
    void      init();
    QString   m_currentFill;
    int       m_iconWidth;
    int       m_iconHeight;
};

#endif

//-----------------------------------------------------------------------------
//  @file   size_range_dialog.h
//  @date   29 Mar, 2026
//
//  @brief
//  Dialog for restricting the active size and height range in a
//  multisize measurement file, and for editing optional display
//  Aliases for each size/height value.
//
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
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

#ifndef SIZE_RANGE_DIALOG_H
#define SIZE_RANGE_DIALOG_H

#include "../vformat/measurements.h"

#include <QDialog>
#include <QMap>

namespace Ui
{
    class SizeRangeDialog;
}

class QComboBox;
class MeasurementDoc;
class QTableWidget;
class QTabWidget;

class SizeRangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit             SizeRangeDialog(MeasurementDoc *measurements, const QStringList &allSizes,
                                         const QStringList &allHeights,
                                         const QMap<int, QString> &sizeAliases = QMap<int, QString>(),
                                         const QMap<int, QString> &heightAliases = QMap<int, QString>(),
                                         const QMap<int, QString> &sizeColors = QMap<int, QString>(),
                                         QWidget *parent = nullptr);

                        ~SizeRangeDialog();

    int                  minSize() const;
    int                  maxSize() const;
    int                  sizeStep() const;
    int                  minHeight() const;
    int                  maxHeight() const;
    int                  heightStep() const;

    QMap<int, QString>   sizeAliases() const;
    QMap<int, QString>   heightAliases() const;
    QMap<int, QString>   sizeColors() const;

private slots:
    void                 rangeChanged(int index);

    void                 validate();
    void                 clearSizes();
    void                 clearHeights();

private:
    Q_DISABLE_COPY(SizeRangeDialog)

    Ui::SizeRangeDialog    *ui;
    MeasurementDoc         *m_measurements;
    QStringList             m_allSizes;
    QStringList             m_allHeights;
    int                     m_baseSize;
    int                     m_baseHeight;
    QMap<int, QString>      m_sizeAliases;
    QMap<int, QString>      m_heightAliases;
    QMap<int, QString>      m_sizeColors;

    void                    constrainComboBoxes();
    void                    refresh();
    void                    fillTable(QTableWidget *table, const QStringList &values,
                                      const QMap<int, QString> &Aliases, const QMap<int, QString> &colors,
                                      int base, int rangeMin, int rangeMax, int step);

    void                    cellDoubleClicked(int row, int column);
};

#endif // SIZE_RANGE_DIALOG_H

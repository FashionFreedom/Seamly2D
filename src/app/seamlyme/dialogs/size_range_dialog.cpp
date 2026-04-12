//-----------------------------------------------------------------------------
//  @file   size_range_dialog.cpp
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

#include "size_range_dialog.h"
#include "ui_size_range_dialog.h"

#include "../vformat/measurements.h"

#include <QBrush>
#include <QColor>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QStringList>
#include <QTableWidget>
#include <QTabWidget>
#include <QWidget>

// We need this enum in case we will add or delete a column. It also makes code more readable.
enum {BaseColumn = 0, SizeColumn = 1, HeightColumn = 1 , AliasColumn = 2, ColorColumn = 3};

//---------------------------------------------------------------------------------------------------------------------
SizeRangeDialog::SizeRangeDialog(MeasurementDoc *measurements, const QStringList &allSizes,
                                 const QStringList &allHeights, const QMap<int, QString> &sizeAliases,
                                 const QMap<int, QString> &heightAliases, const QMap<int, QString> &sizeColors,
                                 QWidget *parent)

    : QDialog(parent)
    , ui(new Ui::SizeRangeDialog)
    , m_measurements(measurements)
    , m_allSizes(allSizes)
    , m_allHeights(allHeights)
    , m_baseSize(0)
    , m_baseHeight(0)
    , m_sizeAliases(sizeAliases)
    , m_heightAliases(heightAliases)
    , m_sizeColors(sizeColors)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    for (const QString &string : m_allSizes)
    {
        const int value = string.toInt();
        ui->minSize_ComboBox->addItem(string, value);
        ui->maxSize_ComboBox->addItem(string, value);
    }

    for (const QString &string : m_allHeights)
    {
        const int value = string.toInt();
        ui->minHeight_ComboBox->addItem(string, value);
        ui->maxHeight_ComboBox->addItem(string, value);
    }

    ui->sizeStep_ComboBox->setItemData(0, 2);
    ui->sizeStep_ComboBox->setItemData(1, 4);
    ui->heightStep_ComboBox->setItemData(0, 6);
    ui->heightStep_ComboBox->setItemData(1, 12);

    m_baseSize   = m_measurements->getBaseSize();
    m_baseHeight = m_measurements->getBaseHeight();

    // Set current values using data (numeric values stored in UserRole)
    int index = ui->minSize_ComboBox->findData(m_measurements->minSize());
    if (index != -1)
    {
        ui->minSize_ComboBox->setCurrentIndex(index);
    }

    index = ui->maxSize_ComboBox->findData(m_measurements->maxSize());
    if (index != -1)
    {
        ui->maxSize_ComboBox->setCurrentIndex(index);
    }

    index = ui->minHeight_ComboBox->findData(m_measurements->minHeight());
    if (index != -1)
    {
        ui->minHeight_ComboBox->setCurrentIndex(index);
    }

    index = ui->maxHeight_ComboBox->findData(m_measurements->maxHeight());
    if (index != -1)
    {
        ui->maxHeight_ComboBox->setCurrentIndex(index);
    }

    index = ui->sizeStep_ComboBox->findData(m_measurements->sizeStep());
    if (index != -1)
    {
        ui->sizeStep_ComboBox->setCurrentIndex(index);
    }

    index = ui->heightStep_ComboBox->findData(m_measurements->heightStep());
    if (index != -1)
    {
        ui->heightStep_ComboBox->setCurrentIndex(index);
    }

    constrainComboBoxes();
    refresh();

    connect(ui->minSize_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);
    connect(ui->maxSize_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);
    connect(ui->sizeStep_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);

    connect(ui->minHeight_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);
    connect(ui->maxHeight_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);
    connect(ui->heightStep_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &SizeRangeDialog::rangeChanged);

    connect(ui->clearSizes_PushButton,   &QPushButton::clicked, this, &SizeRangeDialog::clearSizes);
    connect(ui->clearHeights_PushButton, &QPushButton::clicked, this, &SizeRangeDialog::clearHeights);

    connect(ui->sizes_TableWidget,   &QTableWidget::cellDoubleClicked, this, &SizeRangeDialog::cellDoubleClicked);
    connect(ui->heights_TableWidget, &QTableWidget::cellDoubleClicked, this, &SizeRangeDialog::cellDoubleClicked);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

//---------------------------------------------------------------------------------------------------------------------
SizeRangeDialog::~SizeRangeDialog()
{
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::minSize() const
{
    return ui->minSize_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::maxSize() const
{
    return ui->maxSize_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::sizeStep() const
{
    return ui->sizeStep_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::minHeight() const
{
    return ui->minHeight_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::maxHeight() const
{
    return ui->maxHeight_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int SizeRangeDialog::heightStep() const
{
    return ui->heightStep_ComboBox->currentData().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> SizeRangeDialog::sizeAliases() const
{
    QMap<int, QString> map;
    for (int row = 0; row < ui->sizes_TableWidget->rowCount(); ++row)
    {
        QLineEdit *edit = qobject_cast<QLineEdit *>(ui->sizes_TableWidget->cellWidget(row, AliasColumn));
        if (edit && !edit->text().trimmed().isEmpty())
        {
            QTableWidgetItem *item = ui->sizes_TableWidget->item(row, SizeColumn);
            if (item)
            {
                map.insert(item->data(Qt::UserRole).toInt(), edit->text().trimmed());
            }
        }
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> SizeRangeDialog::heightAliases() const
{
    QMap<int, QString> map;
    for (int row = 0; row < ui->heights_TableWidget->rowCount(); ++row)
    {
        QLineEdit *edit = qobject_cast<QLineEdit *>(ui->heights_TableWidget->cellWidget(row, AliasColumn));
        if (edit && !edit->text().trimmed().isEmpty())
        {
            QTableWidgetItem *item = ui->heights_TableWidget->item(row, HeightColumn);
            if (item)
            {
                map.insert(item->data(Qt::UserRole).toInt(), edit->text().trimmed());
            }
        }
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<int, QString> SizeRangeDialog::sizeColors() const
{
    QMap<int, QString> map;
    for (int row = 0; row < ui->sizes_TableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *sizeItem = ui->sizes_TableWidget->item(row, SizeColumn);
        QTableWidgetItem *colorItem = ui->sizes_TableWidget->item(row, ColorColumn);
        if (sizeItem && colorItem)
        {
            map.insert(sizeItem->data(Qt::UserRole).toInt(), colorItem->background().color().name());
        }
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::rangeChanged(int index)
{
    Q_UNUSED(index)
    constrainComboBoxes();
    refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::validate()
{
    const int sMin = minSize();
    const int sMax = maxSize();
    const int hMin = minHeight();
    const int hMax = maxHeight();

    const bool valid = (sMin <= m_baseSize && sMax >= m_baseSize &&
                        hMin <= m_baseHeight && hMax >= m_baseHeight &&
                        sMin <= sMax && hMin <= hMax);

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(valid);
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::clearSizes()
{
    for (int row = 0; row < ui->sizes_TableWidget->rowCount(); ++row)
    {
        // Safely cast cellWidget to a QLineEdit
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(ui->sizes_TableWidget->cellWidget(row, AliasColumn));

        // Clear if the cast was successful
        if (lineEdit)
        {
            lineEdit->clear();
        }
        if (ui->sizes_TableWidget->columnCount() == 4 )
        {
            QTableWidgetItem *colorItem = ui->sizes_TableWidget->item(row, ColorColumn);
            colorItem->setBackground(QBrush(QColor("#000000"))); // Set initial color
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::clearHeights()
{
    for (int row = 0; row < ui->heights_TableWidget->rowCount(); ++row)
    {
        // Safely cast cellWidget to a QLineEdit
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(ui->heights_TableWidget->cellWidget(row, AliasColumn));

        // Clear if the cast was successful
        if (lineEdit)
        {
            lineEdit->clear();
        }

    }
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::constrainComboBoxes()
{
    // Ensure min sizes <= max sizes
    const int sizeMin = ui->minSize_ComboBox->currentData().toInt();
    const int sizeMax = ui->maxSize_ComboBox->currentData().toInt();

    if (sizeMin > sizeMax)
    {
        // Adjust To to match min
        const int index = ui->maxSize_ComboBox->findData(sizeMin);
        if (index != -1)
        {
            ui->maxSize_ComboBox->blockSignals(true);
            ui->maxSize_ComboBox->setCurrentIndex(index);
            ui->maxSize_ComboBox->blockSignals(false);
        }
    }

    // Ensure min heights <= max heights
    const int heightMin = ui->minHeight_ComboBox->currentData().toInt();
    const int heightMax = ui->maxHeight_ComboBox->currentData().toInt();
    if (heightMin > heightMax)
    {
        const int index = ui->maxHeight_ComboBox->findData(heightMin);
        if (index != -1)
        {
            ui->maxHeight_ComboBox->blockSignals(true);
            ui->maxHeight_ComboBox->setCurrentIndex(index);
            ui->maxHeight_ComboBox->blockSignals(false);
        }
    }

    validate();
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::refresh()
{
    // Save any in-progress edits before rebuilding
    QMap<int, QString> currentSizeEdits    = sizeAliases();
    QMap<int, QString> currentHeightEdits  = heightAliases();
    QMap<int, QString> currentColorsEdits = sizeColors();

    // Merge saved aliases with in-progress edits (edits take precedence)
    QMap<int, QString> mergedSizeAliases = m_sizeAliases;
    for (auto it = currentSizeEdits.constBegin(); it != currentSizeEdits.constEnd(); ++it)
    {
        mergedSizeAliases.insert(it.key(), it.value());
    }

    QMap<int, QString> mergedHeightAliases = m_heightAliases;
    for (auto it = currentHeightEdits.constBegin(); it != currentHeightEdits.constEnd(); ++it)
    {
        mergedHeightAliases.insert(it.key(), it.value());
    }

    QMap<int, QString> mergedSizeColors = m_sizeColors;
    for (auto it = currentColorsEdits.constBegin(); it != currentColorsEdits.constEnd(); ++it)
    {
        mergedSizeColors.insert(it.key(), it.value());
    }

    fillTable(ui->sizes_TableWidget, m_allSizes, mergedSizeAliases, mergedSizeColors,
              m_baseSize, minSize(), maxSize(), sizeStep()/2);
    fillTable(ui->heights_TableWidget, m_allHeights, mergedHeightAliases, mergedSizeColors,
              m_baseHeight, minHeight(), maxHeight(), heightStep()/6);
}

//---------------------------------------------------------------------------------------------------------------------
void SizeRangeDialog::fillTable(QTableWidget *table, const QStringList &values,
                                const QMap<int, QString> &aliases, const QMap<int, QString> &colors,
                                int base, int rangeMin, int rangeMax, int step)
{
    table->setRowCount(0);
    table->setColumnWidth(BaseColumn, 40);
    table->setColumnWidth(SizeColumn, 60);
    table->setColumnWidth(AliasColumn, 100);
    table->setColumnWidth(ColorColumn, 100);

    for (int i = 0; i < values.length(); i += step)
    {
        if (i > values.length())
        {
            i = values.length();
        }
        const int value = values.at(i).toInt();
        if (value < rangeMin || value > rangeMax)
        {
            continue;
        }

        const int row = table->rowCount();
        table->insertRow(row);

        // Base size or height
        QTableWidgetItem *baseItem = new QTableWidgetItem(QString(""));
        baseItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        //baseItem->setFlags(baseItem->flags() & ~Qt::ItemIsEditable);
        if (value == base)
        {
            //baseItem->setData(Qt::UserRole, base);
            baseItem->setIcon(QIcon(":/icons/win.icon.theme/32x32/actions/go-next.png"));
        }
        else
        {
            baseItem->setData(Qt::UserRole, 0);
        }
        table->setItem(row, BaseColumn, baseItem);

        // Sizes or heights (read-only)
        QTableWidgetItem *valueItem = new QTableWidgetItem(values.at(i));
        valueItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
        valueItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        valueItem->setData(Qt::UserRole, value);
        table->setItem(row, SizeColumn, valueItem);

        // Aliases (editable line edit)
        QLineEdit *lineEdit = new QLineEdit(table);
        lineEdit->setFrame(false); // Removes the default border/frame
        lineEdit->setText(aliases.value(value));
        lineEdit->setPlaceholderText(values.at(i));
        table->setCellWidget(row, AliasColumn, lineEdit);

        if (table->columnCount() == 4 )
        {
            QTableWidgetItem *colorItem = new QTableWidgetItem("");
            colorItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
            //colorItem->setBackground(QBrush(QColor("#000000"))); // Set initial color
            colorItem->setBackground(QBrush(QColor(colors.value(value)))); // Set initial color
            table->setItem(row, ColorColumn, colorItem);
        }
    }
}

void SizeRangeDialog::cellDoubleClicked(int row, int column)
{
    if (column != ColorColumn) return;

    QTableWidgetItem *item = ui->sizes_TableWidget->item(row, column);
    if (!item) return;

    // Get current color to show in dialog
    QColor currentColor = item->background().color();
    QColor newColor = QColorDialog::getColor(currentColor, this, "Select Color");

    if (newColor.isValid())
    {
        // Update the item's background
        item->setBackground(QBrush(newColor));
    }
    ui->sizes_TableWidget->clearSelection();
}

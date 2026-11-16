//-----------------------------------------------------------------------------
//  @file   piece_info_dialog.cpp
//  @author Coyak2
//  @date   15 Nov, 2026
//
//  @brief  Pattern Piece Info dialog: a table of the pattern pieces with their seamline and cutting line lengths,
//          which can be copied to the clipboard or exported as CSV.
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

#include "piece_info_dialog.h"
#include "ui_piece_info_dialog.h"

#include "../ifc/exception/vexception.h"
#include "../vmisc/def.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QHeaderView>
#include <QLocale>
#include <QTableWidgetItem>

namespace
{
enum Column
{
    ColumnName = 0,
    ColumnLetter,
    ColumnQuantity,
    ColumnSeamAllowance,
    ColumnSeamLine,
    ColumnCutLine,
    ColumnCount
};

//---------------------------------------------------------------------------------------------------------------------
/// @brief The NumericItem class sorts by the number stored in Qt::UserRole instead of the text, so "100.5" sorts after
///        "20.3". Cells without a number (-1) sort first.
///
/// @param text Text name of the QTableWidgetItem
/// @param value Data of the QTableWidgetItem
//---------------------------------------------------------------------------------------------------------------------
class NumericItem : public QTableWidgetItem
{
public:
    NumericItem(const QString &text, qreal value)
        : QTableWidgetItem(text)
    {
        setData(Qt::UserRole, value);
        setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }

    virtual bool operator<(const QTableWidgetItem &other) const override
    {
        return data(Qt::UserRole).toDouble() < other.data(Qt::UserRole).toDouble();
    }
};

//---------------------------------------------------------------------------------------------------------------------
/// @brief numberLocale set the locale number options to omit the group separtors.
//---------------------------------------------------------------------------------------------------------------------
QLocale numberLocale()
{
    // No thousands separators: the values are copied and exported to spreadsheets, which may not read them back.
    QLocale locale = qApp->Settings()->getOsSeparator() ? QLocale() : QLocale::c();
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    return locale;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief Creates a numeric table item displaying a length converted from pixels to the application's current unit.
///
/// @param pixels The length in pixels. Returns an empty item with a sort value of -1 if negative.
/// @return A pointer to the newly created NumericItem.
//---------------------------------------------------------------------------------------------------------------------
QTableWidgetItem *lengthItem(qreal pixels)
{
    if (pixels < 0)
    {
        return new NumericItem(QString(), -1);
    }
    const qreal value = qApp->fromPixel(pixels);
    return new NumericItem(numberLocale().toString(value, 'f', 2), value);
}
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
PieceInfoDialog::PieceInfoDialog(const VContainer *data, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PieceInfoDialog)
    , m_data(data)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() &~ Qt::WindowContextHelpButtonHint);

    fillTable();

    connect(ui->clipboard_ToolButton, &QToolButton::clicked, this, &PieceInfoDialog::copyToClipboard);
    connect(ui->csv_ToolButton,       &QToolButton::clicked, this, &PieceInfoDialog::exportToCsv);
}

//---------------------------------------------------------------------------------------------------------------------
PieceInfoDialog::~PieceInfoDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillTable fill table
//---------------------------------------------------------------------------------------------------------------------
void PieceInfoDialog::fillTable()
{
    const QString unit = UnitsToStr(qApp->patternUnit(), true);
    const QHash<quint32, VPiece> *pieces = m_data->DataPieces();

    QTableWidget *table = ui->pieces_TableWidget;
    QString seam_line_str     = tr("Seam Line");
    QString cut_line_str      = tr("Cut Line");
    QString length_around_str = tr("Length around the piece ");

    table->setSortingEnabled(false);
    table->setColumnCount(ColumnCount);
    table->setRowCount(pieces->size());
    table->setHorizontalHeaderLabels({tr("Name"), tr("Letter"), tr("Quantity"),
                                      tr("Seam Allowance (%1)").arg(unit),
                                      seam_line_str + QString("(%1)").arg(unit),
                                      cut_line_str  + QString("(%1)").arg(unit)});
    table->horizontalHeaderItem(ColumnSeamLine)->setToolTip(length_around_str + seam_line_str);
    table->horizontalHeaderItem(ColumnCutLine)->setToolTip(length_around_str + cut_line_str);

    int row = 0;
    for (auto i = pieces->constBegin(); i != pieces->constEnd(); ++i, ++row)
    {
        const VPiece &piece = i.value();
        const VPieceLabelData &label = piece.GetPatternPieceData();

        // Name
        table->setItem(row, ColumnName, new QTableWidgetItem(piece.GetName()));

        // Pattern letter
        QTableWidgetItem *letter_item = new QTableWidgetItem(label.GetLetter());
        letter_item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        table->setItem(row, ColumnLetter, letter_item);

        // Piece quantity
        QTableWidgetItem *quantity = new NumericItem(QString::number(label.GetQuantity()), label.GetQuantity());
        if (label.IsOnFold())
        {
            quantity->setToolTip(tr("Cut on fold"));
        }
        table->setItem(row, ColumnQuantity, quantity);

        // Seam allowance
        QTableWidgetItem *seam_allowance;
        if (piece.hasSeamAllowanceBuiltIn())
        {
            seam_allowance = new NumericItem(tr("Built in"), -1);
        }
        else if (piece.hasSeamAllowance())
        {
            const qreal sa_width = piece.GetSAWidth();
            seam_allowance = new NumericItem(numberLocale().toString(sa_width, 'f', 2), sa_width);
        }
        else
        {
            seam_allowance = new NumericItem(tr("None"), 0);
        }
        table->setItem(row, ColumnSeamAllowance, seam_allowance);

        // seam line length
        qreal seam_line = -1;

        QTableWidgetItem *seam_line_item;
        if (!piece.hasSeamAllowanceBuiltIn())
        {
            seam_line = piece.seamlineLength(m_data);
            seam_line_item = lengthItem(seam_line);
        }
        else
        {
            // With the seam allowance built in, the piece's outline is the cutting line and its seamline isn't drawn.
            seam_line_item = new NumericItem(QString("NA"), -1);
            seam_line_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        }
        table->setItem(row, ColumnSeamLine, seam_line_item);

        // Cut line length
        qreal cut_line  = -1;
        cut_line = piece.cutPathLength(m_data);
        table->setItem(row, ColumnCutLine, lengthItem(cut_line));
    }

    table->setSortingEnabled(true);
    table->sortByColumn(ColumnName, Qt::AscendingOrder);
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief headerTexts the column texts of the table header.
//---------------------------------------------------------------------------------------------------------------------
QStringList PieceInfoDialog::headerTexts() const
{
    QStringList texts;
    for (int column = 0; column < ui->pieces_TableWidget->columnCount(); ++column)
    {
        texts.append(ui->pieces_TableWidget->horizontalHeaderItem(column)->text());
    }
    return texts;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief rowTexts the texts of one row as shown (in the current sort order).
//---------------------------------------------------------------------------------------------------------------------
QStringList PieceInfoDialog::rowTexts(int row) const
{
    QStringList texts;
    for (int column = 0; column < ui->pieces_TableWidget->columnCount(); ++column)
    {
        const QTableWidgetItem *item = ui->pieces_TableWidget->item(row, column);
        texts.append(item != nullptr ? item->text() : QString());
    }
    return texts;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief copyToClipboard copy dialog selection to clipboard as comma separated values.
//---------------------------------------------------------------------------------------------------------------------
void PieceInfoDialog::copyToClipboard()
{
    QItemSelectionModel *model = ui->pieces_TableWidget->selectionModel();
    QModelIndexList selected_indexes = model->selectedIndexes();

    QString clipboard_string;

    for (int i = 0; i < selected_indexes.count(); ++i)
    {
        QModelIndex current = selected_indexes[i];
        QString display_text = current.data(Qt::DisplayRole).toString();

        // Check if another column exists beyond this one.
        if (i + 1 < selected_indexes.count())
        {
            QModelIndex next = selected_indexes[i+1];

            // If the column is on different row, the clipboard should take note.
            if (next.row() != current.row())
            {
                display_text.append("\n");
            }
            else
            {
                // Otherwise append a comma separator.
                display_text.append(" , ");
            }
        }
        clipboard_string.append(display_text);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(clipboard_string);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief exportToCsv saves the table as a CSV file, with the separator, encoding and header chosen as for exporting
/// measurements.
//---------------------------------------------------------------------------------------------------------------------
void PieceInfoDialog::exportToCsv()
{
    const QString suffix = QStringLiteral("csv");
    QString name = QFileInfo(qApp->getFilePath()).completeBaseName();
    if (name.isEmpty())
    {
        name = tr("pieces");
    }
    const QString path = QDir::homePath() + QLatin1Char('/') + name + QLatin1Char('.') + suffix;

    QString file_name = QFileDialog::getSaveFileName(this, tr("Export to CSV"), path,
                                                     tr("Comma-Separated Values") + QLatin1String(" (*.csv)"),
                                                     nullptr, qApp->Settings()->getUseNativeFileDialogs());
    if (file_name.isEmpty())
    {
        return;
    }
    if (QFileInfo(file_name).suffix().isEmpty())
    {
        file_name += QLatin1Char('.') + suffix;
    }

    DialogExportToCSV dialog(this);
    if (dialog.exec() != QDialog::Accepted)
    {
        return;
    }

    QxtCsvModel csv;
    const QStringList header = headerTexts();
    for (int column = 0; column < header.size(); ++column)
    {
        csv.insertColumn(column);
        if (dialog.WithHeader())
        {
            csv.setHeaderText(column, header.at(column));
        }
    }
    for (int row = 0; row < ui->pieces_TableWidget->rowCount(); ++row)
    {
        csv.insertRow(row);
        const QStringList texts = rowTexts(row);
        for (int column = 0; column < texts.size(); ++column)
        {
            csv.setText(row, column, texts.at(column));
        }
    }
    csv.toCSV(file_name, dialog.WithHeader(), dialog.Separator(), dialog.SelectedEncoding());

    qApp->Settings()->SetCSVSeparator(dialog.Separator());
    qApp->Settings()->SetCSVCodec(dialog.SelectedEncoding());
    qApp->Settings()->SetCSVWithHeader(dialog.WithHeader());
}

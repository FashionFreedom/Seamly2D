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
    ColumnSeamline,
    ColumnCuttingLine,
    ColumnCount
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief The NumericItem class sorts by the number stored in Qt::UserRole instead of the text, so "100.5" sorts after
 * "20.3". Cells without a number (-1) sort first.
 */
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
QLocale numberLocale()
{
    // No thousands separators: the values are copied and exported to spreadsheets, which may not read them back.
    QLocale locale = qApp->Settings()->getOsSeparator() ? QLocale() : QLocale::c();
    locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);
    return locale;
}

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
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

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
void PieceInfoDialog::fillTable()
{
    const QString unit = UnitsToStr(qApp->patternUnit(), true);
    const QHash<quint32, VPiece> *pieces = m_data->DataPieces();

    QTableWidget *table = ui->pieces_TableWidget;
    table->setSortingEnabled(false);
    table->setColumnCount(ColumnCount);
    table->setRowCount(pieces->size());
    table->setHorizontalHeaderLabels({tr("Name"), tr("Letter"), tr("Quantity"), tr("Seam allowance (%1)").arg(unit),
                                      tr("Seamline (%1)").arg(unit), tr("Cutting line (%1)").arg(unit)});
    table->horizontalHeaderItem(ColumnSeamline)->setToolTip(tr("Length round the piece's seamline"));
    table->horizontalHeaderItem(ColumnCuttingLine)->setToolTip(tr("Length round the piece's cutting line, "
                                                                  "including the seam allowance"));

    int row = 0;
    for (auto i = pieces->constBegin(); i != pieces->constEnd(); ++i, ++row)
    {
        const VPiece &piece = i.value();
        const VPieceLabelData &label = piece.GetPatternPieceData();

        table->setItem(row, ColumnName, new QTableWidgetItem(piece.GetName()));
        table->setItem(row, ColumnLetter, new QTableWidgetItem(label.GetLetter()));
        QTableWidgetItem *quantity = new NumericItem(QString::number(label.GetQuantity()), label.GetQuantity());
        if (label.IsOnFold())
        {
            quantity->setToolTip(tr("Cut on fold"));
        }
        table->setItem(row, ColumnQuantity, quantity);

        QTableWidgetItem *seamAllowance = nullptr;
        if (piece.hasSeamAllowanceBuiltIn())
        {
            seamAllowance = new NumericItem(tr("Built in"), -1);
        }
        else if (piece.hasSeamAllowance())
        {
            seamAllowance = new NumericItem(numberLocale().toString(piece.GetSAWidth(), 'f', 2), piece.GetSAWidth());
        }
        else
        {
            seamAllowance = new NumericItem(tr("None"), 0);
        }
        table->setItem(row, ColumnSeamAllowance, seamAllowance);

        qreal seamline = -1;
        qreal cuttingLine = -1;
        try
        {
            cuttingLine = piece.cutPathLength(m_data);
            // With the seam allowance built in, the piece's outline is the cutting line and its seamline isn't drawn.
            if (not piece.hasSeamAllowanceBuiltIn())
            {
                seamline = piece.seamlineLength(m_data);
            }
        }
        catch (const VException &)
        {
            // The piece's geometry isn't loaded (another draft block is active); leave the lengths blank.
        }
        QTableWidgetItem *seamlineItem = lengthItem(seamline);
        if (piece.hasSeamAllowanceBuiltIn())
        {
            seamlineItem->setToolTip(tr("Seam allowance built in: the outline is the cutting line"));
        }
        table->setItem(row, ColumnSeamline, seamlineItem);
        table->setItem(row, ColumnCuttingLine, lengthItem(cuttingLine));
    }

    table->setSortingEnabled(true);
    table->sortByColumn(ColumnName, Qt::AscendingOrder);
    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

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
/// @brief copyToClipboard copies the table as tab separated text, which spreadsheets paste as rows and columns.
void PieceInfoDialog::copyToClipboard()
{
    QStringList lines;
    lines.append(headerTexts().join(QLatin1Char('\t')));
    for (int row = 0; row < ui->pieces_TableWidget->rowCount(); ++row)
    {
        QStringList texts = rowTexts(row);
        for (QString &text : texts)
        {
            text.replace(QLatin1Char('\t'), QLatin1Char(' '));
            text.replace(QLatin1Char('\n'), QLatin1Char(' '));
        }
        lines.append(texts.join(QLatin1Char('\t')));
    }
    QApplication::clipboard()->setText(lines.join(QLatin1Char('\n')) + QLatin1Char('\n'));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief exportToCsv saves the table as a CSV file, with the separator, encoding and header chosen as for exporting
/// measurements.
void PieceInfoDialog::exportToCsv()
{
    const QString suffix = QStringLiteral("csv");
    QString name = QFileInfo(qApp->getFilePath()).completeBaseName();
    if (name.isEmpty())
    {
        name = tr("pieces");
    }
    const QString path = QDir::homePath() + QLatin1Char('/') + name + QLatin1Char('.') + suffix;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"), path,
                                                    tr("Comma-Separated Values") + QLatin1String(" (*.csv)"),
                                                    nullptr, qApp->Settings()->getUseNativeFileDialogs());
    if (fileName.isEmpty())
    {
        return;
    }
    if (QFileInfo(fileName).suffix().isEmpty())
    {
        fileName += QLatin1Char('.') + suffix;
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
    csv.toCSV(fileName, dialog.WithHeader(), dialog.Separator(), dialog.SelectedEncoding());

    qApp->Settings()->SetCSVSeparator(dialog.Separator());
    qApp->Settings()->SetCSVCodec(dialog.SelectedEncoding());
    qApp->Settings()->SetCSVWithHeader(dialog.WithHeader());
}

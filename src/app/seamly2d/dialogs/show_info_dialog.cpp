/***********************************************************************
 **
 **  @file   show_info_dialog.cpp
 **  @author DSCaskey <dscaskey@gmail.com>
 **  @date   3 Sep, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2022-2023 Seamly2D project
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

#include "show_info_dialog.h"
#include "ui_show_info_dialog.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFont>
#include <QGuiApplication>
#include <QImage>
#include <QPageLayout>
#include <QPixmap>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QScreen>
#include <QShowEvent>
#include <QString>
#include <QTextDocument>
#include <QtWidgets>

#include "../core/vapplication.h"
#include "../xml/vpattern.h"

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Constructor.
 *
 * Displays the xml pattern information in a dialog. Provides functionality to copy the
 * context to the clipboard, send it to the printer, or save as a PDF file.
 *
 * @param parent Parent object of the dialog.
 */
 ShowInfoDialog::ShowInfoDialog(VPattern *doc, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ShowInfoDialog)
    , doc(doc)
    , m_isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    qApp->Seamly2DSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QByteArray byteArray;
    byteArray.append(doc->GetImage().toUtf8());
    QString url = QString("<img src=\"data:image/png;base64, ") + byteArray + "\" width = 280 />";

    const QString info = tr("<table style=font-size:11pt; font-weight:600>"
                            "<tr><td align = right><b>Company:     </b></td><td>%1</td><tr>"
                            "<tr><td align = right><b>Customer:    </b></td><td>%2</td><tr>"
                            "<tr><td align = right><b>Pattern Name:</b></td><td>%3</td><tr>"
                            "<tr><td align = right><b>Pattern No:  </b></td><td>%4</td><tr>"
                            "<tr><td align = right><b>Version:     </b></td><td>%5</td><tr>"
                            "<tr><td align = right><b>Units:       </b></td><td>%6</td><tr>"
                            "<tr><td align = right><b>Measurements:</b></td><td>%7<br></td><tr>"
                            "<tr><td align = right><b>Description: </b></td><td>%8<br></td><tr>"
                            "<tr><td align = right><b>Notes:       </b></td><td>%9<br></td><tr>"
                            "<tr><td align = right><b>Image:       </b></td><td>%10</td><tr>"
                            "</table>")
                            .arg(doc->GetCompanyName())    //1
                            .arg(doc->GetCustomerName())   //2
                            .arg(doc->GetPatternName())    //3
                            .arg(doc->GetPatternNumber())  //4
                            .arg(doc->GetVersion())        //5
                            .arg(UnitsToStr(doc->measurementUnits())) //6
                            .arg(doc->MPath())             //7
                            .arg(doc->GetDescription())    //8
                            .arg(doc->GetNotes())          //9
                            .arg(url);                     //10

    ui->info_TextBrowser->setHtml(info);

    connect(ui->clipboard_ToolButton, &QToolButton::clicked, this, &ShowInfoDialog::copyToClipboard);
    connect(ui->printer_ToolButton,   &QToolButton::clicked, this, &ShowInfoDialog::sendToPrinter);
    connect(ui->pdf_ToolButton,       &QToolButton::clicked, this, &ShowInfoDialog::exportPdf);
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * Destructor
 *
*/
ShowInfoDialog::~ShowInfoDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Reimplement QWidget showEvent.
 *
*/
//---------------------------------------------------------------------------------------------------------------------
void ShowInfoDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }

    setMaximumSize(size());
    setMinimumSize(size());

    m_isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Copies the text content of the text browser widget to the clipboard.
 */
void ShowInfoDialog::copyToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->info_TextBrowser->toPlainText());

    ui->info_TextBrowser->selectAll();
    ui->info_TextBrowser->copy();
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Sends content of the text browser widget to the printer.
 */
void ShowInfoDialog::sendToPrinter()
{
    QPrinter  printer;
    QPrintDialog printDialog(&printer);
    if(printDialog.exec())
    {
        QTextDocument textDocument;
        textDocument.setHtml(ui->info_TextBrowser->toHtml());
        textDocument.print(&printer);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Exports the content of the text browser widget as a pdf.
 */
void ShowInfoDialog::exportPdf()
{
    QString filters(tr("Info files") + QLatin1String("(*.pdf)"));
    QString filename = QFileDialog::getSaveFileName(this, tr("Export PDF"),
                                                    doc->GetPatternName() + tr("_info") + QLatin1String(".pdf"),
                                                    filters, nullptr, QFileDialog::DontUseNativeDialog);

    if (QFileInfo(filename).suffix().isEmpty())
    {
        filename.append(".pdf");
    }
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(printer.pageLayout().pageSize());
    printer.setOutputFileName(filename);

    QTextDocument textDocument;
    textDocument.setHtml(ui->info_TextBrowser->toHtml());
    textDocument.setPageSize(printer.pageLayout().paintRectPixels(printer.resolution()).size());
    textDocument.print(&printer);
}

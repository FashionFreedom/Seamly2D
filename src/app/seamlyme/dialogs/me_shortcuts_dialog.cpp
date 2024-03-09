/************************************************************************
 **
 **  @file   me_shortcuts_dialog.h
 **  @author DSCaskey <dscaskey@gmail.com>
 **  @date   21 Oct, 2023
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

#include "me_shortcuts_dialog.h"
#include "ui_me_shortcuts_dialog.h"

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFont>
#include <QGuiApplication>
#include <QTextDocument>
#include <QPageLayout>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QScreen>
#include <QShowEvent>
#include <QString>
#include <QtWidgets>

#include "../mapplication.h"

//---------------------------------------------------------------------------------------------------------------------
MeShortcutsDialog::MeShortcutsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MeShortcutsDialog)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    const QString html = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%22        </td><td>%23</td></tr>"
                                    "<tr><td width = 50%>%24        </td><td>%25<br></td></tr>"
                                    "<tr><td width = 50%><b>%2</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%3        </td><td>%4</td></tr>"
                                    "<tr><td width = 50%>%5        </td><td>%6</td></tr>"
                                    "<tr><td width = 50%>%7        </td><td>%8</td></tr>"
                                    "<tr><td width = 50%>%9        </td><td>%10</td></tr>"
                                    "<tr><td width = 50%>%11       </td><td>%12</td></tr>"
                                    "<tr><td width = 50%>%13       </td><td>%14</td></tr>"
                                    "<tr><td width = 50%>%15       </td><td>%16</td></tr>"
                                    "<tr><td width = 50%>%17	   </td><td>%18<br></td></tr>"
                                    "<tr><td width = 50%><b>%19</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%20       </td><td>%21</td></tr>"
                                "</table>")
                                .arg(tr("Keyboard Shortcuts"))
                                .arg(tr("File"))
                                .arg(tr("New")).arg(tr("Ctrl+N"))
                                .arg(tr("Open Individual")).arg(tr("Ctrl+O"))
                                .arg(tr("Open Multisize")).arg(tr("Ctrl+Shift+O"))
                                .arg(tr("Print")).arg(tr("Ctrl+P"))
                                .arg(tr("Save")).arg(tr("Ctrl+S"))
                                .arg(tr("Save as")).arg(tr("Ctrl+Shift+S"))
                                .arg(tr("Export to CSV")).arg(tr("Ctrl+E"))
                                .arg(tr("Exit")).arg(tr("Ctrl+Q"))
                                .arg(tr("Help"))
                                .arg(tr("Keyboard Shortcuts")).arg(tr("K"))
                                .arg(tr("Find previous")).arg(tr("Ctrl+Shift+G"))
                                .arg(tr("Find next")).arg(tr("Ctrl+G"));

    ui->shortcuts_TextBrowser->setHtml(html);

    connect(ui->clipboard_ToolButton, &QToolButton::clicked, this, &MeShortcutsDialog::copyToClipboard);
    connect(ui->printer_ToolButton,   &QToolButton::clicked, this, &MeShortcutsDialog::sendToPrinter);
    connect(ui->pdf_ToolButton,       &QToolButton::clicked, this, &MeShortcutsDialog::exportPdf);
}

//---------------------------------------------------------------------------------------------------------------------
MeShortcutsDialog::~MeShortcutsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void MeShortcutsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
}

void MeShortcutsDialog::copyToClipboard()
{
    //QClipboard *clipboard = QApplication::clipboard();
    //clipboard->setText(ui->shortcuts_TextBrowser->toPlainText());

    ui->shortcuts_TextBrowser->selectAll();
    ui->shortcuts_TextBrowser->copy();
}
void MeShortcutsDialog::sendToPrinter()
{
    QPrinter  printer;
    QPrintDialog printDialog(&printer);
    if(printDialog.exec())
    {
        QTextDocument textDocument;
        textDocument.setHtml(ui->shortcuts_TextBrowser->toHtml());
        textDocument.print(&printer);
    }
}

void MeShortcutsDialog::exportPdf()
{
    QString filename = QFileDialog::getSaveFileName(nullptr, tr("Export PDF"), QString(),
                                                    "*.pdf", nullptr, QFileDialog::DontUseNativeDialog);

    if (QFileInfo(filename).suffix().isEmpty())
    {
        filename.append(".pdf");
    }
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(printer.pageLayout().pageSize());
    printer.setOutputFileName(filename);

    QTextDocument textDocument;
    textDocument.setHtml(ui->shortcuts_TextBrowser->toHtml());
    textDocument.setPageSize(printer.pageLayout().paintRectPixels(printer.resolution()).size());
    textDocument.print(&printer);
}

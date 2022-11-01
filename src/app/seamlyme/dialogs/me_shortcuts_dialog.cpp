/************************************************************************
 **
 **  @file   me_shortcuts_dialog.h
 **  @author DSCaskey <dscaskey@gmail.com>
 **  @date   5 14, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  All Rights Reserved.
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
#include <QTextDocument>
#include <QPageLayout>
#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
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

//---------------------------------------------------------------------------------------------------------------------
void MeShortcutsDialog::setFontPointSize(QWidget *w, int pointSize)
{
    SCASSERT(w != nullptr)

    QFont font = w->font();
    font.setPointSize(pointSize);
    w->setFont(font);
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

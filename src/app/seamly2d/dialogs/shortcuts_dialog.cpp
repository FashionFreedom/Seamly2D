/************************************************************************
 **
 **  @file   shortcuts_dialog.h
 **  @author DSCaskey <dscaskey@gmail.com>
**  @date   21 Oct, 2023
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

#include "shortcuts_dialog.h"
#include "ui_shortcuts_dialog.h"

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

#include "../core/vapplication.h"

//---------------------------------------------------------------------------------------------------------------------
ShortcutsDialog::ShortcutsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ShortcutsDialog)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const QString file = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%><b>%2</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%3       </td><td>%4</td></tr>"
                                    "<tr><td width = 50%>%5       </td><td>%6</td></tr>"
                                    "<tr><td width = 50%>%7       </td><td>%8</td></tr>"
                                    "<tr><td width = 50%>%9       </td><td>%10</td></tr>"
                                    "<tr><td width = 50%>%11      </td><td>%12</td></tr>"
                                    "<tr><td width = 50%>%13      </td><td>%14</td></tr>"
                                    "<tr><td width = 50%>%15      </td><td>%16</td></tr>"
                                    "<tr><td width = 50%>%17	  </td><td>%18</td></tr>"
                                    "<tr><td width = 50%>%19      </td><td>%20<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Keyboard Shortcuts"))                              //1
                                 .arg(tr("File"))                                            //2
                                 .arg(tr("New")).arg(tr("Ctrl+N"))                           //3 & 4
                                 .arg(tr("Open")).arg(tr("Ctrl+O"))                          //5 & 6
                                 .arg(tr("Close")).arg(tr("Ctrl+W"))                         //7 & 8
                                 .arg(tr("Save")).arg(tr("Ctrl+S"))                          //9 & 10
                                 .arg(tr("Save as")).arg(tr("Ctrl+Shift+S"))                 //11 &12
                                 .arg(tr("Print")).arg(tr("Ctrl+P"))                         //13 & 14
                                 .arg(tr("Pattern Preferences")).arg(tr("Ctrl+Shift+Comma")) //15 & 16
                                 .arg(tr("Document Information")).arg(tr("Ctrl+I"))          //17 & 18
                                 .arg(tr("Exit")).arg(tr("Ctrl+Q"));                         //19 & 20

    const QString edit = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td></td>%3</tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Edit"))                                            //1
                                 .arg(tr("Undo")).arg(tr("Ctrl+Z"))                          //2 & 3
                                 .arg(tr("Redo")).arg(tr("Ctrl+Y"));                         //4 & 5

    const QString view = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8       </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10       </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12       </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14       </td><td>%15</td></tr>"
                                    "<tr><td width = 50%>%16       </td><td>%17</td></tr>"
                                    "<tr><td width = 50%>%18       </td><td>%19</td></tr>"
                                    "<tr><td width = 50%>%20       </td><td>%21</td></tr>"
                                    "<tr><td width = 50%>%22       </td><td>%23</td></tr>"
                                    "<tr><td width = 50%>%24       </td><td>%25</td></tr>"
                                    "<tr><td width = 50%>%26       </td><td>%27</td></tr>"
                                    "<tr><td width = 50%>%28       </td><td>%29</td></tr>"
                                    "<tr><td width = 50%>%30       </td><td>%31</td></tr>"
                                    "<tr><td width = 50%>%32       </td><td>%33</td></tr>"
                                    "<tr><td width = 50%>%34       </td><td>%35</td></tr>"
                                    "<tr><td width = 50%>%36       </td><td>%37</td></tr>"
                                    "<tr><td width = 50%>%38       </td><td>%39</td></tr>"
                                    "<tr><td width = 50%>%40       </td><td>%41</td></tr>"
                                    "<tr><td width = 50%>%42       </td><td>%43</td></tr>"
                                    "<tr><td width = 50%>%44       </td><td>%45<br></td></tr>"
                                 "</table>")
                                 .arg(tr("View"))                                            //1
                                 .arg(tr("Draft Mode")).arg(tr("Shift+D"))                   //2 & 3
                                 .arg(tr("Piece Mode")).arg(tr("Shift+P"))                   //4 & 5
                                 .arg(tr("Layout Mode")).arg(tr("Shift+L"))                  //6 & 7
                                 .arg(tr("Zoom In")).arg(tr("Ctrl++"))                       //8 & 9
                                 .arg(tr("Zoom Out")).arg(tr("Ctrl+-"))                      //10 & 11
                                 .arg(tr("Zoom 1:1")).arg(tr("Ctrl+0"))                      //12 & 13
                                 .arg(tr("Zoom to Point")).arg(tr("Ctrl+Alt+P"))             //14 & 15
                                 .arg(tr("Fit All")).arg(tr("Ctrl+="))                       //16 & 17
                                 .arg(tr("Previous")).arg(tr("Ctrl+Left"))                   //18 & 19
                                 .arg(tr("Selected")).arg(tr("Ctrl+Right"))                  //20 & 21
                                 .arg(tr("Area")).arg(tr("Ctrl+A"))                          //22 & 23
                                 .arg(tr("Pan")).arg(tr("Z, P"))                             //24 & 25
                                 .arg(tr("Show Name Text")).arg(tr("V, P"))                  //26 & 27
                                 .arg(tr("Increase Text Size")).arg(tr("Ctrl+]"))            //28 & 29
                                 .arg(tr("Decrease Text Size")).arg(tr("Ctrl+["))            //30 & 31
                                 .arg(tr("Use Tool Color")).arg(tr("T"))                     //32 & 33
                                 .arg(tr("Wireframe")).arg(tr("V, W"))                       //34 & 35
                                 .arg(tr("Curve Control Points")).arg(tr("V, C"))            //36 & 37
                                 .arg(tr("Axis Origin")).arg(tr("V, A"))                     //38 & 39
                                 .arg(tr("Seam Allowance")).arg(tr("V, S"))                  //40 & 41
                                 .arg(tr("Grainlines")).arg(tr("V, G"))                      //42 & 43
                                 .arg(tr("Labels")).arg(tr("V, L"));                         //44 & 45

    const QString measurements = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2        </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4        </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6        </td><td>%7<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Measurements"))                                    //1
                                 .arg(tr("Open SeamlyMe")).arg(tr("Ctrl+M"))                 //2 & 3
                                 .arg(tr("Variables Table")).arg(tr("Ctrl+T"))               //4 & 5
                                 .arg(tr("Export Variables Table  to CSV")).arg(tr("Ctrl+E"));//6 & 7

    const QString tools = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2        </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4        </td><td>%5<br></td></tr>"
                                  "</table>")
                                   .arg(tr("Tools"))                                        //1
                                   .arg(tr("New Draft Block")).arg(tr("Ctrl+Shift+N"))      //2 & 3
                                   .arg(tr("Rename Draft Block")).arg(tr("F2"));            //4 & 5

    const QString points = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2        </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4        </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6        </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8        </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10        </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12        </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14        </td><td>%15</td></tr>"
                                    "<tr><td width = 50%>%16        </td><td>%17</td></tr>"
                                    "<tr><td width = 50%>%18       </td><td>%19</td></tr>"
                                    "<tr><td width = 50%>%20       </td><td>%21</td></tr>"
                                    "<tr><td width = 50%>%22       </td><td>%23<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Point"))                                           //1
                                 .arg(tr("Length and Angle")).arg(tr("L, A"))                //2 & 3
                                 .arg(tr("On Line")).arg(tr("O, L"))                         //4 & 5
                                 .arg(tr("On Perpendicular")).arg(tr("O, P"))                //6 & 7
                                 .arg(tr("On Bisector")).arg(tr("O, B"))                     //8 & 9
                                 .arg(tr("Length to Line")).arg(tr("P, S"))                  //10 & 11
                                 .arg(tr("Intersect Arc and Line")).arg(tr("A, L"))          //12 & 13
                                 .arg(tr("Intersect Axis and Triangle")).arg(tr("X, T"))     //14 & 15
                                 .arg(tr("Intersect XY")).arg(tr("X, Y"))                    //16 & 17
                                 .arg(tr("Intersect Line and Perpendicular")).arg(tr("L, P"))//18 & 19
                                 .arg(tr("Intersect Line and Axis")).arg(tr("L, X"))         //20 & 21
                                 .arg(tr("Midpoint On Line")).arg(tr("Shift+O, Shift+L"));   //22 & 23


    const QString line = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Line"))                                            //1
                                 .arg(tr("Line")).arg(tr("Alt+L"))                           //2 & 3
                                 .arg(tr("Point - Intersect Lines")).arg(tr("I, L"));        //4 & 5

    const QString curves = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8       </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10       </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12       </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14       </td><td>%15</td></tr>"
                                    "<tr><td width = 50%>%16       </td><td>%17<br></td></tr>"
                                  "</table>")
                                 .arg(tr("Curves"))                                          //1
                                 .arg(tr("Curve - Interactive")).arg(tr("Alt+C"))            //2 &3
                                 .arg(tr("Spline - Interactive")).arg(tr("Alt+S"))           //4 & 5
                                 .arg(tr("Curve - Fixed")).arg(tr("Alt+Shift+C"))            //6 & 7
                                 .arg(tr("Spline - Fixed")).arg(tr("Alt+Shift+S"))           //8 & 9
                                 .arg(tr("Point - On Curve")).arg(tr("O, C"))                //10 & 11
                                 .arg(tr("Point - On Spline	")).arg(tr("O, S"))              //12 & 13
                                 .arg(tr("Point - Intersect Curves")).arg(tr("I, C"))        //14 & 15
                                 .arg(tr("Point - Intersect Curve and Axis")).arg(tr("C, X"));//16 & 17


    const QString arcs = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8       </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10       </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12       </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14       </td><td>%15</td></tr>"
                                    "<tr><td width = 50%>%16       </td><td>%17</td></tr>"
                                    "<tr><td width = 50%>%18       </td><td>%19<br></td></tr>"
                                  "</table>")
                                  .arg(tr("Arcs"))                                            //1
                                  .arg(tr("Arc - Radius and Angle")).arg(tr("Alt+A"))         //2 & 3
                                  .arg(tr("Arc - Radius and Length")).arg(tr("Alt+Shift+A"))  //4 & 5
                                  .arg(tr("Point - On Arc")).arg(tr("O, A"))                  //6 & 7
                                  .arg(tr("Point - Intersect Arc and Axis")).arg(tr("A, X"))  //8 & 9
                                  .arg(tr("Point - Intersect Arcs")).arg(tr("I, A"))          //10 & 11
                                  .arg(tr("Point - Intersect Circles")).arg(tr("Shift+I, Shift+C"))//12 & 13
                                  .arg(tr("Point - Intersect Circle and Tangent")).arg(tr("C, T")) //14 & 15
                                  .arg(tr("Point - Intersect Arc and Tangent")).arg(tr("A, T"))    //16 & 17
                                  .arg(tr("Elliptical Arc")).arg(tr("Alt+E"));                 //18 & 19


    const QString operations = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8       </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10       </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12       </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14       </td><td>%15<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Operations"))                                      //1
                                 .arg(tr("Add Objects to Group")).arg(tr("G"))               //2 & 3
                                 .arg(tr("Rotation")).arg(tr("R"))                           //4 & 5
                                 .arg(tr("Mirror by Line")).arg(tr("M, L"))                  //6 & 7
                                 .arg(tr("Mirror by Axis")).arg(tr("M, A"))                  //8 & 9
                                 .arg(tr("Move")).arg(tr("Alt+M"))                           //10 & 11
                                 .arg(tr("True Darts")).arg(tr("T, D"))                      //12 & 13
                                 .arg(tr("Export Draft Blocks")).arg(tr("E, D"));            //14 & 15

    const QString pattern = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7</td></tr>"
                                    "<tr><td width = 50%>%8       </td><td>%9</td></tr>"
                                    "<tr><td width = 50%>%10       </td><td>%11</td></tr>"
                                    "<tr><td width = 50%>%12       </td><td>%13</td></tr>"
                                    "<tr><td width = 50%>%14       </td><td>%15</td></tr>"
                                    "<tr><td width = 50%>%16       </td><td>%17</td></tr>"
                                    "<tr><td width = 50%>%18       </td><td>%19</td></tr>"
                                    "<tr><td width = 50%>%20       </td><td>%21</td></tr>"
                                    "<tr><td width = 50%>%22       </td><td>%23</td></tr>"
                                    "<tr><td width = 50%>%24       </td><td>%25<br></td></tr>"
                                  "</table>")
                                  .arg(tr("Pattern Piece"))                                   //1
                                  .arg(tr("New Pattern Piece")).arg(tr("N, P"))               //2 & 3
                                  .arg(tr("Anchor Point")).arg(tr("A, P"))                    //4 & 5
                                  .arg(tr("Internal Path")).arg(tr("I, P"))                   //6 & 7
                                  .arg(tr("Insert Nodes")).arg(tr("I, N"))                    //8 & 9
                                  .arg(tr("Edit Properties")).arg(tr("P"))                    //10 & 11
                                  .arg(tr("Toggle Lock")).arg(tr("Ctrl+L"))                   //12 & 13
                                  .arg(tr("Include in Layout")).arg(tr("I"))                  //14 & 15
                                  .arg(tr("Forbid Flipping")).arg(tr("F"))                    //16 & 17
                                  .arg(tr("Raise To Top")).arg(tr("Ctrl+Home"))               //18 & 19
                                  .arg(tr("Lower To Bottom")).arg(tr("Ctrl+End"))             //20 & 21
                                  .arg(tr("Rename")).arg(tr("F2"))                            //21 & 23
                                  .arg(tr("Delete")).arg(tr("Del"));                          //24 & 25

    const QString details = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Details"))                                         //1
                                 .arg(tr("Unite Pieces")).arg(tr("U"))                       //2 & 3
                                 .arg(tr("Export Pieces")).arg(tr("E, P"));                  //4 & 5

    const QString layout = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5</td></tr>"
                                    "<tr><td width = 50%>%6       </td><td>%7<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Layout"))                                          //1
                                 .arg(tr("New Layout")).arg(tr("N, L"))                      //2 & 3
                                 .arg(tr("Export Layout")).arg(tr("E, L"))                   //4 & 5
                                 .arg(tr("Last Tool")).arg(tr("Ctrl+Shift+L"));              //6 & 7


    const QString history = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3<br></td></tr>"
                                 "</table>")
                                 .arg(tr("History"))                                         //1
                                 .arg(tr("History")).arg(tr("Ctrl+H"));                      //2 & 3


    const QString utilities = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3</td></tr>"
                                    "<tr><td width = 50%>%4       </td><td>%5<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Utilities"))                                       //1
                                 .arg(tr("Calculator")).arg(tr("Ctrl+Shift+C"))              //2 & 3
                                 .arg(tr("Decimal Chart")).arg(tr("Ctrl+Shift+D"));          //4 & 5

    const QString help = QString("<table style=font-size:11pt; font-weight:600>"
                                    "<tr><td width = 50%><b>%1</b></td><td></td></tr>"
                                    "<tr><td width = 50%>%2       </td><td>%3<br></td></tr>"
                                 "</table>")
                                 .arg(tr("Help"))                                            //1
                                 .arg(tr("Keyboard Shortcuts")).arg(tr("K"));                //2 & 3

    ui->shortcuts_TextBrowser->setHtml(file + edit + view + measurements + tools + points + line + curves +
                                       arcs + operations + pattern + details + layout + history + utilities + help);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    connect(ui->clipboard_ToolButton, &QToolButton::clicked, this, &ShortcutsDialog::copyToClipboard);
    connect(ui->printer_ToolButton,   &QToolButton::clicked, this, &ShortcutsDialog::sendToPrinter);
    connect(ui->pdf_ToolButton,       &QToolButton::clicked, this, &ShortcutsDialog::exportPdf);
}

//---------------------------------------------------------------------------------------------------------------------
ShortcutsDialog::~ShortcutsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void ShortcutsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
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

void ShortcutsDialog::copyToClipboard()
{
    ui->shortcuts_TextBrowser->selectAll();
    ui->shortcuts_TextBrowser->copy();
}
void ShortcutsDialog::sendToPrinter()
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

void ShortcutsDialog::exportPdf()
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

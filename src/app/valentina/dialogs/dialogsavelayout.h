/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/valentina-project/vpo2                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialogsavelayout.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://github.com/valentina-project/vpo2> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef DIALOGSAVELAYOUT_H
#define DIALOGSAVELAYOUT_H

#include <QDialog>
#include <QMarginsF>
#include "../vgeometry/vgeometrydef.h"
#include "vabstractlayoutdialog.h"

#ifdef Q_OS_WIN
#   define PDFTOPS "pdftops.exe"
#else
#   define PDFTOPS "pdftops"
#endif

namespace Ui
{
    class DialogSaveLAyout;
}

enum class LayoutExportFormats : char
{
    SVG = 0,
    PDF = 1,
    PDFTiled = 2,
    PNG = 3,
    OBJ = 4,              /* Wavefront OBJ*/
    PS  = 5,
    EPS = 6,
    DXF_AC1006_Flat = 7,  /* R10. */
    DXF_AC1009_Flat = 8,  /* R11 & R12. */
    DXF_AC1012_Flat = 9,  /* R13. */
    DXF_AC1014_Flat = 10,  /* R14. */
    DXF_AC1015_Flat = 11, /* ACAD 2000. */
    DXF_AC1018_Flat = 12, /* ACAD 2004. */
    DXF_AC1021_Flat = 13, /* ACAD 2007. */
    DXF_AC1024_Flat = 14, /* ACAD 2010. */
    DXF_AC1027_Flat = 15, /* ACAD 2013. */
    DXF_AC1006_AAMA = 16, /* R10. */
    DXF_AC1009_AAMA = 17, /* R11 & R12. */
    DXF_AC1012_AAMA = 18, /* R13. */
    DXF_AC1014_AAMA = 19, /* R14. */
    DXF_AC1015_AAMA = 20, /* ACAD 2000. */
    DXF_AC1018_AAMA = 21, /* ACAD 2004. */
    DXF_AC1021_AAMA = 22, /* ACAD 2007. */
    DXF_AC1024_AAMA = 23, /* ACAD 2010. */
    DXF_AC1027_AAMA = 24, /* ACAD 2013. */
    DXF_AC1006_ASTM = 25, /* R10. */
    DXF_AC1009_ASTM = 26, /* R11 & R12. */
    DXF_AC1012_ASTM = 27, /* R13. */
    DXF_AC1014_ASTM = 28, /* R14. */
    DXF_AC1015_ASTM = 29, /* ACAD 2000. */
    DXF_AC1018_ASTM = 30, /* ACAD 2004. */
    DXF_AC1021_ASTM = 31, /* ACAD 2007. */
    DXF_AC1024_ASTM = 32, /* ACAD 2010. */
    DXF_AC1027_ASTM = 33, /* ACAD 2013. */
    COUNT                 /*Use only for validation*/
};

class DialogSaveLayout : public  VAbstractLayoutDialog
{
    Q_OBJECT

public:
    explicit DialogSaveLayout(int count, Draw mode = Draw::Layout, const QString &fileName = QString(),
                              QWidget *parent = nullptr);
    virtual ~DialogSaveLayout();

    QString Path() const;
    QString FileName() const;

    LayoutExportFormats Format() const;
    void                SelectFormat(LayoutExportFormats format);

    void SetBinaryDXFFormat(bool binary);
    bool IsBinaryDXFFormat() const;

    static QString MakeHelpFormatList();
    void   SetDestinationPath(const QString& cmdDestinationPath);

    Draw Mode() const;

    static QString ExportFormatDescription(LayoutExportFormats format);
    static QString ExportFromatSuffix(LayoutExportFormats format);

    bool IsTextAsPaths() const;
    void SetTextAsPaths(bool textAsPaths);

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void InitTemplates(QComboBox *comboBoxTemplates);

private slots:
    void Save();
    void PathChanged(const QString &text);
    void ShowExample();


private:
    Q_DISABLE_COPY(DialogSaveLayout)
    Ui::DialogSaveLAyout *ui;
    int count;
    bool isInitialized;
    Draw m_mode;

    static bool havePdf;
    static bool tested;
    static bool SupportPSTest();
    static bool TestPdf();
    static QVector<std::pair<QString, LayoutExportFormats> > InitFormats();

    void RemoveFormatFromList(LayoutExportFormats format);

    void ReadSettings();
    void WriteSettings() const;
};

#endif // DIALOGSAVELAYOUT_H

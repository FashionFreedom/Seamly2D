///---------------------------------------------------------------------------------------------------------------------
/// @file   export_format_combobox.cpp
/// @author DS Caskey
/// @date   @date 11 Nov, 2026
///
/// @brief
/// @copyright
/// This source code is part of the Seamly2D project, a pattern making
/// program, whose allow create and modeling patterns of clothing.
/// Copyright (C) 2013-2026 Seamly2D project
/// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
///
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
//
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include <QAbstractItemView>
#include <QList>
#include <QPen>
#include <Qt>
#include <QPainter>
#include <QPixmap>
#include <QProcess>
#include <QDebug>
#include <QVariant>

#include "export_format_combobox.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

#ifdef Q_OS_WIN
#   define PDFTOPS "pdftops.exe"
#else
#   define PDFTOPS "pdftops"
#endif

bool ExportFormatCombobox::have_pdf = false;
bool ExportFormatCombobox::tested   = false;

//---------------------------------------------------------------------------------------------------------------------
/// Constructor with name.
//---------------------------------------------------------------------------------------------------------------------
ExportFormatCombobox::ExportFormatCombobox(QWidget *parent, const char *name)
	: QComboBox(parent)
	, m_current_format_type()
    , m_formats()
{
	setObjectName(name);
	setEditable ( false );
	init();
}

//---------------------------------------------------------------------------------------------------------------------
/// Destructor
//---------------------------------------------------------------------------------------------------------------------
ExportFormatCombobox::~ExportFormatCombobox() {}

//---------------------------------------------------------------------------------------------------------------------
/// Initialisation called from constructor or manually but only once.
//---------------------------------------------------------------------------------------------------------------------
void ExportFormatCombobox::init()
{
    this->blockSignals(true);

    initFormats();
    for (ExportFormat &format : m_formats)
    {
        addItem(format.translation, QVariant(static_cast<int>(format.type)));
    }
    setMaxVisibleItems(15);
    this->view()->setTextElideMode(Qt::ElideNone);

    this->blockSignals(false);
	connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExportFormatCombobox::updateExportFormat);

    setCurrentToDefault();
    updateExportFormat(currentIndex());
}

LayoutExportFormat ExportFormatCombobox::getExportFormat() const
{
    return m_current_format_type;
}

//---------------------------------------------------------------------------------------------------------------------
/// Sets the currently selected format item to the given format.
//---------------------------------------------------------------------------------------------------------------------
void ExportFormatCombobox::setExportFormat(LayoutExportFormat &type)
{
    m_current_format_type = type;

    setCurrentIndex(findData(QVariant(static_cast<int>(type))));

    if (currentIndex()!= count() -1 )
    {
        updateExportFormat(currentIndex());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QList<LayoutExportFormat> ExportFormatCombobox::initFormatTypes()
{
    QList<LayoutExportFormat> types =
    {

        LayoutExportFormat::SVG,
        LayoutExportFormat::PDF,
        LayoutExportFormat::PDFTiled,
        LayoutExportFormat::PNG,
        LayoutExportFormat::JPG,
        LayoutExportFormat::BMP,
        LayoutExportFormat::TIF,
        LayoutExportFormat::PPM,
        LayoutExportFormat::OBJ
    };

    if (supportPSTest())
    {
        types +=
        {
            LayoutExportFormat::PS,
            LayoutExportFormat::EPS
        };
    }

    types +=
    {
        LayoutExportFormat::DXF_AC1006_Flat,
        LayoutExportFormat::DXF_AC1009_Flat,
        LayoutExportFormat::DXF_AC1012_Flat,
        LayoutExportFormat::DXF_AC1014_Flat,
        LayoutExportFormat::DXF_AC1015_Flat,
        LayoutExportFormat::DXF_AC1018_Flat,
        LayoutExportFormat::DXF_AC1021_Flat,
        LayoutExportFormat::DXF_AC1024_Flat,
        LayoutExportFormat::DXF_AC1027_Flat,
        LayoutExportFormat::DXF_AC1006_AAMA,
        LayoutExportFormat::DXF_AC1009_AAMA,
        LayoutExportFormat::DXF_AC1012_AAMA,
        LayoutExportFormat::DXF_AC1014_AAMA,
        LayoutExportFormat::DXF_AC1015_AAMA,
        LayoutExportFormat::DXF_AC1018_AAMA,
        LayoutExportFormat::DXF_AC1021_AAMA,
        LayoutExportFormat::DXF_AC1024_AAMA,
        LayoutExportFormat::DXF_AC1027_AAMA
    };

    return types;
}

//---------------------------------------------------------------------------------------------------------------------
void ExportFormatCombobox::initFormats()
{
    QList<LayoutExportFormat> types = initFormatTypes();
    for (LayoutExportFormat type : types)
    {
        ExportFormat format;
        format.source      = exportFormatDescription(type, true);
        format.translation = exportFormatDescription(type, false);
        format.type        = type;
        m_formats.append(format);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportFormatCombobox::supportPSTest()
{
    if (!tested)
    {
        have_pdf = testPdf();
        tested = true;
    }
    return have_pdf;
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportFormatCombobox::testPdf()
{
    bool res = false;

    QProcess proc;
    QStringList args;

#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    // Seek pdftops in app bundle or near seamaly2d.exe
    proc.start(qApp->applicationDirPath() + QLatin1String("/")+ PDFTOPS, QStringList());
#else
    proc.start(PDFTOPS, QStringList()); // Seek pdftops in standard path
#endif

    if (proc.waitForStarted(15000) && (proc.waitForFinished(15000) || proc.state() == QProcess::NotRunning))
    {
        res = true;
    }
    else
    {
        qWarning() << PDFTOPS << "error" << proc.error() << proc.errorString();
    }
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportFormatCombobox::exportFormatDescription(LayoutExportFormat type, bool is_source)
{
    const QString dxf_suffix = QStringLiteral("(*.dxf)");
    QString dxf_flat_files_str = tr("(flat) files");
    QString files_str = tr("files");

    if (is_source)
    {
        dxf_flat_files_str = QString("(flat) files");
        files_str = QString("files");
    }

    switch(type)
    {
        case LayoutExportFormat::SVG:
            return QString("SVG %1 (*.svg)").arg(files_str);
        case LayoutExportFormat::PDF:
            return QString("PDF %1 (*.pdf)").arg(files_str);
        case LayoutExportFormat::PNG:
            return QString("PNG %1 (*.png)").arg(files_str);
        case LayoutExportFormat::JPG:
            return QString("JPG %1 (*.jpg)").arg(files_str);
        case LayoutExportFormat::BMP:
            return QString("BMP %1 (*.bmp)").arg(files_str);
        case LayoutExportFormat::TIF:
            return QString("TIF %1 (*.tif)").arg(files_str);
        case LayoutExportFormat::PPM:
            return QString("PPM %1 (*.ppm)").arg(files_str);
        case LayoutExportFormat::OBJ:
            return "Wavefront OBJ (*.obj)";
        case LayoutExportFormat::PS:
            return QString("PS %1 (*.ps)").arg(files_str);
        case LayoutExportFormat::EPS:
            return QString("EPS %1 (*.eps)").arg(files_str);
        case LayoutExportFormat::DXF_AC1006_Flat:
            return QString("AutoCAD DXF R10 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1009_Flat:
            return QString("AutoCAD DXF R11/12 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1012_Flat:
            return QString("AutoCAD DXF R13 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1014_Flat:
            return QString("AutoCAD DXF R14 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1015_Flat:
            return QString("AutoCAD DXF 2000 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1018_Flat:
            return QString("AutoCAD DXF 2004 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1021_Flat:
            return QString("AutoCAD DXF 2007 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1024_Flat:
            return QString("AutoCAD DXF 2010 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1027_Flat:
            return QString("AutoCAD DXF 2013 %1 %2").arg(dxf_flat_files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1006_AAMA:
            return QString("AutoCAD DXF R10 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1009_AAMA:
            return QString("AutoCAD DXF R11/12 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1012_AAMA:
            return QString("AutoCAD DXF R13 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1014_AAMA:
            return QString("AutoCAD DXF R14 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1015_AAMA:
            return QString("AutoCAD DXF 2000 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1018_AAMA:
            return QString("AutoCAD DXF 2004 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1021_AAMA:
            return QString("AutoCAD DXF 2007 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1024_AAMA:
            return QString("AutoCAD DXF 2010 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1027_AAMA:
            return QString("AutoCAD DXF 2013 AAMA %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1006_ASTM:
            return QString("AutoCAD DXF R10 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1009_ASTM:
            return QString("AutoCAD DXF R11/12 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1012_ASTM:
            return QString("AutoCAD DXF R13 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1014_ASTM:
            return QString("AutoCAD DXF R14 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1015_ASTM:
            return QString("AutoCAD DXF 2000 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1018_ASTM:
            return QString("AutoCAD DXF 2004 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1021_ASTM:
            return QString("AutoCAD DXF 2007 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1024_ASTM:
            return QString("AutoCAD DXF 2010 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::DXF_AC1027_ASTM:
            return QString("AutoCAD DXF 2013 ASTM %1 %2").arg(files_str, dxf_suffix);
        case LayoutExportFormat::PDFTiled:
            return QString("PDF tiled %1 (*.pdf)").arg(files_str);
        default:
            return QString("");
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportFormatCombobox::makeHelpFormatList()
{
    QString help("\n");
    QList<LayoutExportFormat> types = initFormatTypes();
    for (LayoutExportFormat type : types)
    {
        ExportFormat format;
        format.source      = exportFormatDescription(type, true);
        format.type        = type;
        help += QLatin1String("\t") + format.source + QLatin1String(" = ") +
                QString::number(static_cast<int>(format.type)) +
                QLatin1String("\n");
    }
    return help;
}

//---------------------------------------------------------------------------------------------------------------------
/// Called when the width has changed. This method sets the current width to the value chosen or even
/// offers a dialog to the user that allows him/ her to choose an individual width.
//---------------------------------------------------------------------------------------------------------------------
void ExportFormatCombobox::updateExportFormat(int index)
{
    QVariant format = itemData(index);
    if (format.isValid())
    {
       m_current_format_type = static_cast<LayoutExportFormat>(this->currentData().toInt());
    }

    emit exportFormatChanged(m_current_format_type);
}

QString ExportFormatCombobox::getCurrentSource()
{
    for (ExportFormat &format : m_formats)
    {
        if (format.translation == currentText())
        {
            return format.source;
        }
    }
    return QString("");
}

void ExportFormatCombobox::setCurrentToDefault()
{
    QString default_format = qApp->Settings()->getExportFormat();

    for (ExportFormat &format : m_formats)
    {
        if (format.source == default_format)
        {
            int index = findText(format.translation);
            if (index != -1)
            {
                setCurrentIndex(index);
            }
            else
            {
                setCurrentIndex(0);
            }
        }

    }
}

 /******************************************************************************
  *   @file   export_format_combobox.cpp
  **  @author DS Caskey
  **  @date   Mar 15, 2022
  **
  **  @brief
  **  @copyright
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
  *****************************************************************************/

#include <QAbstractItemView>
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

bool ExportFormatCombobox::havePdf = false;
bool ExportFormatCombobox::tested  = false;

/**
 * Constructor with name.
 */
ExportFormatCombobox::ExportFormatCombobox(QWidget *parent, const char *name)
		: QComboBox(parent)
      	, m_currentFormat()
{
		 setObjectName(name);
		 setEditable ( false );
		 init();
}

/**
 * Destructor
 */
ExportFormatCombobox::~ExportFormatCombobox() {}

/**
 * Initialisation called from constructor or manually but only once.
 */
void ExportFormatCombobox::init()
{
    this->blockSignals(true);

    int count = 0;
    foreach (auto& v, initFormats())
    {
        addItem(v.first, QVariant(static_cast<int>(v.second)));
        count++;
    }
    setMaxVisibleItems(count);

    this->view()->setTextElideMode(Qt::ElideNone);

    this->blockSignals(false);
		connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ExportFormatCombobox::updateExportFormat);

    setCurrentToDefault();
    updateExportFormat(currentIndex());
}

LayoutExportFormat ExportFormatCombobox::getExportFormat() const
{
    return m_currentFormat;
}

/**
 * Sets the currently selected format item to the given format.
 */
void ExportFormatCombobox::setExportFormat(LayoutExportFormat &format)
{
    m_currentFormat = format;

    setCurrentIndex(findData(QVariant(static_cast<int>(format))));

    if (currentIndex()!= count() -1 )
    {
        updateExportFormat(currentIndex());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<std::pair<QString, LayoutExportFormat>> ExportFormatCombobox::initFormats()
{
    QVector<std::pair<QString, LayoutExportFormat>> list;

    auto InitFormat = [&list](LayoutExportFormat format)
    {
        list.append(std::make_pair(exportFormatDescription(format), format));
    };

    InitFormat(LayoutExportFormat::SVG);
    InitFormat(LayoutExportFormat::PDF);
    InitFormat(LayoutExportFormat::PDFTiled);
    InitFormat(LayoutExportFormat::PNG);
    InitFormat(LayoutExportFormat::JPG);
    InitFormat(LayoutExportFormat::BMP);
    InitFormat(LayoutExportFormat::TIF);
    InitFormat(LayoutExportFormat::PPM);
    InitFormat(LayoutExportFormat::OBJ);
    if (supportPSTest())
    {
        InitFormat(LayoutExportFormat::PS);
        InitFormat(LayoutExportFormat::EPS);
    }
    InitFormat(LayoutExportFormat::DXF_AC1006_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1009_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1012_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1014_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1015_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1018_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1021_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1024_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1027_Flat);
    InitFormat(LayoutExportFormat::DXF_AC1006_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1009_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1012_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1014_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1015_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1018_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1021_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1024_AAMA);
    InitFormat(LayoutExportFormat::DXF_AC1027_AAMA);
    // We will support them anyway
//    InitFormat(LayoutExportFormat::DXF_AC1006_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1009_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1012_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1014_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1015_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1018_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1021_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1024_ASTM);
//    InitFormat(LayoutExportFormat::DXF_AC1027_ASTM);

    return list;
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportFormatCombobox::supportPSTest()
{
    if (!tested)
    {
        havePdf = testPdf();
        tested = true;
    }
    return havePdf;
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportFormatCombobox::testPdf()
{
    bool res = false;

    QProcess proc;
    QStringList args;

#if defined(Q_OS_WIN) || defined(Q_OS_OSX)
    // Seek pdftops in app bundle or near valentin.exe
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
        qDebug()<<PDFTOPS<<"error"<<proc.error()<<proc.errorString();
    }
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportFormatCombobox::exportFormatDescription(LayoutExportFormat format)
{
    const QString dxfSuffix = QStringLiteral("(*.dxf)");
    const QString dxfFlatFilesStr = tr("(flat) files");
    const QString filesStr = tr("files");

    switch(format)
    {
        case LayoutExportFormat::SVG:
            return QString("SVG %1 (*.svg)").arg(filesStr);
        case LayoutExportFormat::PDF:
            return QString("PDF %1 (*.pdf)").arg(filesStr);
        case LayoutExportFormat::PNG:
            return QString("PNG %1 (*.png)").arg(filesStr);
        case LayoutExportFormat::JPG:
            return QString("JPG %1 (*.jpg)").arg(filesStr);
        case LayoutExportFormat::BMP:
            return QString("BMP %1 (*.bmp)").arg(filesStr);
        case LayoutExportFormat::TIF:
            return QString("TIF %1 (*.tif)").arg(filesStr);
        case LayoutExportFormat::PPM:
            return QString("PPM %1 (*.ppm)").arg(filesStr);
        case LayoutExportFormat::OBJ:
            return "Wavefront OBJ (*.obj)";
        case LayoutExportFormat::PS:
            return QString("PS %1 (*.ps)").arg(filesStr);
        case LayoutExportFormat::EPS:
            return QString("EPS %1 (*.eps)").arg(filesStr);
        case LayoutExportFormat::DXF_AC1006_Flat:
            return QString("AutoCAD DXF R10 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1009_Flat:
            return QString("AutoCAD DXF R11/12 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1012_Flat:
            return QString("AutoCAD DXF R13 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1014_Flat:
            return QString("AutoCAD DXF R14 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1015_Flat:
            return QString("AutoCAD DXF 2000 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1018_Flat:
            return QString("AutoCAD DXF 2004 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1021_Flat:
            return QString("AutoCAD DXF 2007 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1024_Flat:
            return QString("AutoCAD DXF 2010 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1027_Flat:
            return QString("AutoCAD DXF 2013 %1 %2").arg(dxfFlatFilesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1006_AAMA:
            return QString("AutoCAD DXF R10 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1009_AAMA:
            return QString("AutoCAD DXF R11/12 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1012_AAMA:
            return QString("AutoCAD DXF R13 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1014_AAMA:
            return QString("AutoCAD DXF R14 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1015_AAMA:
            return QString("AutoCAD DXF 2000 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1018_AAMA:
            return QString("AutoCAD DXF 2004 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1021_AAMA:
            return QString("AutoCAD DXF 2007 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1024_AAMA:
            return QString("AutoCAD DXF 2010 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1027_AAMA:
            return QString("AutoCAD DXF 2013 AAMA %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1006_ASTM:
            return QString("AutoCAD DXF R10 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1009_ASTM:
            return QString("AutoCAD DXF R11/12 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1012_ASTM:
            return QString("AutoCAD DXF R13 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1014_ASTM:
            return QString("AutoCAD DXF R14 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1015_ASTM:
            return QString("AutoCAD DXF 2000 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1018_ASTM:
            return QString("AutoCAD DXF 2004 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1021_ASTM:
            return QString("AutoCAD DXF 2007 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1024_ASTM:
            return QString("AutoCAD DXF 2010 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::DXF_AC1027_ASTM:
            return QString("AutoCAD DXF 2013 ASTM %1 %2").arg(filesStr, dxfSuffix);
        case LayoutExportFormat::PDFTiled:
            return QString("PDF tiled %1 (*.pdf)").arg(filesStr);
        default:
            return QString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportFormatCombobox::makeHelpFormatList()
{
   QString out("\n");
   foreach(auto& v, initFormats())
   {
       out += QLatin1String("\t") + v.first + QLatin1String(" = ") + QString::number(static_cast<int>(v.second))
               + QLatin1String("\n");
   }
   return out;
}

/**
 * Called when the width has changed. This method sets the current width to the value chosen or even
 * offers a dialog to the user that allows him/ her to choose an individual width.
 */
void ExportFormatCombobox::updateExportFormat(int index)
{
    QVariant format = itemData(index);
    if(format != QVariant::Invalid )
    {
       m_currentFormat = static_cast<LayoutExportFormat>(this->currentData().toInt());
    }

    emit exportFormatChanged(m_currentFormat);
}

void ExportFormatCombobox::setCurrentToDefault()
{
    QString format = qApp->Settings()->getExportFormat();

    int index = findText(format);
    if (index != -1)
    {
        setCurrentIndex(index);
    }
    else
    {
        setCurrentIndex(0);
    }
}

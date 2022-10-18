/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file   dialogsavelayout.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#include "dialogsavelayout.h"
#include "ui_dialogsavelayout.h"
#include "../options.h"
#include "../core/vapplication.h"
#include "../vmisc/vsettings.h"
#include "../ifc/exception/vexception.h"
#include "../vwidgets/export_format_combobox.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>
#include <QRegularExpression>
#include <QtDebug>

const QString baseFilenameRegExp = QStringLiteral("^[\\p{L}\\p{Nd}\\-. _]+$");

//---------------------------------------------------------------------------------------------------------------------
DialogSaveLayout::DialogSaveLayout(int count, Draw mode, const QString &fileName, QWidget *parent)
    : VAbstractLayoutDialog(parent)
    , ui(new Ui::DialogSaveLAyout)
    , count(count)
    , isInitialized(false)
    , m_mode(mode)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->lineEditPath->setClearButtonEnabled(true);
    ui->lineEditFileName->setClearButtonEnabled(true);

    qApp->Seamly2DSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(false);

    ui->lineEditFileName->setValidator( new QRegularExpressionValidator(QRegularExpression(baseFilenameRegExp), this));

    const QString mask = fileName+QLatin1String("_");
    if (VApplication::IsGUIMode())
    {
        ui->lineEditFileName->setText(mask);
    }
    else
    {
        if (QRegularExpression(baseFilenameRegExp).match(mask).hasMatch())
        {
            ui->lineEditFileName->setText(mask);
        }
        else
        {
            VException e(tr("The base filename does not match a regular expression."));
            throw e;
        }
    }

    if (m_mode == Draw::Calculation)
    {
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1006_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1009_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1012_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1014_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1015_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1018_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1021_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1024_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1027_Flat);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1006_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1009_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1012_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1014_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1015_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1018_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1021_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1024_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1027_AAMA);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1006_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1009_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1012_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1014_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1015_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1018_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1021_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1024_ASTM);
        RemoveFormatFromList(LayoutExportFormat::DXF_AC1027_ASTM);
        //RemoveFormatFromList(LayoutExportFormat::PS);
        //RemoveFormatFromList(LayoutExportFormat::PDF);
        //RemoveFormatFromList(LayoutExportFormat::EPS);
    }
#ifdef V_NO_ASSERT // Temporarily unavailable
    RemoveFormatFromList(LayoutExportFormat::OBJ);
#endif

    if (m_mode != Draw::Layout)
    {
        RemoveFormatFromList(LayoutExportFormat::PDFTiled);
    }
    else
    {
        ui->checkBoxTextAsPaths->setVisible(false);
    }

    connect(ok_Button, &QPushButton::clicked, this, &DialogSaveLayout::Save);
    connect(ui->lineEditFileName, &QLineEdit::textChanged, this, &DialogSaveLayout::ShowExample);
    connect(ui->comboBoxFormat, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogSaveLayout::ShowExample);
    connect(ui->pushButtonBrowse, &QPushButton::clicked, this, [this]()
    {
        const QString dirPath = qApp->Seamly2DSettings()->GetPathLayout();
        bool usedNotExistedDir = false;
        QDir directory(dirPath);
        if (not directory.exists())
        {
            usedNotExistedDir = directory.mkpath(".");
        }

        const QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder"), dirPath,
                                                              QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks
                                                              | QFileDialog::DontUseNativeDialog);
        if (not dir.isEmpty())
        {// If paths equal the signal will not be called, we will do this manually
            dir == ui->lineEditPath->text() ? PathChanged(dir) : ui->lineEditPath->setText(dir);
        }

        if (usedNotExistedDir)
        {
            QDir directory(dirPath);
            directory.rmpath(".");
        }
    });
    connect(ui->lineEditPath, &QLineEdit::textChanged, this, &DialogSaveLayout::PathChanged);

    ui->lineEditPath->setText(qApp->Seamly2DSettings()->GetPathLayout());

    InitTemplates(ui->comboBoxTemplates);

    ReadSettings();
    ShowExample();//Show example for current format.
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::InitTemplates(QComboBox *comboBoxTemplates)
{
    VAbstractLayoutDialog::InitTemplates(comboBoxTemplates);

    // remove the custom format,
    int indexCustom = comboBoxTemplates->count() -1;
    comboBoxTemplates->removeItem(indexCustom);
}


//---------------------------------------------------------------------------------------------------------------------

void DialogSaveLayout::SelectFormat(LayoutExportFormat format)
{
    if (static_cast<int>(format) < 0 || format >= LayoutExportFormat::COUNT)
    {
        VException e(tr("Tried to use out of range format number."));
        throw e;
    }

    const int i = ui->comboBoxFormat->findData(static_cast<int>(format));
    if (i < 0)
    {
        VException e(tr("Selected not present format."));
        throw e;
    }
    ui->comboBoxFormat->setCurrentIndex(i);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetBinaryDXFFormat(bool binary)
{
    switch(Format())
    {
        case LayoutExportFormat::DXF_AC1006_Flat:
        case LayoutExportFormat::DXF_AC1009_Flat:
        case LayoutExportFormat::DXF_AC1012_Flat:
        case LayoutExportFormat::DXF_AC1014_Flat:
        case LayoutExportFormat::DXF_AC1015_Flat:
        case LayoutExportFormat::DXF_AC1018_Flat:
        case LayoutExportFormat::DXF_AC1021_Flat:
        case LayoutExportFormat::DXF_AC1024_Flat:
        case LayoutExportFormat::DXF_AC1027_Flat:
        case LayoutExportFormat::DXF_AC1006_AAMA:
        case LayoutExportFormat::DXF_AC1009_AAMA:
        case LayoutExportFormat::DXF_AC1012_AAMA:
        case LayoutExportFormat::DXF_AC1014_AAMA:
        case LayoutExportFormat::DXF_AC1015_AAMA:
        case LayoutExportFormat::DXF_AC1018_AAMA:
        case LayoutExportFormat::DXF_AC1021_AAMA:
        case LayoutExportFormat::DXF_AC1024_AAMA:
        case LayoutExportFormat::DXF_AC1027_AAMA:
        case LayoutExportFormat::DXF_AC1006_ASTM:
        case LayoutExportFormat::DXF_AC1009_ASTM:
        case LayoutExportFormat::DXF_AC1012_ASTM:
        case LayoutExportFormat::DXF_AC1014_ASTM:
        case LayoutExportFormat::DXF_AC1015_ASTM:
        case LayoutExportFormat::DXF_AC1018_ASTM:
        case LayoutExportFormat::DXF_AC1021_ASTM:
        case LayoutExportFormat::DXF_AC1024_ASTM:
        case LayoutExportFormat::DXF_AC1027_ASTM:
            ui->checkBoxBinaryDXF->setChecked(binary);
            break;
        case LayoutExportFormat::SVG:
        case LayoutExportFormat::PDF:
        case LayoutExportFormat::PDFTiled:
        case LayoutExportFormat::PNG:
        case LayoutExportFormat::JPG:
        case LayoutExportFormat::BMP:
        case LayoutExportFormat::PPM:
        case LayoutExportFormat::OBJ:
        case LayoutExportFormat::PS:
        case LayoutExportFormat::EPS:
        case LayoutExportFormat::TIF:
        default:
            ui->checkBoxBinaryDXF->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogSaveLayout::IsBinaryDXFFormat() const
{
    switch(Format())
    {
        case LayoutExportFormat::DXF_AC1006_Flat:
        case LayoutExportFormat::DXF_AC1009_Flat:
        case LayoutExportFormat::DXF_AC1012_Flat:
        case LayoutExportFormat::DXF_AC1014_Flat:
        case LayoutExportFormat::DXF_AC1015_Flat:
        case LayoutExportFormat::DXF_AC1018_Flat:
        case LayoutExportFormat::DXF_AC1021_Flat:
        case LayoutExportFormat::DXF_AC1024_Flat:
        case LayoutExportFormat::DXF_AC1027_Flat:
        case LayoutExportFormat::DXF_AC1006_AAMA:
        case LayoutExportFormat::DXF_AC1009_AAMA:
        case LayoutExportFormat::DXF_AC1012_AAMA:
        case LayoutExportFormat::DXF_AC1014_AAMA:
        case LayoutExportFormat::DXF_AC1015_AAMA:
        case LayoutExportFormat::DXF_AC1018_AAMA:
        case LayoutExportFormat::DXF_AC1021_AAMA:
        case LayoutExportFormat::DXF_AC1024_AAMA:
        case LayoutExportFormat::DXF_AC1027_AAMA:
        case LayoutExportFormat::DXF_AC1006_ASTM:
        case LayoutExportFormat::DXF_AC1009_ASTM:
        case LayoutExportFormat::DXF_AC1012_ASTM:
        case LayoutExportFormat::DXF_AC1014_ASTM:
        case LayoutExportFormat::DXF_AC1015_ASTM:
        case LayoutExportFormat::DXF_AC1018_ASTM:
        case LayoutExportFormat::DXF_AC1021_ASTM:
        case LayoutExportFormat::DXF_AC1024_ASTM:
        case LayoutExportFormat::DXF_AC1027_ASTM:
            return ui->checkBoxBinaryDXF->isChecked();
        case LayoutExportFormat::SVG:
        case LayoutExportFormat::PDF:
        case LayoutExportFormat::PDFTiled:
        case LayoutExportFormat::PNG:
        case LayoutExportFormat::JPG:
        case LayoutExportFormat::BMP:
        case LayoutExportFormat::PPM:
        case LayoutExportFormat::OBJ:
        case LayoutExportFormat::PS:
        case LayoutExportFormat::EPS:
        case LayoutExportFormat::TIF:
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetDestinationPath(const QString &cmdDestinationPath)
{
    QString path;
    if (cmdDestinationPath.isEmpty())
    {
        path = QDir::currentPath();
    }
    else if (QDir(cmdDestinationPath).isAbsolute())
    {
        path = cmdDestinationPath;
    }
    else
    {
        QDir dir;
        if (not dir.cd(cmdDestinationPath))
        {
            VException e(tr("The destination directory doesn't exists or is not readable."));
            throw e;
        }
        path = dir.absolutePath();
    }

    qDebug() << "Output full path: " << path << "\n";
    ui->lineEditPath->setText(path);
}

//---------------------------------------------------------------------------------------------------------------------
Draw DialogSaveLayout::Mode() const
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSaveLayout::exportFormatSuffix(LayoutExportFormat format)
{
    switch(format)
    {
        case LayoutExportFormat::SVG:
            return ".svg";
        case LayoutExportFormat::PDF:
        case LayoutExportFormat::PDFTiled:
            return ".pdf";
        case LayoutExportFormat::PNG:
            return ".png";
        case LayoutExportFormat::JPG:
            return ".jpg";
        case LayoutExportFormat::BMP:
            return ".bmp";
        case LayoutExportFormat::PPM:
            return ".ppm";
        case LayoutExportFormat::OBJ:
            return ".obj";
        case LayoutExportFormat::PS:
            return ".ps";
        case LayoutExportFormat::EPS:
            return ".eps";
        case LayoutExportFormat::TIF:
            return ".tif";
        case LayoutExportFormat::DXF_AC1006_Flat:
        case LayoutExportFormat::DXF_AC1009_Flat:
        case LayoutExportFormat::DXF_AC1012_Flat:
        case LayoutExportFormat::DXF_AC1014_Flat:
        case LayoutExportFormat::DXF_AC1015_Flat:
        case LayoutExportFormat::DXF_AC1018_Flat:
        case LayoutExportFormat::DXF_AC1021_Flat:
        case LayoutExportFormat::DXF_AC1024_Flat:
        case LayoutExportFormat::DXF_AC1027_Flat:
        case LayoutExportFormat::DXF_AC1006_AAMA:
        case LayoutExportFormat::DXF_AC1009_AAMA:
        case LayoutExportFormat::DXF_AC1012_AAMA:
        case LayoutExportFormat::DXF_AC1014_AAMA:
        case LayoutExportFormat::DXF_AC1015_AAMA:
        case LayoutExportFormat::DXF_AC1018_AAMA:
        case LayoutExportFormat::DXF_AC1021_AAMA:
        case LayoutExportFormat::DXF_AC1024_AAMA:
        case LayoutExportFormat::DXF_AC1027_AAMA:
        case LayoutExportFormat::DXF_AC1006_ASTM:
        case LayoutExportFormat::DXF_AC1009_ASTM:
        case LayoutExportFormat::DXF_AC1012_ASTM:
        case LayoutExportFormat::DXF_AC1014_ASTM:
        case LayoutExportFormat::DXF_AC1015_ASTM:
        case LayoutExportFormat::DXF_AC1018_ASTM:
        case LayoutExportFormat::DXF_AC1021_ASTM:
        case LayoutExportFormat::DXF_AC1024_ASTM:
        case LayoutExportFormat::DXF_AC1027_ASTM:
            return ".dxf";
        default:
            return QString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
DialogSaveLayout::~DialogSaveLayout()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSaveLayout::Path() const
{
    return ui->lineEditPath->text();
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSaveLayout::FileName() const
{
    return ui->lineEditFileName->text();
}

//---------------------------------------------------------------------------------------------------------------------
LayoutExportFormat DialogSaveLayout::Format() const
{
    return static_cast<LayoutExportFormat>(ui->comboBoxFormat->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSaveLayout::formatText() const
{
    return ui->comboBoxFormat->currentText();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::Save()
{
    for (int i=0; i < count; ++i)
    {
        const QString name = Path()+QLatin1String("/")+FileName()+QString::number(i+1)+exportFormatSuffix(Format());
        if (QFile::exists(name))
        {
            QMessageBox::StandardButton res = QMessageBox::question(this, tr("Name conflict"),
                                  tr("Folder already contains file with name %1. Rewrite all conflict file names?")
                                  .arg(name), QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
            if (res == QMessageBox::No)
            {
                reject();
                return;
            }
            else
            {
                break;
            }
        }
    }
    WriteSettings();
    accept();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::PathChanged(const QString &text)
{
    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(ok_Button != nullptr)

    QPalette palette = ui->lineEditPath->palette();

    QDir dir(text);
    dir.setPath(text);
    if (dir.exists(text))
    {
        ok_Button->setEnabled(true);
        palette.setColor(ui->lineEditPath->foregroundRole(), Qt::black);
    }
    else
    {
        ok_Button->setEnabled(false);
        palette.setColor(ui->lineEditPath->foregroundRole(), Qt::red);
    }

    ui->lineEditPath->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::ShowExample()
{
    const LayoutExportFormat currentFormat = Format();
    ui->checkBoxTextAsPaths->setEnabled(true);
    ui->exportQuality_Slider->setEnabled(false);
    ui->groupBoxPaperFormat->setEnabled(false);
    ui->groupBoxMargins->setEnabled(false);
    ui->labelExample->setText(FileName() + QLatin1String("1") + exportFormatSuffix(currentFormat));

    switch(currentFormat)
    {
        case LayoutExportFormat::DXF_AC1006_Flat:
        case LayoutExportFormat::DXF_AC1009_Flat:
        case LayoutExportFormat::DXF_AC1012_Flat:
        case LayoutExportFormat::DXF_AC1014_Flat:
        case LayoutExportFormat::DXF_AC1015_Flat:
        case LayoutExportFormat::DXF_AC1018_Flat:
        case LayoutExportFormat::DXF_AC1021_Flat:
        case LayoutExportFormat::DXF_AC1024_Flat:
        case LayoutExportFormat::DXF_AC1027_Flat:
        case LayoutExportFormat::DXF_AC1006_AAMA:
        case LayoutExportFormat::DXF_AC1009_AAMA:
        case LayoutExportFormat::DXF_AC1012_AAMA:
        case LayoutExportFormat::DXF_AC1014_AAMA:
        case LayoutExportFormat::DXF_AC1015_AAMA:
        case LayoutExportFormat::DXF_AC1018_AAMA:
        case LayoutExportFormat::DXF_AC1021_AAMA:
        case LayoutExportFormat::DXF_AC1024_AAMA:
        case LayoutExportFormat::DXF_AC1027_AAMA:
        case LayoutExportFormat::DXF_AC1006_ASTM:
        case LayoutExportFormat::DXF_AC1009_ASTM:
        case LayoutExportFormat::DXF_AC1012_ASTM:
        case LayoutExportFormat::DXF_AC1014_ASTM:
        case LayoutExportFormat::DXF_AC1015_ASTM:
        case LayoutExportFormat::DXF_AC1018_ASTM:
        case LayoutExportFormat::DXF_AC1021_ASTM:
        case LayoutExportFormat::DXF_AC1024_ASTM:
        case LayoutExportFormat::DXF_AC1027_ASTM:
            ui->checkBoxBinaryDXF->setEnabled(true);
            break;
        case LayoutExportFormat::PDFTiled:
            ui->groupBoxPaperFormat->setEnabled(true);
            ui->groupBoxMargins->setEnabled(true);
            break;
        case LayoutExportFormat::PNG:
            ui->exportQuality_Slider->setEnabled(true);
            break;
        case LayoutExportFormat::JPG:
            ui->exportQuality_Slider->setEnabled(true);
            break;
        case LayoutExportFormat::PDF:
        case LayoutExportFormat::SVG:
        case LayoutExportFormat::BMP:
        case LayoutExportFormat::PPM:
        case LayoutExportFormat::OBJ:
        case LayoutExportFormat::PS:
        case LayoutExportFormat::EPS:
        case LayoutExportFormat::TIF:
        default:
            ui->checkBoxBinaryDXF->setEnabled(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogSaveLayout::IsTextAsPaths() const
{
    return ui->checkBoxTextAsPaths->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::SetTextAsPaths(bool textAsPaths)
{
    if (m_mode != Draw::Layout)
    {
        ui->checkBoxTextAsPaths->setChecked(textAsPaths);
    }
    else
    {
        ui->checkBoxTextAsPaths->setChecked(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::showEvent(QShowEvent *event)
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

    setFixedHeight(size().height());

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSaveLayout::RemoveFormatFromList(LayoutExportFormat format)
{
    const int index = ui->comboBoxFormat->findData(static_cast<int>(format));
    if (index != -1)
    {
        ui->comboBoxFormat->removeItem(index);
        ui->comboBoxFormat->setCurrentToDefault();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Reads the values of the variables needed for the save layout dialog, for instance
 * the margins, teamplte and orientation of tiled pdf. Then sets the corresponding
 * elements of the dialog to these values.
 *
 * @brief DialogSaveLayout::ReadSettings
 */
void DialogSaveLayout::ReadSettings()
{
    VSettings *settings = qApp->Seamly2DSettings();
    const Unit unit = qApp->patternUnit();

    // read Margins top, right, bottom, left
    QMarginsF margins = settings->GetTiledPDFMargins(unit);

    ui->doubleSpinBoxLeftField->setValue(margins.left());
    ui->doubleSpinBoxTopField->setValue(margins.top());
    ui->doubleSpinBoxRightField->setValue(margins.right());
    ui->doubleSpinBoxBottomField->setValue(margins.bottom());
    ui->exportQuality_Slider->setValue(settings->getExportQuality());

    // read Template
    QSizeF size = QSizeF(settings->GetTiledPDFPaperWidth(Unit::Mm), settings->GetTiledPDFPaperHeight(Unit::Mm));

    const int max = static_cast<int>(PaperSizeTemplate::Custom);
    for (int i=0; i < max; ++i)
    {

        const QSizeF tmplSize = GetTemplateSize(static_cast<PaperSizeTemplate>(i), Unit::Mm);
        if (size == tmplSize)
        {
            ui->comboBoxTemplates->setCurrentIndex(i);
            break;
        }
    }

    // read Orientation
    if(settings->GetTiledPDFOrientation() == PageOrientation::Portrait)
    {
        ui->toolButtonPortrait->setChecked(true);
    }
    else
    {
        ui->toolButtonLandscape->setChecked(true);
    }

}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Writes the values of some variables (like the margins, template and orientation of tiled pdf).
 * of the save layout dialog into the settings.
 *
 * @brief DialogSaveLayout::WriteSettings
 */
void DialogSaveLayout::WriteSettings() const
{
    VSettings *settings = qApp->Seamly2DSettings();
    const Unit unit = qApp->patternUnit();

    // write Margins top, right, bottom, left
    QMarginsF margins = QMarginsF(
        ui->doubleSpinBoxLeftField->value(),
        ui->doubleSpinBoxTopField->value(),
        ui->doubleSpinBoxRightField->value(),
        ui->doubleSpinBoxBottomField->value()
    );
    settings->SetTiledPDFMargins(margins,unit);
    settings->setExportQuality(ui->exportQuality_Slider->value());

    // write Template
    PaperSizeTemplate temp;
    temp = static_cast<PaperSizeTemplate>(ui->comboBoxTemplates->currentData().toInt());
    const QSizeF size = GetTemplateSize(temp, Unit::Mm);

    settings->SetTiledPDFPaperHeight(size.height(),Unit::Mm);
    settings->SetTiledPDFPaperWidth(size.width(),Unit::Mm);

    // write Orientation
    if(ui->toolButtonPortrait->isChecked())
    {
        settings->SetTiledPDFOrientation(PageOrientation::Portrait);
    }
    else
    {
        settings->SetTiledPDFOrientation(PageOrientation::Landscape);
    }

    //Export Format
    if (qApp->Settings()->useLastExportFormat())
    {
        qApp->Settings()->setExportFormat(formatText());
    }
}

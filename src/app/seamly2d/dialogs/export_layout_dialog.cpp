/************************************************************************
 **
 **  @file   export_layout_dialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @author Douglas S Caskey
 **  @date   Nov 4, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#include "export_layout_dialog.h"
#include "ui_export_layout_dialog.h"
#include "../options.h"
#include "../core/vapplication.h"
#include "../vmisc/def.h"
#include "../vmisc/vsettings.h"
#include "../ifc/exception/vexception.h"
#include "../vwidgets/export_format_combobox.h"
#include "../vwidgets/page_format_combobox.h"

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QtDebug>
#include <QRegularExpression>
#include <QSpinBox>
#include <QtDebug>

const QString fileNameRegExp = QStringLiteral("^[\\p{L}\\p{Nd}\\-. _]+$");

//---------------------------------------------------------------------------------------------------------------------
ExportLayoutDialog::ExportLayoutDialog(int count, Draw mode, const QString &fileName, QWidget *parent)
    : AbstractLayoutDialog(parent)
    , ui(new Ui::ExportLayoutDialog)
    , m_count(count)
    , m_isInitialized(false)
    , m_mode(mode)
    , m_SaveButton(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->path_LineEdit->setClearButtonEnabled(true);
    ui->filename_LineEdit->setClearButtonEnabled(true);

    qApp->Seamly2DSettings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    m_SaveButton = ui->buttonBox->button(QDialogButtonBox::Save);
    SCASSERT(m_SaveButton != nullptr)
    m_SaveButton->setEnabled(false);

    ui->filename_LineEdit->setValidator( new QRegularExpressionValidator(QRegularExpression(fileNameRegExp), this));

    const QString mask = fileName + modeString();
    if (VApplication::IsGUIMode())
    {
        ui->filename_LineEdit->setText(mask);
    }
    else
    {
        if (QRegularExpression(fileNameRegExp).match(mask).hasMatch())
        {
            ui->filename_LineEdit->setText(mask);
        }
        else
        {
            VException e(tr("The base filename does not match a regular expression."));
            throw e;
        }
    }

    if (m_mode == Draw::Calculation)
    {
        removeFormatFromList(LayoutExportFormat::DXF_AC1006_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1009_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1012_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1014_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1015_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1018_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1021_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1024_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1027_Flat);
        removeFormatFromList(LayoutExportFormat::DXF_AC1006_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1009_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1012_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1014_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1015_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1018_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1021_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1024_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1027_AAMA);
        removeFormatFromList(LayoutExportFormat::DXF_AC1006_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1009_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1012_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1014_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1015_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1018_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1021_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1024_ASTM);
        removeFormatFromList(LayoutExportFormat::DXF_AC1027_ASTM);
        //removeFormatFromList(LayoutExportFormat::PS);
        //removeFormatFromList(LayoutExportFormat::PDF);
        //removeFormatFromList(LayoutExportFormat::EPS);
    }
#ifdef V_NO_ASSERT // Temporarily unavailable
    removeFormatFromList(LayoutExportFormat::OBJ);
#endif

    if (m_mode != Draw::Layout)
    {
        removeFormatFromList(LayoutExportFormat::PDFTiled);
    }
    else
    {
        ui->textAsPaths_Checkbox->setVisible(false);
    }

    connect(m_SaveButton, &QPushButton::clicked, this, &ExportLayoutDialog::save);
    connect(ui->filename_LineEdit, &QLineEdit::textChanged, this, &ExportLayoutDialog::showExportFiles);
    connect(ui->format_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &ExportLayoutDialog::showExportFiles);
    connect(ui->browse_PushButton, &QPushButton::clicked, this, [this]()
    {
        const QString dirPath = qApp->Seamly2DSettings()->getLayoutPath();
        bool usedNotExistedDir = false;
        QDir directory(dirPath);
        if (!directory.exists())
        {
            usedNotExistedDir = directory.mkpath(".");
        }

        const QString dir = QFileDialog::getExistingDirectory(this, tr("Select folder"), dirPath,
                                                              QFileDialog::ShowDirsOnly
                                                              | QFileDialog::DontResolveSymlinks
                                                              | QFileDialog::DontUseNativeDialog);
        if (!dir.isEmpty())
        {// If the paths are equal the slot will not be called, we will do this manually
            dir == ui->path_LineEdit->text() ? pathChanged(dir) : ui->path_LineEdit->setText(dir);
        }

        if (usedNotExistedDir)
        {
            QDir directory(dirPath);
            directory.rmpath(".");
        }
    });
    connect(ui->path_LineEdit, &QLineEdit::textChanged, this, &ExportLayoutDialog::pathChanged);
    connect(ui->quality_Slider, &QSlider::valueChanged, this, [this]()
    {
        ui->percent_SpinBox->setValue(ui->quality_Slider->value());
    });
    connect(ui->percent_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i)
    {
        ui->quality_Slider->setValue(i);
    });

    ui->path_LineEdit->setText(qApp->Seamly2DSettings()->getLayoutPath());

    initTemplates(ui->templates_ComboBox);

    readSettings();
    showExportFiles();//Show example for current format.
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::initTemplates(QComboBox *templates_ComboBox)
{
    // remove the custom format,
    int indexCustom = templates_ComboBox->count() -1;
    templates_ComboBox->removeItem(indexCustom);
}


//---------------------------------------------------------------------------------------------------------------------

void ExportLayoutDialog::selectFormat(LayoutExportFormat format)
{
    if (static_cast<int>(format) < 0 || format >= LayoutExportFormat::COUNT)
    {
        VException e(tr("Tried to use out of range format number."));
        throw e;
    }

    const int i = ui->format_ComboBox->findData(static_cast<int>(format));
    if (i < 0)
    {
        VException e(tr("Selected not present format."));
        throw e;
    }
    ui->format_ComboBox->setCurrentIndex(i);
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::setBinaryDXFFormat(bool binary)
{
    switch(format())
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
            ui->binaryDXF_CheckBox->setChecked(binary);
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
            ui->binaryDXF_CheckBox->setChecked(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportLayoutDialog::isBinaryDXFFormat() const
{
    switch(format())
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
            return ui->binaryDXF_CheckBox->isChecked();
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
void ExportLayoutDialog::setDestinationPath(const QString &cmdDestinationPath)
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
        if (!dir.cd(cmdDestinationPath))
        {
            VException e(tr("The destination directory doesn't exist or is not readable."));
            throw e;
        }
        path = dir.absolutePath();
    }

    qDebug() << "Full output path: " << path << "\n";
    ui->path_LineEdit->setText(path);
}

//---------------------------------------------------------------------------------------------------------------------
Draw ExportLayoutDialog::mode() const
{
    return m_mode;
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportLayoutDialog::exportFormatSuffix(LayoutExportFormat format)
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
ExportLayoutDialog::~ExportLayoutDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportLayoutDialog::path() const
{
    return ui->path_LineEdit->text();
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportLayoutDialog::fileName() const
{
    return ui->filename_LineEdit->text();
}

//---------------------------------------------------------------------------------------------------------------------
LayoutExportFormat ExportLayoutDialog::format() const
{
    return static_cast<LayoutExportFormat>(ui->format_ComboBox->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
QString ExportLayoutDialog::formatText() const
{
    return ui->format_ComboBox->currentText();
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::save()
{
        QString text = tr("%1 already exists.").arg(fileName() + exportFormatSuffix(format()));
        QString infoText = QStringLiteral("Do you want to replace it?");

        if (ui->exportFiles_ListWidget->count() > 1)
        {
            text     = tr("%1 files with basename %2 already exist.")
                          .arg(ui->exportFiles_ListWidget->count())
                          .arg(fileName());
            infoText = tr("Do you want to replace them?");
        }

        QString increment;

    for (int i=0; i < m_count; ++i)
    {
        if (m_mode == Draw::Layout)
        {
            increment = QStringLiteral("_0") + QString::number(i+1);
        }

        const QString name = QString("%1/%2%3%4")
        .arg(path())                        //1
        .arg(fileName())                    //2
        .arg(increment)                     //3
        .arg(exportFormatSuffix(format())); //4

        if (QFile::exists(name))
        {
            QMessageBox messageBox(this);
            messageBox.setWindowTitle(tr("Confirm Export"));
            messageBox.setIcon(QMessageBox::Question);
            messageBox.setText(text);
            messageBox.setInformativeText(infoText);
            messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            messageBox.setDefaultButton(QMessageBox::Yes);
            int result = messageBox.exec();

            if (result == QMessageBox::No)
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
    writeSettings();
    accept();
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::pathChanged(const QString &text)
{
    QPalette palette = ui->path_LineEdit->palette();

    QDir dir(text);
    dir.setPath(text);
    if (dir.exists(text))
    {
        m_SaveButton->setEnabled(true);
        palette.setColor(ui->path_LineEdit->foregroundRole(), Qt::black);
    }
    else
    {
        m_SaveButton->setEnabled(false);
        palette.setColor(ui->path_LineEdit->foregroundRole(), Qt::red);
    }

    ui->path_LineEdit->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::showExportFiles()
{
    ui->exportFiles_ListWidget->clear();

    if (m_mode != Draw::Layout)
    {

        ui->export_GroupBox->hide();
        adjustSize();
        return;
    }

    const LayoutExportFormat currentFormat = format();
    ui->textAsPaths_Checkbox->setEnabled(true);
    ui->quality_Slider->setEnabled(false);
    ui->paperFormat_GroupBox->setEnabled(false);
    ui->margins_GroupBox->setEnabled(false);
    ui->export_GroupBox->show();
    adjustSize();

    if (currentFormat == LayoutExportFormat::PDFTiled)
    {
        const QString name = QString("%1%2")
        .arg(fileName())                     //1
        .arg(exportFormatSuffix(format()));  //2

        QListWidgetItem *item = new QListWidgetItem(name);
        SCASSERT(item != nullptr)
        ui->exportFiles_ListWidget->addItem(item);
    }
    else
    {
        for (int i=0; i < m_count; ++i)
        {
            const QString name = QString("%1_0%2%3")
            .arg(fileName())                     //1
            .arg(QString::number(i+1))           //2
            .arg(exportFormatSuffix(format()));  //3

            QListWidgetItem *item = new QListWidgetItem(name);
            SCASSERT(item != nullptr)
            ui->exportFiles_ListWidget->addItem(item);
        }
    }

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
            ui->binaryDXF_CheckBox->setEnabled(true);
            break;
        case LayoutExportFormat::PDFTiled:
            ui->paperFormat_GroupBox->setEnabled(true);
            ui->margins_GroupBox->setEnabled(true);
            break;
        case LayoutExportFormat::PNG:
            ui->quality_Slider->setEnabled(true);
            break;
        case LayoutExportFormat::JPG:
            ui->quality_Slider->setEnabled(true);
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
            ui->binaryDXF_CheckBox->setEnabled(false);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool ExportLayoutDialog::isTextAsPaths() const
{
    return ui->textAsPaths_Checkbox->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::setTextAsPaths(bool textAsPaths)
{
    if (m_mode != Draw::Layout)
    {
        ui->textAsPaths_Checkbox->setChecked(textAsPaths);
    }
    else
    {
        ui->textAsPaths_Checkbox->setChecked(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (m_isInitialized)
    {
        return;
    }

    setFixedHeight(size().height());

    m_isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void ExportLayoutDialog::removeFormatFromList(LayoutExportFormat format)
{
    const int index = ui->format_ComboBox->findData(static_cast<int>(format));
    if (index != -1)
    {
        ui->format_ComboBox->removeItem(index);
        ui->format_ComboBox->setCurrentToDefault();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Reads the values of the variables needed for the save layout dialog, for instance
 * the margins, teamplte and orientation of tiled pdf. Then sets the corresponding
 * elements of the dialog to these values.
 *
 * @brief ExportLayoutDialog::readSettings
 */
void ExportLayoutDialog::readSettings()
{
    VSettings *settings = qApp->Seamly2DSettings();
    const Unit unit = qApp->patternUnit();

    // read Margins top, right, bottom, left
    QMarginsF margins = settings->GetTiledPDFMargins(unit);

    ui->leftField_DoubleSpinBox->setValue(margins.left());
    ui->topField_DoubleSpinBox->setValue(margins.top());
    ui->rightField_DoubleSpinBox->setValue(margins.right());
    ui->bottomField_DoubleSpinBox->setValue(margins.bottom());
    ui->quality_Slider->setValue(settings->getExportQuality());
    ui->percent_SpinBox->setValue(ui->quality_Slider->value());

    // read Template
    QSizeF size = QSizeF(settings->getTiledPDFPaperWidth(Unit::Mm), settings->getTiledPDFPaperHeight(Unit::Mm));

    const int max = static_cast<int>(PaperSizeFormat::Custom);
    for (int i=0; i < max; ++i)
    {

        const QSizeF tmplSize = getTemplateSize(static_cast<PaperSizeFormat>(i), Unit::Mm);
        if (size == tmplSize)
        {
            ui->templates_ComboBox->setCurrentIndex(i);
            break;
        }
    }

    // read Orientation
    if(settings->getTiledPDFOrientation() == PageOrientation::Portrait)
    {
        ui->portrait_ToolButton->setChecked(true);
    }
    else
    {
        ui->landscape_ToolButton->setChecked(true);
    }

}

//---------------------------------------------------------------------------------------------------------------------
/**
 * Writes the values of some variables (like the margins, template and orientation of tiled pdf).
 * of the save layout dialog into the settings.
 *
 * @brief ExportLayoutDialog::writeSettings
 */
void ExportLayoutDialog::writeSettings() const
{
    VSettings *settings = qApp->Seamly2DSettings();
    const Unit unit = qApp->patternUnit();

    // Margins:  left, top, right, bottom
    QMarginsF margins = QMarginsF(ui->leftField_DoubleSpinBox->value(),
                                  ui->topField_DoubleSpinBox->value(),
                                  ui->rightField_DoubleSpinBox->value(),
                                  ui->bottomField_DoubleSpinBox->value());

    settings->setTiledPDFMargins(margins,unit);
    settings->setExportQuality(ui->quality_Slider->value());

    // write Template
    PaperSizeFormat temp;
    temp = static_cast<PaperSizeFormat>(ui->templates_ComboBox->currentData().toInt());
    const QSizeF size = getTemplateSize(temp, Unit::Mm);

    settings->setTiledPDFPaperHeight(size.height(),Unit::Mm);
    settings->setTiledPDFPaperWidth(size.width(),Unit::Mm);

    // write Orientation
    if(ui->portrait_ToolButton->isChecked())
    {
        settings->setTiledPDFOrientation(PageOrientation::Portrait);
    }
    else
    {
        settings->setTiledPDFOrientation(PageOrientation::Landscape);
    }

    //Export Format
    if (qApp->Settings()->useLastExportFormat())
    {
        qApp->Settings()->setExportFormat(formatText());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief modeString()
 * @return Returns a string that is the mode type
 */
QString ExportLayoutDialog::modeString() const
{
    QString modeStr = QStringLiteral("");
    if (qApp->Seamly2DSettings()->useModeType())
    {
        switch (m_mode)
        {
            case Draw::Calculation:
            {
                modeStr = QStringLiteral("_block");
                break;
            }
            case Draw::Modeling:
            {
                modeStr = QStringLiteral("_pieces");
                break;
            }
            case Draw::Layout:
            default:
            {
                modeStr = QStringLiteral("_layout");
                break;
            }
        }
    }
    return modeStr;
}

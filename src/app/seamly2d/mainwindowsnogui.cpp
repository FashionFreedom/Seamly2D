/***************************************************************************
 **  @file   mainwindowsnogui.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   mainwindowsnogui.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
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

#include "mainwindowsnogui.h"
#include "core/vapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vobj/vobjpaintdevice.h"
#include "../vdxf/vdxfpaintdevice.h"
#include "dialogs/layoutsettings_dialog.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vlayout/vlayoutgenerator.h"
#include "dialogs/dialoglayoutprogress.h"
#include "dialogs/export_layout_dialog.h"
#include "../vlayout/vposter.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/measurements_def.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/tools/pattern_piece_tool.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMessageBox>
#include <QProcess>
#include <QToolButton>
#include <QtSvg>
#include <QPrintPreviewDialog>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QImageWriter>

#ifdef Q_OS_WIN
#   define PDFTOPS "pdftops.exe"
#else
#   define PDFTOPS "pdftops"
#endif

namespace
{
bool CreateLayoutPath(const QString &path)
{
    bool usedNotExistedDir = true;
    QDir dir(path);
    dir.setPath(path);
    if (!dir.exists(path))
    {
        usedNotExistedDir = dir.mkpath(".");
    }
    return usedNotExistedDir;
}

void RemoveLayoutPath(const QString &path, bool usedNotExistedDir)
{
    if (usedNotExistedDir)
    {
        QDir dir(path);
        dir.rmpath(".");
    }
}
}

//---------------------------------------------------------------------------------------------------------------------
MainWindowsNoGUI::MainWindowsNoGUI(QWidget *parent)
    : VAbstractMainWindow(parent),
      pieceList(),
      currentScene(nullptr),
      tempSceneLayout(nullptr),
      pattern(new VContainer(qApp->TrVars(), qApp->patternUnitP())),
      doc(nullptr),
      papers(),
      shadows(),
      scenes(),
      pieces(),
      piecesOnLayout(),
      undoAction(nullptr),
      redoAction(nullptr),
      actionDockWidgetToolOptions(nullptr),
      actionDockWidgetGroups(nullptr),
      actionDockWidgetLayouts(nullptr),
      actionDockWidgetToolbox(nullptr),
      isNoScaling(false),
      isLayoutStale(true),
      ignoreMargins(false),
      margins(),
      paperSize(),
      isTiled(false),
      isAutoCrop(false),
      isUnitePages(false),
      layoutPrinterName()

{
    InitTempLayoutScene();
}

//---------------------------------------------------------------------------------------------------------------------
MainWindowsNoGUI::~MainWindowsNoGUI()
{
    delete tempSceneLayout;
    delete pattern;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::toolLayoutSettings(QToolButton *tButton, bool checked)
{
    //QToolButton *tButton = qobject_cast< QToolButton * >(this->sender());
    SCASSERT(tButton != nullptr)

    if (checked)
    {
        VLayoutGenerator lGenerator;

        LayoutSettingsDialog layout(&lGenerator, this);
        if (layout.exec() == QDialog::Rejected)
        {
            tButton->setChecked(false);
            return;
        }
        layoutPrinterName = layout.SelectedPrinter();
        LayoutSettings(lGenerator);
        tButton->setChecked(false);
    }
    else
    {
        tButton->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindowsNoGUI::LayoutSettings(VLayoutGenerator& lGenerator)
{
    lGenerator.setPieces(pieceList);
    DialogLayoutProgress progress(pieceList.count(), this);
    if (VApplication::IsGUIMode())
    {
        connect(&lGenerator, &VLayoutGenerator::Start,     &progress,   &DialogLayoutProgress::Start);
        connect(&lGenerator, &VLayoutGenerator::Arranged,  &progress,   &DialogLayoutProgress::Arranged);
        connect(&lGenerator, &VLayoutGenerator::Error,     &progress,   &DialogLayoutProgress::Error);
        connect(&lGenerator, &VLayoutGenerator::Finished,  &progress,   &DialogLayoutProgress::Finished);
        connect(&progress,   &DialogLayoutProgress::Abort, &lGenerator, &VLayoutGenerator::Abort);
    }
    else
    {
        connect(&lGenerator, &VLayoutGenerator::Error, this, &MainWindowsNoGUI::ErrorConsoleMode);
    }
    lGenerator.Generate();

    switch (lGenerator.State())
    {
        case LayoutErrors::NoError:
            CleanLayout();
            papers = lGenerator.GetPapersItems();// Blank sheets
            pieces = lGenerator.getAllPieceItems();// All pieces items
            piecesOnLayout = lGenerator.getAllPieces();// All pieces items
            shadows = CreateShadows(papers);
            scenes = CreateScenes(papers, shadows, pieces);
            PrepareSceneList();
            ignoreMargins = not lGenerator.IsUsePrinterFields();
            margins = lGenerator.GetPrinterFields();
            paperSize = QSizeF(lGenerator.GetPaperWidth(), lGenerator.GetPaperHeight());
            isAutoCrop = lGenerator.GetAutoCrop();
            isUnitePages = lGenerator.IsUnitePages();
            isLayoutStale = false;
            if (VApplication::IsGUIMode())
            {
                QApplication::alert(this);
            }
            break;
        case LayoutErrors::ProcessStoped:
        case LayoutErrors::PrepareLayoutError:
        case LayoutErrors::EmptyPaperError:
            if (VApplication::IsGUIMode())
            {
                QApplication::alert(this);
            }
            return false;
        default:
            break;

    }
    return true;
}
//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ErrorConsoleMode(const LayoutErrors &state)
{
    switch (state)
    {
        case LayoutErrors::NoError:
            return;
        case LayoutErrors::PrepareLayoutError:
            qCritical() << tr("Couldn't prepare data for creation layout");
            break;
        case LayoutErrors::EmptyPaperError:
            qCritical() << tr("One or more pattern pieces are bigger than the paper format you selected. Please select a bigger paper format.");
            break;
        case LayoutErrors::ProcessStoped:
        default:
            break;
    }

    qApp->exit(V_EX_DATAERR);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportData(const QVector<VLayoutPiece> &pieceList, const ExportLayoutDialog &dialog)
{
    const LayoutExportFormat format = dialog.format();

    if (format == LayoutExportFormat::DXF_AC1006_AAMA ||
        format == LayoutExportFormat::DXF_AC1009_AAMA ||
        format == LayoutExportFormat::DXF_AC1012_AAMA ||
        format == LayoutExportFormat::DXF_AC1014_AAMA ||
        format == LayoutExportFormat::DXF_AC1015_AAMA ||
        format == LayoutExportFormat::DXF_AC1018_AAMA ||
        format == LayoutExportFormat::DXF_AC1021_AAMA ||
        format == LayoutExportFormat::DXF_AC1024_AAMA ||
        format == LayoutExportFormat::DXF_AC1027_AAMA)
    {
        if (dialog.mode() == Draw::Layout)
        {
            for (int i = 0; i < piecesOnLayout.size(); ++i)
            {
                const QString name = QString("%1/%2_0%3%4")
                .arg(dialog.path())                                          //1
                .arg(dialog.fileName())                                      //2
                .arg(QString::number(i+1))                                   //3
                .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format())); //4

                QGraphicsRectItem *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
                SCASSERT(paper != nullptr)

                ExportApparelLayout(dialog, piecesOnLayout.at(i), name, paper->rect().size().toSize());
            }
        }
        else
        {
            exportPiecesAsApparelLayout(dialog, pieceList);
        }
    }
    else
    {
        if (dialog.mode() == Draw::Layout)
        {
            ExportFlatLayout(dialog, scenes, papers, shadows, pieces, ignoreMargins, margins);
        }
        else
        {
            exportPiecesAsFlatLayout(dialog, pieceList);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportFlatLayout(const ExportLayoutDialog &dialog, const QList<QGraphicsScene *> &scenes,
                                        const QList<QGraphicsItem *> &papers, const QList<QGraphicsItem *> &shadows,
                                        const QList<QList<QGraphicsItem *> > &pieces, bool ignoreMargins,
                                        const QMarginsF &margins)
{
    const QString path = dialog.path();
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (!usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    qApp->Seamly2DSettings()->SetPathLayout(path);
    const LayoutExportFormat format = dialog.format();

    if (format == LayoutExportFormat::PDFTiled && dialog.mode() == Draw::Layout)
    {
        const QString name = QString("%1/%2%3")
        .arg(path)                                                   //1
        .arg(dialog.fileName())                                      //2
        .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format())); //3

        PdfTiledFile(name);
    }
    else
    {
        ExportScene(dialog, scenes, papers, shadows, pieces, ignoreMargins, margins);
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::exportPiecesAsFlatLayout(const ExportLayoutDialog &dialog,
                                                 const QVector<VLayoutPiece> &pieceList)
{
    if (pieceList.isEmpty())
    {
        return;
    }

    QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    for (int i=0; i < pieceList.count(); ++i)
    {
        QGraphicsItem *item = pieceList.at(i).GetItem(dialog.isTextAsPaths());
        item->setPos(pieceList.at(i).GetMx(), pieceList.at(i).GetMy());
        list.append(item);
    }

    for (int i=0; i < list.size(); ++i)
    {
        scene->addItem(list.at(i));
    }

    QList<QGraphicsItem *> papers;// Blank sheets
    QRect rect = scene->itemsBoundingRect().toRect();

    const int mx = rect.x();
    const int my = rect.y();

    QTransform transform;
    transform = transform.translate(-mx, -my);

    for (int i=0; i < list.size(); ++i)
    {
        list.at(i)->setTransform(transform);
    }

    rect = scene->itemsBoundingRect().toRect();

    QGraphicsRectItem *paper = new QGraphicsRectItem(rect);
    paper->setPen(QPen(Qt::black, 1));
    paper->setBrush(QBrush(Qt::white));
    papers.append(paper);

    QList<QList<QGraphicsItem *> > pieces;// All pieces
    pieces.append(list);

    QList<QGraphicsItem *> shadows = CreateShadows(papers);
    QList<QGraphicsScene *> scenes = CreateScenes(papers, shadows, pieces);

    const bool ignoreMargins = false;
    const qreal margin = ToPixel(1, Unit::Cm);
    ExportFlatLayout(dialog, scenes, papers, shadows, pieces, ignoreMargins,
                     QMarginsF(margin, margin, margin, margin));

    qDeleteAll(scenes);//Scene will clear all other items
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportApparelLayout(const ExportLayoutDialog &dialog, const QVector<VLayoutPiece> &pieces,
                                           const QString &name, const QSize &size) const
{
    const QString path = dialog.path();
    bool usedNotExistedDir = CreateLayoutPath(path);
    if (!usedNotExistedDir)
    {
        qCritical() << tr("Can't create a path");
        return;
    }

    qApp->Seamly2DSettings()->SetPathLayout(path);
    const LayoutExportFormat format = dialog.format();

    switch (format)
    {
        case LayoutExportFormat::DXF_AC1006_ASTM:
        case LayoutExportFormat::DXF_AC1009_ASTM:
        case LayoutExportFormat::DXF_AC1012_ASTM:
        case LayoutExportFormat::DXF_AC1014_ASTM:
        case LayoutExportFormat::DXF_AC1015_ASTM:
        case LayoutExportFormat::DXF_AC1018_ASTM:
        case LayoutExportFormat::DXF_AC1021_ASTM:
        case LayoutExportFormat::DXF_AC1024_ASTM:
        case LayoutExportFormat::DXF_AC1027_ASTM:
            Q_UNREACHABLE(); // For now not supported
            break;
        case LayoutExportFormat::DXF_AC1006_AAMA:
            AAMADxfFile(name, DRW::AC1006, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1009_AAMA:
            AAMADxfFile(name, DRW::AC1009, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1012_AAMA:
            AAMADxfFile(name, DRW::AC1012, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1014_AAMA:
            AAMADxfFile(name, DRW::AC1014, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1015_AAMA:
            AAMADxfFile(name, DRW::AC1015, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1018_AAMA:
            AAMADxfFile(name, DRW::AC1018, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1021_AAMA:
            AAMADxfFile(name, DRW::AC1021, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1024_AAMA:
            AAMADxfFile(name, DRW::AC1024, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        case LayoutExportFormat::DXF_AC1027_AAMA:
            AAMADxfFile(name, DRW::AC1027, dialog.isBinaryDXFFormat(), size, pieces);
            break;
        default:
            qWarning() << "Can't recognize file type." << Q_FUNC_INFO;
            break;
    }

    RemoveLayoutPath(path, usedNotExistedDir);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::exportPiecesAsApparelLayout(const ExportLayoutDialog &dialog,
                                                    QVector<VLayoutPiece> pieceList)
{
    if (pieceList.isEmpty())
    {
        return;
    }

    QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());

    QList<QGraphicsItem *> list;
    for (int i=0; i < pieceList.count(); ++i)
    {
        QGraphicsItem *item = pieceList.at(i).GetItem(dialog.isTextAsPaths());
        item->setPos(pieceList.at(i).GetMx(), pieceList.at(i).GetMy());
        list.append(item);
    }

    for (int i=0; i < list.size(); ++i)
    {
        scene->addItem(list.at(i));
    }

    QRect rect = scene->itemsBoundingRect().toRect();

    const int mx = rect.x();
    const int my = rect.y();

    QTransform transform;
    transform = transform.translate(-mx, -my);

    for (int i=0; i < list.size(); ++i)
    {
        list.at(i)->setTransform(transform);
    }

    rect = scene->itemsBoundingRect().toRect();

    for (int i=0; i < pieceList.count(); ++i)
    {
        QTransform moveTransform = pieceList[i].getTransform();
        moveTransform = moveTransform.translate(pieceList.at(i).GetMx(), pieceList.at(i).GetMy());
        moveTransform = moveTransform.translate(-mx, -my);
        pieceList[i].setTransform(moveTransform);
    }

    QString increment  = QStringLiteral("");
    if (dialog.mode() == Draw::Layout)
    {
        increment = QStringLiteral("_01");
    }

    const QString name = QString("%1/%2%3%4")
    .arg(dialog.path())                                          //1
    .arg(dialog.fileName())                                      //2
    .arg(increment)                                              //3
    .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format())); //4

    ExportApparelLayout(dialog, pieceList, name, rect.size());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPages(QPrinter *printer)
{
    VSettings *settings = qApp->Seamly2DSettings();

    // Here we try to understand the difference between the printer's dpi and scene dpi.
    // Get printer rect according to our dpi.
    const QRectF printerPageRect(0, 0, ToPixel(printer->pageRect(QPrinter::Millimeter).width(), Unit::Mm),
                                 ToPixel(printer->pageRect(QPrinter::Millimeter).height(), Unit::Mm));
    QRect pageRect = printer->pageLayout().paintRectPixels(printer->resolution());
    const double xscale = pageRect.width() / printerPageRect.width();
    const double yscale = pageRect.height() / printerPageRect.height();
    const double scale = qMin(xscale, yscale);

    QPainter painter;
    if (!painter.begin(printer))
    { // failed to open file
        qWarning("failed to open file, is it writable?");
        return;
    }

    painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, widthMainLine, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush ( QBrush ( Qt::NoBrush ) );

    int count = 0;
    QSharedPointer<QVector<PosterData>> poster;
    QSharedPointer<VPoster> posterazor;

    if (isTiled)
    {
        // when isTiled, the landscape tiles have to be rotated, because the pages
        // stay portrait in the pdf
        if(settings->getTiledPDFOrientation() == PageOrientation::Landscape)
        {
            painter.rotate(-90);
            painter.translate(-ToPixel(printer->pageRect(QPrinter::Millimeter).height(),Unit::Mm), 0);
        }

        poster = QSharedPointer<QVector<PosterData>>(new QVector<PosterData>());
        posterazor = QSharedPointer<VPoster>(new VPoster(printer));

        for (int i=0; i < scenes.size(); ++i)
        {

            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));

            if (paper)
            {
                *poster += posterazor->Calc(paper->rect().toRect(), i, settings->getTiledPDFOrientation());
            }
        }

        count = poster->size();
    }
    else
    {
        count = scenes.size();
    }

    // Handle the fromPage(), toPage(), supportsMultipleCopies(), and numCopies() values from QPrinter.
    int firstPage = printer->fromPage() - 1;
    if (firstPage >= count)
    {
        return;
    }
    if (firstPage == -1)
    {
        firstPage = 0;
    }

    int lastPage = printer->toPage() - 1;
    if (lastPage == -1 || lastPage >= count)
    {
        lastPage = count - 1;
    }

    const int numPages = lastPage - firstPage + 1;
    int copyCount = 1;
    if (!printer->supportsMultipleCopies())
    {
        copyCount = printer->copyCount();
    }

    for (int i = 0; i < copyCount; ++i)
    {
        for (int j = 0; j < numPages; ++j)
        {
            if (i != 0 || j != 0)
            {
                if (!printer->newPage())
                {
                    qWarning("Failed in flushing page to disk, disk may be full.");
                    return;
                }
            }
            int index;
            if (printer->pageOrder() == QPrinter::FirstPageFirst)
            {
                index = firstPage + j;
            }
            else
            {
                index = lastPage - j;
            }

            int paperIndex = -1;
            isTiled ? paperIndex = static_cast<int>(poster->at(index).index) : paperIndex = index;

            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(paperIndex));
            if (paper)
            {
                QVector<QGraphicsItem *> posterData;
                if (isTiled)
                {
                    // Draw borders
                    posterData = posterazor->Borders(paper, poster->at(index), scenes.size());
                }

                PreparePaper(paperIndex);

                // Render
                QRectF source;
                isTiled ? source = poster->at(index).rect : source = paper->rect();

                qreal x,y;
                if(printer->fullPage())
                {
                    QMarginsF printerMargins = printer->pageLayout().margins();
                    x = qFloor(ToPixel(printerMargins.left(),Unit::Mm));
                    y = qFloor(ToPixel(printerMargins.top(),Unit::Mm));
                }
                else
                {
                    x = 0; y = 0;
                }

                QRectF target(x * scale, y * scale, source.width() * scale, source.height() * scale);

                scenes.at(paperIndex)->render(&painter, target, source, Qt::IgnoreAspectRatio);

                if (isTiled)
                {
                    // Remove borders
                    qDeleteAll(posterData);
                }

                // Restore
                RestorePaper(paperIndex);
            }
        }
    }

    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPreviewOrigin()
{
    if (!isPagesUniform())
    {
        qCritical() << tr("For previewing multipage document all sheet should have the same size.");
        return;
    }

    isTiled = false;
    PrintPreview();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPreviewTiled()
{
    isTiled = true;
    PrintPreview();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintOrigin()
{
    if (!isPagesUniform())
    {
        qCritical() << tr("For printing multipages document all sheet should have the same size.");
        return;
    }

    isTiled = false;
    LayoutPrint();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintTiled()
{
    isTiled = true;
    LayoutPrint();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief refreshLabels call to recalculate piece labels. For example after changing a font.
 */
void MainWindowsNoGUI::refreshLabels()
{
    const QHash<quint32, VPiece> *list = pattern->DataPieces();
    QHash<quint32, VPiece>::const_iterator i = list->constBegin();
    while (i != list->constEnd())
    {
        if (PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(i.key())))
        {
            tool->UpdatePatternLabel();
            tool->UpdatePieceLabel();
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief refreshGrainLines call to recalculate piece grainlines. For example after show / hide grainlines.
 */
void MainWindowsNoGUI::refreshGrainLines()
{
    const QHash<quint32, VPiece> *list = pattern->DataPieces();
    QHash<quint32, VPiece>::const_iterator i = list->constBegin();
    while (i != list->constEnd())
    {
        if (PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(i.key())))
        {
            tool->UpdateGrainline();
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief refreshSeamAllowances call to show / hide seam allowances.
 */
void MainWindowsNoGUI::refreshSeamAllowances()
{
    const QHash<quint32, VPiece> *list = pattern->DataPieces();
    QHash<quint32, VPiece>::const_iterator i = list->constBegin();
    while (i != list->constEnd())
    {
        if (PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(i.key())))
        {
            tool->RefreshGeometry();
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VLayoutPiece> MainWindowsNoGUI::preparePiecesForLayout(const QHash<quint32, VPiece> &pieces)
{
    QVector<VLayoutPiece> pieceList;
    if (!pieces.isEmpty())
    {
        QHash<quint32, VPiece>::const_iterator i = pieces.constBegin();
        while (i != pieces.constEnd())
        {
            VAbstractTool *tool = qobject_cast<VAbstractTool*>(VAbstractPattern::getTool(i.key()));
            SCASSERT(tool != nullptr)
            pieceList.append(VLayoutPiece::Create(i.value(), tool->getData()));
            ++i;
        }
    }

    return pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::InitTempLayoutScene()
{
    tempSceneLayout = new VMainGraphicsScene();
    tempSceneLayout->setBackgroundBrush( QBrush(QColor(Qt::gray), Qt::SolidPattern) );
}

//---------------------------------------------------------------------------------------------------------------------
QIcon MainWindowsNoGUI::ScenePreview(int i) const
{
    QImage image;
    QGraphicsRectItem *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
    if (paper)
    {
        const QRectF r = paper->rect();
        // Create the image with the exact size of the shrunk scene
        image = QImage(QSize(static_cast<qint32>(r.width()), static_cast<qint32>(r.height())), QImage::Format_RGB32);

        if (!image.isNull())
        {
            image.fill(Qt::white);
            QPainter painter(&image);
            painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(QPen(Qt::black, widthMainLine, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush ( QBrush ( Qt::NoBrush ) );
            scenes.at(i)->render(&painter, r, r, Qt::IgnoreAspectRatio);
            painter.end();
        }
        else
        {
            qWarning() << "Cannot create image. Size too big";
        }
    }
    else
    {
        image = QImage(QSize(101, 146), QImage::Format_RGB32);
        image.fill(Qt::white);
    }
    return QIcon(QBitmap::fromImage(image));
}

//---------------------------------------------------------------------------------------------------------------------
QList<QGraphicsItem *> MainWindowsNoGUI::CreateShadows(const QList<QGraphicsItem *> &papers)
{
    QList<QGraphicsItem *> shadows;

    for (int i=0; i< papers.size(); ++i)
    {
        qreal x1=0, y1=0, x2=0, y2=0;
        QGraphicsRectItem *item = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
        if (item)
        {
            item->rect().getCoords(&x1, &y1, &x2, &y2);
            QGraphicsRectItem *shadowPaper = new QGraphicsRectItem(QRectF(x1+4, y1+4, x2+4, y2+4));
            shadowPaper->setBrush(QBrush(Qt::black));
            shadows.append(shadowPaper);
        }
        else
        {
            shadows.append(nullptr);
        }
    }

    return shadows;
}

//---------------------------------------------------------------------------------------------------------------------
QList<QGraphicsScene *> MainWindowsNoGUI::CreateScenes(const QList<QGraphicsItem *> &papers,
                                                       const QList<QGraphicsItem *> &shadows,
                                                       const QList<QList<QGraphicsItem *> > &pieces)
{
    QList<QGraphicsScene *> scenes;
    for (int i=0; i<papers.size(); ++i)
    {
        QGraphicsScene *scene = new VMainGraphicsScene();
        scene->setBackgroundBrush(QBrush(QColor(Qt::gray), Qt::SolidPattern));
        scene->addItem(shadows.at(i));
        scene->addItem(papers.at(i));

        QList<QGraphicsItem *> item = pieces.at(i);
        for (int i=0; i < item.size(); ++i)
        {
            scene->addItem(item.at(i));
        }

        scenes.append(scene);
    }

    return scenes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportSVG save layout to svg file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportSVG(const QString &name, QGraphicsRectItem *paper, QGraphicsScene *scene) const
{
    QSvgGenerator generator;
    generator.setFileName(name);
    generator.setSize(paper->rect().size().toSize());
    generator.setViewBox(paper->rect());
    generator.setTitle(tr("Pattern"));
    generator.setDescription(doc->GetDescription());
    generator.setResolution(static_cast<int>(PrintDPI));
    QPainter painter;
    painter.begin(&generator);
    painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
    painter.setRenderHint(QPainter::Antialiasing, true);
    //painter.setPen(QPen(Qt::black, widthHairLine, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter, paper->rect(), paper->rect(), Qt::IgnoreAspectRatio);
    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportPNG save layout to png file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportPNG(const QString &fileName,  QGraphicsScene *scene) const
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);                                              // Start all pixels transparent
    QPainter painter(&image);
    painter.setFont(qApp->Seamly2DSettings()->getLabelFont());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter);
    image.save(fileName, "PNG", qApp->Seamly2DSettings()->getExportQuality());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportTIF save layout to tif file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportTIF(const QString &fileName,  QGraphicsScene *scene) const
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);                                              // Start all pixels transparent
    QPainter painter(&image);
    painter.setFont(qApp->Seamly2DSettings()->getLabelFont());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter);

    QImageWriter writer;
    writer.setFormat("TIF");
    writer.setCompression(1);
    writer.setFileName(fileName);
    writer.write(image);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportJPG save layout to jpg file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportJPG(const QString &fileName,  QGraphicsScene *scene) const
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::white);                                              // Start all pixels transparent
    QPainter painter(&image);
    painter.setFont(qApp->Seamly2DSettings()->getLabelFont());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter);
    image.save(fileName, "JPG", qApp->Seamly2DSettings()->getExportQuality());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportBMP save layout to bmp file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportBMP(const QString &fileName,  QGraphicsScene *scene) const
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::white);                                              // Start all pixels transparent
    QPainter painter(&image);
    painter.setFont(qApp->Seamly2DSettings()->getLabelFont());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter);
    image.save(fileName, "BMP", qApp->Seamly2DSettings()->getExportQuality());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportPPM save layout to gif file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportPPM(const QString &fileName,  QGraphicsScene *scene) const
{
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);                                              // Start all pixels transparent
    QPainter painter(&image);
    painter.setFont(qApp->Seamly2DSettings()->getLabelFont());
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter);
    image.save(fileName, "PPM", qApp->Seamly2DSettings()->getExportQuality());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportPDF save layout to pdf file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportPDF(const QString &name, QGraphicsRectItem *paper, QGraphicsScene *scene,
                               bool ignoreMargins, const QMarginsF &margins) const
{
    QPrinter printer;
    printer.setCreator(QGuiApplication::applicationDisplayName()+QLatin1String(" ")+
                       QCoreApplication::applicationVersion());
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(name);
    printer.setDocName(FileName());
    printer.setResolution(static_cast<int>(PrintDPI));
    printer.setPageOrientation(QPageLayout::Portrait);
    printer.setFullPage(ignoreMargins);

    const QRectF sourceRect = paper->rect();
    const QRectF targetRect = QRectF(QPoint(0,0), QPoint(sourceRect.width(),sourceRect.height()));
    QSizeF size(FromPixel(sourceRect.width() + margins.left() + margins.right(), Unit::Mm),
                FromPixel(sourceRect.height() + margins.top() + margins.bottom(), Unit::Mm));
    QPageSize pageSize(size, QPageSize::Unit::Millimeter);
    printer.setPageSize(pageSize);

    if (!ignoreMargins)
    {
        const qreal left = FromPixel(margins.left(), Unit::Mm);
        const qreal top = FromPixel(margins.top(), Unit::Mm);
        const qreal right = FromPixel(margins.right(), Unit::Mm);
        const qreal bottom = FromPixel(margins.bottom(), Unit::Mm);

        const bool success = printer.setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter);
        if (!success)
        {
            qWarning() << tr("Cannot set printer margins");
        }
    }

    QPainter painter;
    if (painter.begin(&printer) == false)
    {
        qCritical("%s", qUtf8Printable(tr("Can't open printer %1").arg(name))); // failed to open file
        return;
    }
    painter.setFont(QFont( "Arial", 8, QFont::Normal));
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, widthMainLine, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(QBrush(Qt::NoBrush));
    scene->render(&painter, targetRect, sourceRect, Qt::KeepAspectRatio);
    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PdfTiledFile(const QString &name)
{
    isTiled = true;

    if (isLayoutStale)
    {
        if (ContinueIfLayoutStale() == QMessageBox::No)
        {
            return;
        }
    }
    QPrinter printer;
    SetPrinterSettings(&printer, PrintType::PrintPDF);

    // Call IsPagesFit after setting a printer settings and check if pages is not bigger than printer's paper size
    if (!isTiled && not IsPagesFit(printer.pageLayout().fullRectPixels(printer.resolution()).size()))
    {
        qWarning() << tr("Pages will be cropped because they do not fit printer paper size.");
    }

    printer.setOutputFileName(name);
    printer.setResolution(static_cast<int>(PrintDPI));
    PrintPages( &printer );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportEPS( save layout to eps file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportEPS(const QString &name, QGraphicsRectItem *paper, QGraphicsScene *scene,
                               bool ignoreMargins, const QMarginsF &margins) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        exportPDF(tmp.fileName(), paper, scene, ignoreMargins, margins);
        QStringList params = QStringList() << "-eps" << tmp.fileName() << name;
        convertPdfToPs(params);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief exportPS save layout to ps file.
 * @param fileName name layout file.
 */
void MainWindowsNoGUI::exportPS(const QString &name, QGraphicsRectItem *paper, QGraphicsScene *scene, bool
                              ignoreMargins, const QMarginsF &margins) const
{
    QTemporaryFile tmp;
    if (tmp.open())
    {
        exportPDF(tmp.fileName(), paper, scene, ignoreMargins, margins);
        QStringList params = QStringList() << tmp.fileName() << name;
        convertPdfToPs(params);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief convertPdfToPs use external tool "pdftops" for converting pdf to eps or ps format.
 * @param params string with parameter for tool. Parameters have format: "-eps input_file out_file". Use -eps when
 * need create eps file.
 */
void MainWindowsNoGUI::convertPdfToPs(const QStringList &params) const
{
#ifndef QT_NO_CURSOR
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    QProcess proc;
#if defined(Q_OS_MAC)
    // Fix issue #594. Broken export on Mac.
    proc.setWorkingDirectory(qApp->applicationDirPath());
    proc.start(QLatin1String("./") + PDFTOPS, params);
#else
    proc.start(PDFTOPS, params);
#endif
    if (proc.waitForStarted(15000))
    {
        proc.waitForFinished(15000);
    }
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif

    QFile f(params.last());
    if (f.exists() == false)
    {
        const QString msg = tr("Creating file '%1' failed! %2").arg(params.last()).arg(proc.errorString());
        QMessageBox msgBox(QMessageBox::Critical, tr("Critical error!"), msg, QMessageBox::Ok | QMessageBox::Default);
        msgBox.exec();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ObjFile(const QString &name, QGraphicsRectItem *paper, QGraphicsScene *scene) const
{
    VObjPaintDevice generator;
    generator.setFileName(name);
    generator.setSize(paper->rect().size().toSize());
    generator.setResolution(static_cast<int>(PrintDPI));
    QPainter painter;
    painter.begin(&generator);
    scene->render(&painter, paper->rect(), paper->rect(), Qt::IgnoreAspectRatio);
    painter.end();
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void MainWindowsNoGUI::FlatDxfFile(const QString &name, int version, bool binary, QGraphicsRectItem *paper,
                               QGraphicsScene *scene, const QList<QList<QGraphicsItem *> > &pieces) const
{
    PrepareTextForDXF(endStringPlaceholder, pieces);
    VDxfPaintDevice generator;
    generator.setFileName(name);
    generator.setSize(paper->rect().size().toSize());
    generator.setResolution(PrintDPI);
    generator.setVersion(static_cast<DRW::Version>(version));
    generator.SetBinaryFormat(binary);
    generator.setInsunits(VarInsunits::Millimeters);// Decided to always use mm. See issue #745

    QPainter painter;
    if (painter.begin(&generator))
    {
        scene->render(&painter, paper->rect(), paper->rect(), Qt::IgnoreAspectRatio);
        painter.end();
    }
    RestoreTextAfterDXF(endStringPlaceholder, pieces);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::AAMADxfFile(const QString &name, int version, bool binary, const QSize &size,
                                   const QVector<VLayoutPiece> &pieces) const
{
    VDxfPaintDevice generator;
    generator.setFileName(name);
    generator.setSize(size);
    generator.setResolution(PrintDPI);
    generator.setVersion(static_cast<DRW::Version>(version));
    generator.SetBinaryFormat(binary);
    generator.setInsunits(VarInsunits::Millimeters);// Decided to always use mm. See issue #745
    generator.ExportToAAMA(pieces);
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PreparePaper(int index) const
{
    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(index));
    if (paper)
    {
        QBrush brush(Qt::white);
        scenes.at(index)->setBackgroundBrush(brush);
        shadows.at(index)->setVisible(false);
        paper->setPen(QPen(Qt::white, 0.1, Qt::NoPen));// border
    }

}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::RestorePaper(int index) const
{
    auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(index));
    if (paper)
    {
        // Restore
        paper->setPen(QPen(Qt::black, widthMainLine));
        QBrush brush(Qt::gray);
        scenes.at(index)->setBackgroundBrush(brush);
        shadows.at(index)->setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PrepareTextForDXF prepare QGraphicsSimpleTextItem items for export to flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark end of each string by adding special
 * placeholder. This method append it.
 *
 * @param placeholder placeholder that will be appended to each QGraphicsSimpleTextItem item's text string.
 */
void MainWindowsNoGUI::PrepareTextForDXF(const QString &placeholder,
                                         const QList<QList<QGraphicsItem *> > &pieces) const
{
    for (int i = 0; i < pieces.size(); ++i)
    {
        const QList<QGraphicsItem *> &paperItems = pieces.at(i);
        for (int j = 0; j < paperItems.size(); ++j)
        {
            QList<QGraphicsItem *> pieceChildren = paperItems.at(j)->childItems();
            for (int k = 0; k < pieceChildren.size(); ++k)
            {
                QGraphicsItem *item = pieceChildren.at(k);
                if (item->type() == QGraphicsSimpleTextItem::Type)
                {
                    if(QGraphicsSimpleTextItem *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item))
                    {
                        textItem->setText(textItem->text() + placeholder);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MainWindowsNoGUI::RestoreTextAfterDXF restore QGraphicsSimpleTextItem items after export to flat dxf.
 *
 * Because QPaintEngine::drawTextItem doesn't pass whole string per time we mark end of each string by adding special
 * placeholder. This method remove it.
 *
 * @param placeholder placeholder that will be removed from each QGraphicsSimpleTextItem item's text string.
 */
void MainWindowsNoGUI::RestoreTextAfterDXF(const QString &placeholder,
                                           const QList<QList<QGraphicsItem *> > &pieces) const
{
    for (int i = 0; i < pieces.size(); ++i)
    {
        const QList<QGraphicsItem *> &paperItems = pieces.at(i);
        for (int j = 0; j < paperItems.size(); ++j)
        {
            QList<QGraphicsItem *> pieceChildren = paperItems.at(i)->childItems();
            for (int k = 0; k < pieceChildren.size(); ++k)
            {
                QGraphicsItem *item = pieceChildren.at(k);
                if (item->type() == QGraphicsSimpleTextItem::Type)
                {
                    if(QGraphicsSimpleTextItem *textItem = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(item))
                    {
                        QString text = textItem->text();
                        text.replace(placeholder, "");
                        textItem->setText(text);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::PrintPreview()
{
    if (isLayoutStale)
    {
        if (ContinueIfLayoutStale() == QMessageBox::No)
        {
            return;
        }
    }

    QPrinterInfo info = QPrinterInfo::printerInfo(layoutPrinterName);
    if(info.isNull() || info.printerName().isEmpty())
    {
        info = QPrinterInfo::defaultPrinter();
    }
    QSharedPointer<QPrinter> printer = PreparePrinter(info);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    SetPrinterSettings(printer.data(), PrintType::PrintPreview);
    printer->setResolution(static_cast<int>(PrintDPI));
    // display print preview dialog
    QPrintPreviewDialog preview(printer.data());
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &MainWindowsNoGUI::PrintPages);
    preview.exec();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::LayoutPrint()
{
    if (isLayoutStale)
    {
        if (ContinueIfLayoutStale() == QMessageBox::No)
        {
            return;
        }
    }
    // display print dialog and if accepted print
    QPrinterInfo info = QPrinterInfo::printerInfo(layoutPrinterName);
    if(info.isNull() || info.printerName().isEmpty())
    {
        info = QPrinterInfo::defaultPrinter();
    }
    QSharedPointer<QPrinter> printer = PreparePrinter(info, QPrinter::HighResolution);
    if (printer.isNull())
    {
        qCritical("%s\n\n%s", qUtf8Printable(tr("Print error")),
                  qUtf8Printable(tr("Cannot proceed because there are no available printers in your system.")));
        return;
    }

    SetPrinterSettings(printer.data(), PrintType::PrintNative);
    QPrintDialog dialog(printer.data(), this );
    // If only user couldn't change page margins we could use method setMinMax();
    dialog.setOption(QPrintDialog::PrintCurrentPage, false);
    if ( dialog.exec() == QDialog::Accepted )
    {
        printer->setResolution(static_cast<int>(PrintDPI));
        PrintPages(printer.data());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::SetPrinterSettings(QPrinter *printer, const PrintType &printType)
{
    SCASSERT(printer != nullptr)
    printer->setCreator(QGuiApplication::applicationDisplayName()+" "+QCoreApplication::applicationVersion());
    printer->setPageOrientation(QPageLayout::Orientation::Portrait);

    if (!isTiled)
    {
        QSizeF size = QSizeF(FromPixel(paperSize.width(), Unit::Mm), FromPixel(paperSize.height(), Unit::Mm));
        if (isAutoCrop || isUnitePages)
        {
            auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(0));
            if (paper)
            {
                size = QSizeF(FromPixel(paperSize.width(), Unit::Mm),
                              FromPixel(paper->rect().height() + margins.top() + margins.bottom(), Unit::Mm));
            }
        }

        const QPageSize pSZ = FindQPrinterPageSize(size);
        if (pSZ.id() == QPageSize::Custom)
        {
            QPageSize pageSize(size, QPageSize::Unit::Millimeter);
            printer->setPageSize (pageSize);
        }
        else
        {
            printer->setPageSize (pSZ);
        }
    }
    else
    {
        VSettings *settings = qApp->Seamly2DSettings();
        QSizeF size = QSizeF(
            settings->getTiledPDFPaperWidth(Unit::Mm),
            settings->getTiledPDFPaperHeight(Unit::Mm)

            );
        const QPageSize pSZ = FindQPrinterPageSize(size);
        printer->setPageSize(pSZ);
        // no need to take custom into account, because custom isn't a format option for tiled pdf.
    }


    printer->setFullPage(true);
    //printer->setFullPage(ignoreMargins);

    qreal left, top, right, bottom;

    if (!isTiled)
    {
        QMarginsF pageMargin = QMarginsF(UnitConvertor(margins, Unit::Px, Unit::Mm));
        left = pageMargin.left();
        top = pageMargin.top();
        right = pageMargin.right();
        bottom = pageMargin.bottom();
    }
    else
    {
        VSettings *settings = qApp->Seamly2DSettings();
        QMarginsF  pageMargin = QMarginsF(settings->GetTiledPDFMargins(Unit::Mm));
        if(settings->getTiledPDFOrientation() == PageOrientation::Landscape)
        {
            // because when painting we have a -90rotation in landscape modus,
            // see function PrintPages.
            left = pageMargin.bottom();
            top = pageMargin.left();
            right = pageMargin.top();
            bottom = pageMargin.right();
        }
        else
        {
            left = pageMargin.left();
            top = pageMargin.top();
            right = pageMargin.right();
            bottom = pageMargin.bottom();
        }
    }

    const bool success = printer->setPageMargins(QMarginsF(left, top, right, bottom), QPageLayout::Millimeter);
    if (!success)
    {
        qWarning() << tr("Cannot set printer margins");
    }

    switch(printType)
    {
        case PrintType::PrintPDF:
        {
            const QString outputFileName = QDir::homePath() + QDir::separator() + FileName();
            #ifdef Q_OS_WIN
            printer->setOutputFileName(outputFileName);
            #else
            printer->setOutputFileName(outputFileName + QLatin1String(".pdf"));
            #endif

            #ifdef Q_OS_MAC
            printer->setOutputFormat(QPrinter::NativeFormat);
            #else
            printer->setOutputFormat(QPrinter::PdfFormat);
            #endif
            break;
        }
        case PrintType::PrintNative:
            printer->setOutputFileName("");//Disable printing to file if was enabled.
            printer->setOutputFormat(QPrinter::NativeFormat);
            break;
        case PrintType::PrintPreview: /*do nothing*/
        default:
            break;
    }

    printer->setDocName(FileName());

    IsLayoutGrayscale() ? printer->setColorMode(QPrinter::GrayScale) : printer->setColorMode(QPrinter::Color);
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindowsNoGUI::IsLayoutGrayscale() const
{
    const QRect target = QRect(0, 0, 100, 100);//Small image less memory need

    for (int i=0; i < scenes.size(); ++i)
    {
        auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
        if (paper)
        {
            // Hide shadow and paper border
            PreparePaper(i);

            // Render png
            QImage image(target.size(), QImage::Format_RGB32);
            image.fill(Qt::white);
            QPainter painter(&image);
            painter.setPen(QPen(Qt::black, widthMainLine, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter.setBrush ( QBrush ( Qt::NoBrush ) );
            scenes.at(i)->render(&painter, target, paper->rect(), Qt::KeepAspectRatio);
            painter.end();

            // Restore
            RestorePaper(i);

            if (!image.isGrayscale())
            {
                return false;
            }
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MainWindowsNoGUI::FindTemplate
 * @param size has to be in Mm
 * @return
 */
QPageSize MainWindowsNoGUI::FindQPrinterPageSize(const QSizeF &size) const
{
    if (size == QSizeF(841, 1189))
    {
        return QPageSize(QPageSize::A0);
    }

    if (size == QSizeF(594, 841))
    {
        return QPageSize(QPageSize::A1);
    }

    if (size == QSizeF(420, 594))
    {
        return QPageSize(QPageSize::A2);
    }

    if (size == QSizeF(297, 420))
    {
        return QPageSize(QPageSize::A3);
    }

    if (size == QSizeF(210, 297))
    {
        return QPageSize(QPageSize::A4);
    }

    if (size == QSizeF(215.9, 355.6))
    {
        return QPageSize(QPageSize::Legal);
    }

    if (size == QSizeF(215.9, 279.4))
    {
        return QPageSize(QPageSize::Letter);
    }

    return QPageSize(QPageSize::Custom);
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindowsNoGUI::isPagesUniform() const
{
    if (papers.size() < 2)
    {
        return true;
    }
    else
    {
        auto *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(0));
        SCASSERT(paper != nullptr)
        for (int i=1; i < papers.size(); ++i)
        {
            auto *p = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
            SCASSERT(p != nullptr)
            if (paper->rect() != p->rect())
            {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindowsNoGUI::IsPagesFit(const QSizeF &printPaper) const
{
    // On previous stage already was checked if pages have uniform size
    // Enough will be to check only one page
    QGraphicsRectItem *p = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(0));
    SCASSERT(p != nullptr)
    const QSizeF pSize = p->rect().size();
    if (pSize.height() <= printPaper.height() && pSize.width() <= printPaper.width())
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::ExportScene(const ExportLayoutDialog &dialog, const QList<QGraphicsScene *> &scenes,
                                   const QList<QGraphicsItem *> &papers, const QList<QGraphicsItem *> &shadows,
                                   const QList<QList<QGraphicsItem *> > &pieces, bool ignoreMargins,
                                   const QMarginsF &margins) const
{
    for (int i=0; i < scenes.size(); ++i)
    {
        QString increment  = QStringLiteral("");
        QGraphicsRectItem *paper = qgraphicsitem_cast<QGraphicsRectItem *>(papers.at(i));
        if (paper)
        {
            if (dialog.mode() == Draw::Layout)
            {
                increment = QStringLiteral("_0") + QString::number(i+1);
            }
            const QString name = QString("%1/%2%3%4")
            .arg(dialog.path())                                          //1
            .arg(dialog.fileName())                                      //2
            .arg(increment)                                              //3
            .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format())); //4

            QBrush *brush = new QBrush();
            brush->setColor( QColor( Qt::white ) );
            QGraphicsScene *scene = scenes.at(i);
            scene->setBackgroundBrush( *brush );
            shadows[i]->setVisible(false);
            paper->setPen(QPen(QBrush(Qt::white, Qt::NoBrush), 0.1, Qt::NoPen));

            switch (dialog.format())
            {
                case LayoutExportFormat::SVG:
                    paper->setVisible(false);
                    exportSVG(name, paper, scene);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::PDF:
                    exportPDF(name, paper, scene, ignoreMargins, margins);
                    break;
                case LayoutExportFormat::PNG:
                    exportPNG(name, scene);
                    break;
                case LayoutExportFormat::JPG:
                    exportJPG(name, scene);
                    break;
                case LayoutExportFormat::BMP:
                    exportBMP(name, scene);
                    break;
                case LayoutExportFormat::TIF:
                    exportTIF(name, scene);
                    break;
                case LayoutExportFormat::PPM:
                    exportPPM(name, scene);
                    break;
                case LayoutExportFormat::OBJ:
                    paper->setVisible(false);
                    ObjFile(name, paper, scene);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::PS:
                    exportPS(name, paper, scene, ignoreMargins, margins);
                    break;
                case LayoutExportFormat::EPS:
                    exportEPS(name, paper, scene, ignoreMargins, margins);
                    break;
                case LayoutExportFormat::DXF_AC1006_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1006, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1009_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1009, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1012_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1012, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1014_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1014, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1015_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1015, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1018_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1018, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1021_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1021, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1024_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1024, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                case LayoutExportFormat::DXF_AC1027_Flat:
                    paper->setVisible(false);
                    FlatDxfFile(name, DRW::AC1027, dialog.isBinaryDXFFormat(), paper, scene, pieces);
                    paper->setVisible(true);
                    break;
                default:
                    qWarning() << "Can't recognize file type." << Q_FUNC_INFO;
                    break;
            }
            paper->setPen(QPen(Qt::black, 1));
            brush->setColor( QColor( Qt::gray ) );
            brush->setStyle( Qt::SolidPattern );
            scenes[i]->setBackgroundBrush( *brush );
            shadows[i]->setVisible(true);
            delete brush;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MainWindowsNoGUI::FileName() const
{
    QString fileName;
    qApp->getFilePath().isEmpty() ? fileName = tr("unnamed") : fileName = qApp->getFilePath();
    return QFileInfo(fileName).baseName();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindowsNoGUI::setSizeHeightForIndividualM() const
{
    const QHash<QString, QSharedPointer<VInternalVariable> > * vars = pattern->DataVariables();

    if (vars->contains(size_M))
    {
        VContainer::setSize(*vars->value(size_M)->GetValue());
    }
    else
    {
        VContainer::setSize(0);
    }

    if (vars->contains(height_M))
    {
        VContainer::setHeight(*vars->value(height_M)->GetValue());
    }
    else
    {
        VContainer::setHeight(0);
    }

    doc->SetPatternWasChanged(true);
    emit doc->UpdatePatternLabel();
}

//---------------------------------------------------------------------------------------------------------------------
int MainWindowsNoGUI::ContinueIfLayoutStale()
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setWindowTitle(tr("The layout is stale."));
    msgBox.setText(tr("The layout was not updated since last pattern modification. Do you want to continue?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = static_cast<QGridLayout*>(msgBox.layout());
    SCASSERT(layout != nullptr)
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());
    return msgBox.exec();
}

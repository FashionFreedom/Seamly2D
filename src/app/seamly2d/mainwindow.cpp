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
 **  @file   mainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../vgeometry/vspline.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/exception/vexceptionundo.h"
#include "version.h"
#include "core/vapplication.h"
#include "../vmisc/customevents.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/def.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "undocommands/renamepp.h"
#include "core/vtooloptionspropertybrowser.h"
#include "options.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vmisc/logging.h"
#include "../vformat/vmeasurements.h"
#include "../ifc/xml/vvstconverter.h"
#include "../ifc/xml/vvitconverter.h"
#include "../vwidgets/vwidgetpopup.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/mouse_coordinates.h"
#include "../vtools/tools/drawTools/drawtools.h"
#include "../vtools/dialogs/tooldialogs.h"
#include "tools/vtoolseamallowance.h"
#include "tools/nodeDetails/vtoolinternalpath.h"
#include "tools/nodeDetails/vtoolpin.h"
#include "tools/vtooluniondetails.h"
#include "dialogs/dialogs.h"

#include "../vtools/undocommands/addgroup.h"
#include "../vpatterndb/vpiecepath.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vtools/dialogs/support/dialogeditlabel.h"

#include <QInputDialog>
#include <QtDebug>
#include <QMessageBox>
#include <QShowEvent>
#include <QScrollBar>
#include <QFileDialog>
#include <QSourceLocation>
#include <QUndoStack>
#include <QAction>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <chrono>
#include <thread>
#include <QFileSystemWatcher>
#include <QComboBox>
#include <QFontComboBox>
#include <QTextCodec>
#include <QDoubleSpinBox>

#if defined(Q_OS_MAC)
#include <QMimeData>
#include <QDrag>
#endif //defined(Q_OS_MAC)

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vMainWindow, "v.mainwindow")

QT_WARNING_POP

const QString autosavePrefix = QStringLiteral(".autosave");

// String below need for getting translation for key Ctrl
const QString strQShortcut   = QStringLiteral("QShortcut"); // Context
const QString strCtrl        = QStringLiteral("Ctrl"); // String

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MainWindow constructor.
 * @param parent parent widget.
 */
MainWindow::MainWindow(QWidget *parent)
    : MainWindowsNoGUI(parent)
    , ui(new Ui::MainWindow)
    , watcher(new QFileSystemWatcher(this))
    , currentTool(Tool::Arrow)
    , lastUsedTool(Tool::Arrow)
    , draftScene(nullptr)
    , pieceScene(nullptr)
    , mouseCoordinates(nullptr)
    , infoToolButton(nullptr)
    , helpLabel(nullptr)
    , isInitialized(false)
    , mChanges(false)
    , mChangesAsked(true)
    , patternReadOnly(false)
    , dialogTable(nullptr)
    , dialogTool()
    , dialogHistory(nullptr)
    , fontComboBox(nullptr)
    , fontSizeComboBox(nullptr)
    , comboBoxDraws(nullptr)
    , patternPieceLabel(nullptr)
    , mode(Draw::Calculation)
    , currentDrawIndex(0)
    , currentToolBoxIndex(0)
    , isToolOptionsDockVisible(true)
    , isGroupsDockVisible(true)
    , isLayoutsDockVisible(false)
    , isToolboxDockVisible(true)
    , drawMode(true)
    , recentFileActs()
    , separatorAct(nullptr)
    , leftGoToStage(nullptr)
    , rightGoToStage(nullptr)
    , autoSaveTimer(nullptr)
    , guiEnabled(true)
    , gradationHeights(nullptr)
    , gradationSizes(nullptr)
    , gradationHeightsLabel(nullptr)
    , gradationSizesLabel(nullptr)
    , toolProperties(nullptr)
    , groupsWidget(nullptr)
    , patternPiecesWidget(nullptr)
    , lock(nullptr)
    , zoomScaleSpinBox(nullptr)
{
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs[i] = nullptr;
    }

    CreateActions();
    InitScenes();

    doc = new VPattern(pattern, &mode, draftScene, pieceScene);
    connect(doc, &VPattern::ClearMainWindow, this, &MainWindow::Clear);
    connect(doc, &VPattern::patternChanged, this, &MainWindow::PatternChangesWereSaved);
    connect(doc, &VPattern::UndoCommand, this, &MainWindow::FullParseFile);
    connect(doc, &VPattern::SetEnabledGUI, this, &MainWindow::SetEnabledGUI);
    connect(doc, &VPattern::CheckLayout, this, [this]()
    {
        if (pattern->DataPieces()->count() == 0)
        {
            if(not ui->draftMode_Action->isChecked())
            {
                draftMode_Action(true);
            }
        }
    });
    connect(doc, &VPattern::setCurrentDraftBlock, this, &MainWindow::GlobalchangeDraftBlock);
    qApp->setCurrentDocument(doc);

    InitDocksContain();
    CreateMenus();
    initDraftToolBar();
    initPointNameToolBar();
    initModesToolBar();
    InitToolButtons();

    helpLabel = new QLabel(QObject::tr("Create new pattern piece to start working."));
    ui->statusBar->addWidget(helpLabel);

    initToolsToolBar();

    connect(qApp->getUndoStack(), &QUndoStack::cleanChanged, this, &MainWindow::PatternChangesWereSaved);

    InitAutoSave();

    ui->draft_ToolBox->setCurrentIndex(0);

    ReadSettings();
    initToolBarVisibility();

    setCurrentFile("");
    WindowsLocale();

    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::showLayoutPages);
    ui->layoutPages_DockWidget->setVisible(false);

    connect(watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::MeasurementsChanged);
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget *old, QWidget *now)
    {
        if (old == nullptr && isAncestorOf(now) == true)
        {// focus IN
            static bool asking = false;
            if (not asking && mChanges && not mChangesAsked)
            {
                asking = true;
                mChangesAsked = true;
                const auto answer = QMessageBox::question(this, tr("Measurements"),
                                                 tr("Measurements were changed. Do you want to sync measurements now?"),
                                                          QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
                if (answer == QMessageBox::Yes)
                {
                    SyncMeasurements();
                }
                asking = false;
            }
        }

        // In case we will need it
        // else if (isAncestorOf(old) == true && now == nullptr)
        // focus OUT
    });

#if defined(Q_OS_MAC)
    // On Mac default icon size is 32x32.
    ui->draft_ToolBar->setIconSize(QSize(24, 24));
    ui->mode_ToolBar->setIconSize(QSize(24, 24));
    ui->edit_Toolbar->setIconSize(QSize(24, 24));
    ui->zoom_ToolBar->setIconSize(QSize(24, 24));

    setUnifiedTitleAndToolBarOnMac(true);

    // Mac OS Dock Menu
    QMenu *menu = new QMenu(this);

    QAction *newPattern_Action = menu->addAction(tr("New pattern"));
    newPattern_Action->setMenuRole(QAction::NoRole);
    connect(newPattern_Action, &QAction::triggered, this, &MainWindow::New);

    QAction *openPattern_Action = menu->addAction(tr("Open pattern"));
    openPattern_Action->setMenuRole(QAction::NoRole);
    connect(openPattern_Action, &QAction::triggered, this, &MainWindow::Open);

    QAction *openSeamlyMe_Action = menu->addAction(tr("Create/Edit measurements"));
    openSeamlyMe_Action->setMenuRole(QAction::NoRole);
    connect(openSeamlyMe_Action, &QAction::triggered, this, &MainWindow::CreateMeasurements);

    QAction *appPreferences_Action = menu->addAction(tr("Preferences"));
    appPreferences_Action->setMenuRole(QAction::NoRole);
    connect(appPreferences_Action, &QAction::triggered, this, &MainWindow::Preferences);

    menu->setAsDockMenu();
#endif //defined(Q_OS_MAC)
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AddPP(const QString &PPName)
{
    if (doc->appendPP(PPName) == false)
    {
        qCDebug(vMainWindow, "Error creating pattern piece with the name %s.", qUtf8Printable(PPName));
        return;
    }

    if (comboBoxDraws->count() == 0)
    {
        draftScene->InitOrigins();
        draftScene->enablePiecesMode(qApp->Seamly2DSettings()->getShowControlPoints());
        pieceScene->InitOrigins();
    }

    comboBoxDraws->blockSignals(true);
    comboBoxDraws->addItem(PPName);

    pattern->ClearGObjects();
    //Create single point
    ui->view->itemClicked(nullptr);//hide options previous tool
    const QString label = doc->GenerateLabel(LabelType::NewPatternPiece);
    const QPointF startPosition = StartPositionNewPP();
    VPointF *point = new VPointF(startPosition.x(), startPosition.y(), label, 5, 10);
    auto spoint = VToolBasePoint::Create(0, PPName, point, draftScene, doc, pattern, Document::FullParse,
                                         Source::FromGui);
    ui->view->itemClicked(spoint);

    setEnableTools(true);
    SetEnableWidgets(true);

    const qint32 index = comboBoxDraws->findText(PPName);
    if ( index != -1 )
    { // -1 for not found
        comboBoxDraws->setCurrentIndex(index);
    }
    else
    {
        comboBoxDraws->setCurrentIndex(0);
    }
    comboBoxDraws->blockSignals(false);

    // Show best for new PP
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view, spoint);
    ui->view->zoom100Percent();

    ui->newDraft_Action->setEnabled(true);
    helpLabel->setText("");
}

//---------------------------------------------------------------------------------------------------------------------
QPointF MainWindow::StartPositionNewPP() const
{
    const qreal originX = 30.0;
    const qreal originY = 40.0;
    const qreal margin = 40.0;
    if (comboBoxDraws->count() > 1)
    {
        const QRectF rect = draftScene->visibleItemsBoundingRect();
        if (rect.width() <= rect.height())
        {
            return QPointF(rect.width()+margin, originY);
        }
        else
        {
            return QPointF(originX, rect.height()+margin);
        }
    }
    else
    {
        return QPointF(originX, originY);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitScenes()
{
    draftScene = new VMainGraphicsScene(this);
    currentScene = draftScene;
    qApp->setCurrentScene(&currentScene);
    connect(this, &MainWindow::EnableItemMove, draftScene, &VMainGraphicsScene::EnableItemMove);
    connect(this, &MainWindow::ItemsSelection, draftScene, &VMainGraphicsScene::ItemsSelection);

    connect(this, &MainWindow::EnableLabelSelection, draftScene, &VMainGraphicsScene::ToggleLabelSelection);
    connect(this, &MainWindow::EnablePointSelection, draftScene, &VMainGraphicsScene::TogglePointSelection);
    connect(this, &MainWindow::EnableLineSelection, draftScene, &VMainGraphicsScene::ToggleLineSelection);
    connect(this, &MainWindow::EnableArcSelection, draftScene, &VMainGraphicsScene::ToggleArcSelection);
    connect(this, &MainWindow::EnableElArcSelection, draftScene, &VMainGraphicsScene::ToggleElArcSelection);
    connect(this, &MainWindow::EnableSplineSelection, draftScene, &VMainGraphicsScene::ToggleSplineSelection);
    connect(this, &MainWindow::EnableSplinePathSelection, draftScene, &VMainGraphicsScene::ToggleSplinePathSelection);

    connect(this, &MainWindow::EnableLabelHover, draftScene, &VMainGraphicsScene::ToggleLabelHover);
    connect(this, &MainWindow::EnablePointHover, draftScene, &VMainGraphicsScene::TogglePointHover);
    connect(this, &MainWindow::EnableLineHover, draftScene, &VMainGraphicsScene::ToggleLineHover);
    connect(this, &MainWindow::EnableArcHover, draftScene, &VMainGraphicsScene::ToggleArcHover);
    connect(this, &MainWindow::EnableElArcHover, draftScene, &VMainGraphicsScene::ToggleElArcHover);
    connect(this, &MainWindow::EnableSplineHover, draftScene, &VMainGraphicsScene::ToggleSplineHover);
    connect(this, &MainWindow::EnableSplinePathHover, draftScene, &VMainGraphicsScene::ToggleSplinePathHover);

    connect(draftScene, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);

    pieceScene = new VMainGraphicsScene(this);
    connect(this, &MainWindow::EnableItemMove, pieceScene, &VMainGraphicsScene::EnableItemMove);

    connect(this, &MainWindow::EnableNodeLabelSelection, pieceScene, &VMainGraphicsScene::ToggleNodeLabelSelection);
    connect(this, &MainWindow::EnableNodePointSelection, pieceScene, &VMainGraphicsScene::ToggleNodePointSelection);
    connect(this, &MainWindow::EnableDetailSelection, pieceScene, &VMainGraphicsScene::ToggleDetailSelection);

    connect(this, &MainWindow::EnableNodeLabelHover, pieceScene, &VMainGraphicsScene::ToggleNodeLabelHover);
    connect(this, &MainWindow::EnableNodePointHover, pieceScene, &VMainGraphicsScene::ToggleNodePointHover);
    connect(this, &MainWindow::EnableDetailHover, pieceScene, &VMainGraphicsScene::ToggleDetailHover);

    connect(pieceScene, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);

    ui->view->setScene(currentScene);

    draftScene->setCurrentTransform(ui->view->transform());
    pieceScene->setCurrentTransform(ui->view->transform());

    connect(ui->view, &VMainGraphicsView::mouseRelease, this, [this](){EndVisualization(true);});
    connect(ui->view, &VMainGraphicsView::signalZoomScaleChanged, this, &MainWindow::zoomScaleChanged);

    QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    policy.setHorizontalStretch(12);
    ui->view->setSizePolicy(policy);
    qApp->setSceneView(ui->view);
}

//---------------------------------------------------------------------------------------------------------------------
QSharedPointer<VMeasurements> MainWindow::OpenMeasurementFile(const QString &path)
{
    QSharedPointer<VMeasurements> measurements;
    if (path.isEmpty())
    {
        return measurements;
    }

    try
    {
        measurements = QSharedPointer<VMeasurements>(new VMeasurements(pattern));
        measurements->SetSize(VContainer::rsize());
        measurements->SetHeight(VContainer::rheight());
        measurements->setXMLContent(path);

        if (measurements->Type() == MeasurementsType::Unknown)
        {
            VException e(tr("Measurement file has unknown format."));
            throw e;
        }

        if (measurements->Type() == MeasurementsType::Multisize)
        {
            VVSTConverter converter(path);
            measurements->setXMLContent(converter.Convert());// Read again after conversion
        }
        else
        {
            VVITConverter converter(path);
            measurements->setXMLContent(converter.Convert());// Read again after conversion
        }

        if (not measurements->IsDefinedKnownNamesValid())
        {
            VException e(tr("Measurement file contains invalid known measurement(s)."));
            throw e;
        }

        CheckRequiredMeasurements(measurements.data());

        if (measurements->Type() == MeasurementsType::Multisize)
        {
            if (measurements->MUnit() == Unit::Inch)
            {
                qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Wrong units.")),
                          qUtf8Printable(tr("Application doesn't support multisize table with inches.")));
                measurements->clear();
                if (not VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_DATAERR);
                }
                return measurements;
            }
        }
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        measurements->clear();
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return measurements;
    }
    return measurements;
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::LoadMeasurements(const QString &path)
{
    QSharedPointer<VMeasurements> measurements = OpenMeasurementFile(path);

    if (measurements->isNull())
    {
        return false;
    }

    if (qApp->patternUnit() == Unit::Inch && measurements->Type() == MeasurementsType::Multisize)
    {
        qWarning()<<tr("Gradation doesn't support inches");
        return false;
    }

    try
    {
        qApp->setPatternType(measurements->Type());
        initStatusBar();
        pattern->ClearVariables(VarType::Measurement);
        measurements->ReadMeasurements();
    }
    catch (VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    if (measurements->Type() == MeasurementsType::Multisize)
    {
        VContainer::SetSize(UnitConvertor(measurements->BaseSize(), measurements->MUnit(),
                                          *measurements->GetData()->GetPatternUnit()));
        VContainer::SetHeight(UnitConvertor(measurements->BaseHeight(), measurements->MUnit(),
                                            *measurements->GetData()->GetPatternUnit()));

        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
    else if (measurements->Type() == MeasurementsType::Individual)
    {
        SetSizeHeightForIndividualM();
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::UpdateMeasurements(const QString &path, int size, int height)
{
    QSharedPointer<VMeasurements> measurements = OpenMeasurementFile(path);

    if (measurements->isNull())
    {
        return false;
    }

    if (qApp->patternType() != measurements->Type())
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Measurement files types have not match.")));
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_DATAERR);
        }
        return false;
    }

    try
    {
        pattern->ClearVariables(VarType::Measurement);
        measurements->ReadMeasurements();
    }
    catch (VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    if (measurements->Type() == MeasurementsType::Multisize)
    {
        VContainer::SetSize(size);
        VContainer::SetHeight(height);

        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
    else if (measurements->Type() == MeasurementsType::Individual)
    {
        SetSizeHeightForIndividualM();
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CheckRequiredMeasurements(const VMeasurements *measurements)
{
    auto tempMeasurements = measurements->ListAll();
    auto docMeasurements = doc->ListMeasurements();
    const QSet<QString> match = QSet<QString>(docMeasurements.begin(), docMeasurements.end()).
                                    subtract(QSet<QString>(tempMeasurements.begin(), tempMeasurements.end()));
    if (not match.isEmpty())
    {
		QList<QString> list = match.values();
        for (int i = 0; i < list.size(); ++i)
        {
            list[i] = qApp->TrVars()->MToUser(list.at(i));
        }

        VException e(tr("Measurement file doesn't include all the required measurements."));
        e.AddMoreInformation(tr("Please, additionally provide: %1").arg(QStringList(list).join(", ")));
        throw e;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetToolButton set tool and show dialog.
 * @param checked true if tool button checked.
 * @param t tool type.
 * @param cursor path tool cursor icon.
 * @param toolTip first tooltipe.
 * @param closeDialogSlot function what handle after close dialog.
 */
template <typename Dialog, typename Func>
void MainWindow::SetToolButton(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                               Func closeDialogSlot)
{
    if (checked)
    {
        CancelTool();
        emit EnableItemMove(false);
        currentTool = lastUsedTool = t;
        auto cursorResource = cursor;
        if (qApp->devicePixelRatio() >= 2)
        {
            // Try to load HiDPI versions of the cursors if available
            auto cursorHidpiResource = QString(cursor).replace(".png", "@2x.png");
            if (QFileInfo(cursorResource).exists())
            {
                cursorResource = cursorHidpiResource;
            }
        }
        QPixmap pixmap(cursorResource);
        QCursor cur(pixmap, 2, 2);
        ui->view->viewport()->setCursor(cur);
        helpLabel->setText(toolTip);
        ui->view->setShowToolOptions(false);
        dialogTool = QSharedPointer<Dialog>(new Dialog(pattern, 0, this));

        switch(t)
        {
            case Tool::Midpoint:
                dialogTool->Build(t);
                break;
            case Tool::InternalPath:
            case Tool::Pin:
            case Tool::InsertNodes:
                dialogTool->SetPiecesList(doc->GetActivePPPieces());
                break;
            default:
                break;
        }

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        connect(scene, &VMainGraphicsScene::ChosenObject, dialogTool.data(), &DialogTool::ChosenObject);
        connect(scene, &VMainGraphicsScene::SelectedObject, dialogTool.data(), &DialogTool::SelectedObject);
        connect(dialogTool.data(), &DialogTool::DialogClosed, this, closeDialogSlot);
        connect(dialogTool.data(), &DialogTool::ToolTip, this, &MainWindow::ShowToolTip);
        ui->view->itemClicked(nullptr);
    }
    else
    {
        if (QToolButton *tButton = qobject_cast< QToolButton * >(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename Dialog, typename Func, typename Func2>
/**
 * @brief SetToolButtonWithApply set tool and show dialog.
 * @param checked true if tool button checked.
 * @param t tool type.
 * @param cursor path tool cursor icon.
 * @param toolTip first tooltipe.
 * @param closeDialogSlot function to handle close of dialog.
 * @param applyDialogSlot function to handle apply in dialog.
 */
void MainWindow::SetToolButtonWithApply(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                                        Func closeDialogSlot, Func2 applyDialogSlot)
{
    if (checked)
    {
        SetToolButton<Dialog>(checked, t, cursor, toolTip, closeDialogSlot);

        connect(dialogTool.data(), &DialogTool::DialogApplied, this, applyDialogSlot);
    }
    else
    {
        if (QToolButton *tButton = qobject_cast< QToolButton * >(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialog handle close dialog
 * @param result result working dialog.
 */
template <typename DrawTool>
void MainWindow::ClosedDialog(int result)
{
    SCASSERT(not dialogTool.isNull())
    if (result == QDialog::Accepted)
    {
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        QGraphicsItem *tool = dynamic_cast<QGraphicsItem *>(DrawTool::Create(dialogTool, scene, doc, pattern));
        // Do not check for nullptr! See issue #719.
        ui->view->itemClicked(tool);
    }
    handleArrowTool(true);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialogWithApply handle close dialog that has apply button
 * @param result result working dialog.
 */
template <typename DrawTool>
void MainWindow::ClosedDialogWithApply(int result, VMainGraphicsScene *scene)
{
    SCASSERT(not dialogTool.isNull())
    if (result == QDialog::Accepted)
    {
        ApplyDialog<DrawTool>(scene);
    }
    // If before Cancel was used Apply we have an item
    DrawTool *vtool = qobject_cast<DrawTool *>(dialogTool->GetAssociatedTool());// Don't check for nullptr here
    if (dialogTool->GetAssociatedTool() != nullptr)
    {
        SCASSERT(vtool != nullptr)
        vtool->DialogLinkDestroy();
        connect(vtool, &DrawTool::ToolTip, this, &MainWindow::ShowToolTip);
    }
    handleArrowTool(true);
    ui->view->itemClicked(vtool);// Don't check for nullptr here
    // If insert not to the end of file call lite parse
    if (doc->getCursor() > 0)
    {
        doc->LiteParseTree(Document::LiteParse);
        if (dialogHistory)
        {
            dialogHistory->updateHistory();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ApplyDialog handle apply in dialog
 */
template <typename DrawTool>
void MainWindow::ApplyDialog(VMainGraphicsScene *scene)
{
    SCASSERT(not dialogTool.isNull())

    // Only create tool if not already created with apply
    if (dialogTool->GetAssociatedTool() == nullptr)
    {
        SCASSERT(scene != nullptr)

        dialogTool->SetAssociatedTool(DrawTool::Create(dialogTool, scene, doc, pattern));
    }
    else
    { // Or update associated tool with data
        DrawTool * vtool = qobject_cast<DrawTool *>(dialogTool->GetAssociatedTool());
        SCASSERT(vtool != nullptr)
        vtool->FullUpdateFromGuiApply();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool>
void MainWindow::ClosedDrawDialogWithApply(int result)
{
    ClosedDialogWithApply<DrawTool>(result, draftScene);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool>
void MainWindow::ApplyDrawDialog()
{
    ApplyDialog<DrawTool>(draftScene);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool>
void MainWindow::ClosedDetailsDialogWithApply(int result)
{
    ClosedDialogWithApply<DrawTool>(result, pieceScene);
    if (pattern->DataPieces()->size() > 0)
    {
        ui->anchorPoint_ToolButton->setEnabled(true);
        ui->internalPath_ToolButton->setEnabled(true);
        ui->insertNodes_ToolButton->setEnabled(true);
        ui->anchorPoint_Action->setEnabled(true);
        ui->internalPath_Action->setEnabled(true);
        ui->insertNodes_Action->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename DrawTool>
void MainWindow::ApplyDetailsDialog()
{
    ApplyDialog<DrawTool>(pieceScene);
}

//Points
//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleMidpointTool(bool checked)
{
    ToolSelectPointByRelease();
    // Reuse DialogAlongLine and VToolAlongLine but with different cursor
    SetToolButtonWithApply<DialogAlongLine>
    (
        checked,
        Tool::Midpoint,
        ":/cursor/midpoint_cursor.png",
        tr("<b>Tool::Points - Midpoint along Line</b>: Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
        &MainWindow::ApplyDrawDialog<VToolAlongLine>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointAtDistanceAngleTool handler for handlePointAtDistanceAngle tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePointAtDistanceAngleTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogEndLine>
    (
        checked,
        Tool::EndLine,
        ":/cursor/endline_cursor.png",
        tr("<b>Tool::Points - Point at Distance & Angle</b>: Select point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolEndLine>,
        &MainWindow::ApplyDrawDialog<VToolEndLine>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleAlongLineTool handler for point along Line tools.
 * @param checked true - button checked.
 */
void MainWindow::handleAlongLineTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogAlongLine>
    (
        checked,
        Tool::AlongLine,
        ":/cursor/alongline_cursor.png",
        tr("<b>Tool::Points - Point along Line:</b> Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
        &MainWindow::ApplyDrawDialog<VToolAlongLine>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleNormalTool handler point on perpendicular tool.
 * @param checked true - button checked.
 */
void MainWindow::handleNormalTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogNormal>
    (
        checked,
        Tool::Normal,
        ":/cursor/normal_cursor.png",
        tr("<b>Tool::Points - Point on Perpendicular:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolNormal>,
        &MainWindow::ApplyDrawDialog<VToolNormal>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleBisectorTool handler for bisector tool.
 * @param checked true - button checked.
 */
void MainWindow::handleBisectorTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogBisector>
    (
        checked,
        Tool::Bisector,
        ":/cursor/bisector_cursor.png",
        tr("<b>Tool::Points - Point along Bisector:</b> Select first point of angle"),
        &MainWindow::ClosedDrawDialogWithApply<VToolBisector>,
        &MainWindow::ApplyDrawDialog<VToolBisector>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleShoulderPointTool handler for shoulderPoint tool.
 * @param checked true - button checked.
 */
void MainWindow::handleShoulderPointTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogShoulderPoint>
    (
        checked,
        Tool::ShoulderPoint,
        ":/cursor/shoulder_cursor.png",
        tr("<b>Tool::Points - Shoulder Point:</b> Select point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolShoulderPoint>,
        &MainWindow::ApplyDrawDialog<VToolShoulderPoint>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointOfContactTool handler for pointOfContact tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePointOfContactTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogPointOfContact>
    (
        checked, Tool::PointOfContact,
        ":/cursor/pointcontact_cursor.png",
        tr("<b>Tool::Points - Intersection Point of Line and Arc:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfContact>,
        &MainWindow::ApplyDrawDialog<VToolPointOfContact>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleTriangleTool handler tool triangle.
 * @param checked true - button checked.
 */
void MainWindow::handleTriangleTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogTriangle>
    (
        checked,
        Tool::Triangle,
        ":/cursor/triangle_cursor.png",
        tr("<b>Tool::Points - Triangle:</b> Select first point of axis"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTriangle>,
        &MainWindow::ApplyDrawDialog<VToolTriangle>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointIntersectXYTool handler for pointOfIntersection tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePointIntersectXYTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<PointIntersectXYDialog>
    (
        checked,
        Tool::PointOfIntersection,
        ":/cursor/pointofintersect_cursor.png",
        tr("<b>Tool::Points - Intersection Point XY from 2 Points:</b> Select point for X value (vertical)"),
        &MainWindow::ClosedDrawDialogWithApply<PointIntersectXYTool>,
        &MainWindow::ApplyDrawDialog<PointIntersectXYTool>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleHeightTool handler tool height.
 * @param checked true - button checked.
 */
void MainWindow::handleHeightTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogHeight>
    (
        checked,
        Tool::Height,
        ":/cursor/height_cursor.png",
        tr("<b>Tool::Points - Intersection Point of Line and Perpendicular:</b> Select base point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolHeight>,
        &MainWindow::ApplyDrawDialog<VToolHeight>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleLineIntersectAxisTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLineIntersectAxis>
    (
        checked,
        Tool::LineIntersectAxis,
        ":/cursor/line_intersect_axis_cursor.png",
        tr("<b>Tool::Points - Intersection Point of Line and Axis:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolLineIntersectAxis>
    );
}

//Lines
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleLineTool handler for line tool.
 * @param checked true - button checked.
 */
void MainWindow::handleLineTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLine>
    (
        checked,
        Tool::Line,
        ":/cursor/line_cursor.png",
        tr("<b>Tool::Lines - Line:</b>:Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLine>,
        &MainWindow::ApplyDrawDialog<VToolLine>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleLineIntersectTool handler for lineIntersect tool.
 * @param checked true - button checked.
 */
void MainWindow::handleLineIntersectTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogLineIntersect>
    (
        checked,
        Tool::LineIntersect,
        ":/cursor/intersect_cursor.png",
        tr("<b>Tool::Lines - Intersection Point of 2 Lines:</b> Select first point of first line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersect>,
        &MainWindow::ApplyDrawDialog<VToolLineIntersect>
    );
}

//Curves
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleCurveTool handler for curve tool.
 * @param checked true - button checked.
 */
void MainWindow::handleCurveTool(bool checked)
{
    ToolSelectPointByPress();
    SetToolButtonWithApply<DialogSpline>
    (
        checked,
        Tool::Spline,
        ":/cursor/spline_cursor.png",
        tr("<b>Tool::Curves - Curve:</b> Select start point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSpline>,
        &MainWindow::ApplyDrawDialog<VToolSpline>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleSplineTool handler for spline tool.
 * @param checked true - button checked.
 */
void MainWindow::handleSplineTool(bool checked)
{
    ToolSelectPointByPress();
    SetToolButtonWithApply<DialogSplinePath>
    (
        checked,
        Tool::SplinePath,
        ":/cursor/splinepath_cursor.png",
        tr("<b>Tool::Curves - Spline:</b> Select start point of spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSplinePath>,
        &MainWindow::ApplyDrawDialog<VToolSplinePath>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleCurveWithControlPointsTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogCubicBezier>
    (
        checked,
        Tool::CubicBezier,
        ":/cursor/cubic_bezier_cursor.png",
        tr("<b>Tool::Curves - Curve with Control Points:</b> Select first point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezier>,
        &MainWindow::ApplyDrawDialog<VToolCubicBezier>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleSplineWithControlPointsTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogCubicBezierPath>
    (
        checked,
        Tool::CubicBezierPath,
        ":/cursor/cubic_bezier_path_cursor.png",
        tr("<b>Tool::Curves - Spline with Control Points:</b> Select first point of spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezierPath>,
        &MainWindow::ApplyDrawDialog<VToolCubicBezierPath>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointAlongCurveTool handler for point along curve tool.
 * @param checked true - button is checked
 */
void MainWindow::handlePointAlongCurveTool(bool checked)
{
    ToolSelectSpline();
    SetToolButtonWithApply<DialogCutSpline>
    (
        checked,
        Tool::CutSpline,
        ":/cursor/spline_cut_point_cursor.png",
        tr("<b>Tool::Curves - Point along Curve:</b> Select first point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSpline>,
        &MainWindow::ApplyDrawDialog<VToolCutSpline>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointAlongSplineTool handler for point along spline tool.
 * @param checked true - button is checked
 */
void MainWindow::handlePointAlongSplineTool(bool checked)
{
    ToolSelectSplinePath();
    SetToolButtonWithApply<DialogCutSplinePath>
    (
        checked,
        Tool::CutSplinePath,
        ":/cursor/splinepath_cut_point_cursor.png",
        tr("<b>Tool::Curves - Point along Spline:</b> Select spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSplinePath>,
        &MainWindow::ApplyDrawDialog<VToolCutSplinePath>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleCurveIntersectCurveTool(bool checked)
{
    ToolSelectCurve();
    SetToolButtonWithApply<DialogPointOfIntersectionCurves>
    (
        checked,
        Tool::PointOfIntersectionCurves,
        "://cursor/intersection_curves_cursor.png",
        tr("<b>Tool::Curves - Intersection Point of Curves:</b> Select first curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionCurves>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionCurves>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleCurveIntersectAxisTool(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButtonWithApply<DialogCurveIntersectAxis>
    (
        checked,
        Tool::CurveIntersectAxis,
        ":/cursor/curve_intersect_axis_cursor.png",
        tr("<b>Tool::Curves - Intersection Point of Curve and Axis:</b> Select curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>
    );
}

//Arcs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleArcTool handler for arc tool.
 * @param checked true - button checked.
 */
void MainWindow::handleArcTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogArc>
    (
        checked,
        Tool::Arc,
        ":/cursor/arc_cursor.png",
        tr("<b>Tool::Arcs - Arc:</b> Select point of center of arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolArc>,
        &MainWindow::ApplyDrawDialog<VToolArc>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePointAlongArcTool handler for PointAlongArc tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePointAlongArcTool(bool checked)
{
    ToolSelectArc();
    SetToolButtonWithApply<DialogCutArc>
    (
        checked,
        Tool::CutArc,
        ":/cursor/arc_cut_cursor.png",
        tr("<b>Tool::Arc - Point along Arc:</b> Select arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutArc>,
        &MainWindow::ApplyDrawDialog<VToolCutArc>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleArcIntersectAxisTool(bool checked)
{
    ToolSelectAllDrawObjects();
    // Reuse handleCurveIntersectAxisTool but with different cursor and tool tip
    SetToolButtonWithApply<DialogCurveIntersectAxis>
    (
        checked,
        Tool::ArcIntersectAxis,
        ":/cursor/arc_intersect_axis_cursor.png",
        tr("<b>Tool::Arc - Intersection Point of Arc and Axis:</b> Select arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handlePointOfIntersectionArcsTool(bool checked)
{
    ToolSelectArc();
    SetToolButtonWithApply<DialogPointOfIntersectionArcs>
    (
        checked,
        Tool::PointOfIntersectionArcs,
        "://cursor/point_of_intersection_arcs.png",
        tr("<b>Tool::Arc - Intersection Point of Arcs:</b> Select first an arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionArcs>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionArcs>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handlePointOfIntersectionCirclesTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<IntersectCirclesDialog>
    (
        checked,
        Tool::PointOfIntersectionCircles,
        "://cursor/point_of_intersection_circles.png",
        tr("<b>Tool::Arc - Intersection Point of Circles:</b> Select first circle center"),
        &MainWindow::ClosedDrawDialogWithApply<IntersectCirclesTool>,
        &MainWindow::ApplyDrawDialog<IntersectCirclesTool>
    );
}



//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handlePointFromCircleAndTangentTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<IntersectCircleTangentDialog>
    (
        checked,
        Tool::PointFromCircleAndTangent,
        "://cursor/point_from_circle_and_tangent_cursor.png",
        tr("<b>Tool::Arc - Tangency Point of Circle and Tangent:</b> Select point on tangent"),
        &MainWindow::ClosedDrawDialogWithApply<IntersectCircleTangentTool>,
        &MainWindow::ApplyDrawDialog<IntersectCircleTangentTool>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handlePointFromArcAndTangentTool(bool checked)
{
    ToolSelectPointArc();
    SetToolButtonWithApply<DialogPointFromArcAndTangent>
    (
        checked,
        Tool::PointFromArcAndTangent,
        "://cursor/point_from_arc_and_tangent_cursor.png",
        tr("<b>Tool::Arc - Tangency Point of Arc and Tangent:</b> Select point on tangent"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointFromArcAndTangent>,
        &MainWindow::ApplyDrawDialog<VToolPointFromArcAndTangent>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleArcWithLengthTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogArcWithLength>
    (
        checked,
        Tool::ArcWithLength,
        "://cursor/arc_with_length_cursor.png",
        tr("<b>Tool::Arc - Arc with Length:</b> Select point of the center of the arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolArcWithLength>,
        &MainWindow::ApplyDrawDialog<VToolArcWithLength>
    );
}

//Elliptical Arcs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleEllipticalArcTool handler for EllipticalArc tool.
 * @param checked true - button checked.
 */
void MainWindow::handleEllipticalArcTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogEllipticalArc>
    (
        checked,
        Tool::EllipticalArc,
        ":/cursor/el_arc_cursor.png",
        tr("<b>Tool::Elliptical Arcs - Elliptical Arc:</b> Select point of center of elliptical arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolEllipticalArc>,
        &MainWindow::ApplyDrawDialog<VToolEllipticalArc>
    );
}

//Operations
//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleGroupTool(bool checked)
{
    ToolSelectGroupObjects();
    const QString tooltip = tr("<b>Tool::Operations - Create Group:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to finish group creation ")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButton<DialogGroup>
    (
        checked,
        Tool::Group,
        ":/cursor/group_cursor.png",
        tooltip,
        &MainWindow::ClosedDialogGroup
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogGroup(int result)
{
    SCASSERT(dialogTool != nullptr)
    if (result == QDialog::Accepted)
    {
        QSharedPointer<DialogGroup> dialog = dialogTool.objectCast<DialogGroup>();
        SCASSERT(dialog != nullptr)
        const QDomElement group = doc->CreateGroup(VContainer::getNextId(), dialog->GetName(), dialog->GetGroup());
        if (not group.isNull())
        {
            AddGroup *addGroup = new AddGroup(group, doc);
            connect(addGroup, &AddGroup::UpdateGroups, groupsWidget, &VWidgetGroups::UpdateGroups);
            qApp->getUndoStack()->push(addGroup);
        }
    }
    handleArrowTool(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleRotationTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Operations - Rotation:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButtonWithApply<DialogRotation>
    (
        checked,
        Tool::Rotation,
        ":/cursor/rotation_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolRotation>,
        &MainWindow::ApplyDrawDialog<VToolRotation>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleMirrorByLineTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Operations - Mirror by Line:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButtonWithApply<DialogMirrorByLine>
    (
        checked,
        Tool::MirrorByLine,
        ":/cursor/mirror_by_line_cursor.png",
        tooltip, &MainWindow::ClosedDrawDialogWithApply<VToolMirrorByLine>,
        &MainWindow::ApplyDrawDialog<VToolMirrorByLine>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleMirrorByAxisTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Operations - Mirror by Axis:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButtonWithApply<DialogMirrorByAxis>
    (
        checked,
        Tool::MirrorByAxis,
        ":/cursor/mirror_by_axis_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolMirrorByAxis>,
        &MainWindow::ApplyDrawDialog<VToolMirrorByAxis>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleMoveTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Operations - Move:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButtonWithApply<DialogMove>
    (
        checked,
        Tool::Move,
        ":/cursor/move_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolMove>,
        &MainWindow::ApplyDrawDialog<VToolMove>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleTrueDartTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogTrueDarts>
    (
        checked,
        Tool::TrueDarts,
        "://cursor/true_darts_cursor.png",
        tr("<b>Tool::Operations - TrueDarts:</b> Select the first base line point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTrueDarts>,
        &MainWindow::ApplyDrawDialog<VToolTrueDarts>
    );
}

//Add Details
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePatternPieceTool handler for pattern piece tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePatternPieceTool(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButtonWithApply<DialogSeamAllowance>
    (
        checked,
        Tool::Piece,
        ":/cursor/new_detail_cursor.png",
        tr("<b>Tool::Add Details - Add New Pattern Piece:</b> Select main path of objects clockwise."),
        &MainWindow::ClosedDetailsDialogWithApply<VToolSeamAllowance>,
        &MainWindow::ApplyDetailsDialog<VToolSeamAllowance>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleAnchorPointTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButton<DialogPin>
    (
        checked,
        Tool::Pin,
        ":/cursor/anchor_point_cursor.png",
        tr("<b>Tool::Add Details - Add Anchor Point:</b> Select anchor point"),
        &MainWindow::ClosedDialogAnchorPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogAnchorPoint(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolPin::Create(dialogTool, doc, pattern);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleInternalPathTool(bool checked)
{
    ToolSelectAllDrawObjects();
    SetToolButton<DialogInternalPath>
    (
        checked,
        Tool::InternalPath,
        ":/cursor/path_cursor.png",
        tr("<b>Tool::Add Details - Internal Path:</b> Select path objects, use <b>SHIFT</b> to reverse curve direction"),
        &MainWindow::ClosedDialogInternalPath
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogInternalPath(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolInternalPath::Create(dialogTool, pieceScene, doc, pattern);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleInsertNodesTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Add Details - Insert Nodes:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButton<InsertNodesDialog>
    (
        checked,
        Tool::InsertNodes,
        "://cursor/insert_nodes_cursor.png",
        tooltip,
        &MainWindow::ClosedInsertNodesDialog
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedInsertNodesDialog(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        QSharedPointer<InsertNodesDialog> tool = dialogTool.objectCast<InsertNodesDialog>();
        SCASSERT(tool != nullptr)
        VToolSeamAllowance::insertNodes(tool->getNodes(), tool->getPieceId(), pieceScene, pattern, doc);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//Pieces
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleUnionDetailsTool handler for unionDetails tool.
 * @param checked true - button checked.
 */
void MainWindow::handleUnionDetailsTool(bool checked)
{
    ToolSelectDetail();
    SetToolButton<DialogUnionDetails>
    (
        checked,
        Tool::UnionDetails,
        ":/cursor/union_cursor.png",
        tr("<b>Tool::Pattern Piece - Union:</b> Select pattern piece"),
        &MainWindow::ClosedDialogUnionDetails
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ClosedDialogUnionDetails actions after closing DialogUnionDetails.
 * @param result result of dialog working.
 */
void MainWindow::ClosedDialogUnionDetails(int result)
{
    ClosedDialog<VToolUnionDetails>(result);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleNewLayout handler for New Layout tool.
 * @param tButton - toolbutton.
 * @param checked true - button checked.
 */
void MainWindow::handleNewLayout(bool checked)
{
      toolLayoutSettings(ui->layoutSettings_ToolButton, checked);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.Tip show tools tooltip.
 * @param toolTip tooltip text.
 */
void MainWindow::ShowToolTip(const QString &toolTip)
{
    helpLabel->setText(toolTip);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showEvent handle after show window.
 * @param event show event.
 */
void MainWindow::showEvent( QShowEvent *event )
{
    QMainWindow::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    MinimumScrollBar();

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        undoAction->setText(tr("&Undo"));
        redoAction->setText(tr("&Redo"));
        helpLabel->setText(QObject::tr("Changes applied."));
        patternPieceLabel->setText(tr("Draft Block:"));
        UpdateWindowTitle();
        emit pieceScene->LanguageChanged();
    }
    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle after close window.
 * @param event close event.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    qCDebug(vMainWindow, "Closing main window");
    if (MaybeSave())
    {
        FileClosedCorrect();

        event->accept();
        QApplication::closeAllWindows();
    }
    else
    {
        qCDebug(vMainWindow, "Closing canceled.");
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::customEvent(QEvent *event)
{
    if (event->type() == UNDO_EVENT)
    {
        qApp->getUndoStack()->undo();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CleanLayout()
{
    qDeleteAll (scenes);
    scenes.clear();
    shadows.clear();
    papers.clear();
    ui->listWidget->clear();
    SetLayoutModeActions();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PrepareSceneList()
{
    for (int i=1; i<=scenes.size(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(ScenePreview(i-1), QString::number(i));
        ui->listWidget->addItem(item);
    }

    if (not scenes.isEmpty())
    {
        ui->listWidget->setCurrentRow(0);
        SetLayoutModeActions();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ExportToCSVData(const QString &fileName, const DialogExportToCSV &dialog)
{
    QxtCsvModel csv;

    csv.insertColumn(0);
    csv.insertColumn(1);
    csv.insertColumn(2);

    if (dialog.WithHeader())
    {
        csv.setHeaderText(0, tr("Name"));
        csv.setHeaderText(1, tr("The calculated value"));
        csv.setHeaderText(2, tr("Formula"));
    }

    const QMap<QString, QSharedPointer<VIncrement> > increments = pattern->variablesData();
    QMap<QString, QSharedPointer<VIncrement> >::const_iterator i;
    QMap<quint32, QString> map;
    //Sorting QHash by id
    for (i = increments.constBegin(); i != increments.constEnd(); ++i)
    {
        QSharedPointer<VIncrement> incr = i.value();
        map.insert(incr->getIndex(), i.key());
    }

    qint32 currentRow = -1;
    QMapIterator<quint32, QString> iMap(map);
    while (iMap.hasNext())
    {
        iMap.next();
        QSharedPointer<VIncrement> incr = increments.value(iMap.value());
        currentRow++;

        csv.insertRow(currentRow);
        csv.setText(currentRow, 0, incr->GetName()); // name
        csv.setText(currentRow, 1, qApp->LocaleToString(*incr->GetValue())); // calculated value

        QString formula;
        try
        {
            formula = qApp->TrVars()->FormulaToUser(incr->GetFormula(), qApp->Settings()->GetOsSeparator());
        }
        catch (qmu::QmuParserError &e)
        {
            Q_UNUSED(e)
            formula = incr->GetFormula();
        }

        csv.setText(currentRow, 2, formula); // formula
    }

    csv.toCSV(fileName, dialog.WithHeader(), dialog.Separator(), QTextCodec::codecForMib(dialog.SelectedMib()));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadIndividual()
{
    const QString filter = tr("Individual measurements") + QLatin1String(" (*.vit);;") + tr("Multisize measurements") +
                           QLatin1String(" (*.vst)");
    //Use standard path to individual measurements
    const QString path = qApp->Seamly2DSettings()->GetPathIndividualMeasurements();

    bool usedNotExistedDir = false;
    QDir directory(path);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                       QFileDialog::DontUseNativeDialog);

    if (not mPath.isEmpty())
    {
        if (LoadMeasurements(mPath))
        {
            if (not doc->MPath().isEmpty())
            {
                watcher->removePath(AbsoluteMPath(qApp->GetPPath(), doc->MPath()));
            }
            ui->unloadMeasurements_Action->setEnabled(true);
            doc->SetMPath(RelativeMPath(qApp->GetPPath(), mPath));
            watcher->addPath(mPath);
            PatternChangesWereSaved(false);
            ui->editCurrent_Action->setEnabled(true);
            helpLabel->setText(tr("Measurements loaded"));
            doc->LiteParseTree(Document::LiteParse);

            UpdateWindowTitle();
        }
    }

    if (usedNotExistedDir)
    {
        QDir directory(path);
        directory.rmpath(".");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadMultisize()
{
    const QString filter = tr("Multisize measurements") + QLatin1String(" (*.vst);;") + tr("Individual measurements") +
                           QLatin1String("(*.vit)");
    //Use standard path to multisize measurements
    QString path = qApp->Seamly2DSettings()->GetPathMultisizeMeasurements();
    path = VCommonSettings::PrepareMultisizeTables(path);
    const QString mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                       QFileDialog::DontUseNativeDialog);

    if (not mPath.isEmpty())
    {
        QString hText;
        if (not gradationHeights.isNull())
        {
            hText = gradationHeights->currentText();
        }
        QString sText;
        if (not gradationSizes.isNull())
        {
            sText = gradationSizes->currentText();
        }

        if(LoadMeasurements(mPath))
        {
            if (not doc->MPath().isEmpty())
            {
                watcher->removePath(AbsoluteMPath(qApp->GetPPath(), doc->MPath()));
            }
            ui->unloadMeasurements_Action->setEnabled(true);
            doc->SetMPath(RelativeMPath(qApp->GetPPath(), mPath));
            watcher->addPath(mPath);
            PatternChangesWereSaved(false);
            ui->editCurrent_Action->setEnabled(true);
            helpLabel->setText(tr("Measurements loaded"));
            doc->LiteParseTree(Document::LiteParse);

            UpdateWindowTitle();

            if (qApp->patternType() == MeasurementsType::Multisize)
            {
                if (not hText.isEmpty() && not gradationHeights.isNull())
                {
                    gradationHeights->setCurrentText(hText);
                }

                if (not sText.isEmpty() && not gradationSizes.isNull())
                {
                    gradationSizes->setCurrentText(sText);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UnloadMeasurements()
{
    if (doc->MPath().isEmpty())
    {
        ui->unloadMeasurements_Action->setDisabled(true);
        return;
    }

    if (doc->ListMeasurements().isEmpty())
    {
        watcher->removePath(AbsoluteMPath(qApp->GetPPath(), doc->MPath()));
        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            initStatusBar();
        }
        qApp->setPatternType(MeasurementsType::Unknown);
        doc->SetMPath(QString());
        emit doc->UpdatePatternLabel();
        PatternChangesWereSaved(false);
        ui->editCurrent_Action->setEnabled(false);
        ui->unloadMeasurements_Action->setDisabled(true);
        helpLabel->setText(tr("Measurements unloaded"));

        UpdateWindowTitle();
    }
    else
    {
        qCWarning(vMainWindow, "%s",
                  qUtf8Printable(tr("Couldn't unload measurements. Some of them are used in the pattern.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowMeasurements()
{
    if (not doc->MPath().isEmpty())
    {
        const QString absoluteMPath = AbsoluteMPath(qApp->GetPPath(), doc->MPath());

        QStringList arguments;
        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            arguments = QStringList()
                    << absoluteMPath
                    << "-u"
                    << UnitsToStr(qApp->patternUnit())
                    << "-e"
                    << QString().setNum(static_cast<int>(UnitConvertor(VContainer::height(), doc->MUnit(), Unit::Cm)))
                    << "-s"
                    << QString().setNum(static_cast<int>(UnitConvertor(VContainer::size(), doc->MUnit(), Unit::Cm)));
        }
        else
        {
            arguments = QStringList() << absoluteMPath
                                      << "-u"
                                      << UnitsToStr(qApp->patternUnit());
        }

        if (isNoScaling)
        {
            arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        const QString seamlyme = qApp->SeamlyMeFilePath();
        const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();
        QProcess::startDetached(seamlyme, arguments, workingDirectory);
    }
    else
    {
        ui->editCurrent_Action->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::MeasurementsChanged(const QString &path)
{
    mChanges = false;
    QFileInfo checkFile(path);
    if (checkFile.exists())
    {
        mChanges = true;
        mChangesAsked = false;
    }
    else
    {
        for(int i=0; i<=1000; i=i+10)
        {
            if (checkFile.exists())
            {
                mChanges = true;
                mChangesAsked = false;
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    UpdateWindowTitle();
    ui->syncMeasurements_Action->setEnabled(mChanges);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SyncMeasurements()
{
    if (mChanges)
    {
        const QString path = AbsoluteMPath(qApp->GetPPath(), doc->MPath());
        if(UpdateMeasurements(path, static_cast<int>(VContainer::size()), static_cast<int>(VContainer::height())))
        {
            if (not watcher->files().contains(path))
            {
                watcher->addPath(path);
            }
            const QString msg = tr("Measurements have been synced");
            qCDebug(vMainWindow, "%s", qUtf8Printable(msg));
            helpLabel->setText(msg);
            VWidgetPopup::PopupMessage(this, msg);
            doc->LiteParseTree(Document::LiteParse);
            mChanges = false;
            mChangesAsked = true;
            UpdateWindowTitle();
            ui->syncMeasurements_Action->setEnabled(mChanges);
        }
        else
        {
            qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't sync measurements.")));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void MainWindow::OpenAt(QAction *where)
{
    const QString path = qApp->GetPPath().left(qApp->GetPPath().indexOf(where->text())) + where->text();
    if (path == qApp->GetPPath())
    {
        return;
    }
    QProcess process;
    process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}
#endif //defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initStatusBar initialize horizontal bar for presenting status information
 */
void MainWindow::initStatusBar()
{
    if (!mouseCoordinates.isNull())
    {
        delete mouseCoordinates;
    }
    if (!infoToolButton.isNull())
    {
        delete infoToolButton;
    }
    if (!gradationHeights.isNull())
    {
        delete gradationHeights;
    }
    if (!gradationSizes.isNull())
    {
        delete gradationSizes;
    }
    if (!gradationHeightsLabel.isNull())
    {
        delete gradationHeightsLabel;
    }
    if (!gradationSizesLabel.isNull())
    {
        delete gradationSizesLabel;
    }

    if (qApp->patternType() == MeasurementsType::Multisize)
    {
        const QStringList listHeights = VMeasurement::ListHeights(doc->GetGradationHeights(), qApp->patternUnit());
        const QStringList listSizes = VMeasurement::ListSizes(doc->GetGradationSizes(), qApp->patternUnit());

        gradationHeightsLabel = new QLabel(tr("Height:"), this);
        gradationHeights = SetGradationList(gradationHeightsLabel, listHeights);

        // set default height
        SetDefaultHeight();

        connect(gradationHeights.data(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &MainWindow::ChangedHeight);

        gradationSizesLabel = new QLabel(tr("Size:"), this);
        gradationSizes = SetGradationList(gradationSizesLabel, listSizes);

        // set default size
        SetDefaultSize();

        connect(gradationSizes.data(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &MainWindow::ChangedSize);

    }

    mouseCoordinates = new MouseCoordinates(qApp->patternUnit());
    ui->statusBar->addPermanentWidget((mouseCoordinates));

    infoToolButton = new QToolButton();
    infoToolButton->setDefaultAction(ui->documentInfo_Action);
    ui->statusBar->addPermanentWidget((infoToolButton));
}

//---------------------------------------------------------------------------------------------------------------------
QComboBox *MainWindow::SetGradationList(QLabel *label, const QStringList &list)
{
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(list);
    ui->statusBar->addPermanentWidget(label);
    ui->statusBar->addPermanentWidget(comboBox);

    return comboBox;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::initModesToolBar()
{
    leftGoToStage = new QLabel(this);
    leftGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_left_to_right_arrow.png"));
    ui->mode_ToolBar->insertWidget(ui->pieceMode_Action, leftGoToStage);

    rightGoToStage = new QLabel(this);
    rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));
    ui->mode_ToolBar->insertWidget(ui->layoutMode_Action, rightGoToStage);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initPointNameToolBar enable Point Name toolbar.
 */
void MainWindow::initPointNameToolBar()
{
    fontComboBox = new QFontComboBox ;
    fontComboBox->setCurrentFont(qApp->Seamly2DSettings()->getPointNameFont());
    ui->pointName_ToolBar->insertWidget(ui->showPointNames_Action,fontComboBox);
    fontComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    fontComboBox->setEnabled(true);

    connect(fontComboBox, static_cast<void (QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont font)
            {
                qApp->Seamly2DSettings()->setPointNameFont(font);
                upDateScenes();
            });

    fontSizeComboBox = new QComboBox ;
    ui->pointName_ToolBar->insertWidget(ui->showPointNames_Action,fontSizeComboBox);
    fontSizeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    fontSizeComboBox->addItem("6", QVariant(static_cast<int>(6)));
    fontSizeComboBox->addItem("7", QVariant(static_cast<int>(7)));
    fontSizeComboBox->addItem("8", QVariant(static_cast<int>(8)));
    fontSizeComboBox->addItem("9", QVariant(static_cast<int>(9)));
    fontSizeComboBox->addItem("10", QVariant(static_cast<int>(10)));
    fontSizeComboBox->addItem("11", QVariant(static_cast<int>(11)));
    fontSizeComboBox->addItem("12", QVariant(static_cast<int>(12)));
    fontSizeComboBox->addItem("13", QVariant(static_cast<int>(13)));
    fontSizeComboBox->addItem("14", QVariant(static_cast<int>(14)));
    fontSizeComboBox->addItem("15", QVariant(static_cast<int>(15)));
    fontSizeComboBox->addItem("16", QVariant(static_cast<int>(16)));
    fontSizeComboBox->addItem("18", QVariant(static_cast<int>(18)));
    fontSizeComboBox->addItem("20", QVariant(static_cast<int>(20)));
    fontSizeComboBox->addItem("22", QVariant(static_cast<int>(22)));
    fontSizeComboBox->addItem("24", QVariant(static_cast<int>(24)));
    fontSizeComboBox->addItem("26", QVariant(static_cast<int>(26)));
    fontSizeComboBox->addItem("28", QVariant(static_cast<int>(28)));
    fontSizeComboBox->addItem("32", QVariant(static_cast<int>(32)));
    fontSizeComboBox->addItem("36", QVariant(static_cast<int>(36)));
    fontSizeComboBox->addItem("40", QVariant(static_cast<int>(40)));
    fontSizeComboBox->addItem("44", QVariant(static_cast<int>(44)));
    fontSizeComboBox->addItem("48", QVariant(static_cast<int>(48)));
    fontSizeComboBox->addItem("54", QVariant(static_cast<int>(54)));
    fontSizeComboBox->addItem("60", QVariant(static_cast<int>(60)));
    fontSizeComboBox->addItem("66", QVariant(static_cast<int>(66)));
    fontSizeComboBox->addItem("72", QVariant(static_cast<int>(72)));
    fontSizeComboBox->addItem("80", QVariant(static_cast<int>(80)));
    fontSizeComboBox->addItem("96", QVariant(static_cast<int>(96)));

    int index = fontSizeComboBox->findData(qApp->Seamly2DSettings()->getPointNameSize());
    if (index < 0 || index > 28)
    {
        index = 18;
    }
    fontSizeComboBox->setCurrentIndex(index);

    connect(fontSizeComboBox, &QComboBox::currentTextChanged, this, [this](QString text)
            {
                qApp->Seamly2DSettings()->setPointNameSize(text.toInt());
                upDateScenes();
            });
    fontSizeComboBox->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initDraftToolBar enable draw toolbar.
 */
void MainWindow::initDraftToolBar()
{
    patternPieceLabel = new QLabel(tr("Draft Block:"));
    ui->draft_ToolBar->addWidget(patternPieceLabel);

    // By using Qt UI Designer we can't add QComboBox to toolbar
    comboBoxDraws = new QComboBox;
    ui->draft_ToolBar->addWidget(comboBoxDraws);
    comboBoxDraws->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    comboBoxDraws->setEnabled(false);

    connect(comboBoxDraws,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){changeDraftBlock(index);});

    connect(ui->renameDraft_Action, &QAction::triggered, this, [this]()
    {
        const QString activeDraftBlock = doc->getActiveDraftBlockName();
        const QString draftBlockName = PatternPieceName(activeDraftBlock);
        if (draftBlockName.isEmpty())
        {
            return;
        }
        RenameDraftBlock *draftBlock = new RenameDraftBlock(doc, draftBlockName, comboBoxDraws);
        qApp->getUndoStack()->push(draftBlock);
    });
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::initToolsToolBar()
{
    /*First we will try use Standard Shortcuts from Qt, but because keypad "-" and "+" not the same keys like in main
    keypad, shortcut Ctrl+"-" or "+" from keypad will not working with standard shortcut (QKeySequence::ZoomIn or
    QKeySequence::ZoomOut). For example "+" is Qt::Key_Plus + Qt::KeypadModifier for keypad.
    Also for me don't work Qt:CTRL and work Qt::ControlModifier.*/

    QList<QKeySequence> zoomInShortcuts;
    zoomInShortcuts.append(QKeySequence(QKeySequence::ZoomIn));
    zoomInShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier));
    ui->zoomIn_Action->setShortcuts(zoomInShortcuts);
    connect(ui->zoomIn_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomIn);

    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts.append(QKeySequence(QKeySequence::ZoomOut));
    zoomOutShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier));
    ui->zoomOut_Action->setShortcuts(zoomOutShortcuts);
    connect(ui->zoomOut_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomOut);

    QList<QKeySequence> zoom100PercentShortcuts;
    zoom100PercentShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0));
    zoom100PercentShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0 + Qt::KeypadModifier));
    ui->zoom100Percent_Action->setShortcuts(zoom100PercentShortcuts);
    connect(ui->zoom100Percent_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoom100Percent);

    QList<QKeySequence> zoomToFitShortcuts;
    zoomToFitShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Equal));
    ui->zoomToFit_Action->setShortcuts(zoomToFitShortcuts);
    connect(ui->zoomToFit_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomToFit);

    QList<QKeySequence> zoomToSelectedShortcuts;
    zoomToSelectedShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Right));
    ui->zoomToSelected_Action->setShortcuts(zoomToSelectedShortcuts);
    connect(ui->zoomToSelected_Action, &QAction::triggered, this, &MainWindow::zoomToSelected);

    QList<QKeySequence> zoomToPreviousShortcuts;
    zoomToPreviousShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Left));
    ui->zoomToPrevious_Action->setShortcuts(zoomToPreviousShortcuts);
    connect(ui->zoomToPrevious_Action, &QAction::triggered,  this, &MainWindow::zoomToPrevious);

    QList<QKeySequence> zoomToAreaShortcuts;
    zoomToAreaShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_A));
    ui->zoomToArea_Action->setShortcuts(zoomToAreaShortcuts);
    connect(ui->zoomToArea_Action, &QAction::toggled, this, &MainWindow::zoomToArea);

    resetPanShortcuts();
    connect(ui->zoomPan_Action, &QAction::toggled, this, &MainWindow::zoomPan);

    if (zoomScaleSpinBox != nullptr)
    {
        delete zoomScaleSpinBox;
    }
    zoomScaleSpinBox = new QDoubleSpinBox();
    zoomScaleSpinBox->setDecimals(1);
    zoomScaleSpinBox->setAlignment(Qt::AlignRight);
    zoomScaleSpinBox->setSingleStep(0.1);
    zoomScaleSpinBox->setMinimum(qFloor(VMainGraphicsView::MinScale()*1000)/10.0);
    zoomScaleSpinBox->setMaximum(qFloor(VMainGraphicsView::MaxScale()*1000)/10.0);
    zoomScaleSpinBox->setSuffix("%");
    zoomScaleSpinBox->setMaximumWidth(80);
    zoomScaleSpinBox->setKeyboardTracking(false);
    ui->zoom_ToolBar->insertWidget(ui->zoomIn_Action,zoomScaleSpinBox);

    zoomScaleChanged(ui->view->transform().m11());
    connect(zoomScaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,[this](double d){ui->view->zoomByScale(d/100.0);});

}

void MainWindow::initToolBarVisibility()
{
    updateToolBarVisibility();
    connect(ui->tools_ToolBox_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->tools_ToolBox_ToolBar->setVisible(visible);
        qApp->Settings()->setShowToolsToolBar(visible);
    });
    connect(ui->points_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->points_ToolBar->setVisible(visible);
        qApp->Settings()->setShowPointToolBar(visible);
    });
    connect(ui->lines_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->lines_ToolBar->setVisible(visible);
        qApp->Settings()->setShowLineToolBar(visible);
    });
    connect(ui->curves_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->curves_ToolBar->setVisible(visible);
        qApp->Settings()->setShowCurveToolBar(visible);
    });
    connect(ui->arcs_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->arcs_ToolBar->setVisible(visible);
        qApp->Settings()->setShowArcToolBar(visible);
    });
    connect(ui->operations_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->operations_ToolBar->setVisible(visible);
        qApp->Settings()->setShowOpsToolBar(visible);
    });
    connect(ui->pieces_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->pieces_ToolBar->setVisible(visible);
        qApp->Settings()->setShowPieceToolBar(visible);
    });
    connect(ui->details_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->details_ToolBar->setVisible(visible);
        qApp->Settings()->setShowDetailsToolBar(visible);
    });
    connect(ui->layout_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->layout_ToolBar->setVisible(visible);
        qApp->Settings()->setShowLayoutToolBar(visible);
    });
}

void MainWindow::updateToolBarVisibility()
{
    setToolBarVisibility(ui->tools_ToolBox_ToolBar, qApp->Settings()->getShowToolsToolBar());
    setToolBarVisibility(ui->points_ToolBar, qApp->Settings()->getShowPointToolBar());
    setToolBarVisibility(ui->lines_ToolBar, qApp->Settings()->getShowLineToolBar());
    setToolBarVisibility(ui->curves_ToolBar, qApp->Settings()->getShowCurveToolBar());
    setToolBarVisibility(ui->arcs_ToolBar, qApp->Settings()->getShowArcToolBar());
    setToolBarVisibility(ui->operations_ToolBar, qApp->Settings()->getShowOpsToolBar());
    setToolBarVisibility(ui->pieces_ToolBar, qApp->Settings()->getShowPieceToolBar());
    setToolBarVisibility(ui->details_ToolBar, qApp->Settings()->getShowDetailsToolBar());
    setToolBarVisibility(ui->layout_ToolBar, qApp->Settings()->getShowLayoutToolBar());
}

void MainWindow::setToolBarVisibility(QToolBar *toolbar, bool visible)
{
    toolbar->blockSignals(true);
    toolbar->setVisible(visible);
    toolbar->blockSignals(false);
}

void MainWindow::zoomScaleChanged(qreal scale)
{
    zoomScaleSpinBox->blockSignals(true);
    zoomScaleSpinBox->setValue(qFloor(scale*1000)/10.0);
    zoomScaleSpinBox->blockSignals(false);
    qCDebug(vMainWindow, "Value %f\n", (qreal(qFloor(scale*1000)/10.0)));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomToSelected()
{
    if (qApp->getCurrentScene() == draftScene)
    {
        ui->view->zoomToRect(doc->ActiveDrawBoundingRect());
    }
    else if (qApp->getCurrentScene() == pieceScene)
    {
        QGraphicsItem *item = qApp->getCurrentScene()->focusItem();
        {
            if ((item != nullptr) && (item->type() == QGraphicsItem::UserType + static_cast<int>(Tool::Piece)))
            {
                QRectF rect;
                rect = item->boundingRect();
                rect.translate(item->scenePos());
                ui->view->zoomToRect(rect);
            }
        }
    }
}

void MainWindow::zoomToPrevious()
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
    SCASSERT(scene != nullptr)

    /*Set transform for current scene*/
    scene->swapTransforms();
    ui->view->setTransform(scene->transform());
    zoomScaleChanged(ui->view->transform().m11());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomToArea(bool checked)
{
      ui->view->zoomToAreaEnabled(checked);

      if (ui->zoomToArea_Action->isChecked())
      {
          ui->zoomPan_Action->setChecked(false);
      }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomPan(bool checked)
{
    ui->view->zoomPanEnabled(checked);
    if (checked)
    {
        ui->zoomToArea_Action->setChecked(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitToolButtons()
{
    connect(ui->arrowPointer_ToolButton, &QToolButton::clicked, this, &MainWindow::handleArrowTool);

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Check if all tools were connected.");

    connect(ui->pointAtDistanceAngle_ToolButton, &QToolButton::clicked,
            this, &MainWindow::handlePointAtDistanceAngleTool);
    connect(ui->line_ToolButton,           &QToolButton::clicked, this, &MainWindow::handleLineTool);
    connect(ui->alongLine_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleAlongLineTool);
    connect(ui->shoulderPoint_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleShoulderPointTool);
    connect(ui->normal_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleNormalTool);
    connect(ui->bisector_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleBisectorTool);
    connect(ui->lineIntersect_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleLineIntersectTool);
    connect(ui->curve_ToolButton,          &QToolButton::clicked, this, &MainWindow::handleCurveTool);
    connect(ui->curveWithCPs_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleCurveWithControlPointsTool);
    connect(ui->arc_ToolButton,            &QToolButton::clicked, this, &MainWindow::handleArcTool);
    connect(ui->spline_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleSplineTool);
    connect(ui->splineWithCPs_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleSplineWithControlPointsTool);
    connect(ui->pointOfContact_ToolButton, &QToolButton::clicked, this, &MainWindow::handlePointOfContactTool);
    connect(ui->addPatternPiece_ToolButton,&QToolButton::clicked, this, &MainWindow::handlePatternPieceTool);
    connect(ui->internalPath_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleInternalPathTool);
    connect(ui->height_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleHeightTool);
    connect(ui->triangle_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleTriangleTool);
    connect(ui->pointIntersectXY_ToolButton,    &QToolButton::clicked, this, &MainWindow::handlePointIntersectXYTool);
    connect(ui->pointAlongCurve_ToolButton,     &QToolButton::clicked, this, &MainWindow::handlePointAlongCurveTool);
    connect(ui->pointAlongSpline_ToolButton,    &QToolButton::clicked, this, &MainWindow::handlePointAlongSplineTool);
    connect(ui->unitePieces_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleUnionDetailsTool);
    connect(ui->pointAlongArc_ToolButton,       &QToolButton::clicked, this, &MainWindow::handlePointAlongArcTool);
    connect(ui->lineIntersectAxis_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleLineIntersectAxisTool);
    connect(ui->curveIntersectAxis_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleCurveIntersectAxisTool);
    connect(ui->arcIntersectAxis_ToolButton,    &QToolButton::clicked, this, &MainWindow::handleArcIntersectAxisTool);
    connect(ui->layoutSettings_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleNewLayout);
    connect(ui->pointOfIntersectionArcs_ToolButton,    &QToolButton::clicked,
            this, &MainWindow::handlePointOfIntersectionArcsTool);
    connect(ui->pointOfIntersectionCircles_ToolButton, &QToolButton::clicked,
            this, &MainWindow::handlePointOfIntersectionCirclesTool);
    connect(ui->pointOfIntersectionCurves_ToolButton,         &QToolButton::clicked,
            this, &MainWindow::handleCurveIntersectCurveTool);
    connect(ui->pointFromCircleAndTangent_ToolButton,  &QToolButton::clicked,
            this, &MainWindow::handlePointFromCircleAndTangentTool);
    connect(ui->pointFromArcAndTangent_ToolButton,     &QToolButton::clicked,
            this, &MainWindow::handlePointFromArcAndTangentTool);
    connect(ui->arcWithLength_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleArcWithLengthTool);
    connect(ui->trueDarts_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleTrueDartTool);
    connect(ui->exportDraftBlocks_ToolButton, &QToolButton::clicked, this, &MainWindow::exportDraftBlocksAs);
    connect(ui->group_ToolButton,          &QToolButton::clicked, this, &MainWindow::handleGroupTool);
    connect(ui->rotation_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleRotationTool);
    connect(ui->mirrorByLine_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleMirrorByLineTool);
    connect(ui->mirrorByAxis_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleMirrorByAxisTool);
    connect(ui->move_ToolButton,           &QToolButton::clicked, this, &MainWindow::handleMoveTool);
    connect(ui->midpoint_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleMidpointTool);
    connect(ui->exportLayout_ToolButton,   &QToolButton::clicked, this, &MainWindow::exportLayoutAs);
    connect(ui->exportPiecesAs_ToolButton, &QToolButton::clicked, this, &MainWindow::exportPiecesAs);
    connect(ui->ellipticalArc_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleEllipticalArcTool);
    connect(ui->anchorPoint_ToolButton,    &QToolButton::clicked, this, &MainWindow::handleAnchorPointTool);
    connect(ui->insertNodes_ToolButton,     &QToolButton::clicked, this, &MainWindow::handleInsertNodesTool);
}

void MainWindow::handlePointsMenu()
{
    qCDebug(vMainWindow, "Points Menu selected. \n");

    QMenu menu;

    QAction *action_Midpoint            = menu.addAction(QIcon(":/toolicon/32x32/midpoint.png"),               tr("Midpoint"));
    QAction *action_PointAtDA           = menu.addAction(QIcon(":/toolicon/32x32/segment.png"),                tr("Point at Distance and Angle"));
    QAction *action_PointAlongLine      = menu.addAction(QIcon(":/toolicon/32x32/along_line.png"),             tr("Point at Distance along Line"));
    QAction *action_AlongPerpendicular  = menu.addAction(QIcon(":/toolicon/32x32/normal.png"),                 tr("Point on Perpendicular"));
    QAction *action_Bisector            = menu.addAction(QIcon(":/toolicon/32x32/bisector.png"),               tr("Point along Bisector"));
    QAction *action_Shoulder            = menu.addAction(QIcon(":/toolicon/32x32/shoulder.png"),               tr("Point on Shoulder"));
    QAction *action_PointOfContact      = menu.addAction(QIcon(":/toolicon/32x32/point_of_contact.png"),       tr("Intersection Point of Line and Arc"));
    QAction *action_Triangle            = menu.addAction(QIcon(":/toolicon/32x32/triangle.png"),               tr("Triangle"));
    QAction *action_PointIntersectXY    = menu.addAction(QIcon(":/toolicon/32x32/point_intersectxy_icon.png"), tr("Intersect XY") + "\tXY");
    QAction *action_PerpendicularPoint  = menu.addAction(QIcon(":/toolicon/32x32/height.png"),                 tr("Intersection Point of Line and Perpendicular"));
    QAction *action_PointIntersectAxis  = menu.addAction(QIcon(":/toolicon/32x32/line_intersect_axis.png"),    tr("Intersection Point of Line and Axis"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Midpoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->midpoint_ToolButton->setChecked(true);
        handleMidpointTool(true);
    }
    else if (selectedAction == action_PointAtDA)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointAtDistanceAngle_ToolButton->setChecked(true);
        handlePointAtDistanceAngleTool(true);
    }
    else if (selectedAction == action_PointAlongLine)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->alongLine_ToolButton->setChecked(true);
        handleAlongLineTool(true);
    }
    else if (selectedAction == action_AlongPerpendicular)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->normal_ToolButton->setChecked(true);
        handleNormalTool(true);
    }
    else if (selectedAction == action_Bisector)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->bisector_ToolButton->setChecked(true);
        handleBisectorTool(true);
    }
    else if (selectedAction == action_Shoulder)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->shoulderPoint_ToolButton->setChecked(true);
        handleShoulderPointTool(true);
    }
    else if (selectedAction == action_PointOfContact)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointOfContact_ToolButton->setChecked(true);
        handlePointOfContactTool(true);
    }
    else if (selectedAction == action_Triangle)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->triangle_ToolButton->setChecked(true);
        handleTriangleTool(true);
    }
    else if (selectedAction == action_PointIntersectXY)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointIntersectXY_ToolButton->setChecked(true);
        handlePointIntersectXYTool(true);
    }
    else if (selectedAction == action_PerpendicularPoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->height_ToolButton->setChecked(true);
        handleHeightTool(true);
    }
    else if (selectedAction == action_PointIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->lineIntersectAxis_ToolButton->setChecked(true);
        handleLineIntersectAxisTool(true);
    }
}

void MainWindow::handleLinesMenu()
{
    qCDebug(vMainWindow, "Lines Menu selected. \n");
    QMenu menu;

    QAction *action_Line          = menu.addAction(QIcon(":/toolicon/32x32/line.png"),          tr("Line Between 2 Points"));
    QAction *action_LineIntersect = menu.addAction(QIcon(":/toolicon/32x32/intersect.png"), tr("Point Intersect of 2 Lines"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Line)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->line_ToolButton->setChecked(true);
        handleLineTool(true);
    }
    else if (selectedAction == action_LineIntersect)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->lineIntersect_ToolButton->setChecked(true);
        handleLineIntersectTool(true);
    }
}

void MainWindow::handleArcsMenu()
{
    qCDebug(vMainWindow, "Arcs Menu selected. \n");
    QMenu menu;

    QAction *action_Arc              = menu.addAction(QIcon(":/toolicon/32x32/arc.png"),                           tr("Arc"));
    QAction *action_PointAlongArc    = menu.addAction(QIcon(":/toolicon/32x32/arc_cut.png"),                       tr("Point along Arc"));
    QAction *action_ArcIntersectAxis = menu.addAction(QIcon(":/toolicon/32x32/arc_intersect_axis.png"),            tr("Intersection Point of Arc and Axis"));
    QAction *action_ArcIntersectArc  = menu.addAction(QIcon(":/toolicon/32x32/point_of_intersection_arcs.png"),    tr("Intersection Point of Arcs"));
    QAction *action_CircleIntersect  = menu.addAction(QIcon(":/toolicon/32x32/point_of_intersection_circles.png"), tr("Intersection Point of Circles"));
    QAction *action_CircleTangent    = menu.addAction(QIcon(":/toolicon/32x32/point_from_circle_and_tangent.png"), tr("Tangency Point of Circle and Tangent"));
    QAction *action_ArcTangent       = menu.addAction(QIcon(":/toolicon/32x32/point_from_arc_and_tangent.png"),    tr("Tangency Point of Arc and Tangent"));
    QAction *action_ArcWithLength    = menu.addAction(QIcon(":/toolicon/32x32/arc_with_length.png"),               tr("Arc with Length"));
    QAction *action_EllipticalArc    = menu.addAction(QIcon(":/toolicon/32x32/el_arc.png"),                        tr("Elliptical Arc"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Arc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arc_ToolButton->setChecked(true);
        handleArcTool(true);
    }
    else if (selectedAction == action_PointAlongArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointAlongArc_ToolButton->setChecked(true);
        handlePointAlongArcTool(true);
    }
    else if (selectedAction == action_ArcIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcIntersectAxis_ToolButton->setChecked(true);
        handleArcIntersectAxisTool(true);
    }
    else if (selectedAction == action_ArcIntersectArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionArcs_ToolButton->setChecked(true);
        handlePointOfIntersectionArcsTool(true);
    }
    else if (selectedAction == action_CircleIntersect)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionCircles_ToolButton->setChecked(true);
        handlePointOfIntersectionCirclesTool(true);
    }
    else if (selectedAction == action_CircleTangent)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromCircleAndTangent_ToolButton->setChecked(true);
        handlePointFromCircleAndTangentTool(true);
    }
    else if (selectedAction == action_ArcTangent)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromArcAndTangent_ToolButton->setChecked(true);
        handlePointFromArcAndTangentTool(true);
    }
    else if (selectedAction == action_ArcWithLength)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcWithLength_ToolButton->setChecked(true);
        handleArcWithLengthTool(true);
    }
    else if (selectedAction == action_EllipticalArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->ellipticalArc_ToolButton->setChecked(true);
        handleEllipticalArcTool(true);
    }
}

void MainWindow::handleCurvesMenu()
{
    qCDebug(vMainWindow, "Curves Menu selected. \n");
    QMenu menu;

    QAction *action_Curve                = menu.addAction(QIcon(":/toolicon/32x32/spline.png"),               tr("Curve"));
    QAction *action_Spline               = menu.addAction(QIcon(":/toolicon/32x32/splinePath.png"),           tr("Spline"));
    QAction *action_CurveWithCPs         = menu.addAction(QIcon(":/toolicon/32x32/cubic_bezier.png"),         tr("Curve with Control Points"));
    QAction *action_SplineWithCPs        = menu.addAction(QIcon(":/toolicon/32x32/cubic_bezier_path.png"),    tr("Spline with Control Points"));
    QAction *action_PointAlongCurve      = menu.addAction(QIcon(":/toolicon/32x32/spline_cut_point.png"),     tr("Point along Curve"));
    QAction *action_PointAlongSpline     = menu.addAction(QIcon(":/toolicon/32x32/splinePath_cut_point.png"), tr("Point along Spline"));
    QAction *action_CurveIntersectCurve  = menu.addAction(QIcon(":/toolicon/32x32/intersection_curves.png"),  tr("Intersection Point of Curves"));
    QAction *action_CurveIntersectAxis   = menu.addAction(QIcon(":/toolicon/32x32/curve_intersect_axis.png"), tr("Intersection Point of Curve & Axis"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Curve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curve_ToolButton->setChecked(true);
        handleCurveTool(true);
    }
    else if (selectedAction == action_Spline)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->spline_ToolButton->setChecked(true);
        handleSplineTool(true);
    }
    else if (selectedAction == action_PointAlongCurve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongCurve_ToolButton->setChecked(true);
        handlePointAlongCurveTool(true);
    }
    else if (selectedAction == action_PointAlongSpline)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongSpline_ToolButton->setChecked(true);
        handlePointAlongSplineTool(true);
    }
    else if (selectedAction == action_CurveWithCPs)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveWithCPs_ToolButton->setChecked(true);
        handleCurveWithControlPointsTool(true);
    }
    else if (selectedAction == action_SplineWithCPs)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->splineWithCPs_ToolButton->setChecked(true);
        handleSplineWithControlPointsTool(true);
    }
    else if (selectedAction == action_CurveIntersectCurve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointOfIntersectionCurves_ToolButton->setChecked(true);
        handleCurveIntersectCurveTool(true);
    }
    else if (selectedAction == action_CurveIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveIntersectAxis_ToolButton->setChecked(true);
        handleCurveIntersectAxisTool(true);
    }
}

void MainWindow::handleCirclesMenu()
{
    qCDebug(vMainWindow, "Circles Menu selected. \n");

}

void MainWindow::handleOperationsMenu()
{
    qCDebug(vMainWindow, "Operations Menu selected. \n");
    QMenu menu;

    QAction *action_Group        = menu.addAction(QIcon(":/toolicon/32x32/group.png"),          tr("New Group"));
    QAction *action_Rotate       = menu.addAction(QIcon(":/toolicon/32x32/rotation.png"),       tr("Rotate"));
    QAction *action_MirrorByLine = menu.addAction(QIcon(":/toolicon/32x32/mirror_by_line.png"), tr("Mirror by Line"));
    QAction *action_MirrorByAxis = menu.addAction(QIcon(":/toolicon/32x32/mirror_by_axis.png"), tr("Mirror by Axis"));
    QAction *action_Move         = menu.addAction(QIcon(":/toolicon/32x32/move.png"),           tr("Move"));
    QAction *action_TrueDarts    = menu.addAction(QIcon(":/toolicon/32x32/true_darts.png"),     tr("True Darts"));
    QAction *action_ExportDraftBlocks = menu.addAction(QIcon(":/toolicon/32x32/export.png"), tr("Export Draft Blocks"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Group)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->group_ToolButton->setChecked(true);
        handleGroupTool(true);
    }
    else if (selectedAction == action_Rotate)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->rotation_ToolButton->setChecked(true);
        handleRotationTool(true);
    }
    else if (selectedAction == action_MirrorByLine)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByLine_ToolButton->setChecked(true);
        handleMirrorByLineTool(true);
    }
    else if (selectedAction == action_MirrorByAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByAxis_ToolButton->setChecked(true);
        handleMirrorByAxisTool(true);
    }
    else if (selectedAction == action_Move)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->move_ToolButton->setChecked(true);
        handleMoveTool(true);
    }
    else if (selectedAction == action_TrueDarts)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->trueDarts_ToolButton->setChecked(true);
        handleTrueDartTool(true);
    }
    else if (selectedAction == action_ExportDraftBlocks)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        exportDraftBlocksAs();
    }
}

void MainWindow::handlePieceMenu()
{
    qCDebug(vMainWindow, "Add Details Menu selected. \n");
    QMenu menu;

    QAction *action_Piece        = menu.addAction(QIcon(":/toolicon/32x32/new_detail.png"),   tr("New Pattern Piece"));
    QAction *action_AnchorPoint  = menu.addAction(QIcon(":/toolicon/32x32/anchor_point.png"), tr("Add AnchorPoint"));
    QAction *action_InternalPath = menu.addAction(QIcon(":/toolicon/32x32/path.png"),         tr("Create Internal Path"));
    QAction *action_InsertNodes  = menu.addAction(QIcon(":/toolicon/32x32/insert_nodes_icon.png"), tr("Insert Nodes in Path"));

    action_AnchorPoint->setEnabled(pattern->DataPieces()->size() > 0);
    action_InternalPath->setEnabled(pattern->DataPieces()->size() > 0);
    action_InsertNodes->setEnabled(pattern->DataPieces()->size() > 0);

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Piece)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->addPatternPiece_ToolButton->setChecked(true);
        handlePatternPieceTool(true);
    }
    else if (selectedAction == action_AnchorPoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->anchorPoint_ToolButton->setChecked(true);
        handleAnchorPointTool(true);
    }
    else if (selectedAction == action_InternalPath)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->internalPath_ToolButton->setChecked(true);
        handleInternalPathTool(true);
    }
    else if (selectedAction == action_InsertNodes)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->insertNodes_ToolButton->setChecked(true);
        handleInsertNodesTool(true);
    }
}

void MainWindow::handleDetailsMenu()
{
    qCDebug(vMainWindow, "PatternPieces Menu selected. \n");
    QMenu menu;

    QAction *action_Union        = menu.addAction(QIcon(":/toolicon/32x32/union.png"),  tr("Union Tool"));
    QAction *action_ExportPieces = menu.addAction(QIcon(":/toolicon/32x32/export.png"), tr("Export Pattern Pieces"));
    QAction *selectedAction      = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Union)
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        ui->unitePieces_ToolButton->setChecked(true);
        handleUnionDetailsTool(true);
    }
    else if (selectedAction == action_ExportPieces)
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        exportPiecesAs();
    }
}

void MainWindow::handleLayoutMenu()
{
    qCDebug(vMainWindow, "Layout Menu selected. \n");


    QMenu menu;

    QAction *action_NewLayout    = menu.addAction(QIcon(":/toolicon/32x32/layout_settings.png"), tr("New Print Layout (NL)"));
    QAction *action_ExportLayout = menu.addAction(QIcon(":/toolicon/32x32/export.png"), tr("Export Layout (EL)"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_NewLayout)
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        ui->layoutSettings_ToolButton->setChecked(true);
        handleNewLayout(true);
    }
    else if (selectedAction == action_ExportLayout)
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        exportLayoutAs();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseMove save mouse position and show user.
 * @param scenePos position mouse.
 */
void MainWindow::MouseMove(const QPointF &scenePos)
{
    if (mouseCoordinates)
    {
        mouseCoordinates->updateCoordinates(scenePos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief CancelTool cancel tool.
 */
void MainWindow::CancelTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    qCDebug(vMainWindow, "Canceling tool.");
    dialogTool.clear();
    qCDebug(vMainWindow, "Dialog closed.");

    currentScene->setFocus(Qt::OtherFocusReason);
    currentScene->clearSelection();
    ui->view->itemClicked(nullptr); // Hide visualization to avoid a crash

    ui->zoomPan_Action->setChecked(false);         //Disable Pan Zoom
    ui->view->zoomPanEnabled(false);

    ui->zoomToArea_Action->setChecked(false);      //Disable Zoom to Area
    ui->view->zoomToAreaEnabled(false);

    switch ( currentTool )
    {
        case Tool::Arrow:
            ui->arrowPointer_ToolButton->setChecked(false);
            ui->arrow_Action->setChecked(false);
            helpLabel->setText("");

            // Crash: using CRTL+Z while using line tool.
            undoAction->setEnabled(false);
            redoAction->setEnabled(false);
            VAbstractTool::m_suppressContextMenu = true;
            return;
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
            Q_UNREACHABLE(); //-V501
            //Nothing to do here because we can't create this tool from main window.
            break;
        case Tool::EndLine:
            ui->pointAtDistanceAngle_ToolButton->setChecked(false);
            break;
        case Tool::Line:
            ui->line_ToolButton->setChecked(false);
            break;
        case Tool::AlongLine:
            ui->alongLine_ToolButton->setChecked(false);
            break;
        case Tool::Midpoint:
            ui->midpoint_ToolButton->setChecked(false);
            break;
        case Tool::ShoulderPoint:
            ui->shoulderPoint_ToolButton->setChecked(false);
            break;
        case Tool::Normal:
            ui->normal_ToolButton->setChecked(false);
            break;
        case Tool::Bisector:
            ui->bisector_ToolButton->setChecked(false);
            break;
        case Tool::LineIntersect:
            ui->lineIntersect_ToolButton->setChecked(false);
            break;
        case Tool::Spline:
            ui->curve_ToolButton->setChecked(false);
            break;
        case Tool::CubicBezier:
            ui->curveWithCPs_ToolButton->setChecked(false);
            break;
        case Tool::Arc:
            ui->arc_ToolButton->setChecked(false);
            break;
        case Tool::ArcWithLength:
            ui->arcWithLength_ToolButton->setChecked(false);
            break;
        case Tool::SplinePath:
            ui->spline_ToolButton->setChecked(false);
            break;
        case Tool::CubicBezierPath:
            ui->splineWithCPs_ToolButton->setChecked(false);
            break;
        case Tool::PointOfContact:
            ui->pointOfContact_ToolButton->setChecked(false);
            break;
        case Tool::Piece:
            ui->addPatternPiece_ToolButton->setChecked(false);
            break;
        case Tool::InternalPath:
            ui->internalPath_ToolButton->setChecked(false);
            break;
        case Tool::Height:
            ui->height_ToolButton->setChecked(false);
            break;
        case Tool::Triangle:
            ui->triangle_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersection:
            ui->pointIntersectXY_ToolButton->setChecked(false);
            break;
        case Tool::CutSpline:
            ui->pointAlongCurve_ToolButton->setChecked(false);
            break;
        case Tool::CutSplinePath:
            ui->pointAlongSpline_ToolButton->setChecked(false);
            break;
        case Tool::UnionDetails:
            ui->unitePieces_ToolButton->setChecked(false);
            break;
        case Tool::CutArc:
            ui->pointAlongArc_ToolButton->setChecked(false);
            break;
        case Tool::LineIntersectAxis:
            ui->lineIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::CurveIntersectAxis:
            ui->curveIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::ArcIntersectAxis:
            ui->arcIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionArcs:
            ui->pointOfIntersectionArcs_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionCircles:
            ui->pointOfIntersectionCircles_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionCurves:
            ui->pointOfIntersectionCurves_ToolButton->setChecked(false);
            break;
        case Tool::PointFromCircleAndTangent:
            ui->pointFromCircleAndTangent_ToolButton->setChecked(false);
            break;
        case Tool::PointFromArcAndTangent:
            ui->pointFromArcAndTangent_ToolButton->setChecked(false);
            break;
        case Tool::TrueDarts:
            ui->trueDarts_ToolButton->setChecked(false);
            break;
        case Tool::Group:
            ui->group_ToolButton->setChecked(false);
            break;
        case Tool::Rotation:
            ui->rotation_ToolButton->setChecked(false);
            break;
        case Tool::MirrorByLine:
            ui->mirrorByLine_ToolButton->setChecked(false);
            break;
        case Tool::MirrorByAxis:
            ui->mirrorByAxis_ToolButton->setChecked(false);
            break;
        case Tool::Move:
            ui->move_ToolButton->setChecked(false);
            break;
        case Tool::EllipticalArc:
            ui->ellipticalArc_ToolButton->setChecked(false);
            break;
        case Tool::Pin:
            ui->anchorPoint_ToolButton->setChecked(false);
            break;
        case Tool::InsertNodes:
            ui->insertNodes_ToolButton->setChecked(false);
            break;
    }

    // Crash: using CRTL+Z while using line tool.
    undoAction->setEnabled(qApp->getUndoStack()->canUndo());
    redoAction->setEnabled(qApp->getUndoStack()->canRedo());
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleArrowTool enable arrow tool.
 */
void  MainWindow::handleArrowTool(bool checked)
{
    if (checked && currentTool != Tool::Arrow)
    {
        qCDebug(vMainWindow, "Arrow tool.");
        CancelTool();
        ui->arrowPointer_ToolButton->setChecked(true);
        ui->arrow_Action->setChecked(true);
        currentTool = Tool::Arrow;
        emit EnableItemMove(true);
        emit ItemsSelection(SelectionType::ByMouseRelease);
        VAbstractTool::m_suppressContextMenu = false;

        // Only true for rubber band selection
        emit EnableLabelSelection(true);
        emit EnablePointSelection(false);
        emit EnableLineSelection(false);
        emit EnableArcSelection(false);
        emit EnableElArcSelection(false);
        emit EnableSplineSelection(false);
        emit EnableSplinePathSelection(false);
        emit EnableNodeLabelSelection(true);
        emit EnableNodePointSelection(true);
        emit EnableDetailSelection(true);// Disable when done visualization details

        // Hovering
        emit EnableLabelHover(true);
        emit EnablePointHover(true);
        emit EnableLineHover(true);
        emit EnableArcHover(true);
        emit EnableElArcHover(true);
        emit EnableSplineHover(true);
        emit EnableSplinePathHover(true);
        emit EnableNodeLabelHover(true);
        emit EnableNodePointHover(true);
        emit EnableDetailHover(true);

        ui->view->allowRubberBand(true);

        ui->view->viewport()->unsetCursor();
        helpLabel->setText("");
        ui->view->setShowToolOptions(true);
        qCDebug(vMainWindow, "Enabled arrow tool.");
    }
    else
    {
        ui->arrowPointer_ToolButton->setChecked(true);
        ui->arrow_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyPressEvent handle key press events.
 * @param event key event.
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            handleArrowTool(true);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            EndVisualization();
            break;
        case Qt::Key_Space:
            if (qApp->Seamly2DSettings()->isPanActiveSpaceKey())
            {
                ui->zoomPan_Action->setChecked(true);
            }
            break;
        default:
            break;
    }
    QMainWindow::keyPressEvent (event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key press events.
 * @param event key event.
 */
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Space:
            if (qApp->Seamly2DSettings()->isPanActiveSpaceKey())
            {
                ui->zoomPan_Action->setChecked(false);
            }
        default:
            break;
    }
    QMainWindow::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveCurrentScene save scene options before set another.
 */
void MainWindow::SaveCurrentScene()
{
    if (mode == Draw::Calculation || mode == Draw::Modeling)
    {
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        /*Save transform*/
        scene->setCurrentTransform(ui->view->transform());
        /*Save scroll bars value for previous scene.*/
        QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
        scene->setHorScrollBar(horScrollBar->value());
        QScrollBar *verScrollBar = ui->view->verticalScrollBar();
        scene->setVerScrollBar(verScrollBar->value());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RestoreCurrentScene restore scene options after change.
 */
void MainWindow::RestoreCurrentScene()
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
    SCASSERT(scene != nullptr)

    /*Set transform for current scene*/
    ui->view->setTransform(scene->transform());
    zoomScaleChanged(ui->view->transform().m11());
    /*Set value for current scene scroll bar.*/
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(scene->getHorScrollBar());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(scene->getVerScrollBar());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief draftMode_Action show draw scene.
 * @param checked true - button checked.
 */
void MainWindow::draftMode_Action(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(0);
        qCDebug(vMainWindow, "Show draft scene");
        handleArrowTool(true);

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_left_to_right_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));

        ui->draftMode_Action->setChecked(true);
        ui->pieceMode_Action->setChecked(false);
        ui->layoutMode_Action->setChecked(false);

        SaveCurrentScene();

        currentScene = draftScene;
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        mode = Draw::Calculation;
        comboBoxDraws->setCurrentIndex(currentDrawIndex);//restore current pattern peace
        drawMode = true;

        setEnableTools(true);
        SetEnableWidgets(true);

        draftScene->enablePiecesMode(qApp->Seamly2DSettings()->getShowControlPoints());
        draftScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

        updateViewToolbar();

        //ui->toggleAnchorPoints_Action->setChecked(qApp->Settings()->getShowAnchorPoints());
        //draftScene->setOriginsVisible(qApp->Settings()->getShowAnchorPoints());

        ui->useToolColor_Action->setChecked(qApp->Settings()->getUseToolColor());

        ui->draft_ToolBox->setCurrentIndex(currentToolBoxIndex);

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(true);
            gradationHeights->setVisible(true);
            gradationSizesLabel->setVisible(true);
            gradationSizes->setVisible(true);
        }
        ui->layoutPages_DockWidget->blockSignals(true);
        ui->layoutPages_DockWidget->setVisible(false);
        ui->layoutPages_DockWidget->blockSignals(false);

        ui->toolProperties_DockWidget->setVisible(isToolOptionsDockVisible);

        ui->groups_DockWidget->setWidget(groupsWidget);
        ui->groups_DockWidget->setWindowTitle(tr("Group Manager"));
        ui->groups_DockWidget->setToolTip(tr("Show which groups in the list are visible"));
        ui->groups_DockWidget->setVisible(isGroupsDockVisible);
    }
    else
    {
        ui->draftMode_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ActionDetails show details scene.
 * @param checked true - button checked.
 */
void MainWindow::ActionDetails(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(1);
        handleArrowTool(true);

        if(drawMode)
        {
            currentDrawIndex = comboBoxDraws->currentIndex();//save current pattern piece
            drawMode = false;
        }
        comboBoxDraws->setCurrentIndex(comboBoxDraws->count()-1);// Need to get data about all details

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/right_to_left_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));

        ui->draftMode_Action->setChecked(false);
        ui->pieceMode_Action->setChecked(true);
        ui->layoutMode_Action->setChecked(false);

        if(not qApp->getOpeningPattern())
        {
            if (pattern->DataPieces()->count() == 0)
            {
                QMessageBox::information(this, tr("Piece mode"), tr("You can't use Piece mode yet. "
                                                                     "Please, create at least one pattern piece."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                draftMode_Action(true);
                return;
            }
        }

        patternPiecesWidget->UpdateList();

        qCDebug(vMainWindow, "Show piece scene");
        SaveCurrentScene();

        currentScene = pieceScene;
        ui->view->itemClicked(nullptr);
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        if (mode == Draw::Calculation)
        {
            currentToolBoxIndex = ui->piece_ToolBox->currentIndex();
        }
        mode = Draw::Modeling;
        setEnableTools(true);
        SetEnableWidgets(true);

        pieceScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

        updateViewToolbar();

        ui->piece_ToolBox->setCurrentIndex(ui->piece_ToolBox->indexOf(ui->details_Page));

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(true);
            gradationHeights->setVisible(true);
            gradationSizesLabel->setVisible(true);
            gradationSizes->setVisible(true);
        }

        ui->layoutPages_DockWidget->blockSignals(true);
        ui->layoutPages_DockWidget->setVisible(false);
        ui->layoutPages_DockWidget->blockSignals(false);

        ui->toolProperties_DockWidget->setVisible(isToolOptionsDockVisible);

        ui->groups_DockWidget->setWidget(patternPiecesWidget);
        ui->groups_DockWidget->setWindowTitle(tr("Pattern Pieces"));
        ui->groups_DockWidget->setToolTip(tr("Show which pattern pieces will included in layout"));
        ui->groups_DockWidget->setVisible(isGroupsDockVisible);

        helpLabel->setText("");
    }
    else
    {
        ui->pieceMode_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ActionLayout begin creation layout.
 * @param checked true - button checked.
 */
void MainWindow::ActionLayout(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(2);
        handleArrowTool(true);

        if(drawMode)
        {
            currentDrawIndex = comboBoxDraws->currentIndex();//save current pattern piece
            drawMode = false;
        }
        comboBoxDraws->setCurrentIndex(comboBoxDraws->count()-1);// Need to get data about all details

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/right_to_left_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_right_to_left_arrow.png"));

        ui->draftMode_Action->setChecked(false);
        ui->pieceMode_Action->setChecked(false);
        ui->layoutMode_Action->setChecked(true);

        QHash<quint32, VPiece> details;
        if(not qApp->getOpeningPattern())
        {
            const QHash<quint32, VPiece> *allDetails = pattern->DataPieces();
            if (allDetails->count() == 0)
            {
                QMessageBox::information(this, tr("Layout mode"), tr("You can't use Layout mode yet. "
                                                                     "Please, create at least one pattern piece."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                draftMode_Action(true);
                return;
            }
            else
            {
                QHash<quint32, VPiece>::const_iterator i = allDetails->constBegin();
                while (i != allDetails->constEnd())
                {
                    if (i.value().IsInLayout())
                    {
                        details.insert(i.key(), i.value());
                    }
                    ++i;
                }

                if (details.count() == 0)
                {
                    QMessageBox::information(this, tr("Layout mode"),  tr("You can't use Layout mode yet. Please, "
                                                                          "include at least one pattern piece in layout."),
                                             QMessageBox::Ok, QMessageBox::Ok);
                    mode == Draw::Calculation ? draftMode_Action(true) : ActionDetails(true);
                    return;
                }
            }
        }

        comboBoxDraws->setCurrentIndex(-1);// Hide pattern pieces

        qCDebug(vMainWindow, "Show layout scene");

        SaveCurrentScene();

        try
        {
            listDetails = PrepareDetailsForLayout(details);
        }
        catch (VException &e)
        {
            listDetails.clear();
            QMessageBox::warning(this, tr("Layout mode"),
                                 tr("You can't use Layout mode yet.") + QLatin1String(" \n") + e.ErrorMessage(),
                                 QMessageBox::Ok, QMessageBox::Ok);
            mode == Draw::Calculation ? draftMode_Action(true) : ActionDetails(true);
            return;
        }

        currentScene = tempSceneLayout;
        ui->view->itemClicked(nullptr);
        ui->view->setScene(currentScene);

        if (mode == Draw::Calculation)
        {
            currentToolBoxIndex = ui->layout_ToolBox->currentIndex();
        }
        mode = Draw::Layout;
        setEnableTools(true);
        SetEnableWidgets(true);
        ui->layout_ToolBox->setCurrentIndex(ui->layout_ToolBox->indexOf(ui->layout_Page));

        mouseCoordinates->updateCoordinates(QPointF());

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(false);
            gradationHeights->setVisible(false);
            gradationSizesLabel->setVisible(false);
            gradationSizes->setVisible(false);
        }

        ui->layoutPages_DockWidget->setVisible(isLayoutsDockVisible);

        ui->toolProperties_DockWidget->blockSignals(true);
        ui->toolProperties_DockWidget->setVisible(false);
        ui->toolProperties_DockWidget->blockSignals(false);

        ui->groups_DockWidget->blockSignals(true);
        ui->groups_DockWidget->setVisible(false);
        ui->groups_DockWidget->blockSignals(false);

        showLayoutPages(ui->listWidget->currentRow());

        if (scenes.isEmpty())
        {
            ui->layoutSettings_ToolButton->click();
        }

        helpLabel->setText("");
    }
    else
    {
        ui->layoutMode_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveAs save as pattern file.
 * @return true for successes saving.
 */
bool MainWindow::SaveAs()
{
    QString filters(tr("Pattern files") + QLatin1String("(*.val)"));
    QString dir;
    if (qApp->GetPPath().isEmpty())
    {
        dir = qApp->Seamly2DSettings()->GetPathPattern();
    }
    else
    {
        dir = QFileInfo(qApp->GetPPath()).absolutePath();
    }

    bool usedNotExistedDir = false;
    QDir directory(dir);
    if (not directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"),
                                                    dir + QLatin1String("/") + tr("pattern") + QLatin1String(".val"),
                                                    filters, nullptr, QFileDialog::DontUseNativeDialog);

    auto RemoveTempDir = [usedNotExistedDir, dir]()
    {
        if (usedNotExistedDir)
        {
            QDir directory(dir);
            directory.rmpath(".");
        }
    };

    if (fileName.isEmpty())
    {
        RemoveTempDir();
        return false;
    }

    QFileInfo f( fileName );
    if (f.suffix().isEmpty() && f.suffix() != QLatin1String("val"))
    {
        fileName += QLatin1String(".val");
    }

    if (f.exists())
    {
        // Temporary try to lock the file before saving
        // Also help to rewrite current read-only pattern
        VLockGuard<char> tmp(fileName);
        if (not tmp.IsLocked())
        {
            qCCritical(vMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. This file is already opened in another window.")));
            RemoveTempDir();
            return false;
        }
    }

    // Need for restoring previous state in case of failure
    const bool wasModified = doc->IsModified(); // Need because SetReadOnly() will change internal state
    const bool readOnly = doc->IsReadOnly();

    doc->SetReadOnly(false);// Save as... disable read only state
    QString error;
    const bool result = SavePattern(fileName, error);
    if (result == false)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        // Restoring previous state
        doc->SetReadOnly(readOnly);
        doc->SetModified(wasModified);

        RemoveTempDir();
        return result;
    }

    patternReadOnly = false;

    qCDebug(vMainWindow, "Locking file");
    VlpCreateLock(lock, fileName);

    if (lock->IsLocked())
    {
        qCDebug(vMainWindow, "Pattern file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        qCDebug(vMainWindow, "Failed to lock %s", qUtf8Printable(fileName));
        qCDebug(vMainWindow, "Error type: %d", lock->GetLockError());
        qCCritical(vMainWindow, "%s",
                   qUtf8Printable(tr("Failed to lock. This file is already opened in another window. Expect "
                                     "collisions when running 2 copies of the program.")));
    }

    RemoveTempDir();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Save save pattern file.
 * @return true for successes saving.
 */
bool MainWindow::Save()
{
    if (qApp->GetPPath().isEmpty() || patternReadOnly)
    {
        return SaveAs();
    }
    else
    {
        if (m_curFileFormatVersion < VPatternConverter::PatternMaxVer
                && not ContinueFormatRewrite(m_curFileFormatVersionStr, VPatternConverter::PatternMaxVerStr))
        {
            return false;
        }
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
        const bool isFileWritable = QFileInfo(qApp->GetPPath()).isWritable();
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

        if (not isFileWritable)
        {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Question);
            messageBox.setText(tr("The document has no write permissions."));
            messageBox.setInformativeText("Do you want to change the permissions?");
            messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Yes);

            if (messageBox.exec() == QMessageBox::Yes)
            {
#ifdef Q_OS_WIN32
                qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
                bool changed = QFile::setPermissions(qApp->GetPPath(),
                                                    QFileInfo(qApp->GetPPath()).permissions() | QFileDevice::WriteUser);
#ifdef Q_OS_WIN32
                qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

                if (not changed)
                {
                    QMessageBox messageBox(this);
                    messageBox.setIcon(QMessageBox::Warning);
                    messageBox.setText(tr("Cannot set permissions for %1 to writable.").arg(qApp->GetPPath()));
                    messageBox.setInformativeText(tr("Could not save the file."));
                    messageBox.setDefaultButton(QMessageBox::Ok);
                    messageBox.setStandardButtons(QMessageBox::Ok);
                    messageBox.exec();
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        QString error;
        bool result = SavePattern(qApp->GetPPath(), error);
        if (result)
        {
            QFile::remove(qApp->GetPPath() + autosavePrefix);
            m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
            m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
        }
        else
        {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setText(tr("Could not save the file"));
            messageBox.setDefaultButton(QMessageBox::Ok);
            messageBox.setDetailedText(error);
            messageBox.setStandardButtons(QMessageBox::Ok);
            messageBox.exec();
        }
        return result;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Open ask user select pattern file.
 */
void MainWindow::Open()
{
    qCDebug(vMainWindow, "Opening new file.");
    const QString filter(tr("Pattern files (*.val)"));
    //Get list last open files
    const QStringList files = qApp->Seamly2DSettings()->GetRecentFileList();
    QString dir;
    if (files.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        //Absolute path to last open file
        dir = QFileInfo(files.first()).absolutePath();
    }
    qCDebug(vMainWindow, "Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), dir, filter, nullptr,
                                                          QFileDialog::DontUseNativeDialog);
    if (filePath.isEmpty())
    {
        return;
    }
    LoadPattern(filePath);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clear reset to default window.
 */
void MainWindow::Clear()
{
    qCDebug(vMainWindow, "Resetting main window.");
    lock.reset();
    qCDebug(vMainWindow, "Unlocked pattern file.");
    draftMode_Action(true);
    qCDebug(vMainWindow, "Returned to Draw mode.");
    setCurrentFile(QString());
    pattern->Clear();
    qCDebug(vMainWindow, "Clearing pattern.");
    if (not qApp->GetPPath().isEmpty() && not doc->MPath().isEmpty())
    {
        watcher->removePath(AbsoluteMPath(qApp->GetPPath(), doc->MPath()));
    }
    doc->clear();
    qCDebug(vMainWindow, "Clearing scenes.");
    draftScene->clear();
    pieceScene->clear();
    handleArrowTool(true);
    comboBoxDraws->clear();
    ui->draftMode_Action->setEnabled(false);
    ui->pieceMode_Action->setEnabled(false);
    ui->layoutMode_Action->setEnabled(false);
    ui->newDraft_Action->setEnabled(false);
    ui->renameDraft_Action->setEnabled(false);
    ui->save_Action->setEnabled(false);
    ui->saveAs_Action->setEnabled(false);
    ui->patternPreferences_Action->setEnabled(false);

    // disable zoom actions until a pattern is loaded
    zoomScaleSpinBox->setEnabled(false);
    ui->zoomIn_Action->setEnabled(false);
    ui->zoomOut_Action->setEnabled(false);
    ui->zoomToFit_Action->setEnabled(false);
    ui->zoomToSelected_Action->setEnabled(false);
    ui->zoom100Percent_Action->setEnabled(false);
    ui->zoomToPrevious_Action->setEnabled(false);
    ui->zoomToArea_Action->setEnabled(false);
    ui->zoomPan_Action->setEnabled(false);

    //disable history menu actions
    ui->history_Action->setEnabled(false);
    ui->table_Action->setEnabled(false);

    ui->lastTool_Action->setEnabled(false);
    ui->increaseSize_Action->setEnabled(false);
    ui->decreaseSize_Action->setEnabled(false);
    ui->useToolColor_Action->setEnabled(false);
    ui->showPointNames_Action->setEnabled(false);
    ui->toggleWireframe_Action->setEnabled(false);
    ui->toggleControlPoints_Action->setEnabled(false);
    ui->toggleAxisOrigin_Action->setEnabled(false);
    ui->toggleSeamAllowances_Action->setEnabled(false);
    ui->toggleGrainLines_Action->setEnabled(false);
    ui->toggleLabels_Action->setEnabled(false);
    //ui->toggleAnchorPoints_Action->setEnabled(false);


    //disable measurements menu actions
    ui->loadIndividual_Action->setEnabled(false);
    ui->loadMultisize_Action->setEnabled(false);
    ui->unloadMeasurements_Action->setEnabled(false);
    ui->editCurrent_Action->setEnabled(false);

    setEnableTools(false);
    qApp->setPatternUnit(Unit::Cm);
    qApp->setPatternType(MeasurementsType::Unknown);

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    CleanLayout();
    listDetails.clear(); // don't move to CleanLayout()
    qApp->getUndoStack()->clear();
    toolProperties->ClearPropertyBrowser();
    toolProperties->itemClicked(nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::FileClosedCorrect()
{
    WriteSettings();

    //File was closed correct.
    QStringList restoreFiles = qApp->Seamly2DSettings()->GetRestoreFileList();
    restoreFiles.removeAll(qApp->GetPPath());
    qApp->Seamly2DSettings()->SetRestoreFileList(restoreFiles);

    // Remove autosave file
    QFile autofile(qApp->GetPPath() + autosavePrefix);
    if (autofile.exists())
    {
        autofile.remove();
    }
    qCDebug(vMainWindow, "File %s closed correct.", qUtf8Printable(qApp->GetPPath()));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::FullParseFile()
{
    qCDebug(vMainWindow, "Full parsing file");

    toolProperties->ClearPropertyBrowser();
    try
    {
        SetEnabledGUI(true);
        doc->Parse(Document::FullParse);
    }
    catch (const VExceptionUndo &e)
    {
        Q_UNUSED(e)
        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }
    catch (const VExceptionObjectError &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                               qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionConversionError &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                               qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                               qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionWrongId &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                               qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")),
                               qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    QString patternPiece;
    if (comboBoxDraws->currentIndex() != -1)
    {
        patternPiece = comboBoxDraws->itemText(comboBoxDraws->currentIndex());
    }
    comboBoxDraws->blockSignals(true);
    comboBoxDraws->clear();

    QStringList patternPieceNames = doc->getPatternPieces();
    patternPieceNames.sort();
    comboBoxDraws->addItems(patternPieceNames);

    if (not drawMode)
    {
        comboBoxDraws->setCurrentIndex(comboBoxDraws->count()-1);
    }
    else
    {
        const qint32 index = comboBoxDraws->findText(patternPiece);
        if ( index != -1 )
        {
            comboBoxDraws->setCurrentIndex(index);
        }
    }
    comboBoxDraws->blockSignals(false);
    ui->patternPreferences_Action->setEnabled(true);

    GlobalchangeDraftBlock(patternPiece);

    setEnableTools(comboBoxDraws->count() > 0);
    patternPiecesWidget->UpdateList();

    VMainGraphicsView::NewSceneRect(draftScene, qApp->getSceneView());
    VMainGraphicsView::NewSceneRect(pieceScene, qApp->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::GlobalchangeDraftBlock(const QString &patternPiece)
{
    const qint32 index = comboBoxDraws->findText(patternPiece);
    try
    {
        if ( index != -1 )
        { // -1 for not found
            changeDraftBlock(index, false);
        }
        else
        {
            changeDraftBlock(0, false);
        }
    }
    catch (VExceptionBadId &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Bad id.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const VExceptionEmptyParameter &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        SetEnabledGUI(false);
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetEnabledGUI(bool enabled)
{
    if (guiEnabled != enabled)
    {
        if (enabled == false)
        {
            handleArrowTool(true);
            qApp->getUndoStack()->clear();
        }
        SetEnableWidgets(enabled);

        guiEnabled = enabled;

        setEnableTools(enabled);
        ui->statusBar->setEnabled(enabled);
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
    #endif
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetEnableWidgets enable action button.
 * @param enable enable value.
 */
void MainWindow::SetEnableWidgets(bool enable)
{
    const bool draftStage = (mode == Draw::Calculation);
    const bool pieceStage = (mode == Draw::Modeling);
    const bool designStage = (draftStage || pieceStage);
    const bool layoutStage = (mode == Draw::Layout);

    comboBoxDraws->setEnabled(enable && draftStage);
    ui->arrow_Action->setEnabled(enable && designStage);

    // enable file menu actions
    ui->save_Action->setEnabled(isWindowModified() && enable && not patternReadOnly);
    ui->saveAs_Action->setEnabled(enable);
    ui->patternPreferences_Action->setEnabled(enable && designStage);

   // enable edit  menu actions
    undoAction->setEnabled(enable && designStage && qApp->getUndoStack()->canUndo());
    redoAction->setEnabled(enable && designStage && qApp->getUndoStack()->canRedo());

    // enable view menu actions
    ui->draftMode_Action->setEnabled(enable);
    ui->pieceMode_Action->setEnabled(enable);
    ui->layoutMode_Action->setEnabled(enable);
    zoomScaleSpinBox->setEnabled(enable);
    ui->zoomIn_Action->setEnabled(enable);
    ui->zoomOut_Action->setEnabled(enable);
    ui->zoomToFit_Action->setEnabled(enable);
    ui->zoomToSelected_Action->setEnabled(enable);
    ui->zoom100Percent_Action->setEnabled(enable);
    ui->zoomToPrevious_Action->setEnabled(enable);
    ui->zoomToArea_Action->setEnabled(enable);
    ui->zoomPan_Action->setEnabled(enable);

    ui->increaseSize_Action->setEnabled(enable);
    ui->decreaseSize_Action->setEnabled(enable);
    ui->useToolColor_Action->setEnabled(enable && draftStage);
    ui->showPointNames_Action->setEnabled(enable);
    ui->toggleWireframe_Action->setEnabled(enable);
    ui->toggleControlPoints_Action->setEnabled(enable && draftStage);
    ui->toggleAxisOrigin_Action->setEnabled(enable);
    ui->toggleSeamAllowances_Action->setEnabled(enable && pieceStage);
    ui->toggleGrainLines_Action->setEnabled(enable && pieceStage);
    ui->toggleLabels_Action->setEnabled(enable && pieceStage);
    //ui->toggleAnchorPoints_Action->setEnabled(enable && draftStage);


    //enable tool menu actions
    ui->newDraft_Action->setEnabled(enable && draftStage);
    ui->renameDraft_Action->setEnabled(enable && draftStage);

    //enable measurement menu actions
    ui->loadIndividual_Action->setEnabled(enable && designStage);
    ui->loadMultisize_Action->setEnabled(enable && designStage);
    ui->unloadMeasurements_Action->setEnabled(enable && designStage);
    ui->table_Action->setEnabled(enable && draftStage);

    //enable history menu actions
    ui->history_Action->setEnabled(enable && draftStage);

    //enable utilities menu actions
    ui->calculator_Action->setEnabled(enable);
    ui->decimalChart_Action->setEnabled(enable);

    //enable help menu
    ui->shortcuts_Action->setEnabled(enable);

    //enable dock widget actions
    actionDockWidgetToolOptions->setEnabled(enable && designStage);
    actionDockWidgetGroups->setEnabled(enable && designStage);
    actionDockWidgetLayouts->setEnabled(enable && layoutStage);

    //Now we don't want allow user call context menu
    draftScene->SetDisableTools(!enable, doc->getActiveDraftBlockName());
    ui->view->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateHeightsList(const QStringList &list)
{
    QString val;
    if (gradationHeights->currentIndex() != -1)
    {
        val = gradationHeights->currentText();
    }

    gradationHeights->blockSignals(true);
    gradationHeights->clear();
    gradationHeights->addItems(list);
    gradationHeights->blockSignals(false);

    int index = gradationHeights->findText(val);
    if (index != -1)
    {
        gradationHeights->setCurrentIndex(index);
    }
    else
    {
        ChangedHeight(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateSizesList(const QStringList &list)
{
    QString val;
    if (gradationSizes->currentIndex() != -1)
    {
        val = gradationSizes->currentText();
    }

    gradationSizes->blockSignals(true);
    gradationSizes->clear();
    gradationSizes->addItems(list);
    gradationSizes->blockSignals(false);

    int index = gradationSizes->findText(val);
    if (index != -1)
    {
        gradationSizes->setCurrentIndex(index);
    }
    else
    {
        ChangedSize(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NewPattern create new empty pattern.
 */
void MainWindow::New()
{
    if (comboBoxDraws->count() == 0)
    {
        // Creating a new pattern design requires creating a new pattern piece
        qCDebug(vMainWindow, "New Pattern Piece.");
        QString patternPieceName = tr("Pattern piece %1").arg(comboBoxDraws->count()+1);
        qCDebug(vMainWindow, "Generated Pattern Piece name: %s", qUtf8Printable(patternPieceName));

        qCDebug(vMainWindow, "First Pattern Piece");
        DialogNewPattern newPattern(pattern, patternPieceName, this);
        if (newPattern.exec() == QDialog::Accepted)
        {
            patternPieceName = newPattern.name();
            qApp->setPatternUnit(newPattern.PatternUnit());
            qCDebug(vMainWindow, "Pattern Piece name: %s", qUtf8Printable(patternPieceName));
        }
        else
        {
            qCDebug(vMainWindow, "Creating new Pattern Piece was canceled.");
            return;
        }

        //Set scene size to size scene view
        VMainGraphicsView::NewSceneRect(draftScene, ui->view);
        VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

        AddPP(patternPieceName);

        mouseCoordinates = new MouseCoordinates(qApp->patternUnit());
        ui->statusBar->addPermanentWidget((mouseCoordinates));

        m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
        m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
    }
    else
    {
        OpenNewSeamly2D();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief haveChange enable action save if we have unsaved change.
 */
void MainWindow::PatternChangesWereSaved(bool saved)
{
    if (guiEnabled)
    {
        const bool state = doc->IsModified() || !saved;
        setWindowModified(state);
        not patternReadOnly ? ui->save_Action->setEnabled(state): ui->save_Action->setEnabled(false);
        isLayoutStale = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedSize change new size value.
 * @param index index of the selected item.
 */
void MainWindow::ChangedSize(int index)
{
    const int size = static_cast<int>(VContainer::size());
    if (UpdateMeasurements(AbsoluteMPath(qApp->GetPPath(), doc->MPath()),
                           gradationSizes.data()->itemText(index).toInt(),
                           static_cast<int>(VContainer::height())))
    {
        doc->LiteParseTree(Document::LiteParse);
        emit pieceScene->DimensionsChanged();
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't update measurements.")));

        const qint32 index = gradationSizes->findText(QString().setNum(size));
        if (index != -1)
        {
            gradationSizes->setCurrentIndex(index);
        }
        else
        {
            qCDebug(vMainWindow, "Couldn't restore size value.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedGrowth change new height value.
 * @param index index of the selected item.
 */
void MainWindow::ChangedHeight(int index)
{
    const int height = static_cast<int>(VContainer::height());
    if (UpdateMeasurements(AbsoluteMPath(qApp->GetPPath(), doc->MPath()), static_cast<int>(VContainer::size()),
                           gradationHeights.data()->itemText(index).toInt()))
    {
        doc->LiteParseTree(Document::LiteParse);
        emit pieceScene->DimensionsChanged();
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't update measurements.")));

        const qint32 index = gradationHeights->findText(QString().setNum(height));
        if (index != -1)
        {
            gradationHeights->setCurrentIndex(index);
        }
        else
        {
            qCDebug(vMainWindow, "Couldn't restore height value.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetDefaultHeight()
{
    const QString defHeight = QString().setNum(doc->GetDefCustomHeight());
    int index = gradationHeights->findText(defHeight);
    if (index != -1)
    {
        gradationHeights->setCurrentIndex(index);
    }
    else
    {
        index = gradationHeights->findText(QString().setNum(VContainer::height()));
        if (index != -1)
        {
            gradationHeights->setCurrentIndex(index);
        }
    }
    VContainer::SetHeight(gradationHeights->currentText().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetDefaultSize()
{
    const QString defSize = QString().setNum(doc->GetDefCustomSize());
    int index = gradationSizes->findText(defSize);
    if (index != -1)
    {
        gradationSizes->setCurrentIndex(index);
    }
    else
    {
        index = gradationSizes->findText(QString().setNum(VContainer::size()));
        if (index != -1)
        {
            gradationSizes->setCurrentIndex(index);
        }
    }
    VContainer::SetSize(gradationSizes->currentText().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setEnableTools enable button.
 * @param enable enable value.
 */
void MainWindow::setEnableTools(bool enable)
{
    bool draftTools = false;
    bool pieceTools = false;
    bool layoutTools = false;

    switch (mode)
    {
        case Draw::Calculation:
            draftTools = enable;
            break;
        case Draw::Modeling:
            pieceTools = enable;
            break;
        case Draw::Layout:
            layoutTools = enable;
            break;
        default:
            break;
    }

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    //Toolbox Drawing Tools
    //Points
    ui->pointAtDistanceAngle_ToolButton->setEnabled(draftTools);
    ui->alongLine_ToolButton->setEnabled(draftTools);
    ui->normal_ToolButton->setEnabled(draftTools);
    ui->bisector_ToolButton->setEnabled(draftTools);
    ui->shoulderPoint_ToolButton->setEnabled(draftTools);
    ui->pointOfContact_ToolButton->setEnabled(draftTools);
    ui->triangle_ToolButton->setEnabled(draftTools);
    ui->pointIntersectXY_ToolButton->setEnabled(draftTools);
    ui->height_ToolButton->setEnabled(draftTools);
    ui->lineIntersectAxis_ToolButton->setEnabled(draftTools);
    ui->midpoint_ToolButton->setEnabled(draftTools);

    //Lines
    ui->line_ToolButton->setEnabled(draftTools);
    ui->lineIntersect_ToolButton->setEnabled(draftTools);

    //Curves
    ui->curve_ToolButton->setEnabled(draftTools);
    ui->spline_ToolButton->setEnabled(draftTools);
    ui->curveWithCPs_ToolButton->setEnabled(draftTools);
    ui->splineWithCPs_ToolButton->setEnabled(draftTools);
    ui->pointAlongCurve_ToolButton->setEnabled(draftTools);
    ui->pointAlongSpline_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionCurves_ToolButton->setEnabled(draftTools);
    ui->curveIntersectAxis_ToolButton->setEnabled(draftTools);

    //Arcs
    ui->arc_ToolButton->setEnabled(draftTools);
    ui->pointAlongArc_ToolButton->setEnabled(draftTools);
    ui->arcIntersectAxis_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionArcs_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionCircles_ToolButton->setEnabled(draftTools);
    ui->pointFromCircleAndTangent_ToolButton->setEnabled(draftTools);
    ui->pointFromArcAndTangent_ToolButton->setEnabled(draftTools);
    ui->arcWithLength_ToolButton->setEnabled(draftTools);
    ui->ellipticalArc_ToolButton->setEnabled(draftTools);

    //Operations
    ui->group_ToolButton->setEnabled(draftTools);
    ui->rotation_ToolButton->setEnabled(draftTools);
    ui->mirrorByLine_ToolButton->setEnabled(draftTools);
    ui->mirrorByAxis_ToolButton->setEnabled(draftTools);
    ui->move_ToolButton->setEnabled(draftTools);
    ui->trueDarts_ToolButton->setEnabled(draftTools);
    ui->exportDraftBlocks_ToolButton->setEnabled(draftTools);

    //Piece
    ui->addPatternPiece_ToolButton->setEnabled(draftTools);
    ui->anchorPoint_ToolButton->setEnabled(draftTools  & (pattern->DataPieces()->size() > 0));
    ui->internalPath_ToolButton->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->insertNodes_ToolButton->setEnabled(draftTools   & (pattern->DataPieces()->size() > 0));

    //Details
    ui->unitePieces_ToolButton->setEnabled(pieceTools);
    ui->exportPiecesAs_ToolButton->setEnabled(pieceTools);

    //Layout
    ui->layoutSettings_ToolButton->setEnabled(layoutTools);

    //enable Toolbox Toolbar actions
    ui->arrow_Action->setEnabled(enable);
    ui->points_Action->setEnabled(draftTools);
    ui->lines_Action->setEnabled(draftTools);
    ui->arcs_Action->setEnabled(draftTools);
    ui->curves_Action->setEnabled(draftTools);
    ui->modifications_Action->setEnabled(draftTools);
    ui->pieces_Action->setEnabled(draftTools);
    ui->details_Action->setEnabled(pieceTools);
    ui->layout_Action->setEnabled(layoutTools);

    //Menu Actions
    //Points
    ui->midpoint_Action->setEnabled(draftTools);
    ui->pointAtDistanceAngle_Action->setEnabled(draftTools);
    ui->pointAlongLine_Action->setEnabled(draftTools);
    ui->pointAlongPerpendicular_Action->setEnabled(draftTools);
    ui->bisector_Action->setEnabled(draftTools);
    ui->pointOnShoulder_Action->setEnabled(draftTools);
    ui->pointOfContact_Action->setEnabled(draftTools);
    ui->triangle_Action->setEnabled(draftTools);
    ui->pointIntersectXY_Action->setEnabled(draftTools);
    ui->perpendicularPoint_Action->setEnabled(draftTools);
    ui->pointIntersectAxis_Action->setEnabled(draftTools);

    //Lines
    ui->lineTool_Action->setEnabled(draftTools);
    ui->lineIntersect_Action->setEnabled(draftTools);

    //Curves
    ui->curve_Action->setEnabled(draftTools);
    ui->spline_Action->setEnabled(draftTools);
    ui->curveWithCPs_Action->setEnabled(draftTools);
    ui->splineWithCPs_Action->setEnabled(draftTools);
    ui->pointAlongCurve_Action->setEnabled(draftTools);
    ui->pointAlongSpline_Action->setEnabled(draftTools);
    ui->curveIntersectCurve_Action->setEnabled(draftTools);
    ui->splineIntersectAxis_Action->setEnabled(draftTools);

    //Arcs
    ui->arcTool_Action->setEnabled(draftTools);
    ui->pointAlongArc_Action->setEnabled(draftTools);
    ui->arcIntersectAxis_Action->setEnabled(draftTools);
    ui->arcIntersectArc_Action->setEnabled(draftTools);
    ui->circleIntersect_Action->setEnabled(draftTools);
    ui->circleTangent_Action->setEnabled(draftTools);
    ui->arcTangent_Action->setEnabled(draftTools);;
    ui->arcWithLength_Action->setEnabled(draftTools);
    ui->ellipticalArc_Action->setEnabled(draftTools);

    //Operations
    ui->group_Action->setEnabled(draftTools);
    ui->rotation_Action->setEnabled(draftTools);
    ui->mirrorByLine_Action->setEnabled(draftTools);
    ui->mirrorByAxis_Action->setEnabled(draftTools);
    ui->move_Action->setEnabled(draftTools);
    ui->trueDarts_Action->setEnabled(draftTools);
    ui->exportDraftBlocks_Action->setEnabled(draftTools);

    //Piece
    ui->addPiece_Action->setEnabled(draftTools);
    ui->anchorPoint_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->internalPath_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->insertNodes_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));

    //Details
    ui->union_Action->setEnabled(pieceTools);
    ui->exportPieces_Action->setEnabled(pieceTools);

    //Layout
    ui->newPrintLayout_Action->setEnabled(layoutTools);
    ui->exportLayout_Action->setEnabled(layoutTools);
    ui->lastTool_Action->setEnabled(draftTools);

    ui->arrowPointer_ToolButton->setEnabled(draftTools || pieceTools);
    ui->arrowPointer_ToolButton->setChecked(draftTools || pieceTools);
    ui->arrow_Action->setChecked(draftTools || pieceTools);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetLayoutModeActions()
{
    const bool enabled = not scenes.isEmpty();

    ui->exportLayout_ToolButton->setEnabled(enabled);
    ui->exportAs_Action->setEnabled(enabled);
    ui->printPreview_Action->setEnabled(enabled);
    ui->printPreviewTiled_Action->setEnabled(enabled);
    ui->print_Action->setEnabled(enabled);
    ui->printTiled_Action->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MinimumScrollBar set scroll bar to minimum.
 */
void MainWindow::MinimumScrollBar()
{
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(horScrollBar->minimum());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(verScrollBar->minimum());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SavePattern save pattern file.
 * @param fileName pattern file name.
 * @return true if all is good.
 */
bool MainWindow::SavePattern(const QString &fileName, QString &error)
{
    qCDebug(vMainWindow, "Saving pattern file %s.", qUtf8Printable(fileName));
    QFileInfo tempInfo(fileName);

    const QString mPath = AbsoluteMPath(qApp->GetPPath(), doc->MPath());
    if (not mPath.isEmpty() && qApp->GetPPath() != fileName)
    {
        doc->SetMPath(RelativeMPath(fileName, mPath));
    }

    const bool result = doc->SaveDocument(fileName, error);
    if (result)
    {
        if (tempInfo.suffix() != QLatin1String("autosave"))
        {
            setCurrentFile(fileName);
            helpLabel->setText(tr("File saved"));
            qCDebug(vMainWindow, "File %s saved.", qUtf8Printable(fileName));
            PatternChangesWereSaved(result);
        }
    }
    else
    {
        doc->SetMPath(mPath);
        emit doc->UpdatePatternLabel();
        qCDebug(vMainWindow, "Could not save file %s. %s.", qUtf8Printable(fileName), qUtf8Printable(error));
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AutoSavePattern start safe saving.
 */
void MainWindow::AutoSavePattern()
{
    qCDebug(vMainWindow, "Autosaving pattern.");

    if (qApp->GetPPath().isEmpty() == false && this->isWindowModified() == true)
    {
        QString autofile = qApp->GetPPath() + autosavePrefix;
        QString error;
        SavePattern(autofile, error);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentFile the function is called to reset the state of a few variables when a file
 * is loaded or saved, or when the user starts editing a new file (in which case fileName is empty).
 * @param fileName file name.
 */
void MainWindow::setCurrentFile(const QString &fileName)
{
    qCDebug(vMainWindow, "Set current name to \"%s\"", qUtf8Printable(fileName));
    qApp->SetPPath(fileName);
    doc->SetPatternWasChanged(true);
    emit doc->UpdatePatternLabel();
    qApp->getUndoStack()->setClean();

    if (not qApp->GetPPath().isEmpty() && VApplication::IsGUIMode())
    {
        qCDebug(vMainWindow, "Updating recent file list.");
        VSettings *settings = qApp->Seamly2DSettings();
        QStringList files = settings->GetRecentFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }

        settings->SetRecentFileList(files);
        UpdateRecentFileActions();

        qCDebug(vMainWindow, "Updating restore file list.");
        QStringList restoreFiles = settings->GetRestoreFileList();
        restoreFiles.removeAll(fileName);
        restoreFiles.prepend(fileName);
        settings->SetRestoreFileList(restoreFiles);
    }

    UpdateWindowTitle();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ReadSettings read setting for app.
 */
void MainWindow::ReadSettings()
{
    qCDebug(vMainWindow, "Reading settings.");
    const VSettings *settings = qApp->Seamly2DSettings();
    restoreGeometry(settings->GetGeometry());
    restoreState(settings->GetWindowState());
    restoreState(settings->GetToolbarsState(), APP_VERSION);

    // Scene antialiasing
    const bool graphOutputValue = settings->GetGraphicalOutput();
    ui->view->setRenderHint(QPainter::Antialiasing, graphOutputValue);
    ui->view->setRenderHint(QPainter::SmoothPixmapTransform, graphOutputValue);

    // Stack limit
    qApp->getUndoStack()->setUndoLimit(settings->GetUndoCount());

    // Text under tool button icon
    ToolBarStyles();

    isToolOptionsDockVisible = ui->toolProperties_DockWidget->isVisible();
    isGroupsDockVisible = ui->groups_DockWidget->isVisible();
    isLayoutsDockVisible = ui->layoutPages_DockWidget->isVisible();
    isToolboxDockVisible = ui->layoutPages_DockWidget->isVisible();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief WriteSettings save setting for app.
 */
void MainWindow::WriteSettings()
{
    draftMode_Action(true);

    VSettings *setings = qApp->Seamly2DSettings();
    setings->SetGeometry(saveGeometry());
    setings->SetWindowState(saveState());
    setings->SetToolbarsState(saveState(APP_VERSION));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MaybeSave The function is called to save pending changes.
 * @return returns true in all cases, except when the user clicks Cancel.
 */
bool MainWindow::MaybeSave()
{
    if (this->isWindowModified() && guiEnabled)
    {
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
                                                               tr("The pattern has been modified.\n"
                                                                  "Do you want to save your changes?"),
                                                               QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
                                                               QMessageBox::Cancel, this, Qt::Sheet));

        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);

        messageBox->setButtonText(QMessageBox::Yes,
                                  qApp->GetPPath().isEmpty() || patternReadOnly ? tr("Save...") : tr("Save"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        messageBox->setWindowModality(Qt::ApplicationModal);
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        switch (ret)
        {
            case QMessageBox::Yes:
                if (patternReadOnly)
                {
                    return SaveAs();
                }
                else
                {
                    return Save();
                }
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateRecentFileActions()
{
    qCDebug(vMainWindow, "Updating recent file actions.");
    const QStringList files = qApp->Seamly2DSettings()->GetRecentFileList();
    const int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));

    for (int i = 0; i < numRecentFiles; ++i)
    {
       QString text = QString("&%1. %2").arg(i + 1).arg(StrippedName(files.at(i)));
       recentFileActs[i]->setText(text);
       recentFileActs[i]->setData(files.at(i));
       recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    {
       recentFileActs[j]->setVisible(false);
    }

    separatorAct->setVisible(numRecentFiles > 0);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CreateMenus()
{
    //Add last 5 most recent projects to file menu.
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        ui->file_Menu->insertAction(ui->exit_Action, recentFileActs[i]);
    }
    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->file_Menu->insertAction(ui->exit_Action, separatorAct);
    UpdateRecentFileActions();

    //Add Undo/Redo actions to edit menu.
    QList<QKeySequence> undoShortcuts;
    undoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Z));
    undoShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_Backspace));

    undoAction = qApp->getUndoStack()->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(undoShortcuts);
    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    connect(undoAction, &QAction::triggered, toolProperties, &VToolOptionsPropertyBrowser::RefreshOptions);
    ui->edit_Menu->addAction(undoAction);
    ui->edit_Toolbar->addAction(undoAction);

    QList<QKeySequence> redoShortcuts;
    redoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Y));
    redoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_Z));
    redoShortcuts.append(QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_Backspace));

    redoAction = qApp->getUndoStack()->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(redoShortcuts);
    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    connect(redoAction, &QAction::triggered, toolProperties, &VToolOptionsPropertyBrowser::RefreshOptions);
    ui->edit_Menu->addAction(redoAction);
    ui->edit_Toolbar->addAction(redoAction);

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->edit_Menu->addAction(separatorAct);

    AddDocks();

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->view_Menu->addAction(separatorAct);

    QMenu *menu = new QMenu("Toolbars");
    ui->view_Menu->addMenu(menu);

    menu->addAction(ui->file_ToolBar->toggleViewAction());
    connect(ui->file_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->file_ToolBar->setVisible(visible);
    });
    menu->addAction(ui->edit_Toolbar->toggleViewAction());
    connect(ui->edit_Toolbar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->edit_Toolbar->setVisible(visible);
    });
    menu->addAction(ui->view_ToolBar->toggleViewAction());
    connect(ui->view_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->view_ToolBar->setVisible(visible);
    });

    menu->addAction(ui->mode_ToolBar->toggleViewAction());
    connect(ui->mode_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->mode_ToolBar->setVisible(visible);
    });
    menu->addAction(ui->draft_ToolBar->toggleViewAction());
    connect(ui->draft_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->draft_ToolBar->setVisible(visible);
    });

    menu->addAction(ui->zoom_ToolBar->toggleViewAction());
    connect(ui->zoom_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->zoom_ToolBar->setVisible(visible);
    });
    menu->addAction(ui->tools_ToolBox_ToolBar->toggleViewAction());
    menu->addAction(ui->points_ToolBar->toggleViewAction());
    menu->addAction(ui->lines_ToolBar->toggleViewAction());
    menu->addAction(ui->curves_ToolBar->toggleViewAction());
    menu->addAction(ui->arcs_ToolBar->toggleViewAction());
    menu->addAction(ui->operations_ToolBar->toggleViewAction());
    menu->addAction(ui->pieces_ToolBar->toggleViewAction());
    menu->addAction(ui->details_ToolBar->toggleViewAction());
    menu->addAction(ui->layout_ToolBar->toggleViewAction());
    menu->addAction(ui->pointName_ToolBar->toggleViewAction());
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
void MainWindow::LastUsedTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    if (currentTool == lastUsedTool)
    {
        return;
    }

    switch ( lastUsedTool )
    {
        case Tool::Arrow:
            ui->arrowPointer_ToolButton->setChecked(true);
            ui->arrow_Action->setChecked(true);
            handleArrowTool(true);
            break;
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
            Q_UNREACHABLE(); //-V501
            //Nothing to do here because we can't create this tool from main window.
            break;
        case Tool::EndLine:
            ui->pointAtDistanceAngle_ToolButton->setChecked(true);
            handlePointAtDistanceAngleTool(true);
            break;
        case Tool::Line:
            ui->line_ToolButton->setChecked(true);
            handleLineTool(true);
            break;
        case Tool::AlongLine:
            ui->alongLine_ToolButton->setChecked(true);
            handleAlongLineTool(true);
            break;
        case Tool::Midpoint:
            ui->midpoint_ToolButton->setChecked(true);
            handleMidpointTool(true);
            break;
        case Tool::ShoulderPoint:
            ui->shoulderPoint_ToolButton->setChecked(true);
            handleShoulderPointTool(true);
            break;
        case Tool::Normal:
            ui->normal_ToolButton->setChecked(true);
            handleNormalTool(true);
            break;
        case Tool::Bisector:
            ui->bisector_ToolButton->setChecked(true);
            handleBisectorTool(true);
            break;
        case Tool::LineIntersect:
            ui->lineIntersect_ToolButton->setChecked(true);
            handleLineIntersectTool(true);
            break;
        case Tool::Spline:
            ui->curve_ToolButton->setChecked(true);
            handleCurveTool(true);
            break;
        case Tool::CubicBezier:
            ui->curveWithCPs_ToolButton->setChecked(true);
            handleCurveWithControlPointsTool(true);
            break;
        case Tool::Arc:
            ui->arc_ToolButton->setChecked(true);
            handleArcTool(true);
            break;
        case Tool::SplinePath:
            ui->spline_ToolButton->setChecked(true);
            handleSplineTool(true);
            break;
        case Tool::CubicBezierPath:
            ui->splineWithCPs_ToolButton->setChecked(true);
            handleSplineWithControlPointsTool(true);
            break;
        case Tool::PointOfContact:
            ui->pointOfContact_ToolButton->setChecked(true);
            handlePointOfContactTool(true);
            break;
        case Tool::Piece:
            ui->addPatternPiece_ToolButton->setChecked(true);
            handlePatternPieceTool(true);
            break;
        case Tool::InternalPath:
            ui->internalPath_ToolButton->setChecked(true);
            handleInternalPathTool(true);
            break;
        case Tool::Height:
            ui->height_ToolButton->setChecked(true);
            handleHeightTool(true);
            break;
        case Tool::Triangle:
            ui->triangle_ToolButton->setChecked(true);
            handleTriangleTool(true);
            break;
        case Tool::PointOfIntersection:
            ui->pointIntersectXY_ToolButton->setChecked(true);
            handlePointIntersectXYTool(true);
            break;
        case Tool::PointOfIntersectionArcs:
            ui->pointOfIntersectionArcs_ToolButton->setChecked(true);
            handlePointOfIntersectionArcsTool(true);
            break;
        case Tool::CutSpline:
            ui->pointAlongCurve_ToolButton->setChecked(true);
            handlePointAlongCurveTool(true);
            break;
        case Tool::CutSplinePath:
            ui->pointAlongSpline_ToolButton->setChecked(true);
            handlePointAlongSplineTool(true);
            break;
        case Tool::UnionDetails:
            ui->unitePieces_ToolButton->setChecked(true);
            handleUnionDetailsTool(true);
            break;
        case Tool::CutArc:
            ui->pointAlongArc_ToolButton->setChecked(true);
            handlePointAlongArcTool(true);
            break;
        case Tool::LineIntersectAxis:
            ui->lineIntersectAxis_ToolButton->setChecked(true);
            handleLineIntersectAxisTool(true);
            break;
        case Tool::CurveIntersectAxis:
            ui->curveIntersectAxis_ToolButton->setChecked(true);
            handleCurveIntersectAxisTool(true);
            break;
        case Tool::ArcIntersectAxis:
            ui->arcIntersectAxis_ToolButton->setChecked(true);
            handleArcIntersectAxisTool(true);
            break;
        case Tool::PointOfIntersectionCircles:
            ui->pointOfIntersectionCircles_ToolButton->setChecked(true);
            handlePointOfIntersectionCirclesTool(true);
            break;
        case Tool::PointOfIntersectionCurves:
            ui->pointOfIntersectionCurves_ToolButton->setChecked(true);
            handleCurveIntersectCurveTool(true);
            break;
        case Tool::PointFromCircleAndTangent:
            ui->pointFromCircleAndTangent_ToolButton->setChecked(true);
            handlePointFromCircleAndTangentTool(true);
            break;
        case Tool::PointFromArcAndTangent:
            ui->pointFromArcAndTangent_ToolButton->setChecked(true);
            handlePointFromArcAndTangentTool(true);
            break;
        case Tool::ArcWithLength:
            ui->arcWithLength_ToolButton->setChecked(true);
            handleArcWithLengthTool(true);
            break;
        case Tool::TrueDarts:
            ui->trueDarts_ToolButton->setChecked(true);
            handleTrueDartTool(true);
            break;
        case Tool::Group:
            ui->group_ToolButton->setChecked(true);
            handleGroupTool(true);
            break;
        case Tool::Rotation:
            ui->rotation_ToolButton->setChecked(true);
            handleRotationTool(true);
            break;
        case Tool::MirrorByLine:
            ui->mirrorByLine_ToolButton->setChecked(true);
            handleMirrorByLineTool(true);
            break;
        case Tool::MirrorByAxis:
            ui->mirrorByAxis_ToolButton->setChecked(true);
            handleMirrorByAxisTool(true);
            break;
        case Tool::Move:
            ui->move_ToolButton->setChecked(true);
            handleMoveTool(true);
            break;
        case Tool::EllipticalArc:
            ui->ellipticalArc_ToolButton->setChecked(true);
            handleEllipticalArcTool(true);
            break;
        case Tool::Pin:
            ui->anchorPoint_ToolButton->setChecked(true);
            handleAnchorPointTool(true);
            break;
        case Tool::InsertNodes:
            ui->insertNodes_ToolButton->setChecked(true);
            handleInsertNodesTool(true);
            break;
    }
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::AddDocks()
{
    //Add dock
    actionDockWidgetToolOptions = ui->toolProperties_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetToolOptions);
    connect(ui->toolProperties_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isToolOptionsDockVisible = visible;
    });

    actionDockWidgetGroups = ui->groups_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetGroups);
    connect(ui->groups_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isGroupsDockVisible = visible;
    });

    actionDockWidgetLayouts = ui->layoutPages_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetLayouts);
    connect(ui->layoutPages_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isLayoutsDockVisible = visible;
    });

    actionDockWidgetToolbox = ui->toolbox_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetToolbox);
    connect(ui->toolbox_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isToolboxDockVisible  = visible;
    });
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitDocksContain()
{
    qCDebug(vMainWindow, "Initialize Tool Options Property editor.");
    toolProperties = new VToolOptionsPropertyBrowser(ui->toolProperties_DockWidget);

    connect(ui->view, &VMainGraphicsView::itemClicked, toolProperties, &VToolOptionsPropertyBrowser::itemClicked);
    connect(doc, &VPattern::FullUpdateFromFile, toolProperties, &VToolOptionsPropertyBrowser::UpdateOptions);

    qCDebug(vMainWindow, "Initialize Groups manager.");
    groupsWidget = new VWidgetGroups(doc, this);
    ui->groups_DockWidget->setWidget(groupsWidget);

    patternPiecesWidget = new VWidgetDetails(pattern, doc, this);
    connect(doc, &VPattern::FullUpdateFromFile, patternPiecesWidget, &VWidgetDetails::UpdateList);
    connect(doc, &VPattern::UpdateInLayoutList, patternPiecesWidget, &VWidgetDetails::UpdateList);
    connect(doc, &VPattern::ShowDetail, patternPiecesWidget, &VWidgetDetails::SelectDetail);
    connect(patternPiecesWidget, &VWidgetDetails::Highlight, pieceScene, &VMainGraphicsScene::HighlightItem);
    patternPiecesWidget->setVisible(false);

    ui->toolbox_StackedWidget->setCurrentIndex(0);
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::OpenNewSeamly2D(const QString &fileName) const
{
    if (this->isWindowModified() || qApp->GetPPath().isEmpty() == false)
    {
        VApplication::NewSeamly2D(fileName);
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CreateActions()
{
    ui->setupUi(this);

    //Files menu
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::New);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::Open);
    connect(ui->save_Action, &QAction::triggered, this, &MainWindow::Save);
    connect(ui->saveAs_Action, &QAction::triggered, this, &MainWindow::SaveAs);
    connect(ui->closePattern_Action, &QAction::triggered, this, [this]()
    {
        if (MaybeSave())
        {
            FileClosedCorrect();
            Clear();
        }
    });

    connect(ui->exportAs_Action, &QAction::triggered, this, &MainWindow::exportLayoutAs);
    connect(ui->printPreview_Action, &QAction::triggered, this, &MainWindow::PrintPreviewOrigin);
    connect(ui->printPreviewTiled_Action, &QAction::triggered, this, &MainWindow::PrintPreviewTiled);
    connect(ui->print_Action, &QAction::triggered, this, &MainWindow::PrintOrigin);
    connect(ui->printTiled_Action, &QAction::triggered, this, &MainWindow::PrintTiled);

    connect(ui->appPreferences_Action, &QAction::triggered, this, &MainWindow::Preferences);
    connect(ui->patternPreferences_Action, &QAction::triggered, this, [this]()
    {
        DialogPatternProperties proper(doc, pattern, this);
        connect(&proper, &DialogPatternProperties::UpdateGradation, this, [this]()
        {
            UpdateHeightsList(VMeasurement::ListHeights(doc->GetGradationHeights(), qApp->patternUnit()));
            UpdateSizesList(VMeasurement::ListSizes(doc->GetGradationSizes(), qApp->patternUnit()));
        });
        proper.exec();
    });
    ui->patternPreferences_Action->setEnabled(false);

    //Actions for recent files loaded by a main window application.
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        QAction *action = new QAction(this);
        action->setVisible(false);
        recentFileActs[i] = action;
        connect(recentFileActs[i], &QAction::triggered, this, [this]()
        {
            if (QAction *action = qobject_cast<QAction*>(sender()))
            {
                const QString filePath = action->data().toString();
                if (not filePath.isEmpty())
                {
                    LoadPattern(filePath);
                }
            }
        });
    }

    connect(ui->documentInfo_Action, &QAction::triggered, this, [this]()
    {
        ShowInfoDialog *infoDialog = new ShowInfoDialog(doc, this);
        infoDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        infoDialog->adjustSize();
        infoDialog->show();
    });

    connect(ui->exit_Action, &QAction::triggered, this, &MainWindow::close);

    //Edit Menu
    connect(ui->labelTemplateEditor_Action, &QAction::triggered, this, [this]()
    {
        DialogEditLabel editor(doc);
        editor.exec();
    });

    //View menu
    connect(ui->draftMode_Action, &QAction::triggered, this, &MainWindow::draftMode_Action);
    connect(ui->pieceMode_Action, &QAction::triggered, this, &MainWindow::ActionDetails);
    connect(ui->layoutMode_Action, &QAction::triggered, this, &MainWindow::ActionLayout);

    connect(ui->toggleWireframe_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setWireframe(checked);
        ui->view->itemClicked(nullptr);
        upDateScenes();
    });

    connect(ui->toggleControlPoints_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowControlPoints(checked);
        ui->view->itemClicked(nullptr);
        draftScene->enablePiecesMode(checked);
    });

    connect(ui->toggleAxisOrigin_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowAxisOrigin(checked);
        draftScene->setOriginsVisible(checked);
        pieceScene->setOriginsVisible(checked);
    });

    connect(ui->toggleSeamAllowances_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowSeamAllowances(checked);
        ui->view->itemClicked(nullptr);
        refreshSeamAllowances();
    });

    connect(ui->toggleGrainLines_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowGrainlines(checked);
        ui->view->itemClicked(nullptr);
        refreshGrainLines();
    });

    connect(ui->toggleLabels_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowLabels(checked);
        ui->view->itemClicked(nullptr);
        RefreshDetailsLabel();
    });
/**
    connect(ui->toggleAnchorPoints_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setShowAnchorPoints(checked);
    });
**/
    connect(ui->increaseSize_Action, &QAction::triggered, this, [this]()
    {
        int index = qMin(fontSizeComboBox->currentIndex() + 1, fontSizeComboBox->count()-1);
        fontSizeComboBox->setCurrentIndex(index);
        qApp->Seamly2DSettings()->setPointNameSize(fontSizeComboBox->currentText().toInt());
        upDateScenes();
    });

    connect(ui->decreaseSize_Action, &QAction::triggered, this, [this]()
    {
        const int index = qMax(fontSizeComboBox->currentIndex() - 1, 0);
        fontSizeComboBox->setCurrentIndex(index);
        qApp->Seamly2DSettings()->setPointNameSize(fontSizeComboBox->currentText().toInt());
        upDateScenes();
    });

    connect(ui->showPointNames_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setHidePointNames(checked);
        upDateScenes();
    });

    connect(ui->useToolColor_Action, &QAction::triggered, this, [this](bool checked)
    {
        qApp->Seamly2DSettings()->setUseToolColor(checked);
        upDateScenes();
    });

    //Tools menu
    connect(ui->newDraft_Action, &QAction::triggered, this, [this]()
    {
        qCDebug(vMainWindow, "New Pattern Piece.");
        QString patternPieceName = tr("Pattern Piece %1").arg(comboBoxDraws->count()+1);
        qCDebug(vMainWindow, "Generated Pattern Piece name: %s", qUtf8Printable(patternPieceName));

        qCDebug(vMainWindow, "Pattern Piece count %d", comboBoxDraws->count());
        patternPieceName = PatternPieceName(patternPieceName);
        qCDebug(vMainWindow, "Pattern Piece name: %s", qUtf8Printable(patternPieceName));
        if (patternPieceName.isEmpty())
        {
            qCDebug(vMainWindow, "Pattern Piece name is empty.");
            return;
        }

        AddPP(patternPieceName);
    });

    //Tools->Point submenu actions
    connect(ui->midpoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->midpoint_ToolButton->setChecked(true);
        handleMidpointTool(true);
    });
    connect(ui->pointAtDistanceAngle_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointAtDistanceAngle_ToolButton->setChecked(true);
        handlePointAtDistanceAngleTool(true);
    });
    connect(ui->pointAlongLine_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->alongLine_ToolButton->setChecked(true);
        handleAlongLineTool(true);
    });
    connect(ui->pointAlongPerpendicular_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->normal_ToolButton->setChecked(true);
        handleNormalTool(true);
    });
    connect(ui->bisector_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->bisector_ToolButton->setChecked(true);
        handleBisectorTool(true);
    });
    connect(ui->pointOnShoulder_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->shoulderPoint_ToolButton->setChecked(true);
        handleShoulderPointTool(true);
    });
    connect(ui->pointOfContact_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointOfContact_ToolButton->setChecked(true);
        handlePointOfContactTool(true);
    });
    connect(ui->triangle_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->triangle_ToolButton->setChecked(true);
        handleTriangleTool(true);
    });
    connect(ui->pointIntersectXY_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointIntersectXY_ToolButton->setChecked(true);
        handlePointIntersectXYTool(true);
    });
    connect(ui->perpendicularPoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->height_ToolButton->setChecked(true);
        handleHeightTool(true);
    });
    connect(ui->pointIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->lineIntersectAxis_ToolButton->setChecked(true);
        handleLineIntersectAxisTool(true);
    });

    //Tools->Line submenu actions
    connect(ui->lineTool_Action, &QAction::triggered, this, [this]
    {

        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->line_ToolButton->setChecked(true);
        handleLineTool(true);
    });
    connect(ui->lineIntersect_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->lineIntersect_ToolButton->setChecked(true);
        handleLineIntersectTool(true);
    });

    //Tools->Curve submenu actions
    connect(ui->curve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curve_ToolButton->setChecked(true);
        handleCurveTool(true);
    });
    connect(ui->spline_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->spline_ToolButton->setChecked(true);
        handleSplineTool(true);
    });
    connect(ui->curveWithCPs_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveWithCPs_ToolButton->setChecked(true);
        handleCurveWithControlPointsTool(true);
    });
    connect(ui->splineWithCPs_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->splineWithCPs_ToolButton->setChecked(true);
        handleSplineWithControlPointsTool(true);
    });
    connect(ui->pointAlongCurve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongCurve_ToolButton->setChecked(true);
        handlePointAlongCurveTool(true);
    });
    connect(ui->pointAlongSpline_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongSpline_ToolButton->setChecked(true);
        handlePointAlongSplineTool(true);
    });
    connect(ui->curveIntersectCurve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointOfIntersectionCurves_ToolButton->setChecked(true);
        handleCurveIntersectCurveTool(true);
    });
    connect(ui->splineIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveIntersectAxis_ToolButton->setChecked(true);
        handleCurveIntersectAxisTool(true);
    });

    //Tools->Arc submenu actions
    connect(ui->arcTool_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arc_ToolButton->setChecked(true);
        handleArcTool(true);
    });

    connect(ui->pointAlongArc_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointAlongArc_ToolButton->setChecked(true);
        handlePointAlongArcTool(true);
    });

    connect(ui->arcIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcIntersectAxis_ToolButton->setChecked(true);
        handleArcIntersectAxisTool(true);
    });

    connect(ui->arcIntersectArc_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionArcs_ToolButton->setChecked(true);
        handlePointOfIntersectionArcsTool(true);
    });

    connect(ui->circleIntersect_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionCircles_ToolButton->setChecked(true);
        handlePointOfIntersectionCirclesTool(true);
    });

    connect(ui->circleTangent_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromCircleAndTangent_ToolButton->setChecked(true);
        handlePointFromCircleAndTangentTool(true);
    });

    connect(ui->arcTangent_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromArcAndTangent_ToolButton->setChecked(true);
        handlePointFromArcAndTangentTool(true);
    });

    connect(ui->arcWithLength_Action, &QAction::triggered, this, [this]
    {
         ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
         ui->arcWithLength_ToolButton->setChecked(true);
         handleArcWithLengthTool(true);
     });

    connect(ui->ellipticalArc_Action, &QAction::triggered, this, [this]
    {
         ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
         ui->ellipticalArc_ToolButton->setChecked(true);
         handleEllipticalArcTool(true);
     });

    //Tools->Operations submenu actions
    connect(ui->group_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->group_ToolButton->setChecked(true);
        handleGroupTool(true);
    });

    connect(ui->rotation_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->rotation_ToolButton->setChecked(true);
        handleRotationTool(true);
    });

    connect(ui->mirrorByLine_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByLine_ToolButton->setChecked(true);
        handleMirrorByLineTool(true); });
    connect(ui->mirrorByAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByAxis_ToolButton->setChecked(true);
        handleMirrorByAxisTool(true);
    });

    connect(ui->move_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->move_ToolButton->setChecked(true);
        handleMoveTool(true);
    });

    connect(ui->trueDarts_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->trueDarts_ToolButton->setChecked(true);
        handleTrueDartTool(true);
    });

    connect(ui->exportDraftBlocks_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        exportDraftBlocksAs();
    });

    //Tools->Add Details submenu actions
    connect(ui->union_Action, &QAction::triggered, this, [this]
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        ui->unitePieces_ToolButton->setChecked(true);
        handleUnionDetailsTool(true);
    });

    connect(ui->exportPieces_Action, &QAction::triggered, this, [this]
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        exportPiecesAs();
    });

    //Tools->Piece submenu actions
    connect(ui->addPiece_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->addPatternPiece_ToolButton->setChecked(true);
        handlePatternPieceTool(true);
    });
    connect(ui->anchorPoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->anchorPoint_ToolButton->setChecked(true);
        handleAnchorPointTool(true);
    });
    connect(ui->internalPath_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->internalPath_ToolButton->setChecked(true);
        handleInternalPathTool(true);
    });
    connect(ui->insertNodes_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->insertNodes_ToolButton->setChecked(true);
        handleInsertNodesTool(true);
    });

    //Tools->Layout submenu actions
    connect(ui->newPrintLayout_Action, &QAction::triggered, this, [this]
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        ui->layoutSettings_ToolButton->setChecked(true);
        handleNewLayout(true);
    });

    connect(ui->exportLayout_Action, &QAction::triggered, this, [this]
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        exportLayoutAs();
    });

    connect(ui->lastTool_Action, &QAction::triggered, this, &MainWindow::LastUsedTool);

    //Measurements menu
    connect(ui->openSeamlyMe_Action, &QAction::triggered, this, [this]()
    {
        const QString seamlyme = qApp->SeamlyMeFilePath();
        const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();

        QStringList arguments;
        if (isNoScaling)
        {
            arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        QProcess::startDetached(seamlyme, arguments, workingDirectory);
    });

    connect(ui->editCurrent_Action, &QAction::triggered, this, &MainWindow::ShowMeasurements);
    connect(ui->unloadMeasurements_Action, &QAction::triggered, this, &MainWindow::UnloadMeasurements);
    connect(ui->loadIndividual_Action, &QAction::triggered, this, &MainWindow::LoadIndividual);
    connect(ui->loadMultisize_Action, &QAction::triggered, this, &MainWindow::LoadMultisize);
    connect(ui->syncMeasurements_Action, &QAction::triggered, this, &MainWindow::SyncMeasurements);
    connect(ui->table_Action, &QAction::triggered, this, [this](bool checked)
    {
        if (checked)
        {
            dialogTable = new DialogVariables(pattern, doc, this);
            connect(dialogTable.data(), &DialogVariables::updateProperties, toolProperties,
                    &VToolOptionsPropertyBrowser::RefreshOptions);
            connect(dialogTable.data(), &DialogVariables::DialogClosed, this, [this]()
            {
                ui->table_Action->setChecked(false);
                if (dialogTable != nullptr)
                {
                    delete dialogTable;
                }
            });
            dialogTable->show();
        }
        else
        {
            ui->table_Action->setChecked(true);
            dialogTable->activateWindow();
        }
    });
    connect(ui->exportVariablesToCSV_Action, &QAction::triggered, this, &MainWindow::ExportToCSV);

    //History menu
    connect(ui->history_Action, &QAction::triggered, this, [this](bool checked)
    {
        if (checked)
        {
            dialogHistory = new DialogHistory(pattern, doc, this);
            dialogHistory->setWindowFlags(Qt::Window);
            connect(this, &MainWindow::RefreshHistory, dialogHistory.data(), &DialogHistory::updateHistory);
            connect(dialogHistory.data(), &DialogHistory::DialogClosed, this, [this]()
            {
                ui->history_Action->setChecked(false);
                if (dialogHistory != nullptr)
                {
                    delete dialogHistory;
                }
            });
            // Fix issue #526. Dialog Detail is not on top after selection second object on Mac.
            dialogHistory->setWindowFlags(dialogHistory->windowFlags() | Qt::WindowStaysOnTopHint);
            dialogHistory->show();
        }
        else
        {
            ui->history_Action->setChecked(true);
            dialogHistory->activateWindow();
        }
    });

    //Utilities menu
    connect(ui->calculator_Action, &QAction::triggered, this, [this]()
    {
        CalculatorDialog *calcDialog = new CalculatorDialog(this);
        calcDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        calcDialog->setWindowTitle(tr("Calculator"));
        calcDialog->adjustSize();
        calcDialog->show();
    });

    connect(ui->decimalChart_Action, &QAction::triggered, this, [this]()
    {
        DecimalChartDialog *decimalchartDialog = new DecimalChartDialog(this);
        decimalchartDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        decimalchartDialog->show();
    });

    //Help menu
    connect(ui->shortcuts_Action, &QAction::triggered, this, [this]()
    {
        ShortcutsDialog *shortcutsDialog = new ShortcutsDialog(this);
        shortcutsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        shortcutsDialog->show();
    });
    connect(ui->wiki_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Showing online help");
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://wiki.seamly.net/wiki/Main_Page")));
    });

    connect(ui->forum_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Opening forum");
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://forum.seamly.net/")));
    });

    connect(ui->reportBug_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Reporting bug");
        QDesktopServices::openUrl(QUrl(QStringLiteral(
            "https://github.com/FashionFreedom/Seamly2D/issues/new?&labels=bug&template=bug_report.md&title=BUG%3A")));
    });

    connect(ui->aboutQt_Action, &QAction::triggered, this, [this]()
    {
        QMessageBox::aboutQt(this, tr("About Qt"));
    });

    connect(ui->aboutSeamly2D_Action, &QAction::triggered, this, [this]()
    {
        DialogAboutApp *aboutDialog = new DialogAboutApp(this);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        aboutDialog->show();
    });

    //Toolbox toolbar
    connect(ui->arrow_Action,         &QAction::triggered, this, &MainWindow::handleArrowTool);
    connect(ui->points_Action,        &QAction::triggered, this, &MainWindow::handlePointsMenu);
    connect(ui->lines_Action,         &QAction::triggered, this, &MainWindow::handleLinesMenu);
    connect(ui->arcs_Action,          &QAction::triggered, this, &MainWindow::handleArcsMenu);
    connect(ui->curves_Action,        &QAction::triggered, this, &MainWindow::handleCurvesMenu);
    connect(ui->modifications_Action, &QAction::triggered, this, &MainWindow::handleOperationsMenu);
    connect(ui->details_Action,       &QAction::triggered, this, &MainWindow::handleDetailsMenu);
    connect(ui->pieces_Action,        &QAction::triggered, this, &MainWindow::handlePieceMenu);
    connect(ui->layout_Action,        &QAction::triggered, this, &MainWindow::handleLayoutMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitAutoSave()
{
    //Autosaving file each 1 minutes
    delete autoSaveTimer;
    autoSaveTimer = nullptr;

    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::AutoSavePattern);
    autoSaveTimer->stop();

    if (qApp->Seamly2DSettings()->GetAutosaveState())
    {
        const qint32 autoTime = qApp->Seamly2DSettings()->getAutosaveInterval();
        autoSaveTimer->start(autoTime*60000);
        qCDebug(vMainWindow, "Autosaving every %d minutes.", autoTime);
    }
    qApp->setAutoSaveTimer(autoSaveTimer);
}

//---------------------------------------------------------------------------------------------------------------------
QString MainWindow::PatternPieceName(const QString &text)
{
    QInputDialog *dialog = new QInputDialog(this);
    dialog->setInputMode( QInputDialog::TextInput );
    dialog->setLabelText(tr("Name:"));
    dialog->setTextEchoMode(QLineEdit::Normal);
    dialog->setWindowTitle(tr("Draft block."));
    dialog->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint
                                         & ~Qt::WindowMaximizeButtonHint
                                         & ~Qt::WindowMinimizeButtonHint);
    dialog->resize(300, 100);
    dialog->setTextValue(text);
    QString draftBlockName;
    while (1)
    {
        const bool result = dialog->exec();
        draftBlockName = dialog->textValue();
        if (result == false || draftBlockName.isEmpty())
        {
            delete dialog;
            return QString();
        }
        if (comboBoxDraws->findText(draftBlockName) == -1)
        {
            break; //exit dialog
        }
        //repeat show dialog
        QMessageBox messageBox;
        messageBox.setWindowTitle(tr("Name Exists"));
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Retry);
        messageBox.setText(tr("The action can't be completed because the Draft Block name already exists."));
        int boxResult = messageBox.exec();

        switch (boxResult)
        {
            case QMessageBox::Retry:
                break;    // Repeat Dialog
            case QMessageBox::Cancel:
                return QString();  // Exit Dialog
            default:
                break;   // should never be reached
        }
    }
    delete dialog;
    return draftBlockName;
}

//---------------------------------------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    CancelTool();
    CleanLayout();

    delete doc;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LoadPattern open pattern file.
 * @param fileName name of file.
 */
bool MainWindow::LoadPattern(const QString &fileName, const QString& customMeasureFile)
{
    qCDebug(vMainWindow, "Loading new file %s.", qUtf8Printable(fileName));

    //We have unsaved changes or load more then one file per time
    if (OpenNewSeamly2D(fileName))
    {
        return false;
    }

    if (fileName.isEmpty())
    {
        qCDebug(vMainWindow, "New loaded filename is empty.");
        Clear();
        return false;
    }

    try
    {
        // Here comes undocumented Seamly2D's feature.
        // Because app bundle in Mac OS X doesn't allow setup association for SeamlyMe we must do this through Seamly2D
        VMeasurements measurements(pattern);
        measurements.SetSize(VContainer::rsize());
        measurements.SetHeight(VContainer::rheight());
        measurements.setXMLContent(fileName);

        if (measurements.Type() == MeasurementsType::Multisize || measurements.Type() == MeasurementsType::Individual)
        {
            const QString seamlyme = qApp->SeamlyMeFilePath();
            const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();

            QStringList arguments = QStringList() << fileName;
            if (isNoScaling)
            {
                arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
            }

            QProcess::startDetached(seamlyme, arguments, workingDirectory);
            qApp->exit(V_EX_OK);
            return false; // stop continue processing
        }
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        Clear();
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    qCDebug(vMainWindow, "Locking file");
    VlpCreateLock(lock, fileName);

    if (lock->IsLocked())
    {
        qCDebug(vMainWindow, "Pattern file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        if (not IgnoreLocking(lock->GetLockError(), fileName))
        {
            return false;
        }
    }

    // On this stage scene empty. Fit scene size to view size
    VMainGraphicsView::NewSceneRect(draftScene, ui->view);
    VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

    qApp->setOpeningPattern();//Begin opening file
    try
    {
        VPatternConverter converter(fileName);
        m_curFileFormatVersion = converter.GetCurrentFormatVarsion();
        m_curFileFormatVersionStr = converter.GetVersionStr();
        doc->setXMLContent(converter.Convert());
        if (!customMeasureFile.isEmpty())
        {
            doc->SetMPath(RelativeMPath(fileName, customMeasureFile));
        }
        qApp->setPatternUnit(doc->MUnit());
        const QString path = AbsoluteMPath(fileName, doc->MPath());

        if (not path.isEmpty())
        {
            // Check if exist
            const QString newPath = CheckPathToMeasurements(fileName, path);
            if (newPath.isEmpty())
            {
                qApp->setOpeningPattern();// End opening file
                Clear();
                qCCritical(vMainWindow, "%s", qUtf8Printable(tr("The measurements file '%1' could not be found.")
                                                             .arg(path)));
                if (not VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_NOINPUT);
                }
                return false;
            }

            if (not LoadMeasurements(newPath))
            {
                qCCritical(vMainWindow, "%s", qUtf8Printable(tr("The measurements file '%1' could not be found.")
                                                             .arg(newPath)));
                qApp->setOpeningPattern();// End opening file
                Clear();
                if (not VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_NOINPUT);
                }
                return false;
            }
            else
            {
                ui->unloadMeasurements_Action->setEnabled(true);
                watcher->addPath(path);
                ui->editCurrent_Action->setEnabled(true);
            }
        }

        if (qApp->patternType() == MeasurementsType::Unknown)
        {// Show toolbar only if was not uploaded any measurements.
            initStatusBar();
        }
    }
    catch (VException &e)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File error.")),
                   qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        qApp->setOpeningPattern();// End opening file
        Clear();
        if (not VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    FullParseFile();

    if (guiEnabled)
    { // No errors occurred
        patternReadOnly = doc->IsReadOnly();
        SetEnableWidgets(true);
        setCurrentFile(fileName);
        helpLabel->setText(tr("File loaded"));
        qCDebug(vMainWindow, "File loaded.");

        //Fit scene size to best size for first show
        zoomFirstShow();

        draftMode_Action(true);

        qApp->setOpeningPattern();// End opening file
        return true;
    }
    else
    {
        qApp->setOpeningPattern();// End opening file
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MainWindow::GetUnlokedRestoreFileList() const
{
    QStringList restoreFiles;
    //Take all files that need to be restored
    QStringList files = qApp->Seamly2DSettings()->GetRestoreFileList();
    if (files.size() > 0)
    {
        for (int i = 0; i < files.size(); ++i)
        {
            // Seeking file that really needs reopen
            VLockGuard<char> tmp(files.at(i));
            if (tmp.IsLocked())
            {
                restoreFiles.append(files.at(i));
            }
        }

        // Clearing list after filtering
        for (int i = 0; i < restoreFiles.size(); ++i)
        {
            files.removeAll(restoreFiles.at(i));
        }

        // Clear all files that do not exist.
        QStringList filtered;
        for (int i = 0; i < files.size(); ++i)
        {
            if (QFileInfo(files.at(i)).exists())
            {
                filtered.append(files.at(i));
            }
        }

        qApp->Seamly2DSettings()->SetRestoreFileList(filtered);
    }
    return restoreFiles;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolBarStyles()
{
    ToolBarStyle(ui->draft_ToolBar);
    ToolBarStyle(ui->mode_ToolBar);
    ToolBarStyle(ui->edit_Toolbar);
    ToolBarStyle(ui->zoom_ToolBar);
    ToolBarStyle(ui->file_ToolBar);

    fontComboBox->setCurrentFont(qApp->Seamly2DSettings()->getPointNameFont());
    int index = fontSizeComboBox->findData(qApp->Seamly2DSettings()->getPointNameSize());
    fontSizeComboBox->setCurrentIndex(index);
}

void MainWindow::resetOrigins()
{
    draftScene->InitOrigins();
    draftScene->setOriginsVisible(true);
    pieceScene->InitOrigins();
    pieceScene->setOriginsVisible(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::showLayoutPages(int index)
{
    if (index < 0 || index >= scenes.size())
    {
        ui->view->setScene(tempSceneLayout);
    }
    else
    {
        ui->view->setScene(scenes.at(index));
    }

    ui->view->fitInView(ui->view->scene()->sceneRect(), Qt::KeepAspectRatio);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::Preferences()
{
    // Calling constructor of the dialog take some time. Because of this user have time to call the dialog twice.
    static QPointer<DialogPreferences> guard;// Prevent any second run
    if (guard.isNull())
    {
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        DialogPreferences *preferences = new DialogPreferences(this);
        // QScopedPointer needs to be sure any exception will never block guard
        QScopedPointer<DialogPreferences> dialog(preferences);
        guard = preferences;
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::WindowsLocale); // Must be first
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::ToolBarStyles);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::updateToolBarVisibility);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::RefreshDetailsLabel);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::resetOrigins);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::upDateScenes);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::updateViewToolbar);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::resetPanShortcuts);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, [this](){emit doc->FullUpdateFromFile();});
        connect(dialog.data(), &DialogPreferences::updateProperties,
                toolProperties, &VToolOptionsPropertyBrowser::RefreshOptions);

        connect(dialog.data(), &DialogPreferences::updateProperties, ui->view, &VMainGraphicsView::resetScrollBars);
        connect(dialog.data(), &DialogPreferences::updateProperties, ui->view, &VMainGraphicsView::resetScrollAnimations);


        QGuiApplication::restoreOverrideCursor();

        if (guard->exec() == QDialog::Accepted)
        {
            InitAutoSave();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void MainWindow::CreateMeasurements()
{
    const QString seamlyme = qApp->SeamlyMeFilePath();
    const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();

    QStringList arguments;
    if (isNoScaling)
    {
        arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
    }

    QProcess::startDetached(seamlyme, arguments, workingDirectory);
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::exportLayoutAs()
{
    ui->exportLayout_ToolButton->setChecked(true);

    if (isLayoutStale)
    {
        if (ContinueIfLayoutStale() == QMessageBox::No)
        {
            ui->exportLayout_ToolButton->setChecked(false);
            return;
        }
    }

    try
    {
        ExportLayoutDialog dialog(scenes.size(), Draw::Layout, FileName(), this);

        if (dialog.exec() == QDialog::Rejected)
        {
            ui->exportLayout_ToolButton->setChecked(false);
            return;
        }

        ExportData(QVector<VLayoutPiece>(), dialog);
    }
    catch (const VException &e)
    {
        ui->exportLayout_ToolButton->setChecked(false);
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export error.")),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }
    ui->exportLayout_ToolButton->setChecked(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::exportPiecesAs()
{
    ui->arrowPointer_ToolButton->setChecked(false);
    ui->arrow_Action->setChecked(false);
    ui->exportPiecesAs_ToolButton->setChecked(true);

    const QHash<quint32, VPiece> *allDetails = pattern->DataPieces();
    QHash<quint32, VPiece>::const_iterator i = allDetails->constBegin();
    QHash<quint32, VPiece> detailsInLayout;
    while (i != allDetails->constEnd())
    {
        if (i.value().IsInLayout())
        {
            detailsInLayout.insert(i.key(), i.value());
        }
        ++i;
    }

    if (detailsInLayout.count() == 0)
    {
        QMessageBox::information(this, tr("Layout mode"),  tr("You don't have any pieces to export. Please, "
                                                              "include at least one piece in layout."),
                                 QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QVector<VLayoutPiece> listDetails;
    try
    {
        listDetails = PrepareDetailsForLayout(detailsInLayout);
    }
    catch (VException &e)
    {
        QMessageBox::warning(this, tr("Export pieces"),
                             tr("Can't export pieces.") + QLatin1String(" \n") + e.ErrorMessage(),
                             QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        ExportLayoutDialog dialog(1, Draw::Modeling, FileName(), this);
        dialog.setWindowTitle("Export Pattern Pieces");

        if (dialog.exec() == QDialog::Rejected)
        {
            ui->exportPiecesAs_ToolButton->setChecked(false);
            return;
        }

        ExportData(listDetails, dialog);
    }
    catch (const VException &e)
    {
        ui->exportPiecesAs_ToolButton->setChecked(false);
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export error.")),
                  qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        return;
    }

    ui->arrowPointer_ToolButton->setChecked(true);
    ui->arrow_Action->setChecked(true);
    ui->exportPiecesAs_ToolButton->setChecked(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::exportDraftBlocksAs()
{
    //select export tool button
    ui->arrowPointer_ToolButton->setChecked(false);
    ui->arrow_Action->setChecked(false);
    ui->exportDraftBlocks_ToolButton->setChecked(true);

    //Get view info so we can restore after export
    int vScrollBar = ui->view->verticalScrollBar()->value();
    int hScrollBar = ui->view->horizontalScrollBar()->value();
    QTransform viewTransform = ui->view->transform();

    //Include all items in draft scene
    ui->view->zoomToFit();
    ui->view->repaint();
    ui->view->zoom100Percent();

    // Enable all draft blocks in the scene
    const QList<QGraphicsItem *> items = draftScene->items();
    for (auto *item : items)
    {
        item->setEnabled(true);
    }
    ui->view->repaint();

    draftScene->setOriginsVisible(false);

    //Open a file dialog to save export
    ExportLayoutDialog dialog(1, Draw::Calculation, FileName(), this);
    dialog.setWindowTitle("Export Draft Blocks");

    if (dialog.exec() == QDialog::Accepted)
    {
        const QString filename = QString("%1/%2%3")
        .arg(dialog.path())                                          //1
        .arg(dialog.fileName())                                      //2
        .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format())); //3

        QRectF rect;
        rect = draftScene->itemsBoundingRect();
        draftScene->update(rect);
        QGraphicsRectItem *paper = new QGraphicsRectItem(rect);
        QMarginsF margins = QMarginsF(0, 0, 0, 0);

        switch(dialog.format())
        {
            case LayoutExportFormat::SVG:
                {
                    exportSVG(filename, paper, draftScene);
                    break;
                }
            case LayoutExportFormat::PNG:
                {
                    exportPNG(filename, draftScene);
                    break;
                }
            case LayoutExportFormat::JPG:
                {
                    exportJPG(filename, draftScene);
                    break;
                }
            case LayoutExportFormat::BMP:
                {
                    exportBMP(filename, draftScene);
                    break;
                }
            case LayoutExportFormat::TIF:
                {
                    exportTIF(filename, draftScene);
                    break;
                }
            case LayoutExportFormat::PPM:
                {
                    exportPPM(filename, draftScene);
                    break;
                }
            case LayoutExportFormat::PDF:
                {
                    exportPDF(filename, paper, draftScene, true, margins);
                    break;
                }
            case LayoutExportFormat::PDFTiled:
            case LayoutExportFormat::OBJ:
            case LayoutExportFormat::PS:
                {
                    exportPS(filename, paper, draftScene, true, margins);
                    break;
                }
            case LayoutExportFormat::EPS:
                {
                    exportEPS(filename, paper, draftScene, true, margins);
                    break;
                }
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
            default:
                break;
        }
    }

    // Disable draft blocks in the scenee except current block
    doc->changeActiveDraftBlock(doc->getActiveDraftBlockName(), Document::FullParse);

    draftScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

    // Restore scale, scrollbars, current active draft block
    ui->view->setTransform(viewTransform);
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view);
    zoomScaleChanged(ui->view->transform().m11());

    ui->view->verticalScrollBar()->setValue(vScrollBar);
    ui->view->horizontalScrollBar()->setValue(hScrollBar);

    //reset tool buttons
    ui->arrowPointer_ToolButton->setChecked(true);
    ui->arrow_Action->setChecked(true);
    ui->exportDraftBlocks_ToolButton->setChecked(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ReopenFilesAfterCrash(QStringList &args)
{
    const QStringList files = GetUnlokedRestoreFileList();
    if (files.size() > 0)
    {
        qCDebug(vMainWindow, "Reopen files after crash.");

        QStringList restoreFiles;
        for (int i = 0; i < files.size(); ++i)
        {
            QFile file(files.at(i) + autosavePrefix);
            if (file.exists())
            {
                restoreFiles.append(files.at(i));
            }
        }

        if (restoreFiles.size() > 0)
        {
            QMessageBox::StandardButton reply;
            const QString mes = tr("Seamly2D didn't shut down correctly. Do you want reopen files (%1) you had open?")
                    .arg(restoreFiles.size());
            reply = QMessageBox::question(this, tr("Reopen files."), mes, QMessageBox::Yes|QMessageBox::No,
                                          QMessageBox::Yes);
            if (reply == QMessageBox::Yes)
            {
                qCDebug(vMainWindow, "User said Yes.");

                for (int i = 0; i < restoreFiles.size(); ++i)
                {
                    QString error;
                    if (VDomDocument::SafeCopy(restoreFiles.at(i) + autosavePrefix, restoreFiles.at(i), error))
                    {
                        QFile autoFile(restoreFiles.at(i) + autosavePrefix);
                        autoFile.remove();
                        LoadPattern(restoreFiles.at(i));
                        args.removeAll(restoreFiles.at(i));// Do not open file twice after we restore him.
                    }
                    else
                    {
                        qCDebug(vMainWindow, "Could not copy %s%s to %s %s",
                                qUtf8Printable(restoreFiles.at(i)), qUtf8Printable(autosavePrefix),
                                qUtf8Printable(restoreFiles.at(i)), qUtf8Printable(error));
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MainWindow::CheckPathToMeasurements(const QString &patternPath, const QString &path)
{
    if (path.isEmpty())
    {
        return path;
    }

    QFileInfo table(path);
    if (table.exists() == false)
    {
        if (!VApplication::IsGUIMode())
        {
            return QString();// console mode doesn't support fixing path to a measurement file
        }
        else
        {
            const QString text = tr("The measurements file <br/><br/> <b>%1</b> <br/><br/> could not be found. Do you "
                                    "want to update the file location?").arg(path);
            QMessageBox::StandardButton res = QMessageBox::question(this, tr("Loading measurements file"), text,
                                                                    QMessageBox::Yes | QMessageBox::No,
                                                                    QMessageBox::Yes);
            if (res == QMessageBox::No)
            {
                return QString();
            }
            else
            {
                MeasurementsType patternType;
                if (table.suffix() == QLatin1String("vst"))
                {
                    patternType = MeasurementsType::Multisize;
                }
                else if (table.suffix() == QLatin1String("vit"))
                {
                    patternType = MeasurementsType::Individual;
                }
                else
                {
                    patternType = MeasurementsType::Unknown;
                }

                QString mPath;
                if (patternType == MeasurementsType::Multisize)
                {
                    const QString filter = tr("Multisize measurements") + QLatin1String(" (*.vst)");
                    //Use standard path to multisize measurements
                    QString path = qApp->Seamly2DSettings()->GetPathMultisizeMeasurements();
                    path = VCommonSettings::PrepareMultisizeTables(path);
                    mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                         QFileDialog::DontUseNativeDialog);
                }
                else if (patternType == MeasurementsType::Individual)
                {
                    const QString filter = tr("Individual measurements") + QLatin1String(" (*.vit)");
                    //Use standard path to individual measurements
                    const QString path = qApp->Seamly2DSettings()->GetPathIndividualMeasurements();

                    bool usedNotExistedDir = false;
                    QDir directory(path);
                    if (not directory.exists())
                    {
                        usedNotExistedDir = directory.mkpath(".");
                    }

                    mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                         QFileDialog::DontUseNativeDialog);

                    if (usedNotExistedDir)
                    {
                        QDir directory(path);
                        directory.rmpath(".");
                    }
                }
                else
                {
                    const QString filter = tr("Individual measurements") + QLatin1String(" (*.vit);;") +
                                           tr("Multisize measurements") + QLatin1String(" (*.vst)");
                    //Use standard path to individual measurements
                    const QString path = qApp->Seamly2DSettings()->GetPathIndividualMeasurements();
                    VCommonSettings::PrepareMultisizeTables(VCommonSettings::GetDefPathMultisizeMeasurements());

                    bool usedNotExistedDir = false;
                    QDir directory(path);
                    if (not directory.exists())
                    {
                        usedNotExistedDir = directory.mkpath(".");
                    }

                    mPath = QFileDialog::getOpenFileName(this, tr("Open file"), path, filter, nullptr,
                                                         QFileDialog::DontUseNativeDialog);

                    if (usedNotExistedDir)
                    {
                        QDir directory(path);
                        directory.rmpath(".");
                    }
                }

                if (mPath.isEmpty())
                {
                    return mPath;
                }
                else
                {
                    QScopedPointer<VMeasurements> measurements(new VMeasurements(pattern));
                    measurements->SetSize(VContainer::rsize());
                    measurements->SetHeight(VContainer::rheight());
                    measurements->setXMLContent(mPath);

                    patternType = measurements->Type();

                    if (patternType == MeasurementsType::Unknown)
                    {
                        VException e(tr("Measurement file has unknown format."));
                        throw e;
                    }

                    if (patternType == MeasurementsType::Multisize)
                    {
                        VVSTConverter converter(mPath);
                        measurements->setXMLContent(converter.Convert());// Read again after conversion
                    }
                    else
                    {
                        VVITConverter converter(mPath);
                        measurements->setXMLContent(converter.Convert());// Read again after conversion
                    }

                    if (not measurements->IsDefinedKnownNamesValid())
                    {
                        VException e(tr("Measurement file contains invalid known measurement(s)."));
                        throw e;
                    }

                    CheckRequiredMeasurements(measurements.data());

                    qApp->setPatternType(patternType);
                    doc->SetMPath(RelativeMPath(patternPath, mPath));
                    PatternChangesWereSaved(false);
                    return mPath;
                }
            }
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::changeDraftBlock(int index, bool zoomBestFit)
{
    if (index != -1)
    {
        doc->changeActiveDraftBlock(comboBoxDraws->itemText(index));
        doc->setCurrentData();
        emit RefreshHistory();
        if (drawMode)
        {
            handleArrowTool(true);
            if (zoomBestFit)
            {
                zoomToSelected();
            }
        }
        toolProperties->itemClicked(nullptr);//hide options for tool in previous pattern piece
        groupsWidget->UpdateGroups();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::EndVisualization(bool click)
{
    if (not dialogTool.isNull())
    {
        dialogTool->ShowDialog(click);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief zoomFirstShow Fit scene size to best size for first show
 */
void MainWindow::zoomFirstShow()
{
    /* If don't call zoomToFit() twice, after first scaling or moving pattern piece, scene change coordinate and whole
     * pattern will be moved. Looks very ugly. It is best solution that i have now.
     */
    if (pattern->DataPieces()->size() > 0)
    {
        ActionDetails(true);
        ui->view->zoomToFit();
    }
    if (not ui->draftMode_Action->isChecked())
    {
        draftMode_Action(true);
    }
    zoomToSelected();

    VMainGraphicsView::NewSceneRect(draftScene, ui->view);
    VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

    if (pattern->DataPieces()->size() > 0)
    {
        ActionDetails(true);
        ui->view->zoomToFit();
    }

    if (not ui->draftMode_Action->isChecked())
    {
        draftMode_Action(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::DoExport(const VCommandLinePtr &expParams)
{
    const QHash<quint32, VPiece> *details = pattern->DataPieces();
    if(not qApp->getOpeningPattern())
    {
        if (details->count() == 0)
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("You can't export empty scene.")));
            qApp->exit(V_EX_DATAERR);
            return;
        }
    }
    listDetails = PrepareDetailsForLayout(*details);

    const bool exportOnlyDetails = expParams->IsExportOnlyDetails();
    if (exportOnlyDetails)
    {
        try
        {
            ExportLayoutDialog dialog(1, Draw::Modeling, expParams->OptBaseName(), this);
            dialog.setDestinationPath(expParams->OptDestinationPath());
            dialog.selectFormat(static_cast<LayoutExportFormat>(expParams->OptExportType()));
            dialog.setBinaryDXFFormat(expParams->IsBinaryDXF());
            dialog.setTextAsPaths(expParams->isTextAsPaths());

            ExportData(listDetails, dialog);
        }
        catch (const VException &e)
        {
            qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export error.")), qUtf8Printable(e.ErrorMessage()));
            qApp->exit(V_EX_DATAERR);
            return;
        }
    }
    else
    {
        auto settings = expParams->DefaultGenerator();
        settings->SetTestAsPaths(expParams->isTextAsPaths());

        if (LayoutSettings(*settings.get()))
        {
            try
            {
                ExportLayoutDialog dialog(scenes.size(), Draw::Layout, expParams->OptBaseName(), this);
                dialog.setDestinationPath(expParams->OptDestinationPath());
                dialog.selectFormat(static_cast<LayoutExportFormat>(expParams->OptExportType()));
                dialog.setBinaryDXFFormat(expParams->IsBinaryDXF());

                ExportData(listDetails, dialog);
            }
            catch (const VException &e)
            {
                qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export error.")), qUtf8Printable(e.ErrorMessage()));
                qApp->exit(V_EX_DATAERR);
                return;
            }
        }
        else
        {
            return;
        }
    }

    qApp->exit(V_EX_OK);
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::SetSize(const QString &text)
{
    if (not VApplication::IsGUIMode())
    {
        if (this->isWindowModified() || not qApp->GetPPath().isEmpty())
        {
            if (qApp->patternType() == MeasurementsType::Multisize)
            {
                const int size = static_cast<int>(UnitConvertor(text.toInt(), Unit::Cm, *pattern->GetPatternUnit()));
                const qint32 index = gradationSizes->findText(QString().setNum(size));
                if (index != -1)
                {
                    gradationSizes->setCurrentIndex(index);
                }
                else
                {
                    qCCritical(vMainWindow, "%s",
                              qUtf8Printable(tr("Not supported size value '%1' for this pattern file.").arg(text)));
                    return false;
                }
            }
            else
            {
                qCCritical(vMainWindow, "%s",
                          qUtf8Printable(tr("Couldn't set size. Need a file with multisize measurements.")));
                return false;
            }
        }
        else
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set size. File wasn't opened.")));
            return false;
        }
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::SetHeight(const QString &text)
{
    if (not VApplication::IsGUIMode())
    {
        if (this->isWindowModified() || not qApp->GetPPath().isEmpty())
        {
            if (qApp->patternType() == MeasurementsType::Multisize)
            {
                const int height = static_cast<int>(UnitConvertor(text.toInt(), Unit::Cm, *pattern->GetPatternUnit()));
                const qint32 index = gradationHeights->findText(QString().setNum(height));
                if (index != -1)
                {
                    gradationHeights->setCurrentIndex(index);
                }
                else
                {
                    qCCritical(vMainWindow, "%s",
                              qUtf8Printable(tr("Not supported height value '%1' for this pattern file.").arg(text)));
                    return false;
                }
            }
            else
            {
                qCCritical(vMainWindow, "%s",
                          qUtf8Printable(tr("Couldn't set height. Need a file with multisize measurements.")));
                return false;
            }
        }
        else
        {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set height. File wasn't opened.")));
            return false;
        }
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ProcessCMD()
{
    const VCommandLinePtr cmd = qApp->CommandLine();
    auto args = cmd->OptInputFileNames();

    isNoScaling = cmd->IsNoScalingEnabled();

    if (VApplication::IsGUIMode())
    {
        ReopenFilesAfterCrash(args);
    }
    else
    {
        if (args.size() != 1)
        {
            qCritical() << tr("Please, provide one input file.");
            qApp->exit(V_EX_NOINPUT);
            return;
        }
    }

    for (int i=0, sz = args.size(); i < sz; ++i)
    {
        const bool loaded = LoadPattern(args.at(static_cast<int>(i)), cmd->OptMeasurePath());

        if (not loaded && not VApplication::IsGUIMode())
        {
            return; // process only one input file
        }

        bool hSetted = true;
        bool sSetted = true;
        if (loaded && (cmd->IsTestModeEnabled() || cmd->IsExportEnabled()))
        {
            if (cmd->IsSetGradationSize())
            {
                sSetted = SetSize(cmd->OptGradationSize());
            }

            if (cmd->IsSetGradationHeight())
            {
                hSetted = SetHeight(cmd->OptGradationHeight());
            }
        }

        if (not cmd->IsTestModeEnabled())
        {
            if (cmd->IsExportEnabled())
            {
                if (loaded && hSetted && sSetted)
                {
                    DoExport(cmd);
                    return; // process only one input file
                }
                else
                {
                    qApp->exit(V_EX_DATAERR);
                    return;
                }
                break;
            }
        }
    }

    if (not VApplication::IsGUIMode())
    {
        qApp->exit(V_EX_OK);// close program after processing in console mode
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString MainWindow::GetPatternFileName()
{
    QString shownName = tr("untitled.val");
    if(not qApp->GetPPath().isEmpty())
    {
        shownName = StrippedName(qApp->GetPPath());
    }
    shownName += QLatin1String("[*]");
    return shownName;
}

//---------------------------------------------------------------------------------------------------------------------
QString MainWindow::GetMeasurementFileName()
{
    if(doc->MPath().isEmpty())
    {
        return "";
    }
    else
    {
        QString shownName(" [");
        shownName += StrippedName(AbsoluteMPath(qApp->GetPPath(), doc->MPath()));

        if(mChanges)
        {
            shownName += QLatin1String("*");
        }

        shownName += QLatin1String("]");
        return shownName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateWindowTitle()
{
    bool isFileWritable = true;
    if (not qApp->GetPPath().isEmpty())
    {
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
        isFileWritable = QFileInfo(qApp->GetPPath()).isWritable();
#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/
    }

    if (not patternReadOnly && isFileWritable)
    {
        setWindowTitle(GetPatternFileName()+GetMeasurementFileName() + QString(" - ") + VER_INTERNALNAME_STR);
    }
    else
    {
        setWindowTitle(GetPatternFileName()+GetMeasurementFileName() +QLatin1String(" (") +
                       tr("read only") + QLatin1String(")") + QString(" - ") + VER_INTERNALNAME_STR);
    }
    setWindowFilePath(qApp->GetPPath());

#if defined(Q_OS_MAC)
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() +
                                  QLatin1String("/../Resources/Seamly2D.icns"));
    QIcon icon;
    if (not qApp->GetPPath().isEmpty())
    {
        if (not isWindowModified())
        {
            icon = fileIcon;
        }
        else
        {
            static QIcon darkIcon;

            if (darkIcon.isNull())
            {
                darkIcon = QIcon(darkenPixmap(fileIcon.pixmap(16, 16)));
            }
            icon = darkIcon;
        }
    }
    setWindowIcon(icon);
#endif //defined(Q_OS_MAC)
}

void MainWindow::upDateScenes()
{
    if (draftScene)
    {
        draftScene->update();
    }

    if (pieceScene)
    {
        pieceScene->update();
    }
}

void MainWindow::updateViewToolbar()
{
    ui->toggleWireframe_Action->setChecked(qApp->Settings()->isWireframe());
    ui->toggleControlPoints_Action->setChecked(qApp->Settings()->getShowControlPoints());
    ui->toggleAxisOrigin_Action->setChecked(qApp->Settings()->getShowAxisOrigin());
    ui->toggleGrainLines_Action->setChecked(qApp->Settings()->showGrainlines());
    ui->toggleSeamAllowances_Action->setChecked(qApp->Settings()->showSeamAllowances());
    ui->toggleLabels_Action->setChecked(qApp->Settings()->showLabels());
}

void MainWindow::resetPanShortcuts()
{
    QList<QKeySequence> zoomPanShortcuts;
    zoomPanShortcuts = ui->zoomPan_Action->shortcuts();
    zoomPanShortcuts.removeAll(QKeySequence(Qt::Key_Space));
    if (!qApp->Seamly2DSettings()->isPanActiveSpaceKey())
    {
        zoomPanShortcuts.append(QKeySequence(Qt::Key_Space));
    }
    ui->zoomPan_Action->setShortcuts(zoomPanShortcuts);
}

//---------------------------------------------------------------------------------------------------------------------
bool MainWindow::IgnoreLocking(int error, const QString &path)
{
    QMessageBox::StandardButton answer = QMessageBox::Abort;
    if (VApplication::IsGUIMode())
    {
        switch(error)
        {
            case QLockFile::LockFailedError:
                answer = QMessageBox::warning(this, tr("Locking file"),
                                               tr("This file already opened in another window. Ignore if you want "
                                                  "to continue (not recommended, can cause a data corruption)."),
                                               QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::PermissionError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("The lock file could not be created, for lack of permissions. "
                                                  "Ignore if you want to continue (not recommended, can cause "
                                                  "a data corruption)."),
                                               QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::UnknownError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                               tr("Unknown error happened, for instance a full partition prevented "
                                                  "writing out the lock file. Ignore if you want to continue (not "
                                                  "recommended, can cause a data corruption)."),
                                               QMessageBox::Abort|QMessageBox::Ignore, QMessageBox::Abort);
                break;
            default:
                answer = QMessageBox::Abort;
                break;
        }
    }

    if (answer == QMessageBox::Abort)
    {
        qCDebug(vMainWindow, "Failed to lock %s", qUtf8Printable(path));
        qCDebug(vMainWindow, "Error type: %d", error);
        Clear();
        if (not VApplication::IsGUIMode())
        {
            switch(error)
            {
                case QLockFile::LockFailedError:
                    qCCritical(vMainWindow, "%s",
                               qUtf8Printable(tr("This file already opened in another window.")));
                    break;
                case QLockFile::PermissionError:
                    qCCritical(vMainWindow, "%s",
                               qUtf8Printable(tr("The lock file could not be created, for lack of permissions.")));
                    break;
                case QLockFile::UnknownError:
                    qCCritical(vMainWindow, "%s",
                               qUtf8Printable(tr("Unknown error happened, for instance a full partition prevented "
                                                 "writing out the lock file.")));
                    break;
                default:
                    break;
            }

            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPoint() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointByRelease() const
{
    ToolSelectPoint();
    emit ItemsSelection(SelectionType::ByMouseRelease);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointByPress() const
{
    ToolSelectPoint();
    emit ItemsSelection(SelectionType::ByMousePress);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectSpline() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectSplinePath() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectArc() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectPointArc() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectCurve() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectAllDrawObjects() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectOperationObjects() const
{
    // Only true for rubber band selection
    emit EnableLabelSelection(true);
    emit EnablePointSelection(true);
    emit EnableLineSelection(false);
    emit EnableArcSelection(true);
    emit EnableElArcSelection(true);
    emit EnableSplineSelection(true);
    emit EnableSplinePathSelection(true);

    // Hovering
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectGroupObjects() const
{
    ToolSelectOperationObjects();
    // Only true for rubber band selection
    emit EnableLineSelection(true);

    // Hovering
    emit EnableLineHover(true);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ToolSelectDetail() const
{
    // Only true for rubber band selection
    emit EnableNodeLabelSelection(false);
    emit EnableNodePointSelection(false);
    emit EnableDetailSelection(true);// Disable when done visualization details

    // Hovering
    emit EnableNodeLabelHover(true);
    emit EnableNodePointHover(true);
    emit EnableDetailHover(true);

    emit ItemsSelection(SelectionType::ByMouseRelease);

    ui->view->allowRubberBand(false);
}

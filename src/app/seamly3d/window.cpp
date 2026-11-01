// SPDX-License-Identifier: GPL-3.0-or-later
#include "window.h"
#include "viewport.h"
#include "dressform.h"
#include "atelier.h"
#include <QAction>
#include <QActionGroup>
#include <QSignalBlocker>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QPainter>
#include <QSaveFile>
#include <QSet>
#include <cmath>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTextStream>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QStyle>
#include <QToolButton>
#include <QWhatsThis>
#include <QSizePolicy>
#include <QFrame>
#include <QLineEdit>
#include <QUuid>
#include <QStyleFactory>
#include <QShortcut>
#include <QProxyStyle>
#include <QStyleOption>
#include <QUndoStack>
#include <QUndoCommand>
#include <functional>
#include <algorithm>

namespace Seamly3D
{
namespace
{
class InspectionStyle final : public QProxyStyle
{
public:
    InspectionStyle() : QProxyStyle(QStyleFactory::create(QStringLiteral("Fusion"))) {}
    void drawPrimitive(PrimitiveElement element,const QStyleOption *option,QPainter *painter,const QWidget *widget=nullptr) const override
    {
        if (element!=PE_IndicatorItemViewItemCheck) { QProxyStyle::drawPrimitive(element,option,painter,widget); return; }
        // Selection may invert text colours, but the visibility tick must retain
        // independent contrast. Keep standard item hit-testing and keyboard input.
        const auto colors=Atelier::palette(); const QRectF box=QRectF(option->rect).adjusted(1,1,-1,-1);
        painter->save(); painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(QPen(colors.color(QPalette::Mid),1.2)); painter->setBrush(colors.color(QPalette::Base));
        painter->drawRoundedRect(box,2,2);
        if (option->state & State_On)
        {
            QPainterPath tick; tick.moveTo(box.left()+box.width()*0.2,box.top()+box.height()*0.5);
            tick.lineTo(box.left()+box.width()*0.43,box.top()+box.height()*0.74);
            tick.lineTo(box.left()+box.width()*0.8,box.top()+box.height()*0.25);
            painter->setPen(QPen(colors.color(QPalette::Text),1.8,Qt::SolidLine,Qt::RoundCap,Qt::RoundJoin));
            painter->setBrush(Qt::NoBrush); painter->drawPath(tick);
        }
        painter->restore();
    }
};
class ProjectEdit final : public QUndoCommand
{
    Project m_before,m_after;
    std::function<void(const Project &)> m_restore;
public:
    ProjectEdit(const Project &before,const Project &after,const QString &label,std::function<void(const Project &)> restore)
        : QUndoCommand(label),m_before(before),m_after(after),m_restore(std::move(restore)) {}
    void undo() override { m_restore(m_before); }
    void redo() override { m_restore(m_after); }
};
QString savePath(QWidget *parent, const QString &title, const QString &path, const QString &filter, const QString &suffix)
{
    QFileDialog dialog(parent,title,path,filter);
    dialog.setAcceptMode(QFileDialog::AcceptSave); dialog.setDefaultSuffix(suffix);
    return dialog.exec()==QDialog::Accepted ? dialog.selectedFiles().first() : QString();
}
}

Window::Window()
{
    m_history=new QUndoStack(this); m_history->setObjectName(QStringLiteral("assemblyHistory")); m_history->setUndoLimit(50);
    connect(m_history,&QUndoStack::cleanChanged,this,[this]() { updateDirty(); });
    setObjectName(QStringLiteral("seamlyWorkspace"));
    applyTheme();
#if QT_VERSION >= QT_VERSION_CHECK(6,5,0)
    connect(QGuiApplication::styleHints(),&QStyleHints::colorSchemeChanged,this,[this]() { applyTheme(); });
#endif
    m_project.dressForm.enabled=true;
    setWindowTitle(tr("Seamly3D — Assembly preview[*]")); resize(1440,900);
    setMinimumSize(960,640);
    auto file=menuBar()->addMenu(tr("&File"));
    auto open=file->addAction(tr("&Open project…")); open->setShortcut(QKeySequence::Open);
    connect(open,&QAction::triggered,this,[this]() {
        const auto path=QFileDialog::getOpenFileName(this,tr("Open Seamly3D project"),QString(),tr("Seamly3D projects (*.sm3d)"));
        if (!path.isEmpty()) openProject(path);
    });
    auto saveAction=file->addAction(tr("&Save")); saveAction->setShortcut(QKeySequence::Save);
    connect(saveAction,&QAction::triggered,this,[this]() { saveProject(false); });
    auto saveAs=file->addAction(tr("Save &as…")); saveAs->setShortcut(QKeySequence::SaveAs);
    connect(saveAs,&QAction::triggered,this,[this]() { saveProject(true); });
    file->addSeparator();
    connect(file->addAction(tr("Import updated geometry…")),&QAction::triggered,this,[this]() { importGeometry(); });
    connect(file->addAction(tr("Export current mesh as OBJ…")),&QAction::triggered,this,[this]() { exportObj(); });
    file->addSeparator();
    connect(file->addAction(tr("Open assembly example")),&QAction::triggered,this,[this]() { showDemo(); });
    connect(file->addAction(tr("Open dress form example")),&QAction::triggered,this,[this]() { showDressFormDemo(); });
    connect(file->addAction(tr("Open draping example")),&QAction::triggered,this,[this]() {
        auto project=dressFormDemoProject(); project.source=tr("Pinned draping example"); project.simulation.physical=true; project.meshSize=35;
        for (auto &piece : project.pieces) piece.pinTop=true;
        if (maybeSave() && acceptProject(project,QString())) { m_unsavedOrigin=true; updateDirty(); m_inspector->setCurrentIndex(3); }
    });
    connect(file->addAction(tr("&Quit")),&QAction::triggered,this,&QWidget::close);
    auto edit=menuBar()->addMenu(tr("&Edit"));
    auto undo=m_history->createUndoAction(this,tr("Undo")); undo->setObjectName(QStringLiteral("undoAction")); undo->setShortcuts(QKeySequence::Undo);
    auto redo=m_history->createRedoAction(this,tr("Redo")); redo->setObjectName(QStringLiteral("redoAction")); redo->setShortcuts(QKeySequence::Redo);
    undo->setIcon(Atelier::icon(QStringLiteral("undo"))); redo->setIcon(Atelier::icon(QStringLiteral("redo")));
    m_undoAction=undo; m_redoAction=redo; edit->addAction(undo); edit->addAction(redo);

    auto toolbar=addToolBar(tr("Workspace")); toolbar->setMovable(false);
    toolbar->setObjectName(QStringLiteral("workspaceToolbar"));
    toolbar->setIconSize(QSize(18,18)); toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto brand=new QLabel(QStringLiteral("Seamly3D")); brand->setObjectName(QStringLiteral("workspaceBrand")); toolbar->addWidget(brand);
    open->setIcon(Atelier::icon(QStringLiteral("open")));
    saveAction->setIcon(Atelier::icon(QStringLiteral("save")));
    toolbar->addAction(open); toolbar->addAction(saveAction);
    toolbar->addSeparator(); toolbar->addAction(undo); toolbar->addAction(redo);
    for (auto action : {undo,redo})
        if (auto button=qobject_cast<QToolButton *>(toolbar->widgetForAction(action))) button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    auto spacer=new QWidget; spacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); toolbar->addWidget(spacer);
    auto fit=new QAction(Atelier::icon(QStringLiteral("fit")),tr("Fit view"),this); fit->setShortcut(QKeySequence(Qt::Key_F));
    connect(fit,&QAction::triggered,this,[this]() { m_view->fit(); });
    auto wire=new QAction(Atelier::icon(QStringLiteral("mesh")),tr("Mesh"),this); wire->setCheckable(true);
    connect(wire,&QAction::toggled,this,[this](bool checked) { m_view->setWireframe(checked); });
    toolbar->addSeparator();
    auto reset=toolbar->addAction(Atelier::icon(QStringLiteral("reset")),tr("Reset assembly"));
    connect(reset,&QAction::triggered,this,[this]() { if (rebuild()) m_view->fit(); });
    m_run=new QPushButton(tr("Assemble")); m_run->setObjectName(QStringLiteral("assembleButton"));
    m_run->setDefault(true); m_run->setMinimumWidth(104); m_run->setMinimumHeight(28);
    m_run->setToolTip(tr("Assemble or pause the preview (Space while the canvas has focus)"));
    toolbar->addWidget(m_run);

    auto root=new QWidget; auto layout=new QVBoxLayout(root); layout->setContentsMargins(0,0,0,0); layout->setSpacing(0);
    auto splitter=new QSplitter; splitter->setObjectName(QStringLiteral("workspaceSplitter"));
    splitter->setChildrenCollapsible(false); splitter->setHandleWidth(1); layout->addWidget(splitter,1);
    auto stage=new QWidget; auto stageLayout=new QVBoxLayout(stage); stageLayout->setContentsMargins(0,0,0,0); stageLayout->setSpacing(0);
    auto sceneTools=new QToolBar(tr("Scene tools")); sceneTools->setObjectName(QStringLiteral("sceneToolbar"));
    sceneTools->setIconSize(QSize(18,18)); sceneTools->setToolButtonStyle(Qt::ToolButtonTextBesideIcon); stageLayout->addWidget(sceneTools);
    m_view=new Viewport(&m_project,&m_assembly); stageLayout->addWidget(m_view,1); splitter->addWidget(stage);
    auto simulateShortcut=new QShortcut(QKeySequence(Qt::Key_Space),m_view);
    simulateShortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(simulateShortcut,&QShortcut::activated,m_run,&QPushButton::click);
    auto modes=new QActionGroup(this); modes->setExclusive(true);
    const QString modeLabels[]={tr("Inspect"),tr("Move"),tr("Rotate")};
    const QString modeIcons[]={QStringLiteral("select"),QStringLiteral("move"),QStringLiteral("rotate")};
    const int modeKeys[]={Qt::Key_V,Qt::Key_G,Qt::Key_R};
    for (int mode=0; mode<3; ++mode)
    {
        auto action=sceneTools->addAction(Atelier::icon(modeIcons[mode]),modeLabels[mode]); action->setCheckable(true); modes->addAction(action);
        action->setObjectName(QStringLiteral("sceneTool%1").arg(mode)); action->setShortcut(QKeySequence(modeKeys[mode])); action->setChecked(mode==0);
        connect(action,&QAction::triggered,this,[this,mode]() { m_view->setToolMode(mode); });
        connect(m_view,&Viewport::toolModeChanged,action,[this,action,mode]() { action->setChecked(m_view->toolMode()==mode); });
    }
    auto toolSpacer=new QWidget; toolSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred); sceneTools->addWidget(toolSpacer);
    auto views=new QComboBox; views->setObjectName(QStringLiteral("cameraViews")); views->setAccessibleName(tr("Camera view"));
    views->addItems({tr("Three-quarter"),tr("Front"),tr("Back"),tr("Right"),tr("Left"),tr("Top")}); views->setMinimumContentsLength(12);
    views->setToolTip(tr("Camera views: 0 three-quarter, 1 front, 2 back, 3 right, 4 left, 5 top")); sceneTools->addWidget(views);
    connect(views,QOverload<int>::of(&QComboBox::activated),m_view,&Viewport::setView);
    connect(m_view,&Viewport::viewChanged,views,[views](int view) { QSignalBlocker blocked(views); views->setCurrentIndex(view); if (view<0) views->setPlaceholderText(tr("Custom view")); });
    for (int view=0; view<6; ++view)
    {
        auto action=new QAction(m_view); action->setShortcut(QKeySequence(Qt::Key_0+view));
        action->setShortcutContext(Qt::WidgetWithChildrenShortcut); m_view->addAction(action);
        connect(action,&QAction::triggered,this,[this,view]() { m_view->setView(view); });
    }
    sceneTools->addAction(fit); sceneTools->addAction(wire);
    // Keep fit available even when the toolbar moves actions into its overflow menu.
    addAction(fit);
    m_inspector=new QTabWidget; m_inspector->setObjectName(QStringLiteral("inspectorTabs"));
    m_inspector->setDocumentMode(true); m_inspector->setUsesScrollButtons(false);
    m_inspector->setMinimumWidth(400); m_inspector->setMaximumWidth(480);
    m_inspector->setAccessibleName(tr("Assembly inspector"));
    QVBoxLayout *seamFooter=nullptr;
    auto page=[this,&seamFooter](const QString &title,bool pinnedActions=false) {
        auto scroll=new QScrollArea; scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto content=new QWidget; auto column=new QVBoxLayout(content);
        column->setContentsMargins(20,20,20,20); column->setSpacing(12);
        scroll->setWidget(content);
        if (pinnedActions)
        {
            auto container=new QWidget; auto outer=new QVBoxLayout(container); outer->setContentsMargins(0,0,0,0); outer->setSpacing(0);
            outer->addWidget(scroll,1);
            auto footer=new QWidget; footer->setObjectName(QStringLiteral("seamEditorFooter")); footer->setAutoFillBackground(true);
            seamFooter=new QVBoxLayout(footer); seamFooter->setContentsMargins(20,10,20,12); seamFooter->setSpacing(8);
            outer->addWidget(footer); m_inspector->addTab(container,title);
        }
        else m_inspector->addTab(scroll,title);
        return column;
    };
    auto heading=[](const QString &text) {
        auto label=new QLabel(text); label->setProperty("sectionHeading",true); auto font=label->font(); font.setBold(true); label->setFont(font); return label;
    };
    auto note=[](const QString &text) {
        auto label=new QLabel(text); label->setWordWrap(true); auto font=label->font(); font.setPointSizeF(qMax(10.0,font.pointSizeF()-1));
        label->setFont(font); label->setProperty("contextNote",true); return label;
    };
    auto formControls=page(tr("Dress form"));
    auto pieceControls=page(tr("Pieces"));
    auto seamControls=page(tr("Seams"),true);
    auto clothControls=page(tr("Cloth"));
    m_inspector->setTabIcon(3,Atelier::icon(QStringLiteral("mesh")));
    m_inspector->setTabIcon(0,Atelier::icon(QStringLiteral("form")));
    m_inspector->setTabIcon(1,Atelier::icon(QStringLiteral("pieces")));
    m_inspector->setTabIcon(2,Atelier::icon(QStringLiteral("seam")));
    formControls->addWidget(note(tr("Set the form's measurements. Dimensions are in centimetres.")));
    m_formGroup=new QGroupBox(tr("Use dress form"));
    m_formGroup->setFlat(true); m_formGroup->setCheckable(true); m_formGroup->setChecked(true);
    m_formGroup->setObjectName(QStringLiteral("dressFormGroup"));
    auto formLayout=new QFormLayout(m_formGroup);
    formLayout->setContentsMargins(0,16,0,0); formLayout->setVerticalSpacing(10);
    const QString formLabels[]={tr("Bust (cm)"),tr("Waist (cm)"),tr("Hips (cm)"),tr("Neck (cm)"),tr("Shoulder width (cm)"),tr("Torso height (cm)"),tr("Contact gap (cm)")};
    const double low[]={50,40,50,25,25,40,0},high[]={160,160,180,60,65,100,3};
    for (int i=0; i<7; ++i)
    {
        m_formMeasures[i]=new QDoubleSpinBox; m_formMeasures[i]->setRange(low[i],high[i]); m_formMeasures[i]->setDecimals(1);
        m_formMeasures[i]->setObjectName(QStringLiteral("formMeasure%1").arg(i));
        m_formMeasures[i]->setKeyboardTracking(false);
        formLayout->addRow(formLabels[i],m_formMeasures[i]);
        connect(m_formMeasures[i],QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this]() { updateDressForm(); });
    }
    m_formContact=new QCheckBox(tr("Basic body contact")); m_formStand=new QCheckBox(tr("Show stand"));
    m_formContact->setObjectName(QStringLiteral("formContact")); m_formStand->setObjectName(QStringLiteral("formStand"));
    formLayout->addRow(m_formContact); formLayout->addRow(m_formStand);
    formControls->addWidget(m_formGroup); formControls->addStretch();
    m_pieceListHeading=heading(tr("Scene pieces")); pieceControls->addWidget(m_pieceListHeading);
    m_pieceList=new QListWidget; m_pieceList->setObjectName(QStringLiteral("scenePieceList"));
    // macOS's native item indicator can paint repeated checks at the same row
    // under a stylesheet. Qt's standard Fusion delegate keeps every row legible.
    auto listStyle=new InspectionStyle; listStyle->setParent(m_pieceList); m_pieceList->setStyle(listStyle);
    m_pieceList->setAccessibleName(tr("Scene pieces; check to show a piece"));
    m_pieceList->setMinimumHeight(90); m_pieceList->setMaximumHeight(140); m_pieceList->setTextElideMode(Qt::ElideMiddle);
    pieceControls->addWidget(m_pieceList);
    auto inspectionButtons=new QHBoxLayout;
    m_isolate=new QPushButton(tr("Isolate selected")); m_isolate->setObjectName(QStringLiteral("isolatePieceButton"));
    m_showAll=new QPushButton(tr("Show all")); m_showAll->setObjectName(QStringLiteral("showAllPiecesButton"));
    inspectionButtons->addWidget(m_isolate); inspectionButtons->addWidget(m_showAll); pieceControls->addLayout(inspectionButtons);
    m_transparent=new QCheckBox(tr("Transparent selected piece")); m_transparent->setObjectName(QStringLiteral("transparentPiece"));
    m_transparent->setToolTip(tr("Inspection only. Hidden and transparent pieces still participate in assembly and mesh export."));
    pieceControls->addWidget(m_transparent);
    m_posePiece=new QComboBox; m_posePiece->setObjectName(QStringLiteral("placementPiece"));
    m_posePiece->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); m_posePiece->setMinimumContentsLength(14);
    m_posePiece->setAccessibleName(tr("Selected piece for placement")); m_posePiece->setParent(root); m_posePiece->hide();
    connect(m_pieceList,&QListWidget::currentRowChanged,this,[this](int row) { if (!m_updating) m_posePiece->setCurrentIndex(row); });
    connect(m_pieceList,&QListWidget::itemChanged,this,[this](QListWidgetItem *item) {
        if (!m_updating) m_view->setPieceVisible(item->data(Qt::UserRole).toString(),item->checkState()==Qt::Checked);
    });
    connect(m_isolate,&QPushButton::clicked,this,[this]() { m_view->isolatePiece(m_posePiece->currentData().toString()); });
    connect(m_showAll,&QPushButton::clicked,m_view,&Viewport::showAllPieces);
    connect(m_transparent,&QCheckBox::toggled,this,[this](bool checked) {
        if (!m_updating) m_view->setPieceTransparent(m_posePiece->currentData().toString(),checked);
    });
    connect(m_view,&Viewport::inspectionChanged,this,[this]() { syncInspection(); });
    m_pieceHint=note(tr("Open a project exported from Seamly2D to position its pieces."));
    pieceControls->addWidget(m_pieceHint);
    pieceControls->addWidget(heading(tr("Place around dress form")));
    m_placement=new QComboBox; m_placement->addItems({tr("Front"),tr("Right"),tr("Back"),tr("Left")});
    pieceControls->addWidget(m_placement);
    m_place=new QPushButton(tr("Place selected piece")); m_place->setObjectName(QStringLiteral("placeOnFormButton")); pieceControls->addWidget(m_place);
    m_arrange=new QPushButton(tr("Arrange all pieces around form")); m_arrange->setObjectName(QStringLiteral("arrangeOnFormButton")); pieceControls->addWidget(m_arrange);
    connect(m_formGroup,&QGroupBox::toggled,this,[this]() { updateDressForm(); });
    connect(m_formContact,&QCheckBox::toggled,this,[this]() { updateDressForm(); });
    connect(m_formStand,&QCheckBox::toggled,this,[this]() { updateDressForm(); });
    connect(m_arrange,&QPushButton::clicked,this,[this]() { arrangeOnForm(true); });
    connect(m_place,&QPushButton::clicked,this,[this]() { arrangeOnForm(false); });
    m_seamsHeading=heading(tr("Saved seams")); seamControls->addWidget(m_seamsHeading);
    m_seams=new QListWidget; m_seams->setObjectName(QStringLiteral("seamList"));
    m_seams->setMinimumHeight(70); m_seams->setMaximumHeight(100); m_seams->setTextElideMode(Qt::ElideMiddle);
    m_seams->setAccessibleName(tr("Saved seams")); seamControls->addWidget(m_seams);
    auto seamManagement=new QHBoxLayout;
    m_newSeam=new QPushButton(tr("New seam")); m_newSeam->setObjectName(QStringLiteral("newSeamButton")); seamManagement->addWidget(m_newSeam);
    m_remove=new QPushButton(tr("Remove selected")); m_remove->setObjectName(QStringLiteral("removeSeamButton")); seamManagement->addWidget(m_remove);
    seamControls->addLayout(seamManagement);
    auto seamIdentity=new QFormLayout;
    seamIdentity->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_seamName=new QLineEdit; m_seamName->setObjectName(QStringLiteral("seamName")); m_seamName->setMaxLength(128);
    m_seamName->setPlaceholderText(tr("Name this seam")); m_seamName->setAccessibleName(tr("Seam name")); seamIdentity->addRow(tr("Name"),m_seamName);
    m_seamEnabled=new QCheckBox(tr("Include in assembly")); m_seamEnabled->setChecked(true); m_seamEnabled->setObjectName(QStringLiteral("seamEnabled"));
    seamIdentity->addRow(m_seamEnabled); seamControls->addLayout(seamIdentity);
    seamControls->addWidget(note(tr("Choose both ranges. Side A is solid; side B is dashed in the 3D guides.")));

    auto selectors=new QHBoxLayout;
    for (int side=0; side<2; ++side)
    {
        auto box=new QWidget; auto column=new QVBoxLayout(box); column->setContentsMargins(0,0,0,0); column->setSpacing(8);
        column->addWidget(heading(side==0 ? tr("Side A") : tr("Side B")));
        m_pieces[side]=new QComboBox; m_pieces[side]->setObjectName(QStringLiteral("piece%1").arg(side)); m_pieces[side]->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); m_pieces[side]->setMinimumContentsLength(10);
        m_pieces[side]->setAccessibleName(side==0 ? tr("Piece A") : tr("Piece B")); column->addWidget(m_pieces[side]);
        m_contours[side]=new ContourView; m_contours[side]->setSide(side); column->addWidget(m_contours[side]);
        auto form=new QFormLayout;
        m_start[side]=new QDoubleSpinBox; m_end[side]=new QDoubleSpinBox;
        m_start[side]->setObjectName(QStringLiteral("start%1").arg(side)); m_end[side]->setObjectName(QStringLiteral("end%1").arg(side));
        for (auto input : {m_start[side],m_end[side]}) { input->setDecimals(6); input->setRange(0,99.999999); input->setSingleStep(0.1); input->setSuffix(QStringLiteral("%")); } m_end[side]->setValue(25);
        form->addRow(tr("Start (%)"),m_start[side]); form->addRow(tr("End (%)"),m_end[side]); column->addLayout(form);
        m_contours[side]->endpointPicked=[this,side](double position,bool end) { (end ? m_end[side] : m_start[side])->setValue(position/BoundarySamples*100); };
        connect(m_pieces[side],QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this]() { updateArcs(); });
        connect(m_start[side],QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this]() { updateArcs(); });
        connect(m_end[side],QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this]() { updateArcs(); });
        selectors->addWidget(box);
    }
    seamControls->addLayout(selectors);
    auto hint=note(tr("Equal start and end select the full perimeter."));
    hint->setToolTip(tr("Percentages follow arc length along the original contour and wrap through zero. Green marks the start; red marks the end."));
    seamControls->addWidget(hint);
    m_reverse=new QCheckBox(tr("Reverse correspondence on side B")); m_reverse->setChecked(true); seamControls->addWidget(m_reverse);
    m_lengths=new QLabel; m_lengths->setToolTip(tr("Undeformed sewing lengths. A small gap after assembly does not establish garment fit.")); m_lengths->setWordWrap(true); seamControls->addWidget(m_lengths);
    m_add=new QPushButton(tr("Add seam")); m_add->setObjectName(QStringLiteral("addSeamButton")); seamFooter->addWidget(m_add);
    auto editingButtons=new QHBoxLayout;
    m_applySeam=new QPushButton(tr("Apply changes")); m_applySeam->setObjectName(QStringLiteral("applySeamButton")); editingButtons->addWidget(m_applySeam);
    m_revertSeam=new QPushButton(tr("Revert changes")); m_revertSeam->setObjectName(QStringLiteral("revertSeamButton")); editingButtons->addWidget(m_revertSeam);
    seamFooter->addLayout(editingButtons);
    m_seamFeedback=note(QString()); m_seamFeedback->setObjectName(QStringLiteral("seamFeedback")); seamFooter->addWidget(m_seamFeedback); seamControls->addStretch();
    connect(m_add,&QPushButton::clicked,this,[this]() {
        if (m_project.pieces.isEmpty()) return;
        auto seam=draftSeam(); seam.id=QUuid::createUuid().toString(QUuid::WithoutBraces);
        if (seam.name.isEmpty()) seam.name=tr("Seam %1").arg(m_project.seams.size()+1);
        auto candidate=m_project; candidate.seams<<seam; Project validated; QString message;
        if (!decode(encode(candidate),validated,message)) { m_seamFeedback->setText(message); m_seamFeedback->show(); return; }
        const auto before=m_project; m_project=candidate; m_editSeamId=seam.id; commitEdit(before,tr("Add seam"));
    });
    connect(m_applySeam,&QPushButton::clicked,this,[this]() { applySeamDraft(); });
    connect(m_revertSeam,&QPushButton::clicked,this,[this]() { loadSeamEditor(m_editSeamId); });
    connect(m_newSeam,&QPushButton::clicked,this,[this]() { if (applySeamDraft()) loadSeamEditor(QString()); });
    connect(m_remove,&QPushButton::clicked,this,[this]() {
        const int row=editedSeamRow(); if (row<0) return;
        const auto before=m_project; m_project.seams.removeAt(row); m_editSeamId.clear(); commitEdit(before,tr("Remove seam"));
    });
    connect(m_seams,&QListWidget::currentRowChanged,this,[this](int row) {
        if (m_updating || row<0) return;
        const QString id=m_seams->item(row)->data(Qt::UserRole).toString();
        if (!applySeamDraft()) {
            QSignalBlocker blocked(m_seams); m_seams->setCurrentRow(editedSeamRow()); return;
        }
        loadSeamEditor(id);
    });
    connect(m_seamName,&QLineEdit::textChanged,this,[this]() { updateSeamDraft(); });
    connect(m_seamEnabled,&QCheckBox::toggled,this,[this]() { updateSeamDraft(); });
    connect(m_reverse,&QCheckBox::toggled,this,[this]() { updateSeamDraft(); });
    connect(m_inspector,&QTabWidget::currentChanged,this,[this](int tab) {
        if (m_updating) return;
        if (tab!=2 && !applySeamDraft()) { QSignalBlocker blocked(m_inspector); m_inspector->setCurrentIndex(2); }
        refreshSeamPreview();
    });
    pieceControls->addWidget(heading(tr("Initial placement")));
    auto poseGrid=new QGridLayout; poseGrid->setHorizontalSpacing(12); poseGrid->setVerticalSpacing(8);
    poseGrid->addWidget(note(tr("Axis")),0,0); poseGrid->addWidget(note(tr("Position (mm)")),0,1); poseGrid->addWidget(note(tr("Rotation (°)")),0,2);
    const QString axes[]={tr("X"),tr("Y"),tr("Z")};
    for (int i=0; i<6; ++i)
    {
        m_pose[i]=new QDoubleSpinBox; m_pose[i]->setObjectName(QStringLiteral("pose%1").arg(i));
        m_pose[i]->setRange(i<3 ? -100000 : -360,i<3 ? 100000 : 360);
        m_pose[i]->setKeyboardTracking(false); m_pose[i]->setDecimals(1); m_pose[i]->setSingleStep(i<3 ? 5 : 15); m_pose[i]->setMinimumWidth(90);
        m_pose[i]->setAccessibleName(i<3 ? tr("%1 position in millimetres").arg(axes[i]) : tr("%1 rotation in degrees").arg(axes[i-3]));
        poseGrid->addWidget(m_pose[i],i%3+1,i<3 ? 1 : 2);
        if (i<3) { auto label=new QLabel(axes[i]); label->setBuddy(m_pose[i]); poseGrid->addWidget(label,i+1,0); }
        connect(m_pose[i],QOverload<double>::of(&QDoubleSpinBox::valueChanged),this,[this,i](double value) {
            if (m_updating || m_posePiece->currentIndex()<0) return;
            const auto before=m_project;
            auto &piece=m_project.pieces[m_posePiece->currentIndex()];
            if (i<3) piece.translation[i]=float(value); else piece.rotation[i-3]=float(value);
            commitEdit(before,i<3 ? tr("Move piece") : tr("Rotate piece"));
        });
    }
    poseGrid->setColumnStretch(1,1); poseGrid->setColumnStretch(2,1); pieceControls->addLayout(poseGrid); pieceControls->addStretch();
    connect(m_posePiece,QOverload<int>::of(&QComboBox::currentIndexChanged),this,[this](int index) {
        if (m_updating) return;
        for (auto input : m_pose) input->setEnabled(index>=0);
        if (index<0) { syncInspection(); return; }
        m_posePiece->setToolTip(m_posePiece->currentText());
        m_updating=true;
        for (int i=0; i<6; ++i) m_pose[i]->setValue(i<3 ? m_project.pieces[index].translation[i] : m_project.pieces[index].rotation[i-3]);
        m_updating=false; m_view->selectPiece(m_posePiece->currentData().toString()); syncInspection();
    });
    connect(m_view,&Viewport::pieceSelected,this,[this](const QString &id) {
        if (m_updating) return;
        m_inspector->setCurrentIndex(1);
        m_posePiece->setCurrentIndex(m_posePiece->findData(id));
        if (m_editSeamId.isEmpty()) m_pieces[0]->setCurrentIndex(m_pieces[0]->findData(id)); syncInspection();
    });
    connect(m_view,&Viewport::selectionChanged,this,[this]() {
        if (m_updating) return;
        m_posePiece->setCurrentIndex(m_posePiece->findData(m_view->selectedId())); syncInspection();
    });
    connect(m_view,&Viewport::transformStarted,this,[this](const QString &id) {
        m_dragBefore=m_project;
        for (const auto &piece : m_project.pieces) if (piece.id==id) { m_dragStart=piece; m_dragModified=isWindowModified(); break; }
        rebuild();
    });
    connect(m_view,&Viewport::transformRequested,this,[this](const QString &id,int axis,bool rotation,float total) {
        for (auto &piece : m_project.pieces) if (piece.id==id)
        {
            if (rotation) {
                // Same Euler axis as the numeric editor; wrap to the persisted range.
                double degrees=std::fmod(double(m_dragStart.rotation[axis])+total,360.0);
                piece.rotation[axis]=float(degrees);
            } else piece.translation[axis]=qBound(-100000.f,m_dragStart.translation[axis]+total,100000.f);
            m_updating=true;
            for (int i=0; i<6; ++i) m_pose[i]->setValue(i<3 ? piece.translation[i] : piece.rotation[i-3]);
            m_updating=false; setWindowModified(true); rebuild(); break;
        }
    });
    connect(m_view,&Viewport::transformFinished,this,[this](bool cancel) {
        if (cancel) { m_project=m_dragBefore; rebuild(); sync(); setWindowModified(m_dragModified); }
        else commitEdit(m_dragBefore,tr("Place piece"));
    });
    m_pinTop=new QCheckBox(tr("Fix top centre during draping")); m_pinTop->setObjectName(QStringLiteral("pinTop"));
    m_pinTop->setToolTip(tr("Fix the middle half of the highest edge in the original 2D contour; leave corners free for seams. Applies only to physical draping."));
    pieceControls->insertWidget(3,m_pinTop);
    connect(m_pinTop,&QCheckBox::toggled,this,[this](bool checked) {
        if (m_updating || m_posePiece->currentIndex()<0) return;
        const auto before=m_project; m_project.pieces[m_posePiece->currentIndex()].pinTop=checked; commitEdit(before,tr("Fix cloth edge"));
    });
    clothControls->addWidget(heading(tr("Mesh resolution")));
    clothControls->addWidget(note(tr("Edge size is measured in millimetres. Corners and seam endpoints stay on the original contour when density changes.")));
    auto meshForm=new QFormLayout; meshForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    m_meshSize=new QDoubleSpinBox; m_meshSize->setObjectName(QStringLiteral("meshSize")); m_meshSize->setRange(5,100); m_meshSize->setDecimals(1); m_meshSize->setSuffix(tr(" mm"));
    meshForm->addRow(tr("Maximum edge"),m_meshSize); clothControls->addLayout(meshForm);
    m_meshInfo=new QLabel; m_meshInfo->setObjectName(QStringLiteral("meshInfo")); m_meshInfo->setWordWrap(true); clothControls->addWidget(m_meshInfo);
    clothControls->addWidget(heading(tr("Assembly and draping")));
    m_solverMode=new QComboBox; m_solverMode->setObjectName(QStringLiteral("solverMode"));
    m_solverMode->addItems({tr("Geometric assembly"),tr("Physical draping · experimental")}); clothControls->addWidget(m_solverMode);
    clothControls->addWidget(note(tr("Draping adds gravity and elastic cloth constraints. Use Fix top centre in Pieces to suspend a panel. Body contact is sampled; self-collisions and fabric layers are not supported.")));
    auto materialForm=new QFormLayout; materialForm->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    const QString labels[]={tr("Surface mass (g/m²)"),tr("Stretch softness"),tr("Bend softness"),tr("Damping (1/s)"),tr("Contact friction")};
    const double clothLow[]={10,0,0,0,0},clothHigh[]={2000,0.01,10,20,1}; const int precision[]={0,8,5,2,2};
    for (int i=0;i<5;++i)
    {
        m_clothValues[i]=new QDoubleSpinBox; m_clothValues[i]->setObjectName(QStringLiteral("clothParameter%1").arg(i));
        m_clothValues[i]->setDecimals(precision[i]); m_clothValues[i]->setRange(clothLow[i],clothHigh[i]); m_clothValues[i]->setSingleStep(i==1 ? 0.000001 : i==2 ? 0.01 : 0.1);
        if (i==1 || i==2) m_clothValues[i]->setToolTip(tr("Uncalibrated XPBD compliance in m/N. Zero is rigid; increasing this value makes the constraint softer. Bending uses an approximate distance constraint."));
        materialForm->addRow(labels[i],m_clothValues[i]);
        connect(m_clothValues[i],&QDoubleSpinBox::editingFinished,this,&Window::updateSimulationSettings);
    }
    clothControls->addLayout(materialForm);
    m_solverInfo=new QLabel; m_solverInfo->setObjectName(QStringLiteral("solverInfo")); m_solverInfo->setWordWrap(true); clothControls->addWidget(m_solverInfo);
    clothControls->addWidget(note(tr("Fixed simulation step: 1/60 s. Stops after one second of low motion or at 20 s. A time limit is not convergence. Parameters are not calibrated textile presets."))); clothControls->addStretch();
    connect(m_meshSize,&QDoubleSpinBox::editingFinished,this,&Window::updateSimulationSettings);
    connect(m_solverMode,QOverload<int>::of(&QComboBox::activated),this,[this]() { updateSimulationSettings(); });
    splitter->addWidget(m_inspector); splitter->setStretchFactor(0,1); splitter->setStretchFactor(1,0);
    splitter->setSizes({1000,440}); setCentralWidget(root);
    m_stateSummary=new QLabel; statusBar()->addWidget(m_stateSummary,1);
    m_metrics=new QLabel; statusBar()->addPermanentWidget(m_metrics);
    m_backend=new QLabel; statusBar()->addPermanentWidget(m_backend);
    connect(m_view,&Viewport::backendChanged,this,[this]() { m_backend->setText(m_view->backend()); });
    auto limitations=new QToolButton; limitations->setText(tr("Experimental preview"));
    limitations->setToolTip(tr("Read the preview's capabilities and limitations"));
    limitations->setAccessibleName(tr("Preview limitations")); statusBar()->addPermanentWidget(limitations);
    connect(limitations,&QToolButton::clicked,this,[this,limitations]() {
        QWhatsThis::showText(limitations->mapToGlobal(QPoint(0,0)),
            tr("Geometric assembly and experimental physical draping are available in Cloth. Body contact is sampled and approximate. "
               "Draping uses gravity, stretch/shear and approximate bending constraints. Self-collisions, layers, calibrated materials and stuffing are not simulated. "
               "Seam gap and edge strain do not establish garment fit."),limitations);
    });
    m_timer=new QTimer(this); m_timer->setInterval(33);
    connect(m_timer,&QTimer::timeout,this,[this]() {
        advance(m_project.simulation.physical ? 2 : 8);
        if (m_assembly.simulationStopped()) { stop(); statusBar()->showMessage(m_assembly.simulationStatus()+tr(" Reset assembly to restart.")); }
        else if (!m_project.simulation.physical && m_iterations>=2400) { stop(); statusBar()->showMessage(tr("Preview stopped after 2400 iterations. Inspect seams and reset placement if needed.")); }
    });
    connect(m_run,&QPushButton::clicked,this,[this]() {
        if (m_timer->isActive()) stop();
        else { m_iterations=0; m_timer->start(); m_run->setText(tr("Pause")); }
    });
    sync(); m_view->fit();
}
void Window::applyTheme()
{
    setStyleSheet(Atelier::styleSheet());
    const auto colors=Atelier::palette(); setPalette(colors);
    for (auto widget : findChildren<QWidget *>()) widget->setPalette(colors);
    update();
}
void Window::updateDirty() { setWindowModified(m_unsavedOrigin || !m_history->isClean()); }
int Window::editedSeamRow() const
{
    for (int i=0; i<m_project.seams.size(); ++i) if (m_project.seams[i].id==m_editSeamId) return i;
    return -1;
}
Seam Window::draftSeam() const
{
    Seam seam; seam.id=m_editSeamId; seam.name=m_seamName->text().trimmed(); seam.enabled=m_seamEnabled->isChecked();
    seam.a=m_pieces[0]->currentData().toString(); seam.b=m_pieces[1]->currentData().toString();
    seam.startA=m_start[0]->value()*BoundarySamples/100; seam.endA=m_end[0]->value()*BoundarySamples/100; seam.startB=m_start[1]->value()*BoundarySamples/100; seam.endB=m_end[1]->value()*BoundarySamples/100;
    seam.reverse=m_reverse->isChecked(); return seam;
}
bool Window::seamDraftDirty() const
{
    const int row=editedSeamRow(); if (row<0) return false;
    const auto draft=draftSeam(),saved=m_project.seams[row];
    return draft.name!=saved.name || draft.enabled!=saved.enabled || draft.a!=saved.a || draft.b!=saved.b ||
        std::abs(draft.startA-saved.startA)>1e-6 || std::abs(draft.endA-saved.endA)>1e-6 || std::abs(draft.startB-saved.startB)>1e-6 || std::abs(draft.endB-saved.endB)>1e-6 || draft.reverse!=saved.reverse;
}
void Window::loadSeamEditor(const QString &id)
{
    const bool wasUpdating=m_updating; m_updating=true; m_editSeamId=id;
    const int row=editedSeamRow();
    if (row>=0)
    {
        const auto seam=m_project.seams[row];
        m_seamName->setText(seam.name); m_seamEnabled->setChecked(seam.enabled);
        m_pieces[0]->setCurrentIndex(m_pieces[0]->findData(seam.a)); m_pieces[1]->setCurrentIndex(m_pieces[1]->findData(seam.b));
        m_start[0]->setValue(seam.startA/BoundarySamples*100); m_end[0]->setValue(seam.endA/BoundarySamples*100); m_start[1]->setValue(seam.startB/BoundarySamples*100); m_end[1]->setValue(seam.endB/BoundarySamples*100);
        m_reverse->setChecked(seam.reverse); m_seams->setCurrentRow(row);
    }
    else
    {
        m_editSeamId.clear(); m_seamName->clear(); m_seamEnabled->setChecked(true); m_seams->setCurrentRow(-1);
    }
    m_updating=wasUpdating; if (!m_updating) updateArcs();
}
bool Window::applySeamDraft()
{
    if (!seamDraftDirty()) return true;
    const int row=editedSeamRow(); auto candidate=m_project; candidate.seams[row]=draftSeam();
    Project validated; QString message;
    if (!decode(encode(candidate),validated,message)) { m_seamFeedback->setText(message+tr(" Correct the ranges or Revert changes.")); return false; }
    const auto before=m_project; m_project=candidate; return commitEdit(before,tr("Edit seam"));
}
void Window::refreshSeamPreview()
{
    if (m_inspector->currentIndex()==2 && !m_project.pieces.isEmpty()) { const auto seam=draftSeam(); m_view->setSeamPreview(&seam); }
    else m_view->setSeamPreview(nullptr);
}
void Window::updateSeamDraft()
{
    if (m_updating) return;
    const bool editing=editedSeamRow()>=0,dirty=seamDraftDirty();
    bool valid=true; QString message;
    if (dirty)
    {
        auto candidate=m_project; candidate.seams[editedSeamRow()]=draftSeam(); Project validated;
        valid=decode(encode(candidate),validated,message); if (m_timer->isActive()) stop();
    }
    m_add->setVisible(!editing); m_applySeam->setVisible(editing); m_revertSeam->setVisible(editing);
    m_applySeam->setEnabled(dirty && valid); m_revertSeam->setEnabled(dirty); m_remove->setEnabled(editing);
    m_undoAction->setEnabled(m_history->canUndo() && !dirty); m_redoAction->setEnabled(m_history->canRedo() && !dirty);
    m_seamFeedback->setText(!valid ? message+tr(" Correct the ranges or Revert changes.") :
        (dirty ? tr("Unapplied changes. Apply or Revert before undo. Switching seams applies valid changes.") :
         (editing && !m_seamEnabled->isChecked() ? tr("Paused: this seam is saved but excluded from assembly.") : QString())));
    m_seamFeedback->setVisible(!m_seamFeedback->text().isEmpty());
    const bool activeSeams=(m_project.simulation.physical && !m_project.pieces.isEmpty()) || std::any_of(m_project.seams.cbegin(),m_project.seams.cend(),[](const Seam &seam) { return seam.enabled; });
    const auto &form=m_project.dressForm;
    m_run->setEnabled(!dirty && !m_assembly.simulationStopped() && (activeSeams || (form.enabled && form.contact && !m_project.pieces.isEmpty())));
    refreshSeamPreview();
}
void Window::restoreHistory(const Project &project)
{
    stop(); m_project=project; rebuild(); sync();
}
bool Window::commitEdit(const Project &before,const QString &label)
{
    if (encode(before)==encode(m_project)) { updateDirty(); return true; }
    Assembly check; QString message;
    if (!check.reset(m_project,message)) { m_project=before; rebuild(); sync(); updateDirty(); error(message); return false; }
    m_history->push(new ProjectEdit(before,m_project,label,[this](const Project &project) { restoreHistory(project); }));
    updateDirty(); return true;
}
void Window::syncInspection()
{
    const bool updating=m_updating; m_updating=true;
    const auto id=m_posePiece->currentData().toString();
    m_pieceList->setCurrentRow(-1);
    for (int row=0; row<m_pieceList->count(); ++row)
    {
        auto item=m_pieceList->item(row); const auto itemId=item->data(Qt::UserRole).toString();
        item->setCheckState(m_view->pieceVisible(itemId) ? Qt::Checked : Qt::Unchecked);
        item->setToolTip(item->text()+QStringLiteral("\n")+
            (m_view->pieceVisible(itemId) ? tr("Visible") : tr("Hidden"))+
            (m_view->pieceTransparent(itemId) ? tr(" · Transparent") : QString()));
        if (itemId==id) m_pieceList->setCurrentRow(row);
    }
    m_isolate->setEnabled(!id.isEmpty()); m_transparent->setEnabled(!id.isEmpty());
    m_transparent->setChecked(!id.isEmpty() && m_view->pieceTransparent(id));
    m_showAll->setEnabled(!m_project.pieces.isEmpty());
    const int pieceIndex=m_posePiece->currentIndex();
    m_pinTop->setEnabled(pieceIndex>=0); m_pinTop->setChecked(pieceIndex>=0 && m_project.pieces[pieceIndex].pinTop);
    m_updating=updating;
}
void Window::error(const QString &message) { QMessageBox::warning(this,tr("Seamly3D"),message); }
void Window::stop() { m_timer->stop(); m_run->setText(tr("Assemble")); }
bool Window::rebuild()
{
    stop(); QString message;
    if (m_project.pieces.isEmpty() && !m_project.dressForm.enabled)
    { m_assembly=Assembly(); m_view->synchronize(); updateMetrics(); return true; }
    if (!m_assembly.reset(m_project,message)) { error(message); return false; }
    m_iterations=0; updateMetrics(); m_view->synchronize(); return true;
}
void Window::advance(int iterations) { m_assembly.step(iterations); m_iterations+=iterations; updateMetrics(); m_view->synchronize(); }
void Window::updateMetrics()
{
    m_metrics->setText(tr("Seam gap %1 mm   ·   Edge strain %2%")
                       .arg(m_assembly.seamError(),0,'f',2).arg(m_assembly.meanStrain()*100,0,'f',1));
    int vertices=0,triangles=0; for (const auto &mesh : m_assembly.meshes()) { vertices+=mesh.vertices.size(); triangles+=mesh.triangles.size(); }
    m_meshInfo->setText(tr("%1 vertices · %2 triangles").arg(vertices).arg(triangles));
    m_solverInfo->setText(m_project.simulation.physical ? tr("%1\nSimulated %2 s · Maximum speed %3 mm/s").arg(m_assembly.simulationStatus()).arg(m_assembly.simulationTime(),0,'f',2).arg(m_assembly.maximumSpeed(),0,'f',1) : tr("Geometric assembly · no gravity"));
    if (m_assembly.simulationStopped()) m_run->setEnabled(false);
    const int active=int(std::count_if(m_project.seams.cbegin(),m_project.seams.cend(),[](const Seam &seam) { return seam.enabled; }));
    m_stateSummary->setText(tr("Pieces %1   ·   Seams %2/%3 active").arg(m_project.pieces.size()).arg(active).arg(m_project.seams.size()));
}
void Window::updateArcs()
{
    if (m_updating) return;
    double lengths[2]={0,0};
    for (int side=0; side<2; ++side)
    {
        m_pieces[side]->setToolTip(m_pieces[side]->currentText());
        const int index=m_pieces[side]->currentIndex();
        const Piece *piece=index>=0 ? &m_project.pieces[index] : nullptr;
        m_contours[side]->setPiece(piece,m_start[side]->value()*BoundarySamples/100,m_end[side]->value()*BoundarySamples/100);
        if (piece) lengths[side]=seamLength(*piece,m_start[side]->value()*BoundarySamples/100,m_end[side]->value()*BoundarySamples/100);
    }
    m_lengths->setText(tr("A %1 mm   ·   B %2 mm\nDifference %3 mm")
                       .arg(lengths[0],0,'f',1).arg(lengths[1],0,'f',1).arg(std::abs(lengths[0]-lengths[1]),0,'f',1));
    updateSeamDraft();
}
void Window::sync()
{
    const QString a=m_pieces[0]->currentData().toString(),b=m_pieces[1]->currentData().toString(),pose=m_posePiece->currentData().toString();
    m_updating=true;
    for (auto combo : {m_pieces[0],m_pieces[1],m_posePiece})
    {
        combo->clear(); for (const auto &p : m_project.pieces) { combo->addItem(p.name,p.id); combo->setItemData(combo->count()-1,p.name,Qt::ToolTipRole); }
    }
    m_pieces[0]->setCurrentIndex(qMax(0,m_pieces[0]->findData(a)));
    m_pieces[1]->setCurrentIndex(b.isEmpty() && m_project.pieces.size()>1 ? 1 : qMax(0,m_pieces[1]->findData(b)));
    m_posePiece->setCurrentIndex(qMax(0,m_posePiece->findData(pose)));
    m_posePiece->setToolTip(m_posePiece->currentText());
    m_pieceList->clear();
    for (const auto &piece : m_project.pieces)
    {
        auto item=new QListWidgetItem(piece.name,m_pieceList); item->setData(Qt::UserRole,piece.id);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(m_view->pieceVisible(piece.id) ? Qt::Checked : Qt::Unchecked);
    }
    m_pieceListHeading->setText(tr("Scene pieces (%1)").arg(m_project.pieces.size()));
    m_seams->clear();
    auto name=[this](const QString &id) { for (const auto &p : m_project.pieces) if (p.id==id) return p.name; return id; };
    for (const auto &seam : m_project.seams)
    {
        auto item=new QListWidgetItem(seam.name+(seam.enabled ? QString() : tr(" · Paused")),m_seams); item->setData(Qt::UserRole,seam.id);
        item->setToolTip(tr("%1 [%2%→%3%] ↔ %4 [%5%→%6%]%7\nID: %8").arg(name(seam.a)).arg(seam.startA/BoundarySamples*100).arg(seam.endA/BoundarySamples*100).arg(name(seam.b)).arg(seam.startB/BoundarySamples*100).arg(seam.endB/BoundarySamples*100).arg(seam.reverse ? tr(" reversed") : QString(),seam.id));
    }
    m_seamsHeading->setText(tr("Saved seams (%1)").arg(m_project.seams.size()));
    loadSeamEditor(m_editSeamId);
    const int index=m_posePiece->currentIndex();
    for (int i=0; i<6; ++i) { m_pose[i]->setEnabled(index>=0); if (index>=0) m_pose[i]->setValue(i<3 ? m_project.pieces[index].translation[i] : m_project.pieces[index].rotation[i-3]); }
    m_meshSize->setValue(m_project.meshSize); m_solverMode->setCurrentIndex(m_project.simulation.physical ? 1 : 0);
    const auto &sim=m_project.simulation; const double cloth[]={sim.density,sim.stretchCompliance,sim.bendCompliance,sim.damping,sim.friction};
    for (int i=0;i<5;++i) { m_clothValues[i]->setValue(cloth[i]); m_clothValues[i]->setEnabled(sim.physical); }
    const auto &form=m_project.dressForm;
    m_formGroup->setChecked(form.enabled); m_formContact->setChecked(form.contact); m_formStand->setChecked(form.stand);
    const double measures[]={form.bust,form.waist,form.hips,form.neck,form.shoulders,form.height,form.clearance};
    for (int i=0; i<7; ++i) m_formMeasures[i]->setValue(measures[i]/10);
    const bool canArrange=form.enabled && !m_project.pieces.isEmpty();
    m_arrange->setEnabled(canArrange); m_place->setEnabled(canArrange); m_placement->setEnabled(canArrange);
    m_pieceHint->setText(m_project.pieces.isEmpty() ? tr("Open a project exported from Seamly2D to position its pieces.") :
        (form.enabled ? tr("Click a piece in the 3D view or choose it above. Placement changes reset the preview.") : tr("Enable the dress form to use placement presets. Numeric transforms remain available.")));
    m_add->setEnabled(!m_project.pieces.isEmpty());
    updateMetrics(); m_view->synchronize();
    m_view->selectPiece(m_posePiece->currentData().toString());
    m_updating=false; updateArcs(); syncInspection();
}
bool Window::acceptProject(const Project &project, const QString &path)
{
    Assembly candidate; Project normalized; QString message;
    if (!decode(encode(project),normalized,message) || !candidate.reset(normalized,message)) { error(message); return false; }
    m_view->endTransform(true);
    stop(); m_editSeamId.clear(); m_project=normalized; m_assembly=candidate; m_path=path; m_iterations=0;
    setWindowModified(false); setWindowTitle(tr("%1 — Seamly3D[*]").arg(path.isEmpty() ? tr("Assembly example") : QFileInfo(path).fileName()));
    m_view->resetInspection(); m_unsavedOrigin=false; m_history->clear(); m_history->setClean();
    sync(); updateDirty(); m_view->fit(); return true;
}
bool Window::openProject(const QString &path)
{
    Project project; QString message;
    if (!load(path,project,message)) { error(message); return false; }
    if (!maybeSave()) return false;
    return acceptProject(project,path);
}
void Window::showDemo() { if (maybeSave() && acceptProject(demoProject(),QString())) { m_unsavedOrigin=true; updateDirty(); } }
void Window::showDressFormDemo() { if (maybeSave() && acceptProject(dressFormDemoProject(),QString())) { m_unsavedOrigin=true; updateDirty(); } }
void Window::updateSimulationSettings()
{
    if (m_updating) return;
    auto candidate=m_project; candidate.meshSize=m_meshSize->value(); candidate.simulation.physical=m_solverMode->currentIndex()==1;
    auto &sim=candidate.simulation; sim.density=m_clothValues[0]->value(); sim.stretchCompliance=m_clothValues[1]->value();
    sim.bendCompliance=m_clothValues[2]->value(); sim.damping=m_clothValues[3]->value(); sim.friction=m_clothValues[4]->value();
    if (!applySeamDraft()) { sync(); return; }
    candidate.seams=m_project.seams; const auto before=m_project; m_project=candidate; commitEdit(before,tr("Change mesh or draping"));
}
void Window::updateDressForm()
{
    if (m_updating) return;
    auto form=m_project.dressForm;
    form.enabled=m_formGroup->isChecked(); form.contact=m_formContact->isChecked(); form.stand=m_formStand->isChecked();
    double *measures[]={&form.bust,&form.waist,&form.hips,&form.neck,&form.shoulders,&form.height,&form.clearance};
    for (int i=0; i<7; ++i) *measures[i]=m_formMeasures[i]->value()*10;
    QString message; if (!validateDressForm(form,message)) { error(message); return; }
    const auto before=m_project; m_project.dressForm=form; commitEdit(before,tr("Edit dress form")); m_view->fit();
}
void Window::arrangeOnForm(bool all)
{
    if (m_project.pieces.isEmpty()) return;
    const auto before=m_project;
    if (all) for (int i=0; i<m_project.pieces.size(); ++i)
        placeAroundDressForm(m_project.pieces[i],m_project.dressForm,360.0*i/m_project.pieces.size());
    else if (m_posePiece->currentIndex()>=0)
        placeAroundDressForm(m_project.pieces[m_posePiece->currentIndex()],m_project.dressForm,90.0*m_placement->currentIndex());
    commitEdit(before,all ? tr("Arrange pieces") : tr("Place on form")); m_view->fit();
}
bool Window::rendererReady() const { return m_view->rendererReady(); }
QString Window::rendererError() const { return m_view->rendererError(); }
QString Window::graphicsBackend() const { return m_view->backend(); }
bool Window::render(const QString &path)
{
    const auto frame=m_view->capture();
    if (frame.isNull()) return false;
    auto image=grab().toImage();
    QPainter painter(&image);
    painter.drawImage(QRect(m_view->mapTo(this,QPoint()),m_view->size()),frame);
    painter.end(); return image.save(path);
}
bool Window::saveProject(bool saveAs)
{
    if (m_project.pieces.isEmpty() && !m_project.dressForm.enabled) return false;
    QString path=m_path;
    if (saveAs || path.isEmpty())
    {
        path=savePath(this,tr("Save assembly project"),path,tr("Seamly3D projects (*.sm3d)"),QStringLiteral("sm3d"));
    }
    if (path.isEmpty()) return false;
    if (!path.endsWith(QStringLiteral(".sm3d"),Qt::CaseInsensitive)) path+=QStringLiteral(".sm3d");
    return saveToPath(path);
}
bool Window::saveToPath(const QString &path)
{
    m_view->endTransform(false);
    if (!applySeamDraft()) return false;
    QString message;
    if (!save(path,m_project,message)) { error(message); return false; }
    m_path=path; m_unsavedOrigin=false; m_history->setClean(); updateDirty(); setWindowTitle(tr("%1 — Seamly3D[*]").arg(QFileInfo(path).fileName())); return true;
}
bool Window::maybeSave()
{
    m_view->endTransform(false);
    if (!applySeamDraft()) return false;
    if (!isWindowModified()) return true;
    const auto answer=QMessageBox::question(this,tr("Save assembly?"),tr("Save changes to the assembly project?"),QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
    if (answer==QMessageBox::Cancel) return false;
    return answer==QMessageBox::Discard || saveProject(false);
}
void Window::closeEvent(QCloseEvent *event) { if (maybeSave()) event->accept(); else event->ignore(); }
void Window::importGeometry()
{
    const auto path=QFileDialog::getOpenFileName(this,tr("Import geometry exported from Seamly2D"),QString(),tr("Seamly3D projects (*.sm3d)"));
    if (!path.isEmpty()) importFromPath(path);
}
bool Window::importFromPath(const QString &path)
{
    m_view->endTransform(false);
    if (!applySeamDraft()) return false;
    Project incoming; QString message;
    if (!load(path,incoming,message)) { error(message); return false; }
    const Project candidate=mergeGeometry(m_project,incoming);
    const int removed=int(m_project.seams.size()-candidate.seams.size());
    Assembly assembly;
    if (!assembly.reset(candidate,message)) { error(message); return false; }
    const auto before=m_project; m_project=candidate; commitEdit(before,tr("Import geometry")); m_view->fit();
    statusBar()->showMessage(tr("Geometry imported. %1 seams removed because a contour changed or a piece disappeared; redefine these seams.").arg(removed));
    return true;
}
void Window::exportObj()
{
    if (seamDraftDirty())
    {
        m_seamFeedback->setText(tr("Apply or Revert seam changes before exporting the mesh.")); m_seamFeedback->show();
        m_inspector->setCurrentIndex(2); return;
    }
    if (m_project.pieces.isEmpty()) return;
    QString path=savePath(this,tr("Export assembled mesh"),QString(),tr("Wavefront OBJ (*.obj)"),QStringLiteral("obj"));
    if (path.isEmpty()) return;
    if (!path.endsWith(QStringLiteral(".obj"),Qt::CaseInsensitive)) path+=QStringLiteral(".obj");
    QSaveFile file(path); if (!file.open(QIODevice::WriteOnly)) { error(file.errorString()); return; }
    QTextStream stream(&file); stream << "# Seamly3D assembly preview; coordinates in millimetres\n";
    int offset=1,index=0;
    for (const auto &mesh : m_assembly.meshes())
    {
        stream << "o piece_" << ++index << '\n';
        for (const auto &v : mesh.vertices) stream << "v " << v.x() << ' ' << v.y() << ' ' << v.z() << '\n';
        for (const auto &t : mesh.triangles) stream << "f " << t[0]+offset << ' ' << t[1]+offset << ' ' << t[2]+offset << '\n';
        offset+=int(mesh.vertices.size());
    }
    stream.flush();
    if (stream.status()!=QTextStream::Ok || !file.commit()) error(file.errorString());
}
}

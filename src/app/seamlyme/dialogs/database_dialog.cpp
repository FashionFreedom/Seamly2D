//---------------------------------------------------------------------------------------------------------------------
//  @file   database_dialog.cpp
//  @author Douglas S Caskey
//  @date   3 Sep, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogmdatabase.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   26 Jul, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "database_dialog.h"
#include "ui_database_dialog.h"
#include "../application_me.h"
#include "../vpatterndb/measurements_def.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QScreen>
#include <QSvgRenderer>
#include <QtSvg>

//---------------------------------------------------------------------------------------------------------------------
/// @brief MeasurementDatabaseDialog Overloaded constructor initialized to display interactive list selection tools.
///
/// Allocates user interface containers while explicitly forcing selection mode logic to true. It caches an external
/// database filtering tracking list, dynamically caps the dialog window height to eighty percent of the primary
/// monitor's vertical availability threshold, and applies standard cross-platform window flags. It compiles the
/// core tree category hierarchies, maps keyboard navigation monitors to the selection tree, binds dynamic scaling
/// filters to the diagram container frame, and links signal-slot logic hooks for real-time text query filtering,
/// state validations, and coordinate snapshots before reading layout states.
///
/// @param list A primary list of pre-existing data token values used to evaluate baseline checkbox activations.
/// @param parent A pointer to the parent widget container managing this dialog instance.
//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::MeasurementDatabaseDialog(const QStringList &list, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMDataBase)
    , m_selectMode(true)
    , m_measurements(list)
    , m_newMeasurements()
    , m_currentSvgPath()
    , m_currentNumber()
    , m_currentName()
    , m_groupA(nullptr)
    , m_groupB(nullptr)
    , m_groupC(nullptr)
    , m_groupD(nullptr)
    , m_groupE(nullptr)
    , m_groupF(nullptr)
    , m_groupG(nullptr)
    , m_groupH(nullptr)
    , m_groupI(nullptr)
    , m_groupJ(nullptr)
    , m_groupK(nullptr)
    , m_groupL(nullptr)
    , m_groupM(nullptr)
    , m_groupN(nullptr)
    , m_groupO(nullptr)
    , m_groupP(nullptr)
    , m_groupQ(nullptr)
{
    ui->setupUi(this);
    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    initDataBase();

    ui->treeWidget->installEventFilter(this);
    ui->diagram_Frame->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged,                this, &MeasurementDatabaseDialog::filterGroups);
    connect(ui->treeWidget, &QTreeWidget::itemChanged,                this, &MeasurementDatabaseDialog::updateChecks);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,                this, &MeasurementDatabaseDialog::showDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MeasurementDatabaseDialog::treeContextMenu);

    readSettings();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief MeasurementDatabaseDialog Constructs the database dialog window and initializes system layers.
///
/// Allocates internal user interface objects, establishes default fallback tracking variables, and null-initializes
/// categorical tree groupings. It handles cross-platform window hint adjustments, constructs the foundational
/// database tree layout, registers operational keyboard and mouse event monitors, and establishes signal-slot
/// connections for search string filtering, node navigation, and right-click context menus before restoring
/// persistent geometry layouts.
///
/// @param parent A pointer to the parent widget container managing this dialog instance.
//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::MeasurementDatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMDataBase)
    , m_selectMode(false)
    , m_measurements()
    , m_newMeasurements()
    , m_currentSvgPath()
    , m_currentNumber()
    , m_currentName()
    , m_currentDescription()
    , m_groupA(nullptr)
    , m_groupB(nullptr)
    , m_groupC(nullptr)
    , m_groupD(nullptr)
    , m_groupE(nullptr)
    , m_groupF(nullptr)
    , m_groupG(nullptr)
    , m_groupH(nullptr)
    , m_groupI(nullptr)
    , m_groupJ(nullptr)
    , m_groupK(nullptr)
    , m_groupL(nullptr)
    , m_groupM(nullptr)
    , m_groupN(nullptr)
    , m_groupO(nullptr)
    , m_groupP(nullptr)
    , m_groupQ(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    initDataBase();

    ui->treeWidget->installEventFilter(this);
    ui->treeWidget->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged,                this, &MeasurementDatabaseDialog::filterGroups);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,                this, &MeasurementDatabaseDialog::showDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MeasurementDatabaseDialog::treeContextMenu);
    connect(ui->treeWidget, &QTreeWidget::itemActivated,              this, &MeasurementDatabaseDialog::showDescription);

    readSettings();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ~MeasurementDatabaseDialog Destroys the dialog instance and frees allocated interface memory.
///
/// Triggers the geometry serialization routine to write the window's current layout dimensions to the
/// persistent system registry, then deallocates the user interface component tree to prevent memory leaks.
//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::~MeasurementDatabaseDialog()
{
    writeSettings();
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief getNewMeasurementNames Retrieves the list of newly selected measurement identifiers.
///
/// Returns a collection of unique internal data tracking keys representing the items that have been
/// checked by the user during the current interaction session.
///
/// @return QStringList A list containing the newly committed measurement token names.
//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDatabaseDialog::getNewMeasurementNames() const
{
    return m_newMeasurements;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief retranslateGroups Synchronizes language localized strings across all primary categories and sub-items.
///
/// Iterates sequentially through alphabetical structural groups A to Q to apply updated, translated titles
/// and item descriptions during an application language transition. It concludes by re-rendering the active
/// row's diagram preview and text description layers to match the new localization settings.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateGroups()
{
    retranslateGroup(m_groupA, groupAText, ListGroupA());
    retranslateGroup(m_groupB, groupBText, ListGroupB());
    retranslateGroup(m_groupC, groupCText, ListGroupC());
    retranslateGroup(m_groupD, groupDText, ListGroupD());
    retranslateGroup(m_groupE, groupEText, ListGroupE());
    retranslateGroup(m_groupF, groupFText, ListGroupF());
    retranslateGroup(m_groupG, groupGText, ListGroupG());
    retranslateGroup(m_groupH, groupHText, ListGroupH());
    retranslateGroup(m_groupI, groupIText, ListGroupI());
    retranslateGroup(m_groupJ, groupJText, ListGroupJ());
    retranslateGroup(m_groupK, groupKText, ListGroupK());
    retranslateGroup(m_groupL, groupLText, ListGroupL());
    retranslateGroup(m_groupM, groupMText, ListGroupM());
    retranslateGroup(m_groupN, groupNText, ListGroupN());
    retranslateGroup(m_groupO, groupOText, ListGroupO());
    retranslateGroup(m_groupP, groupPText, ListGroupP());
    retranslateGroup(m_groupQ, groupQText, ListGroupQ());

    showDescription(ui->treeWidget->currentItem(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief changeEvent Intercepts system and application state changes to update the interface runtime.
///
/// Monitors changes to the application environment, specifically handling runtime language mutations.
/// When a dynamic LanguageChange event is received, it triggers the user interface translation layout
/// routines to re-localize text strings before forwarding the event processing back to the base class.
///
/// @param event The event structure detailing the state change being dispatched.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief eventFilter Monitors keyboard navigation and widget container resizing events.
///
/// Intercepts directional up/down arrow key presses within the measurement selection tree widget to
/// synchronize and trigger preview updates immediately before selection transitions finalize. It also
/// intercepts geometric changes to the graphic frame container to force real-time vector image scaling.
///
/// @param object The object currently being monitored by the filter layout.
/// @param event The layout or input event structure being dispatched.
/// @return bool True if the event was handled and consumed; otherwise returns the base class result.
//---------------------------------------------------------------------------------------------------------------------
bool MeasurementDatabaseDialog::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->treeWidget && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch(keyEvent->key())
        {
            case Qt::Key_Up:
            {
                const QModelIndex model = ui->treeWidget->indexAbove(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemAbove(ui->treeWidget->currentItem());
                showDescription(item, model.column());
                break;
            }
            case Qt::Key_Down:
            {
                const QModelIndex model = ui->treeWidget->indexBelow(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemBelow(ui->treeWidget->currentItem());
                showDescription(item, model.column());
                break;
            }
            default:
                break;
        }
    }
    else if (object == ui->diagram_Frame && event->type() == QEvent::Resize)
    {
        renderScaledDiagram();
    }
    return QDialog::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief updateChecks Synchronizes multi-state checkbox flags recursively through the database tree.
///
/// Evaluates user modifications to tree item selection states. When a parent category checkbox state changes,
/// it automatically ripples those assignments down to uncommitted child leaves. For individual leaf item modifications,
/// it commits or rolls back entries from tracking data arrays and recursively updates parent branches to
/// Checked, Unchecked, or PartiallyChecked layout flags.
///
/// @param item The tree widget item containing a selection state that needs tracking or propagation.
/// @param column The zero-based column position, or a fallback flag (-1) indicating internal recursive execution.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::updateChecks(QTreeWidgetItem *item, int column)
{
    bool diff = false;
    if (column != 0 && column != -1)
    {
        return;
    }

    if (item->childCount() != 0 && item->checkState(0) != Qt::PartiallyChecked && column != -1)
    {
        bool flag = false; // Check if we could change atleast one children
        Qt::CheckState state = item->checkState(0);
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (!m_measurements.contains(item->child(i)->data(0, Qt::UserRole).toString()))
            {
                item->child(i)->setCheckState(0, state);
                flag = true;
            }

            if (flag == false) // All child in the list
            {
                item->setCheckState(0, Qt::Checked);
            }
        }
    }
    else if (item->childCount() == 0 || column == -1)
    {

        QString itemName = item->data(0, Qt::UserRole).toString();
        Qt::CheckState state = item->checkState(0);

        if (state == Qt::CheckState::Unchecked)
        {
            m_newMeasurements.removeOne(itemName);
        }
        else if (state == Qt::CheckState::Checked &&
                 !m_measurements.contains(itemName) &&
                 !m_newMeasurements.contains(itemName) &&
                 !itemName.isEmpty())
        {
            m_newMeasurements.append(itemName);
        }

        QTreeWidgetItem *parent = item->parent();
        if (parent == nullptr)
        {
            return;
        }
        for (int j = 0; j < parent->childCount(); ++j)
        {
            if (j != parent->indexOfChild(item)
                    && item->checkState(0) != parent->child(j)->checkState(0))
            {
                diff = true;
            }
        }
        if (diff)
        {
            parent->setCheckState(0, Qt::PartiallyChecked);
        }
        else
        {
            parent->setCheckState(0, item->checkState(0));
        }

        updateChecks(parent, -1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showDescription Processes tree item selection changes to load and display the matching measurement diagram.
///
/// Validates the active tree item and column selection index to ensure only valid, childless leaf nodes
/// are processed. For qualified selections, it extracts the underlying measurement data string, maps it
/// to its corresponding vector graphic index, and caches the layout path state. It then triggers an initial
/// high-resolution rendering pass or falls back to a warning view if the index is unknown.
///
/// @param item The tree widget item row that was selected or clicked.
/// @param column The zero-based column position where the selection event occurred.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::showDescription(QTreeWidgetItem *item, int column)
{
    if ((column != 0 && column != -1) || item == nullptr || (item->childCount() != 0 ))
    {
        ui->diagram_Label->clear();
        return;
    }

    const QString name        = item->data(0, Qt::UserRole).toString();
    const VTranslateVars *trv = qApp->translateVariables();
    const QString number      = trv->MNumber(name);
    const QString description = trv->Description(name);

    // Clear variables so resizes don't draw a stale image
    m_currentSvgPath.clear();
    m_currentNumber.clear();
    m_currentName.clear();
    m_currentDescription.clear();

    // Save the current states for the resize event to look at later
    m_currentSvgPath     = QString("://diagrams/%1.svg").arg(MapDiagrams(trv, number));
    m_currentNumber      = number;
    m_currentName        = trv->guiText(name);
    m_currentDescription = description;

    // Execute the initial draw pass using the live frame size
    renderScaledDiagram();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief treeContextMenu Instantiates and executes a contextual right-click menu for the measurement tree.
///
/// Automatically forces a description and diagram preview update for the currently right-clicked tree item.
/// It constructs dynamic menu actions for bulk expansion and collapse operations. If selection mode is active,
/// it evaluates the overall checklist footprint to dynamically append a toggle action for mass checking
/// or unchecking before displaying the menu at the cursor's global screen coordinates.
///
/// @param pos The local mouse coordinates where the context menu event was triggered.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::treeContextMenu(const QPoint &pos)
{
    // Because item also will be selected need to show description
    const QModelIndex model = ui->treeWidget->currentIndex();
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    showDescription(item, model.column());

    QAction *actionCollapseAll = new QAction(tr("Collapse All"), this);
    connect(actionCollapseAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::collapseAll);

    QAction *actionExpandeAll = new QAction(tr("Expand All"), this);
    connect(actionExpandeAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::expandAll);

    QMenu menu(this);
    menu.addAction(actionCollapseAll);
    menu.addAction(actionExpandeAll);

    if (m_selectMode)
    {
        QString actionName;
        globalCheckState() == Qt::Checked ? actionName = tr("Check all") : actionName = tr("Uncheck all");

        QAction *actionRecheckState = new QAction(actionName, this);
        connect(actionRecheckState, &QAction::triggered, this, &MeasurementDatabaseDialog::recheckState);

        menu.addAction(actionRecheckState);
    }
    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief recheckState Mass toggles checkbox selection states across all primary database group hierarchies.
///
/// Evaluates whether the dialog is executing under a valid selection layout mode. When confirmed, it calculates
/// the target inverse state via the global selection profile and sequentially updates the check flags across
/// alphabetical structural groups A through Q.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::recheckState()
{
    if (m_selectMode)
    {
        const Qt::CheckState check = globalCheckState();

        changeCheckState(m_groupA, check);
        changeCheckState(m_groupB, check);
        changeCheckState(m_groupC, check);
        changeCheckState(m_groupD, check);
        changeCheckState(m_groupE, check);
        changeCheckState(m_groupF, check);
        changeCheckState(m_groupG, check);
        changeCheckState(m_groupH, check);
        changeCheckState(m_groupI, check);
        changeCheckState(m_groupJ, check);
        changeCheckState(m_groupK, check);
        changeCheckState(m_groupL, check);
        changeCheckState(m_groupM, check);
        changeCheckState(m_groupN, check);
        changeCheckState(m_groupO, check);
        changeCheckState(m_groupP, check);
        changeCheckState(m_groupQ, check);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief initDataBase Dispatches system configuration vectors to build the database tree layout categories.
///
/// Coordinates the initial batch construction routines for the underlying structural tree. It maps individual
/// category identifiers, localized text descriptions, and raw measurements lists across alphabetic groupings
/// A to Q to register and render node elements cleanly.
///
/// @param list The primary dataset array containing raw string keys to populate and evaluate tree checkboxes.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::initDataBase(const QStringList &list)
{
    initGroup(&m_groupA, groupAText, ListGroupA(), list);
    initGroup(&m_groupB, groupBText, ListGroupB(), list);
    initGroup(&m_groupC, groupCText, ListGroupC(), list);
    initGroup(&m_groupD, groupDText, ListGroupD(), list);
    initGroup(&m_groupE, groupEText, ListGroupE(), list);
    initGroup(&m_groupF, groupFText, ListGroupF(), list);
    initGroup(&m_groupG, groupGText, ListGroupG(), list);
    initGroup(&m_groupH, groupHText, ListGroupH(), list);
    initGroup(&m_groupI, groupIText, ListGroupI(), list);
    initGroup(&m_groupJ, groupJText, ListGroupJ(), list);
    initGroup(&m_groupK, groupKText, ListGroupK(), list);
    initGroup(&m_groupL, groupLText, ListGroupL(), list);
    initGroup(&m_groupM, groupMText, ListGroupM(), list);
    initGroup(&m_groupN, groupNText, ListGroupN(), list);
    initGroup(&m_groupO, groupOText, ListGroupO(), list);
    initGroup(&m_groupP, groupPText, ListGroupP(), list);
    initGroup(&m_groupQ, groupQText, ListGroupQ(), list);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief initGroup Orchestrates the layout construction for a top-level category node and its children.
///
/// Instantiates a primary structural category group within the tree layout, then loops through the
/// provided sub-category key list to register and append individual leaf child nodes under the parent block.
///
/// @param group A double pointer used to assign the newly allocated top-level tree widget item.
/// @param groupName The visible, localized text title given to the main category folder.
/// @param groupList A string list containing the internal data identifier keys for this category's children.
/// @param measurementList A master checklist filter array used to evaluate initial leaf node checkbox states.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::initGroup(QTreeWidgetItem **group, const QString &groupName,
                                          const QStringList &groupList, const QStringList &measurementList)
{
    *group = addMeasurementGroup(groupName);
    for (int i=0; i < groupList.size(); ++i)
    {
        addMeasurement(*group, groupList.at(i), measurementList);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief addMeasurementGroup Allocates and stylizes a primary top-level folder element inside the tree layout.
///
/// Spawns a new parent item anchored to the main tree widget frame. It configures basic tooltips, display text,
/// and forces a default expanded state. If selection mode is active, it injects checkbox interactive capability,
/// sets a baseline unchecked state, and shades the row background using a light gray tint.
///
/// @param groupName The visible, localized text header applied directly to the top-level row.
/// @return QTreeWidgetItem* A pointer to the newly allocated and configured parent folder item.
//---------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem *MeasurementDatabaseDialog::addMeasurementGroup(const QString &groupName)
{
    QTreeWidgetItem *group = new QTreeWidgetItem(ui->treeWidget);
    group->setText(0, groupName);
    group->setToolTip(0, groupName);
    group->setExpanded(true);
    if (m_selectMode)
    {
        group->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        group->setCheckState(0, Qt::Unchecked);
        group->setBackground(0, QBrush(Qt::lightGray));
        group->setForeground(0, QBrush(Qt::black));
    }
    return group;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief addMeasurement Compiles database tokens to allocate, format, and filter a child tree node.
///
/// Asserts that a valid parent node exists, then appends a new child element directly under it. If operating
/// under selection mode rules, it references active array indexes to determine if the node is hard-locked, pre-checked,
/// or unchecked—updating its flags, background highlight masks, and recursively balancing parent checkboxes. It concludes
/// by translating internal data definitions into formatted user-facing measurement labels.
///
/// @param group A pointer to the direct parent category node where this item will reside.
/// @param name The primary database dictionary key identifier string for this specific metric.
/// @param list A verification dataset array used to dictate baseline checklist activations.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::addMeasurement(QTreeWidgetItem *group, const QString &name, const QStringList &list)
{
    SCASSERT(group != nullptr)

    QTreeWidgetItem *item = new QTreeWidgetItem(group);

    if (m_selectMode)
    {
        if (m_measurements.contains(name))
        {
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setCheckState(0, Qt::Checked);
            item->setBackground(0, QBrush(QColor(231, 231, 0, 127)));
        }
        else if (list.contains(name))
        {
            item->setCheckState(0, Qt::Checked);
        }
        else
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        updateChecks(item, 0);
    }

    const QString text = qApp->translateVariables()->MNumber(name) + ". " + qApp->translateVariables()->MToUser(name);
    item->setText(0, text);
    item->setToolTip(0, text);
    item->setData(0, Qt::UserRole, name);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief readSettings Pulls application metadata arrays to restore window positioning parameters.
///
/// Queries the global configuration registry using internal settings subroutines to extract and restore
/// the dialog's previous on-screen layout geometry, window state, and bounding coordinates.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::readSettings()
{
    restoreGeometry(qApp->seamlyMeSettings()->getDataBaseGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief writeSettings Serializes current interface coordinates directly to the persistent system registry.
///
/// Captures the dialog window's current geometric footprint, display size, and layout coordinates, writing
/// the serialized byte block directly into the application's global configuration file layer.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::writeSettings()
{
    qApp->seamlyMeSettings()->setDataBaseGeometry(saveGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief filterGroups Dispatches text filtering queries to evaluate node visibility across all tree categories.
///
/// Iterates sequentially through alphabetical structural groups A to Q to evaluate their child elements
/// against the provided text pattern. After refining row visibilities, it checks the active tree selection footprint,
/// automatically resetting the diagram preview container to a blank layout if the query hides the active row.
///
/// @param searchString The case-insensitive text fragment used to isolate matching tree rows.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::filterGroups(const QString searchString)
{
    filterGroup(m_groupA, searchString);
    filterGroup(m_groupB, searchString);
    filterGroup(m_groupC, searchString);
    filterGroup(m_groupD, searchString);
    filterGroup(m_groupE, searchString);
    filterGroup(m_groupF, searchString);
    filterGroup(m_groupG, searchString);
    filterGroup(m_groupH, searchString);
    filterGroup(m_groupI, searchString);
    filterGroup(m_groupJ, searchString);
    filterGroup(m_groupK, searchString);
    filterGroup(m_groupL, searchString);
    filterGroup(m_groupM, searchString);
    filterGroup(m_groupN, searchString);
    filterGroup(m_groupO, searchString);
    filterGroup(m_groupP, searchString);
    filterGroup(m_groupQ, searchString);

    const QList<QTreeWidgetItem *> selected = ui->treeWidget->selectedItems();
    selected.isEmpty() ? showDescription(nullptr, -1) : showDescription(selected.first(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief filterGroup Evaluates individual child items within a specified group against a search string.
///
/// Asserts that a valid target folder pointer exists, then loops through its direct leaf children to verify if their
/// display labels match the query block. Unmatched sub-elements are hidden from the tree view layout. It concludes
/// by hiding the entire category parent row if neither the folder name nor its contents fulfill the search pattern.
///
/// @param group A pointer to the primary structural folder item to evaluate and filter.
/// @param searchString The case-insensitive lookup text pattern passed down from the master search bar.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::filterGroup(QTreeWidgetItem *group, const QString &searchString)
{
    SCASSERT(group != nullptr)

    bool isMatch = false;
    for (int i=0; i < group->childCount(); ++i)
    {
        QTreeWidgetItem *item = group->child(i);
        const bool isHidden = !item->text(0).contains(searchString, Qt::CaseInsensitive);

        item->setHidden(isHidden);
        if (!isHidden)
        {
            isMatch = true;
        }
    }

    group->setHidden(!group->text(0).contains(searchString, Qt::CaseInsensitive) && !isMatch);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief retranslateGroup Refreshes localized string values across a master folder header and its list children.
///
/// Updates the primary visibility string and tooltips on a parent category row during system language
/// transitions. It then loops sequentially through the provided sub-category tracking tokens to synchronize and
/// re-translate every individual child node item residing underneath the group.
///
/// @param group A pointer to the core category layout node needing language synchronization.
/// @param groupName The fresh, newly translated text phrase assigned to the main folder header.
/// @param list An array of internal database keys mapping to the specific indices of the child rows.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateGroup(QTreeWidgetItem *group, const QString &groupName,
                                                 const QStringList &list)
{
    group->setText(0, groupName);
    group->setToolTip(0, groupName);

    for (int i=0; i<list.size(); ++i)
    {
        retranslateMeasurement(group, i, list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief retranslateMeasurement Formats and updates localized display labels on an isolated child leaf node.
///
/// Dispatches the internal token name to application layout variables to generate a freshly synchronized user string
/// combining translation coordinates and metric codes. It then target the child element at the specified index position
/// to overwrite its visible labels and tooltip containers.
///
/// @param group A pointer to the parent category widget item holding the target child row.
/// @param index The zero-based array index identifying the child node's location inside the sub-tree block.
/// @param name The core configuration token key used to compute the fresh localized output labels.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name)
{
    const QString text = qApp->translateVariables()->MNumber(name) + ". " + qApp->translateVariables()->MToUser(name);

    QTreeWidgetItem *item = group->child(index);
    item->setText(0, text);
    item->setToolTip(0, text);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief changeCheckState Adjusts choice check configurations across a target folder widget row container.
///
/// Asserts that a valid structural tree node item exists, then updates its index selection flag layer to match
/// the incoming multi-state check properties.
///
/// @param group A pointer to the core parent category row targeted for a state update.
/// @param state The target state option assignment (Checked, Unchecked, or PartiallyChecked) to apply.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::changeCheckState(QTreeWidgetItem *group, Qt::CheckState state)
{
    SCASSERT(group != nullptr)
    group->setCheckState(0, state);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief globalCheckState Evaluates and returns the collective check state baseline across all database categories.
///
/// Asserts that all alphabetical structural group folders (A through Q) are validly allocated in memory.
/// If selection mode rules are active, it scans the state layer of every parent node container. It returns
/// Qt::Checked if every group is currently unchecked, returns Qt::Unchecked if every group is currently checked,
/// and defaults to returning Qt::Checked as a fallback toggle key if a split state exists.
///
/// @return Qt::CheckState The calculated collective check state value to determine mass selection overrides.
//---------------------------------------------------------------------------------------------------------------------
Qt::CheckState MeasurementDatabaseDialog::globalCheckState() const
{
    SCASSERT(m_groupA != nullptr)
    SCASSERT(m_groupB != nullptr)
    SCASSERT(m_groupC != nullptr)
    SCASSERT(m_groupD != nullptr)
    SCASSERT(m_groupE != nullptr)
    SCASSERT(m_groupF != nullptr)
    SCASSERT(m_groupG != nullptr)
    SCASSERT(m_groupH != nullptr)
    SCASSERT(m_groupI != nullptr)
    SCASSERT(m_groupJ != nullptr)
    SCASSERT(m_groupK != nullptr)
    SCASSERT(m_groupL != nullptr)
    SCASSERT(m_groupM != nullptr)
    SCASSERT(m_groupN != nullptr)
    SCASSERT(m_groupO != nullptr)
    SCASSERT(m_groupP != nullptr)
    SCASSERT(m_groupQ != nullptr)

    if (m_selectMode)
    {
        if (m_groupA->checkState(0) == Qt::Unchecked &&
            m_groupB->checkState(0) == Qt::Unchecked &&
            m_groupC->checkState(0) == Qt::Unchecked &&
            m_groupD->checkState(0) == Qt::Unchecked &&
            m_groupE->checkState(0) == Qt::Unchecked &&
            m_groupF->checkState(0) == Qt::Unchecked &&
            m_groupG->checkState(0) == Qt::Unchecked &&
            m_groupH->checkState(0) == Qt::Unchecked &&
            m_groupI->checkState(0) == Qt::Unchecked &&
            m_groupJ->checkState(0) == Qt::Unchecked &&
            m_groupK->checkState(0) == Qt::Unchecked &&
            m_groupL->checkState(0) == Qt::Unchecked &&
            m_groupM->checkState(0) == Qt::Unchecked &&
            m_groupN->checkState(0) == Qt::Unchecked &&
            m_groupO->checkState(0) == Qt::Unchecked &&
            m_groupP->checkState(0) == Qt::Unchecked &&
            m_groupQ->checkState(0) == Qt::Unchecked)
        {
            return Qt::Checked;
        }
        else if (m_groupA->checkState(0) == Qt::Checked &&
                 m_groupB->checkState(0) == Qt::Checked &&
                 m_groupC->checkState(0) == Qt::Checked &&
                 m_groupD->checkState(0) == Qt::Checked &&
                 m_groupE->checkState(0) == Qt::Checked &&
                 m_groupF->checkState(0) == Qt::Checked &&
                 m_groupG->checkState(0) == Qt::Checked &&
                 m_groupH->checkState(0) == Qt::Checked &&
                 m_groupI->checkState(0) == Qt::Checked &&
                 m_groupJ->checkState(0) == Qt::Checked &&
                 m_groupK->checkState(0) == Qt::Checked &&
                 m_groupL->checkState(0) == Qt::Checked &&
                 m_groupM->checkState(0) == Qt::Checked &&
                 m_groupN->checkState(0) == Qt::Checked &&
                 m_groupO->checkState(0) == Qt::Checked &&
                 m_groupP->checkState(0) == Qt::Checked &&
                 m_groupQ->checkState(0) == Qt::Checked)
        {
            return Qt::Unchecked;
        }
        return Qt::Checked;
    }

    return Qt::Unchecked;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief renderScaledDiagram Generates a pixel-perfect, crisp vector diagram to fit the current frame size.
///
/// Loads the active SVG file path from memory and dynamically reads the real-time dimensions of the
/// main window's frame container. It compares horizontal and vertical scaling factors to determine the
/// maximum aspect-ratio fit, preventing layout bleeding. The mathematical vector paths are then rendered
/// onto a sharp, transparent canvas layout, updating the display widget alongside its text caption label.
//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::renderScaledDiagram()
{
    if (m_currentSvgPath.isEmpty()) return;

    QSvgRenderer renderer(m_currentSvgPath);
    QSize nativeSize = renderer.defaultSize();

    if (nativeSize.isValid() && nativeSize.width() > 0 && nativeSize.height() > 0)
    {
        // Read from the live frame container dimensions. Falling back to safe defaults if frame hasn't painted yet.
        double maxW = (ui->diagram_Frame->width() > 0)  ? (ui->diagram_Frame->width() - 10.0) : 290.0;
        double maxH = (ui->diagram_Frame->height() > 0) ? (ui->diagram_Frame->height() - 120.0) : 500.0;

        double ratioW = maxW / nativeSize.width();
        double ratioH = maxH / nativeSize.height();
        double scaleFactor = (ratioW < ratioH) ? ratioW : ratioH;

        int targetWidth = static_cast<int>(nativeSize.width() * scaleFactor);
        int targetHeight = static_cast<int>(nativeSize.height() * scaleFactor);

        QPixmap pixmap(targetWidth, targetHeight);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        renderer.render(&painter);
        painter.end();

        ui->diagram_Label->setPixmap(pixmap);
    }

    ui->caption_Label->setText(QString("<b>%1</b>. <i>%2</i>").arg(m_currentNumber, m_currentName));
    ui->description_Label->setText(m_currentDescription);
}

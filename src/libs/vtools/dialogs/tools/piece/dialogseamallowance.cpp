/************************************************************************
 **
 **  @file   dialogseamallowance.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Seamly2D project
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

#include "dialogseamallowance.h"
#include "ui_dialogseamallowance.h"
#include "ui_tabpaths.h"
#include "ui_tablabels.h"
#include "ui_tabgrainline.h"
#include "ui_tabanchors.h"
#include "ui_tabnotches.h"
#include "../vwidgets/fancytabbar/fancytabbar.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/calculator.h"
#include "visualization/path/vistoolpiece.h"
#include "visualization/path/pieceanchorpoint_visual.h"
#include "dialoginternalpath.h"
#include "../../../undocommands/savepiecepathoptions.h"
#include "../../support/dialogeditwrongformula.h"
#include "../../support/dialogeditlabel.h"
#include "../../../tools/vtoolseamallowance.h"

#include <QMenu>
#include <QTimer>
#include <QtNumeric>

enum TabOrder {Paths=0, AnchorPoints=1, Labels=2, Grainline=3, Notches=4, Count=5};

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void EnableDefButton(QPushButton *defButton, const QString &formula)
{
    SCASSERT(defButton != nullptr)

    if (formula != currentSeamAllowance)
    {
        defButton->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString GetFormulaFromUser(QPlainTextEdit *textEdit)
{
    SCASSERT(textEdit != nullptr)

    QString formula = textEdit->toPlainText();
    formula.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
}
}

//---------------------------------------------------------------------------------------------------------------------
DialogSeamAllowance::DialogSeamAllowance(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogSeamAllowance),
      uiPathsTab(new Ui::PathsTab),
      uiLabelsTab(new Ui::LabelsTab),
      uiGrainlineTab(new Ui::GrainlineTab),
      uiAnchorPointsTab(new Ui::AnchorPointsTab),
      uiNotchesTab(new Ui::NotchesTab),
      m_pathsTab(new QWidget),
      m_labelsTab(new QWidget),
      m_grainlineTab(new QWidget),
      m_anchorPointsTab(new QWidget),
      m_notchesTab(new QWidget),
      m_ftb(new FancyTabBar(FancyTabBar::Left, this)),
      applyAllowed(false),// By default disabled
      flagGrainlineAnchor(true),
      flagPieceLabelAnchor(true),
      flagPatternLabelAnchor(true),
      flagGrainlineFormula(true),
      flagPieceLabelAngle(true),
      flagPieceLabelFormula(true),
      flagPatternLabelAngle(true),
      flagPatternLabelFormula(true),
      flagBeforeFormula(true),
      flagAfterFormula(true),
      flagMainPath(true),
      m_bAddMode(true),
      m_mx(0),
      m_my(0),
      m_dialog(),
      m_anchorPoints(),
      m_oldData(),
      m_oldGeom(),
      m_oldGrainline(),
      m_iRotBaseHeight(0),
      m_iLenBaseHeight(0),
      m_dLabelWidthBaseHeight(0),
      m_dLabelHeightBaseHeight(0),
      m_dLabelAngleBaseHeight(0),
      m_pLabelWidthBaseHeight(0),
      m_pLabelHeightBaseHeight(0),
      m_pLabelAngleBaseHeight(0),
      m_widthFormula(0),
      m_beforeWidthFormula(0),
      m_afterWidthFormula(0),
      m_timerWidth(nullptr),
      m_timerWidthBefore(nullptr),
      m_timerWidthAfter(nullptr),
      m_saWidth(0),
      m_templateLines()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/new_detail.png"));

    InitOkCancelApply(ui);
    InitFancyTabBar();
    EnableApply(applyAllowed);

    InitMainPathTab();
    InitSeamAllowanceTab();
    InitInternalPathsTab();
    InitPatternPieceDataTab();
    InitLabelsTab();
    InitGrainlineTab();
    initializeAnchorsTab();
    InitNotchesTab();

    flagName = true;
    ChangeColor(uiLabelsTab->labelEditName, okColor);
    flagMainPath = MainPathIsValid();
    CheckState();

    if (!applyAllowed && vis.isNull())
    {
        vis = new VisToolPiece(data);
    }

    m_ftb->SetCurrentIndex(TabOrder::Paths); // Always make first tab active on start.
}

//---------------------------------------------------------------------------------------------------------------------
DialogSeamAllowance::~DialogSeamAllowance()
{
    delete m_anchorPoints;
    delete m_pathsTab;
    delete m_anchorPointsTab;
    delete m_grainlineTab;
    delete m_labelsTab;
    delete m_notchesTab;
    delete uiNotchesTab;
    delete uiAnchorPointsTab;
    delete uiGrainlineTab;
    delete uiLabelsTab;
    delete uiPathsTab;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableApply(bool enable)
{
    SCASSERT(apply_Button != nullptr);
    apply_Button->setEnabled(enable);
    applyAllowed = enable;

    uiPathsTab->seamAllowance_Tab->setEnabled(applyAllowed);
    uiPathsTab->internalPaths_Tab->setEnabled(applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::AnchorPoints, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Labels, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Grainline, applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Notches, applyAllowed);
}

//---------------------------------------------------------------------------------------------------------------------
VPiece DialogSeamAllowance::GetPiece() const
{
    return CreatePiece();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPiece(const VPiece &piece)
{
    uiPathsTab->mainPath_ListWidget->clear();
    for (int i = 0; i < piece.GetPath().CountNodes(); ++i)
    {
        NewMainPathItem(piece.GetPath().at(i));
    }

    uiPathsTab->hideSeamLine_CheckBox->setChecked(piece.isHideSeamLine());
    uiPathsTab->customSeamAllowance_ListWidget->blockSignals(true);
    uiPathsTab->customSeamAllowance_ListWidget->clear();
    for (int i = 0; i < piece.GetCustomSARecords().size(); ++i)
    {
        NewCustomSA(piece.GetCustomSARecords().at(i));
    }
    uiPathsTab->customSeamAllowance_ListWidget->blockSignals(false);

    uiPathsTab->internalPaths_ListWidget->clear();
    for (int i = 0; i < piece.GetInternalPaths().size(); ++i)
    {
        NewInternalPath(piece.GetInternalPaths().at(i));
    }

    uiAnchorPointsTab->anchorPoints_ListWidget->clear();
    for (int i = 0; i < piece.getAnchors().size(); ++i)
    {
        newAnchorPoint(piece.getAnchors().at(i));
    }

    initAnchorPointComboboxes();

    uiPathsTab->startPoint_ComboBox->blockSignals(true);
    uiPathsTab->startPoint_ComboBox->clear();
    uiPathsTab->startPoint_ComboBox->blockSignals(false);

    uiPathsTab->endPoint_ComboBox->blockSignals(true);
    uiPathsTab->endPoint_ComboBox->clear();
    uiPathsTab->endPoint_ComboBox->blockSignals(false);

    customSeamAllowanceChanged(0);

    uiPathsTab->forbidFlipping_CheckBox->setChecked(piece.IsForbidFlipping());
    uiPathsTab->seams_CheckBox->setChecked(piece.IsSeamAllowance());
    uiPathsTab->builtIn_CheckBox->setChecked(piece.IsSeamAllowanceBuiltIn());
    uiLabelsTab->detailName_LineEdit->setText(piece.GetName());

    const QString width = qApp->TrVars()->FormulaToUser(piece.GetFormulaSAWidth(), qApp->Settings()->GetOsSeparator());
    uiPathsTab->widthFormula_PlainTextEdit->setPlainText(width);
    m_saWidth = piece.GetSAWidth();

    m_mx = piece.GetMx();
    m_my = piece.GetMy();

    m_oldData = piece.GetPatternPieceData();
    uiLabelsTab->letter_LineEdit->setText(m_oldData.GetLetter());
    uiLabelsTab->annotation_LineEdit->setText(m_oldData.GetAnnotation());
    uiLabelsTab->orientation_LineEdit->setText(m_oldData.GetOrientation());
    uiLabelsTab->rotation_LineEdit->setText(m_oldData.GetRotationWay());
    uiLabelsTab->tilt_LineEdit->setText(m_oldData.GetTilt());
    uiLabelsTab->foldPosition_LineEdit->setText(m_oldData.GetFoldPosition());
    uiLabelsTab->quantity_SpinBox->setValue(m_oldData.GetQuantity());
    uiLabelsTab->fold_CheckBox->setChecked(m_oldData.IsOnFold());
    m_templateLines = m_oldData.GetLabelTemplate();

    uiGrainlineTab->arrow_ComboBox->setCurrentIndex(int(piece.GetGrainlineGeometry().GetArrowType()));

    uiLabelsTab->pieceLabel_GroupBox->setChecked(m_oldData.IsVisible());
    ChangeCurrentData(uiLabelsTab->dLabelCenterAnchor_ComboBox, m_oldData.centerAnchorPoint());
    ChangeCurrentData(uiLabelsTab->dLabelTopLeftAnchor_ComboBox, m_oldData.topLeftAnchorPoint());
    ChangeCurrentData(uiLabelsTab->dLabelBottomRightAnchor_ComboBox, m_oldData.bottomRightAnchorPoint());
    SetDLWidth(m_oldData.GetLabelWidth());
    SetDLHeight(m_oldData.GetLabelHeight());
    SetDLAngle(m_oldData.GetRotation());

    m_oldGeom = piece.GetPatternInfo();
    uiLabelsTab->patternLabel_GroupBox->setChecked(m_oldGeom.IsVisible());
    ChangeCurrentData(uiLabelsTab->pLabelCenterAnchor_ComboBox, m_oldGeom.centerAnchorPoint());
    ChangeCurrentData(uiLabelsTab->pLabelTopLeftAnchor_ComboBox, m_oldGeom.topLeftAnchorPoint());
    ChangeCurrentData(uiLabelsTab->pLabelBottomRightAnchor_ComboBox, m_oldGeom.bottomRightAnchorPoint());
    SetPLWidth(m_oldGeom.GetLabelWidth());
    SetPLHeight(m_oldGeom.GetLabelHeight());
    SetPLAngle(m_oldGeom.GetRotation());

    m_oldGrainline = piece.GetGrainlineGeometry();
    uiGrainlineTab->grainline_GroupBox->setChecked(m_oldGrainline.IsVisible());
    ChangeCurrentData(uiGrainlineTab->centerGrainlineAnchor_ComboBox, m_oldGrainline.centerAnchorPoint());
    ChangeCurrentData(uiGrainlineTab->topGrainlineAnchor_ComboBox, m_oldGrainline.topAnchorPoint());
    ChangeCurrentData(uiGrainlineTab->bottomGrainlineAnchor_ComboBox, m_oldGrainline.bottomAnchorPoint());
    SetGrainlineAngle(m_oldGrainline.GetRotation());
    SetGrainlineLength(m_oldGrainline.GetLength());

    ValidObjects(MainPathIsValid());
    EnabledGrainline();
    EnabledDetailLabel();
    EnabledPatternLabel();

    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChosenObject gets id and type of selected object. Save correct data and ignore wrong.
 * @param id id of objects (points, arcs, splines, spline paths)
 * @param type type of object
 */
void DialogSeamAllowance::ChosenObject(quint32 id, const SceneObject &type)
{
    if (!prepare)
    {
        bool reverse = false;
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            reverse = true;
        }
        if (id != GetLastId())
        {
            switch (type)
            {
                case SceneObject::Arc:
                    NewMainPathItem(VPieceNode(id, Tool::NodeArc, reverse));
                    break;
                case SceneObject::ElArc:
                    NewMainPathItem(VPieceNode(id, Tool::NodeElArc, reverse));
                    break;
                case SceneObject::Point:
                    NewMainPathItem(VPieceNode(id, Tool::NodePoint));
                    break;
                case SceneObject::Spline:
                    NewMainPathItem(VPieceNode(id, Tool::NodeSpline, reverse));
                    break;
                case SceneObject::SplinePath:
                    NewMainPathItem(VPieceNode(id, Tool::NodeSplinePath, reverse));
                    break;
                case (SceneObject::Line):
                case (SceneObject::Detail):
                case (SceneObject::Unknown):
                default:
                    qDebug() << "Got wrong scene object. Ignore.";
                    break;
            }
        }
        else
        {
            if (uiPathsTab->mainPath_ListWidget->count() > 1)
            {
                delete GetItemById(id);
            }
        }

        ValidObjects(MainPathIsValid());

        if (!applyAllowed)
        {
            auto visPath = qobject_cast<VisToolPiece *>(vis);
            SCASSERT(visPath != nullptr);
            const VPiece p = CreatePiece();
            visPath->SetPiece(p);

            if (p.GetPath().CountNodes() == 1)
            {
                emit ToolTip(tr("Select main path objects clockwise, Use <b>SHIFT</b> to reverse curve direction, "
                                "Press <b>ENTER</b> to finish piece creation "));

                if (!qApp->getCurrentScene()->items().contains(visPath))
                {
                    visPath->VisualMode(NULL_ID);
                }
                else
                {
                    visPath->RefreshGeometry();
                }
            }
            else
            {
                visPath->RefreshGeometry();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowDialog(bool click)
{
    if (click == false)
    {
        emit ToolTip("");
        prepare = true;

        if (!applyAllowed)
        {
            auto visPath = qobject_cast<VisToolPiece *>(vis);
            SCASSERT(visPath != nullptr);
            visPath->SetMode(Mode::Show);
            visPath->RefreshGeometry();
        }

        // Fix issue #526. Dialog Detail is not on top after selection second object on Mac.
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SaveData()
{}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CheckState()
{
    SCASSERT(ok_Button != nullptr);
    ok_Button->setEnabled(flagName
                    && flagFormula
                    && flagMainPath
                    && (flagGrainlineFormula || flagGrainlineAnchor)
                    && flagPieceLabelAngle && (flagPieceLabelFormula || flagPieceLabelAnchor)
                    && flagPatternLabelAngle && (flagPatternLabelFormula || flagPatternLabelAnchor));

    if (applyAllowed)
    {
        ok_Button->setEnabled(flagBeforeFormula && flagAfterFormula);
        // In case dialog does not have an apply button
        if (apply_Button != nullptr)
        {
            apply_Button->setEnabled(ok_Button->isEnabled());
        }
    }

    if (flagMainPath)
    {
        m_ftb->SetTabText(TabOrder::Paths, tr("Paths"));
        QString tooltip = tr("Ready!");
        if (!applyAllowed)
        {
            tooltip = tooltip + QLatin1String("  <b>") +
                    tr("Press OK to create pattern piece") + QLatin1String("</b>");
        }
        uiPathsTab->status_Label->setText(tooltip);
    }
    else
    {
        m_ftb->SetTabText(TabOrder::Paths, tr("Paths") + QLatin1String("*"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::closeEvent(QCloseEvent *event)
{
    uiPathsTab->widthFormula_PlainTextEdit->blockSignals(true);
    uiPathsTab->beforeWidthFormula_PlainTextEdit->blockSignals(true);
    uiPathsTab->afterWidthFormula_PlainTextEdit->blockSignals(true);
    uiGrainlineTab->rotationFormula_LineEdit->blockSignals(true);
    uiGrainlineTab->lengthFormula_LineEdit->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showEvent(QShowEvent *event)
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

    const QSize size = qApp->Settings()->GetToolSeamAllowanceDialogSize();
    if (!size.isEmpty())
    {
        resize(size);
    }

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::resizeEvent(QResizeEvent *event)
{
    // Remember the size for the next time this dialog is opened, but only
    // if the dialog was already initialized.
    if (isInitialized)
    {
        qApp->Settings()->SetToolSeamAllowanceDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NameDetailChanged()
{
    QLineEdit *name = qobject_cast<QLineEdit*>(sender());
    if (name)
    {
        if (name->text().isEmpty())
        {
            flagName = false;
            ChangeColor(uiLabelsTab->labelEditName, Qt::red);
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
            QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->pieceLabelData_Tab), icon);
        }
        else
        {
            flagName = true;
            ChangeColor(uiLabelsTab->labelEditName, okColor);
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->pieceLabelData_Tab),
                                               QIcon());
        }
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowMainPathContextMenu(const QPoint &pos)
{
    const int row = uiPathsTab->mainPath_ListWidget->currentRow();
    if (uiPathsTab->mainPath_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->mainPath_ListWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    NodeInfo info;
    QListWidgetItem *rowItem = uiPathsTab->mainPath_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    QAction *actionNotch = nullptr;
    QAction *actionReverse = nullptr;
    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        actionReverse = menu->addAction(tr("Reverse"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());
    }
    else
    {
        actionNotch = menu->addAction(tr("Notch"));
        actionNotch->setCheckable(true);
        actionNotch->setChecked(rowNode.isNotch());
    }

    QAction *actionExcluded = menu->addAction(tr("Excluded"));
    actionExcluded->setCheckable(true);
    actionExcluded->setChecked(rowNode.isExcluded());

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(uiPathsTab->mainPath_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiPathsTab->mainPath_ListWidget->item(row);
    }
    else if (rowNode.GetTypeTool() != Tool::NodePoint && selectedAction == actionReverse)
    {
        rowNode.SetReverse(!rowNode.GetReverse());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
    else if (selectedAction == actionExcluded)
    {
        rowNode.SetExcluded(!rowNode.isExcluded());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
        rowItem->setFont(NodeFont(rowNode.isExcluded()));
    }
    else if (selectedAction == actionNotch)
    {
        rowNode.setNotch(!rowNode.isNotch());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }

    ValidObjects(MainPathIsValid());
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowCustomSAContextMenu(const QPoint &pos)
{
    const int row = uiPathsTab->customSeamAllowance_ListWidget->currentRow();
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionOption = menu->addAction(QIcon::fromTheme("preferences-other"), tr("Options"));

    QListWidgetItem *rowItem = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));

    QAction *actionReverse = menu->addAction(tr("Reverse"));
    actionReverse->setCheckable(true);
    actionReverse->setChecked(record.reverse);

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(uiPathsTab->customSeamAllowance_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiPathsTab->customSeamAllowance_ListWidget->item(row);
    }
    else if (selectedAction == actionReverse)
    {
        record.reverse = !record.reverse;
        rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
        rowItem->setText(GetPathName(record.path, record.reverse));
    }
    else if (selectedAction == actionOption)
    {
        auto *dialog = new DialogInternalPath(data, record.path, this);
        dialog->SetPiecePath(data->GetPiecePath(record.path));
        dialog->SetPieceId(toolId);
        if (record.includeType == PiecePathIncludeType::AsMainPath)
        {
            dialog->SetFormulaSAWidth(GetFormulaSAWidth());
        }
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &DialogSeamAllowance::PathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ShowInternalPathsContextMenu(const QPoint &pos)
{
    const int row = uiPathsTab->internalPaths_ListWidget->currentRow();
    if (uiPathsTab->internalPaths_ListWidget->count() == 0 || row == -1
            || row >= uiPathsTab->internalPaths_ListWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionOption = menu->addAction(QIcon::fromTheme("preferences-other"), tr("Options"));
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(uiPathsTab->internalPaths_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiPathsTab->internalPaths_ListWidget->item(row);
    }
    else if (selectedAction == actionOption)
    {
        QListWidgetItem *rowItem = uiPathsTab->internalPaths_ListWidget->item(row);
        SCASSERT(rowItem != nullptr);
        const quint32 pathId = qvariant_cast<quint32>(rowItem->data(Qt::UserRole));

        auto *dialog = new DialogInternalPath(data, pathId, this);
        dialog->SetPiecePath(data->GetPiecePath(pathId));
        dialog->SetPieceId(toolId);
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &DialogSeamAllowance::PathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showAnchorsContextMenu(const QPoint &pos)
{
    const int row = uiAnchorPointsTab->anchorPoints_ListWidget->currentRow();
    if (uiAnchorPointsTab->anchorPoints_ListWidget->count() == 0 || row == -1 || row >= uiAnchorPointsTab->anchorPoints_ListWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(uiAnchorPointsTab->anchorPoints_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiAnchorPointsTab->anchorPoints_ListWidget->item(row);
        FancyTabChanged(m_ftb->CurrentIndex());
        initAnchorPointComboboxes();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ListChanged()
{
    if (!applyAllowed)
    {
        auto visPath = qobject_cast<VisToolPiece *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPiece(CreatePiece());
        visPath->RefreshGeometry();
    }
    InitNodesList();
    InitNotchesList();
    customSeamAllowanceChanged(uiPathsTab->customSeamAllowance_ListWidget->currentRow());
    setMoveExclusions();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableSeamAllowance(bool enable)
{
    uiPathsTab->builtIn_CheckBox->setEnabled(enable);
    uiPathsTab->hideSeamLine_CheckBox->setEnabled(enable);

    if (!enable)
    {
        uiPathsTab->groupBoxAutomatic->setEnabled(enable);
        uiPathsTab->groupBoxCustom->setEnabled(enable);
    }
    else
    {
        uiPathsTab->builtIn_CheckBox->toggled(uiPathsTab->builtIn_CheckBox->isChecked());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::enableBuiltIn(bool enable)
{
    uiPathsTab->groupBoxAutomatic->setEnabled(!enable);
    uiPathsTab->groupBoxCustom->setEnabled(!enable);

    if (enable)
    {
        InitNodesList();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NodeChanged(int index)
{
    uiPathsTab->beforeWidthFormula_PlainTextEdit->setDisabled(true);
    uiPathsTab->beforeExpr_ToolButton->setDisabled(true);
    uiPathsTab->beforeDefault_PushButton->setDisabled(true);

    uiPathsTab->afterWidthFormula_PlainTextEdit->setDisabled(true);
    uiPathsTab->afterExpr_ToolButton->setDisabled(true);
    uiPathsTab->afterDefault_PushButton->setDisabled(true);

    uiPathsTab->angle_ComboBox->setDisabled(true);

    uiPathsTab->angle_ComboBox->blockSignals(true);

    if (index != -1)
    {
        const VPiece piece = CreatePiece();
        const int nodeIndex = piece.GetPath().indexOfNode(uiPathsTab->nodes_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = piece.GetPath().at(nodeIndex);

            // Seam allowance before nodw
            uiPathsTab->beforeWidthFormula_PlainTextEdit->setEnabled(true);
            uiPathsTab->beforeExpr_ToolButton->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            EnableDefButton(uiPathsTab->beforeDefault_PushButton, w1Formula);
            w1Formula = qApp->TrVars()->FormulaToUser(w1Formula, qApp->Settings()->GetOsSeparator());
            if (w1Formula.length() > 80)// increase height if needed.
            {
                this->DeployWidthBeforeFormulaTextEdit();
            }
            uiPathsTab->beforeWidthFormula_PlainTextEdit->setPlainText(w1Formula);
            MoveCursorToEnd(uiPathsTab->beforeWidthFormula_PlainTextEdit);

            // Seam allowance after node
            uiPathsTab->afterWidthFormula_PlainTextEdit->setEnabled(true);
            uiPathsTab->afterExpr_ToolButton->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            EnableDefButton(uiPathsTab->afterDefault_PushButton, w2Formula);
            w2Formula = qApp->TrVars()->FormulaToUser(w2Formula, qApp->Settings()->GetOsSeparator());
            if (w2Formula.length() > 80)// increase height if needed.
            {
                this->DeployWidthAfterFormulaTextEdit();
            }
            uiPathsTab->afterWidthFormula_PlainTextEdit->setPlainText(w2Formula);
            MoveCursorToEnd(uiPathsTab->afterWidthFormula_PlainTextEdit);

            // Angle type
            uiPathsTab->angle_ComboBox->setEnabled(true);
            const int index = uiPathsTab->angle_ComboBox->findData(static_cast<unsigned char>(node.GetAngleType()));
            if (index != -1)
            {
                uiPathsTab->angle_ComboBox->setCurrentIndex(index);
            }
        }
    }
    else
    {
        uiPathsTab->beforeWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
        uiPathsTab->afterWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
        uiPathsTab->angle_ComboBox->setCurrentIndex(-1);
    }
    uiPathsTab->angle_ComboBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchChanged(int index)
{
    uiNotchesTab->slitNotch_RadioButton->setDisabled(true);
    uiNotchesTab->tNotch_RadioButton->setDisabled(true);
    uiNotchesTab->uNotch_RadioButton->setDisabled(true);
    uiNotchesTab->vInternalNotch_RadioButton->setDisabled(true);
    uiNotchesTab->vExternalNotch_RadioButton->setDisabled(true);
    uiNotchesTab->castleNotch_RadioButton->setDisabled(true);
    uiNotchesTab->diamondNotch_RadioButton->setDisabled(true);

    uiNotchesTab->straightforward_RadioButton->setDisabled(true);
    uiNotchesTab->bisector_RadioButton->setDisabled(true);
    uiNotchesTab->intersection_RadioButton->setDisabled(true);

    uiNotchesTab->showNotch_CheckBox->setDisabled(true);
    uiNotchesTab->showNotch_CheckBox->blockSignals(true);

    uiNotchesTab->showSecondNotch_CheckBox->setDisabled(true);
    uiNotchesTab->showSecondNotch_CheckBox->blockSignals(true);

    uiNotchesTab->notchType_GroupBox->blockSignals(true);
    uiNotchesTab->notchSubType_GroupBox->blockSignals(true);

    if (index != -1)
    {
        const VPiece piece = CreatePiece();
        const int nodeIndex = piece.GetPath().indexOfNode(uiNotchesTab->notches_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = piece.GetPath().at(nodeIndex);

            // Notch type
            uiNotchesTab->slitNotch_RadioButton->setEnabled(true);
            uiNotchesTab->tNotch_RadioButton->setEnabled(true);
            uiNotchesTab->uNotch_RadioButton->setEnabled(true);
            uiNotchesTab->vInternalNotch_RadioButton->setEnabled(true);
            uiNotchesTab->vExternalNotch_RadioButton->setEnabled(true);
            uiNotchesTab->castleNotch_RadioButton->setEnabled(true);
            uiNotchesTab->diamondNotch_RadioButton->setEnabled(true);

            uiNotchesTab->notchAngle_DoubleSpinBox->setEnabled(false);

            switch(node.getNotchType())
            {
                case NotchType::Slit:
                    uiNotchesTab->slitNotch_RadioButton->setChecked(true);
                    uiNotchesTab->notchAngle_DoubleSpinBox->setEnabled(true);
                    uiNotchesTab->notchAngle_DoubleSpinBox->setValue(node.getNotchAngle());
                    break;
                case NotchType::TNotch:
                    uiNotchesTab->tNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::UNotch:
                    uiNotchesTab->uNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::VInternal:
                    uiNotchesTab->vInternalNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::VExternal:
                    uiNotchesTab->vExternalNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::Castle:
                    uiNotchesTab->castleNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::Diamond:
                    uiNotchesTab->diamondNotch_RadioButton->setChecked(true);
                    break;
                default:
                    break;
            }

            // Notch Sub type
            uiNotchesTab->straightforward_RadioButton->setEnabled(true);
            uiNotchesTab->bisector_RadioButton->setEnabled(true);
            uiNotchesTab->intersection_RadioButton->setEnabled(true);

            switch(node.getNotchSubType())
            {
                case NotchSubType::Straightforward:
                    uiNotchesTab->straightforward_RadioButton->setChecked(true);
                    break;
                case NotchSubType::Bisector:
                    uiNotchesTab->bisector_RadioButton->setChecked(true);
                    break;
                case NotchSubType::Intersection:
                    uiNotchesTab->intersection_RadioButton->setChecked(true);
                    break;
                default:
                    break;
            }

            // Show the seam allowance notch
            uiNotchesTab->showNotch_CheckBox->setEnabled(true);
            uiNotchesTab->showNotch_CheckBox->setChecked(node.showNotch());

            // Show the seam line notch
            uiNotchesTab->showSecondNotch_CheckBox->setEnabled(true);
            uiNotchesTab->showSecondNotch_CheckBox->setChecked(node.showSecondNotch());

            uiNotchesTab->notchLength_DoubleSpinBox->setEnabled(true);
            uiNotchesTab->notchLength_DoubleSpinBox->setValue(node.getNotchLength());

            uiNotchesTab->notchWidth_DoubleSpinBox->setEnabled(true);
            uiNotchesTab->notchWidth_DoubleSpinBox->setValue(node.getNotchWidth());

            uiNotchesTab->notchCount_SpinBox->setEnabled(true);
            uiNotchesTab->notchCount_SpinBox->setValue(node.getNotchCount());
        }
    }

    uiNotchesTab->showNotch_CheckBox->blockSignals(false);
    uiNotchesTab->showSecondNotch_CheckBox->blockSignals(false);

    uiNotchesTab->notchType_GroupBox->blockSignals(false);
    uiNotchesTab->notchSubType_GroupBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAStartPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiPathsTab->customSeamAllowance_ListWidget->currentRow();
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.startPoint = uiPathsTab->startPoint_ComboBox->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAEndPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiPathsTab->customSeamAllowance_ListWidget->currentRow();
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.endPoint = uiPathsTab->endPoint_ComboBox->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::CSAIncludeTypeChanged(int index)
{
    Q_UNUSED(index);

    const int row = uiPathsTab->customSeamAllowance_ListWidget->currentRow();
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.includeType = static_cast<PiecePathIncludeType>(uiPathsTab->comboBoxIncludeType->currentData().toUInt());
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NodeAngleChanged(int index)
{
    const int i = uiPathsTab->nodes_ComboBox->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiPathsTab->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetAngleType(static_cast<PieceNodeAngle>(uiPathsTab->angle_ComboBox->currentData().toUInt()));
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ReturnDefBefore()
{
    const QString allowance = qApp->TrVars()->FormulaToUser(currentSeamAllowance, qApp->Settings()->GetOsSeparator());
    uiPathsTab->beforeWidthFormula_PlainTextEdit->setPlainText(allowance);
    if (QPushButton *button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ReturnDefAfter()
{
    const QString allowance = qApp->TrVars()->FormulaToUser(currentSeamAllowance, qApp->Settings()->GetOsSeparator());
    uiPathsTab->afterWidthFormula_PlainTextEdit->setPlainText(allowance);
    if (QPushButton *button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::customSeamAllowanceChanged(int row)
{
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= uiPathsTab->customSeamAllowance_ListWidget->count())
    {
        uiPathsTab->startPoint_ComboBox->blockSignals(true);
        uiPathsTab->startPoint_ComboBox->clear();
        uiPathsTab->startPoint_ComboBox->blockSignals(false);

        uiPathsTab->endPoint_ComboBox->blockSignals(true);
        uiPathsTab->endPoint_ComboBox->clear();
        uiPathsTab->endPoint_ComboBox->blockSignals(false);

        uiPathsTab->comboBoxIncludeType->blockSignals(true);
        uiPathsTab->comboBoxIncludeType->clear();
        uiPathsTab->comboBoxIncludeType->blockSignals(false);
        return;
    }

    const QListWidgetItem *item = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const CustomSARecord record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));

    uiPathsTab->startPoint_ComboBox->blockSignals(true);
    InitCSAPoint(uiPathsTab->startPoint_ComboBox);
    {
        const int index = uiPathsTab->startPoint_ComboBox->findData(record.startPoint);
        if (index != -1)
        {
            uiPathsTab->startPoint_ComboBox->setCurrentIndex(index);
        }
    }
    uiPathsTab->startPoint_ComboBox->blockSignals(false);

    uiPathsTab->endPoint_ComboBox->blockSignals(true);
    InitCSAPoint(uiPathsTab->endPoint_ComboBox);
    {
        const int index = uiPathsTab->endPoint_ComboBox->findData(record.endPoint);
        if (index != -1)
        {
            uiPathsTab->endPoint_ComboBox->setCurrentIndex(index);
        }
    }
    uiPathsTab->endPoint_ComboBox->blockSignals(false);

    uiPathsTab->comboBoxIncludeType->blockSignals(true);
    InitSAIncludeType();
    {
        const int index = uiPathsTab->comboBoxIncludeType->findData(static_cast<unsigned char>(record.includeType));
        if (index != -1)
        {
            uiPathsTab->comboBoxIncludeType->setCurrentIndex(index);
        }
    }
    uiPathsTab->comboBoxIncludeType->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PathDialogClosed(int result)
{
    if (result == QDialog::Accepted)
    {
        SCASSERT(!m_dialog.isNull());
        DialogInternalPath *dialogTool = qobject_cast<DialogInternalPath*>(m_dialog.data());
        SCASSERT(dialogTool != nullptr);
        try
        {
            const VPiecePath newPath = dialogTool->GetPiecePath();
            const VPiecePath oldPath = data->GetPiecePath(dialogTool->GetToolId());

            SavePiecePathOptions *saveCommand = new SavePiecePathOptions(oldPath, newPath, qApp->getCurrentDocument(),
                                                                         const_cast<VContainer *>(data),
                                                                         dialogTool->GetToolId());
            qApp->getUndoStack()->push(saveCommand);
            UpdateCurrentCustomSARecord();
            UpdateCurrentInternalPathRecord();
        }
        catch (const VExceptionBadId &e)
        {
            qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Error. Can't save piece path.")),
                       qUtf8Printable(e.ErrorMessage()), qUtf8Printable(e.DetailedInformation()));
        }
    }
    delete m_dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FancyTabChanged(int index)
{
    if (index < 0 || index >= TabOrder::Count)
    {
        return;
    }

    m_pathsTab->hide();
    m_labelsTab->hide();
    m_grainlineTab->hide();
    m_anchorPointsTab->hide();
    m_notchesTab->hide();

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
    switch(index)
    {
        case TabOrder::Paths:
            m_pathsTab->show();
            break;
        case TabOrder::AnchorPoints:
            m_anchorPointsTab->show();
            break;
        case TabOrder::Labels:
            m_labelsTab->show();
            break;
        case TabOrder::Grainline:
            m_grainlineTab->show();
            break;
        case TabOrder::Notches:
            m_notchesTab->show();
            break;
    }
QT_WARNING_POP

    if (index == TabOrder::AnchorPoints || index == TabOrder::Grainline
            || (index == TabOrder::Labels &&
                uiLabelsTab->tabWidget->currentIndex() == uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab)))
    {
        showAnchorPoints();
    }
    else
    {
        if (!m_anchorPoints.isNull())
        {
            delete m_anchorPoints;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::TabChanged(int index)
{
    if (index == uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab))
    {
        showAnchorPoints();
    }
    else
    {
        if (!m_anchorPoints.isNull())
        {
            delete m_anchorPoints;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchTypeChanged(int id)
{
    const int i = uiNotchesTab->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            uiNotchesTab->notchAngle_DoubleSpinBox->setEnabled(false);
            uiNotchesTab->notchAngle_DoubleSpinBox->setValue(0.0);

            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchType notchType = stringToNotchType(qApp->Settings()->getDefaultNotchType());

            if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->slitNotch_RadioButton))
            {
                notchType = NotchType::Slit;
                uiNotchesTab->notchAngle_DoubleSpinBox->setEnabled(true);
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->tNotch_RadioButton))
            {
                notchType = NotchType::TNotch;
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->uNotch_RadioButton))
            {
                notchType = NotchType::UNotch;
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->vInternalNotch_RadioButton))
            {
                notchType = NotchType::VInternal;
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->vExternalNotch_RadioButton))
            {
                notchType = NotchType::VExternal;
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->castleNotch_RadioButton))
            {
                notchType = NotchType::Castle;
            }
            else if (id == uiNotchesTab->notchType_ButtonGroup->id(uiNotchesTab->diamondNotch_RadioButton))
            {
                notchType = NotchType::Diamond;
            }

            rowNode.setNotchType(notchType);
            const NodeInfo info = getNodeInfo(rowNode, true);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setIcon(QIcon(info.icon));
            rowItem->setText(info.name);

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchSubTypeChanged(int id)
{
    const int i = uiNotchesTab->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchSubType notchSubType = NotchSubType::Straightforward;
            if (id == uiNotchesTab->notchSubType_ButtonGroup->id(uiNotchesTab->straightforward_RadioButton))
            {
                notchSubType = NotchSubType::Straightforward;
            }
            else if (id == uiNotchesTab->notchSubType_ButtonGroup->id(uiNotchesTab->bisector_RadioButton))
            {
                notchSubType = NotchSubType::Bisector;
            }
            else if (id == uiNotchesTab->notchSubType_ButtonGroup->id(uiNotchesTab->intersection_RadioButton))
            {
                notchSubType = NotchSubType::Intersection;
            }

            rowNode.setNotchSubType(notchSubType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showNotchChanged(int state)
{
    const int i = uiNotchesTab->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setShowNotch(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showSecondNotchChanged(int state)
{
    const int i = uiNotchesTab->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setShowSecondNotch(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchLengthChanged(qreal value)
{
  const int i = uiNotchesTab->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchLength(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          ListChanged();
      }
  }

}

void DialogSeamAllowance::resetNotchLength()
{
    uiNotchesTab->notchLength_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchLength());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchWidthChanged(qreal value)
{
  const int i = uiNotchesTab->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchWidth(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          ListChanged();
      }
  }

}

void DialogSeamAllowance::resetNotchWidth()
{
    uiNotchesTab->notchWidth_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchWidth());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchAngleChanged(qreal value)
{
  const int i = uiNotchesTab->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchAngle(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          ListChanged();
      }
  }
}

void DialogSeamAllowance::resetNotchAngle()
{
    uiNotchesTab->notchAngle_DoubleSpinBox->setValue(0.0);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::notchCountChanged(int value)
{
  const int i = uiNotchesTab->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchCount(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          ListChanged();
      }
  }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateGrainlineValues()
{
    QPlainTextEdit *apleSender[2] = {uiGrainlineTab->rotationFormula_LineEdit, uiGrainlineTab->lengthFormula_LineEdit};
    bool bFormulasOK[2] = {true, true};

    for (int i = 0; i < 2; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiGrainlineTab->labelRot;
            plbText = uiGrainlineTab->labelEditRot;
            qsUnit = degreeSymbol;
        }
        else
        {
            plbVal = uiGrainlineTab->labelLen;
            plbText = uiGrainlineTab->labelEditLen;
            qsUnit = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender[i]->toPlainText().simplified();
        QString qsVal;
        try
        {
            qsFormula.replace("\n", " ");
            qsFormula = qApp->TrVars()->FormulaFromUser(qsFormula, qApp->Settings()->GetOsSeparator());
            Calculator cal;
            qreal dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) == true || qIsNaN(dVal) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if (i == 1 && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                qsVal.setNum(dVal, 'f', 2);
                ChangeColor(plbText, okColor);
            }
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            !flagGrainlineAnchor ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagGrainlineFormula = bFormulasOK[0] && bFormulasOK[1];
    if (!flagGrainlineFormula && !flagGrainlineAnchor)
    {
        m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline") + QLatin1String("*"));
    }
    else
    {
        ResetGrainlineWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateDetailLabelValues()
{
    QPlainTextEdit *apleSender[3] = {uiLabelsTab->dLabelWidthFormula_LineEdit, uiLabelsTab->dLabelHeightFormula_LineEdit,
                                     uiLabelsTab->dLabelAngleFormula_LineEdit};
    bool bFormulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiLabelsTab->labelDLWidth;
            plbText = uiLabelsTab->labelEditDLWidth;
            qsUnit = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else if (i == 1)
        {
            plbVal = uiLabelsTab->labelDLHeight;
            plbText = uiLabelsTab->labelEditDLHeight;
            qsUnit = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else
        {
            plbVal = uiLabelsTab->labelDLAngle;
            plbText = uiLabelsTab->labelEditDLAngle;
            qsUnit = degreeSymbol;
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender[i]->toPlainText().simplified();
        QString qsVal;
        try
        {
            qsFormula.replace("\n", " ");
            qsFormula = qApp->TrVars()->FormulaFromUser(qsFormula, qApp->Settings()->GetOsSeparator());
            Calculator cal;
            qreal dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) == true || qIsNaN(dVal) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if ((i == 0 || i == 1) && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                qsVal.setNum(dVal, 'f', 2);
                ChangeColor(plbText, okColor);
            }
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            !flagPieceLabelAnchor ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagPieceLabelAngle = bFormulasOK[2];
    flagPieceLabelFormula = bFormulasOK[0] && bFormulasOK[1];
    if (!flagPieceLabelAngle || !(flagPieceLabelFormula || flagPieceLabelAnchor) || !flagPatternLabelAngle || !(flagPatternLabelFormula || flagPatternLabelAnchor))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    else
    {
        ResetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdatePatternLabelValues()
{
    QPlainTextEdit *apleSender[3] = {uiLabelsTab->pLabelWidthFormula_LineEdit, uiLabelsTab->pLabelHeightFormula_LineEdit,
                                     uiLabelsTab->pLabelAngleFormula_LineEdit};
    bool bFormulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel *plbVal;
        QLabel *plbText;
        QString qsUnit;
        if (i == 0)
        {
            plbVal = uiLabelsTab->labelPLWidth;
            plbText = uiLabelsTab->labelEditPLWidth;
            qsUnit = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else if (i == 1)
        {
            plbVal = uiLabelsTab->labelPLHeight;
            plbText = uiLabelsTab->labelEditPLHeight;
            qsUnit = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else
        {
            plbVal = uiLabelsTab->labelPLAngle;
            plbText = uiLabelsTab->labelEditPLAngle;
            qsUnit = degreeSymbol;
        }

        plbVal->setToolTip(tr("Value"));

        QString qsFormula = apleSender[i]->toPlainText().simplified();
        QString qsVal;
        try
        {
            qsFormula.replace("\n", " ");
            qsFormula = qApp->TrVars()->FormulaFromUser(qsFormula, qApp->Settings()->GetOsSeparator());
            Calculator cal;
            qreal dVal = cal.EvalFormula(data->DataVariables(), qsFormula);
            if (qIsInf(dVal) == true || qIsNaN(dVal) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if ((i == 0 || i == 1) && dVal <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                qsVal.setNum(dVal, 'f', 2);
                ChangeColor(plbText, okColor);
            }
        }
        catch (qmu::QmuParserError &e)
        {
            qsVal = tr("Error");
            !flagPatternLabelAnchor ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagPatternLabelAngle = bFormulasOK[2];
    flagPatternLabelFormula = bFormulasOK[0] && bFormulasOK[1];
    if (!flagPieceLabelAngle || !(flagPieceLabelFormula || flagPieceLabelAnchor) || !flagPatternLabelAngle || !(flagPatternLabelFormula || flagPatternLabelAnchor))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    else
    {
        ResetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledGrainline()
{
    if (uiGrainlineTab->grainline_GroupBox->isChecked() == true)
    {
        UpdateGrainlineValues();
        grainlineAnchorChanged();
    }
    else
    {
        flagGrainlineFormula = true;
        ResetGrainlineWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledDetailLabel()
{
    if (uiLabelsTab->pieceLabel_GroupBox->isChecked() == true)
    {
        UpdateDetailLabelValues();
        pieceLabelAnchorChanged();
    }
    else
    {
        flagPieceLabelAngle = true;
        flagPieceLabelFormula = true;
        ResetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledPatternLabel()
{
    if (uiLabelsTab->patternLabel_GroupBox->isChecked() == true)
    {
        UpdatePatternLabelValues();
        patternLabelAnchorChanged();
    }
    else
    {
        flagPatternLabelAngle = true;
        flagPatternLabelFormula = true;
        ResetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditGrainlineFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiGrainlineTab->length_PushButton)
    {
        pleFormula = uiGrainlineTab->lengthFormula_LineEdit;
        bCheckZero = true;
        title = tr("Edit length");
    }
    else if (sender() == uiGrainlineTab->rotation_PushButton)
    {
        pleFormula = uiGrainlineTab->rotationFormula_LineEdit;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(qApp->TrVars()->TryFormulaFromUser(pleFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString qsFormula = dlg.GetFormula();
        qsFormula.replace("\n", " ");

        if (sender() == uiGrainlineTab->length_PushButton)
        {
            SetGrainlineLength(qsFormula);
        }
        else if (sender() == uiGrainlineTab->rotation_PushButton)
        {
            SetGrainlineAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdateGrainlineValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditDLFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiLabelsTab->dLabelHeght_PushButton)
    {
        pleFormula = uiLabelsTab->dLabelHeightFormula_LineEdit;
        bCheckZero = true;
        title = tr("Edit height");
    }
    else if (sender() == uiLabelsTab->dLabelWidth_PushButton)
    {
        pleFormula = uiLabelsTab->dLabelWidthFormula_LineEdit;
        bCheckZero = true;
        title = tr("Edit width");
    }
    else if (sender() == uiLabelsTab->dLabelAngle_PushButton)
    {
        pleFormula = uiLabelsTab->dLabelAngleFormula_LineEdit;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(qApp->TrVars()->TryFormulaFromUser(pleFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString qsFormula = dlg.GetFormula();
        qsFormula.replace("\n", " ");
        if (sender() == uiLabelsTab->dLabelHeght_PushButton)
        {
            SetDLHeight(qsFormula);
        }
        else if (sender() == uiLabelsTab->dLabelWidth_PushButton)
        {
            SetDLWidth(qsFormula);
        }
        else if (sender() == uiLabelsTab->dLabelAngle_PushButton)
        {
            SetDLAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdateDetailLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditPLFormula()
{
    QPlainTextEdit *pleFormula;
    bool bCheckZero;
    QString title;

    if (sender() == uiLabelsTab->pLabelHeight_PushButton)
    {
        pleFormula = uiLabelsTab->pLabelHeightFormula_LineEdit;
        bCheckZero = true;
        title = tr("Edit height");
    }
    else if (sender() == uiLabelsTab->pLabelWidth_PushButton)
    {
        pleFormula = uiLabelsTab->pLabelWidthFormula_LineEdit;
        bCheckZero = true;
        title = tr("Edit width");
    }
    else if (sender() == uiLabelsTab->pLabelAngle_PushButton)
    {
        pleFormula = uiLabelsTab->pLabelAngleFormula_LineEdit;
        bCheckZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    DialogEditWrongFormula dlg(data, NULL_ID, this);
    dlg.setWindowTitle(title);
    dlg.SetFormula(qApp->TrVars()->TryFormulaFromUser(pleFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dlg.setCheckZero(bCheckZero);
    if (dlg.exec() == QDialog::Accepted)
    {
        QString qsFormula = dlg.GetFormula();
        qsFormula.replace("\n", " ");
        if (sender() == uiLabelsTab->pLabelHeight_PushButton)
        {
            SetPLHeight(qsFormula);
        }
        else if (sender() == uiLabelsTab->pLabelWidth_PushButton)
        {
            SetPLWidth(qsFormula);
        }
        else if (sender() == uiLabelsTab->pLabelAngle_PushButton)
        {
            SetPLAngle(qsFormula);
        }
        else
        {
            // should not get here!
            pleFormula->setPlainText(qsFormula);
        }
        UpdatePatternLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployGrainlineRotation()
{
    DeployFormula(uiGrainlineTab->rotationFormula_LineEdit, uiGrainlineTab->showRotation_PushButton, m_iRotBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployGrainlineLength()
{
    DeployFormula(uiGrainlineTab->lengthFormula_LineEdit, uiGrainlineTab->showLength_PushButton, m_iLenBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLWidth()
{
    DeployFormula(uiLabelsTab->dLabelWidthFormula_LineEdit, uiLabelsTab->showDLWidth_PushButton, m_dLabelWidthBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLHeight()
{
    DeployFormula(uiLabelsTab->dLabelHeightFormula_LineEdit, uiLabelsTab->showDLHeight_PushButton, m_dLabelHeightBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployDLAngle()
{
    DeployFormula(uiLabelsTab->dLabelAngleFormula_LineEdit, uiLabelsTab->showDLAngle_PushButton, m_dLabelAngleBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLWidth()
{
    DeployFormula(uiLabelsTab->pLabelWidthFormula_LineEdit, uiLabelsTab->showPLabelWidth_PushButton, m_pLabelWidthBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLHeight()
{
    DeployFormula(uiLabelsTab->pLabelHeightFormula_LineEdit, uiLabelsTab->showPLabelHeight_PushButton, m_pLabelHeightBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployPLAngle()
{
    DeployFormula(uiLabelsTab->pLabelAngleFormula_LineEdit, uiLabelsTab->showPLabelAngle_PushButton, m_pLabelAngleBaseHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ResetGrainlineWarning()
{
    if (flagGrainlineFormula || flagGrainlineAnchor)
    {
        m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ResetLabelsWarning()
{
    if (flagPieceLabelAngle && (flagPieceLabelFormula || flagPieceLabelAnchor) && flagPatternLabelAngle && (flagPatternLabelFormula || flagPatternLabelAnchor))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::evaluateDefaultWidth()
{
    labelEditFormula = uiPathsTab->widthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const QString formula = uiPathsTab->widthFormula_PlainTextEdit->toPlainText();
    m_saWidth = Eval(formula, flagFormula, uiPathsTab->widthResult_Label, postfix, false, true);

    if (m_saWidth >= 0)
    {
        VContainer *locData = const_cast<VContainer *> (data);
        locData->AddVariable(currentSeamAllowance, new VIncrement(locData, currentSeamAllowance, 0, m_saWidth,
                                                                  QString().setNum(m_saWidth), true,
                                                                  tr("Current seam allowance")));

        evaluateBeforeWidth();
        evaluateAfterWidth();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::evaluateBeforeWidth()
{
    if (uiPathsTab->nodes_ComboBox->count() > 0)
    {
        labelEditFormula = uiPathsTab->beforeWidthEdit_Label;
        const QString postfix = UnitsToStr(qApp->patternUnit(), true);
        const QString formula = uiPathsTab->beforeWidthFormula_PlainTextEdit->toPlainText();
        Eval(formula, flagBeforeFormula, uiPathsTab->beforeWidthResult_Label, postfix, false, true);

        const QString formulaSABefore = GetFormulaFromUser(uiPathsTab->beforeWidthFormula_PlainTextEdit);
        UpdateNodeSABefore(formulaSABefore);
        EnableDefButton(uiPathsTab->beforeDefault_PushButton, formulaSABefore);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::evaluateAfterWidth()
{
    if (uiPathsTab->nodes_ComboBox->count() > 0)
    {
        labelEditFormula = uiPathsTab->afterWidthEdit_Label;
        const QString postfix = UnitsToStr(qApp->patternUnit(), true);
        const QString formula = uiPathsTab->afterWidthFormula_PlainTextEdit->toPlainText();
        Eval(formula, flagAfterFormula, uiPathsTab->afterWidthResult_Label, postfix, false, true);

        const QString formulaSAAfter = GetFormulaFromUser(uiPathsTab->afterWidthFormula_PlainTextEdit);
        UpdateNodeSAAfter(formulaSAAfter);
        EnableDefButton(uiPathsTab->afterDefault_PushButton, formulaSAAfter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXWidth()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width"));
    dialog->SetFormula(GetFormulaSAWidth());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetFormulaSAWidth(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXWidthBefore()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(GetFormulaFromUser(uiPathsTab->beforeWidthFormula_PlainTextEdit));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSABefore(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::FXWidthAfter()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(GetFormulaFromUser(uiPathsTab->afterWidthFormula_PlainTextEdit));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSAAfter(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::defaultWidthChanged()
{
    labelEditFormula = uiPathsTab->widthEdit_Label;
    labelResultCalculation = uiPathsTab->widthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagFormula, uiPathsTab->widthFormula_PlainTextEdit, m_timerWidth, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::beforeWidthChanged()
{
    labelEditFormula = uiPathsTab->beforeWidthEdit_Label;
    labelResultCalculation = uiPathsTab->beforeWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);

    ValFormulaChanged(flagBeforeFormula, uiPathsTab->beforeWidthFormula_PlainTextEdit, m_timerWidthBefore, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::afterWidthChanged()
{
    labelEditFormula = uiPathsTab->afterWidthEdit_Label;
    labelResultCalculation = uiPathsTab->afterWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);

    ValFormulaChanged(flagAfterFormula, uiPathsTab->afterWidthFormula_PlainTextEdit, m_timerWidthAfter, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthFormulaTextEdit()
{
    DeployFormula(uiPathsTab->widthFormula_PlainTextEdit, uiPathsTab->widthGrow_PushButton, m_widthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthBeforeFormulaTextEdit()
{
    DeployFormula(uiPathsTab->beforeWidthFormula_PlainTextEdit, uiPathsTab->beforeWidthGrow_PushButton,
                  m_beforeWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DeployWidthAfterFormulaTextEdit()
{
    DeployFormula(uiPathsTab->afterWidthFormula_PlainTextEdit, uiPathsTab->afterWidthGrow_PushButton,
                  m_afterWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::grainlineAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(uiGrainlineTab->topGrainlineAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(uiGrainlineTab->bottomGrainlineAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagGrainlineAnchor = true;
        color = okColor;

        ResetGrainlineWarning();
    }
    else
    {
        flagGrainlineAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        if (!flagGrainlineFormula && !flagGrainlineAnchor)
        {
            m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
        }
    }
    UpdateGrainlineValues();
    ChangeColor(uiGrainlineTab->topGrainlineAnchor_Label, color);
    ChangeColor(uiGrainlineTab->bottomGrainlineAnchor_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::pieceLabelAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(uiLabelsTab->dLabelTopLeftAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(uiLabelsTab->dLabelBottomRightAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagPieceLabelAnchor = true;
        color = okColor;

        if (flagPatternLabelAnchor)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
        }
    }
    else
    {
        flagPieceLabelAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    UpdateDetailLabelValues();
    ChangeColor(uiLabelsTab->labelDLTopLeftAnchor, color);
    ChangeColor(uiLabelsTab->labelDLBottomRightAnchor, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::patternLabelAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(uiLabelsTab->pLabelTopLeftAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(uiLabelsTab->pLabelBottomRightAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagPatternLabelAnchor = true;
        color = okColor;

        if (flagPieceLabelAnchor)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
        }
    }
    else
    {
        flagPatternLabelAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    UpdatePatternLabelValues();
    ChangeColor(uiLabelsTab->labelPLTopLeftAnchor, color);
    ChangeColor(uiLabelsTab->labelPLBottomRightAnchor, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditLabel()
{
    DialogEditLabel editor(qApp->getCurrentDocument());
    editor.SetTemplate(m_templateLines);
    editor.SetPiece(GetPiece());

    if (QDialog::Accepted == editor.exec())
    {
        m_templateLines = editor.GetTemplate();
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiece DialogSeamAllowance::CreatePiece() const
{
    VPiece piece;
    piece.GetPath().SetNodes(GetListInternals<VPieceNode>(uiPathsTab->mainPath_ListWidget));
    piece.SetCustomSARecords(GetListInternals<CustomSARecord>(uiPathsTab->customSeamAllowance_ListWidget));
    piece.SetInternalPaths(GetListInternals<quint32>(uiPathsTab->internalPaths_ListWidget));
    piece.setAnchors(GetListInternals<quint32>(uiAnchorPointsTab->anchorPoints_ListWidget));
    piece.SetForbidFlipping(uiPathsTab->forbidFlipping_CheckBox->isChecked());
    piece.SetSeamAllowance(uiPathsTab->seams_CheckBox->isChecked());
    piece.SetSeamAllowanceBuiltIn(uiPathsTab->builtIn_CheckBox->isChecked());
    piece.setHideSeamLine(uiPathsTab->hideSeamLine_CheckBox->isChecked());
    piece.SetName(uiLabelsTab->detailName_LineEdit->text());
    piece.SetMx(m_mx);
    piece.SetMy(m_my);
    piece.SetFormulaSAWidth(GetFormulaFromUser(uiPathsTab->widthFormula_PlainTextEdit), m_saWidth);
    piece.GetPatternPieceData().SetLetter(uiLabelsTab->letter_LineEdit->text());
    piece.GetPatternPieceData().SetAnnotation(uiLabelsTab->annotation_LineEdit->text());
    piece.GetPatternPieceData().SetOrientation(uiLabelsTab->orientation_LineEdit->text());
    piece.GetPatternPieceData().SetRotationWay(uiLabelsTab->rotation_LineEdit->text());
    piece.GetPatternPieceData().SetTilt(uiLabelsTab->tilt_LineEdit->text());
    piece.GetPatternPieceData().SetFoldPosition(uiLabelsTab->foldPosition_LineEdit->text());
    piece.GetPatternPieceData().SetQuantity(uiLabelsTab->quantity_SpinBox->value());
    piece.GetPatternPieceData().SetOnFold(uiLabelsTab->fold_CheckBox->isChecked());
    piece.GetPatternPieceData().SetLabelTemplate(m_templateLines);
    piece.GetPatternPieceData().SetPos(m_oldData.GetPos());
    piece.GetPatternPieceData().SetLabelWidth(GetFormulaFromUser(uiLabelsTab->dLabelWidthFormula_LineEdit));
    piece.GetPatternPieceData().SetLabelHeight(GetFormulaFromUser(uiLabelsTab->dLabelHeightFormula_LineEdit));
    piece.GetPatternPieceData().SetFontSize(m_oldData.getFontSize());
    piece.GetPatternPieceData().SetRotation(GetFormulaFromUser(uiLabelsTab->dLabelAngleFormula_LineEdit));
    piece.GetPatternPieceData().SetVisible(uiLabelsTab->pieceLabel_GroupBox->isChecked());
    piece.GetPatternPieceData().setCenterAnchorPoint(getCurrentObjectId(uiLabelsTab->dLabelCenterAnchor_ComboBox));
    piece.GetPatternPieceData().setTopLeftAnchorPoint(getCurrentObjectId(uiLabelsTab->dLabelTopLeftAnchor_ComboBox));
    piece.GetPatternPieceData().setBottomRightAnchorPoint(getCurrentObjectId(uiLabelsTab->dLabelBottomRightAnchor_ComboBox));

    piece.GetPatternInfo() = m_oldGeom;
    piece.GetPatternInfo().SetVisible(uiLabelsTab->patternLabel_GroupBox->isChecked());
    piece.GetPatternInfo().setCenterAnchorPoint(getCurrentObjectId(uiLabelsTab->pLabelCenterAnchor_ComboBox));
    piece.GetPatternInfo().setTopLeftAnchorPoint(getCurrentObjectId(uiLabelsTab->pLabelTopLeftAnchor_ComboBox));
    piece.GetPatternInfo().setBottomRightAnchorPoint(getCurrentObjectId(uiLabelsTab->pLabelBottomRightAnchor_ComboBox));
    piece.GetPatternInfo().SetLabelWidth(GetFormulaFromUser(uiLabelsTab->pLabelWidthFormula_LineEdit));
    piece.GetPatternInfo().SetLabelHeight(GetFormulaFromUser(uiLabelsTab->pLabelHeightFormula_LineEdit));
    piece.GetPatternInfo().SetRotation(GetFormulaFromUser(uiLabelsTab->pLabelAngleFormula_LineEdit));

    piece.GetGrainlineGeometry() = m_oldGrainline;
    piece.GetGrainlineGeometry().SetVisible(uiGrainlineTab->grainline_GroupBox->isChecked());
    piece.GetGrainlineGeometry().SetRotation(GetFormulaFromUser(uiGrainlineTab->rotationFormula_LineEdit));
    piece.GetGrainlineGeometry().SetLength(GetFormulaFromUser(uiGrainlineTab->lengthFormula_LineEdit));
    piece.GetGrainlineGeometry().SetArrowType(static_cast<ArrowType>(uiGrainlineTab->arrow_ComboBox->currentIndex()));
    piece.GetGrainlineGeometry().setCenterAnchorPoint(getCurrentObjectId(uiGrainlineTab->centerGrainlineAnchor_ComboBox));
    piece.GetGrainlineGeometry().setTopAnchorPoint(getCurrentObjectId(uiGrainlineTab->topGrainlineAnchor_ComboBox));
    piece.GetGrainlineGeometry().setBottomAnchorPoint(getCurrentObjectId(uiGrainlineTab->bottomGrainlineAnchor_ComboBox));

    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NewMainPathItem(const VPieceNode &node)
{
    newNodeItem(uiPathsTab->mainPath_ListWidget, node);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NewCustomSA(const CustomSARecord &record)
{
    if (record.path > NULL_ID)
    {
        const QString name = GetPathName(record.path, record.reverse);

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setFont(QFont("Times", 12, QFont::Bold));
        item->setData(Qt::UserRole, QVariant::fromValue(record));
        uiPathsTab->customSeamAllowance_ListWidget->addItem(item);
        uiPathsTab->customSeamAllowance_ListWidget->setCurrentRow(uiPathsTab->customSeamAllowance_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NewInternalPath(quint32 path)
{
    if (path > NULL_ID)
    {
        const QString name = GetPathName(path);

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setFont(QFont("Times", 12, QFont::Bold));
        item->setData(Qt::UserRole, QVariant::fromValue(path));
        uiPathsTab->internalPaths_ListWidget->addItem(item);
        uiPathsTab->internalPaths_ListWidget->setCurrentRow(uiPathsTab->internalPaths_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::newAnchorPoint(quint32 anchorPoint)
{
    if (anchorPoint > NULL_ID)
    {
        const QSharedPointer<VGObject> anchor = data->GetGObject(anchorPoint);

        QListWidgetItem *item = new QListWidgetItem(anchor->name());
        item->setFont(QFont("Times", 12, QFont::Bold));
        item->setData(Qt::UserRole, QVariant::fromValue(anchorPoint));
        uiAnchorPointsTab->anchorPoints_ListWidget->addItem(item);
        uiAnchorPointsTab->anchorPoints_ListWidget->setCurrentRow(uiAnchorPointsTab->anchorPoints_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSeamAllowance::GetPathName(quint32 path, bool reverse) const
{
    QString name;

    if (path > NULL_ID)
    {
        name = data->GetPiecePath(path).GetName();

        if (reverse)
        {
            name = QLatin1String("- ") + name;
        }
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogSeamAllowance::MainPathIsValid() const
{
    QString url = DialogWarningIcon();

    if(CreatePiece().MainPathPoints(data).count() < 3)
    {
        url += tr("You need more points!");
        uiPathsTab->status_Label->setText(url);
        return false;
    }
    else
    {
        if(!MainPathIsClockwise())
        {
            url += tr("You must choose points in a clockwise direction!");
            uiPathsTab->status_Label->setText(url);
            return false;
        }
        if (FirstPointEqualLast(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("First point cannot be same as last point!");
            uiPathsTab->status_Label->setText(url);
            return  false;
        }
        else if (DoublePoints(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("You have double points!");
            uiPathsTab->status_Label->setText(url);
            return  false;
        }
        else if (!EachPointLabelIsUnique(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("Each point in the path must be unique!");
            uiPathsTab->status_Label->setText(url);
            return  false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ValidObjects(bool value)
{
    flagMainPath = value;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogSeamAllowance::MainPathIsClockwise() const
{
    const QVector<QPointF> points = CreatePiece().MainPathPoints(data);

    if(points.count() < 3)
    {
        return false;
    }

    const qreal res = VPiece::SumTrapezoids(points);
    if (res < 0)
    {
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitNodesList()
{
    const quint32 id = uiPathsTab->nodes_ComboBox->currentData().toUInt();

    uiPathsTab->nodes_ComboBox->blockSignals(true);
    uiPathsTab->nodes_ComboBox->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiPathsTab->mainPath_ListWidget);

    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && !node.isExcluded())
        {
            const NodeInfo info = getNodeInfo(node);

            uiPathsTab->nodes_ComboBox->addItem(info.name, node.GetId());
        }
    }
    uiPathsTab->nodes_ComboBox->blockSignals(false);

    const int index = uiPathsTab->nodes_ComboBox->findData(id);
    if (index != -1)
    {
        uiPathsTab->nodes_ComboBox->setCurrentIndex(index);
        NodeChanged(index);// Need in case combox index was not changed
    }
    else
    {
        uiPathsTab->nodes_ComboBox->count() > 0 ? NodeChanged(0) : NodeChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitNotchesList()
{
    const quint32 id = uiNotchesTab->notches_ComboBox->currentData().toUInt();

    uiNotchesTab->notches_ComboBox->blockSignals(true);
    uiNotchesTab->notches_ComboBox->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiPathsTab->mainPath_ListWidget);

    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && node.isNotch())
        {
            const NodeInfo info = getNodeInfo(node);

            uiNotchesTab->notches_ComboBox->addItem(QIcon(info.icon), info.name, node.GetId());
        }
    }
    uiNotchesTab->notches_ComboBox->blockSignals(false);

    const int index = uiNotchesTab->notches_ComboBox->findData(id);
    if (index != -1)
    {
        uiNotchesTab->notches_ComboBox->setCurrentIndex(index);
        notchChanged(index);// Need in case combox index was not changed
    }
    else
    {
        uiNotchesTab->notches_ComboBox->count() > 0 ? notchChanged(0) : notchChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QListWidgetItem *DialogSeamAllowance::GetItemById(quint32 id)
{
    for (qint32 i = 0; i < uiPathsTab->mainPath_ListWidget->count(); ++i)
    {
        QListWidgetItem *item = uiPathsTab->mainPath_ListWidget->item(i);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));

        if (node.GetId() == id)
        {
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogSeamAllowance::GetLastId() const
{
    const int count = uiPathsTab->mainPath_ListWidget->count();
    if (count > 0)
    {
        QListWidgetItem *item = uiPathsTab->mainPath_ListWidget->item(count-1);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node.GetId();
    }
    else
    {
        return NULL_ID;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetCurrentSABefore(const QString &formula)
{
    UpdateNodeSABefore(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetCurrentSAAfter(const QString &formula)
{
    UpdateNodeSAAfter(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodeSABefore(const QString &formula)
{
    const int index = uiPathsTab->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiPathsTab->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSABefore(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateNodeSAAfter(const QString &formula)
{
    const int index = uiPathsTab->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(uiPathsTab->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSAAfter(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitFancyTabBar()
{
    m_ftb->InsertTab(TabOrder::Paths, QIcon("://icon/32x32/paths@2x.png"), tr("Paths"));
    m_ftb->InsertTab(TabOrder::AnchorPoints, QIcon("://icon/32x32/anchor_point@2x.png"), tr("Anchor Points"));
    m_ftb->InsertTab(TabOrder::Labels, QIcon("://icon/32x32/labels@2x.png"), tr("Labels"));
    m_ftb->InsertTab(TabOrder::Grainline, QIcon("://icon/32x32/grainline@2x.png"), tr("Grainlines"));
    m_ftb->InsertTab(TabOrder::Notches, QIcon("://icon/32x32/notches@2x.png"), tr("Notches"));

    ui->horizontalLayout->addWidget(m_ftb, 0, Qt::AlignLeft);

    m_ftb->SetTabEnabled(TabOrder::Paths, true);

    m_pathsTab->hide();
    uiPathsTab->setupUi(m_pathsTab);
    ui->horizontalLayout->addWidget(m_pathsTab, 1);

    m_labelsTab->hide();
    uiLabelsTab->setupUi(m_labelsTab);
    ui->horizontalLayout->addWidget(m_labelsTab, 1);

    m_grainlineTab->hide();
    uiGrainlineTab->setupUi(m_grainlineTab);
    ui->horizontalLayout->addWidget(m_grainlineTab, 1);

    m_anchorPointsTab->hide();
    uiAnchorPointsTab->setupUi(m_anchorPointsTab);
    ui->horizontalLayout->addWidget(m_anchorPointsTab, 1);

    m_notchesTab->hide();
    uiNotchesTab->setupUi(m_notchesTab);
    ui->horizontalLayout->addWidget(m_notchesTab, 1);

    connect(m_ftb, &FancyTabBar::CurrentChanged, this, &DialogSeamAllowance::FancyTabChanged);
    connect(uiLabelsTab->tabWidget, &QTabWidget::currentChanged, this, &DialogSeamAllowance::TabChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitMainPathTab()
{
    uiPathsTab->forbidFlipping_CheckBox->setChecked(qApp->Settings()->getForbidPieceFlipping());
    uiPathsTab->hideSeamLine_CheckBox->setChecked(qApp->Settings()->isHideSeamLine());

    uiPathsTab->mainPath_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiPathsTab->mainPath_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowMainPathContextMenu);

    connect(uiPathsTab->mainPath_ListWidget->model(), &QAbstractItemModel::rowsMoved, this, [this]()
    {
      ListChanged();
      ValidObjects(MainPathIsValid());
    });

    connect(uiPathsTab->mainPath_ListWidget, &QListWidget::itemSelectionChanged,
            this, &DialogSeamAllowance::setMoveExclusions);
    connect(uiPathsTab->moveTop_ToolButton,&QToolButton::clicked, this, [this]()
    {
        moveListRowTop(uiPathsTab->mainPath_ListWidget);
        ValidObjects(MainPathIsValid());
    });
    connect(uiPathsTab->moveUp_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowUp(uiPathsTab->mainPath_ListWidget);
        ValidObjects(MainPathIsValid());
    });
    connect(uiPathsTab->moveDown_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowDown(uiPathsTab->mainPath_ListWidget);
        ValidObjects(MainPathIsValid());
    });
    connect(uiPathsTab->moveBottom_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowBottom(uiPathsTab->mainPath_ListWidget);
        ValidObjects(MainPathIsValid());
    });
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitSeamAllowanceTab()
{
    plainTextEditFormula = uiPathsTab->widthFormula_PlainTextEdit;
    this->m_widthFormula = uiPathsTab->widthFormula_PlainTextEdit->height();
    this->m_beforeWidthFormula = uiPathsTab->beforeWidthFormula_PlainTextEdit->height();
    this->m_afterWidthFormula = uiPathsTab->afterWidthFormula_PlainTextEdit->height();

    uiPathsTab->widthFormula_PlainTextEdit->installEventFilter(this);
    uiPathsTab->beforeWidthFormula_PlainTextEdit->installEventFilter(this);
    uiPathsTab->afterWidthFormula_PlainTextEdit->installEventFilter(this);

    m_timerWidth = new QTimer(this);
    connect(m_timerWidth, &QTimer::timeout, this, &DialogSeamAllowance::evaluateDefaultWidth);

    m_timerWidthBefore = new QTimer(this);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogSeamAllowance::evaluateBeforeWidth);

    m_timerWidthAfter = new QTimer(this);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogSeamAllowance::evaluateAfterWidth);

    connect(uiPathsTab->seams_CheckBox, &QCheckBox::toggled, this, &DialogSeamAllowance::EnableSeamAllowance);
    connect(uiPathsTab->builtIn_CheckBox, &QCheckBox::toggled, this, &DialogSeamAllowance::enableBuiltIn);

    // Initialize the default seam allowance, convert the value if app unit is different than pattern unit
    m_saWidth = UnitConvertor(qApp->Settings()->GetDefaultSeamAllowance(),
                              StrToUnits(qApp->Settings()->GetUnit()), qApp->patternUnit());

    uiPathsTab->widthFormula_PlainTextEdit->setPlainText(qApp->LocaleToString(m_saWidth));

    InitNodesList();
    connect(uiPathsTab->nodes_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::NodeChanged);

    connect(uiPathsTab->beforeDefault_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::ReturnDefBefore);
    connect(uiPathsTab->afterDefault_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::ReturnDefAfter);

    InitNodeAngles(uiPathsTab->angle_ComboBox);
    connect(uiPathsTab->angle_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::NodeAngleChanged);

    uiPathsTab->customSeamAllowance_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiPathsTab->customSeamAllowance_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowCustomSAContextMenu);
    connect(uiPathsTab->customSeamAllowance_ListWidget, &QListWidget::currentRowChanged, this,
            &DialogSeamAllowance::customSeamAllowanceChanged);
    connect(uiPathsTab->startPoint_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::CSAStartPointChanged);
    connect(uiPathsTab->endPoint_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogSeamAllowance::CSAEndPointChanged);
    connect(uiPathsTab->comboBoxIncludeType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::CSAIncludeTypeChanged);

    connect(uiPathsTab->toolButtonExprWidth, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidth);
    connect(uiPathsTab->beforeExpr_ToolButton, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidthBefore);
    connect(uiPathsTab->afterExpr_ToolButton, &QPushButton::clicked, this, &DialogSeamAllowance::FXWidthAfter);

    connect(uiPathsTab->widthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::defaultWidthChanged);
    connect(uiPathsTab->beforeWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::beforeWidthChanged);
    connect(uiPathsTab->afterWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::afterWidthChanged);

    connect(uiPathsTab->widthGrow_PushButton, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployWidthFormulaTextEdit);
    connect(uiPathsTab->beforeWidthGrow_PushButton, &QPushButton::clicked,
            this, &DialogSeamAllowance::DeployWidthBeforeFormulaTextEdit);
    connect(uiPathsTab->afterWidthGrow_PushButton, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployWidthAfterFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitCSAPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();
    box->addItem(tr("Empty"), NULL_ID);

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(uiPathsTab->mainPath_ListWidget);

    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode &node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && !node.isExcluded())
        {
            const NodeInfo info = getNodeInfo(node);
            box->addItem(info.name, node.GetId());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::initAnchorPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);

    quint32 currentId = NULL_ID;
    if (box->count() > 0)
    {
        currentId = box->currentData().toUInt();
    }

    box->clear();
    box->addItem(QLatin1String("<") + tr("None") + QLatin1String(">"), NULL_ID);

    const QVector<quint32> anchorPoints = GetListInternals<quint32>(uiAnchorPointsTab->anchorPoints_ListWidget);

    for (int i = 0; i < anchorPoints.size(); ++i)
    {
        const QSharedPointer<VGObject> anchorPoint = data->GetGObject(anchorPoints.at(i));
        box->addItem(anchorPoint->name(), anchorPoints.at(i));
    }

    const int index = uiPathsTab->nodes_ComboBox->findData(currentId);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitSAIncludeType()
{
    uiPathsTab->comboBoxIncludeType->clear();

    uiPathsTab->comboBoxIncludeType->addItem(tr("main path"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsMainPath));
    uiPathsTab->comboBoxIncludeType->addItem(tr("custom seam allowance"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsCustomSA));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitInternalPathsTab()
{
    uiPathsTab->internalPaths_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiPathsTab->internalPaths_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowInternalPathsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPatternPieceDataTab()
{
    uiLabelsTab->detailName_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->letter_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->annotation_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->orientation_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->rotation_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->tilt_LineEdit->setClearButtonEnabled(true);
    uiLabelsTab->foldPosition_LineEdit->setClearButtonEnabled(true);

    connect(uiLabelsTab->detailName_LineEdit, &QLineEdit::textChanged, this, &DialogSeamAllowance::NameDetailChanged);
    connect(uiLabelsTab->pushButtonEditPieceLabel, &QPushButton::clicked, this, &DialogSeamAllowance::EditLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitLabelsTab()
{
    m_dLabelWidthBaseHeight = uiLabelsTab->dLabelWidthFormula_LineEdit->height();
    m_dLabelHeightBaseHeight = uiLabelsTab->dLabelHeightFormula_LineEdit->height();
    m_dLabelAngleBaseHeight = uiLabelsTab->dLabelAngleFormula_LineEdit->height();

    connect(uiLabelsTab->pieceLabel_GroupBox, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledDetailLabel);
    initAnchorPoint(uiLabelsTab->dLabelCenterAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->dLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->dLabelBottomRightAnchor_ComboBox);

    connect(uiLabelsTab->dLabelTopLeftAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::pieceLabelAnchorChanged);

    connect(uiLabelsTab->dLabelBottomRightAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::pieceLabelAnchorChanged);

    connect(uiLabelsTab->dLabelWidth_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);
    connect(uiLabelsTab->dLabelHeght_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);
    connect(uiLabelsTab->dLabelAngle_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::EditDLFormula);

    connect(uiLabelsTab->dLabelWidthFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);
    connect(uiLabelsTab->dLabelHeightFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);
    connect(uiLabelsTab->dLabelAngleFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateDetailLabelValues);

    connect(uiLabelsTab->showDLWidth_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLWidth);
    connect(uiLabelsTab->showDLHeight_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLHeight);
    connect(uiLabelsTab->showDLAngle_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::DeployDLAngle);

    EnabledDetailLabel();

    m_pLabelWidthBaseHeight  = uiLabelsTab->pLabelWidthFormula_LineEdit->height();
    m_pLabelHeightBaseHeight = uiLabelsTab->pLabelHeightFormula_LineEdit->height();
    m_pLabelAngleBaseHeight  = uiLabelsTab->pLabelAngleFormula_LineEdit->height();

    connect(uiLabelsTab->patternLabel_GroupBox, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledPatternLabel);
    initAnchorPoint(uiLabelsTab->pLabelCenterAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->pLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->pLabelBottomRightAnchor_ComboBox);

    connect(uiLabelsTab->pLabelTopLeftAnchor_ComboBox,
            &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::patternLabelAnchorChanged);
    connect(uiLabelsTab->pLabelBottomRightAnchor_ComboBox,
            &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::patternLabelAnchorChanged);

    connect(uiLabelsTab->pLabelWidth_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);
    connect(uiLabelsTab->pLabelHeight_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);
    connect(uiLabelsTab->pLabelAngle_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::EditPLFormula);

    connect(uiLabelsTab->pLabelWidthFormula_LineEdit,  &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);
    connect(uiLabelsTab->pLabelHeightFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);
    connect(uiLabelsTab->pLabelAngleFormula_LineEdit,  &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdatePatternLabelValues);

    connect(uiLabelsTab->showPLabelWidth_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLWidth);
    connect(uiLabelsTab->showPLabelHeight_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLHeight);
    connect(uiLabelsTab->showPLabelAngle_PushButton,  &QPushButton::clicked, this, &DialogSeamAllowance::DeployPLAngle);

    uiLabelsTab->detailName_LineEdit->setText(createPieceName());
    EnabledPatternLabel();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitGrainlineTab()
{
    connect(uiGrainlineTab->grainline_GroupBox,  &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledGrainline);
    connect(uiGrainlineTab->rotation_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::EditGrainlineFormula);
    connect(uiGrainlineTab->length_PushButton,   &QPushButton::clicked, this, &DialogSeamAllowance::EditGrainlineFormula);
    connect(uiGrainlineTab->lengthFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateGrainlineValues);
    connect(uiGrainlineTab->rotationFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::UpdateGrainlineValues);

    connect(uiGrainlineTab->showRotation_PushButton, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployGrainlineRotation);
    connect(uiGrainlineTab->showLength_PushButton, &QPushButton::clicked, this,
            &DialogSeamAllowance::DeployGrainlineLength);

    EnabledGrainline();

    uiGrainlineTab->arrow_ComboBox->addItem(tr("Both"));
    uiGrainlineTab->arrow_ComboBox->addItem(tr("Just front"));
    uiGrainlineTab->arrow_ComboBox->addItem(tr("Just rear"));

    m_iRotBaseHeight = uiGrainlineTab->rotationFormula_LineEdit->height();
    m_iLenBaseHeight = uiGrainlineTab->lengthFormula_LineEdit->height();

    initAnchorPoint(uiGrainlineTab->centerGrainlineAnchor_ComboBox);
    initAnchorPoint(uiGrainlineTab->topGrainlineAnchor_ComboBox);
    initAnchorPoint(uiGrainlineTab->bottomGrainlineAnchor_ComboBox);

    connect(uiGrainlineTab->topGrainlineAnchor_ComboBox,
            &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::grainlineAnchorChanged);
    connect(uiGrainlineTab->bottomGrainlineAnchor_ComboBox,
            &QComboBox::currentTextChanged,
            this, &DialogSeamAllowance::grainlineAnchorChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::initializeAnchorsTab()
{
    uiAnchorPointsTab->anchorPoints_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiAnchorPointsTab->anchorPoints_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::showAnchorsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitNotchesTab()
{
    InitNotchesList();

    uiNotchesTab->notchLength_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchLength());
    uiNotchesTab->notchWidth_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchWidth());

    connect(uiNotchesTab->notches_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::notchChanged);

    connect(uiNotchesTab->notchType_ButtonGroup,  &QButtonGroup::idClicked,
            this, &DialogSeamAllowance::notchTypeChanged);

    connect(uiNotchesTab->notchSubType_ButtonGroup,  &QButtonGroup::idClicked,
            this, &DialogSeamAllowance::notchSubTypeChanged);

    connect(uiNotchesTab->showNotch_CheckBox, &QCheckBox::stateChanged, this,
            &DialogSeamAllowance::showNotchChanged);

    connect(uiNotchesTab->showSecondNotch_CheckBox, &QCheckBox::stateChanged, this,
            &DialogSeamAllowance::showSecondNotchChanged);

    connect(uiNotchesTab->notchLength_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DialogSeamAllowance::notchLengthChanged);

    connect(uiNotchesTab->resetLength_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::resetNotchLength);

    connect(uiNotchesTab->notchWidth_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DialogSeamAllowance::notchWidthChanged);

    connect(uiNotchesTab->resetWidth_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::resetNotchWidth);

    connect(uiNotchesTab->notchAngle_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DialogSeamAllowance::notchAngleChanged);

    connect(uiNotchesTab->resetAngle_PushButton, &QPushButton::clicked, this, &DialogSeamAllowance::resetNotchAngle);

    connect(uiNotchesTab->notchCount_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &DialogSeamAllowance::notchCountChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::initAnchorPointComboboxes()
{
    initAnchorPoint(uiGrainlineTab->centerGrainlineAnchor_ComboBox);
    initAnchorPoint(uiGrainlineTab->topGrainlineAnchor_ComboBox);
    initAnchorPoint(uiGrainlineTab->bottomGrainlineAnchor_ComboBox);

    initAnchorPoint(uiLabelsTab->dLabelCenterAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->dLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->dLabelBottomRightAnchor_ComboBox);

    initAnchorPoint(uiLabelsTab->pLabelCenterAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->pLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(uiLabelsTab->pLabelBottomRightAnchor_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSeamAllowance::GetFormulaSAWidth() const
{
    QString width = uiPathsTab->widthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(width, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetFormulaSAWidth(const QString &formula)
{
    const QString width = qApp->TrVars()->FormulaToUser(formula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployWidthFormulaTextEdit();
    }
    uiPathsTab->widthFormula_PlainTextEdit->setPlainText(width);

    VisToolPiece *path = qobject_cast<VisToolPiece *>(vis);
    SCASSERT(path != nullptr)
    const VPiece p = CreatePiece();
    path->SetPiece(p);

    MoveCursorToEnd(uiPathsTab->widthFormula_PlainTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateCurrentCustomSARecord()
{
    const int row = uiPathsTab->customSeamAllowance_ListWidget->currentRow();
    if (uiPathsTab->customSeamAllowance_ListWidget->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = uiPathsTab->customSeamAllowance_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const CustomSARecord record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));
    item->setText(GetPathName(record.path, record.reverse));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::UpdateCurrentInternalPathRecord()
{
    const int row = uiPathsTab->internalPaths_ListWidget->currentRow();
    if (uiPathsTab->internalPaths_ListWidget->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = uiPathsTab->internalPaths_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const quint32 path = qvariant_cast<quint32>(item->data(Qt::UserRole));
    item->setText(GetPathName(path));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetGrainlineAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployGrainlineRotation();
    }
    uiGrainlineTab->rotationFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiGrainlineTab->rotationFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetGrainlineLength(QString lengthFormula)
{
    if (lengthFormula.isEmpty())
    {
        lengthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(lengthFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployGrainlineLength();
    }

    uiGrainlineTab->lengthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiGrainlineTab->lengthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(widthFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLWidth();
    }

    uiLabelsTab->dLabelWidthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->dLabelWidthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(heightFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLHeight();
    }

    uiLabelsTab->dLabelHeightFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->dLabelHeightFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetDLAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployDLAngle();
    }

    uiLabelsTab->dLabelAngleFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->dLabelAngleFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(widthFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLWidth();
    }

    uiLabelsTab->pLabelWidthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->pLabelWidthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(heightFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLHeight();
    }

    uiLabelsTab->pLabelHeightFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->pLabelHeightFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::SetPLAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (formula.length() > 80)
    {
        this->DeployPLAngle();
    }

    uiLabelsTab->pLabelAngleFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(uiLabelsTab->pLabelAngleFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::showAnchorPoints()
{
    if (m_anchorPoints.isNull())
    {
        m_anchorPoints = new PieceAnchorPointVisual(data);
    }

    m_anchorPoints->setAnchors(GetListInternals<quint32>(uiAnchorPointsTab->anchorPoints_ListWidget));

    if (!qApp->getCurrentScene()->items().contains(m_anchorPoints))
    {
        m_anchorPoints->VisualMode(NULL_ID);
        m_anchorPoints->setZValue(10); // anchor points should be on top
        VToolSeamAllowance *tool = qobject_cast<VToolSeamAllowance*>(VAbstractPattern::getTool(toolId));
        SCASSERT(tool != nullptr);
        m_anchorPoints->setParentItem(tool);
    }
    else
    {
        m_anchorPoints->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::setMoveExclusions()
{
    uiPathsTab->moveTop_ToolButton->setEnabled(false);
    uiPathsTab->moveUp_ToolButton->setEnabled(false);
    uiPathsTab->moveDown_ToolButton->setEnabled(false);
    uiPathsTab->moveBottom_ToolButton->setEnabled(false);

    if (uiPathsTab->mainPath_ListWidget->count() > 1)
    {
        if (uiPathsTab->mainPath_ListWidget->currentRow() == 0)
        {
            uiPathsTab->moveDown_ToolButton->setEnabled(true);
            uiPathsTab->moveBottom_ToolButton->setEnabled(true);
        }
        else if (uiPathsTab->mainPath_ListWidget->currentRow() == uiPathsTab->mainPath_ListWidget->count() - 1)
        {
            uiPathsTab->moveTop_ToolButton->setEnabled(true);
            uiPathsTab->moveUp_ToolButton->setEnabled(true);
        }
        else
        {
            uiPathsTab->moveTop_ToolButton->setEnabled(true);
            uiPathsTab->moveUp_ToolButton->setEnabled(true);
            uiPathsTab->moveDown_ToolButton->setEnabled(true);
            uiPathsTab->moveBottom_ToolButton->setEnabled(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogSeamAllowance::createPieceName() const
{
    QList<VPiece> pieces = data->DataPieces()->values();
    QStringList pieceNames;

    for (int i = 0; i < pieces.size(); ++i)
    {
        pieceNames.append(pieces.at(i).GetName());
    }

    const QString defaultName = tr("PatternPiece");
    QString pieceName = defaultName;
    int i = 0;

    while(pieceNames.contains(pieceName))
    {
        pieceName = defaultName + QString("_%1").arg(++i);
    }
    return pieceName;
}

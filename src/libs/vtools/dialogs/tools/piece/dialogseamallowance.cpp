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
 **  @file   dialogseamallowance.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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
#include "ui_tabpins.h"
#include "ui_tabnotches.h"
#include "../vwidgets/fancytabbar/fancytabbar.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/calculator.h"
#include "visualization/path/vistoolpiece.h"
#include "visualization/path/vispiecepins.h"
#include "dialoginternalpath.h"
#include "../../../undocommands/savepiecepathoptions.h"
#include "../../support/dialogeditwrongformula.h"
#include "../../support/dialogeditlabel.h"
#include "../../../tools/vtoolseamallowance.h"

#include <QMenu>
#include <QTimer>
#include <QtNumeric>

enum TabOrder {Paths=0, Pins=1, Labels=2, Grainline=3, Notches=4, Count=5};

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
      uiPinsTab(new Ui::PinsTab),
      uiNotchesTab(new Ui::NotchesTab),
      m_pathsTab(new QWidget),
      m_labelsTab(new QWidget),
      m_grainlineTab(new QWidget),
      m_pinsTab(new QWidget),
      m_notchesTab(new QWidget),
      m_ftb(new FancyTabBar(FancyTabBar::Left, this)),
      applyAllowed(false),// By default disabled
      flagGPin(true),
      flagDPin(true),
      flagPPin(true),
      flagGFormulas(true),
      flagDLAngle(true),
      flagDLFormulas(true),
      flagPLAngle(true),
      flagPLFormulas(true),
      m_bAddMode(true),
      m_mx(0),
      m_my(0),
      m_dialog(),
      m_visPins(),
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
    InitPinsTab();
    InitNotchesTab();

    flagName = true;//We have default name of piece.
    ChangeColor(uiLabelsTab->labelEditName, okColor);
    flagError = MainPathIsValid();
    CheckState();

    if (not applyAllowed)
    {
        vis = new VisToolPiece(data);
    }

    m_ftb->SetCurrentIndex(TabOrder::Paths);// Show always first tab active on start.
}

//---------------------------------------------------------------------------------------------------------------------
DialogSeamAllowance::~DialogSeamAllowance()
{
    delete m_visPins;
    delete m_pathsTab;
    delete m_pinsTab;
    delete m_grainlineTab;
    delete m_labelsTab;
    delete m_notchesTab;
    delete uiNotchesTab;
    delete uiPinsTab;
    delete uiGrainlineTab;
    delete uiLabelsTab;
    delete uiPathsTab;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableApply(bool enable)
{
    SCASSERT(bApply != nullptr);
    bApply->setEnabled(enable);
    applyAllowed = enable;

    uiPathsTab->seamAllowance_Tab->setEnabled(applyAllowed);
    uiPathsTab->internalPaths_Tab->setEnabled(applyAllowed);
    m_ftb->SetTabEnabled(TabOrder::Pins, applyAllowed);
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

    uiPathsTab->hideMainPath_CheckBox->setChecked(piece.IsHideMainPath());
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

    uiPinsTab->pins_ListWidget->clear();
    for (int i = 0; i < piece.GetPins().size(); ++i)
    {
        NewPin(piece.GetPins().at(i));
    }

    InitAllPinComboboxes();

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

    uiLabelsTab->detailLabel_GroupBox->setChecked(m_oldData.IsVisible());
    ChangeCurrentData(uiLabelsTab->dLabelCenterPin_ComboBox, m_oldData.CenterPin());
    ChangeCurrentData(uiLabelsTab->dLabelTopLeftPin_ComboBox, m_oldData.TopLeftPin());
    ChangeCurrentData(uiLabelsTab->dLabelBottomRightPin_ComboBox, m_oldData.BottomRightPin());
    SetDLWidth(m_oldData.GetLabelWidth());
    SetDLHeight(m_oldData.GetLabelHeight());
    SetDLAngle(m_oldData.GetRotation());

    m_oldGeom = piece.GetPatternInfo();
    uiLabelsTab->patternLabel_GroupBox->setChecked(m_oldGeom.IsVisible());
    ChangeCurrentData(uiLabelsTab->pLabelCenterPin_ComboBox, m_oldGeom.CenterPin());
    ChangeCurrentData(uiLabelsTab->pLabelTopLeftPin_ComboBox, m_oldGeom.TopLeftPin());
    ChangeCurrentData(uiLabelsTab->pLabelBottomRightPin_ComboBox, m_oldGeom.BottomRightPin());
    SetPLWidth(m_oldGeom.GetLabelWidth());
    SetPLHeight(m_oldGeom.GetLabelHeight());
    SetPLAngle(m_oldGeom.GetRotation());

    m_oldGrainline = piece.GetGrainlineGeometry();
    uiGrainlineTab->grainline_GroupBox->setChecked(m_oldGrainline.IsVisible());
    ChangeCurrentData(uiGrainlineTab->grainlineCenterPin_ComboBox, m_oldGrainline.CenterPin());
    ChangeCurrentData(uiGrainlineTab->grainlineTopPin_ComboBox, m_oldGrainline.TopPin());
    ChangeCurrentData(uiGrainlineTab->grainlineBottomPin_ComboBox, m_oldGrainline.BottomPin());
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
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of objects (points, arcs, splines, spline paths)
 * @param type type of object
 */
void DialogSeamAllowance::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not prepare)
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

        if (not applyAllowed)
        {
            auto visPath = qobject_cast<VisToolPiece *>(vis);
            SCASSERT(visPath != nullptr);
            const VPiece p = CreatePiece();
            visPath->SetPiece(p);

            if (p.GetPath().CountNodes() == 1)
            {
                emit ToolTip(tr("Select main path objects clockwise, <b>Shift</b> - reverse direction curve, "
                                "<b>Enter</b> - finish creation"));

                if (not qApp->getCurrentScene()->items().contains(visPath))
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

        if (not applyAllowed)
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
    SCASSERT(bOk != nullptr);
    bOk->setEnabled(flagName && flagError && flagFormula && (flagGFormulas || flagGPin)
                    && flagDLAngle && (flagDLFormulas || flagDPin) && flagPLAngle && (flagPLFormulas || flagPPin));
    // In case dialog hasn't apply button
    if ( bApply != nullptr && applyAllowed)
    {
        bApply->setEnabled(bOk->isEnabled());
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

    const QSize sz = qApp->Settings()->GetToolSeamAllowanceDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (isInitialized)
    {
        qApp->Settings()->SetToolSeamAllowanceDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NameDetailChanged()
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
    {
        if (edit->text().isEmpty())
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
        rowNode.SetReverse(not rowNode.GetReverse());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
    else if (selectedAction == actionExcluded)
    {
        rowNode.SetExcluded(not rowNode.isExcluded());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
        rowItem->setFont(NodeFont(rowNode.isExcluded()));
    }
    else if (selectedAction == actionNotch)
    {
        rowNode.setNotch(not rowNode.isNotch());
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
        record.reverse = not record.reverse;
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
void DialogSeamAllowance::ShowPinsContextMenu(const QPoint &pos)
{
    const int row = uiPinsTab->pins_ListWidget->currentRow();
    if (uiPinsTab->pins_ListWidget->count() == 0 || row == -1 || row >= uiPinsTab->pins_ListWidget->count())
    {
        return;
    }

    QScopedPointer<QMenu> menu(new QMenu());
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(uiPinsTab->pins_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete uiPinsTab->pins_ListWidget->item(row);
        FancyTabChanged(m_ftb->CurrentIndex());
        InitAllPinComboboxes();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ListChanged()
{
    if (not applyAllowed)
    {
        auto visPath = qobject_cast<VisToolPiece *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPiece(CreatePiece());
        visPath->RefreshGeometry();
    }
    InitNodesList();
    InitNotchesList();
    customSeamAllowanceChanged(uiPathsTab->customSeamAllowance_ListWidget->currentRow());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnableSeamAllowance(bool enable)
{
    uiPathsTab->builtIn_CheckBox->setEnabled(enable);
    uiPathsTab->groupBoxAutomatic->setEnabled(enable);
    uiPathsTab->groupBoxCustom->setEnabled(enable);

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

            // Seam alowance before
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

            // Seam alowance after
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
        uiPathsTab->beforeWidthFormula_PlainTextEdit->setPlainText("");
        uiPathsTab->afterWidthFormula_PlainTextEdit->setPlainText("");
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

            // Sub type
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
    uiPathsTab->beforeWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ReturnDefAfter()
{
    uiPathsTab->afterWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
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

    const QListWidgetItem *item = uiPathsTab->customSeamAllowance_ListWidget->item( row );
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
        SCASSERT(not m_dialog.isNull());
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
    m_pinsTab->hide();
    m_notchesTab->hide();

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
    switch(index)
    {
        case TabOrder::Paths:
            m_pathsTab->show();
            break;
        case TabOrder::Pins:
            m_pinsTab->show();
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

    if (index == TabOrder::Pins || index == TabOrder::Grainline
            || (index == TabOrder::Labels &&
                uiLabelsTab->tabWidget->currentIndex() == uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab)))
    {
        ShowPins();
    }
    else
    {
        if (not m_visPins.isNull())
        {
            delete m_visPins;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::TabChanged(int index)
{
    if (index == uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab))
    {
        ShowPins();
    }
    else
    {
        if (not m_visPins.isNull())
        {
            delete m_visPins;
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
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toReal());
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
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toReal());
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
      QListWidgetItem *rowItem = GetItemById(uiNotchesTab->notches_ComboBox->currentData().toReal());
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
    QPlainTextEdit* apleSender[2] = {uiGrainlineTab->rotationFormula_LineEdit, uiGrainlineTab->lengthFormula_LineEdit};
    bool bFormulasOK[2] = {true, true};

    for (int i = 0; i < 2; ++i)
    {
        QLabel* plbVal;
        QLabel* plbText;
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
            not flagGPin ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagGFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagGFormulas && not flagGPin)
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
    QPlainTextEdit* apleSender[3] = {uiLabelsTab->dLabelWidthFormula_LineEdit, uiLabelsTab->dLabelHeightFormula_LineEdit,
                                     uiLabelsTab->dLabelAngleFormula_LineEdit};
    bool bFormulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel* plbVal;
        QLabel* plbText;
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
            not flagDPin ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagDLAngle = bFormulasOK[2];
    flagDLFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagDLAngle || not (flagDLFormulas || flagDPin) || not flagPLAngle || not (flagPLFormulas || flagPPin))
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
    QPlainTextEdit* apleSender[3] = {uiLabelsTab->pLabelWidthFormula_LineEdit, uiLabelsTab->pLabelHeightFormula_LineEdit,
                                     uiLabelsTab->pLabelAngleFormula_LineEdit};
    bool bFormulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel* plbVal;
        QLabel* plbText;
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
            not flagPPin ? ChangeColor(plbText, Qt::red) : ChangeColor(plbText, okColor);
            bFormulasOK[i] = false;
            plbVal->setToolTip(tr("Parser error: %1").arg(e.GetMsg()));
        }

        if (bFormulasOK[i] && qsVal.isEmpty() == false)
        {
            qsVal += qsUnit;
        }
        plbVal->setText(qsVal);
    }

    flagPLAngle = bFormulasOK[2];
    flagPLFormulas = bFormulasOK[0] && bFormulasOK[1];
    if (not flagDLAngle || not (flagDLFormulas || flagDPin) || not flagPLAngle || not (flagPLFormulas || flagPPin))
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
        GrainlinePinPointChanged();
    }
    else
    {
        flagGFormulas = true;
        ResetGrainlineWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EnabledDetailLabel()
{
    if (uiLabelsTab->detailLabel_GroupBox->isChecked() == true)
    {
        UpdateDetailLabelValues();
        DetailPinPointChanged();
    }
    else
    {
        flagDLAngle = true;
        flagDLFormulas = true;
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
        PatternPinPointChanged();
    }
    else
    {
        flagPLAngle = true;
        flagPLFormulas = true;
        ResetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EditGrainlineFormula()
{
    QPlainTextEdit* pleFormula;
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
    QPlainTextEdit* pleFormula;
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
    QPlainTextEdit* pleFormula;
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
    if (flagGFormulas || flagGPin)
    {
        m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ResetLabelsWarning()
{
    if (flagDLAngle && (flagDLFormulas || flagDPin) && flagPLAngle && (flagPLFormulas || flagPPin))
    {
        m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidth()
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

        EvalWidthBefore();
        EvalWidthAfter();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidthBefore()
{
    labelEditFormula = uiPathsTab->beforeWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const QString formula = uiPathsTab->beforeWidthFormula_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, uiPathsTab->beforeWidthResult_Label, postfix, false, true);

    const QString formulaSABefore = GetFormulaFromUser(uiPathsTab->beforeWidthFormula_PlainTextEdit);
    UpdateNodeSABefore(formulaSABefore);
    EnableDefButton(uiPathsTab->beforeDefault_PushButton, formulaSABefore);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::EvalWidthAfter()
{
    labelEditFormula = uiPathsTab->afterWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const QString formula = uiPathsTab->afterWidthFormula_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, uiPathsTab->afterWidthResult_Label, postfix, false, true);

    const QString formulaSAAfter = GetFormulaFromUser(uiPathsTab->afterWidthFormula_PlainTextEdit);
    UpdateNodeSAAfter(formulaSAAfter);
    EnableDefButton(uiPathsTab->afterDefault_PushButton, formulaSAAfter);
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
void DialogSeamAllowance::WidthChanged()
{
    labelEditFormula = uiPathsTab->widthEdit_Label;
    labelResultCalculation = uiPathsTab->widthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagFormula, uiPathsTab->widthFormula_PlainTextEdit, m_timerWidth, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::WidthBeforeChanged()
{
    labelEditFormula = uiPathsTab->beforeWidthEdit_Label;
    labelResultCalculation = uiPathsTab->beforeWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, uiPathsTab->beforeWidthFormula_PlainTextEdit, m_timerWidthBefore, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::WidthAfterChanged()
{
    labelEditFormula = uiPathsTab->afterWidthEdit_Label;
    labelResultCalculation = uiPathsTab->afterWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, uiPathsTab->afterWidthFormula_PlainTextEdit, m_timerWidthAfter, postfix);
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
void DialogSeamAllowance::GrainlinePinPointChanged()
{
    QColor color = okColor;
    const quint32 topPinId = getCurrentObjectId(uiGrainlineTab->grainlineTopPin_ComboBox);
    const quint32 bottomPinId = getCurrentObjectId(uiGrainlineTab->grainlineBottomPin_ComboBox);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagGPin = true;
        color = okColor;

        ResetGrainlineWarning();
    }
    else
    {
        flagGPin = false;
        topPinId == NULL_ID && bottomPinId == NULL_ID ? color = okColor : color = errorColor;

        if (not flagGFormulas && not flagGPin)
        {
            m_ftb->SetTabText(TabOrder::Grainline, tr("Grainline"));
        }
    }
    UpdateGrainlineValues();
    ChangeColor(uiGrainlineTab->labelGrainlineTopPin, color);
    ChangeColor(uiGrainlineTab->labelGrainlineBottomPin, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::DetailPinPointChanged()
{
    QColor color = okColor;
    const quint32 topPinId = getCurrentObjectId(uiLabelsTab->dLabelTopLeftPin_ComboBox);
    const quint32 bottomPinId = getCurrentObjectId(uiLabelsTab->dLabelBottomRightPin_ComboBox);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagDPin = true;
        color = okColor;

        if (flagPPin)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
        }
    }
    else
    {
        flagDPin = false;
        topPinId == NULL_ID && bottomPinId == NULL_ID ? color = okColor : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    UpdateDetailLabelValues();
    ChangeColor(uiLabelsTab->labelDLTopLeftPin, color);
    ChangeColor(uiLabelsTab->labelDLBottomRightPin, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::PatternPinPointChanged()
{
    QColor color = okColor;
    const quint32 topPinId = getCurrentObjectId(uiLabelsTab->pLabelTopLeftPin_ComboBox);
    const quint32 bottomPinId = getCurrentObjectId(uiLabelsTab->pLabelBottomRightPin_ComboBox);
    if (topPinId != NULL_ID && bottomPinId != NULL_ID && topPinId != bottomPinId)
    {
        flagPPin = true;
        color = okColor;

        if (flagDPin)
        {
            m_ftb->SetTabText(TabOrder::Labels, tr("Labels"));
            uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), QIcon());
        }
    }
    else
    {
        flagPPin = false;
        topPinId == NULL_ID && bottomPinId == NULL_ID ? color = okColor : color = errorColor;

        m_ftb->SetTabText(TabOrder::Labels, tr("Labels") + QLatin1String("*"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        uiLabelsTab->tabWidget->setTabIcon(uiLabelsTab->tabWidget->indexOf(uiLabelsTab->labels_Tab), icon);
    }
    UpdatePatternLabelValues();
    ChangeColor(uiLabelsTab->labelPLTopLeftPin, color);
    ChangeColor(uiLabelsTab->labelPLBottomRightPin, color);
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
    piece.SetPins(GetListInternals<quint32>(uiPinsTab->pins_ListWidget));
    piece.SetForbidFlipping(uiPathsTab->forbidFlipping_CheckBox->isChecked());
    piece.SetSeamAllowance(uiPathsTab->seams_CheckBox->isChecked());
    piece.SetSeamAllowanceBuiltIn(uiPathsTab->builtIn_CheckBox->isChecked());
    piece.SetHideMainPath(uiPathsTab->hideMainPath_CheckBox->isChecked());
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
    piece.GetPatternPieceData().SetFontSize(m_oldData.GetFontSize());
    piece.GetPatternPieceData().SetRotation(GetFormulaFromUser(uiLabelsTab->dLabelAngleFormula_LineEdit));
    piece.GetPatternPieceData().SetVisible(uiLabelsTab->detailLabel_GroupBox->isChecked());
    piece.GetPatternPieceData().SetCenterPin(getCurrentObjectId(uiLabelsTab->dLabelCenterPin_ComboBox));
    piece.GetPatternPieceData().SetTopLeftPin(getCurrentObjectId(uiLabelsTab->dLabelTopLeftPin_ComboBox));
    piece.GetPatternPieceData().SetBottomRightPin(getCurrentObjectId(uiLabelsTab->dLabelBottomRightPin_ComboBox));

    piece.GetPatternInfo() = m_oldGeom;
    piece.GetPatternInfo().SetVisible(uiLabelsTab->patternLabel_GroupBox->isChecked());
    piece.GetPatternInfo().SetCenterPin(getCurrentObjectId(uiLabelsTab->pLabelCenterPin_ComboBox));
    piece.GetPatternInfo().SetTopLeftPin(getCurrentObjectId(uiLabelsTab->pLabelTopLeftPin_ComboBox));
    piece.GetPatternInfo().SetBottomRightPin(getCurrentObjectId(uiLabelsTab->pLabelBottomRightPin_ComboBox));
    piece.GetPatternInfo().SetLabelWidth(GetFormulaFromUser(uiLabelsTab->pLabelWidthFormula_LineEdit));
    piece.GetPatternInfo().SetLabelHeight(GetFormulaFromUser(uiLabelsTab->pLabelHeightFormula_LineEdit));
    piece.GetPatternInfo().SetRotation(GetFormulaFromUser(uiLabelsTab->pLabelAngleFormula_LineEdit));

    piece.GetGrainlineGeometry() = m_oldGrainline;
    piece.GetGrainlineGeometry().SetVisible(uiGrainlineTab->grainline_GroupBox->isChecked());
    piece.GetGrainlineGeometry().SetRotation(GetFormulaFromUser(uiGrainlineTab->rotationFormula_LineEdit));
    piece.GetGrainlineGeometry().SetLength(GetFormulaFromUser(uiGrainlineTab->lengthFormula_LineEdit));
    piece.GetGrainlineGeometry().SetArrowType(static_cast<ArrowType>(uiGrainlineTab->arrow_ComboBox->currentIndex()));
    piece.GetGrainlineGeometry().SetCenterPin(getCurrentObjectId(uiGrainlineTab->grainlineCenterPin_ComboBox));
    piece.GetGrainlineGeometry().SetTopPin(getCurrentObjectId(uiGrainlineTab->grainlineTopPin_ComboBox));
    piece.GetGrainlineGeometry().SetBottomPin(getCurrentObjectId(uiGrainlineTab->grainlineBottomPin_ComboBox));

    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::NewMainPathItem(const VPieceNode &node)
{
    NewNodeItem(uiPathsTab->mainPath_ListWidget, node);
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
void DialogSeamAllowance::NewPin(quint32 pinPoint)
{
    if (pinPoint > NULL_ID)
    {
        const QSharedPointer<VGObject> pin = data->GetGObject(pinPoint);

        QListWidgetItem *item = new QListWidgetItem(pin->name());
        item->setFont(QFont("Times", 12, QFont::Bold));
        item->setData(Qt::UserRole, QVariant::fromValue(pinPoint));
        uiPinsTab->pins_ListWidget->addItem(item);
        uiPinsTab->pins_ListWidget->setCurrentRow(uiPinsTab->pins_ListWidget->count()-1);
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
    bool valid = true;

    if(CreatePiece().MainPathPoints(data).count() < 3)
    {
        url += tr("You need more points!");
        uiPathsTab->status_Label->setText(url);
        valid = false;
    }
    else
    {
        if(not MainPathIsClockwise())
        {
            url += tr("You have to choose points in a clockwise direction!");
            uiPathsTab->status_Label->setText(url);
            valid = false;
        }
        if (FirstPointEqualLast(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("First point cannot be equal to the last point!");
            uiPathsTab->status_Label->setText(url);
            valid = false;
        }
        else if (DoublePoints(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("You have double points!");
            uiPathsTab->status_Label->setText(url);
            valid = false;
        }
        else if (not EachPointLabelIsUnique(uiPathsTab->mainPath_ListWidget))
        {
            url += tr("Each point in the path must be unique!");
            uiPathsTab->status_Label->setText(url);
            valid = false;
        }
    }

    if (valid)
    {
        m_ftb->SetTabText(TabOrder::Paths, tr("Paths"));
        QString tooltip = tr("Ready!");
        if (not applyAllowed)
        {
            tooltip = tooltip + QLatin1String("  <b>") +
                    tr("To open all detail's features complete creating the main path.") + QLatin1String("</b>");
        }
        uiPathsTab->status_Label->setText(tooltip);
    }
    else
    {
        m_ftb->SetTabText(TabOrder::Paths, tr("Paths") + QLatin1String("*"));
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::ValidObjects(bool value)
{
    flagError = value;
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
        if (node.GetTypeTool() == Tool::NodePoint && not node.isExcluded())
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
    m_ftb->InsertTab(TabOrder::Paths, QIcon("://icon/32x32/paths.png"), tr("Paths"));
    m_ftb->InsertTab(TabOrder::Pins, QIcon("://icon/32x32/pins.png"), tr("Pins"));
    m_ftb->InsertTab(TabOrder::Labels, QIcon("://icon/32x32/labels.png"), tr("Labels"));
    m_ftb->InsertTab(TabOrder::Grainline, QIcon("://icon/32x32/grainline.png"), tr("Grainline"));
    m_ftb->InsertTab(TabOrder::Notches, QIcon("://icon/32x32/notches.png"), tr("Notches"));

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

    m_pinsTab->hide();
    uiPinsTab->setupUi(m_pinsTab);
    ui->horizontalLayout->addWidget(m_pinsTab, 1);

    m_notchesTab->hide();
    uiNotchesTab->setupUi(m_notchesTab);
    ui->horizontalLayout->addWidget(m_notchesTab, 1);

    connect(m_ftb, &FancyTabBar::CurrentChanged, this, &DialogSeamAllowance::FancyTabChanged);
    connect(uiLabelsTab->tabWidget, &QTabWidget::currentChanged, this, &DialogSeamAllowance::TabChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitMainPathTab()
{
    uiPathsTab->forbidFlipping_CheckBox->setChecked(qApp->Settings()->GetForbidWorkpieceFlipping());
    uiPathsTab->hideMainPath_CheckBox->setChecked(qApp->Settings()->IsHideMainPath());

    uiPathsTab->mainPath_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiPathsTab->mainPath_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowMainPathContextMenu);
    connect(uiPathsTab->mainPath_ListWidget->model(), &QAbstractItemModel::rowsMoved, this,
            &DialogSeamAllowance::ListChanged);

    connect(uiPathsTab->mainPath_ListWidget, &QListWidget::itemSelectionChanged,
            this, &DialogSeamAllowance::setMoveExclusions);
    connect(uiPathsTab->moveTop_ToolButton,    &QToolButton::clicked,
            this, [this](){moveListRowTop(uiPathsTab->mainPath_ListWidget);});
    connect(uiPathsTab->moveUp_ToolButton,     &QToolButton::clicked,
            this, [this](){moveListRowUp(uiPathsTab->mainPath_ListWidget);});
    connect(uiPathsTab->moveDown_ToolButton,   &QToolButton::clicked,
            this, [this](){moveListRowDown(uiPathsTab->mainPath_ListWidget);});
    connect(uiPathsTab->moveBottom_ToolButton, &QToolButton::clicked,
            this, [this](){moveListRowBottom(uiPathsTab->mainPath_ListWidget);});
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
    connect(m_timerWidth, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidth);

    m_timerWidthBefore = new QTimer(this);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidthBefore);

    m_timerWidthAfter = new QTimer(this);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogSeamAllowance::EvalWidthAfter);

    connect(uiPathsTab->seams_CheckBox, &QCheckBox::toggled, this, &DialogSeamAllowance::EnableSeamAllowance);

    // init the default seam allowance, convert the value if app unit is different than pattern unit
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
            &DialogSeamAllowance::WidthChanged);
    connect(uiPathsTab->beforeWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::WidthBeforeChanged);
    connect(uiPathsTab->afterWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogSeamAllowance::WidthAfterChanged);

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
        if (node.GetTypeTool() == Tool::NodePoint && not node.isExcluded())
        {
            const NodeInfo info = getNodeInfo(node);
            box->addItem(info.name, node.GetId());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPinPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);

    quint32 currentId = NULL_ID;
    if (box->count() > 0)
    {
        currentId = box->currentData().toUInt();
    }

    box->clear();
    box->addItem(QLatin1String("<") + tr("no pin") + QLatin1String(">"), NULL_ID);

    const QVector<quint32> pins = GetListInternals<quint32>(uiPinsTab->pins_ListWidget);

    for (int i = 0; i < pins.size(); ++i)
    {
        const QSharedPointer<VGObject> pin = data->GetGObject(pins.at(i));
        box->addItem(pin->name(), pins.at(i));
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

    connect(uiLabelsTab->detailLabel_GroupBox, &QGroupBox::toggled, this, &DialogSeamAllowance::EnabledDetailLabel);
    InitPinPoint(uiLabelsTab->dLabelCenterPin_ComboBox);
    InitPinPoint(uiLabelsTab->dLabelTopLeftPin_ComboBox);
    InitPinPoint(uiLabelsTab->dLabelBottomRightPin_ComboBox);

    connect(uiLabelsTab->dLabelTopLeftPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::DetailPinPointChanged);
    connect(uiLabelsTab->dLabelBottomRightPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::DetailPinPointChanged);

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
    InitPinPoint(uiLabelsTab->pLabelCenterPin_ComboBox);
    InitPinPoint(uiLabelsTab->pLabelTopLeftPin_ComboBox);
    InitPinPoint(uiLabelsTab->pLabelBottomRightPin_ComboBox);

    connect(uiLabelsTab->pLabelTopLeftPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::PatternPinPointChanged);
    connect(uiLabelsTab->pLabelBottomRightPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::PatternPinPointChanged);

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

    InitPinPoint(uiGrainlineTab->grainlineCenterPin_ComboBox);
    InitPinPoint(uiGrainlineTab->grainlineTopPin_ComboBox);
    InitPinPoint(uiGrainlineTab->grainlineBottomPin_ComboBox);

    connect(uiGrainlineTab->grainlineTopPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::GrainlinePinPointChanged);
    connect(uiGrainlineTab->grainlineBottomPin_ComboBox,
            static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::GrainlinePinPointChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitPinsTab()
{
    uiPinsTab->pins_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(uiPinsTab->pins_ListWidget, &QListWidget::customContextMenuRequested, this,
            &DialogSeamAllowance::ShowPinsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogSeamAllowance::InitNotchesTab()
{
    InitNotchesList();

    uiNotchesTab->notchLength_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchLength());
    uiNotchesTab->notchWidth_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchWidth());

    connect(uiNotchesTab->notches_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogSeamAllowance::notchChanged);

    connect(uiNotchesTab->notchType_ButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &DialogSeamAllowance::notchTypeChanged);
    connect(uiNotchesTab->notchSubType_ButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
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
void DialogSeamAllowance::InitAllPinComboboxes()
{
    InitPinPoint(uiGrainlineTab->grainlineCenterPin_ComboBox);
    InitPinPoint(uiGrainlineTab->grainlineTopPin_ComboBox);
    InitPinPoint(uiGrainlineTab->grainlineBottomPin_ComboBox);

    InitPinPoint(uiLabelsTab->dLabelCenterPin_ComboBox);
    InitPinPoint(uiLabelsTab->dLabelTopLeftPin_ComboBox);
    InitPinPoint(uiLabelsTab->dLabelBottomRightPin_ComboBox);

    InitPinPoint(uiLabelsTab->pLabelCenterPin_ComboBox);
    InitPinPoint(uiLabelsTab->pLabelTopLeftPin_ComboBox);
    InitPinPoint(uiLabelsTab->pLabelBottomRightPin_ComboBox);
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
void DialogSeamAllowance::ShowPins()
{
    if (m_visPins.isNull())
    {
        m_visPins = new VisPiecePins(data);
    }

    m_visPins->SetPins(GetListInternals<quint32>(uiPinsTab->pins_ListWidget));

    if (not qApp->getCurrentScene()->items().contains(m_visPins))
    {
        m_visPins->VisualMode(NULL_ID);
        m_visPins->setZValue(10); // pins should be on top
        VToolSeamAllowance *tool = qobject_cast<VToolSeamAllowance*>(VAbstractPattern::getTool(toolId));
        SCASSERT(tool != nullptr);
        m_visPins->setParentItem(tool);
    }
    else
    {
        m_visPins->RefreshGeometry();
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

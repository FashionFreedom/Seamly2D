/***************************************************************************
 **  @file   dialoginternalpath.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   dialoginternalpath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "dialoginternalpath.h"
#include "ui_dialoginternalpath.h"
#include "../vpatterndb/vpiecenode.h"
#include "visualization/path/vistoolinternalpath.h"
#include "../../../tools/vabstracttool.h"
#include "../../../tools/pattern_piece_tool.h"
#include "../../support/edit_formula_dialog.h"

#include <QMenu>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
DialogInternalPath::DialogInternalPath(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogInternalPath)
    , m_showMode(false)
    , m_saWidth(0)
    , m_timerWidth(nullptr)
    , m_timerWidthBefore(nullptr)
    , m_timerWidthAfter(nullptr)
    , m_widthFormula(0)
    , m_beforeWidthFormula(0)
    , m_afterWidthFormula(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/path.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    initializeOkCancel(ui);

    InitPathTab();
    InitSeamAllowanceTab();
    InitNotchesTab();

    flagName = true;//We have default name of piece.
    flagError = PathIsValid();
    CheckState();

    vis = new VisToolInternalPath(data);

    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->seamAllowance_Tab));
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->notches_Tab));

    connect(ui->piece_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        ValidObjects(PathIsValid());
    });
}

//---------------------------------------------------------------------------------------------------------------------
DialogInternalPath::~DialogInternalPath()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::EnbleShowMode(bool disable)
{
    m_showMode = disable;
    ui->type_ComboBox->setDisabled(m_showMode);
    ui->piece_ComboBox->setDisabled(m_showMode);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ChosenObject(quint32 id, const SceneObject &type)
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
                    NewItem(VPieceNode(id, Tool::NodeArc, reverse));
                    break;
                case SceneObject::ElArc:
                    NewItem(VPieceNode(id, Tool::NodeElArc, reverse));
                    break;
                case SceneObject::Point:
                    NewItem(VPieceNode(id, Tool::NodePoint));
                    break;
                case SceneObject::Spline:
                    NewItem(VPieceNode(id, Tool::NodeSpline, reverse));
                    break;
                case SceneObject::SplinePath:
                    NewItem(VPieceNode(id, Tool::NodeSplinePath, reverse));
                    break;
                case (SceneObject::Line):
                case (SceneObject::Piece):
                case (SceneObject::Unknown):
                default:
                    qWarning() << "Got wrong scene object. Ignore.";
                    break;
            }
        }
        else
        {
            if (ui->listWidget->count() > 1)
            {
                delete getItemById(id);
            }
        }

        ValidObjects(PathIsValid());

        if (not m_showMode)
        {
            auto visPath = qobject_cast<VisToolInternalPath *>(vis);
            SCASSERT(visPath != nullptr);
            const VPiecePath p = CreatePath();
            visPath->SetPath(p);

            if (p.CountNodes() == 1)
            {
                emit ToolTip(tr("Select main path objects, Use <b>SHIFT</b> to reverse curve direction, "
                                "Press <b>ENTER</b> to finish path creation "));

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
void DialogInternalPath::ShowDialog(bool click)
{
    if (click == false)
    {
        if (CreatePath().CountNodes() > 0)
        {
            emit ToolTip("");
            prepare = true;

            if (not m_showMode)
            {
                auto visPath = qobject_cast<VisToolInternalPath *>(vis);
                SCASSERT(visPath != nullptr);
                visPath->SetMode(Mode::Show);
                visPath->RefreshGeometry();
            }
            setModal(true);
            show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::CheckState()
{
    SCASSERT(ok_Button != nullptr);
    ok_Button->setEnabled(flagName && flagError);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ShowVisualization()
{
    AddVisualization<VisToolInternalPath>();

    if (m_showMode)
    {
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(GetPieceId()));
        SCASSERT(tool != nullptr);
        auto visPath = qobject_cast<VisToolInternalPath *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->setParentItem(tool);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::closeEvent(QCloseEvent *event)
{
    ui->widthFormula_PlainTextEdit->blockSignals(true);
    ui->beforeWidthFormula_PlainTextEdit->blockSignals(true);
    ui->afterWidthFormula_PlainTextEdit->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ShowContextMenu(const QPoint &pos)
{
    const int row = ui->listWidget->currentRow();
    if (ui->listWidget->count() == 0 || row == -1 || row >= ui->listWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->listWidget));

    NodeInfo info;

    QListWidgetItem *rowItem = ui->listWidget->item(row);
    SCASSERT(rowItem != nullptr);
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    //QAction *actionNotch = nullptr;
    QAction *actionReverse = nullptr;
    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        actionReverse = menu->addAction(tr("Reverse"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());
    }
    //else
    //{
    //    actionNotch = menu->addAction(tr("Notch"));
    //    actionNotch->setCheckable(true);
    //    actionNotch->setChecked(rowNode.isNotch());
    //}

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(ui->listWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->listWidget->item(row);
    }
    else if (rowNode.GetTypeTool() != Tool::NodePoint && selectedAction == actionReverse)
    {
        rowNode.SetReverse(not rowNode.GetReverse());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
    //else if (selectedAction == actionNotch)
    //{
    //    rowNode.setNotch(not rowNode.isNotch());
    //    info = getNodeInfo(rowNode, true);
    //    rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
    //    rowItem->setIcon(QIcon(info.icon));
    //    rowItem->setText(info.name);
    //}

    ValidObjects(PathIsValid());
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ListChanged()
{
    if (not m_showMode)
    {
        auto visPath = qobject_cast<VisToolInternalPath *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPath(CreatePath());
        visPath->RefreshGeometry();
    }

    initializeNotchesList();
    initializeNodesList();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::NameChanged()
{
    if (ui->pathName_LineEdit->text().isEmpty())
    {
        flagName = false;
        ChangeColor(ui->name_Label, Qt::red);
    }
    else
    {
        flagName = true;
        ChangeColor(ui->name_Label, okColor);
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::nodeChanged(int index)
{
    ui->beforeWidthFormula_PlainTextEdit->setDisabled(true);
    ui->beforeExpr_ToolButton->setDisabled(true);
    ui->beforeDefault_PushButton->setDisabled(true);

    ui->afterWidthFormula_PlainTextEdit->setDisabled(true);
    ui->afterExpr_ToolButton->setDisabled(true);
    ui->afterDefault_PushButton->setDisabled(true);

    ui->angle_ComboBox->setDisabled(true);

    ui->angle_ComboBox->blockSignals(true);

    if (index != -1)
    {
        const VPiecePath path = CreatePath();
        const int nodeIndex = path.indexOfNode(ui->nodes_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = path.at(nodeIndex);

            // Seam allowance before
            ui->beforeWidthFormula_PlainTextEdit->setEnabled(true);
            ui->beforeExpr_ToolButton->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            if (w1Formula != currentSeamAllowance)
            {
                ui->beforeDefault_PushButton->setEnabled(true);
            }
            if (w1Formula.length() > 80)// increase height if needed.
            {
                this->expandWidthBeforeFormulaTextEdit();
            }
            w1Formula = qApp->translateVariables()->FormulaToUser(w1Formula, qApp->Settings()->getOsSeparator());
            ui->beforeWidthFormula_PlainTextEdit->setPlainText(w1Formula);
            MoveCursorToEnd(ui->beforeWidthFormula_PlainTextEdit);

            // Seam allowance after
            ui->afterWidthFormula_PlainTextEdit->setEnabled(true);
            ui->afterExpr_ToolButton->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            if (w2Formula != currentSeamAllowance)
            {
                ui->afterDefault_PushButton->setEnabled(true);
            }
            if (w2Formula.length() > 80)// increase height if needed.
            {
                this->expandWidthAfterFormulaTextEdit();
            }
            w2Formula = qApp->translateVariables()->FormulaToUser(w2Formula, qApp->Settings()->getOsSeparator());
            ui->afterWidthFormula_PlainTextEdit->setPlainText(w2Formula);
            MoveCursorToEnd(ui->afterWidthFormula_PlainTextEdit);

            // Angle type
            ui->angle_ComboBox->setEnabled(true);
            const int index = ui->angle_ComboBox->findData(static_cast<unsigned char>(node.GetAngleType()));
            if (index != -1)
            {
                ui->angle_ComboBox->setCurrentIndex(index);
            }
        }
    }
    else
    {
        ui->beforeWidthFormula_PlainTextEdit->setPlainText("");
        ui->afterWidthFormula_PlainTextEdit->setPlainText("");
        ui->angle_ComboBox->setCurrentIndex(-1);
    }

    ui->angle_ComboBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::notchChanged(int index)
{
    ui->slitNotch_RadioButton->setDisabled(true);
    ui->tNotch_RadioButton->setDisabled(true);
    ui->uNotch_RadioButton->setDisabled(true);
    ui->vInternalNotch_RadioButton->setDisabled(true);
    ui->vExternalNotch_RadioButton->setDisabled(true);
    ui->castleNotch_RadioButton->setDisabled(true);
    ui->diamondNotch_RadioButton->setDisabled(true);

    ui->straightforward_RadioButton->setDisabled(true);
    ui->bisector_RadioButton->setDisabled(true);
    ui->intersection_RadioButton->setDisabled(true);

    ui->showSeamlineNotch_CheckBox->setDisabled(true);
    ui->showSeamlineNotch_CheckBox->blockSignals(true);

    ui->notchType_GroupBox->blockSignals(true);
    ui->notchSubType_GroupBox->blockSignals(true);

    if (index != -1)
    {
        const VPiecePath path = CreatePath();
        const int nodeIndex = path.indexOfNode(ui->notches_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = path.at(nodeIndex);

            // Notch type
            ui->slitNotch_RadioButton->setEnabled(true);
            ui->tNotch_RadioButton->setEnabled(true);
            ui->uNotch_RadioButton->setEnabled(true);
            ui->vInternalNotch_RadioButton->setEnabled(true);
            ui->vExternalNotch_RadioButton->setEnabled(true);
            ui->castleNotch_RadioButton->setEnabled(true);
            ui->diamondNotch_RadioButton->setEnabled(true);

            switch(node.getNotchType())
            {
                case NotchType::Slit:
                    ui->slitNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::TNotch:
                    ui->tNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::UNotch:
                    ui->uNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::VInternal:
                    ui->vInternalNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::VExternal:
                    ui->vExternalNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::Castle:
                    ui->castleNotch_RadioButton->setChecked(true);
                    break;
                case NotchType::Diamond:
                    ui->diamondNotch_RadioButton->setChecked(true);
                    break;
                default:
                    break;
            }

            // Sub type
            ui->straightforward_RadioButton->setEnabled(true);
            ui->bisector_RadioButton->setEnabled(true);
            ui->intersection_RadioButton->setEnabled(true);

            switch(node.getNotchSubType())
            {
                case NotchSubType::Straightforward:
                    ui->straightforward_RadioButton->setChecked(true);
                    break;
                case NotchSubType::Bisector:
                    ui->bisector_RadioButton->setChecked(true);
                    break;
                case NotchSubType::Intersection:
                    ui->intersection_RadioButton->setChecked(true);
                    break;
                default:
                    break;
            }
        }
    }
    ui->notchType_GroupBox->blockSignals(false);
    ui->notchSubType_GroupBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::enableDefaultBeforeButton()
{
    ui->beforeWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::enableDefaultAfterButton()
{
    ui->afterWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::notchTypeChanged(int id)
{
    const int i = ui->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            const NodeInfo info = getNodeInfo(rowNode, true);

            NotchType notchType = NotchType::Slit;
            if (id == ui->notchType_ButtonGroup->id(ui->slitNotch_RadioButton))
            {
                notchType = NotchType::Slit;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->tNotch_RadioButton))
            {
                notchType = NotchType::TNotch;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->uNotch_RadioButton))
            {
                notchType = NotchType::UNotch;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->vInternalNotch_RadioButton))
            {
                notchType = NotchType::VInternal;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->vExternalNotch_RadioButton))
            {
                notchType = NotchType::VExternal;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->castleNotch_RadioButton))
            {
                notchType = NotchType::Castle;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->diamondNotch_RadioButton))
            {
                notchType = NotchType::Diamond;
            }

            rowNode.setNotchType(notchType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setIcon(QIcon(info.icon));
            rowItem->setText(info.name);

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::notchSubTypeChanged(int id)
{
    const int i = ui->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchSubType notchSubType = NotchSubType::Straightforward;
            if (id == ui->notchSubType_ButtonGroup->id(ui->straightforward_RadioButton))
            {
                notchSubType = NotchSubType::Straightforward;
            }
            else if (id == ui->notchSubType_ButtonGroup->id(ui->bisector_RadioButton))
            {
                notchSubType = NotchSubType::Bisector;
            }
            else if (id == ui->notchSubType_ButtonGroup->id(ui->intersection_RadioButton))
            {
                notchSubType = NotchSubType::Intersection;
            }

            rowNode.setNotchSubType(notchSubType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            //rowItem->setText(getNodeInfo(rowNode, true));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::evaluateDefaultWidth()
{
    labelEditFormula = ui->widthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const QString formula = ui->widthFormula_PlainTextEdit->toPlainText();
    m_saWidth = Eval(formula, flagFormula, ui->widthResult_Label, postfix, false, true);

    if (m_saWidth >= 0)
    {
        VContainer *locData = const_cast<VContainer *> (data);
        locData->AddVariable(currentSeamAllowance, new CustomVariable(locData, currentSeamAllowance, 0, m_saWidth,
                                                                  QString().setNum(m_saWidth), true,
                                                                  tr("Current seam allowance")));

        evaluateBeforeWidth();
        evaluateAfterWidth();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::evaluateBeforeWidth()
{
    labelEditFormula = ui->beforeWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    QString formula = ui->beforeWidthFormula_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, ui->beforeWidthResult_Label, postfix, false, true);

    formula = getSeamAllowanceWidthFormulaBefore();
    if (formula != currentSeamAllowance)
    {
        ui->beforeDefault_PushButton->setEnabled(true);
    }

    updateNodeBeforeSeamAllowance(formula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::evaluateAfterWidth()
{
    labelEditFormula = ui->afterWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    QString formula = ui->afterWidthFormula_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, ui->afterWidthResult_Label, postfix, false, true);

    formula = getSeamAllowanceWidthFormulaAfter();
    if (formula != currentSeamAllowance)
    {
        ui->afterDefault_PushButton->setEnabled(true);
    }

    updateNodeAfterSeamAllowance(formula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::editDefaultSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit seam allowance width"));
    dialog->SetFormula(getSeamAllowanceWidthFormula());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setSeamAllowanceWidthFormula(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::editBeforeSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(getSeamAllowanceWidthFormulaBefore());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSABefore(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::editAfterSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, ToolDialog, this);
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(getSeamAllowanceWidthFormulaAfter());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setCurrentAfterSeamAllowance(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::defaultWidthChanged()
{
    labelEditFormula = ui->widthEdit_Label;
    labelResultCalculation = ui->widthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagFormula, ui->widthFormula_PlainTextEdit, m_timerWidth, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::beforeWidthChanged()
{
    labelEditFormula = ui->beforeWidthEdit_Label;
    labelResultCalculation = ui->beforeWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, ui->beforeWidthFormula_PlainTextEdit, m_timerWidthBefore, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::afterWidthChanged()
{
    labelEditFormula = ui->afterWidthEdit_Label;
    labelResultCalculation = ui->afterWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, ui->afterWidthFormula_PlainTextEdit, m_timerWidthAfter, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::expandWidthFormulaTextEdit()
{
    DeployFormula(ui->widthFormula_PlainTextEdit, ui->widthGrow_PushButton, m_widthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::expandWidthBeforeFormulaTextEdit()
{
    DeployFormula(ui->beforeWidthFormula_PlainTextEdit, ui->beforeWidthGrow_PushButton, m_beforeWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::expandWidthAfterFormulaTextEdit()
{
    DeployFormula(ui->afterWidthFormula_PlainTextEdit, ui->afterWidthGrow_PushButton, m_afterWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitPathTab()
{
    ui->pathName_LineEdit->setClearButtonEnabled(true);

    int index = ui->penType_ComboBox->findData(LineTypeNone);
    if (index != -1)
    {
        ui->penType_ComboBox->removeItem(index);
    }

    connect(ui->pathName_LineEdit, &QLineEdit::textChanged, this, &DialogInternalPath::NameChanged);

    InitPathTypes();
    connect(ui->type_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            [this]()
    {
        ui->penType_ComboBox->setEnabled(GetType() == PiecePathType::InternalPath);
        ui->cutOnFabric_CheckBox->setEnabled(GetType() == PiecePathType::InternalPath);
        ValidObjects(PathIsValid());
    });

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->listWidget, &QListWidget::customContextMenuRequested, this, &DialogInternalPath::ShowContextMenu);

    connect(ui->listWidget, &QListWidget::itemSelectionChanged, this, &DialogInternalPath::setMoveExclusions);
    connect(ui->moveTop_ToolButton,    &QToolButton::clicked, this, [this](){moveListRowTop(ui->listWidget);});
    connect(ui->moveUp_ToolButton,     &QToolButton::clicked, this, [this](){moveListRowTop(ui->listWidget);});
    connect(ui->moveDown_ToolButton,   &QToolButton::clicked, this, [this](){moveListRowDown(ui->listWidget);});
    connect(ui->moveBottom_ToolButton, &QToolButton::clicked, this, [this](){moveListRowBottom(ui->listWidget);});
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitSeamAllowanceTab()
{
    plainTextEditFormula = ui->widthFormula_PlainTextEdit;
    this->m_widthFormula = ui->widthFormula_PlainTextEdit->height();
    this->m_beforeWidthFormula = ui->beforeWidthFormula_PlainTextEdit->height();
    this->m_afterWidthFormula = ui->afterWidthFormula_PlainTextEdit->height();

    ui->widthFormula_PlainTextEdit->installEventFilter(this);
    ui->beforeWidthFormula_PlainTextEdit->installEventFilter(this);
    ui->afterWidthFormula_PlainTextEdit->installEventFilter(this);

    m_timerWidth = new QTimer(this);
    connect(m_timerWidth, &QTimer::timeout, this, &DialogInternalPath::evaluateDefaultWidth);

    m_timerWidthBefore = new QTimer(this);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogInternalPath::evaluateBeforeWidth);

    m_timerWidthAfter = new QTimer(this);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogInternalPath::evaluateAfterWidth);

    // Default value for seam allowence is 1 cm. But pattern have different units, so just set 1 in dialog not enough.
    m_saWidth = UnitConvertor(1, Unit::Cm, qApp->patternUnit());
    ui->widthFormula_PlainTextEdit->setPlainText(qApp->LocaleToString(m_saWidth));

    initializeNodesList();
    connect(ui->nodes_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogInternalPath::nodeChanged);

    connect(ui->beforeDefault_PushButton, &QPushButton::clicked, this, &DialogInternalPath::enableDefaultBeforeButton);
    connect(ui->afterDefault_PushButton, &QPushButton::clicked, this, &DialogInternalPath::enableDefaultAfterButton);

    initializeNodeAngles(ui->angle_ComboBox);
    connect(ui->angle_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogInternalPath::nodeAngleChanged);

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogInternalPath::editDefaultSeamAllowanceWidth);
    connect(ui->beforeExpr_ToolButton, &QPushButton::clicked, this, &DialogInternalPath::editBeforeSeamAllowanceWidth);
    connect(ui->afterExpr_ToolButton, &QPushButton::clicked, this, &DialogInternalPath::editAfterSeamAllowanceWidth);

    connect(ui->widthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this, &DialogInternalPath::defaultWidthChanged);
    connect(ui->beforeWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogInternalPath::beforeWidthChanged);
    connect(ui->afterWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogInternalPath::afterWidthChanged);

    connect(ui->widthGrow_PushButton, &QPushButton::clicked, this, &DialogInternalPath::expandWidthFormulaTextEdit);
    connect(ui->beforeWidthGrow_PushButton, &QPushButton::clicked,
            this, &DialogInternalPath::expandWidthBeforeFormulaTextEdit);
    connect(ui->afterWidthGrow_PushButton, &QPushButton::clicked, this,
            &DialogInternalPath::expandWidthAfterFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitNotchesTab()
{
    initializeNotchesList();
    connect(ui->notches_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogInternalPath::notchChanged);

    connect(ui->notchType_ButtonGroup,     &QButtonGroup::idClicked, this, &DialogInternalPath::notchTypeChanged);
    connect(ui->notchSubType_ButtonGroup,  &QButtonGroup::idClicked, this, &DialogInternalPath::notchSubTypeChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitPathTypes()
{
    ui->type_ComboBox->addItem(tr("Internal path"), static_cast<int>(PiecePathType::InternalPath));
    ui->type_ComboBox->addItem(tr("Custom seam allowance"), static_cast<int>(PiecePathType::CustomSeamAllowance));

    ui->penType_ComboBox->setEnabled(GetType() == PiecePathType::InternalPath);
    ui->cutOnFabric_CheckBox->setEnabled(GetType() == PiecePathType::InternalPath);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::initializeNodesList()
{
    const quint32 id = ui->nodes_ComboBox->currentData().toUInt();

    ui->nodes_ComboBox->blockSignals(true);
    ui->nodes_ComboBox->clear();

    const VPiecePath path = CreatePath();

    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode node = path.at(i);
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            const NodeInfo info = getNodeInfo(node);
            ui->nodes_ComboBox->addItem(info.name, node.GetId());
        }
    }
    ui->nodes_ComboBox->blockSignals(false);

    const int index = ui->nodes_ComboBox->findData(id);
    if (index != -1)
    {
        ui->nodes_ComboBox->setCurrentIndex(index);
        nodeChanged(index);// Need in case combox index was not changed
    }
    else
    {
        ui->nodes_ComboBox->count() > 0 ? nodeChanged(0) : nodeChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::initializeNotchesList()
{
    const quint32 id = ui->notches_ComboBox->currentData().toUInt();

    ui->notches_ComboBox->blockSignals(true);
    ui->notches_ComboBox->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->listWidget);

    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && node.isNotch())
        {
            const NodeInfo info = getNodeInfo(node);

            ui->notches_ComboBox->addItem(QIcon(info.icon), info.name, node.GetId());
        }
    }
    ui->notches_ComboBox->blockSignals(false);

    const int index = ui->notches_ComboBox->findData(id);
    if (index != -1)
    {
        ui->notches_ComboBox->setCurrentIndex(index);
        notchChanged(index);// Need in case combox index was not changed
    }
    else
    {
        ui->notches_ComboBox->count() > 0 ? notchChanged(0) : notchChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::nodeAngleChanged(int index)
{
    const int i = ui->nodes_ComboBox->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            const PieceNodeAngle angle = static_cast<PieceNodeAngle>(ui->angle_ComboBox->currentData().toUInt());
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetAngleType(angle);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath DialogInternalPath::GetPiecePath() const
{
    return CreatePath();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetPiecePath(const VPiecePath &path)
{
    ui->listWidget->clear();
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        NewItem(path.at(i));
    }

    SetType(path.GetType());
    ui->pathName_LineEdit->setText(path.GetName());

    VisToolInternalPath *visPath = qobject_cast<VisToolInternalPath *>(vis);
    SCASSERT(visPath != nullptr);
    visPath->SetPath(path);
    SetPenType(path.GetPenType());
    SetCutPath(path.IsCutPath());

    ValidObjects(PathIsValid());

    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
PiecePathType DialogInternalPath::GetType() const
{
    return static_cast<PiecePathType>(ui->type_ComboBox->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetType(PiecePathType type)
{
    const qint32 index = ui->type_ComboBox->findData(static_cast<int>(type));
    if (index != -1)
    {
        ui->type_ComboBox->setCurrentIndex(index);
    }

    ui->penType_ComboBox->setEnabled(type == PiecePathType::InternalPath);
    ui->cutOnFabric_CheckBox->setEnabled(type == PiecePathType::InternalPath);
}

//---------------------------------------------------------------------------------------------------------------------
Qt::PenStyle DialogInternalPath::GetPenType() const
{
    return lineTypeToPenStyle(GetComboBoxCurrentData(ui->penType_ComboBox, LineTypeSolidLine));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetPenType(const Qt::PenStyle &type)
{
    ChangeCurrentData(ui->penType_ComboBox, PenStyleToLineType(type));
    vis->setLineStyle(type);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogInternalPath::IsCutPath() const
{
    return ui->cutOnFabric_CheckBox->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetCutPath(bool value)
{
    ui->cutOnFabric_CheckBox->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
QListWidgetItem *DialogInternalPath::getItemById(quint32 id)
{
    for (qint32 i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));

        if (node.GetId() == id)
        {
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogInternalPath::GetLastId() const
{
    const int count = ui->listWidget->count();
    if (count > 0)
    {
        QListWidgetItem *item = ui->listWidget->item(count-1);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node.GetId();
    }
    else
    {
        return NULL_ID;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetCurrentSABefore(const QString &formula)
{
    updateNodeBeforeSeamAllowance(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::setCurrentAfterSeamAllowance(const QString &formula)
{
    updateNodeAfterSeamAllowance(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::updateNodeBeforeSeamAllowance(const QString &formula)
{
    const int index = ui->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setBeforeSAFormula(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::updateNodeAfterSeamAllowance(const QString &formula)
{
    const int index = ui->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setAfterSAFormula(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::setSeamAllowanceWidthFormula(const QString &formula)
{
    if (formula.isEmpty())
    {
        return;
    }

    const QString width = qApp->translateVariables()->FormulaToUser(formula, qApp->Settings()->getOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->expandWidthFormulaTextEdit();
    }
    ui->widthFormula_PlainTextEdit->setPlainText(width);

    VisToolInternalPath *path = qobject_cast<VisToolInternalPath *>(vis);
    SCASSERT(path != nullptr)
    path->SetPath(CreatePath());

    if (ui->tabWidget->indexOf(ui->seamAllowance_Tab) == -1)
    {
        ui->tabWidget->addTab(ui->seamAllowance_Tab, tr("Seam allowance"));
    }

    if (ui->tabWidget->indexOf(ui->notches_Tab) == -1)
    {
        ui->tabWidget->addTab(ui->notches_Tab, tr("Notches"));
    }

    MoveCursorToEnd(ui->widthFormula_PlainTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogInternalPath::GetPieceId() const
{
    return getCurrentObjectId(ui->piece_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetPieceId(quint32 id)
{
    if (ui->piece_ComboBox->count() <= 0)
    {
        ui->piece_ComboBox->addItem(data->GetPiece(id).GetName(), id);
    }
    else
    {
        const qint32 index = ui->piece_ComboBox->findData(id);
        if (index != -1)
        {
            ui->piece_ComboBox->setCurrentIndex(index);
        }
        else
        {
            ui->piece_ComboBox->setCurrentIndex(0);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogInternalPath::getSeamAllowanceWidthFormula() const
{
    QString width = ui->widthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->translateVariables()->TryFormulaFromUser(width, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->piece_ComboBox, list);
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath DialogInternalPath::CreatePath() const
{
    VPiecePath path;
    for (qint32 i = 0; i < ui->listWidget->count(); ++i)
    {
        QListWidgetItem *item = ui->listWidget->item(i);
        path.Append(qvariant_cast<VPieceNode>(item->data(Qt::UserRole)));
    }

    path.SetType(GetType());
    path.SetName(ui->pathName_LineEdit->text());
    path.SetPenType(GetType() == PiecePathType::InternalPath ? GetPenType() : Qt::SolidLine);
    path.SetCutPath(GetType() == PiecePathType::InternalPath ? IsCutPath() : false);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogInternalPath::PathIsValid() const
{
    QString url = DialogWarningIcon();

    if(CreatePath().PathPoints(data).count() < 2)
    {
        url += tr("You need more points!");
        ui->status_Label->setText(url);
        return false;
    }
    else
    {
        if (GetType() == PiecePathType::CustomSeamAllowance && isFirstPointSameAsLast(ui->listWidget))
        {
            url += tr("First point of <b>custom seam allowance</b> cannot be equal to the last point!");
            ui->status_Label->setText(url);
            return false;
        }
        else if (doublePointsExist(ui->listWidget))
        {
            url += tr("You have double points!");
            ui->status_Label->setText(url);
            return false;
        }
        else if (GetType() == PiecePathType::CustomSeamAllowance && not isEachPointNameUnique(ui->listWidget))
        {
            url += tr("Each point in the <b>custom seam allowance</b> path must be unique!");
            ui->status_Label->setText(url);
            return false;
        }
    }

    if (not m_showMode && ui->piece_ComboBox->count() <= 0)
    {
        url += tr("List of details is empty!");
        ui->status_Label->setText(url);
        return false;
    }
    else if (not m_showMode && ui->piece_ComboBox->currentIndex() == -1)
    {
        url += tr("Please, select a detail to insert into!");
        ui->status_Label->setText(url);
        return false;
    }

    ui->status_Label->setText(tr("Ready!"));
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ValidObjects(bool value)
{
    flagError = value;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::NewItem(const VPieceNode &node)
{
    newNodeItem(ui->listWidget, node);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogInternalPath::getSeamAllowanceWidthFormulaBefore() const
{
    QString width = ui->beforeWidthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->translateVariables()->TryFormulaFromUser(width, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogInternalPath::getSeamAllowanceWidthFormulaAfter() const
{
    QString width = ui->afterWidthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->translateVariables()->TryFormulaFromUser(width, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::setMoveExclusions()
{
    ui->moveTop_ToolButton->setEnabled(false);
    ui->moveUp_ToolButton->setEnabled(false);
    ui->moveDown_ToolButton->setEnabled(false);
    ui->moveBottom_ToolButton->setEnabled(false);

    if (ui->listWidget->count() > 1)
    {
        if (ui->listWidget->currentRow() == 0)
        {
            ui->moveDown_ToolButton->setEnabled(true);
            ui->moveBottom_ToolButton->setEnabled(true);
        }
        else if (ui->listWidget->currentRow() == ui->listWidget->count() - 1)
        {
            ui->moveTop_ToolButton->setEnabled(true);
            ui->moveUp_ToolButton->setEnabled(true);
        }
        else
        {
            ui->moveTop_ToolButton->setEnabled(true);
            ui->moveUp_ToolButton->setEnabled(true);
            ui->moveDown_ToolButton->setEnabled(true);
            ui->moveBottom_ToolButton->setEnabled(true);
        }
    }
}

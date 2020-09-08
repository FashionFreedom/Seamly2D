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
 **  @file   dialoginternalpath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#include "dialoginternalpath.h"
#include "ui_dialoginternalpath.h"
#include "../vpatterndb/vpiecenode.h"
#include "visualization/path/vistoolinternalpath.h"
#include "../../../tools/vabstracttool.h"
#include "../../../tools/vtoolseamallowance.h"
#include "../../support/dialogeditwrongformula.h"

#include <QMenu>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
DialogInternalPath::DialogInternalPath(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent),
      ui(new Ui::DialogInternalPath),
      m_showMode(false),
      m_saWidth(0),
      m_timerWidth(nullptr),
      m_timerWidthBefore(nullptr),
      m_timerWidthAfter(nullptr),
      m_widthFormula(0),
      m_beforeWidthFormula(0),
      m_afterWidthFormula(0)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/path.png"));

    InitOkCancel(ui);

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
                case (SceneObject::Detail):
                case (SceneObject::Unknown):
                default:
                    qDebug() << "Got wrong scene object. Ignore.";
                    break;
            }
        }
        else
        {
            if (ui->listWidget->count() > 1)
            {
                delete GetItemById(id);
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
    SCASSERT(bOk != nullptr);
    bOk->setEnabled(flagName && flagError);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ShowVisualization()
{
    AddVisualization<VisToolInternalPath>();

    if (m_showMode)
    {
        VToolSeamAllowance *tool = qobject_cast<VToolSeamAllowance*>(VAbstractPattern::getTool(GetPieceId()));
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
    ui->beforeWidthForumla_PlainTextEdit->blockSignals(true);
    ui->afterWidthForumla_PlainTextEdit->blockSignals(true);
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

    QScopedPointer<QMenu> menu(new QMenu());

    QListWidgetItem *rowItem = ui->listWidget->item(row);
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

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(ui->listWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->listWidget->item(row);
    }
    else if (rowNode.GetTypeTool() != Tool::NodePoint && selectedAction == actionReverse)
    {
        rowNode.SetReverse(not rowNode.GetReverse());
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setText(GetNodeName(rowNode, true));
    }
    else if (selectedAction == actionNotch)
    {
        rowNode.setNotch(not rowNode.isNotch());
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setText(GetNodeName(rowNode, true));
    }

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

    InitNotchesList();
    InitNodesList();
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
void DialogInternalPath::NodeChanged(int index)
{
    ui->beforeWidthForumla_PlainTextEdit->setDisabled(true);
    ui->beforeExpr_ToolButton->setDisabled(true);
    ui->beforeDefault_PushButton->setDisabled(true);

    ui->afterWidthForumla_PlainTextEdit->setDisabled(true);
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

            // Seam alowance before
            ui->beforeWidthForumla_PlainTextEdit->setEnabled(true);
            ui->beforeExpr_ToolButton->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            if (w1Formula != currentSeamAllowance)
            {
                ui->beforeDefault_PushButton->setEnabled(true);
            }
            if (w1Formula.length() > 80)// increase height if needed.
            {
                this->DeployWidthBeforeFormulaTextEdit();
            }
            w1Formula = qApp->TrVars()->FormulaToUser(w1Formula, qApp->Settings()->GetOsSeparator());
            ui->beforeWidthForumla_PlainTextEdit->setPlainText(w1Formula);
            MoveCursorToEnd(ui->beforeWidthForumla_PlainTextEdit);

            // Seam alowance after
            ui->afterWidthForumla_PlainTextEdit->setEnabled(true);
            ui->afterExpr_ToolButton->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            if (w2Formula != currentSeamAllowance)
            {
                ui->afterDefault_PushButton->setEnabled(true);
            }
            if (w2Formula.length() > 80)// increase height if needed.
            {
                this->DeployWidthAfterFormulaTextEdit();
            }
            w2Formula = qApp->TrVars()->FormulaToUser(w2Formula, qApp->Settings()->GetOsSeparator());
            ui->afterWidthForumla_PlainTextEdit->setPlainText(w2Formula);
            MoveCursorToEnd(ui->afterWidthForumla_PlainTextEdit);

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
        ui->beforeWidthForumla_PlainTextEdit->setPlainText("");
        ui->afterWidthForumla_PlainTextEdit->setPlainText("");
        ui->angle_ComboBox->setCurrentIndex(-1);
    }

    ui->angle_ComboBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::notchChanged(int index)
{
    ui->oneLine_RadioButton->setDisabled(true);
    ui->twoLines_RadioButton->setDisabled(true);
    ui->threeLines_RadioButton->setDisabled(true);
    ui->tMark_RadioButton->setDisabled(true);
    ui->vMark_RadioButton->setDisabled(true);

    ui->straightforward_RadioButton->setDisabled(true);
    ui->bisector_RadioButton->setDisabled(true);
    ui->intersection_RadioButton->setDisabled(true);

    ui->notchType_GroupBox->blockSignals(true);
    ui->notchSubType_GroupBox->blockSignals(true);

    if (index != -1)
    {
        const VPiecePath path = CreatePath();
        const int nodeIndex = path.indexOfNode(ui->notches_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = path.at(nodeIndex);

            // Line type
            ui->oneLine_RadioButton->setEnabled(true);
            ui->twoLines_RadioButton->setEnabled(true);
            ui->threeLines_RadioButton->setEnabled(true);
            ui->tMark_RadioButton->setEnabled(true);
            ui->vMark_RadioButton->setEnabled(true);

            switch(node.getNotchType())
            {
                case NotchType::OneLine:
                    ui->oneLine_RadioButton->setChecked(true);
                    break;
                case NotchType::TwoLines:
                    ui->twoLines_RadioButton->setChecked(true);
                    break;
                case NotchType::ThreeLines:
                    ui->threeLines_RadioButton->setChecked(true);
                    break;
                case NotchType::TMark:
                    ui->tMark_RadioButton->setChecked(true);
                    break;
                case NotchType::VMark:
                    ui->vMark_RadioButton->setChecked(true);
                    break;
                default:
                    break;
            }

            // Angle type
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
void DialogInternalPath::ReturnDefBefore()
{
    ui->beforeWidthForumla_PlainTextEdit->setPlainText(currentSeamAllowance);
    if (QPushButton* button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::ReturnDefAfter()
{
    ui->afterWidthForumla_PlainTextEdit->setPlainText(currentSeamAllowance);
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
        QListWidgetItem *rowItem = GetItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchType lineType = NotchType::OneLine;
            if (id == ui->notchType_ButtonGroup->id(ui->oneLine_RadioButton))
            {
                lineType = NotchType::OneLine;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->twoLines_RadioButton))
            {
                lineType = NotchType::TwoLines;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->threeLines_RadioButton))
            {
                lineType = NotchType::ThreeLines;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->tMark_RadioButton))
            {
                lineType = NotchType::TMark;
            }
            else if (id == ui->notchType_ButtonGroup->id(ui->vMark_RadioButton))
            {
                lineType = NotchType::VMark;
            }

            rowNode.setNotchLineType(lineType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(rowNode, true));

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
        QListWidgetItem *rowItem = GetItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchSubType angleType = NotchSubType::Straightforward;
            if (id == ui->notchSubType_ButtonGroup->id(ui->straightforward_RadioButton))
            {
                angleType = NotchSubType::Straightforward;
            }
            else if (id == ui->notchSubType_ButtonGroup->id(ui->bisector_RadioButton))
            {
                angleType = NotchSubType::Bisector;
            }
            else if (id == ui->notchSubType_ButtonGroup->id(ui->intersection_RadioButton))
            {
                angleType = NotchSubType::Intersection;
            }

            rowNode.setNotchAngleType(angleType);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setText(GetNodeName(rowNode, true));

            ListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::EvalWidth()
{
    labelEditFormula = ui->widthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    const QString formula = ui->widthFormula_PlainTextEdit->toPlainText();
    m_saWidth = Eval(formula, flagFormula, ui->widthResult_Label, postfix, false, true);

    if (m_saWidth >= 0)
    {
        VContainer *locData = const_cast<VContainer *> (data);
        locData->AddVariable(currentSeamAllowance, new VIncrement(locData, currentSeamAllowance, 0, m_saWidth,
                                                                  QString().setNum(m_saWidth), true,
                                                                  tr("Current seam aloowance")));

        EvalWidthBefore();
        EvalWidthAfter();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::EvalWidthBefore()
{
    labelEditFormula = ui->beforeWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    QString formula = ui->beforeWidthForumla_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, ui->beforeWidthResult_Label, postfix, false, true);

    formula = GetFormulaSAWidthBefore();
    if (formula != currentSeamAllowance)
    {
        ui->beforeDefault_PushButton->setEnabled(true);
    }

    UpdateNodeSABefore(formula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::EvalWidthAfter()
{
    labelEditFormula = ui->afterWidthEdit_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    QString formula = ui->afterWidthForumla_PlainTextEdit->toPlainText();
    bool flagFormula = false; // fake flag
    Eval(formula, flagFormula, ui->afterWidthResult_Label, postfix, false, true);

    formula = GetFormulaSAWidthAfter();
    if (formula != currentSeamAllowance)
    {
        ui->afterDefault_PushButton->setEnabled(true);
    }

    UpdateNodeSAAfter(formula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::FXWidth()
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
void DialogInternalPath::FXWidthBefore()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(GetFormulaSAWidthBefore());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSABefore(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::FXWidthAfter()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(GetFormulaSAWidthAfter());
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetCurrentSAAfter(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::WidthChanged()
{
    labelEditFormula = ui->widthEdit_Label;
    labelResultCalculation = ui->widthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagFormula, ui->widthFormula_PlainTextEdit, m_timerWidth, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::WidthBeforeChanged()
{
    labelEditFormula = ui->beforeWidthEdit_Label;
    labelResultCalculation = ui->beforeWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, ui->beforeWidthForumla_PlainTextEdit, m_timerWidthBefore, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::WidthAfterChanged()
{
    labelEditFormula = ui->afterWidthEdit_Label;
    labelResultCalculation = ui->afterWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    bool flagFormula = false;
    ValFormulaChanged(flagFormula, ui->afterWidthForumla_PlainTextEdit, m_timerWidthAfter, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::DeployWidthFormulaTextEdit()
{
    DeployFormula(ui->widthFormula_PlainTextEdit, ui->widthGrow_PushButton, m_widthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::DeployWidthBeforeFormulaTextEdit()
{
    DeployFormula(ui->beforeWidthForumla_PlainTextEdit, ui->beforeWidthGrow_PushButton, m_beforeWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::DeployWidthAfterFormulaTextEdit()
{
    DeployFormula(ui->afterWidthForumla_PlainTextEdit, ui->afterWidthGrow_PushButton, m_afterWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitPathTab()
{
    ui->pathName_LineEdit->setClearButtonEnabled(true);

    FillComboBoxTypeLine(ui->penType_ComboBox, CurvePenStylesPics());

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
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitSeamAllowanceTab()
{
    plainTextEditFormula = ui->widthFormula_PlainTextEdit;
    this->m_widthFormula = ui->widthFormula_PlainTextEdit->height();
    this->m_beforeWidthFormula = ui->beforeWidthForumla_PlainTextEdit->height();
    this->m_afterWidthFormula = ui->afterWidthForumla_PlainTextEdit->height();

    ui->widthFormula_PlainTextEdit->installEventFilter(this);
    ui->beforeWidthForumla_PlainTextEdit->installEventFilter(this);
    ui->afterWidthForumla_PlainTextEdit->installEventFilter(this);

    m_timerWidth = new QTimer(this);
    connect(m_timerWidth, &QTimer::timeout, this, &DialogInternalPath::EvalWidth);

    m_timerWidthBefore = new QTimer(this);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &DialogInternalPath::EvalWidthBefore);

    m_timerWidthAfter = new QTimer(this);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &DialogInternalPath::EvalWidthAfter);

    // Default value for seam allowence is 1 cm. But pattern have different units, so just set 1 in dialog not enough.
    m_saWidth = UnitConvertor(1, Unit::Cm, qApp->patternUnit());
    ui->widthFormula_PlainTextEdit->setPlainText(qApp->LocaleToString(m_saWidth));

    InitNodesList();
    connect(ui->nodes_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogInternalPath::NodeChanged);

    connect(ui->beforeDefault_PushButton, &QPushButton::clicked, this, &DialogInternalPath::ReturnDefBefore);
    connect(ui->afterDefault_PushButton, &QPushButton::clicked, this, &DialogInternalPath::ReturnDefAfter);

    InitNodeAngles(ui->angle_ComboBox);
    connect(ui->angle_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &DialogInternalPath::NodeAngleChanged);

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &DialogInternalPath::FXWidth);
    connect(ui->beforeExpr_ToolButton, &QPushButton::clicked, this, &DialogInternalPath::FXWidthBefore);
    connect(ui->afterExpr_ToolButton, &QPushButton::clicked, this, &DialogInternalPath::FXWidthAfter);

    connect(ui->widthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this, &DialogInternalPath::WidthChanged);
    connect(ui->beforeWidthForumla_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogInternalPath::WidthBeforeChanged);
    connect(ui->afterWidthForumla_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &DialogInternalPath::WidthAfterChanged);

    connect(ui->widthGrow_PushButton, &QPushButton::clicked, this, &DialogInternalPath::DeployWidthFormulaTextEdit);
    connect(ui->beforeWidthGrow_PushButton, &QPushButton::clicked,
            this, &DialogInternalPath::DeployWidthBeforeFormulaTextEdit);
    connect(ui->afterWidthGrow_PushButton, &QPushButton::clicked, this,
            &DialogInternalPath::DeployWidthAfterFormulaTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitNotchesTab()
{
    InitNotchesList();
    connect(ui->notches_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DialogInternalPath::notchChanged);

    connect(ui->notchType_ButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &DialogInternalPath::notchTypeChanged);
    connect(ui->notchSubType_ButtonGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked),
            this, &DialogInternalPath::notchSubTypeChanged);
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
void DialogInternalPath::InitNodesList()
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
            const QString name = GetNodeName(node);

            ui->nodes_ComboBox->addItem(name, node.GetId());
        }
    }
    ui->nodes_ComboBox->blockSignals(false);

    const int index = ui->nodes_ComboBox->findData(id);
    if (index != -1)
    {
        ui->nodes_ComboBox->setCurrentIndex(index);
        NodeChanged(index);// Need in case combox index was not changed
    }
    else
    {
        ui->nodes_ComboBox->count() > 0 ? NodeChanged(0) : NodeChanged(-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::InitNotchesList()
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
            const QString name = GetNodeName(node);

            ui->notches_ComboBox->addItem(name, node.GetId());
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
void DialogInternalPath::NodeAngleChanged(int index)
{
    const int i = ui->nodes_ComboBox->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->nodes_ComboBox->currentData().toUInt());
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
    return LineStyleToPenStyle(GetComboBoxCurrentData(ui->penType_ComboBox, TypeLineLine));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetPenType(const Qt::PenStyle &type)
{
    ChangeCurrentData(ui->penType_ComboBox, PenStyleToLineStyle(type));
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
QListWidgetItem *DialogInternalPath::GetItemById(quint32 id)
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
    UpdateNodeSABefore(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetCurrentSAAfter(const QString &formula)
{
    UpdateNodeSAAfter(formula);
    ListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::UpdateNodeSABefore(const QString &formula)
{
    const int index = ui->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSABefore(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::UpdateNodeSAAfter(const QString &formula)
{
    const int index = ui->nodes_ComboBox->currentIndex();
    if (index != -1)
    {
        QListWidgetItem *rowItem = GetItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetFormulaSAAfter(formula);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogInternalPath::SetFormulaSAWidth(const QString &formula)
{
    if (formula.isEmpty())
    {
        return;
    }

    const QString width = qApp->TrVars()->FormulaToUser(formula, qApp->Settings()->GetOsSeparator());
    // increase height if needed.
    if (width.length() > 80)
    {
        this->DeployWidthFormulaTextEdit();
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
QString DialogInternalPath::GetFormulaSAWidth() const
{
    QString width = ui->widthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(width, qApp->Settings()->GetOsSeparator());
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
        if (GetType() == PiecePathType::CustomSeamAllowance && FirstPointEqualLast(ui->listWidget))
        {
            url += tr("First point of <b>custom seam allowance</b> cannot be equal to the last point!");
            ui->status_Label->setText(url);
            return false;
        }
        else if (DoublePoints(ui->listWidget))
        {
            url += tr("You have double points!");
            ui->status_Label->setText(url);
            return false;
        }
        else if (GetType() == PiecePathType::CustomSeamAllowance && not EachPointLabelIsUnique(ui->listWidget))
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
    NewNodeItem(ui->listWidget, node);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogInternalPath::GetFormulaSAWidthBefore() const
{
    QString width = ui->beforeWidthForumla_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(width, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogInternalPath::GetFormulaSAWidthAfter() const
{
    QString width = ui->afterWidthForumla_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(width, qApp->Settings()->GetOsSeparator());
}

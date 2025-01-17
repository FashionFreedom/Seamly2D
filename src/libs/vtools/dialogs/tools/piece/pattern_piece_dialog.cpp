/***************************************************************************
 **  @file   pattern_piece_dialog.cpp
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
 **  @file   seamallowance.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
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
 **  along with Valentina.  if not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "pattern_piece_dialog.h"
#include "ui_pattern_piece_dialog.h"

#include "dialoginternalpath.h"
#include "vpointf.h"
#include "visualization/path/pieceanchorpoint_visual.h"
#include "visualization/path/pattern_piece_visual.h"
#include "../ifc/xml/vlabeltemplateconverter.h"
#include "../vformat/vlabeltemplate.h"
#include "../vmisc/logging.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../../support/edit_formula_dialog.h"
#include "../../support/editlabeltemplate_dialog.h"
#include "../../../tools/pattern_piece_tool.h"
#include "../../../undocommands/savepiecepathoptions.h"

#include <QColorDialog>
#include <QFileInfo>
#include <QLabel>
#include <QLayoutItem>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QPixmap>
#include <QScreen>
#include <QSound>
#include <QtDebug>
#include <QTimer>
#include <QtNumeric>

Q_LOGGING_CATEGORY(pattern_piece_dialog, "pattern_piece_dialog")

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void enableDefaultButton(QPushButton *defButton, const QString &formula)
{
    SCASSERT(defButton != nullptr)

    if (formula != currentSeamAllowance)
    {
        defButton->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString getFormulaFromUser(QPlainTextEdit *textEdit)
{
    SCASSERT(textEdit != nullptr)

    QString formula = textEdit->toPlainText();
    formula.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
}
}

//---------------------------------------------------------------------------------------------------------------------
PatternPieceDialog::PatternPieceDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::PatternPieceDialog)
    , applyAllowed(false)// By default disabled
    , flagGrainlineAnchor(true)
    , flagPieceLabelAnchor(true)
    , flagPatternLabelAnchor(true)
    , flagGrainlineFormula(true)
    , flagPieceLabelAngle(true)
    , flagPieceLabelFormula(true)
    , flagPatternLabelAngle(true)
    , flagPatternLabelFormula(true)
    , flagBeforeFormula(true)
    , flagAfterFormula(true)
    , flagMainPath(true)
    , m_bAddMode(true)
    , m_mx(0)
    , m_my(0)
    , m_inLayout(true)
    , m_isLocked(false)
    , m_dialog()
    , m_anchorPoints()
    , m_oldData()
    , m_oldGeom()
    , m_oldGrainline()
    , m_timerWidth(nullptr)
    , m_timerWidthBefore(nullptr)
    , m_timerWidthAfter(nullptr)
    , m_saWidth(0)
    , m_patternLabelLines()
    , m_pieceLabelLines()
    , m_beep(new QSound(qApp->Settings()->getSelectionSound()))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/new_piece.png"));

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    initializeOkCancelApply(ui);
    enableApply(applyAllowed);

    initializeMainPathTab();
    initializeSeamAllowanceTab();
    initializeInternalPathsTab();
    initializePropertiesTab();
    initializeLabelsTab();
    initializeGrainlineTab();
    initializeAnchorsTab();
    initializeNotchesTab();

    flagName = true;
    ChangeColor(ui->editName_Label, okColor);
    flagMainPath = isMainPathValid();
    CheckState();

    if (!applyAllowed && vis.isNull())
    {
        vis = new PatternPieceVisual(data);
    }

    // Always show Properties page on start.
    ui->pages_StackedWidget->setCurrentIndex(TabOrder::Properties);
    ui->menuTab_ListWidget->setCurrentRow(TabOrder::Properties);

    connect(ui->menuTab_ListWidget, &QListWidget::currentItemChanged, this, &PatternPieceDialog::pageChanged);
}

//---------------------------------------------------------------------------------------------------------------------
PatternPieceDialog::~PatternPieceDialog()
{
    delete m_anchorPoints;
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::pageChanged(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (current == nullptr)
    {
        current = previous;
    }
    int rowIndex = ui->menuTab_ListWidget->row(current);
    ui->pages_StackedWidget->setCurrentIndex(rowIndex);

    if (rowIndex == TabOrder::Labels || rowIndex == TabOrder::AnchorPoints || rowIndex == TabOrder::Grainline)
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
void PatternPieceDialog::enableApply(bool enable)
{
    SCASSERT(apply_Button != nullptr);
    apply_Button->setEnabled(enable);
    applyAllowed = enable;

    ui->internalPaths_Tab->setEnabled(applyAllowed);
    if (applyAllowed == true)
    {
        qCDebug(pattern_piece_dialog, "PatternPieceDialog applyAllowed = True");
        for (int i = TabOrder::SeamAllowance; i < TabOrder::Count; ++i)
        {
            auto flags = ui->menuTab_ListWidget->item(i)->flags();
            ui->menuTab_ListWidget->item(i)->setFlags(flags | Qt::ItemIsEnabled);
        }
      }
    else
    {
        qCDebug(pattern_piece_dialog, "PatternPieceDialog applyAllowed = False");
        for (int i = TabOrder::SeamAllowance; i < TabOrder::Count; ++i)
        {
            auto flags = ui->menuTab_ListWidget->item(i)->flags();
            ui->menuTab_ListWidget->item(i)->setFlags(flags & ~Qt::ItemIsEnabled);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiece PatternPieceDialog::GetPiece() const
{
    return CreatePiece();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::SetPiece(const VPiece &piece)
{
    ui->mainPath_ListWidget->clear();
    for (int i = 0; i < piece.GetPath().CountNodes(); ++i)
    {
        newMainPathItem(piece.GetPath().at(i));
    }

    ui->hideSeamLine_CheckBox->setChecked(piece.isHideSeamLine());
    ui->customSeamAllowance_ListWidget->blockSignals(true);
    ui->customSeamAllowance_ListWidget->clear();
    for (int i = 0; i < piece.GetCustomSARecords().size(); ++i)
    {
        newCustomSeamAllowance(piece.GetCustomSARecords().at(i));
    }
    ui->customSeamAllowance_ListWidget->blockSignals(false);

    ui->internalPaths_ListWidget->clear();
    for (int i = 0; i < piece.GetInternalPaths().size(); ++i)
    {
        newInternalPath(piece.GetInternalPaths().at(i));
    }

    ui->anchorPoints_ListWidget->clear();
    for (int i = 0; i < piece.getAnchors().size(); ++i)
    {
        newAnchorPoint(piece.getAnchors().at(i));
    }

    initAnchorPointComboboxes();

    ui->startPoint_ComboBox->blockSignals(true);
    ui->startPoint_ComboBox->clear();
    ui->startPoint_ComboBox->blockSignals(false);

    ui->endPoint_ComboBox->blockSignals(true);
    ui->endPoint_ComboBox->clear();
    ui->endPoint_ComboBox->blockSignals(false);

    customSeamAllowanceChanged(0);

    ui->forbidFlipping_CheckBox->setChecked(piece.IsForbidFlipping());
    ui->seams_CheckBox->setChecked(piece.IsSeamAllowance());
    ui->builtIn_CheckBox->setChecked(piece.IsSeamAllowanceBuiltIn());
    ui->pieceName_LineEdit->setText(piece.GetName());
    setPieceColor(piece.getColor());
    setPieceFill(piece.getFill());
    setInLayout(piece.isInLayout());
    setPieceLock(piece.isLocked());

    const QString width = qApp->TrVars()->FormulaToUser(piece.getSeamAllowanceWidthFormula(), qApp->Settings()->GetOsSeparator());
    ui->widthFormula_PlainTextEdit->setPlainText(width);
    m_saWidth = piece.GetSAWidth();

    m_mx = piece.GetMx();
    m_my = piece.GetMy();

    m_oldData = piece.GetPatternPieceData();
    ui->letter_LineEdit->setText(m_oldData.GetLetter());
    ui->annotation_LineEdit->setText(m_oldData.GetAnnotation());
    ui->orientation_ComboBox->setCurrentText(m_oldData.GetOrientation());
    ui->rotation_ComboBox->setCurrentText(m_oldData.GetRotationWay());
    ui->tilt_ComboBox->setCurrentText(m_oldData.GetTilt());
    ui->foldPosition_ComboBox->setCurrentText(m_oldData.GetFoldPosition());

    ui->quantity_SpinBox->setValue(m_oldData.GetQuantity());
    ui->fold_CheckBox->setChecked(m_oldData.IsOnFold());
    m_pieceLabelLines = m_oldData.GetLabelTemplate();

    ui->arrow_ComboBox->setCurrentIndex(int(piece.GetGrainlineGeometry().GetArrowType()));

    ui->pieceLabel_GroupBox->setChecked(m_oldData.IsVisible());
    ChangeCurrentData(ui->pieceLabelCenterAnchor_ComboBox, m_oldData.centerAnchorPoint());
    ChangeCurrentData(ui->pieceLabelTopLeftAnchor_ComboBox, m_oldData.topLeftAnchorPoint());
    ChangeCurrentData(ui->pieceLabelBottomRightAnchor_ComboBox, m_oldData.bottomRightAnchorPoint());
    setPieceLabelWidth(m_oldData.GetLabelWidth());
    setPieceLabelHeight(m_oldData.GetLabelHeight());
    setPieceLabelAngle(m_oldData.GetRotation());

    m_oldGeom = piece.GetPatternInfo();
    ui->patternLabel_GroupBox->setChecked(m_oldGeom.IsVisible());
    ChangeCurrentData(ui->patternLabelCenterAnchor_ComboBox, m_oldGeom.centerAnchorPoint());
    ChangeCurrentData(ui->patternLabelTopLeftAnchor_ComboBox, m_oldGeom.topLeftAnchorPoint());
    ChangeCurrentData(ui->patternLabelBottomRightAnchor_ComboBox, m_oldGeom.bottomRightAnchorPoint());
    setPatternLabelWidth(m_oldGeom.GetLabelWidth());
    setPatternLabelHeight(m_oldGeom.GetLabelHeight());
    setPatternLabelAngle(m_oldGeom.GetRotation());

    m_oldGrainline = piece.GetGrainlineGeometry();
    ui->grainline_GroupBox->setChecked(m_oldGrainline.IsVisible());
    ChangeCurrentData(ui->grainlineCenterAnchor_ComboBox, m_oldGrainline.centerAnchorPoint());
    ChangeCurrentData(ui->grainlineTopAnchor_ComboBox, m_oldGrainline.topAnchorPoint());
    ChangeCurrentData(ui->grainlineBottomAnchor_ComboBox, m_oldGrainline.bottomAnchorPoint());
    setGrainlineAngle(m_oldGrainline.GetRotation());
    setGrainlineLength(m_oldGrainline.GetLength());

    validateObjects(isMainPathValid());
    enabledGrainline();
    enabledPieceLabel();
    enabledPatternLabel();

    nodeListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
/**
* @brief ChosenObject adds selected object of id and type to list of path items.
* @param id id of object (points, arcs, splines, spline paths)
* @param type type of scene object
 */
void PatternPieceDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (!prepare)
    {
        bool reverse = false;
        const QSharedPointer<VGObject> obj = data->GetGObject(id);
        const GOType objType = static_cast<GOType>(obj->getType());
        quint32 previousId = getPreviousId();

        if (objType != GOType::Point && objType != GOType::Unknown)
        {
            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                reverse = true;
            }
            else if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
            {
                reverse = false;
            }
            else if (previousId == NULL_ID)
            {
                reverse = false;
            }
            else if (previousId != NULL_ID)
            {
                QPointF previousPoint;
                const QSharedPointer<VGObject> previousObj = data->GetGObject(previousId);
                const GOType previousObjType = static_cast<GOType>(previousObj->getType());
                if (previousObjType == GOType::Point)
                {
                    previousPoint = static_cast<QPointF>(*data->GeometricObject<VPointF>(previousId));
                }
                else
                {
                    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(previousId);
                    getLastNode().GetReverse() ? previousPoint = curve->getFirstPoint()
                                               : previousPoint = curve->getLastPoint();
                }

                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(id);
                if (curve->isPointOnCurve(previousPoint) &&
                    curve->getFirstPoint() != previousPoint &&
                    curve->getLastPoint() != previousPoint)
                {
                    QList<QListWidgetItem *> items = ui->mainPath_ListWidget->findItems(curve->name(),
                                                                                        Qt::MatchContains);

                    if (!items.isEmpty())
                    {
                        VPieceNode rowNode = qvariant_cast<VPieceNode>(items[0]->data(Qt::UserRole));
                        reverse = rowNode.GetReverse();
                    }
                }
                else if (curve->getFirstPoint() != previousPoint)
                {
                    reverse = true;
                }
            }
            else
            {
                QVector<QPointF> points = CreatePiece().MainPathPoints(data);
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(id);
                QVector<QPointF> curvePoints = curve->getPoints();
                points.append(curvePoints);
                reverse = !VPiece::isClockwise(points);
            }
        }

        if (id != previousId)
        {
            switch (type)
            {
                case SceneObject::Arc:
                    newMainPathItem(VPieceNode(id, Tool::NodeArc, reverse));
                    m_beep->play();
                    break;
                case SceneObject::ElArc:
                    newMainPathItem(VPieceNode(id, Tool::NodeElArc, reverse));
                    m_beep->play();
                    break;
                case SceneObject::Point:
                    newMainPathItem(VPieceNode(id, Tool::NodePoint));
                    m_beep->play();
                    break;
                case SceneObject::Spline:
                    newMainPathItem(VPieceNode(id, Tool::NodeSpline, reverse));
                    m_beep->play();
                    break;
                case SceneObject::SplinePath:
                    newMainPathItem(VPieceNode(id, Tool::NodeSplinePath, reverse));
                    m_beep->play();
                    break;
                case (SceneObject::Line):
                case (SceneObject::Piece):
                case (SceneObject::Unknown):
                default:
                    qWarning() << "Invalid scene object. Ignore.";
                    break;
            }
        }
        else
        {
            if (ui->mainPath_ListWidget->count() > 1)
            {
                QApplication::beep();
                delete getItemById(id);
            }
        }

        validateObjects(isMainPathValid());

        if (!applyAllowed)
        {
            auto visPath = qobject_cast<PatternPieceVisual *>(vis);
            SCASSERT(visPath != nullptr);
            const VPiece p = CreatePiece();
            visPath->SetPiece(p);

            if (p.GetPath().CountNodes() == 1)
            {
                emit ToolTip(tr("Select main path objects clockwise, Use <b>SHIFT</b> to reverse curve direction, "
                                " or <b>CTRL</b> to keep curve direction. "
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
void PatternPieceDialog::ShowDialog(bool click)
{
    if (click == false)
    {
        emit ToolTip("");
        prepare = true;

        if (!applyAllowed)
        {
            auto visPath = qobject_cast<PatternPieceVisual *>(vis);
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
void PatternPieceDialog::SaveData()
{}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::CheckState()
{
    SCASSERT(ok_Button != nullptr);
    SCASSERT(apply_Button != nullptr);
    ok_Button->setEnabled(flagName
                    && flagFormula
                    && flagMainPath
                    && (flagGrainlineFormula || flagGrainlineAnchor)
                    && flagPieceLabelAngle && (flagPieceLabelFormula || flagPieceLabelAnchor)
                    && flagPatternLabelAngle && (flagPatternLabelFormula || flagPatternLabelAnchor));

    if (applyAllowed)
    {
        ok_Button->setEnabled(ok_Button->isEnabled() && flagBeforeFormula && flagAfterFormula);
        apply_Button->setEnabled(ok_Button->isEnabled()); // In case dialog does not have an apply button
    }

    if (flagFormula && flagBeforeFormula && flagAfterFormula)
    {
        clearErrorText(TabOrder::SeamAllowance, tr("Seam Allowance "));
    }
    else
    {
        setErrorText(TabOrder::SeamAllowance, tr("Seam Allowance"));
    }

    if (flagMainPath)
    {
        clearErrorText(TabOrder::Paths, tr("Paths "));
        QString tooltip = tr("Ready!");
        if (!applyAllowed)
        {
            tooltip = tooltip + QLatin1String("  <b>") +
                    tr("Press OK to create pattern piece") + QLatin1String("</b>");
        }
        ui->status_Label->setText(tooltip);
    }
    else
    {
        setErrorText(TabOrder::Paths, tr("Paths"));
    }

}

void PatternPieceDialog::setErrorText(TabOrder tab, QString text)
{
    ui->menuTab_ListWidget->item(tab)->setText(text + QLatin1String("*"));
}

void PatternPieceDialog::clearErrorText(TabOrder tab, QString text)
{
    ui->menuTab_ListWidget->item(tab)->setText(text);
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Filters keyboard event to check if the a key was pressed.
 * @param object QObject that sent the event.
 * @param event QEvent.
 * @return if the node list was the sending object:
 *         True if the key(s) pressed makes up a shortcut sequence of a context menu item.
 *         False if any other key.
 *         if event sent by an object other than the main path list widget pass the event on to the parent.
 */
bool PatternPieceDialog::eventFilter(QObject *object, QEvent *event)
{
    if (QListWidget *list = qobject_cast<QListWidget *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            const int row = ui->mainPath_ListWidget->currentRow();
            if (ui->mainPath_ListWidget->count() <= 0 || row < 0 || row >= ui->mainPath_ListWidget->count())
            {
                return false;
            }
            QListWidgetItem *rowItem = list->item(row);
            SCASSERT(rowItem != nullptr);

            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == Qt::Key_Delete)
            {
                delete rowItem;
                return true;
            }
            if (keyEvent->modifiers() & Qt::ControlModifier)
            {
                switch (keyEvent->key())
                {
                    case Qt::Key_R:
                    {
                        reverseNode(rowItem);
                        return true;
                    }
                    case Qt::Key_D:
                    {
                        duplicateNode(rowItem);
                        return true;
                    }
                    case Qt::Key_E:
                    {
                        excludeNode(rowItem);
                        return true;
                    }
                }
            }
            else if (keyEvent->modifiers() & Qt::ShiftModifier)
            {
                VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
                NotchType notchType = rowNode.getNotchType();
                NotchSubType notchSubType = rowNode.getNotchSubType();
                switch (keyEvent->key())
                {
                    case Qt::Key_N:
                    {
                        setNotch(rowItem, false, NotchType::Slit, notchSubType);
                        return true;
                    }
                    case Qt::Key_S:
                    {
                        setNotch(rowItem, true, NotchType::Slit, notchSubType);
                        return true;
                    }
                    case Qt::Key_T:
                    {
                        setNotch(rowItem, true, NotchType::TNotch, notchSubType);
                        return true;
                    }
                    case Qt::Key_U:
                    {
                        setNotch(rowItem, true, NotchType::UNotch, notchSubType);
                        return true;
                    }
                    case Qt::Key_I:
                    {
                        setNotch(rowItem, true, NotchType::VInternal, notchSubType);
                        return true;
                    }
                    case Qt::Key_E:
                    {
                        setNotch(rowItem, true, NotchType::VExternal, notchSubType);
                        return true;
                    }
                    case Qt::Key_C:
                    {
                        setNotch(rowItem, true, NotchType::Castle, notchSubType);
                        return true;
                    }
                    case Qt::Key_D:
                    {
                        setNotch(rowItem, true, NotchType::Diamond, notchSubType);
                        return true;
                    }
                    case Qt::Key_F:
                    {
                        setNotch(rowItem, true, notchType, NotchSubType::Straightforward);
                        return true;
                    }
                    case Qt::Key_B:
                    {
                        setNotch(rowItem, true, notchType, NotchSubType::Bisector);
                        return true;
                    }
                    case Qt::Key_X:
                    {
                        setNotch(rowItem, true, notchType, NotchSubType::Intersection);
                        return true;
                    }
                }
            }
        }
    }
    else
    {
        return DialogTool::eventFilter(object, event);  // pass the event on to the parent class
    }
    return false; // pass the event to the widget
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::closeEvent(QCloseEvent *event)
{
    ui->widthFormula_PlainTextEdit->blockSignals(true);
    ui->beforeWidthFormula_PlainTextEdit->blockSignals(true);
    ui->afterWidthFormula_PlainTextEdit->blockSignals(true);
    ui->rotationFormula_LineEdit->blockSignals(true);
    ui->lengthFormula_LineEdit->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (isInitialized || event->spontaneous())
    {
        return;
    }

    const QSize size = qApp->Settings()->getPatternPieceDialogSize();
    if (!size.isEmpty())
    {
        resize(size);
    }

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::resizeEvent(QResizeEvent *event)
{
    // Remember the size for the next time this dialog is opened, but only
    // if the dialog was already initialized.
    if (isInitialized)
    {
        qApp->Settings()->setPatternPieceDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::pieceNameChanged()
{
    QLineEdit *name = qobject_cast<QLineEdit*>(sender());
    if (name)
    {
        if (name->text().isEmpty())
        {
            flagName = false;
            ChangeColor(ui->editName_Label, Qt::red);
            setErrorText(TabOrder::Properties, tr("Properties"));
        }
        else
        {
            flagName = true;
            ChangeColor(ui->editName_Label, okColor);
            clearErrorText(TabOrder::Properties, tr("Properties "));
        }
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::pieceColorChanged()
{
    const QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"), QColorDialog::DontUseNativeDialog);

    if (color.isValid())
    {
        ui->color_Label->setText(color.name());
        ui->color_Label->setPalette(QPalette(color));
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showMainPathContextMenu(const QPoint &pos)
{
    const int row = ui->mainPath_ListWidget->currentRow();
    if (ui->mainPath_ListWidget->count() == 0 || row == -1 || row >= ui->mainPath_ListWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->mainPath_ListWidget));
    NodeInfo info;
    NotchType notchType = NotchType::Slit;
    NotchSubType notchSubType = NotchSubType::Straightforward;
    bool isNotch = false;
    QListWidgetItem *rowItem = ui->mainPath_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

    QAction *actionNotch     = nullptr;
    QAction *actionNone      = nullptr;
    QAction *actionSlit      = nullptr;
    QAction *actionTNotch    = nullptr;
    QAction *actionUNotch    = nullptr;
    QAction *actionVInternal = nullptr;
    QAction *actionVExternal = nullptr;
    QAction *actionCastle    = nullptr;
    QAction *actionDiamond   = nullptr;

    QAction *actionStraightforward = nullptr;
    QAction *actionBisector        = nullptr;
    QAction *actionIntersection    = nullptr;

    QAction *actionReverse   = nullptr;
    QAction *actionDuplicate = nullptr;

    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        actionReverse = menu->addAction(tr("Reverse") + QStringLiteral("\tCtrl + R"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());

        actionDuplicate = menu->addAction(tr("Duplicate") + QStringLiteral("\tCtrl + D"));
    }
    else
    {
        QMenu *notchMenu = menu->addMenu(tr("Notch"));
        actionNotch = notchMenu->menuAction();
        actionNotch->setCheckable(true);
        actionNotch->setChecked(rowNode.isNotch());

        QMenu *notchTypeMenu = notchMenu->addMenu(tr("Type"));
        actionNone      = notchTypeMenu->addAction( tr("None") + QStringLiteral("\tShift + N"));
        actionSlit      = notchTypeMenu->addAction(QIcon("://icon/24x24/slit_notch.png"),       tr("Slit") + QStringLiteral("\tShift + S"));
        actionTNotch    = notchTypeMenu->addAction(QIcon("://icon/24x24/t_notch.png"),          tr("TNotch") + QStringLiteral("\tShift + T"));
        actionUNotch    = notchTypeMenu->addAction(QIcon("://icon/24x24/u_notch.png"),          tr("UNotch") + QStringLiteral("\tShift + U"));
        actionVInternal = notchTypeMenu->addAction(QIcon("://icon/24x24/internal_v_notch.png"), tr("VInternal") + QStringLiteral("\tShift + I"));
        actionVExternal = notchTypeMenu->addAction(QIcon("://icon/24x24/external_v_notch.png"), tr("VExternal") + QStringLiteral("\tShift + E"));
        actionCastle    = notchTypeMenu->addAction(QIcon("://icon/24x24/castle_notch.png"),     tr("Castle") + QStringLiteral("\tShift + C"));
        actionDiamond   = notchTypeMenu->addAction(QIcon("://icon/24x24/diamond_notch.png"),    tr("Diamond") + QStringLiteral("\tShift + D"));

        QMenu *notchSubtypeMenu = notchMenu->addMenu(tr("Subtype"));
        actionStraightforward = notchSubtypeMenu->addAction(QIcon(), tr("Straightforward") + QStringLiteral("\tShift + F"));
        actionBisector        = notchSubtypeMenu->addAction(QIcon(), tr("Bisector") + QStringLiteral("\tShift + B"));
        actionIntersection    = notchSubtypeMenu->addAction(QIcon(), tr("Intersection") + QStringLiteral("\tShift + X"));
    }

    QAction *actionExcluded = menu->addAction(tr("Excluded") + QStringLiteral("\tCtrl + E"));
    actionExcluded->setCheckable(true);
    actionExcluded->setChecked(rowNode.isExcluded());

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete") + QStringLiteral("\tDel"));

    QAction *selectedAction = menu->exec(ui->mainPath_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->mainPath_ListWidget->item(row);
    }
    else if (rowNode.GetTypeTool() != Tool::NodePoint && selectedAction == actionReverse)
    {
        reverseNode(rowItem);
    }
    else if (rowNode.GetTypeTool() != Tool::NodePoint && selectedAction == actionDuplicate)
    {
        duplicateNode(rowItem);
    }
    else if (selectedAction == actionExcluded)
    {
        excludeNode(rowItem);
    }
    else
    {
        if (selectedAction == actionNone)
        {
            isNotch = false;
            notchType = NotchType::Slit;
        }
        else if (selectedAction == actionSlit)
        {
            isNotch = true;
            notchType = NotchType::Slit;
        }
        else if (selectedAction == actionTNotch)
        {
            isNotch = true;
            notchType = NotchType::TNotch;
        }
        else if (selectedAction == actionUNotch)
        {
            isNotch = true;
            notchType = NotchType::UNotch;
        }
        else if (selectedAction == actionVInternal)
        {
            isNotch = true;
            notchType = NotchType::VInternal;
        }
        else if (selectedAction == actionVExternal)
        {
            isNotch = true;
            notchType = NotchType::VExternal;
        }
        else if (selectedAction == actionCastle)
        {
            isNotch = true;
            notchType = NotchType::Castle;
        }
        else if (selectedAction == actionDiamond)
        {
            isNotch = true;
            notchType = NotchType::Diamond;
        }
        else if (selectedAction == actionStraightforward)
        {
            isNotch = true;
            notchSubType = NotchSubType::Straightforward;
        }
        else if (selectedAction == actionBisector)
        {
            isNotch = true;
            notchSubType = NotchSubType::Bisector;
        }
        else if (selectedAction == actionIntersection)
        {
            isNotch = true;
            notchSubType = NotchSubType::Intersection;
        }

        setNotch(rowItem, isNotch, notchType, notchSubType);
    }

    validateObjects(isMainPathValid());
    nodeListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showCustomSAContextMenu(const QPoint &pos)
{
    const int row = ui->customSeamAllowance_ListWidget->currentRow();
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= ui->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->customSeamAllowance_ListWidget));
    QAction *actionOption = menu->addAction(QIcon::fromTheme("preferences-other"), tr("Options"));

    QListWidgetItem *rowItem = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));

    QAction *actionReverse = menu->addAction(tr("Reverse"));
    actionReverse->setCheckable(true);
    actionReverse->setChecked(record.reverse);

    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(ui->customSeamAllowance_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->customSeamAllowance_ListWidget->item(row);
    }
    else if (selectedAction == actionReverse)
    {
        record.reverse = !record.reverse;
        rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
        rowItem->setText(getPathName(record.path, record.reverse));
    }
    else if (selectedAction == actionOption)
    {
        auto *dialog = new DialogInternalPath(data, record.path, this);
        dialog->SetPiecePath(data->GetPiecePath(record.path));
        dialog->SetPieceId(toolId);
        if (record.includeType == PiecePathIncludeType::AsMainPath)
        {
            dialog->setSeamAllowanceWidthFormula(getSeamAllowanceWidthFormula());
        }
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &PatternPieceDialog::pathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showInternalPathsContextMenu(const QPoint &pos)
{
    const int row = ui->internalPaths_ListWidget->currentRow();
    if (ui->internalPaths_ListWidget->count() == 0 || row == -1
            || row >= ui->internalPaths_ListWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->internalPaths_ListWidget));
    QAction *actionOption = menu->addAction(QIcon::fromTheme("preferences-other"), tr("Options"));
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(ui->internalPaths_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->internalPaths_ListWidget->item(row);
    }
    else if (selectedAction == actionOption)
    {
        QListWidgetItem *rowItem = ui->internalPaths_ListWidget->item(row);
        SCASSERT(rowItem != nullptr);
        const quint32 pathId = qvariant_cast<quint32>(rowItem->data(Qt::UserRole));

        auto *dialog = new DialogInternalPath(data, pathId, this);
        dialog->SetPiecePath(data->GetPiecePath(pathId));
        dialog->SetPieceId(toolId);
        dialog->EnbleShowMode(true);
        m_dialog = dialog;
        m_dialog->setModal(true);
        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &PatternPieceDialog::pathDialogClosed);
        m_dialog->show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showAnchorsContextMenu(const QPoint &pos)
{
    const int row = ui->anchorPoints_ListWidget->currentRow();
    if (ui->anchorPoints_ListWidget->count() == 0 || row == -1 || row >= ui->anchorPoints_ListWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->anchorPoints_ListWidget));
    QAction *actionDelete = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete"));

    QAction *selectedAction = menu->exec(ui->anchorPoints_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionDelete)
    {
        delete ui->anchorPoints_ListWidget->item(row);
        initAnchorPointComboboxes();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::nodeListChanged()
{
    if (!applyAllowed)
    {
        auto visPath = qobject_cast<PatternPieceVisual *>(vis);
        SCASSERT(visPath != nullptr);
        visPath->SetPiece(CreatePiece());
        visPath->RefreshGeometry();
    }
    initializeNodesList();
    initializeNotchesList();
    customSeamAllowanceChanged(ui->customSeamAllowance_ListWidget->currentRow());
    setMoveExclusions();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enableSeamAllowance(bool enable)
{
    ui->builtIn_CheckBox->setEnabled(enable);
    ui->hideSeamLine_CheckBox->setEnabled(enable);

    if (!enable)
    {
        ui->autoCustom_TabWidget->setEnabled(enable);
    }
    else
    {
        ui->builtIn_CheckBox->toggled(ui->builtIn_CheckBox->isChecked());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enableBuiltIn(bool enable)
{
    ui->autoCustom_TabWidget->setEnabled(!enable);

    if (enable)
    {
        initializeNodesList();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::nodeChanged(int index)
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
        const VPiece piece = CreatePiece();
        const int nodeIndex = piece.GetPath().indexOfNode(ui->nodes_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = piece.GetPath().at(nodeIndex);

            // Seam allowance before node
            ui->beforeWidthFormula_PlainTextEdit->setEnabled(true);
            ui->beforeExpr_ToolButton->setEnabled(true);

            QString w1Formula = node.GetFormulaSABefore();
            enableDefaultButton(ui->beforeDefault_PushButton, w1Formula);
            w1Formula = qApp->TrVars()->FormulaToUser(w1Formula, qApp->Settings()->GetOsSeparator());

            ui->beforeWidthFormula_PlainTextEdit->setPlainText(w1Formula);
            MoveCursorToEnd(ui->beforeWidthFormula_PlainTextEdit);

            // Seam allowance after node
            ui->afterWidthFormula_PlainTextEdit->setEnabled(true);
            ui->afterExpr_ToolButton->setEnabled(true);

            QString w2Formula = node.GetFormulaSAAfter();
            enableDefaultButton(ui->afterDefault_PushButton, w2Formula);
            w2Formula = qApp->TrVars()->FormulaToUser(w2Formula, qApp->Settings()->GetOsSeparator());

            ui->afterWidthFormula_PlainTextEdit->setPlainText(w2Formula);
            MoveCursorToEnd(ui->afterWidthFormula_PlainTextEdit);

            //Angle type
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
        ui->beforeWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
        ui->afterWidthFormula_PlainTextEdit->setPlainText(currentSeamAllowance);
        ui->angle_ComboBox->setCurrentIndex(-1);
    }
    ui->angle_ComboBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchChanged(int index)
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

    ui->showNotch_CheckBox->setDisabled(true);
    ui->showNotch_CheckBox->blockSignals(true);

    ui->showSeamlineNotch_CheckBox->setDisabled(true);
    ui->showSeamlineNotch_CheckBox->blockSignals(true);

    ui->notchType_GroupBox->blockSignals(true);
    ui->notchSubType_GroupBox->blockSignals(true);

    if (index != -1)
    {
        const VPiece piece = CreatePiece();
        const int nodeIndex = piece.GetPath().indexOfNode(ui->notches_ComboBox->currentData().toUInt());
        if (nodeIndex != -1)
        {
            const VPieceNode &node = piece.GetPath().at(nodeIndex);

            // Notch type
            ui->slitNotch_RadioButton->setEnabled(true);
            ui->tNotch_RadioButton->setEnabled(true);
            ui->uNotch_RadioButton->setEnabled(true);
            ui->vInternalNotch_RadioButton->setEnabled(true);
            ui->vExternalNotch_RadioButton->setEnabled(true);
            ui->castleNotch_RadioButton->setEnabled(true);
            ui->diamondNotch_RadioButton->setEnabled(true);

            ui->notchAngle_DoubleSpinBox->setEnabled(false);

            switch(node.getNotchType())
            {
                case NotchType::Slit:
                    ui->slitNotch_RadioButton->setChecked(true);
                    ui->notchAngle_DoubleSpinBox->setEnabled(true);
                    ui->notchAngle_DoubleSpinBox->setValue(node.getNotchAngle());
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

            // Notch Sub type
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

            // Show the seam allowance notch
            ui->showNotch_CheckBox->setEnabled(true);
            ui->showNotch_CheckBox->setChecked(node.showNotch());

            // Show the seam line notch
            ui->showSeamlineNotch_CheckBox->setEnabled(true);
            ui->showSeamlineNotch_CheckBox->setChecked(node.showSeamlineNotch());

            ui->notchLength_DoubleSpinBox->setEnabled(true);
            ui->notchLength_DoubleSpinBox->setValue(node.getNotchLength());

            ui->notchWidth_DoubleSpinBox->setEnabled(true);
            ui->notchWidth_DoubleSpinBox->setValue(node.getNotchWidth());

            ui->notchCount_SpinBox->setEnabled(true);
            ui->notchCount_SpinBox->setValue(node.getNotchCount());
        }
    }

    ui->showNotch_CheckBox->blockSignals(false);
    ui->showSeamlineNotch_CheckBox->blockSignals(false);

    ui->notchType_GroupBox->blockSignals(false);
    ui->notchSubType_GroupBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::customSAStartPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = ui->customSeamAllowance_ListWidget->currentRow();
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= ui->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.startPoint = ui->startPoint_ComboBox->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::customSAEndPointChanged(int index)
{
    Q_UNUSED(index);

    const int row = ui->customSeamAllowance_ListWidget->currentRow();
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= ui->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.endPoint = ui->endPoint_ComboBox->currentData().toUInt();
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::customSAIncludeTypeChanged(int index)
{
    Q_UNUSED(index);

    const int row = ui->customSeamAllowance_ListWidget->currentRow();
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= ui->customSeamAllowance_ListWidget->count())
    {
        return;
    }

    QListWidgetItem *rowItem = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(rowItem != nullptr);
    CustomSARecord record = qvariant_cast<CustomSARecord>(rowItem->data(Qt::UserRole));
    record.includeType = static_cast<PiecePathIncludeType>(ui->comboBoxIncludeType->currentData().toUInt());
    rowItem->setData(Qt::UserRole, QVariant::fromValue(record));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::nodeAngleChanged(int index)
{
    const int i = ui->nodes_ComboBox->currentIndex();
    if (i != -1 && index != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->nodes_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.SetAngleType(static_cast<PieceNodeAngle>(ui->angle_ComboBox->currentData().toUInt()));
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            nodeListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enableDefaultBeforeButton()
{
    const QString allowance = qApp->TrVars()->FormulaToUser(currentSeamAllowance, qApp->Settings()->GetOsSeparator());
    ui->beforeWidthFormula_PlainTextEdit->setPlainText(allowance);
    if (QPushButton *button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enableDefaultAfterButton()
{
    const QString allowance = qApp->TrVars()->FormulaToUser(currentSeamAllowance, qApp->Settings()->GetOsSeparator());
    ui->afterWidthFormula_PlainTextEdit->setPlainText(allowance);
    if (QPushButton *button = qobject_cast<QPushButton*>(sender()))
    {
        button->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::customSeamAllowanceChanged(int row)
{
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1 || row >= ui->customSeamAllowance_ListWidget->count())
    {
        ui->startPoint_ComboBox->blockSignals(true);
        ui->startPoint_ComboBox->clear();
        ui->startPoint_ComboBox->blockSignals(false);

        ui->endPoint_ComboBox->blockSignals(true);
        ui->endPoint_ComboBox->clear();
        ui->endPoint_ComboBox->blockSignals(false);

        ui->comboBoxIncludeType->blockSignals(true);
        ui->comboBoxIncludeType->clear();
        ui->comboBoxIncludeType->blockSignals(false);
        return;
    }

    const QListWidgetItem *item = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const CustomSARecord record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));

    ui->startPoint_ComboBox->blockSignals(true);
    initializeCSAPoint(ui->startPoint_ComboBox);
    {
        const int index = ui->startPoint_ComboBox->findData(record.startPoint);
        if (index != -1)
        {
            ui->startPoint_ComboBox->setCurrentIndex(index);
        }
    }
    ui->startPoint_ComboBox->blockSignals(false);

    ui->endPoint_ComboBox->blockSignals(true);
    initializeCSAPoint(ui->endPoint_ComboBox);
    {
        const int index = ui->endPoint_ComboBox->findData(record.endPoint);
        if (index != -1)
        {
            ui->endPoint_ComboBox->setCurrentIndex(index);
        }
    }
    ui->endPoint_ComboBox->blockSignals(false);

    ui->comboBoxIncludeType->blockSignals(true);
    initializeSAIncludeType();
    {
        const int index = ui->comboBoxIncludeType->findData(static_cast<unsigned char>(record.includeType));
        if (index != -1)
        {
            ui->comboBoxIncludeType->setCurrentIndex(index);
        }
    }
    ui->comboBoxIncludeType->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::pathDialogClosed(int result)
{
    if (result == QDialog::Accepted)
    {
        SCASSERT(!m_dialog.isNull());
        DialogInternalPath *dialog = qobject_cast<DialogInternalPath*>(m_dialog.data());
        SCASSERT(dialog != nullptr);
        try
        {
            const VPiecePath newPath = dialog->GetPiecePath();
            const VPiecePath oldPath = data->GetPiecePath(dialog->GetToolId());

            SavePiecePathOptions *saveCommand = new SavePiecePathOptions(oldPath, newPath, qApp->getCurrentDocument(),
                                                                         const_cast<VContainer *>(data),
                                                                         dialog->GetToolId());
            qApp->getUndoStack()->push(saveCommand);
            updateCurrentCustomSARecord();
            updateCurrentInternalPathRecord();
        }
        catch (const VExceptionBadId &error)
        {
            qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Error. Can't save piece path.")),
                       qUtf8Printable(error.ErrorMessage()), qUtf8Printable(error.DetailedInformation()));
        }
    }
    delete m_dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchTypeChanged(int id)
{
    const int i = ui->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            ui->notchAngle_DoubleSpinBox->setEnabled(false);
            ui->notchAngle_DoubleSpinBox->setValue(0.0);

            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

            NotchType notchType = stringToNotchType(qApp->Settings()->getDefaultNotchType());

            if (id == ui->notchType_ButtonGroup->id(ui->slitNotch_RadioButton))
            {
                notchType = NotchType::Slit;
                ui->notchAngle_DoubleSpinBox->setEnabled(true);
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
            const NodeInfo info = getNodeInfo(rowNode, true);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
            rowItem->setIcon(QIcon(info.icon));
            rowItem->setText(info.name);

            nodeListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchSubTypeChanged(int id)
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

            nodeListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showNotchChanged(int state)
{
    const int i = ui->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setShowNotch(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            nodeListChanged();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showSeamlineNotchChanged(int state)
{
    const int i = ui->notches_ComboBox->currentIndex();
    if (i != -1)
    {
        QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
        if (rowItem)
        {
            VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
            rowNode.setShowSeamlineNotch(state);
            rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

            nodeListChanged();
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchLengthChanged(qreal value)
{
  const int i = ui->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchLength(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          nodeListChanged();
      }
  }

}

void PatternPieceDialog::resetNotchLength()
{
    ui->notchLength_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchLength());
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchWidthChanged(qreal value)
{
  const int i = ui->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchWidth(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          nodeListChanged();
      }
  }

}

void PatternPieceDialog::resetNotchWidth()
{
    ui->notchWidth_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchWidth());
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchAngleChanged(qreal value)
{
  const int i = ui->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchAngle(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          nodeListChanged();
      }
  }
}

void PatternPieceDialog::resetNotchAngle()
{
    ui->notchAngle_DoubleSpinBox->setValue(0.0);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::notchCountChanged(int value)
{
  const int i = ui->notches_ComboBox->currentIndex();
  if (i != -1)
  {
      QListWidgetItem *rowItem = getItemById(ui->notches_ComboBox->currentData().toUInt());
      if (rowItem)
      {
          VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
          rowNode.setNotchCount(value);
          rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));

          nodeListChanged();
      }
  }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updateGrainlineValues()
{
    QPlainTextEdit *lineEdit[2] = {ui->rotationFormula_LineEdit, ui->lengthFormula_LineEdit};
    bool formulasOK[2] = {true, true};

    for (int i = 0; i < 2; ++i)
    {
        QLabel *labelValue;
        QLabel *labelText;
        QString labelUnits;
        if (i == 0)
        {
            labelValue = ui->labelRot;
            labelText  = ui->labelEditRot;
            labelUnits = degreeSymbol;
        }
        else
        {
            labelValue = ui->labelLen;
            labelText  = ui->labelEditLen;
            labelUnits = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }

        labelValue->setToolTip(tr("Value"));

        QString formula = lineEdit[i]->toPlainText().simplified();
        QString formulaValueStr;
        try
        {
            formula.replace("\n", " ");
            formula = qApp->TrVars()->FormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
            Calculator calculation;
            qreal calculatedValue = calculation.EvalFormula(data->DataVariables(), formula);
            if (qIsInf(calculatedValue) == true || qIsNaN(calculatedValue) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if (i == 1 && calculatedValue <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                formulaValueStr.setNum(calculatedValue, 'f', 2);
                ChangeColor(labelText, okColor);
            }
        }
        catch (qmu::QmuParserError &error)
        {
            formulaValueStr = tr("Error");
            !flagGrainlineAnchor ? ChangeColor(labelText, Qt::red) : ChangeColor(labelText, okColor);
            formulasOK[i] = false;
            labelValue->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
        }

        if (formulasOK[i] && formulaValueStr.isEmpty() == false)
        {
            formulaValueStr += labelUnits;
        }
        labelValue->setText(formulaValueStr);
    }

    flagGrainlineFormula = formulasOK[0] && formulasOK[1];
    if (!flagGrainlineFormula && !flagGrainlineAnchor)
    {
        setErrorText(TabOrder::Grainline, tr("Grainline"));
    }
    else
    {
        resetGrainlineWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updatePieceLabelValues()
{
    QPlainTextEdit *lineEdit[3] = {ui->pieceLabelWidthFormula_LineEdit, ui->pieceLabelHeightFormula_LineEdit,
                                     ui->pieceLabelAngleFormula_LineEdit};
    bool formulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel *labelValue;
        QLabel *labelText;
        QString labelUnits;
        if (i == 0)
        {
            labelValue = ui->labelDLWidth;
            labelText  = ui->labelEditDLWidth;
            labelUnits = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else if (i == 1)
        {
            labelValue = ui->labelDLHeight;
            labelText  = ui->labelEditDLHeight;
            labelUnits = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else
        {
            labelValue = ui->labelDLAngle;
            labelText  = ui->labelEditDLAngle;
            labelUnits = degreeSymbol;
        }

        labelValue->setToolTip(tr("Value"));

        QString formula = lineEdit[i]->toPlainText().simplified();
        QString formulaValueStr;
        try
        {
            formula.replace("\n", " ");
            formula = qApp->TrVars()->FormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
            Calculator calculation;
            qreal calculatedValue = calculation.EvalFormula(data->DataVariables(), formula);
            if (qIsInf(calculatedValue) == true || qIsNaN(calculatedValue) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if ((i == 0 || i == 1) && calculatedValue <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                formulaValueStr.setNum(calculatedValue, 'f', 2);
                ChangeColor(labelText, okColor);
            }
        }
        catch (qmu::QmuParserError &error)
        {
            formulaValueStr = tr("Error");
            !flagPieceLabelAnchor ? ChangeColor(labelText, Qt::red) : ChangeColor(labelText, okColor);
            formulasOK[i] = false;
            labelValue->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
        }

        if (formulasOK[i] && formulaValueStr.isEmpty() == false)
        {
            formulaValueStr += labelUnits;
        }
        labelValue->setText(formulaValueStr);
    }

    flagPieceLabelAngle = formulasOK[2];
    flagPieceLabelFormula = formulasOK[0] && formulasOK[1];
    if (!flagPieceLabelAngle || !(flagPieceLabelFormula || flagPieceLabelAnchor))
    {
        setErrorText(TabOrder::Labels, tr("Labels"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->pieceLabel_Tab), icon);
    }
    else
    {
        resetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updatePatternLabelValues()
{
    QPlainTextEdit *lineEdit[3] = {ui->patternLabelWidthFormula_LineEdit, ui->patternLabelHeightFormula_LineEdit,
                                     ui->patternLabelAngleFormula_LineEdit};
    bool formulasOK[3] = {true, true, true};

    for (int i = 0; i < 3; ++i)
    {
        QLabel *labelValue;
        QLabel *labelText;
        QString labelUnits;
        if (i == 0)
        {
            labelValue = ui->labelPLWidth;
            labelText  = ui->labelEditPLWidth;
            labelUnits = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else if (i == 1)
        {
            labelValue = ui->labelPLHeight;
            labelText  = ui->labelEditPLHeight;
            labelUnits = QLatin1String(" ") + UnitsToStr(qApp->patternUnit());
        }
        else
        {
            labelValue = ui->labelPLAngle;
            labelText  = ui->labelEditPLAngle;
            labelUnits = degreeSymbol;
        }

        labelValue->setToolTip(tr("Value"));

        QString formula = lineEdit[i]->toPlainText().simplified();
        QString formulaValueStr;
        try
        {
            formula.replace("\n", " ");
            formula = qApp->TrVars()->FormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
            Calculator calculation;
            qreal calculatedValue = calculation.EvalFormula(data->DataVariables(), formula);
            if (qIsInf(calculatedValue) == true || qIsNaN(calculatedValue) == true)
            {
                throw qmu::QmuParserError(tr("Infinite/undefined result"));
            }
            else if ((i == 0 || i == 1) && calculatedValue <= 0.0)
            {
                throw qmu::QmuParserError(tr("Length should be positive"));
            }
            else
            {
                formulaValueStr.setNum(calculatedValue, 'f', 2);
                ChangeColor(labelText, okColor);
            }
        }
        catch (qmu::QmuParserError &error)
        {
            formulaValueStr = tr("Error");
            !flagPatternLabelAnchor ? ChangeColor(labelText, Qt::red) : ChangeColor(labelText, okColor);
            formulasOK[i] = false;
            labelValue->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
        }

        if (formulasOK[i] && formulaValueStr.isEmpty() == false)
        {
            formulaValueStr += labelUnits;
        }
        labelValue->setText(formulaValueStr);
    }

    flagPatternLabelAngle = formulasOK[2];
    flagPatternLabelFormula = formulasOK[0] && formulasOK[1];
    if (!flagPatternLabelAngle || !(flagPatternLabelFormula || flagPatternLabelAnchor))
    {
        setErrorText(TabOrder::Labels, tr("Labels"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->patternLabel_Tab), icon);
    }
    else
    {
        resetLabelsWarning();
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enabledGrainline()
{
    if (ui->grainline_GroupBox->isChecked() == true)
    {
        updateGrainlineValues();
        grainlineAnchorChanged();
    }
    else
    {
        flagGrainlineFormula = true;
        resetGrainlineWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enabledPieceLabel()
{
    if (ui->pieceLabel_GroupBox->isChecked() == true)
    {
        updatePieceLabelValues();
        pieceLabelAnchorChanged();
    }
    else
    {
        flagPieceLabelAngle = true;
        flagPieceLabelFormula = true;
        resetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::enabledPatternLabel()
{
    if (ui->patternLabel_GroupBox->isChecked() == true)
    {
        updatePatternLabelValues();
        patternLabelAnchorChanged();
    }
    else
    {
        flagPatternLabelAngle = true;
        flagPatternLabelFormula = true;
        resetLabelsWarning();
        CheckState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editGrainlineFormula()
{
    QPlainTextEdit *labelFormula;
    bool checkForZero;
    QString title;

    if (sender() == ui->length_PushButton)
    {
        labelFormula = ui->lengthFormula_LineEdit;
        checkForZero = true;
        title = tr("Edit length");
    }
    else if (sender() == ui->rotation_PushButton)
    {
        labelFormula = ui->rotationFormula_LineEdit;
        checkForZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    EditFormulaDialog dialog(data, NULL_ID, this);
    dialog.setWindowTitle(title);
    dialog.SetFormula(qApp->TrVars()->TryFormulaFromUser(labelFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dialog.setCheckZero(checkForZero);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString formula = dialog.GetFormula();
        formula.replace("\n", " ");

        if (sender() == ui->length_PushButton)
        {
            setGrainlineLength(formula);
        }
        else if (sender() == ui->rotation_PushButton)
        {
            setGrainlineAngle(formula);
        }
        else
        {
            // should not get here!
            labelFormula->setPlainText(formula);
        }
        updateGrainlineValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editPieceLabelFormula()
{
    QPlainTextEdit *labelFormula;
    bool checkForZero;
    QString title;

    if (sender() == ui->pieceLabelHeght_PushButton)
    {
        labelFormula = ui->pieceLabelHeightFormula_LineEdit;
        checkForZero = true;
        title = tr("Edit height");
    }
    else if (sender() == ui->pieceLabelWidth_PushButton)
    {
        labelFormula = ui->pieceLabelWidthFormula_LineEdit;
        checkForZero = true;
        title = tr("Edit width");
    }
    else if (sender() == ui->pieceLabelAngle_PushButton)
    {
        labelFormula = ui->pieceLabelAngleFormula_LineEdit;
        checkForZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    EditFormulaDialog dialog(data, NULL_ID, this);
    dialog.setWindowTitle(title);
    dialog.SetFormula(qApp->TrVars()->TryFormulaFromUser(labelFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dialog.setCheckZero(checkForZero);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString formula = dialog.GetFormula();
        formula.replace("\n", " ");
        if (sender() == ui->pieceLabelHeght_PushButton)
        {
            setPieceLabelHeight(formula);
        }
        else if (sender() == ui->pieceLabelWidth_PushButton)
        {
            setPieceLabelWidth(formula);
        }
        else if (sender() == ui->pieceLabelAngle_PushButton)
        {
            setPieceLabelAngle(formula);
        }
        else
        {
            // should not get here!
            labelFormula->setPlainText(formula);
        }
        updatePieceLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editPatternLabelFormula()
{
    QPlainTextEdit *labelFormula;
    bool checkForZero;
    QString title;

    if (sender() == ui->patternLabelHeght_PushButton)
    {
        labelFormula = ui->patternLabelHeightFormula_LineEdit;
        checkForZero = true;
        title = tr("Edit height");
    }
    else if (sender() == ui->patternLabelWidth_PushButton)
    {
        labelFormula = ui->patternLabelWidthFormula_LineEdit;
        checkForZero = true;
        title = tr("Edit width");
    }
    else if (sender() == ui->patternLabelAngle_PushButton)
    {
        labelFormula = ui->patternLabelAngleFormula_LineEdit;
        checkForZero = false;
        title = tr("Edit angle");
    }
    else
    {
        // should not get here!
        return;
    }

    EditFormulaDialog dialog(data, NULL_ID, this);
    dialog.setWindowTitle(title);
    dialog.SetFormula(qApp->TrVars()->TryFormulaFromUser(labelFormula->toPlainText(), qApp->Settings()->GetOsSeparator()));
    dialog.setCheckZero(checkForZero);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString formula = dialog.GetFormula();
        formula.replace("\n", " ");
        if (sender() == ui->patternLabelHeght_PushButton)
        {
            setPatternLabelHeight(formula);
        }
        else if (sender() == ui->patternLabelWidth_PushButton)
        {
            setPatternLabelWidth(formula);
        }
        else if (sender() == ui->patternLabelAngle_PushButton)
        {
            setPatternLabelAngle(formula);
        }
        else
        {
            // should not get here!
            labelFormula->setPlainText(formula);
        }
        updatePatternLabelValues();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::resetGrainlineWarning()
{
    if (flagGrainlineFormula || flagGrainlineAnchor)
    {
        clearErrorText(TabOrder::Grainline, tr("Grainline "));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::resetLabelsWarning()
{
    if (flagPieceLabelAngle && (flagPieceLabelFormula || flagPieceLabelAnchor))
    {
        clearErrorText(TabOrder::Labels, tr("Labels "));
        QIcon icon(":/icon/32x32/piece_label.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->pieceLabel_Tab), icon);
    }
    if (flagPatternLabelAngle && (flagPatternLabelFormula || flagPatternLabelAnchor))
    {
        clearErrorText(TabOrder::Labels, tr("Labels "));
        QIcon icon(":/icon/32x32/pattern_label.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->patternLabel_Tab), icon);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::evaluateDefaultWidth()
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
                                                                  tr("Current seam allowance")));

        evaluateBeforeWidth();
        evaluateAfterWidth();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::evaluateBeforeWidth()
{
    if (ui->nodes_ComboBox->count() > 0)
    {
        labelEditFormula = ui->beforeWidthEdit_Label;
        const QString postfix = UnitsToStr(qApp->patternUnit(), true);
        const QString formula = ui->beforeWidthFormula_PlainTextEdit->toPlainText();
        Eval(formula, flagBeforeFormula, ui->beforeWidthResult_Label, postfix, false, true);

        const QString formulaSABefore = getFormulaFromUser(ui->beforeWidthFormula_PlainTextEdit);
        updateNodeBeforeSeamAllowance(formulaSABefore);
        enableDefaultButton(ui->beforeDefault_PushButton, formulaSABefore);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::evaluateAfterWidth()
{
    if (ui->nodes_ComboBox->count() > 0)
    {
        labelEditFormula = ui->afterWidthEdit_Label;
        const QString postfix = UnitsToStr(qApp->patternUnit(), true);
        const QString formula = ui->afterWidthFormula_PlainTextEdit->toPlainText();
        Eval(formula, flagAfterFormula, ui->afterWidthResult_Label, postfix, false, true);

        const QString formulaSAAfter = getFormulaFromUser(ui->afterWidthFormula_PlainTextEdit);
        updateNodeAfterSeamAllowance(formulaSAAfter);
        enableDefaultButton(ui->afterDefault_PushButton, formulaSAAfter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editDefaultSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
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
void PatternPieceDialog::editBeforeSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width before"));
    dialog->SetFormula(getFormulaFromUser(ui->beforeWidthFormula_PlainTextEdit));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setCurrentBeforeSeamAllowance(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editAfterSeamAllowanceWidth()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit seam allowance width after"));
    dialog->SetFormula(getFormulaFromUser(ui->afterWidthFormula_PlainTextEdit));
    dialog->setCheckLessThanZero(true);
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        setCurrentAfterSeamAllowance(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::defaultWidthChanged()
{
    labelEditFormula = ui->widthEdit_Label;
    labelResultCalculation = ui->widthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    ValFormulaChanged(flagFormula, ui->widthFormula_PlainTextEdit, m_timerWidth, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::beforeWidthChanged()
{
    labelEditFormula = ui->beforeWidthEdit_Label;
    labelResultCalculation = ui->beforeWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);

    ValFormulaChanged(flagBeforeFormula, ui->beforeWidthFormula_PlainTextEdit, m_timerWidthBefore, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::afterWidthChanged()
{
    labelEditFormula = ui->afterWidthEdit_Label;
    labelResultCalculation = ui->afterWidthResult_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);

    ValFormulaChanged(flagAfterFormula, ui->afterWidthFormula_PlainTextEdit, m_timerWidthAfter, postfix);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::grainlineAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(ui->grainlineTopAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(ui->grainlineBottomAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagGrainlineAnchor = true;
        color = okColor;

        resetGrainlineWarning();
    }
    else
    {
        flagGrainlineAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        if (!flagGrainlineFormula && !flagGrainlineAnchor)
        {
            ui->menuTab_ListWidget->item(TabOrder::Grainline)->setText(QString(tr("Grainline")));
        }
    }
    updateGrainlineValues();
    ChangeColor(ui->topGrainlineAnchor_Label, color);
    ChangeColor(ui->bottomGrainlineAnchor_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::pieceLabelAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(ui->pieceLabelTopLeftAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(ui->pieceLabelBottomRightAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagPieceLabelAnchor = true;
        color = okColor;

        if (flagPatternLabelAnchor)
        {
            clearErrorText(TabOrder::Labels, tr("Labels "));
            QIcon icon(":/icon/32x32/piece_label.png");
            ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->pieceLabel_Tab), icon);
        }
    }
    else
    {
        flagPieceLabelAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        setErrorText(TabOrder::Labels, tr("Labels"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->pieceLabel_Tab), icon);
    }
    updatePieceLabelValues();
    ChangeColor(ui->labelDLTopLeftAnchor, color);
    ChangeColor(ui->labelDLBottomRightAnchor, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::patternLabelAnchorChanged()
{
    QColor color = okColor;
    const quint32 topAnchorId = getCurrentObjectId(ui->patternLabelTopLeftAnchor_ComboBox);
    const quint32 bottomAnchorId = getCurrentObjectId(ui->patternLabelBottomRightAnchor_ComboBox);
    if (topAnchorId != NULL_ID && bottomAnchorId != NULL_ID && topAnchorId != bottomAnchorId)
    {
        flagPatternLabelAnchor = true;
        color = okColor;

        if (flagPieceLabelAnchor)
        {
            clearErrorText(TabOrder::Labels, tr("Labels "));
            QIcon icon(":/icon/32x32/pattern_label.png");
            ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->patternLabel_Tab), icon);
        }
    }
    else
    {
        flagPatternLabelAnchor = false;
        topAnchorId == NULL_ID && bottomAnchorId == NULL_ID ? color = okColor : color = errorColor;

        setErrorText(TabOrder::Labels, tr("Labels"));
        QIcon icon(":/icons/win.icon.theme/16x16/status/dialog-warning.png");
        ui->labels_TabWidget->setTabIcon(ui->labels_TabWidget->indexOf(ui->patternLabel_Tab), icon);
    }
    updatePatternLabelValues();
    ChangeColor(ui->labelPLTopLeftAnchor, color);
    ChangeColor(ui->labelPLBottomRightAnchor, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editPatternLabel()
{
    QVector<VLabelTemplateLine> patternLabelLines = qApp->getCurrentDocument()->getPatternLabelTemplate();
    EditLabelTemplateDialog editor(qApp->getCurrentDocument());
    editor.SetTemplate(patternLabelLines);
    editor.SetPiece(GetPiece());

    if (QDialog::Accepted == editor.exec())
    {
        patternLabelLines = editor.GetTemplate();
        qApp->getCurrentDocument()->setPatternLabelTemplate(patternLabelLines);
        emit qApp->getCurrentDocument()->UpdatePatternLabel();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::editPieceLabel()
{
    EditLabelTemplateDialog editor(qApp->getCurrentDocument());
    editor.SetTemplate(m_pieceLabelLines);
    editor.SetPiece(GetPiece());

    if (QDialog::Accepted == editor.exec())
    {
        m_pieceLabelLines = editor.GetTemplate();
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPiece PatternPieceDialog::CreatePiece() const
{
    VPiece piece;
    piece.GetPath().SetNodes(GetListInternals<VPieceNode>(ui->mainPath_ListWidget));
    piece.SetCustomSARecords(GetListInternals<CustomSARecord>(ui->customSeamAllowance_ListWidget));
    piece.SetInternalPaths(GetListInternals<quint32>(ui->internalPaths_ListWidget));
    piece.setAnchors(GetListInternals<quint32>(ui->anchorPoints_ListWidget));
    piece.SetForbidFlipping(ui->forbidFlipping_CheckBox->isChecked());
    piece.SetSeamAllowance(ui->seams_CheckBox->isChecked());
    piece.SetSeamAllowanceBuiltIn(ui->builtIn_CheckBox->isChecked());
    piece.setHideSeamLine(ui->hideSeamLine_CheckBox->isChecked());
    piece.SetName(ui->pieceName_LineEdit->text());
    piece.setColor(getPieceColor());
    piece.setFill(getPieceFill());
    piece.SetInLayout(isInLayout());
    piece.setIsLocked(getPieceLock());
    piece.SetMx(m_mx);
    piece.SetMy(m_my);
    piece.setSeamAllowanceWidthFormula(getFormulaFromUser(ui->widthFormula_PlainTextEdit), m_saWidth);

    piece.GetPatternPieceData() = m_oldData;
    piece.GetPatternPieceData().SetLetter(ui->letter_LineEdit->text());
    piece.GetPatternPieceData().SetAnnotation(ui->annotation_LineEdit->text());
    piece.GetPatternPieceData().SetOrientation(ui->orientation_ComboBox->currentText());
    piece.GetPatternPieceData().SetRotationWay(ui->rotation_ComboBox->currentText());
    piece.GetPatternPieceData().SetTilt(ui->tilt_ComboBox->currentText());
    piece.GetPatternPieceData().SetFoldPosition(ui->foldPosition_ComboBox->currentText());
    piece.GetPatternPieceData().SetQuantity(ui->quantity_SpinBox->value());
    piece.GetPatternPieceData().SetOnFold(ui->fold_CheckBox->isChecked());
    piece.GetPatternPieceData().SetLabelTemplate(m_pieceLabelLines);
    piece.GetPatternPieceData().SetVisible(ui->pieceLabel_GroupBox->isChecked());
    piece.GetPatternPieceData().SetLabelWidth(getFormulaFromUser(ui->pieceLabelWidthFormula_LineEdit));
    piece.GetPatternPieceData().SetLabelHeight(getFormulaFromUser(ui->pieceLabelHeightFormula_LineEdit));
    piece.GetPatternPieceData().SetRotation(getFormulaFromUser(ui->pieceLabelAngleFormula_LineEdit));
    piece.GetPatternPieceData().SetFontSize(m_oldData.getFontSize());
    piece.GetPatternPieceData().setCenterAnchorPoint(getCurrentObjectId(ui->pieceLabelCenterAnchor_ComboBox));
    piece.GetPatternPieceData().setTopLeftAnchorPoint(getCurrentObjectId(ui->pieceLabelTopLeftAnchor_ComboBox));
    piece.GetPatternPieceData().setBottomRightAnchorPoint(getCurrentObjectId(ui->pieceLabelBottomRightAnchor_ComboBox));

    piece.GetPatternInfo() = m_oldGeom;
    piece.GetPatternInfo().SetVisible(ui->patternLabel_GroupBox->isChecked());
    piece.GetPatternInfo().SetLabelWidth(getFormulaFromUser(ui->patternLabelWidthFormula_LineEdit));
    piece.GetPatternInfo().SetLabelHeight(getFormulaFromUser(ui->patternLabelHeightFormula_LineEdit));
    piece.GetPatternInfo().SetRotation(getFormulaFromUser(ui->patternLabelAngleFormula_LineEdit));
    piece.GetPatternInfo().setCenterAnchorPoint(getCurrentObjectId(ui->patternLabelCenterAnchor_ComboBox));
    piece.GetPatternInfo().setTopLeftAnchorPoint(getCurrentObjectId(ui->patternLabelTopLeftAnchor_ComboBox));
    piece.GetPatternInfo().setBottomRightAnchorPoint(getCurrentObjectId(ui->patternLabelBottomRightAnchor_ComboBox));

    piece.GetGrainlineGeometry() = m_oldGrainline;
    piece.GetGrainlineGeometry().SetVisible(ui->grainline_GroupBox->isChecked());
    piece.GetGrainlineGeometry().SetRotation(getFormulaFromUser(ui->rotationFormula_LineEdit));
    piece.GetGrainlineGeometry().SetLength(getFormulaFromUser(ui->lengthFormula_LineEdit));
    piece.GetGrainlineGeometry().SetArrowType(static_cast<ArrowType>(ui->arrow_ComboBox->currentIndex()));
    piece.GetGrainlineGeometry().setCenterAnchorPoint(getCurrentObjectId(ui->grainlineCenterAnchor_ComboBox));
    piece.GetGrainlineGeometry().setTopAnchorPoint(getCurrentObjectId(ui->grainlineTopAnchor_ComboBox));
    piece.GetGrainlineGeometry().setBottomAnchorPoint(getCurrentObjectId(ui->grainlineBottomAnchor_ComboBox));

    if (applyAllowed == false)
    {
        VPiecePath path = piece.GetPath();
        const QRectF rect = QPolygonF(path.PathPoints(data)).boundingRect();

        qreal width = getFormulaValue(ui->pieceLabelWidthFormula_LineEdit);
        qreal height = getFormulaValue(ui->pieceLabelHeightFormula_LineEdit);
        qreal xPos = rect.center().x() + (rect.width()/2.0 - width)/2.0;
        qreal yPos = rect.center().y() - height/2.0;
        piece.GetPatternPieceData().SetPos(QPointF(xPos, yPos));

        xPos = rect.left() + (rect.width()/2.0 - width)/2.0;
        yPos = rect.center().y() - height/2.0;
        piece.GetPatternInfo().SetPos(QPointF(xPos, yPos));

        xPos = rect.center().x();
        yPos = rect.center().y() + getFormulaValue(ui->lengthFormula_LineEdit)/2.0;

        piece.GetGrainlineGeometry().SetPos(QPointF(xPos, yPos));
    }
    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::newMainPathItem(const VPieceNode &node)
{
    newNodeItem(ui->mainPath_ListWidget, node);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::newCustomSeamAllowance(const CustomSARecord &record)
{
    if (record.path > NULL_ID)
    {
        const QString name = getPathName(record.path, record.reverse);

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, QVariant::fromValue(record));
        ui->customSeamAllowance_ListWidget->addItem(item);
        ui->customSeamAllowance_ListWidget->setCurrentRow(ui->customSeamAllowance_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::newInternalPath(quint32 path)
{
    if (path > NULL_ID)
    {
        const QString name = getPathName(path);

        QListWidgetItem *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, QVariant::fromValue(path));
        ui->internalPaths_ListWidget->addItem(item);
        ui->internalPaths_ListWidget->setCurrentRow(ui->internalPaths_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::newAnchorPoint(quint32 anchorPoint)
{
    if (anchorPoint > NULL_ID)
    {
        const QSharedPointer<VGObject> anchor = data->GetGObject(anchorPoint);

        QListWidgetItem *item = new QListWidgetItem(anchor->name());
        item->setData(Qt::UserRole, QVariant::fromValue(anchorPoint));
        ui->anchorPoints_ListWidget->addItem(item);
        ui->anchorPoints_ListWidget->setCurrentRow(ui->anchorPoints_ListWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString PatternPieceDialog::getPathName(quint32 path, bool reverse) const
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
QString PatternPieceDialog::getPieceColor() const
{
    QString color =  ui->color_Label->text();
    return color;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceColor(const QString &color)
{
    ui->color_Label->setText(color);
    ui->color_Label->setPalette(QPalette(color));
}

//---------------------------------------------------------------------------------------------------------------------
QString PatternPieceDialog::getPieceFill() const
{
    QString value =  GetComboBoxCurrentData(ui->fill_ComboBox, FillNone);
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceFill(const QString &value)
{
    ChangeCurrentData(ui->fill_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool PatternPieceDialog::isInLayout() const
{
   return m_inLayout;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setInLayout(const bool &state)
{
    m_inLayout = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool PatternPieceDialog::getPieceLock() const
{
   return m_isLocked;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceLock(const bool &state)
{
    m_isLocked = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool PatternPieceDialog::isMainPathValid() const
{
    QString warning = DialogWarningIcon();

    if(CreatePiece().MainPathPoints(data).count() < 3)
    {
        warning += tr("You need more points!");
        ui->status_Label->setText(warning);
        return false;
    }
    else
    {
        if(!isMainPathClockwise())
        {
            warning += tr("You must choose points in a clockwise direction!");
            ui->status_Label->setText(warning);
            return false;
        }
        if (isFirstPointSameAsLast(ui->mainPath_ListWidget))
        {
            warning += tr("First point cannot be same as last point!");
            ui->status_Label->setText(warning);
            return  false;
        }
        else if (doublePointsExist(ui->mainPath_ListWidget))
        {
            warning += tr("You have double points!");
            ui->status_Label->setText(warning);
            return  false;
        }
        else if (!isEachPointNameUnique(ui->mainPath_ListWidget))
        {
            warning += tr("Each point in the path must be unique!");
            ui->status_Label->setText(warning);
            return  false;
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::validateObjects(bool value)
{
    flagMainPath = value;
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
bool PatternPieceDialog::isMainPathClockwise() const
{
    const QVector<QPointF> points = CreatePiece().MainPathPoints(data);
    return VPiece::isClockwise(points);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeNodesList()
{
    const quint32 id = ui->nodes_ComboBox->currentData().toUInt();

    ui->nodes_GroupBox->setEnabled(true);
    ui->nodes_ComboBox->blockSignals(true);
    ui->nodes_ComboBox->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->mainPath_ListWidget);

    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && !node.isExcluded())
        {
            const NodeInfo info = getNodeInfo(node);

            ui->nodes_ComboBox->addItem(info.name, node.GetId());
        }
    }

    if (pointNodesExist())
    {
        const int index = ui->nodes_ComboBox->findData(id);
        if (index != -1)
        {
            ui->nodes_ComboBox->setCurrentIndex(index);
            nodeChanged(index);// Need in case combobox index was not changed
        }
        else
        {
            ui->nodes_ComboBox->count() > 0 ? nodeChanged(0) : nodeChanged(-1);
        }
        ui->nodes_ComboBox->blockSignals(false);
    }
    else
    {
        ui->nodes_GroupBox->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeNotchesList()
{
    const quint32 id = ui->notches_ComboBox->currentData().toUInt();

    ui->notches_ComboBox->blockSignals(true);
    ui->notches_ComboBox->clear();

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->mainPath_ListWidget);

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
QListWidgetItem *PatternPieceDialog::getItemById(quint32 id)
{
    for (qint32 i = 0; i < ui->mainPath_ListWidget->count(); ++i)
    {
        QListWidgetItem *item = ui->mainPath_ListWidget->item(i);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));

        if (node.GetId() == id)
        {
            return item;
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 PatternPieceDialog::getPreviousId() const
{
    const int count = ui->mainPath_ListWidget->count();
    if (count > 0)
    {
        QListWidgetItem *item = ui->mainPath_ListWidget->item(count-1);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node.GetId();
    }
    else
    {
        return NULL_ID;
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode PatternPieceDialog::getLastNode() const
{
    const int count = ui->mainPath_ListWidget->count();
    if (count > 0)
    {
        QListWidgetItem *item = ui->mainPath_ListWidget->item(count-1);
        const VPieceNode node = qvariant_cast<VPieceNode>(item->data(Qt::UserRole));
        return node;
    }
    else
    {
        return VPieceNode();
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool PatternPieceDialog::pointNodesExist() const
{
    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->mainPath_ListWidget);
    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode node = nodes.at(i);
        if (node.GetTypeTool() == Tool::NodePoint && !node.isExcluded())
        {
            qDebug() << "point nodes exist";
            return true;
        }
    }
    qDebug() << "point nodes do not exist";
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setCurrentBeforeSeamAllowance(const QString &formula)
{
    updateNodeBeforeSeamAllowance(formula);
    nodeListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setCurrentAfterSeamAllowance(const QString &formula)
{
    updateNodeAfterSeamAllowance(formula);
    nodeListChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updateNodeBeforeSeamAllowance(const QString &formula)
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
void PatternPieceDialog::updateNodeAfterSeamAllowance(const QString &formula)
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
void PatternPieceDialog::initializeMainPathTab()
{
    ui->forbidFlipping_CheckBox->setChecked(qApp->Settings()->getForbidPieceFlipping());
    ui->hideSeamLine_CheckBox->setChecked(qApp->Settings()->isHideSeamLine());

    ui->mainPath_ListWidget->installEventFilter(this);
    ui->mainPath_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->mainPath_ListWidget, &QListWidget::customContextMenuRequested, this,
            &PatternPieceDialog::showMainPathContextMenu);

    connect(ui->mainPath_ListWidget->model(), &QAbstractItemModel::rowsMoved, this, [this]()
    {
      nodeListChanged();
      validateObjects(isMainPathValid());
    });

    connect(ui->mainPath_ListWidget, &QListWidget::itemSelectionChanged,
            this, &PatternPieceDialog::setMoveExclusions);
    connect(ui->moveTop_ToolButton,&QToolButton::clicked, this, [this]()
    {
        moveListRowTop(ui->mainPath_ListWidget);
        validateObjects(isMainPathValid());
    });
    connect(ui->moveUp_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowUp(ui->mainPath_ListWidget);
        validateObjects(isMainPathValid());
    });
    connect(ui->moveDown_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowDown(ui->mainPath_ListWidget);
        validateObjects(isMainPathValid());
    });
    connect(ui->moveBottom_ToolButton, &QToolButton::clicked, this, [this]()
    {
        moveListRowBottom(ui->mainPath_ListWidget);
        validateObjects(isMainPathValid());
    });
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeSeamAllowanceTab()
{
    ui->seams_CheckBox->setChecked(qApp->Settings()->getDefaultSeamAllowanceVisibilty());

    plainTextEditFormula = ui->widthFormula_PlainTextEdit;

    ui->widthFormula_PlainTextEdit->installEventFilter(this);
    ui->beforeWidthFormula_PlainTextEdit->installEventFilter(this);
    ui->afterWidthFormula_PlainTextEdit->installEventFilter(this);

    m_timerWidth = new QTimer(this);
    connect(m_timerWidth, &QTimer::timeout, this, &PatternPieceDialog::evaluateDefaultWidth);

    m_timerWidthBefore = new QTimer(this);
    connect(m_timerWidthBefore, &QTimer::timeout, this, &PatternPieceDialog::evaluateBeforeWidth);

    m_timerWidthAfter = new QTimer(this);
    connect(m_timerWidthAfter, &QTimer::timeout, this, &PatternPieceDialog::evaluateAfterWidth);

    connect(ui->seams_CheckBox, &QCheckBox::toggled, this, &PatternPieceDialog::enableSeamAllowance);
    connect(ui->builtIn_CheckBox, &QCheckBox::toggled, this, &PatternPieceDialog::enableBuiltIn);

    // Initialize the default seam allowance, convert the value if app unit is different than pattern unit
    m_saWidth = UnitConvertor(qApp->Settings()->GetDefaultSeamAllowance(),
                              StrToUnits(qApp->Settings()->GetUnit()), qApp->patternUnit());

    ui->widthFormula_PlainTextEdit->setPlainText(qApp->LocaleToString(m_saWidth));

    initializeNodesList();
    connect(ui->nodes_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &PatternPieceDialog::nodeChanged);

    connect(ui->beforeDefault_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::enableDefaultBeforeButton);
    connect(ui->afterDefault_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::enableDefaultAfterButton);

    initializeNodeAngles(ui->angle_ComboBox);
    connect(ui->angle_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &PatternPieceDialog::nodeAngleChanged);

    ui->customSeamAllowance_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->customSeamAllowance_ListWidget, &QListWidget::customContextMenuRequested, this,
            &PatternPieceDialog::showCustomSAContextMenu);
    connect(ui->customSeamAllowance_ListWidget, &QListWidget::currentRowChanged, this,
            &PatternPieceDialog::customSeamAllowanceChanged);
    connect(ui->startPoint_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PatternPieceDialog::customSAStartPointChanged);
    connect(ui->endPoint_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &PatternPieceDialog::customSAEndPointChanged);
    connect(ui->comboBoxIncludeType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PatternPieceDialog::customSAIncludeTypeChanged);

    connect(ui->toolButtonExprWidth, &QPushButton::clicked, this, &PatternPieceDialog::editDefaultSeamAllowanceWidth);
    connect(ui->beforeExpr_ToolButton, &QPushButton::clicked, this, &PatternPieceDialog::editBeforeSeamAllowanceWidth);
    connect(ui->afterExpr_ToolButton, &QPushButton::clicked, this, &PatternPieceDialog::editAfterSeamAllowanceWidth);

    connect(ui->widthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::defaultWidthChanged);
    connect(ui->beforeWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::beforeWidthChanged);
    connect(ui->afterWidthFormula_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::afterWidthChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeCSAPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();
    box->addItem(tr("Empty"), NULL_ID);

    const QVector<VPieceNode> nodes = GetListInternals<VPieceNode>(ui->mainPath_ListWidget);

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
void PatternPieceDialog::initAnchorPoint(QComboBox *box)
{
    SCASSERT(box != nullptr);

    quint32 currentId = NULL_ID;
    if (box->count() > 0)
    {
        currentId = box->currentData().toUInt();
    }

    box->clear();
    box->addItem(QLatin1String("<") + tr("None") + QLatin1String(">"), NULL_ID);

    const QVector<quint32> anchorPoints = GetListInternals<quint32>(ui->anchorPoints_ListWidget);

    for (int i = 0; i < anchorPoints.size(); ++i)
    {
        const QSharedPointer<VGObject> anchorPoint = data->GetGObject(anchorPoints.at(i));
        box->addItem(anchorPoint->name(), anchorPoints.at(i));
    }

    const int index = ui->nodes_ComboBox->findData(currentId);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeSAIncludeType()
{
    ui->comboBoxIncludeType->clear();

    ui->comboBoxIncludeType->addItem(tr("main path"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsMainPath));
    ui->comboBoxIncludeType->addItem(tr("custom seam allowance"),
                                             static_cast<unsigned char>(PiecePathIncludeType::AsCustomSA));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeInternalPathsTab()
{
    ui->internalPaths_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->internalPaths_ListWidget, &QListWidget::customContextMenuRequested, this,
            &PatternPieceDialog::showInternalPathsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializePropertiesTab()
{
    ui->pieceName_LineEdit->setClearButtonEnabled(true);
    ui->letter_LineEdit->setClearButtonEnabled(true);
    ui->annotation_LineEdit->setClearButtonEnabled(true);

    QColor color = QColor(Qt::white);
    ui->color_Label->setText(color.name());
    ui->color_Label->setPalette(QPalette(color));

    connect(ui->pieceName_LineEdit, &QLineEdit::textChanged, this, &PatternPieceDialog::pieceNameChanged);
    connect(ui->colorPicker_ToolButton, &QToolButton::clicked, this, &PatternPieceDialog::pieceColorChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeLabelsTab()
{
    //Piece label
    ui->pieceLabel_GroupBox->setChecked(qApp->Settings()->showPieceLabels());

    ui->pieceLabelWidthFormula_LineEdit->setPlainText(QString::number(qApp->Settings()->getDefaultLabelWidth()));
    ui->pieceLabelHeightFormula_LineEdit->setPlainText(QString::number(qApp->Settings()->getDefaultLabelHeight()));

    connect(ui->pieceLabel_GroupBox, &QGroupBox::toggled, this, &PatternPieceDialog::enabledPieceLabel);
    initAnchorPoint(ui->pieceLabelCenterAnchor_ComboBox);
    initAnchorPoint(ui->pieceLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(ui->pieceLabelBottomRightAnchor_ComboBox);

    connect(ui->pieceLabelTopLeftAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::pieceLabelAnchorChanged);

    connect(ui->pieceLabelBottomRightAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::pieceLabelAnchorChanged);

    connect(ui->pieceLabelWidth_PushButton, &QPushButton::clicked,
            this, &PatternPieceDialog::editPieceLabelFormula);
    connect(ui->pieceLabelHeght_PushButton, &QPushButton::clicked,
            this, &PatternPieceDialog::editPieceLabelFormula);
    connect(ui->pieceLabelAngle_PushButton, &QPushButton::clicked,
            this, &PatternPieceDialog::editPieceLabelFormula);

    connect(ui->pieceLabelWidthFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePieceLabelValues);
    connect(ui->pieceLabelHeightFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePieceLabelValues);
    connect(ui->pieceLabelAngleFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePieceLabelValues);

    connect(ui->editPieceLabel_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::editPieceLabel);

    if (m_pieceLabelLines.isEmpty())
    {
        VLabelTemplate labelTemplate;
        QString filename = qApp->Settings()->getDefaultPatternTemplate();
        if (QFileInfo(filename).exists())
        {
            labelTemplate.setXMLContent(VLabelTemplateConverter(filename).Convert());
            m_pieceLabelLines = labelTemplate.ReadLines();
        }
    }

    enabledPieceLabel();

    //Pattern label
    ui->patternLabel_GroupBox->setChecked(qApp->Settings()->showPatternLabels());

    ui->patternLabelWidthFormula_LineEdit->setPlainText(QString::number(qApp->Settings()->getDefaultLabelWidth()));
    ui->patternLabelHeightFormula_LineEdit->setPlainText(QString::number(qApp->Settings()->getDefaultLabelHeight()));

    connect(ui->patternLabel_GroupBox, &QGroupBox::toggled, this, &PatternPieceDialog::enabledPatternLabel);
    initAnchorPoint(ui->patternLabelCenterAnchor_ComboBox);
    initAnchorPoint(ui->patternLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(ui->patternLabelBottomRightAnchor_ComboBox);

    connect(ui->patternLabelTopLeftAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::patternLabelAnchorChanged);

    connect(ui->patternLabelBottomRightAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::patternLabelAnchorChanged);

    connect(ui->patternLabelWidth_PushButton,  &QPushButton::clicked,
            this, &PatternPieceDialog::editPatternLabelFormula);
    connect(ui->patternLabelHeght_PushButton, &QPushButton::clicked,
            this, &PatternPieceDialog::editPatternLabelFormula);
    connect(ui->patternLabelAngle_PushButton,  &QPushButton::clicked,
            this, &PatternPieceDialog::editPatternLabelFormula);

    connect(ui->patternLabelWidthFormula_LineEdit,  &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePatternLabelValues);
    connect(ui->patternLabelHeightFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePatternLabelValues);
    connect(ui->patternLabelAngleFormula_LineEdit,  &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updatePatternLabelValues);

    connect(ui->editPatternLabel_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::editPatternLabel);

    m_patternLabelLines = qApp->getCurrentDocument()->getPatternLabelTemplate();

    if (m_patternLabelLines.isEmpty())
    {
        VLabelTemplate labelTemplate;
        QString filename = qApp->Settings()->getDefaultPatternTemplate();
        if (QFileInfo(filename).exists())
        {
            labelTemplate.setXMLContent(VLabelTemplateConverter(filename).Convert());
            m_patternLabelLines = labelTemplate.ReadLines();
            qApp->getCurrentDocument()->setPatternLabelTemplate(m_patternLabelLines);
        }
    }

    enabledPatternLabel();

    ui->pieceName_LineEdit->setText(createPieceName());
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeGrainlineTab()
{
    ui->grainline_GroupBox->setChecked(qApp->Settings()->getDefaultGrainlineVisibilty());

    ui->lengthFormula_LineEdit->setPlainText(QString::number(qApp->Settings()->getDefaultGrainlineLength()));

    connect(ui->grainline_GroupBox,  &QGroupBox::toggled,   this, &PatternPieceDialog::enabledGrainline);
    connect(ui->rotation_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::editGrainlineFormula);
    connect(ui->length_PushButton,   &QPushButton::clicked, this, &PatternPieceDialog::editGrainlineFormula);
    connect(ui->lengthFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updateGrainlineValues);

    connect(ui->rotationFormula_LineEdit, &QPlainTextEdit::textChanged, this,
            &PatternPieceDialog::updateGrainlineValues);

    enabledGrainline();

    ui->arrow_ComboBox->addItem(tr("Both"));
    ui->arrow_ComboBox->addItem(tr("Just front"));
    ui->arrow_ComboBox->addItem(tr("Just rear"));

    initAnchorPoint(ui->grainlineCenterAnchor_ComboBox);
    initAnchorPoint(ui->grainlineTopAnchor_ComboBox);
    initAnchorPoint(ui->grainlineBottomAnchor_ComboBox);

    connect(ui->grainlineTopAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::grainlineAnchorChanged);

    connect(ui->grainlineBottomAnchor_ComboBox, &QComboBox::currentTextChanged,
            this, &PatternPieceDialog::grainlineAnchorChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeAnchorsTab()
{
    ui->anchorPoints_ListWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->anchorPoints_ListWidget, &QListWidget::customContextMenuRequested, this,
            &PatternPieceDialog::showAnchorsContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initializeNotchesTab()
{
    initializeNotchesList();

    ui->notchLength_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchLength());
    ui->notchWidth_DoubleSpinBox->setValue(qApp->Settings()->getDefaultNotchWidth());
    ui->showNotch_CheckBox->setChecked(qApp->Settings()->showSeamAllowanceNotch());
    ui->showSeamlineNotch_CheckBox->setChecked(qApp->Settings()->showSeamlineNotch());

    connect(ui->notches_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PatternPieceDialog::notchChanged);

    connect(ui->notchType_ButtonGroup, &QButtonGroup::idClicked,
            this, &PatternPieceDialog::notchTypeChanged);

    connect(ui->notchSubType_ButtonGroup, &QButtonGroup::idClicked,
            this, &PatternPieceDialog::notchSubTypeChanged);

    connect(ui->showNotch_CheckBox, &QCheckBox::stateChanged, this,
            &PatternPieceDialog::showNotchChanged);

    connect(ui->showSeamlineNotch_CheckBox, &QCheckBox::stateChanged, this,
            &PatternPieceDialog::showSeamlineNotchChanged);

    connect(ui->notchLength_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PatternPieceDialog::notchLengthChanged);

    connect(ui->resetLength_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::resetNotchLength);

    connect(ui->notchWidth_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PatternPieceDialog::notchWidthChanged);

    connect(ui->resetWidth_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::resetNotchWidth);

    connect(ui->notchAngle_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &PatternPieceDialog::notchAngleChanged);

    connect(ui->resetAngle_PushButton, &QPushButton::clicked, this, &PatternPieceDialog::resetNotchAngle);

    connect(ui->notchCount_SpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &PatternPieceDialog::notchCountChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::initAnchorPointComboboxes()
{
    initAnchorPoint(ui->grainlineCenterAnchor_ComboBox);
    initAnchorPoint(ui->grainlineTopAnchor_ComboBox);
    initAnchorPoint(ui->grainlineBottomAnchor_ComboBox);

    initAnchorPoint(ui->pieceLabelCenterAnchor_ComboBox);
    initAnchorPoint(ui->pieceLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(ui->pieceLabelBottomRightAnchor_ComboBox);

    initAnchorPoint(ui->patternLabelCenterAnchor_ComboBox);
    initAnchorPoint(ui->patternLabelTopLeftAnchor_ComboBox);
    initAnchorPoint(ui->patternLabelBottomRightAnchor_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
QString PatternPieceDialog::getSeamAllowanceWidthFormula() const
{
    QString width = ui->widthFormula_PlainTextEdit->toPlainText();
    width.replace("\n", " ");
    return qApp->TrVars()->TryFormulaFromUser(width, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setSeamAllowanceWidthFormula(const QString &formula)
{
    const QString width = qApp->TrVars()->FormulaToUser(formula, qApp->Settings()->GetOsSeparator());
    ui->widthFormula_PlainTextEdit->setPlainText(width);

    PatternPieceVisual *path = qobject_cast<PatternPieceVisual *>(vis);
    SCASSERT(path != nullptr)
    const VPiece p = CreatePiece();
    path->SetPiece(p);

    MoveCursorToEnd(ui->widthFormula_PlainTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updateCurrentCustomSARecord()
{
    const int row = ui->customSeamAllowance_ListWidget->currentRow();
    if (ui->customSeamAllowance_ListWidget->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = ui->customSeamAllowance_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const CustomSARecord record = qvariant_cast<CustomSARecord>(item->data(Qt::UserRole));
    item->setText(getPathName(record.path, record.reverse));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::updateCurrentInternalPathRecord()
{
    const int row = ui->internalPaths_ListWidget->currentRow();
    if (ui->internalPaths_ListWidget->count() == 0 || row == -1)
    {
        return;
    }

    QListWidgetItem *item = ui->internalPaths_ListWidget->item(row);
    SCASSERT(item != nullptr);
    const quint32 path = qvariant_cast<quint32>(item->data(Qt::UserRole));
    item->setText(getPathName(path));
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setGrainlineAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    ui->rotationFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->rotationFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setGrainlineLength(QString lengthFormula)
{
    if (lengthFormula.isEmpty())
    {
        lengthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(lengthFormula, qApp->Settings()->GetOsSeparator());
    ui->lengthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->lengthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceLabelWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(widthFormula, qApp->Settings()->GetOsSeparator());
    ui->pieceLabelWidthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->pieceLabelWidthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceLabelHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(heightFormula, qApp->Settings()->GetOsSeparator());
    ui->pieceLabelHeightFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->pieceLabelHeightFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPieceLabelAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    ui->pieceLabelAngleFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->pieceLabelAngleFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPatternLabelWidth(QString widthFormula)
{
    if (widthFormula.isEmpty())
    {
        widthFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(widthFormula, qApp->Settings()->GetOsSeparator());
    ui->patternLabelWidthFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->patternLabelWidthFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPatternLabelHeight(QString heightFormula)
{
    if (heightFormula.isEmpty())
    {
        heightFormula = QString().setNum(UnitConvertor(1, Unit::Cm, *data->GetPatternUnit()));
    }

    const QString formula = qApp->TrVars()->FormulaToUser(heightFormula, qApp->Settings()->GetOsSeparator());
    ui->patternLabelHeightFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->patternLabelHeightFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setPatternLabelAngle(QString angleFormula)
{
    if (angleFormula.isEmpty())
    {
        angleFormula = QString("0");
    }

    const QString formula = qApp->TrVars()->FormulaToUser(angleFormula, qApp->Settings()->GetOsSeparator());
    ui->patternLabelAngleFormula_LineEdit->setPlainText(formula);

    MoveCursorToEnd(ui->patternLabelAngleFormula_LineEdit);
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::showAnchorPoints()
{
    if (m_anchorPoints.isNull())
    {
        m_anchorPoints = new PieceAnchorPointVisual(data);
    }

    m_anchorPoints->setAnchors(GetListInternals<quint32>(ui->anchorPoints_ListWidget));

    if (!qApp->getCurrentScene()->items().contains(m_anchorPoints))
    {
        m_anchorPoints->VisualMode(NULL_ID);
        m_anchorPoints->setZValue(10); // anchor points should be on top
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(toolId));
        SCASSERT(tool != nullptr);
        m_anchorPoints->setParentItem(tool);
    }
    else
    {
        m_anchorPoints->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PatternPieceDialog::setMoveExclusions()
{
    ui->moveTop_ToolButton->setEnabled(false);
    ui->moveUp_ToolButton->setEnabled(false);
    ui->moveDown_ToolButton->setEnabled(false);
    ui->moveBottom_ToolButton->setEnabled(false);

    if (ui->mainPath_ListWidget->count() > 1)
    {
        if (ui->mainPath_ListWidget->currentRow() == 0)
        {
            ui->moveDown_ToolButton->setEnabled(true);
            ui->moveBottom_ToolButton->setEnabled(true);
        }
        else if (ui->mainPath_ListWidget->currentRow() == ui->mainPath_ListWidget->count() - 1)
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

//---------------------------------------------------------------------------------------------------------------------
QString PatternPieceDialog::createPieceName() const
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

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Reverses the selected node points if the node is a curve.
 * @param rowItem list widget item of the selected row.
 */
 void PatternPieceDialog::reverseNode(QListWidgetItem *rowItem)
{
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        rowNode.SetReverse(!rowNode.GetReverse());
        NodeInfo info;
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Duplcates the selected node and adds it excluded at the end of the list widget.
 * @param rowItem list widget item of the selected row.
 */
 void PatternPieceDialog::duplicateNode(QListWidgetItem *rowItem)
{
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        rowNode.SetExcluded(true);
        newNodeItem(ui->mainPath_ListWidget, rowNode, true, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Toggles the exclude state of the selected node.
 * @param rowItem list widget item of the selected row.
 */
 void PatternPieceDialog::excludeNode(QListWidgetItem *rowItem)
{
    NodeInfo info;
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
    rowNode.SetExcluded(!rowNode.isExcluded());
    info = getNodeInfo(rowNode, true);
    rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
    rowItem->setIcon(QIcon(info.icon));
    rowItem->setText(info.name);
    rowItem->setFont(NodeFont(rowNode.isExcluded()));
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Sets the notch type of the node of the selected widget item.
 * @param rowItem list widget item of the selected row.
 * @param notchType of the selected submenu item.
 */
void PatternPieceDialog::setNotch(QListWidgetItem *rowItem, bool isNotch, NotchType notchType,
                                  NotchSubType notchSubType)
{
    VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
    if (rowNode.GetTypeTool() == Tool::NodePoint)
    {
        rowNode.setNotch(isNotch);
        rowNode.setNotchType(notchType);
        rowNode.setNotchSubType(notchSubType);
        NodeInfo info;
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
}

void PatternPieceDialog::setCurrentText(QComboBox *box, const QString &text) const
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findText(text);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    else
    {
        box->setCurrentIndex(0);
    }
}

qreal PatternPieceDialog::getFormulaValue(QPlainTextEdit *text) const
{
    Calculator calculation;
    QString formula = text->toPlainText().simplified();
    formula.replace("\n", " ");
    formula = qApp->TrVars()->FormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
    return ToPixel(calculation.EvalFormula(data->DataVariables(), formula), *data->GetPatternUnit());
}

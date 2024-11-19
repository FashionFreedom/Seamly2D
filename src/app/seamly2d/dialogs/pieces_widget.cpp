/***************************************************************************
 **  @file   pieces_widget.cpp
 **  @author Douglas S Caskey
 **  @date   Nov 22, 2023
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vwidgetdetails.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2016
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
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "pieces_widget.h"
#include "ui_pieces_widget.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/vcontainer.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/undocommands/togglepieceinlayout.h"
#include "../vtools/undocommands/toggle_piecelock.h"
#include "../vtools/undocommands/set_piece_color.h"
#include "../vwidgets/piece_tablewidgetitem.h"
#include "../vwidgets/vmaingraphicsscene.h"

#include <QColorDialog>
#include <QList>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QRegularExpression>
#include <QTableWidget>
#include <QUndoStack>

#define BASE_10 10
#define MAX_LENGTH 3

//---------------------------------------------------------------------------------------------------------------------
PiecesWidget::PiecesWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PiecesWidget)
    , m_doc(doc)
    , m_data(data)
    , m_allPieces()
{
    ui->setupUi(this);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    fillTable(m_data->DataPieces());
    QSettings settings;
    ui->tableWidget->sortItems(settings.value("pieceSort", 4).toInt(), Qt::AscendingOrder);

    connect(ui->includeAllPieces_ToolButton,     &QToolButton::clicked, this,  &PiecesWidget::includeAllPieces);
    connect(ui->invertIncludedPieces_ToolButton, &QToolButton::clicked, this,  [this]()
    {
        m_allPieces = m_data->DataPieces();
        invertIncludedPieces();
    });
    connect(ui->excludeAllPieces_ToolButton,     &QToolButton::clicked, this,  &PiecesWidget::excludeAllPieces);

    connect(ui->lockAllPieces_ToolButton,        &QToolButton::clicked, this,  &PiecesWidget::lockAllPieces);
    connect(ui->invertLockedPieces_ToolButton,   &QToolButton::clicked, this,  [this]()
    {
        m_allPieces = m_data->DataPieces();
        invertLockedPieces();
    });

    connect(ui->unlockAllPieces_ToolButton,      &QToolButton::clicked, this,  &PiecesWidget::unlockAllPieces);

    connect(ui->editColor_ToolButton, &QToolButton::clicked, this,  [this]()
    {
        QList<QTableWidgetItem *> selected = ui->tableWidget->selectedItems();
        if (selected.isEmpty())
        {
            QApplication::beep();
            return;
        }
        QTableWidgetItem *item = ui->tableWidget->item(selected.first()->row(), 0);
        if (!item) return;

        const quint32 id = item->data(Qt::UserRole).toUInt();
        const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
        const bool locked = allPieces->value(id).isLocked();

        if (locked == true)
        {
            QApplication::beep();
            return;
        }
        editPieceColor(id);
        ui->tableWidget->clearSelection();
        emit Highlight(NULL);
    });

    connect(ui->editPiece_ToolButton, &QToolButton::clicked, this,  [this]()
    {
        QList<QTableWidgetItem *> selected = ui->tableWidget->selectedItems();
        if (selected.isEmpty())
        {
            QApplication::beep();
            return;
        }
        QTableWidgetItem *item = ui->tableWidget->item(selected.first()->row(), 0);
        if (!item) return;

        const quint32 id = item->data(Qt::UserRole).toUInt();
        const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
        const bool locked = allPieces->value(id).isLocked();

        if (locked == true)
        {
            QApplication::beep();
            return;
        }
        editPieceProperties(id);
        ui->tableWidget->clearSelection();
        emit Highlight(NULL);
    });

    connect(ui->tableWidget, &QTableWidget::cellClicked,                       this, &PiecesWidget::cellClicked);
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked,                 this, &PiecesWidget::cellDoubleClicked);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested,        this, &PiecesWidget::showContextMenu);
    connect(ui->tableWidget->horizontalHeader(), &QHeaderView::sectionClicked, this, &PiecesWidget::headerClicked);
}

//---------------------------------------------------------------------------------------------------------------------
PiecesWidget::~PiecesWidget()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QWidget::changeEvent(event);
}
//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::togglePiece(quint32 id)
{
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->clearSelection();

    int selectedRow = 0;
    const QHash<quint32, VPiece> *pieces = m_data->DataPieces();

    if (pieces->contains(id))
    {
        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row, 0);
            SCASSERT(item != nullptr)

            if (item && item->data(Qt::UserRole).toUInt() == id)
            {
                    selectedRow = row;
                    VPiece piece = pieces->value(id);
                    const bool inLayout = piece.isInLayout();
                    inLayout ? item->setIcon(QIcon("://icon/32x32/visible_on.png"))
                             : item->setIcon(QIcon("://icon/32x32/visible_off.png"));
                    {
                        QTableWidgetItem *item = ui->tableWidget->item(row, 1);
                        const bool locked = piece.isLocked();
                        locked ? item->setIcon(QIcon("://icon/32x32/lock_on.png"))
                               : item->setIcon(QIcon("://icon/32x32/lock_off.png"));
                    }

                    {
                        QTableWidgetItem *item = ui->tableWidget->item(row, 2);
                        QPixmap pixmap(20, 20);
                        pixmap.fill(QColor(piece.getColor()));
                        item->setIcon(QIcon(pixmap));
                        item->setData(Qt::UserRole, piece.getColor());
                    }

                    {
                        QTableWidgetItem *item = ui->tableWidget->item(row, 3);
                        item->setText(formatLetterString(piece));
                    }

                    {
                        QTableWidgetItem *item = ui->tableWidget->item(row, 4);
                        item->setText(piece.GetName());
                    }
            }
        }
    }

    ui->tableWidget->selectRow(selectedRow);
    ui->tableWidget->setSortingEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::updateList()
{
    fillTable(m_data->DataPieces());
}


//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::selectPiece(quint32 id)
{
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);

        if (item->data(Qt::UserRole).toUInt() == id)
        {
            ui->tableWidget->setCurrentItem(item);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::cellClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (!item) return;

    const quint32 id = item->data(Qt::UserRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    const bool locked = allPieces->value(id).isLocked();

    if (column == 0)
    {
        if (locked == false)
        {
            const bool inLayout = !allPieces->value(id).isInLayout();

            TogglePieceInLayout *command = new TogglePieceInLayout(id, inLayout, m_data, m_doc);
            connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
            qApp->getUndoStack()->push(command);
        }
        else
        {
            QApplication::beep();
        }
    }
    else if (column == 1)
    {
        TogglePieceLock *command = new TogglePieceLock(id, !locked, m_data, m_doc);
        connect(command, &TogglePieceLock::updateList, this, &PiecesWidget::togglePiece);
        qApp->getUndoStack()->push(command);

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        emit scene->pieceLockedChanged(id, locked);
    }
    if (column == 2)
    {
        if (locked == true)
        {
            QApplication::beep();
        }
    }
    else if (column == 3 || column == 4)
    {
        if (locked == true)
        {
            QApplication::beep();
        }

    }
    emit Highlight(id);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::cellDoubleClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (!item) return;

    const quint32 id = item->data(Qt::UserRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    const bool locked = allPieces->value(id).isLocked();

    if (locked == true)
    {
        QApplication::beep();
        ui->tableWidget->clearSelection();
        emit Highlight(NULL);
        return;
    }

    if (column == 2)
    {
        editPieceColor(id);
    }
    else if (column == 3 || column == 4)
    {
        editPieceProperties(id);
    }
    ui->tableWidget->clearSelection();
    emit Highlight(NULL);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::fillTable(const QHash<quint32, VPiece> *pieces)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setRowCount(pieces->size());
    ui->tableWidget->setSortingEnabled(false);

    qint32 currentRow = -1;
    auto i = pieces->constBegin();
    while (i != pieces->constEnd())
    {
        ++currentRow;
        const VPiece piece = i.value();

        // Add in layout item
        PieceTableWidgetItem *item = new PieceTableWidgetItem(m_data);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(20, 20));
        item->setIcon(piece.isInLayout() ? QIcon("://icon/32x32/visible_on.png") : QIcon("://icon/32x32/visible_off.png"));
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() &= ~(Qt::ItemIsEditable)); // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Toggle inclusion of pattern piece in layout"));
        ui->tableWidget->setItem(currentRow, 0, item);

        // Add locked item
        item = new PieceTableWidgetItem(m_data);
        item->setTextAlignment(Qt::AlignHCenter);
        item->setSizeHint(QSize(20, 20));
        item->setIcon(piece.isLocked() ? QIcon("://icon/32x32/lock_on.png") : QIcon("://icon/32x32/lock_off.png"));
        item->setData(Qt::UserRole, i.key());
        item->setFlags(item->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Toggle lock on pattern piece"));
        ui->tableWidget->setItem(currentRow, 1, item);

        // Add color item
        item = new PieceTableWidgetItem(m_data);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setSizeHint(QSize(20, 20));
        QPixmap pixmap(20, 20);
        pixmap.fill(QColor(piece.getColor()));
        item->setIcon(QIcon(pixmap));
        item->setData(Qt::UserRole, piece.getColor());
        item->setFlags(item->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
        item->setToolTip(tr("Double click opens color selector"));
        ui->tableWidget->setItem(currentRow, 2, item);

        { // Add letter item
            QTableWidgetItem *item = new QTableWidgetItem(formatLetterString(piece));
            item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            item->setFlags(item->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
            item->setToolTip(tr("Double click opens pattern piece properties dialog"));
            ui->tableWidget->setItem(currentRow, 3, item);
        }

        { // Add name item
            QTableWidgetItem *item = new QTableWidgetItem(piece.GetName());
            item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
            item->setFlags(item->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
            item->setToolTip(tr("Double click opens pattern piece properties dialog"));
            ui->tableWidget->setItem(currentRow, 4, item);
        }

        ++i;
    }

    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem(makeHeaderName(tr("Included"))));
    ui->tableWidget->horizontalHeaderItem(0)->setToolTip(tr("Pattern piece is included in layout"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(makeHeaderName(tr("Locked"))));
    ui->tableWidget->horizontalHeaderItem(1)->setToolTip(tr("Pattern piece is locked"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(makeHeaderName(tr("Color"))));
    ui->tableWidget->horizontalHeaderItem(2)->setToolTip(tr("Pattern piece color"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(makeHeaderName(tr("Piece"))));
    ui->tableWidget->horizontalHeaderItem(3)->setToolTip(tr("Pattern piece letter"));
    ui->tableWidget->setHorizontalHeaderItem(4, new QTableWidgetItem(tr("Name")));
    ui->tableWidget->horizontalHeaderItem(4)->setToolTip(tr("Pattern piece name"));
    ui->tableWidget->horizontalHeaderItem(4)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::toggleInLayoutPieces(bool inLayout)
{
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->count() == 0)
    {
        return;
    }

    for (int row = 0; row<ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        const quint32 id = item->data(Qt::UserRole).toUInt();
        if (allPieces->contains(id))
        {
            if (!(inLayout == allPieces->value(id).isInLayout()))
            {
                TogglePieceInLayout *command = new TogglePieceInLayout(id, inLayout, m_data, m_doc);
                connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
                qApp->getUndoStack()->push(command);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::toggleLockedPieces(bool lock)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->count() == 0)
    {
        return;
    }

    for (int row = 0; row<ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 1);
        const quint32 id = item->data(Qt::UserRole).toUInt();
        if (allPieces->contains(id))
        {
            if (!(lock == allPieces->value(id).isLocked()))
            {
                TogglePieceLock *command = new TogglePieceLock(id, lock, m_data, m_doc);
                connect(command, &TogglePieceLock::updateList, this, &PiecesWidget::togglePiece);
                qApp->getUndoStack()->push(command);

                VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
                SCASSERT(scene != nullptr)
                emit scene->pieceLockedChanged(id, !lock);
            }
        }
    }
    ui->tableWidget->blockSignals(false);
}
/**
 * @brief headerClicked Sort state whenever header section clicked.
 * @param index
 */
void PiecesWidget::headerClicked(int index)
{
    QSettings settings;
    settings.setValue("pieceSort", index);
}

/**
 * @brief formatLetterString makes name to include piece letter and piece name
 * @param piece pattern piece
 */
QString PiecesWidget::formatLetterString(const VPiece piece)
{
    QRegularExpression regExp("^\\d+$");
    const VPieceLabelData& pieceData = piece.GetPatternPieceData();
    QString letter = pieceData.GetLetter();

    if (!letter.isEmpty())
    {
        if (regExp.match(letter).hasMatch())
        {
            int number = letter.toInt();
            letter = QStringLiteral("%1").arg(number, MAX_LENGTH, BASE_10, QLatin1Char('0'));
        }
    }
    return letter;
}
//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::showContextMenu(const QPoint &pos)
{
    ui->tableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->blockSignals(true);

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->tableWidget));
    QAction *selectAll = menu->addAction(tr("Include all pieces"));
    QAction *selectNone = menu->addAction(tr("Exclude all pieces"));
    QAction *invertSelection = menu->addAction(tr("Invert included pieces"));

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    menu->addAction(separator);

    QAction *lockAll = menu->addAction(tr("Lock all pieces"));
    QAction *unlockAll = menu->addAction(tr("Unlock all pieces"));
    QAction *invertLocked = menu->addAction(tr("Invert locked pieces"));

    m_allPieces = m_data->DataPieces();
    if (m_allPieces->isEmpty())
    {
        return;
    }

    int selectedPieces = 0;
    int lockedPieces = 0;

    auto piece = m_allPieces->constBegin();
    while (piece != m_allPieces->constEnd())
    {
        if(piece.value().isInLayout())
        {
            selectedPieces++;
        }
        if(piece.value().isLocked())
        {
            lockedPieces++;
        }
        ++piece;
    }

    if (selectedPieces == 0)
    {
        selectNone->setDisabled(true);
    }
    else if (selectedPieces == m_allPieces->size())
    {
        selectAll->setDisabled(true);
    }

    if (lockedPieces == 0)
    {
        unlockAll->setDisabled(true);
    }
    else if (lockedPieces == m_allPieces->size())
    {
        lockAll->setDisabled(true);
    }

    QAction *selectedAction = menu->exec(ui->tableWidget->viewport()->mapToGlobal(pos));

    if (selectedAction == selectAll)
    {
        includeAllPieces();
    }
    else if (selectedAction == selectNone)
    {
        excludeAllPieces();
    }
    else if (selectedAction == invertSelection)
    {
        invertIncludedPieces();
    }

    else if (selectedAction == lockAll)
    {
        lockAllPieces();
    }
    else if (selectedAction == unlockAll)
    {
        unlockAllPieces();
    }
    else if (selectedAction == invertLocked)
    {
        invertLockedPieces();
    }
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->blockSignals(false);
}

void PiecesWidget::includeAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Include all pieces"));
    toggleInLayoutPieces(true);
    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::invertIncludedPieces()
{
    if (m_allPieces->isEmpty())
    {
        return;
    }
    qApp->getUndoStack()->beginMacro(tr("Invert included pieces"));

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 0);
        const quint32 id = item->data(Qt::UserRole).toUInt();
        if (m_allPieces->contains(id))
        {
            const bool inLayout = !m_allPieces->value(id).isInLayout();

            TogglePieceInLayout *command = new TogglePieceInLayout(id, inLayout, m_data, m_doc);
            connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
            qApp->getUndoStack()->push(command);
        }
    }

    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::excludeAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Exclude all pieces"));
    toggleInLayoutPieces(false);
    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::lockAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Lock all pieces"));
    toggleLockedPieces(true);
    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::invertLockedPieces()
{
    if (m_allPieces->isEmpty())
    {
        return;
    }
    qApp->getUndoStack()->beginMacro(tr("Invert locked pieces"));

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, 1);
        const quint32 id = item->data(Qt::UserRole).toUInt();
        if (m_allPieces->contains(id))
        {
            const bool lock = !m_allPieces->value(id).isLocked();

            TogglePieceLock *command = new TogglePieceLock(id, lock, m_data, m_doc);
            connect(command, &TogglePieceLock::updateList, this, &PiecesWidget::togglePiece);
            qApp->getUndoStack()->push(command);

            VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
            SCASSERT(scene != nullptr)
            emit scene->pieceLockedChanged(id, !lock);
        }
    }

    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::unlockAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Unlock all pieces"));
    toggleLockedPieces(false);
    qApp->getUndoStack()->endMacro();
}

void PiecesWidget::editPieceColor(quint32 id)
{
    const QColor color = QColorDialog::getColor(Qt::white, this, tr("Select Color"), QColorDialog::DontUseNativeDialog);
    if (color.isValid())
    {
        SetPieceColor *command = new SetPieceColor(id, color.name(), m_data, m_doc);
        connect(command, &SetPieceColor::updateList, this, &PiecesWidget::togglePiece);
        qApp->getUndoStack()->push(command);
        emit Highlight(id);
    }
}

void PiecesWidget::editPieceProperties(quint32 id)
{
    PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
    SCASSERT(tool != nullptr);
    tool->editPieceProperties();
}

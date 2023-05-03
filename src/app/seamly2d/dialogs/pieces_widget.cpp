/***************************************************************************
 **  @file   pieces_widget.cpp
 **  @author Douglas S Caskey
 **  @date   Jan 3, 2023
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
#include "../vpatterndb/vcontainer.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/undocommands/togglepieceinlayout.h"
#include "../vtools/undocommands/toggle_piecelock.h"
#include "../vtools/undocommands/set_piece_color.h"
#include "../vwidgets/piece_tablewidgetitem.h"
#include "../vwidgets/vmaingraphicsscene.h"

#include <QColorDialog>
#include <QMenu>
#include <QPixmap>
#include <QTableWidget>
#include <QUndoStack>

//---------------------------------------------------------------------------------------------------------------------
PiecesWidget::PiecesWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PiecesWidget)
    , m_doc(doc)
    , m_data(data)
{
    ui->setupUi(this);

    ui->tableWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    fillTable(m_data->DataPieces());

    connect(ui->tableWidget, &QTableWidget::cellClicked,                this, &PiecesWidget::cellClicked);
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked,          this, &PiecesWidget::cellDoubleClicked);
    connect(ui->tableWidget, &QTableWidget::customContextMenuRequested, this, &PiecesWidget::showContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
PiecesWidget::~PiecesWidget()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::togglePiece(quint32 id)
{
    ui->tableWidget->setSortingEnabled(false);

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
                    inLayout ? item->setIcon(QIcon("://icon/32x32/checkmark.png")) : item->setIcon(QIcon());

                    QTableWidgetItem *lockItem = ui->tableWidget->item(row, 1);
                    const bool locked = piece.isLocked();
                    locked ? lockItem->setIcon(QIcon("://icon/32x32/lock_on.png"))
                           : lockItem->setIcon(QIcon("://icon/32x32/lock_off.png"));

                    QTableWidgetItem *colorItem = ui->tableWidget->item(row, 2);
                    QPixmap pixmap(20, 20);
                    pixmap.fill(QColor(piece.getColor()));
                    colorItem->setIcon(QIcon(pixmap));

                    QTableWidgetItem *nameItem = ui->tableWidget->item(row, 3);
                    nameItem->setText(piece.GetName());
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

    if (column == 0)
    {
        const bool inLayout = !allPieces->value(id).isInLayout();

        TogglePieceInLayout *command = new TogglePieceInLayout(id, inLayout, m_data, m_doc);
        connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
        qApp->getUndoStack()->push(command);
    }
    else if (column == 1)
    {
        const bool lock = !allPieces->value(id).isLocked();

        TogglePieceLock *command = new TogglePieceLock(id, lock, m_data, m_doc);
        connect(command, &TogglePieceLock::updateList, this, &PiecesWidget::togglePiece);
        qApp->getUndoStack()->push(command);

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        emit scene->pieceLockedChanged(id, !lock);
    }
    else if (column == 3)
    {
        emit Highlight(id);
    }
    ui->tableWidget->clearSelection();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::cellDoubleClicked(int row, int column)
{
    QTableWidgetItem *item = ui->tableWidget->item(row, 0);
    if (!item) return;

    const quint32 id = item->data(Qt::UserRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->value(id).isLocked())
    {
        QApplication::beep();
        ui->tableWidget->clearSelection();
        return;
    }

    if (column == 2)
    {
        const QColor color = QColorDialog::getColor(Qt::white, this, "Select Color", QColorDialog::DontUseNativeDialog);

        if (color.isValid())
        {
            SetPieceColor *command = new SetPieceColor(id, color.name(), m_data, m_doc);
            connect(command, &SetPieceColor::updateList, this, &PiecesWidget::togglePiece);
            qApp->getUndoStack()->push(command);
            emit Highlight(id);
        }
    }
    else if (column == 3)
    {
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
        SCASSERT(tool != nullptr);
        tool->editPieceProperties();
    }
    ui->tableWidget->clearSelection();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::fillTable(const QHash<quint32, VPiece> *pieces)
{
    ui->tableWidget->blockSignals(true);

    const int selectedRow = ui->tableWidget->currentRow();
    ui->tableWidget->clearContents();
    ui->tableWidget->setColumnCount(4);
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
        item->setIcon(piece.isInLayout() ? QIcon("://icon/32x32/checkmark.png") : QIcon());
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
        item->setToolTip("Toggle lock on pattern piece");
        ui->tableWidget->setItem(currentRow, 1, item);

        // Add color item
        item = new PieceTableWidgetItem(m_data);
        item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item->setSizeHint(QSize(20, 20));
        QPixmap pixmap(20, 20);
        pixmap.fill(QColor(piece.getColor()));
        item->setIcon(QIcon(pixmap));
        item->setFlags(item->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
        item->setToolTip("Double click opens color selector");
        ui->tableWidget->setItem(currentRow, 2, item);

        // Add name item
        QString name = piece.GetName();
        if (name.isEmpty())
        {
            name = tr("Unnamed");
        }
        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        nameItem->setFlags(nameItem->flags() &= ~(Qt::ItemIsEditable));  // set the item non-editable (view only), and non-selectable
        nameItem->setToolTip("Double click opens pattern piece properties dialog");
        ui->tableWidget->setItem(currentRow, 3, nameItem);

        ++i;
    }

    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("I"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem("L"));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem("C"));
    ui->tableWidget->setHorizontalHeaderItem(3, new QTableWidgetItem(tr("Name")));
    ui->tableWidget->horizontalHeaderItem(3)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setCurrentCell(selectedRow, 0);
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

    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->count() == 0)
    {
        return;
    }

    int selectedPieces = 0;
    int lockedPieces = 0;

    auto piece = allPieces->constBegin();
    while (piece != allPieces->constEnd())
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
    else if (selectedPieces == allPieces->size())
    {
        selectAll->setDisabled(true);
    }

    if (lockedPieces == 0)
    {
        unlockAll->setDisabled(true);
    }
    else if (lockedPieces == allPieces->size())
    {
        lockAll->setDisabled(true);
    }

    QAction *selectedAction = menu->exec(ui->tableWidget->viewport()->mapToGlobal(pos));

    if (selectedAction == selectAll)
    {
        qApp->getUndoStack()->beginMacro(tr("Include all pieces"));
        toggleInLayoutPieces(true);
        qApp->getUndoStack()->endMacro();
    }
    else if (selectedAction == selectNone)
    {
        qApp->getUndoStack()->beginMacro(tr("Exclude all pieces"));
        toggleInLayoutPieces(false);
        qApp->getUndoStack()->endMacro();
    }
    else if (selectedAction == invertSelection)
    {
        qApp->getUndoStack()->beginMacro(tr("Invert included pieces"));

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row, 0);
            const quint32 id = item->data(Qt::UserRole).toUInt();
            if (allPieces->contains(id))
            {
                const bool inLayout = !allPieces->value(id).isInLayout();

                TogglePieceInLayout *command = new TogglePieceInLayout(id, inLayout, m_data, m_doc);
                connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
                qApp->getUndoStack()->push(command);
            }
        }

        qApp->getUndoStack()->endMacro();
    }

    else if (selectedAction == lockAll)
    {
        qApp->getUndoStack()->beginMacro(tr("Lock all pieces"));
        toggleLockedPieces(true);
        qApp->getUndoStack()->endMacro();
    }
    else if (selectedAction == unlockAll)
    {
        qApp->getUndoStack()->beginMacro(tr("Unlock all pieces"));
        toggleLockedPieces(false);
        qApp->getUndoStack()->endMacro();
    }
    else if (selectedAction == invertLocked)
    {
        qApp->getUndoStack()->beginMacro(tr("Invert locked pieces"));

        for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
        {
            QTableWidgetItem *item = ui->tableWidget->item(row, 1);
            const quint32 id = item->data(Qt::UserRole).toUInt();
            if (allPieces->contains(id))
            {
                const bool lock = !allPieces->value(id).isLocked();

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
    ui->tableWidget->setSortingEnabled(true);
    ui->tableWidget->blockSignals(false);
}

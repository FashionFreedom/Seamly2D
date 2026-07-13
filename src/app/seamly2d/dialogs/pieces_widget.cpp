//-----------------------------------------------------------------------------
//  @file   pieces_widget.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   vwidgetdetails.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   Jun 25, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
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
//-----------------------------------------------------------------------------

#include "pieces_widget.h"
#include "ui_pieces_widget.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vwidgets/global.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/pattern_piece_tool.h"
#include "../vtools/tools/nodeDetails/vnodepoint.h"
#include "../vtools/undocommands/togglepieceinlayout.h"
#include "../vtools/undocommands/toggle_piecelock.h"
#include "../vtools/undocommands/set_piece_color.h"
#include "../vwidgets/vmaingraphicsscene.h"

#include "../vtools/undocommands/savepieceoptions.h"

#include <QColorDialog>
#include <QList>
#include <QSet>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QGraphicsPathItem>
#include <QPixmap>
#include <QRegularExpression>
#include <QTimer>
#include <QTreeWidget>
#include <QUndoStack>
#include <QLineF>

#include <algorithm>

namespace
{
    const int NODE_HIGHLIGHT_KEY = 1000;

    QString toolTypeIconPath(Tool toolType)
    {
        switch (toolType)
        {
            case Tool::NodePoint:
                return QStringLiteral("://icon/16x16/toolsectionpoint.png");
            case Tool::NodeArc:
                return QStringLiteral("://icon/16x16/toolsectionarc.png");
            case Tool::NodeElArc:
                return QStringLiteral("://icon/16x16/toolsectionelarc.png");
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            default:
                return QStringLiteral("://icon/16x16/toolsectioncurve.png");
        }
    }

    QString notchIconPath(NotchType notchType)
    {
        switch (notchType)
        {
            case NotchType::TNotch:
                return QStringLiteral("://icon/24x24/t_notch.png");
            case NotchType::VInternal:
                return QStringLiteral("://icon/24x24/internal_v_notch.png");
            case NotchType::VExternal:
                return QStringLiteral("://icon/24x24/external_v_notch.png");
            case NotchType::UNotch:
                return QStringLiteral("://icon/24x24/u_notch.png");
            case NotchType::Castle:
                return QStringLiteral("://icon/24x24/castle_notch.png");
            case NotchType::Diamond:
                return QStringLiteral("://icon/24x24/diamond_notch.png");
            case NotchType::Slit:
            default:
                return QStringLiteral("://icon/24x24/slit_notch.png");
        }
    }

    // Mirrors VAbstractCurve::DirectionArrows so a reversed node shows a reversed arrow.
    QVector<DirectionArrow> directionArrowsForPoints(const QVector<QPointF> &points, qreal length)
    {
        QVector<DirectionArrow> arrows;
        if (points.count() < 2)
        {
            return arrows;
        }

        const qreal seek_length = qAbs(length) / 2.0;
        qreal found_length = 0;
        QLineF arrow;
        for (qint32 i = 1; i <= points.size() - 1; ++i)
        {
            arrow = QLineF(points.at(i - 1), points.at(i));
            found_length += arrow.length();
            if (seek_length <= found_length)
            {
                arrow.setLength(arrow.length() - (found_length - seek_length));
                break;
            }
        }

        arrow = QLineF(arrow.p2(), arrow.p1());
        const qreal angle = arrow.angle();
        arrow.setLength(VAbstractCurve::lengthCurveDirectionArrow);

        DirectionArrow dArrow;
        arrow.setAngle(angle - 35);
        dArrow.first = arrow;
        arrow.setAngle(angle + 35);
        dArrow.second = arrow;
        arrows.append(dArrow);

        return arrows;
    }
}

#define BASE_10 10
#define MAX_LENGTH 3

//---------------------------------------------------------------------------------------------------------------------
PiecesWidget::PiecesWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PiecesWidget)
    , m_doc(doc)
    , m_data(data)
    , m_allPieces()
    , m_highlightedNodeId(NULL_ID)
    , m_fillTreeInProgress(false)
{
    ui->setupUi(this);

    ui->treeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeWidget->setDragDropMode(QAbstractItemView::InternalMove);
    ui->treeWidget->setDefaultDropAction(Qt::MoveAction);
    ui->treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->treeWidget->setHeaderLabels({makeHeaderName(tr("Included")),
                                     makeHeaderName(tr("Locked")),
                                     makeHeaderName(tr("Color")),
                                     makeHeaderName(tr("Piece")),
                                     tr("Name")});
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    ui->treeWidget->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(4, QHeaderView::Stretch);
    ui->treeWidget->setColumnWidth(0, 48);
    ui->treeWidget->setColumnWidth(1, 26);
    ui->treeWidget->setColumnWidth(2, 26);
    ui->treeWidget->setTextElideMode(Qt::ElideRight);
    ui->treeWidget->setIndentation(12);

    fillTree(m_data->DataPieces());

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
        QList<QTreeWidgetItem *> selected = ui->treeWidget->selectedItems();
        if (selected.isEmpty())
        {
            QApplication::beep();
            return;
        }
        QTreeWidgetItem *item = selected.first();
        if (item->data(0, IsNodeRole).toBool())
        {
            return;
        }

        const quint32 id = item->data(0, PieceIdRole).toUInt();
        const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
        const bool locked = allPieces->value(id).isLocked();

        if (locked == true)
        {
            QApplication::beep();
            return;
        }
        editPieceColor(id);
        ui->treeWidget->clearSelection();
        emit Highlight(NULL);
    });

    connect(ui->editPiece_ToolButton, &QToolButton::clicked, this,  [this]()
    {
        QList<QTreeWidgetItem *> selected = ui->treeWidget->selectedItems();
        if (selected.isEmpty())
        {
            QApplication::beep();
            return;
        }
        QTreeWidgetItem *item = selected.first();
        if (item->data(0, IsNodeRole).toBool())
        {
            return;
        }

        const quint32 id = item->data(0, PieceIdRole).toUInt();
        const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
        const bool locked = allPieces->value(id).isLocked();

        if (locked == true)
        {
            QApplication::beep();
            return;
        }
        editPieceProperties(id);
        ui->treeWidget->clearSelection();
        emit Highlight(NULL);
    });

    connect(ui->treeWidget, &QTreeWidget::itemClicked,                    this, &PiecesWidget::itemClicked);
    connect(ui->treeWidget, &QTreeWidget::itemDoubleClicked,              this, &PiecesWidget::itemDoubleClicked);
    connect(ui->treeWidget, &QTreeWidget::itemChanged,                    this, &PiecesWidget::itemChanged);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested,     this, &PiecesWidget::showContextMenu);
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

    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::togglePiece(quint32 id)
{
    const QHash<quint32, VPiece> *pieces = m_data->DataPieces();
    if (!pieces->contains(id))
    {
        return;
    }

    const VPiece piece = pieces->value(id);
    QList<QTreeWidgetItem *> items = allPieceItems();

    for (QTreeWidgetItem *item : items)
    {
        if (item->data(0, PieceIdRole).toUInt() == id)
        {
            PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
            const bool visible = tool ? tool->isVisible() : true;
            item->setIcon(0, visible ? QIcon("://icon/32x32/visible_on.png")
                                     : QIcon("://icon/32x32/visible_off.png"));
            item->setIcon(1, piece.isLocked() ? QIcon("://icon/32x32/lock_on.png")
                                              : QIcon("://icon/32x32/lock_off.png"));

            QPixmap pixmap(20, 20);
            pixmap.fill(QColor(piece.getColor()));
            item->setIcon(2, QIcon(pixmap));

            item->setText(3, formatLetterString(piece));
            item->setText(4, piece.GetName());

            ui->treeWidget->setCurrentItem(item);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::updateList()
{
    fillTree(m_data->DataPieces());
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::selectPiece(quint32 id)
{
    QList<QTreeWidgetItem *> items = allPieceItems();
    for (QTreeWidgetItem *item : items)
    {
        if (item->data(0, PieceIdRole).toUInt() == id)
        {
            ui->treeWidget->setCurrentItem(item);
            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::clear()
{
    ui->treeWidget->clear();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::itemClicked(QTreeWidgetItem *item, int column)
{
    if (!item)
    {
        return;
    }

    if (item->data(0, IsNodeRole).toBool())
    {
        const quint32 nodeId = item->data(0, NodeIdRole).toUInt();
        clearNodeHighlight();
        if (nodeId == NULL_ID)
        {
            return;
        }

        // In Piece (Stück) mode the current scene is the piece scene, where the
        // VNodePoint items live as children of the PatternPieceTool. NodeIdRole holds
        // the VNode tool id, so getTool() returns the node tool directly.
        // getTool() throws VExceptionBadId for an unknown id.
        VNodePoint *nodePoint = nullptr;
        try
        {
            nodePoint = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(nodeId));
        }
        catch (const VExceptionBadId &)
        {
            return;
        }

        if (nodePoint != nullptr)
        {
            // Point node: it has its own scene item, highlight it directly.
            if (nodePoint->isVisible())
            {
                nodePoint->setHighlighted(true);
                nodePoint->ensureVisible();
                m_highlightedNodeId = nodeId;
            }
        }
        else
        {
            // Curve node (spline/arc/...): no own scene item, it is drawn by the piece
            // path. Draw a temporary red overlay along the curve geometry instead.
            highlightCurveNode(nodeId, item->parent());
        }
        return;
    }

    clearNodeHighlight();

    const quint32 id = item->data(0, PieceIdRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    const bool locked = allPieces->value(id).isLocked();

    if (column == 0)
    {
        PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
        if (tool)
        {
            const bool nowVisible = !tool->isVisible();
            tool->setVisible(nowVisible);

            TogglePieceInLayout *command = new TogglePieceInLayout(id, nowVisible, m_data, m_doc);
            connect(command, &TogglePieceInLayout::updateList, this, &PiecesWidget::togglePiece);
            qApp->getUndoStack()->push(command);

            ui->treeWidget->blockSignals(true);
            item->setIcon(0, nowVisible ? QIcon("://icon/32x32/visible_on.png")
                                        : QIcon("://icon/32x32/visible_off.png"));
            ui->treeWidget->blockSignals(false);
        }
        return;
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
    else if (column == 2 || column == 3 || column == 4)
    {
        if (locked == true)
        {
            QApplication::beep();
        }
    }
    emit Highlight(id);
    emit pieceSelected(id);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (!item)
    {
        return;
    }

    if (item->data(0, IsNodeRole).toBool())
    {
        return;
    }

    const quint32 id = item->data(0, PieceIdRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    const bool locked = allPieces->value(id).isLocked();

    if (locked == true)
    {
        QApplication::beep();
        ui->treeWidget->clearSelection();
        emit Highlight(NULL);
        return;
    }

    if (column == 2)
    {
        editPieceColor(id);
        ui->treeWidget->clearSelection();
        emit Highlight(NULL);
    }
    else if (column == 3)
    {
        editPieceProperties(id);
        ui->treeWidget->clearSelection();
        emit Highlight(NULL);
    }
    else if (column == 4)
    {
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->treeWidget->editItem(item, 4);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem *PiecesWidget::createPieceItem(quint32 id, const VPiece &piece)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();

    PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
    const bool visible = tool ? tool->isVisible() : true;
    item->setIcon(0, visible ? QIcon("://icon/32x32/visible_on.png")
                             : QIcon("://icon/32x32/visible_off.png"));
    item->setToolTip(0, tr("Toggle visibility of pattern piece on canvas"));

    item->setIcon(1, piece.isLocked() ? QIcon("://icon/32x32/lock_on.png")
                                      : QIcon("://icon/32x32/lock_off.png"));
    item->setToolTip(1, tr("Toggle lock on pattern piece"));

    QPixmap pixmap(20, 20);
    pixmap.fill(QColor(piece.getColor()));
    item->setIcon(2, QIcon(pixmap));
    item->setToolTip(2, tr("Double click opens color selector"));

    item->setText(3, formatLetterString(piece));
    item->setToolTip(3, tr("Double click opens pattern piece properties dialog"));

    item->setText(4, piece.GetName());
    item->setToolTip(4, tr("Click to rename"));

    item->setData(0, PieceIdRole, id);
    item->setData(0, IsNodeRole, false);
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);

    const QVector<VPieceNode> nodes = piece.GetPath().getNodes();
    for (const VPieceNode &node : nodes)
    {
        QTreeWidgetItem *nodeItem = createNodeItem(node);
        if (nodeItem)
        {
            item->addChild(nodeItem);
        }
    }

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem *PiecesWidget::createNodeItem(const VPieceNode &node)
{
    QTreeWidgetItem *item = new QTreeWidgetItem();

    const bool isPoint = (node.GetTypeTool() == Tool::NodePoint);

    QString name;
    try
    {
        if (isPoint)
        {
            name = m_data->GeometricObject<VPointF>(node.GetId())->name();
        }
        else
        {
            name = m_data->GeometricObject<VAbstractCurve>(node.GetId())->name();
        }
    }
    catch (...)
    {
        return nullptr;
    }

    // Column 2: modifier icon — notch-type icon for points, reverse icon for curves.
    // (A node is either a point or a curve, so the column serves both cases.)
    if (isPoint && node.isNotch())
    {
        item->setIcon(2, QIcon(notchIconPath(node.getNotchType())));
        item->setToolTip(2, tr("Notch"));
    }
    else if (!isPoint && node.GetReverse())
    {
        item->setIcon(2, QIcon(QStringLiteral("://icon/24x24/reverse.png")));
        item->setToolTip(2, tr("Reversed"));
    }

    // Column 3: tool-type icon — depends only on the node's tool type.
    item->setIcon(3, QIcon(toolTypeIconPath(node.GetTypeTool())));

    item->setText(4, name);
    item->setToolTip(4, tr("Double click to edit node"));

    if (node.isExcluded())
    {
        QFont strikeFont = item->font(4);
        strikeFont.setStrikeOut(true);
        item->setFont(4, strikeFont);
        item->setForeground(4, QColor(Qt::gray));
    }

    item->setData(0, IsNodeRole, true);
    item->setData(0, NodeIdRole, node.GetId());
    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::fillTree(const QHash<quint32, VPiece> *pieces)
{
    if (m_fillTreeInProgress)
    {
        return;
    }
    m_fillTreeInProgress = true;

    QSet<quint32> expandedPieces;
    QList<QTreeWidgetItem *> oldItems = allPieceItems();
    for (QTreeWidgetItem *old : oldItems)
    {
        if (old->isExpanded())
        {
            expandedPieces.insert(old->data(0, PieceIdRole).toUInt());
        }
    }

    ui->treeWidget->blockSignals(true);
    ui->treeWidget->clear();

    auto it = pieces->constBegin();
    while (it != pieces->constEnd())
    {
        QTreeWidgetItem *item = createPieceItem(it.key(), it.value());
        ui->treeWidget->addTopLevelItem(item);
        item->setExpanded(expandedPieces.contains(it.key()));
        ++it;
    }

    ui->treeWidget->blockSignals(false);
    m_fillTreeInProgress = false;
}

//---------------------------------------------------------------------------------------------------------------------
QList<QTreeWidgetItem *> PiecesWidget::allPieceItems() const
{
    QList<QTreeWidgetItem *> result;
    for (int i = 0; i < ui->treeWidget->topLevelItemCount(); ++i)
    {
        result.append(ui->treeWidget->topLevelItem(i));
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::itemChanged(QTreeWidgetItem *item, int column)
{
    if (!item || column != 4 || item->data(0, IsNodeRole).toBool())
    {
        return;
    }

    item->setFlags(item->flags() & ~Qt::ItemIsEditable);

    const quint32 id = item->data(0, PieceIdRole).toUInt();
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (!allPieces->contains(id))
    {
        return;
    }

    const QString newName = item->text(4).trimmed();
    VPiece oldPiece = allPieces->value(id);
    if (newName.isEmpty() || newName == oldPiece.GetName())
    {
        return;
    }

    VPiece newPiece = oldPiece;
    newPiece.SetName(newName);

    SavePieceOptions *command = new SavePieceOptions(oldPiece, newPiece, m_doc, id);
    qApp->getUndoStack()->push(command);
    QTimer::singleShot(0, this, &PiecesWidget::updateList);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::toggleInLayoutPieces(bool inLayout)
{
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->count() == 0)
    {
        return;
    }

    QList<QTreeWidgetItem *> items = allPieceItems();
    for (QTreeWidgetItem *item : items)
    {
        const quint32 id = item->data(0, PieceIdRole).toUInt();
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
    ui->treeWidget->blockSignals(true);
    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (allPieces->count() == 0)
    {
        return;
    }

    QList<QTreeWidgetItem *> items = allPieceItems();
    for (QTreeWidgetItem *item : items)
    {
        const quint32 id = item->data(0, PieceIdRole).toUInt();
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
    ui->treeWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
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
    QTreeWidgetItem *clickedItem = ui->treeWidget->itemAt(pos);
    if (clickedItem && clickedItem->data(0, IsNodeRole).toBool())
    {
        showNodeContextMenu(clickedItem, ui->treeWidget->viewport()->mapToGlobal(pos));
        return;
    }

    ui->treeWidget->blockSignals(true);

    QScopedPointer<QMenu> menu(new QMenu(ui->treeWidget));
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

    QAction *selectedAction = menu->exec(ui->treeWidget->viewport()->mapToGlobal(pos));

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
    ui->treeWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::includeAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Include all pieces"));
    toggleInLayoutPieces(true);
    qApp->getUndoStack()->endMacro();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::invertIncludedPieces()
{
    if (m_allPieces->isEmpty())
    {
        return;
    }
    qApp->getUndoStack()->beginMacro(tr("Invert included pieces"));

    QList<QTreeWidgetItem *> items = allPieceItems();
    for (QTreeWidgetItem *item : items)
    {
        const quint32 id = item->data(0, PieceIdRole).toUInt();
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

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::excludeAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Exclude all pieces"));
    toggleInLayoutPieces(false);
    qApp->getUndoStack()->endMacro();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::lockAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Lock all pieces"));
    toggleLockedPieces(true);
    qApp->getUndoStack()->endMacro();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::invertLockedPieces()
{
    if (m_allPieces->isEmpty())
    {
        return;
    }
    qApp->getUndoStack()->beginMacro(tr("Invert locked pieces"));

    QList<QTreeWidgetItem *> items = allPieceItems();
    for (QTreeWidgetItem *item : items)
    {
        const quint32 id = item->data(0, PieceIdRole).toUInt();
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

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::unlockAllPieces()
{
    qApp->getUndoStack()->beginMacro(tr("Unlock all pieces"));
    toggleLockedPieces(false);
    qApp->getUndoStack()->endMacro();
}

//---------------------------------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::editPieceProperties(quint32 id)
{
    PatternPieceTool *tool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
    SCASSERT(tool != nullptr);
    tool->editPieceProperties();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::clearNodeHighlight()
{
    // Clear a highlighted point node.
    if (m_highlightedNodeId != NULL_ID)
    {
        try
        {
            VNodePoint *nodePoint = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(m_highlightedNodeId));
            if (nodePoint != nullptr)
            {
                nodePoint->setHighlighted(false);
            }
        }
        catch (const VExceptionBadId &)
        {
            // tool no longer exists (e.g. piece deleted) — nothing to clear
        }
        m_highlightedNodeId = NULL_ID;
    }

    // Remove any temporary curve-highlight overlay. Query the live scene so we never
    // dereference an item the scene may already have deleted (e.g. on file reload).
    QGraphicsScene *scene = qApp->getCurrentScene();
    if (scene != nullptr)
    {
        const QList<QGraphicsItem *> items = scene->items();
        for (QGraphicsItem *gItem : items)
        {
            if (gItem->data(NODE_HIGHLIGHT_KEY).toBool())
            {
                scene->removeItem(gItem);
                delete gItem;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::highlightCurveNode(quint32 nodeId, QTreeWidgetItem *pieceItem)
{
    QGraphicsScene *scene = qApp->getCurrentScene();
    if (scene == nullptr)
    {
        return;
    }

    // Find whether this node is reversed within its owning piece.
    bool reversed = false;
    if (pieceItem != nullptr)
    {
        const quint32 pieceId = pieceItem->data(0, PieceIdRole).toUInt();
        const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
        if (allPieces->contains(pieceId))
        {
            const QVector<VPieceNode> nodes = allPieces->value(pieceId).GetPath().getNodes();
            for (const VPieceNode &n : nodes)
            {
                if (n.GetId() == nodeId)
                {
                    reversed = n.GetReverse();
                    break;
                }
            }
        }
    }

    QPainterPath path;
    try
    {
        const QSharedPointer<VAbstractCurve> curve = m_data->GeometricObject<VAbstractCurve>(nodeId);
        path = curve->GetPath();

        // Append the direction arrow, mirroring how curves show direction in draft mode.
        // Reverse the point order for reversed nodes so the arrow points the right way.
        QVector<QPointF> points = curve->getPoints();
        if (reversed)
        {
            std::reverse(points.begin(), points.end());
        }
        const QPainterPath arrows = VAbstractCurve::ShowDirection(
            directionArrowsForPoints(points, curve->GetLength()),
            scaleWidth(VAbstractCurve::lengthCurveDirectionArrow, sceneScale(scene)));
        if (arrows != QPainterPath())
        {
            path.addPath(arrows);
        }
    }
    catch (...)
    {
        return;
    }

    if (path == QPainterPath())
    {
        return;
    }

    QGraphicsPathItem *overlay = new QGraphicsPathItem(path);
    QPen pen(Qt::red);
    pen.setWidth(3);
    pen.setCosmetic(true);   // constant width regardless of zoom
    overlay->setPen(pen);
    overlay->setBrush(Qt::NoBrush);
    overlay->setZValue(1000);
    overlay->setData(NODE_HIGHLIGHT_KEY, true);

    // Parent the overlay to the PatternPieceTool so it follows the piece's transform;
    // the curve geometry is in the same local coordinate system as the piece nodes.
    PatternPieceTool *pieceTool = nullptr;
    if (pieceItem != nullptr)
    {
        const quint32 pieceId = pieceItem->data(0, PieceIdRole).toUInt();
        if (pieceId != NULL_ID)
        {
            try
            {
                pieceTool = qobject_cast<PatternPieceTool *>(VAbstractPattern::getTool(pieceId));
            }
            catch (const VExceptionBadId &)
            {
                pieceTool = nullptr;
            }
        }
    }

    if (pieceTool != nullptr)
    {
        overlay->setParentItem(pieceTool);
    }
    else
    {
        scene->addItem(overlay);
    }

    overlay->ensureVisible();
}

//---------------------------------------------------------------------------------------------------------------------
void PiecesWidget::showNodeContextMenu(QTreeWidgetItem *item, const QPoint &globalPos)
{
    if (!item || !item->parent())
    {
        return;
    }

    const quint32 nodeId = item->data(0, NodeIdRole).toUInt();
    const quint32 pieceId = item->parent()->data(0, PieceIdRole).toUInt();

    const QHash<quint32, VPiece> *allPieces = m_data->DataPieces();
    if (!allPieces->contains(pieceId))
    {
        return;
    }

    VPiece piece = allPieces->value(pieceId);
    QVector<VPieceNode> nodes = piece.GetPath().getNodes();

    int nodeIndex = -1;
    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes.at(i).GetId() == nodeId)
        {
            nodeIndex = i;
            break;
        }
    }
    if (nodeIndex < 0)
    {
        return;
    }

    const VPieceNode &node = nodes.at(nodeIndex);
    const bool isPoint = (node.GetTypeTool() == Tool::NodePoint);
    const bool excluded = node.isExcluded();

    QScopedPointer<QMenu> menu(new QMenu(ui->treeWidget));

    QAction *toggleExclude = menu->addAction(excluded ? tr("Include object")
                                                      : tr("Exclude object"));

    QAction *toggleReverse = nullptr;
    if (!isPoint)
    {
        toggleReverse = menu->addAction(tr("Reverse direction"));
    }

    QAction *addDefaultNotch = nullptr;
    QAction *toggleNotch = nullptr;
    if (isPoint)
    {
        if (!node.isNotch())
        {
            addDefaultNotch = menu->addAction(tr("Add standard notch"));
            toggleNotch = menu->addAction(tr("Add notch"));
        }
        else
        {
            toggleNotch = menu->addAction(tr("Remove notch"));
        }
    }

    QAction *selectedAction = menu->exec(globalPos);
    if (!selectedAction)
    {
        return;
    }

    if (selectedAction == toggleExclude)
    {
        nodes[nodeIndex].SetExcluded(!excluded);
    }
    else if (selectedAction == toggleReverse && toggleReverse)
    {
        nodes[nodeIndex].SetReverse(!node.GetReverse());
    }
    else if (selectedAction == addDefaultNotch && addDefaultNotch)
    {
        nodes[nodeIndex].setNotch(true);
        nodes[nodeIndex].setNotchType(
            static_cast<NotchType>(qApp->Settings()->GetDefaultNotchType()));
        nodes[nodeIndex].setNotchSubType(
            static_cast<NotchSubType>(qApp->Settings()->GetDefaultNotchSubType()));
    }
    else if (selectedAction == toggleNotch && toggleNotch)
    {
        nodes[nodeIndex].setNotch(!node.isNotch());
    }
    else
    {
        return;
    }

    VPiece newPiece = piece;
    newPiece.GetPath().setNodes(nodes);

    SavePieceOptions *command = new SavePieceOptions(piece, newPiece, m_doc, pieceId);
    qApp->getUndoStack()->push(command);
    // Rebuild after the command (and its lite parse) has fully applied, so the tree
    // reflects the new node state. Doing it here (not via NeedLiteParsing) avoids reading
    // stale container data before the parse completes.
    updateList();
}

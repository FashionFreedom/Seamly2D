/***************************************************************************
 **  @file   insert_nodes_dialog.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 202
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   dialoginsertnode.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 3, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "insert_nodes_dialog.h"
#include "ui_insert_nodes_dialog.h"
#include "../vpatterndb/vcontainer.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "vpointf.h"

#include <QMenu>
#include <QKeyEvent>
#include <QSound>

//---------------------------------------------------------------------------------------------------------------------
InsertNodesDialog::InsertNodesDialog(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::InsertNodesDialog)
    , m_nodes({})
    , m_nodeFlag(false)
    , m_piecesFlag(false)
    , m_beep(new QSound(qApp->Settings()->getSelectionSound()))
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/insert_nodes_icon.png"));
    ui->statusMsg_Label->setText("");

    ui->nodes_ListWidget->installEventFilter(this);

    initializeOkCancel(ui);

    validatePieces();

    connect(ui->piece_ComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this]()
    {
        validatePieces();
    });

    connect(ui->nodes_ListWidget, &QListWidget::customContextMenuRequested, this, &InsertNodesDialog::showContextMenu);
}

//---------------------------------------------------------------------------------------------------------------------
InsertNodesDialog::~InsertNodesDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void InsertNodesDialog::SetPiecesList(const QVector<quint32> &list)
{
    FillComboBoxPiecesList(ui->piece_ComboBox, list);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 InsertNodesDialog::getPieceId() const
{
    return getCurrentObjectId(ui->piece_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VPieceNode> InsertNodesDialog::getNodes() const
{
    QVector<VPieceNode> nodes;
    for (qint32 i = 0; i < ui->nodes_ListWidget->count(); ++i)
    {
        VPieceNode node = qvariant_cast<VPieceNode>(ui->nodes_ListWidget->item(i)->data(Qt::UserRole));
        nodes.append(node);
    }
    return nodes;
}

//---------------------------------------------------------------------------------------------------------------------
void InsertNodesDialog::ShowDialog(bool click)
{
    if (!click)
    {
        if (m_nodes.isEmpty())
        {
            return;
        }

        for (auto &node : m_nodes)
        {
            newNodeItem(ui->nodes_ListWidget, node, false, false);
        }

        m_nodes.clear();

        validateNodes();

        prepare = true;
        setModal(true);
        emit ToolTip(QString());
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Handles emitted signal from the scene if an object was selected.
 * @param selected true is object was selected.
 * @param objId Object Id of the selected object.
 * @param toolId Tool Id of the selected object.
 */
void InsertNodesDialog::SelectedObject(bool selected, quint32 objId, quint32 toolId)
{
    Q_UNUSED(toolId)

    if (prepare)
    {
        return;
    }

    auto nodeIterator = std::find_if(m_nodes.begin(), m_nodes.end(),
                                     [objId](const VPieceNode &node) { return node.GetId() == objId; });
    if (selected)
    {
        if (nodeIterator == m_nodes.cend())
        {
            GOType objType = GOType::Unknown;
            try
            {
                objType = data->GetGObject(objId)->getType();
            }
            catch (const VExceptionBadId &)
            {
                qWarning() << "Cannot find an object with id" << objId;
                return;
            }
            m_beep->play();
            bool appendCurve = false;
            QSharedPointer<VGObject> previousObj = nullptr;
            quint32 previousObjId = getLastNodeId();
            if (previousObjId != NULL_ID)
            {
                previousObj = data->GetGObject(previousObjId);
            }

            VPieceNode node;
            VPieceNode node2;
            switch (objType)
            {
                case GOType::Point:
                    node = VPieceNode(objId, Tool::NodePoint);
                    if (previousObj != nullptr)
                    {
                        GOType previousObjType = previousObj->getType();
                        if (previousObjType == GOType::Arc || previousObjType == GOType::EllipticalArc  ||
                            previousObjType == GOType::Spline || previousObjType == GOType::CubicBezier)
                        {
                            const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(previousObjId);
                            const QPointF point = static_cast<QPointF>(*data->GeometricObject<VPointF>(objId));

                            if (curve->isPointOnCurve(point) &&
                                point != curve->getFirstPoint() &&
                                point != curve->getLastPoint())
                            {
                                switch (previousObjType)
                                {
                                    case GOType::Arc:
                                        node2 = VPieceNode(previousObjId, Tool::NodeArc, m_nodes.last().GetReverse());
                                        appendCurve = true;
                                        break;
                                    case GOType::EllipticalArc:
                                        node2 = VPieceNode(previousObjId, Tool::NodeElArc, m_nodes.last().GetReverse());
                                        appendCurve = true;
                                        break;
                                    case GOType::Spline:
                                    case GOType::CubicBezier:
                                        node2 = VPieceNode(previousObjId, Tool::NodeSpline, m_nodes.last().GetReverse());
                                        appendCurve = true;
                                        break;
                                    case GOType::SplinePath:
                                    case GOType::CubicBezierPath:
                                        node2 = VPieceNode(previousObjId, Tool::NodeSplinePath, m_nodes.last().GetReverse());
                                        appendCurve = true;
                                        break;
                                    case GOType::Point:
                                    case GOType::Unknown:
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    break;
                case GOType::Arc:
                    node = VPieceNode(objId, Tool::NodeArc, correctCurveDirection(objId));
                    break;
                case GOType::EllipticalArc:
                    node = VPieceNode(objId, Tool::NodeElArc, correctCurveDirection(objId));
                    break;
                case GOType::Spline:
                case GOType::CubicBezier:
                    node = VPieceNode(objId, Tool::NodeSpline, correctCurveDirection(objId));
                    break;
                case GOType::SplinePath:
                case GOType::CubicBezierPath:
                    node = VPieceNode(objId, Tool::NodeSplinePath, correctCurveDirection(objId));
                    break;
                case GOType::Unknown:
                default:
                    qWarning() << "Ignore unknown object type.";
                    return;
            }

            node.SetExcluded(true);
            m_nodes.append(node);
            if (appendCurve)
            {
                node2.SetExcluded(true);
                m_nodes.append(node2);
            }
            if (objType != GOType::Point)
            {
                insertCurveNodes(node);
            }
        }
    }
    else
    {
        if (nodeIterator != m_nodes.end())
        {
            m_nodes.erase(nodeIterator);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Displays a context menu for the list of node items.
 *        Allows user to set notch type of point nodes.
 *        Allows user to reverse a curve node.
 *        Allows user to delete a node from the list.
 * @param pos Mouse position on the selected list item to display the menu at.
 */
void InsertNodesDialog::showContextMenu(const QPoint &pos)
{
    const int row = ui->nodes_ListWidget->currentRow();
    if (ui->nodes_ListWidget->count() == 0 || row == -1 || row >= ui->nodes_ListWidget->count())
    {
        return;
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-97559: assign parent to QMenu
    QScopedPointer<QMenu> menu(new QMenu(ui->nodes_ListWidget));
    NodeInfo info;
    NotchType notchType = NotchType::Slit;
    bool isNotch = false;
    QListWidgetItem *rowItem = ui->nodes_ListWidget->item(row);
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
    QAction *actionReverse   = nullptr;

    if (rowNode.GetTypeTool() != Tool::NodePoint)
    {
        actionReverse = menu->addAction(tr("Reverse"));
        actionReverse->setCheckable(true);
        actionReverse->setChecked(rowNode.GetReverse());
    }
    else
    {

        QMenu *notchMenu = menu->addMenu(tr("Notch"));
        actionNotch = notchMenu->menuAction();
        actionNotch->setCheckable(true);
        actionNotch->setChecked(rowNode.isNotch());

        actionNone      = notchMenu->addAction( tr("None"));
        actionSlit      = notchMenu->addAction(QIcon("://icon/24x24/slit_notch.png"), tr("Slit"));
        actionTNotch    = notchMenu->addAction(QIcon("://icon/24x24/t_notch.png"), tr("TNotch"));
        actionUNotch    = notchMenu->addAction(QIcon("://icon/24x24/u_notch.png"), tr("UNotch"));
        actionVInternal = notchMenu->addAction(QIcon("://icon/24x24/internal_v_notch.png"), tr("VInternal"));
        actionVExternal = notchMenu->addAction(QIcon("://icon/24x24/external_v_notch.png"), tr("VExternal"));
        actionCastle    = notchMenu->addAction(QIcon("://icon/24x24/castle_notch.png"), tr("Castle"));
        actionDiamond   = notchMenu->addAction(QIcon("://icon/24x24/diamond_notch.png"), tr("Diamond"));
    }

    QAction *actionDelete   = menu->addAction(QIcon::fromTheme("edit-delete"), tr("Delete") + QStringLiteral("\tDel"));
    QAction *selectedAction = menu->exec(ui->nodes_ListWidget->viewport()->mapToGlobal(pos));
    if (selectedAction == actionReverse && rowNode.GetTypeTool() != Tool::NodePoint)
    {
        rowNode.SetReverse(!rowNode.GetReverse());
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }
    else if (selectedAction == actionDelete)
    {
        delete rowItem;
        if (ui->nodes_ListWidget->count() == 0)
        {
            ui->statusMsg_Label->setText(tr("No nodes selected. Press Cancel to continue"));
        }
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

        rowNode.setNotch(isNotch);
        rowNode.setNotchType(notchType);
        info = getNodeInfo(rowNode, true);
        rowItem->setData(Qt::UserRole, QVariant::fromValue(rowNode));
        rowItem->setIcon(QIcon(info.icon));
        rowItem->setText(info.name);
    }

    validateNodes();
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Disable OK button if there are no nodes or a pattern piece selected.
 */
void InsertNodesDialog::checkState()
{
    SCASSERT(ok_Button != nullptr);
    ok_Button->setEnabled(m_nodeFlag && m_piecesFlag);
}


//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Filters keyboard event to check if the delete key was pressed.
 * @param object QObject that sent the event.
 * @param event QEvent.
 * @return If the node list was the sending object: True if the key pressed was the Delete key False if any other key.
 *         If event sent by any other object pass the event on to the parent.
 */
bool InsertNodesDialog::eventFilter(QObject *object, QEvent *event)
{
    if (QListWidget *list = qobject_cast<QListWidget *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Delete)
            {
                const int row = ui->nodes_ListWidget->currentRow();
                if (ui->nodes_ListWidget->count() > 0 || row > -1 || row < ui->nodes_ListWidget->count())
                {
                    QListWidgetItem *rowItem = list->item(row);
                    SCASSERT(rowItem != nullptr);
                    delete rowItem;
                }
                return true;
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
void InsertNodesDialog::validatePieces()
{
    QColor color = okColor;
    if (ui->piece_ComboBox->count() <= 0 || ui->piece_ComboBox->currentIndex() == -1)
    {
        m_piecesFlag = false;
        color = errorColor;
    }
    else
    {
        m_piecesFlag = true;
        color = okColor;
    }
    ChangeColor(ui->piece_Label, color);
    checkState();
}

//---------------------------------------------------------------------------------------------------------------------
void InsertNodesDialog::validateNodes()
{
    m_nodeFlag = ui->nodes_ListWidget->count() > 0;
    checkState();
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Gets last node in the list.
 * @return node Id if list not empty otherwise NULL_ID
 */
 quint32 InsertNodesDialog::getLastNodeId() const
{
    if (!m_nodes.isEmpty())
    {
        VPieceNode node = m_nodes.last();
        return node.GetId();
    }
    else
    {
        return NULL_ID;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Checks whether a selected curve needs to be reversed so it's direction is clockwise.
 * @param curveObjId Id of selected node.
 * @return true if curve needs to be reversed or false if curve does not need to be reversed.
*/
bool InsertNodesDialog::correctCurveDirection(quint32 curveObjId)
{
    const QSharedPointer<VGObject> curveObj = data->GetGObject(curveObjId);
    const QString curveName = curveObj->name();
    quint32 nodeId;
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(curveObjId);

    for (auto &node : m_nodes)
    {
        nodeId = node.GetId();
        const QSharedPointer<VGObject> nodeObj = data->GetGObject(nodeId);
        if (static_cast<GOType>(nodeObj->getType()) == GOType::Point)
        {
            const QPointF point = static_cast<QPointF>(*data->GeometricObject<VPointF>(nodeId));
            if (point == curve->getLastPoint())
            {
                ui->statusMsg_Label->setText(ui->statusMsg_Label->text() + curveName +
                                             tr(" was auto reversed.") + "\n");
                return true;
            }
            else if (point == curve->getFirstPoint())
            {
                return false;
            }
        }
    }
    ui->statusMsg_Label->setText(ui->statusMsg_Label->text() + curveName +
                                 tr(" may need to be manually reversed.") + "\n");
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/*
 * @brief Checks whether to insert of copy of the selected curve before any node points that may exist on the curve.
 * @param curveNode selected curve node.
 */
void InsertNodesDialog::insertCurveNodes(VPieceNode curveNode)
{
    quint32 nodeId;
    quint32 prevNodeId;
    quint32 curveNodeId = curveNode.GetId();

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(curveNodeId);
    for (int i = 0; i < m_nodes.size(); ++i)
    {
        nodeId = m_nodes.value(i).GetId();
        const QSharedPointer<VGObject> nodeObj = data->GetGObject(nodeId);
        if (static_cast<GOType>(nodeObj->getType()) == GOType::Point)
        {
            const QPointF point = static_cast<QPointF>(*data->GeometricObject<VPointF>(nodeId));
            if (curve->isPointOnCurve(point) &&
                point != curve->getFirstPoint() &&
                point != curve->getLastPoint() &&
                curveNodeId != nodeId)
            {
                if (i > 0)
                {
                     prevNodeId = m_nodes.value(i - 1).GetId();
                }
                if (i == 0 || prevNodeId != curveNodeId)
                {
                    VPieceNode node2 = curveNode;
                    node2.SetExcluded(true);
                    m_nodes.insert(i, 1, node2);
                }
            }
        }
    }
}

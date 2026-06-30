 //-----------------------------------------------------------------------------
 //  @file   pieces_widget.h
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
 //  @file   vwidgetdetails.h
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

#ifndef PIECES_WIDGET_H
#define PIECES_WIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QSharedPointer>

class VAbstractPattern;
class VContainer;
class VPiece;
class VPieceNode;
class FabricDoc;

namespace Ui
{
    class PiecesWidget;
}

class PiecesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit           PiecesWidget(VContainer *data, VAbstractPattern *doc,  QWidget *parent = nullptr);
    virtual           ~PiecesWidget();

    void               addFabric(QSharedPointer<FabricDoc> fabric);
    QVector<QSharedPointer<FabricDoc>> fabrics() const { return m_fabrics; }

signals:
    void               Highlight(quint32 id);
    void               pieceSelected(quint32 id);
    void               fabricClicked(int index);
    void               addFabricRequested();
    void               pieceFabricChanged(quint32 pieceId, int fabricIndex);

public slots:
    void               togglePiece(quint32 id);
    void               updateList();
    void               selectPiece(quint32 id);
    void               clear();
    void               clearNodeHighlight();

private slots:
    void               itemClicked(QTreeWidgetItem *item, int column);
    void               itemDoubleClicked(QTreeWidgetItem *item, int column);
    void               itemChanged(QTreeWidgetItem *item, int column);
    void               showContextMenu(const QPoint &pos);
    void               onDropCompleted();

protected:
    virtual void       changeEvent(QEvent* event) override;

private:
    Q_DISABLE_COPY(PiecesWidget)
    Ui::PiecesWidget             *ui;
    VAbstractPattern             *m_doc;
    VContainer                   *m_data;
    const QHash<quint32, VPiece> *m_allPieces;
    QVector<QSharedPointer<FabricDoc>> m_fabrics;
    QHash<quint32, int>              m_pieceFabricMap;
    quint32                          m_highlightedNodeId;
    bool                             m_fillTreeInProgress;

    enum ItemRole
    {
        PieceIdRole   = Qt::UserRole,
        FabricIdRole  = Qt::UserRole + 1,
        IsFabricRole  = Qt::UserRole + 2,
        IsNodeRole    = Qt::UserRole + 3,
        NodeIdRole    = Qt::UserRole + 4
    };

    void               fillTree(const QHash<quint32, VPiece> *pieces);
    QTreeWidgetItem   *createFabricNode(int fabricIndex);
    QTreeWidgetItem   *createPieceItem(quint32 id, const VPiece &piece);
    QTreeWidgetItem   *createNodeItem(const VPieceNode &node);
    void               toggleInLayoutPieces(bool inLayout);
    void               toggleLockedPieces(bool lock);
    QString            formatLetterString(const VPiece piece);
    void               includeAllPieces();
    void               invertIncludedPieces();
    void               excludeAllPieces();
    void               lockAllPieces();
    void               invertLockedPieces();
    void               unlockAllPieces();
    void               editPieceColor(quint32 id);
    void               editPieceProperties(quint32 id);
    void               showNodeContextMenu(QTreeWidgetItem *item, const QPoint &globalPos);
    void               highlightCurveNode(quint32 nodeId, QTreeWidgetItem *pieceItem);
    int                fabricIndexForPiece(quint32 id) const;
    QList<QTreeWidgetItem *> allPieceItems() const;
};

#endif // PIECES_WIDGET_H

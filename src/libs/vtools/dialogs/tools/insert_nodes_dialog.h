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
 **  @file   insert_nodes_dialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 3, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
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

#ifndef DIALOGINSERTNODE_H
#define DIALOGINSERTNODE_H

#include "dialogtool.h"
#include "../vpatterndb/vpiecenode.h"

class VPieceNode;

namespace Ui
{
    class InsertNodesDialog;
}

class InsertNodesDialog : public DialogTool
{
    Q_OBJECT

public:
    explicit              InsertNodesDialog(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual              ~InsertNodesDialog();

    virtual void          SetPiecesList(const QVector<quint32> &list) Q_DECL_OVERRIDE;

    quint32               getPieceId() const;

    QVector<VPieceNode>   getNodes() const;

    virtual void          ShowDialog(bool click) Q_DECL_OVERRIDE;

public slots:
    virtual void          SelectedObject(bool selected, quint32 object, quint32 tool) Q_DECL_OVERRIDE;

private slots:
    void                  showContextMenu(const QPoint &pos);

protected:
    virtual void          checkState() Q_DECL_FINAL;
    virtual bool          eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
                          Q_DISABLE_COPY(InsertNodesDialog)
    Ui::InsertNodesDialog *ui;

    QVector<VPieceNode>   m_nodes;
    bool                  m_nodeFlag;
    bool                  m_piecesFlag;
    void                  validatePieces();
    void                  validateNodes();
    quint32               getLastNodeId() const;
    bool                  correctCurveDirection(quint32 objectId);
    void                  insertCurveNodes(VPieceNode curveNode);
};

#endif // DIALOGINSERTNODE_H

/***************************************************************************
 **  @file   insert_nodes_dialog.h
 **  @author Douglas S Caskey
 **  @date   Dec 11, 2022
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
 **  @file   dialoginsertnode.h
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

#ifndef INSERT_NODES_DIALOG_H
#define INSERT_NODES_DIALOG_H

#include "dialogtool.h"
#include "../vpatterndb/vpiecenode.h"

#include <QSoundEffect>

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
    QSoundEffect          *m_beep;
    void                  validatePieces();
    void                  validateNodes();
    quint32               getLastNodeId() const;
    bool                  correctCurveDirection(quint32 objectId);
    void                  insertCurveNodes(VPieceNode curveNode);
};

#endif // INSERT_NODES_DIALOG_H

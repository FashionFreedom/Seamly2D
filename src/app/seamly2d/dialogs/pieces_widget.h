/***************************************************************************
 **  @file   pieces_widget.h
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
 **  @file   vwidgetdetails.h
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

#ifndef PIECES_WIDGET_H
#define PIECES_WIDGET_H

#include <QWidget>
#include <QTableWidgetItem>

class VAbstractPattern;
class VContainer;
class VPiece;

namespace Ui
{
    class PiecesWidget;
}

class PiecesWidget : public QWidget
{
    Q_OBJECT

public:
    explicit           PiecesWidget(VContainer *data, VAbstractPattern *doc, QWidget *parent = nullptr);
    virtual           ~PiecesWidget();

signals:
    void               Highlight(quint32 id);

public slots:
    void               togglePiece(quint32 id);
    void               updateList();
    void               selectPiece(quint32 id);

private slots:
    void               cellClicked(int row, int column);
    void               cellDoubleClicked(int row, int column);
    void               showContextMenu(const QPoint &pos);

private:
    Q_DISABLE_COPY(PiecesWidget)
    Ui::PiecesWidget  *ui;
    VAbstractPattern  *m_doc;
    VContainer        *m_data;

    void               fillTable(const QHash<quint32, VPiece> *details);
    void               toggleInLayoutPieces(bool inLayout);
    void               toggleLockedPieces(bool lock);
};

#endif // PIECES_WIDGET_H

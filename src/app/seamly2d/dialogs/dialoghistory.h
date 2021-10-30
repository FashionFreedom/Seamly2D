/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   dialoghistory.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef DIALOGHISTORY_H
#define DIALOGHISTORY_H

#include "../vtools/dialogs/tools/dialogtool.h"

#include <QDomElement>

class VPattern;

namespace Ui
{
    class DialogHistory;
}

/**
 * @brief The DialogHistory class show dialog history.
 */
class DialogHistory : public DialogTool
{
    Q_OBJECT
public:
    DialogHistory(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    virtual ~DialogHistory() Q_DECL_OVERRIDE;
public slots:
    virtual void      DialogAccepted() Q_DECL_OVERRIDE;
    /** TODO ISSUE 79 : create real function
    * @brief DialogApply apply data and emit signal about applied dialog.
    */
    virtual void      DialogApply() Q_DECL_OVERRIDE {}
    void              cellClicked(int row, int column);
    void              ChangedCursor(quint32 id);
    void              updateHistory();

signals:
    /**
     * @brief ShowHistoryTool signal change color of selected in records tool
     * @param id id of tool
     * @param enable true enable selection, false disable selection
     */
    void              ShowHistoryTool(quint32 id, bool enable);

protected:
    virtual void      closeEvent ( QCloseEvent * event ) Q_DECL_OVERRIDE;
    virtual void      changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    virtual bool      eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DialogHistory)

    /** @brief ui keeps information about user interface */
    Ui::DialogHistory *ui;

    /** @brief doc dom document container */
    VPattern          *doc;

    /** @brief cursorRow save number of row where is cursor */
    qint32            cursorRow;

    /** @brief cursorToolRecordRow save number of row selected record */
    qint32            cursorToolRecordRow;

    void              FillTable();
    QString           Record(const VToolRecord &tool);
    void              InitialTable();
    void              ShowPoint();
    QString           PointName(quint32 pointId);
    quint32           AttrUInt(const QDomElement &domElement, const QString &name);
    void              RetranslateUi();
    int               CursorRow() const;
    void              findText(const QString &text);
};

#endif // DIALOGHISTORY_H

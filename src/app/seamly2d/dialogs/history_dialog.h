//-----------------------------------------------------------------------------
//  @file   historyDialog.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2015 - 2023 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//
//  @file   dialoghistory.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
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
//
//-----------------------------------------------------------------------------

#ifndef HISTORY_DIALOG_H
#define HISTORY_DIALOG_H

#include "../vtools/dialogs/tools/dialogtool.h"

#include <QDomElement>
#include <QTableWidgetItem>

class VPattern;
class VTableSearch;

namespace Ui
{
    class HistoryDialog;
}

struct RowData
{
     quint32 id{NULL_ID};
     QString icon{QString("")};
     QString name{QString("")};
     QString tool{QString("")};
     QString length{QString("")};
     QString angle{QString("")};
};

class HistoryDialog : public DialogTool
{
    Q_OBJECT
public:
                           HistoryDialog(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    virtual               ~HistoryDialog() override;

public slots:
    virtual void            DialogAccepted() override;
    void                    cellClicked(int row, int column);
    void                    cursorChanged(quint32 id);
    void                    updateHistory(bool saved);

signals:
    // @brief showHistoryTool signal change to highlight selected tool in history
    // @param id id of tool
    // @param enable true enable selection, false disable selection
    void                    showHistoryTool(quint32 id, bool enable);

protected:
    virtual void            closeEvent ( QCloseEvent * event ) override;
    virtual void            changeEvent(QEvent* event) override;
    virtual bool            eventFilter(QObject *object, QEvent *event) override;
    virtual void            showEvent( QShowEvent *event ) override;
    virtual void            resizeEvent(QResizeEvent *event) override;

private:
    Q_DISABLE_COPY(HistoryDialog)

    Ui::HistoryDialog      *ui;                    /// @brief ui keeps information about user interface */
    VPattern               *m_doc;                 /// @brief doc dom document container */
    qint32                  m_cursorRow;           /// @brief cursorRow save number of row where is cursor */
    qint32                  m_cursorToolRecordRow; /// @brief cursorToolRecordRow save number of row selected record */
    QSharedPointer<VTableSearch> m_search{};

    void                    fillTable();
    RowData                 record(const VToolRecord &tool);
    QString                 getName(const quint32 &toolId);
    QString                 getDestinationNames(const QDomElement &domElement, const QString &suffix);
    void                    initializeTable();
    void                    showTool();
    quint32                 attrUInt(const QDomElement &domElement, const QString &name);
    void                    retranslateUi();
    int                     cursorRow() const;
    void                    findText(const QString &text);
    void                    copyToClipboard();
    void                    searchPrev();
    void                    searchNext();
    void                    regexToggled(bool checked);
    void                    caseToggled(bool checked);
    void                    wordToggled(bool checked);
    void                    moveToTop();
    void                    moveToCursor();
    void                    moveToBottom();
};

#endif // HISTORY_DIALOG_H

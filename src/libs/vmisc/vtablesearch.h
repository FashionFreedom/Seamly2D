//-----------------------------------------------------------------------------
//  @file   vtablesearch.h
//  @author Douglas S Caskey
//  @date   12 Jun, 2025
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2025 Seamly2D project
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
//  @file   vtablesearch.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 9, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentine project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Seamly2D project
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

#ifndef VTABLESEARCH_H
#define VTABLESEARCH_H

#include <QObject>
#include <QList>
#include <QString>
#include <QTableWidget>
#include <QtGlobal>

class VTableSearch: public QObject
{
    Q_OBJECT
public:
    explicit VTableSearch(QTableWidget *table, QObject *parent = nullptr);

    void find(const QString &term);
    void findPrevious();
    void findNext();
    void removeRow(int row);
    void addRow(int row);
    void refreshList(const QString &term);

    void setMatchRegEx(bool value);
    void setMatchCase(bool value);
    void setMatchWord(bool value);

signals:
    void hasResult(bool state);

private:
    Q_DISABLE_COPY(VTableSearch)

    QTableWidget              *m_table;
    int                        m_searchIndex;
    QList<QTableWidgetItem *>  m_searchList;
    bool                       m_matchRegEx;
    bool                       m_matchCase;
    bool                       m_matchWord;

    void                       clear();
    void                       showNext(int newIndex);
    QList<QTableWidgetItem *>  findItems(const QString &text);
};

#endif // VTABLESEARCH_H

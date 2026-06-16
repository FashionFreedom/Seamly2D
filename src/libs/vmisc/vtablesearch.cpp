//-----------------------------------------------------------------------------
//  @file   vtablesearch.cpp
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
//  @file   vtablesearch.cpp
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

#include "vtablesearch.h"

#include <QColor>
#include <QPalette>
#include <QRegularExpression>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <Qt>

#include "../vmisc/def.h"

//---------------------------------------------------------------------------------------------------------------------
VTableSearch::VTableSearch(QTableWidget *table, QObject *parent)
    : QObject(parent)
    , m_table(table)
    , m_searchIndex(-1)
    , m_searchList()
    , m_matchRegEx(false)
    , m_matchCase(false)
    , m_matchWord(false)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::find(const QString &text)
{
    SCASSERT(m_table != nullptr)

    clear();

    if (!text.isEmpty())
    {
        m_searchList = findItems(text);

        if (!m_searchList.isEmpty())
        {
            for (QTableWidgetItem *item : m_searchList)
            {
                item->setBackground(QColor("#FAFAC8"));
            }

            m_searchIndex = 0;
            QTableWidgetItem *item = m_searchList.at(m_searchIndex);
            item->setBackground(QColor("#FAFA5A"));
            m_table->scrollToItem(item);

            emit hasResult(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::findPrevious()
{
    int newIndex = m_searchIndex - 1;

    if (newIndex < 0)
    {
        newIndex = m_searchList.size() - 1;
    }

    showNext(newIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::findNext()
{
    int newIndex = m_searchIndex + 1;

    if (newIndex >= m_searchList.size())
    {
        newIndex = 0;
    }

    showNext(newIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::removeRow(int row)
{
    if (m_searchIndex < 0 || m_searchIndex >= m_searchList.size())
    {
        return;
    }

    const int indexRow = m_searchList.at(m_searchIndex)->row();

    if (row <= indexRow)
    {
        for (QTableWidgetItem *item : m_searchList)
        {
            if (item->row() == row)
            {
                --m_searchIndex;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::addRow(int row)
{
    if (m_searchIndex < 0 || m_searchIndex >= m_searchList.size())
    {
        return;
    }

    const int indexRow = m_searchList.at(m_searchIndex)->row();

    if (row <= indexRow)
    {
        for (QTableWidgetItem *item : m_searchList)
        {
            if (item->row() == row)
            {
                ++m_searchIndex;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::refreshList(const QString &text)
{
    SCASSERT(m_table != nullptr)

    if (text.isEmpty())
    {
        return;
    }

    m_searchList = findItems(text);

    for (QTableWidgetItem *item : m_searchList)
    {
        item->setBackground(QColor("#FAFAC8"));
    }

    if (!m_searchList.isEmpty())
    {
        if (m_searchIndex < 0)
        {
           m_searchIndex = m_searchList.size() - 1;
        }
        else if (m_searchIndex >= m_searchList.size())
        {
           m_searchIndex = 0;
        }

        QTableWidgetItem *item = m_searchList.at(m_searchIndex);
        item->setBackground(QColor("#FAFA5A"));
        m_table->scrollToItem(item);

        emit hasResult(true);
    }
    else
    {
        emit hasResult(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::setMatchRegEx(bool value)
{
    m_matchRegEx = value;
    m_matchWord = false;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::setMatchCase(bool value)
{
    m_matchCase = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::setMatchWord(bool value)
{
    m_matchWord = value;
    m_matchRegEx = false;
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::clear()
{
    SCASSERT(m_table != nullptr)

    for (int i = 0; i < m_table->rowCount(); ++i)
    {
        for (int j = 0; j < m_table->columnCount(); ++j)
        {
            if (QTableWidgetItem *item = m_table->item(i, j))
            {
                if (item->row() % 2 != 0 && m_table->alternatingRowColors())
                {
                    item->setBackground(QPalette().alternateBase());
                }
                else
                {
                    item->setBackground(QPalette().base());
                }
            }
        }
    }

    m_searchList.clear();
    m_searchIndex = -1;

    emit hasResult(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VTableSearch::showNext(int newIndex)
{
    if (!m_searchList.isEmpty())
    {
        QTableWidgetItem *item = m_searchList.at(m_searchIndex);
        item->setBackground(QColor("#FAFAC8"));

        item = m_searchList.at(newIndex);
        item->setBackground(QColor("#FAFA5A"));
        m_table->scrollToItem(item);
        m_searchIndex = newIndex;
    }
    else
    {
        clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QList<QTableWidgetItem *> VTableSearch::findItems(const QString &text)
{
    if (m_matchWord && !m_matchCase)
    {
        m_table->findItems(text,  Qt::MatchFixedString);
    }
    else if (m_matchWord && m_matchCase)
    {
        m_table->findItems(text,  Qt::MatchFixedString | Qt::MatchCaseSensitive);
    }
    else if (m_matchRegEx && !m_matchCase)
    {
        QRegularExpression regex(text, QRegularExpression::CaseInsensitiveOption);
        if (regex.isValid())
        {
            return m_table->findItems(regex.pattern(), Qt::MatchRegularExpression);
        }
    }
    else if (m_matchRegEx && m_matchCase)
    {
        QRegularExpression regex(text);
        if (regex.isValid())
        {
            return m_table->findItems(regex.pattern(), Qt::MatchRegularExpression);
        }
    }
    else if (m_matchCase)
    {
        return m_table->findItems(text, Qt::MatchContains | Qt::MatchCaseSensitive);
    }

    return m_table->findItems(text, Qt::MatchContains);
}

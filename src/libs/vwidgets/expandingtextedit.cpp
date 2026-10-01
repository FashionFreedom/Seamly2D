//---------------------------------------------------------------------------------------------------------------------
//   @file   expandingtextedit.cpp
//   @author DS Caskey
//   @date   Feb 18, 2023
//
//   @brief
//   @copyright
//   This source code is part of the Seamly2D project, a pattern making
//   program, whose allow create and modeling patterns of clothing.
//   Copyright (C) 2017-2023 Seamly2D project
//   <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
//   Seamly2D is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   (at your option) any later version.
//
//   Seamly2D is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include "expandingtextedit.h"

#include <QTextCursor>
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
ExpandingTextEdit::ExpandingTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{}

//---------------------------------------------------------------------------------------------------------------------
ExpandingTextEdit::ExpandingTextEdit(const QString &text, QWidget *parent)
    : QPlainTextEdit(text, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
void ExpandingTextEdit::focusInEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusInEvent(e);
    if (height() < 62)
    {
        setFixedHeight(75);
    }

    // Defer the cursor movement so it doesn't collide with Qt's
    // internal mouse-tracking and context menu initialization.
    QTimer::singleShot(0, this, [this]()
    {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
        setTextCursor(cursor);
    });
}

//---------------------------------------------------------------------------------------------------------------------
void ExpandingTextEdit::focusOutEvent(QFocusEvent *e)
{
    QPlainTextEdit::focusOutEvent(e);
    if (height() > 62)
    {
        setFixedHeight(28);
    }
}

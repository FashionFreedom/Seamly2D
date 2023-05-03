/******************************************************************************
 *   @file   expandingtextedit.h
 **  @author DS Caskey
 **  @date   Feb 18, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

#ifndef EXPANDINGTEXTEDIT_H
#define EXPANDINGTEXTEDIT_H

#include <QPlainTextEdit>
#include <QWidget>

class ExpandingTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit     ExpandingTextEdit(QWidget *parent = nullptr);
                 ExpandingTextEdit(const QString &contents, QWidget *parent = nullptr);

protected:
    virtual void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    virtual void focusOutEvent(QFocusEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(ExpandingTextEdit)
};

#endif // EXPANDINGTEXTEDIT_H

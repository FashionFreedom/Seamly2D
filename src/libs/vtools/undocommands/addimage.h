/******************************************************************************
 **  @file   addimage.h
 **  @author Evans PERRET
 **  @date   July 28, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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
 *****************************************************************************/

#ifndef ADDIMAGE_H
#define ADDIMAGE_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "vundocommand.h"

class AddImage : public VUndoCommand
{
    Q_OBJECT
public:
    AddImage(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    virtual  ~AddImage();
    virtual   void  undo() Q_DECL_OVERRIDE; // cppcheck-suppress unusedFunction
    virtual   void  redo() Q_DECL_OVERRIDE; // cppcheck-suppress unusedFunction

private:
    Q_DISABLE_COPY(AddImage)
};

#endif // ADDIMAGE_H

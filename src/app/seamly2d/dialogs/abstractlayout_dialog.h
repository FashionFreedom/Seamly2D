/******************************************************************************
 *   @file   abstractlayout_dialog.cpp
 **  @author Douglas S Caskey
 **  @date   19 Oct, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

#ifndef ABSTRACT_LAYOUT_DIALOG_H
#define ABSTRACT_LAYOUT_DIALOG_H

#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"

#include <QDialog>

class AbstractLayoutDialog : public QDialog
{
public:
    explicit                 AbstractLayoutDialog(QWidget *parent = nullptr);

protected:
    QSizeF                   getTemplateSize(const PaperSizeFormat &tmpl, const Unit &unit) const;
    QSizeF                   RoundTemplateSize(qreal width, qreal height, Unit unit) const;
};

#endif // ABSTRACT_LAYOUT_DIALOG_H

//-----------------------------------------------------------------------------
//  @file   piece_info_dialog.h
//  @author Coyak2
//  @date   15 Nov, 2026
//
//  @brief  Pattern Piece Info dialog: a table of the pattern pieces with their seamline and cutting line lengths,
//          which can be copied to the clipboard or exported as CSV.
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
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

#ifndef PIECE_INFO_DIALOG_H
#define PIECE_INFO_DIALOG_H

#include <QDialog>
#include <QStringList>

class VContainer;

namespace Ui
{
    class PieceInfoDialog;
}

class PieceInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit             PieceInfoDialog(const VContainer* data, QWidget* parent = nullptr);
    virtual             ~PieceInfoDialog();

private:
    Q_DISABLE_COPY(PieceInfoDialog)
    Ui::PieceInfoDialog* ui;
    const VContainer*    m_data;

    void                 fillTable();
    QStringList          headerTexts() const;
    QStringList          rowTexts(int row) const;
    void                 copyToClipboard();
    void                 exportToCsv();
};

#endif // PIECE_INFO_DIALOG_H

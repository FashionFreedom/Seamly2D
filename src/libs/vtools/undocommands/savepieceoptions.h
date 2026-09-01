//---------------------------------------------------------------------------------------------------------------------
//  @file   savepieceoptions.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2023 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   savepieceoptions.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   9 11, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef SAVEPIECEOPTIONS_H
#define SAVEPIECEOPTIONS_H

#include <QHash>
#include <QString>
#include <QtGlobal>

#include "vpiece.h"
#include "vundocommand.h"

class SavePieceOptions : public VUndoCommand
{
public:
                  SavePieceOptions(const VPiece &oldPiece, const VPiece &newPiece, VAbstractPattern *doc,
                                   VContainer *data, quint32 id, QUndoCommand *parent = nullptr);

    virtual      ~SavePieceOptions();

    virtual void  undo() override;
    virtual void  redo() override;
    virtual bool  mergeWith(const QUndoCommand *command) override;
    virtual int   id() const override;
    quint32       pieceId() const;
    VPiece        getNewPiece() const;

private:
    Q_DISABLE_COPY(SavePieceOptions)

    const VPiece  m_oldPiece;
    VPiece        m_newPiece;
    // Frozen at construction time, when m_oldPiece/m_newPiece's name-form formula text was
    // captured - NOT recomputed in undo()/redo(), which could otherwise run long after an
    // intervening rename desyncs the live container's names from that already-captured text.
    const QHash<QString, QString> m_nameToIdToken;
};

//---------------------------------------------------------------------------------------------------------------------
inline quint32 SavePieceOptions::pieceId() const
{
    return nodeId;
}

//---------------------------------------------------------------------------------------------------------------------
inline VPiece SavePieceOptions::getNewPiece() const
{
    return m_newPiece;
}

#endif // SAVEPIECEOPTIONS_H

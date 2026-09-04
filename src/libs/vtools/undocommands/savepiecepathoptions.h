//---------------------------------------------------------------------------------------------------------------------
//  @file   savepiecepathoptions.h
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
//  @file   savepiecepathoptions.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   26 11, 2016
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

#ifndef SAVEPIECEPATHOPTIONS_H
#define SAVEPIECEPATHOPTIONS_H

#include <QHash>
#include <QString>
#include <QtGlobal>

#include "../vpatterndb/vpiecepath.h"
#include "vundocommand.h"

class SavePiecePathOptions : public VUndoCommand
{
public:
    SavePiecePathOptions(quint32 pieceId, const VPiecePath &oldPath, const VPiecePath &newPath, VAbstractPattern *doc,
                         VContainer *data, quint32 id, QUndoCommand *parent = nullptr);
    virtual ~SavePiecePathOptions();

    virtual void undo() override;
    virtual void redo() override;
    virtual bool mergeWith(const QUndoCommand *command) override;
    virtual int  id() const override;
    quint32      pathId() const;
    VPiecePath   newPath() const;

private:
    Q_DISABLE_COPY(SavePiecePathOptions)

    const VPiecePath m_oldPath;
    VPiecePath       m_newPath;
    VContainer      *m_data;
    quint32          m_pieceId;
    // Frozen at construction time, when m_oldPath/m_newPath's name-form formula text was
    // captured - NOT recomputed in undo()/redo(), which could otherwise run long after an
    // intervening rename desyncs the live container's names from that already-captured text.
    const QHash<QString, QString> m_nameToIdToken;
};

#endif // SAVEPIECEPATHOPTIONS_H

/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************/

#ifndef TST_SAVEPIECEUNDOMERGEIDTOKEN_H
#define TST_SAVEPIECEUNDOMERGEIDTOKEN_H

#include <QObject>

// Regression coverage for issue #1678: SavePieceOptions::mergeWith() and
// SavePiecePathOptions::mergeWith() (undo/redo continuation of interactive piece/piece-path edits,
// e.g. dragging a value in the property editor) adopted the merged-in command's new piece/path -
// including its name-form formula text - but kept their own, earlier m_name_to_id_token snapshot
// taken at construction time. A merge spanning a rename could then translate the second command's
// formula against the first command's stale map, silently leaving a composite-variable reference as
// raw display-name text instead of a rename-proof id token. The exact same bug class was already
// found and fixed for MoveSplinePath (commit 89b19f2e89) and MoveSpline (commit 10c5c56f28); this
// proves both SavePieceOptions and SavePiecePathOptions had it too and pins the fix.
class TST_SavePieceUndoMergeIdToken : public QObject
{
    Q_OBJECT
public:
    explicit TST_SavePieceUndoMergeIdToken(QObject *parent = nullptr);

private slots:
    void TestSavePieceOptionsMergeAdoptsMergedCommandsIdTokenMap();
    void TestSavePieceOptionsStaleMapSurvivesASecondRenameWithWrongOrFailingValue();
    void TestSavePiecePathOptionsMergeAdoptsMergedCommandsIdTokenMap();
    void TestSavePiecePathOptionsStaleMapSurvivesASecondRenameWithWrongOrFailingValue();

private:
    Q_DISABLE_COPY(TST_SavePieceUndoMergeIdToken)
};

#endif // TST_SAVEPIECEUNDOMERGEIDTOKEN_H

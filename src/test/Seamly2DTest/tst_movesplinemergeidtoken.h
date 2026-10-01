/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
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

#ifndef TST_MOVESPLINEMERGEIDTOKEN_H
#define TST_MOVESPLINEMERGEIDTOKEN_H

#include <QObject>

// Regression coverage for issue #1678: MoveSpline::mergeWith() (undo/redo continuation of an
// interactive spline drag) adopted the merged-in command's new spline - including its name-form
// formula text - but kept its own, earlier m_name_to_id_token snapshot taken at construction time.
// A merge spanning a rename could then translate the second command's formula against the first
// command's stale map, silently leaving a composite-variable reference as raw display-name text
// instead of a rename-proof id token. The exact same bug class, for the sibling MoveSplinePath
// command, was already found and fixed (commit 89b19f2e92); this proves MoveSpline had it too and
// pins the fix.
class TST_MoveSplineMergeIdToken : public QObject
{
    Q_OBJECT
public:
    explicit TST_MoveSplineMergeIdToken(QObject *parent = nullptr);

private slots:
    void TestMergeAdoptsMergedCommandsIdTokenMap();
    void TestStaleMapSurvivesASecondRenameWithWrongOrFailingValue();

private:
    Q_DISABLE_COPY(TST_MoveSplineMergeIdToken)
};

#endif // TST_MOVESPLINEMERGEIDTOKEN_H

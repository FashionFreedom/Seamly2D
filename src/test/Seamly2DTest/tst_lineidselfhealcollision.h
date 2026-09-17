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

#ifndef TST_LINEIDSELFHEALCOLLISION_H
#define TST_LINEIDSELFHEALCOLLISION_H

#include <QObject>

// Regression coverage for the maintainer bug report against issue-#1678 / PR #1681: an angle
// formula silently changed identity on Open -> Save -> Reload of an old-format (pre-0.7.5)
// pattern, because a self-healed line id could collide with the real id of a later, not-yet-
// parsed point/tool. TestSelfHealedLineIdCanCollideWithNotYetParsedPointId and
// TestCollidingLineAngleIdTokensSilentlyMergeInIdTokenToNameMap reproduce the bug mechanically
// against the raw VContainer/token-map APIs (without the fix applied);
// TestSeedingIdCounterAgainstMaxFileIdPreventsTheCollision proves the fix - seeding
// VContainer's id counter against the whole document's maximum id, exactly as
// VPattern::PrepareForParse() now does - eliminates the collision for the identical scenario.
class TST_LineIdSelfHealCollision : public QObject
{
    Q_OBJECT
public:
    explicit TST_LineIdSelfHealCollision(QObject *parent = nullptr);

private slots:
    void TestSelfHealedLineIdCanCollideWithNotYetParsedPointId();
    void TestCollidingLineAngleIdTokensSilentlyMergeInIdTokenToNameMap();
    void TestSeedingIdCounterAgainstMaxFileIdPreventsTheCollision();

private:
    Q_DISABLE_COPY(TST_LineIdSelfHealCollision)
};

#endif // TST_LINEIDSELFHEALCOLLISION_H

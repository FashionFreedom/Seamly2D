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

#ifndef TST_DUPLICATEPOINTNAMELINECOLLISION_H
#define TST_DUPLICATEPOINTNAMELINECOLLISION_H

#include <QObject>

// Regression coverage for a second, distinct maintainer bug against issue-#1678 / PR #1681: the
// "Defekte Formel" infinite loop reproduced by opening the bundled sample
// src/app/share/samples/patterns/jacket1_52-176.sm2d, saving it, closing it, and reopening it.
//
// Unlike the counter-collision bug fixed for TST_LineIdSelfHealCollision (two different ids
// landing on the same numeric value), this bug is a NAME collision: point display names have
// never been enforced unique pattern-wide, so two entirely unrelated lines can generate the
// identical composite variable display name (e.g. "Line_A_A2") when their endpoints happen to
// share names with another pair of points elsewhere in the pattern - exactly what
// jacket1_52-176.sm2d's draft block does. VContainer::AddVariable()'s "name already exists ->
// update the existing object in place" semantics, meant to support re-parsing the SAME line
// after an edit, instead silently spliced the second line's identity onto the first line's name,
// orphaning the first line's own persisted id. Any formula already saved against that id became
// an unresolvable dangling id-token on the next load.
//
// TestDuplicatePointNamesDoNotCollideAcrossUnrelatedLines proves both lines remain individually
// resolvable after the fix (VContainer::UniqueLineVariableName, vcontainer.cpp).
// TestOldFormatFormulaSurvivesDuplicatePointNameRoundTrip reproduces the full failure end to end,
// using the exact ids/names from jacket1_52-176.sm2d, and proves the fix prevents the dangling
// token from ever being written in the first place.
class TST_DuplicatePointNameLineCollision : public QObject
{
    Q_OBJECT
public:
    explicit TST_DuplicatePointNameLineCollision(QObject *parent = nullptr);

private slots:
    void TestDuplicatePointNamesDoNotCollideAcrossUnrelatedLines();
    void TestOldFormatFormulaSurvivesDuplicatePointNameRoundTrip();

private:
    Q_DISABLE_COPY(TST_DuplicatePointNameLineCollision)
};

#endif // TST_DUPLICATEPOINTNAMELINECOLLISION_H

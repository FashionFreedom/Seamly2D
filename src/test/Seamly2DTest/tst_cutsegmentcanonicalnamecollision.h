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

#ifndef TST_CUTSEGMENTCANONICALNAMECOLLISION_H
#define TST_CUTSEGMENTCANONICALNAMECOLLISION_H

#include <QObject>

// Investigates whether the "canonical vs. squatter" eviction fix in
// VContainer::UniqueCompositeVariableName (commit 816784f838, issue-#1678 / PR #1681) needs its own
// dedicated proof for VarType::ArcRadius - the one composite type its own test coverage did not
// touch. It turned out arcs have a stronger, independent guarantee: VAbstractArc::setId()
// unconditionally regenerates an arc's name (from its own center point and its own id) every time a
// real container id is assigned, so two canonical arcs can never end up sharing an identical
// composite name to begin with - unlike curves/splines, which have no such id-based fallback and are
// exactly why the real bug this fix addresses only ever manifested for curves (see project history,
// keiko_skirt.sm2d / zigzag.sm2d), never for arcs. This test locks that guarantee in directly rather
// than forcing an artificial arc collision that cannot occur through the class's real lifecycle.
class TST_CutSegmentCanonicalNameCollision : public QObject
{
    Q_OBJECT
public:
    explicit TST_CutSegmentCanonicalNameCollision(QObject *parent = nullptr);

private slots:
    void TestCanonicalArcKeepsPlainNameOverCutSegmentSquatter();

private:
    Q_DISABLE_COPY(TST_CutSegmentCanonicalNameCollision)
};

#endif // TST_CUTSEGMENTCANONICALNAMECOLLISION_H

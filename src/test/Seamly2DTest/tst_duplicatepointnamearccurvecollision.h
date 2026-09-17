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

#ifndef TST_DUPLICATEPOINTNAMEARCCURVECOLLISION_H
#define TST_DUPLICATEPOINTNAMEARCCURVECOLLISION_H

#include <QObject>

// Regression coverage extending TST_DuplicatePointNameLineCollision (issue-#1678 / PR #1681) to the
// arc and curve composite variables registered by VContainer::AddArc()/AddCurve()/AddSpline()/
// AddCurveWithSegments(). Point display names have never been enforced unique pattern-wide, and an
// arc's/curve's own auto-generated display name is glued together from its constituent point names
// exactly the way a line's is (VArc::CreateName() uses only its center point's name;
// VAbstractCubicBezier::CreateName() uses only its P1/P4 endpoint names) - so two structurally
// different arcs, or two structurally different curves, in unrelated draft blocks can legally
// generate the identical composite variable display name ("RadiusArc_A", "Spl_A_A2", ...).
//
// Before the fix, VContainer::AddVariable()'s "name already exists -> update the existing object in
// place" semantics would let the second arc's/curve's registration silently splice its identity onto
// the first one's name, discarding the first one's own persisted id from the variable table -
// producing the exact same dangling id-token / "Defekte Formel" symptom already fixed for lines.
//
// TestDuplicateCenterPointNamesDoNotCollideAcrossUnrelatedArcs and
// TestDuplicateEndpointNamesDoNotCollideAcrossUnrelatedCurves each prove that both objects remain
// individually resolvable after the fix (VContainer::UniqueCompositeVariableName, vcontainer.cpp).
class TST_DuplicatePointNameArcCurveCollision : public QObject
{
    Q_OBJECT
public:
    explicit TST_DuplicatePointNameArcCurveCollision(QObject *parent = nullptr);

private slots:
    void TestDuplicateCenterPointNamesDoNotCollideAcrossUnrelatedArcs();
    void TestDuplicateEndpointNamesDoNotCollideAcrossUnrelatedCurves();

private:
    Q_DISABLE_COPY(TST_DuplicatePointNameArcCurveCollision)
};

#endif // TST_DUPLICATEPOINTNAMEARCCURVECOLLISION_H

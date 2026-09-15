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

#ifndef TST_EMPTYPATHSPLINENAMECOLLISION_H
#define TST_EMPTYPATHSPLINENAMECOLLISION_H

#include <QObject>

// Regression coverage for issue #1678: a pathInteractive spline saved to disk with zero <pathPoint>
// children (src/test/CollectionTest/share/zigzag/zigzag.sm2d, <spline type="pathInteractive"
// id="1046" color="blue"/>) has no point names to build a display name from -
// VAbstractCubicBezierPath::CreateName() produced an empty string, and none of VSplinePath's
// constructors even call CreateName() at all when constructed from an empty point vector - so the
// object's name was never set. VContainer::AddCurve() then registered its VCurveLength/VCurveAngle
// composite variables under that empty name, and a second such empty path collided with the first
// one on that same empty name ("token 'id1046' already maps to '', ignoring conflicting name '_2'").
//
// The fix gives VAbstractCubicBezierPath::CreateName() a fallback (its own persisted id) for the
// zero-point case, and has VContainer::AddCurve() force a curve with an empty name to regenerate it
// once the curve has been given its final, real container id.
class TST_EmptyPathSplineNameCollision : public QObject
{
    Q_OBJECT
public:
    explicit TST_EmptyPathSplineNameCollision(QObject *parent = nullptr);

private slots:
    void TestEmptyPathSplineGetsNonEmptyName();
    void TestTwoEmptyPathSplinesDoNotCollide();

private:
    Q_DISABLE_COPY(TST_EmptyPathSplineNameCollision)
};

#endif // TST_EMPTYPATHSPLINENAMECOLLISION_H

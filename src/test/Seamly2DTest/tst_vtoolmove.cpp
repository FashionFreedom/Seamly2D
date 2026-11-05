//---------------------------------------------------------------------------------------------------------------------
//  @file   tst_vtoolmove.cpp
//  @author Andrea Gorletta
//  @date   8 Aug, 2026
//
//  @copyright
//  Copyright (C)  2026 Seamly, LLC
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

#include "tst_vtoolmove.h"

#include "../vtools/tools/drawTools/operation/vtoolmove.h"
#include "../vpatterndb/vcontainer.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"

#include <QtTest>

namespace
{
// Values taken from the pattern file attached to issue #853 by the reporter.
const QPointF rotationPointA5 = QPointF(3.567710, 23.464600);
const qreal   moveLength      = 5.0;
const qreal   moveAngle       = 0.0;
const qreal   rotationAngle   = -20.0;

const quint32 originPointId   = 11;
const quint32 sourcePointId   = 20;
}

//---------------------------------------------------------------------------------------------------------------------
TST_VToolMove::TST_VToolMove(QObject *parent)
    : QObject(parent)
{
}



//---------------------------------------------------------------------------------------------------------------------
void TST_VToolMove::explicitOriginIncludedInTheMoveFollowsTheMove()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    data->UpdateGObject(originPointId, new VPointF(rotationPointA5.x(), rotationPointA5.y(), "A3", 0, 0));

    const QPointF origin = VToolMove::findRotationOrigin(QVector<quint32>{originPointId}, data.data(), moveLength,
                                                         moveAngle, originPointId);
    const QPointF expected = VPointF::MovePF(rotationPointA5, moveLength, moveAngle);

    QVERIFY2(VFuzzyComparePoints(origin, expected),
             qUtf8Printable(QStringLiteral("A selected rotation point that is moved with the source objects must "
                                          "follow the move. Expected (%1, %2), got (%3, %4).")
                            .arg(expected.x()).arg(expected.y()).arg(origin.x()).arg(origin.y())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VToolMove::implicitOriginStillFollowsTheMove()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    // Three points, deliberately asymmetric: their bounding-box centre (10, 10) differs from both their
    // centroid (10, 6.667) and any pairwise midpoint, so this test actually pins "bounding-box centre"
    // rather than passing under any of those other, superficially similar rules too.
    data->UpdateGObject(sourcePointId,     new VPointF(0,  0, "A1", 0, 0));
    data->UpdateGObject(sourcePointId + 1, new VPointF(10, 20, "A2", 0, 0));
    data->UpdateGObject(sourcePointId + 2, new VPointF(20, 0, "A3", 0, 0));

    const QVector<quint32> objectIds{sourcePointId, sourcePointId + 1, sourcePointId + 2};
    const QPointF origin = VToolMove::findRotationOrigin(objectIds, data.data(), moveLength, moveAngle, NULL_ID);

    // Without an explicit rotation point the origin is the bounding box centre translated by the move
    // vector. That behaviour is correct and must not change.
    const QPointF expected = VPointF::MovePF(QPointF(10, 10), moveLength, moveAngle);

    QVERIFY2(VFuzzyComparePoints(origin, expected),
             qUtf8Printable(QStringLiteral("Without an explicit rotation point the origin must keep following the "
                                          "move. Expected (%1, %2), got (%3, %4).")
                            .arg(expected.x()).arg(expected.y()).arg(origin.x()).arg(origin.y())));
}

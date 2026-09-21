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
void TST_VToolMove::explicitOriginIsNotTranslatedByTheMove()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    data->UpdateGObject(originPointId, new VPointF(rotationPointA5.x(), rotationPointA5.y(), "A5", 0, 0));
    data->UpdateGObject(sourcePointId, new VPointF(-6.432290, 8.464600, "A1", 0, 0));

    const QPointF origin = VToolMove::findRotationOrigin(QVector<quint32>{sourcePointId}, data.data(), moveLength,
                                                         moveAngle, originPointId);

    QVERIFY2(VFuzzyComparePoints(origin, rotationPointA5),
             qUtf8Printable(QStringLiteral("An explicitly selected rotation point must not be moved by the move "
                                          "vector. Expected (%1, %2), got (%3, %4).")
                            .arg(rotationPointA5.x()).arg(rotationPointA5.y()).arg(origin.x()).arg(origin.y())));
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
void TST_VToolMove::moveWithRotationMatchesRotationAfterMove()
{
    const Unit unit = Unit::Cm;
    QScopedPointer<VContainer> data(new VContainer(nullptr, &unit));

    data->UpdateGObject(originPointId, new VPointF(rotationPointA5.x(), rotationPointA5.y(), "A5", 0, 0));

    QVector<VPointF> sourcePoints;
    sourcePoints.append(VPointF(-6.432290, 8.464600, "A1", 0, 0));
    sourcePoints.append(VPointF(3.567710, 8.464600, "A2", 0, 0));
    sourcePoints.append(VPointF(3.567710, 18.464600, "A3", 0, 0));
    sourcePoints.append(VPointF(13.567710, 18.464600, "A4", 0, 0));

    QVector<quint32> objectIds;
    for (int i = 0; i < sourcePoints.size(); ++i)
    {
        const quint32 id = sourcePointId + static_cast<quint32>(i);
        data->UpdateGObject(id, new VPointF(sourcePoints.at(i)));
        objectIds.append(id);
    }

    const QPointF origin = VToolMove::findRotationOrigin(objectIds, data.data(), moveLength, moveAngle,
                                                         originPointId);

    for (auto point : sourcePoints)
    {
        // What the Move tool produces.
        const QPointF moveAndRotate =
                static_cast<QPointF>(point.Move(moveLength, moveAngle).Rotate(origin, rotationAngle));

        // What the Rotation tool applied after a plain move produces - the equivalence the reporter
        // demonstrated as correct.
        const QPointF moveThenRotate =
                static_cast<QPointF>(point.Move(moveLength, moveAngle).Rotate(rotationPointA5, rotationAngle));

        QVERIFY2(VFuzzyComparePoints(moveAndRotate, moveThenRotate),
                 qUtf8Printable(QStringLiteral("Moving with a rotation must equal moving and then rotating about "
                                               "the same point. Point %1: got (%2, %3), expected (%4, %5).")
                                .arg(point.name()).arg(moveAndRotate.x()).arg(moveAndRotate.y())
                                .arg(moveThenRotate.x()).arg(moveThenRotate.y())));
    }
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

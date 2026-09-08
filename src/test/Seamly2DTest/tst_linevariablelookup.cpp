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

#include "tst_linevariablelookup.h"

#include <QtTest>

#include "../../libs/vpatterndb/variables/vlinevariablelookup.h"
#include "../../libs/vpatterndb/variables/vlinelength.h"
#include "../../libs/vpatterndb/variables/vlineangle.h"
#include "../../libs/vpatterndb/vcontainer.h"
#include "../../libs/vpatterndb/vtranslatevars.h"
#include "../../libs/vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
TST_LineVariableLookup::TST_LineVariableLookup(QObject *parent)
    : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Mirrors what VContainer::AddLine() does when a draw tool registers its implicit line (see
 * issue #1678): two points plus a persisted line_id, then a lookup by that same line_id must find the
 * VLengthLine/VLineAngle AddLine() created for it - the same lookup VDrawTool::lineLengthName()/
 * lineAngleName() rely on to build "Copy Length"/"Copy Angle" clipboard text.
 */
void TST_LineVariableLookup::TestFindLineLengthReturnsMatchingLine()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line_id = 99;

    data->AddLine(p1_id, p2_id, line_id);

    const QSharedPointer<VLengthLine> found = findLineLength(*data, line_id);
    QVERIFY(!found.isNull());
    QCOMPARE(found->getLineId(), line_id);
    QCOMPARE(found->GetName(), QStringLiteral("Line_A1_A2"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineAngleReturnsMatchingLine()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    const quint32 line_id = 99;

    data->AddLine(p1_id, p2_id, line_id);

    const QSharedPointer<VLineAngle> found = findLineAngle(*data, line_id);
    QVERIFY(!found.isNull());
    QCOMPARE(found->getLineId(), line_id);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineLengthReturnsNullForUnknownId()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 99);

    QVERIFY(findLineLength(*data, 12345).isNull());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_LineVariableLookup::TestFindLineAngleReturnsNullForUnknownId()
{
    const Unit unit = Unit::Cm;
    const VTranslateVars tr_vars;
    QScopedPointer<VContainer> data(new VContainer(&tr_vars, &unit));

    const quint32 p1_id = data->AddGObject(new VPointF(0, 0, QStringLiteral("A1"), 5, 5));
    const quint32 p2_id = data->AddGObject(new VPointF(10, 0, QStringLiteral("A2"), 5, 5));
    data->AddLine(p1_id, p2_id, 99);

    QVERIFY(findLineAngle(*data, 12345).isNull());
}

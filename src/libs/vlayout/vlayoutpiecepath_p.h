/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
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
 **************************************************************************

 ************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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
 *************************************************************************/

#ifndef VLAYOUTPIECEPATH_P_H
#define VLAYOUTPIECEPATH_P_H

#include <QSharedData>
#include <QPointF>
#include <QVector>

#include "../vmisc/diagnostic.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VLayoutPiecePathData : public QSharedData
{
public:
    VLayoutPiecePathData()
        : m_points()
        , m_lineColor(qApp->Settings()->getDefaultInternalColor())
        , m_lineType(lineTypeToPenStyle(qApp->Settings()->getDefaultInternalLinetype()))
        , m_lineWeight(QString::number(qApp->Settings()->getDefaultInternalLineweight()))
        , m_cut(false)
    {}

    VLayoutPiecePathData(const QVector<QPointF> points, QString color,
                         Qt::PenStyle lineType, QString lineWeight, bool cut)
        : m_points(points)
        , m_lineColor(color)
        , m_lineType(lineType)
        , m_lineWeight(lineWeight)
        , m_cut(cut)
    {}

    VLayoutPiecePathData(const VLayoutPiecePathData &path)
        : QSharedData(path)
        , m_points(path.m_points)
        , m_lineColor(path.m_lineColor)
        , m_lineType(path.m_lineType)
        , m_lineWeight(path.m_lineWeight)
        , m_cut(path.m_cut)
    {}

    ~VLayoutPiecePathData() Q_DECL_EQ_DEFAULT;

    QVector<QPointF> m_points; /// @brief m_points list of path points.
    QString          m_lineColor;
    Qt::PenStyle     m_lineType; /// @brief m_penStyle path pen style.
    QString          m_lineWeight;
    bool             m_cut;

private:
    VLayoutPiecePathData &operator=(const VLayoutPiecePathData &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VLAYOUTPIECEPATH_P_H

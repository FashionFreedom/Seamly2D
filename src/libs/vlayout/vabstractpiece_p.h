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
 **  @date   9 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#ifndef VABSTRACTPIECE_P_H
#define VABSTRACTPIECE_P_H

#include <QSharedData>
#include <QString>
#include <QCoreApplication>

#include "../vmisc/diagnostic.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VAbstractPieceData : public QSharedData
{
    Q_DECLARE_TR_FUNCTIONS(VAbstractPieceData)
public:
    VAbstractPieceData()
        : m_name(tr("Piece"))
        , m_color("white")
        , m_fill()
        , m_pieceLock(false)
        , m_forbidFlipping(false)
        , m_seamAllowance(false)
        , m_seamAllowanceBuiltIn(false)
        , m_hideMainPath(false)
        , m_width(0)
        , m_mx(0)
        , m_my(0)
    {}

    VAbstractPieceData(const VAbstractPieceData &piece)
        : QSharedData(piece)
        , m_name(piece.m_name)
        , m_color(piece.m_color)
        , m_fill(piece.m_fill)
        , m_pieceLock(piece.m_pieceLock)
        , m_forbidFlipping(piece.m_forbidFlipping)
        , m_seamAllowance(piece.m_seamAllowance)
        , m_seamAllowanceBuiltIn(piece.m_seamAllowanceBuiltIn)
        , m_hideMainPath(piece.m_hideMainPath)
        , m_width(piece.m_width)
        , m_mx(piece.m_mx)
        , m_my(piece.m_my)
    {}

    ~VAbstractPieceData() Q_DECL_EQ_DEFAULT;

    QString m_name;
    QString m_color;
    QString m_fill;
    bool    m_pieceLock;
    /** @brief forbidFlipping forbid piece be mirrored in a layout. */
    bool    m_forbidFlipping;
    bool    m_seamAllowance;
    bool    m_seamAllowanceBuiltIn;
    bool    m_hideMainPath;
    qreal   m_width;
    qreal   m_mx;
    qreal   m_my;

private:
    VAbstractPieceData &operator=(const VAbstractPieceData &) Q_DECL_EQ_DELETE;
};

QT_WARNING_POP

#endif // VABSTRACTPIECE_P_H

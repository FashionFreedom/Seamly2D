/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @date   3 11, 2016
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

#ifndef VPIECE_P_H
#define VPIECE_P_H

#include <QSharedData>
#include <QVector>

#include "../vmisc/diagnostic.h"
#include "../vmisc/def.h"
#include "vpiecenode.h"
#include "vpiecepath.h"
#include "floatItemData/vpiecelabeldata.h"
#include "floatItemData/vpatternlabeldata.h"
#include "floatItemData/vgrainlinedata.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceData : public QSharedData
{
public:
    explicit VPieceData(PiecePathType type)
        : m_path(type)
        , m_inLayout(true)
        , m_isLocked(false)
        , m_united(false)
        , m_customSARecords()
        , m_internalPaths()
        , m_anchors()
        , m_ppData()
        , m_piPatternInfo()
        , m_glGrainline()
        , m_formulaWidth('0')
    {}

    VPieceData(const VPieceData &piece)
        : QSharedData(piece)
        , m_path(piece.m_path)
        , m_inLayout(piece.m_inLayout)
        , m_isLocked(piece.m_isLocked)
        , m_united(piece.m_united)
        , m_customSARecords(piece.m_customSARecords)
        , m_internalPaths(piece.m_internalPaths)
        , m_anchors(piece.m_anchors)
        , m_ppData(piece.m_ppData)
        , m_piPatternInfo(piece.m_piPatternInfo)
        , m_glGrainline(piece.m_glGrainline)
        , m_formulaWidth(piece.m_formulaWidth)
    {}

    ~VPieceData();

    VPiecePath              m_path;          //! @brief nodes list piece nodes.
    bool                    m_inLayout;
    bool                    m_isLocked;
    bool                    m_united;
    QVector<CustomSARecord> m_customSARecords;
    QVector<quint32>        m_internalPaths;
    QVector<quint32>        m_anchors;
    VPieceLabelData         m_ppData;        //! @brief Pattern piece data
    VPatternLabelData       m_piPatternInfo; //! @brief Pattern info coordinates
    VGrainlineData          m_glGrainline;   //! @brief m_glGrainline grainline geometry object
    QString                 m_formulaWidth;

private:
    VPieceData              &operator=(const VPieceData &) Q_DECL_EQ_DELETE;
};

VPieceData::~VPieceData()
{}

QT_WARNING_POP

#endif // VPIECE_P_H

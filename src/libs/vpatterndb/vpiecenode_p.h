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

#ifndef VPIECENODE_P_H
#define VPIECENODE_P_H

#include <QSharedData>
#include <QDataStream>
#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPieceNodeData : public QSharedData
{
public:
    VPieceNodeData()
        : m_id(NULL_ID)
        , m_typeTool(Tool::NodePoint)
        , m_reverse(false)
        , m_excluded(false)
        , m_isNotch(false)
        , m_isMainPathNode(true)
        , m_beforeWidthFormula(currentSeamAllowance)
        , m_afterWidthFormula(currentSeamAllowance)
        , m_angleType(PieceNodeAngle::ByLength)
        , m_notchType(stringToNotchType(qApp->Settings()->getDefaultNotchType()))
        , m_notchSubType(NotchSubType::Straightforward)
        , m_showNotch(true)
        , m_showSecondNotch(true)
        , m_notchLength(qApp->Settings()->getDefaultNotchLength())
        , m_notchWidth(qApp->Settings()->getDefaultNotchWidth())
        , m_notchAngle(.000)
        , m_notchCount(1)
    {}

    VPieceNodeData(quint32 id, Tool typeTool, bool reverse)
        : m_id(id)
        , m_typeTool(typeTool)
        , m_reverse(reverse)
        , m_excluded(false)
        , m_isNotch(false)
        , m_isMainPathNode(true)
        , m_beforeWidthFormula(currentSeamAllowance)
        , m_afterWidthFormula(currentSeamAllowance)
        , m_angleType(PieceNodeAngle::ByLength)
        , m_notchType(stringToNotchType(qApp->Settings()->getDefaultNotchType()))
        , m_notchSubType(NotchSubType::Straightforward)
        , m_showNotch(true)
        , m_showSecondNotch(true)
        , m_notchLength(qApp->Settings()->getDefaultNotchLength())
        , m_notchWidth(qApp->Settings()->getDefaultNotchWidth())
        , m_notchAngle(.000)
        , m_notchCount(1)
    {
        if (m_typeTool == Tool::NodePoint)
        {
            m_reverse = false;
        }
    }

    VPieceNodeData (const VPieceNodeData& node)
        : QSharedData(node)
        , m_id(node.m_id)
        , m_typeTool(node.m_typeTool)
        , m_reverse(node.m_reverse)
        , m_excluded(node.m_excluded)
        , m_isNotch(node.m_isNotch)
        , m_isMainPathNode(node.m_isMainPathNode)
        , m_beforeWidthFormula(node.m_beforeWidthFormula)
        , m_afterWidthFormula(node.m_afterWidthFormula)
        , m_angleType(node.m_angleType)
        , m_notchType(node.m_notchType)
        , m_notchSubType(node.m_notchSubType)
        , m_showNotch(node.m_showNotch)
        , m_showSecondNotch(node.m_showSecondNotch)
        , m_notchLength(node.m_notchLength)
        , m_notchWidth(node.m_notchWidth)
        , m_notchAngle(node.m_notchAngle)
        , m_notchCount(node.m_notchCount)
    {}

    ~VPieceNodeData() Q_DECL_EQ_DEFAULT;

    friend QDataStream& operator<<(QDataStream& out, const VPieceNodeData& p);
    friend QDataStream& operator>>(QDataStream& in, VPieceNodeData& p);

    quint32        m_id;              //! @brief id object id.
    Tool           m_typeTool;        //! @brief typeTool type of tool
    bool           m_reverse;         //! @brief reverse true if need reverse points list for node.
    bool           m_excluded;        //! @brief m_excluded true if item excluded from main path. Excluded item is
                                      //! not visible and also will not has affect on main path. Also include to exist
                                      //! path items automatically setted excluded. */
    bool           m_isNotch;         //! @brief m_isNotch has sense only for points. If true to seam allowance should
                                      //! a notch should be added.
    bool           m_isMainPathNode;  //! @brief m_isMainPathNode need fin know if allowed for this notch to be double.

    QString        m_beforeWidthFormula;
    QString        m_afterWidthFormula;

    PieceNodeAngle m_angleType;

    NotchType      m_notchType;
    NotchSubType   m_notchSubType;
    bool           m_showNotch;
    bool           m_showSecondNotch;
    qreal          m_notchLength;
    qreal          m_notchWidth;
    qreal          m_notchAngle;
    int            m_notchCount;

private:
    VPieceNodeData &operator=(const VPieceNodeData &) Q_DECL_EQ_DELETE;
};

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
QDataStream &operator<<(QDataStream &out, const VPieceNodeData &p)
{
    out << p.m_id
        << static_cast<int>(p.m_typeTool)
        << p.m_reverse
        << p.m_excluded
        << p.m_isNotch
        << p.m_beforeWidthFormula
        << p.m_afterWidthFormula
        << static_cast<int>(p.m_angleType)
        << static_cast<int>(p.m_notchType)
        << static_cast<int>(p.m_notchSubType)
        << p.m_showNotch
        << p.m_showSecondNotch
        << p.m_notchLength
        << p.m_notchWidth
        << p.m_notchAngle
        << p.m_notchCount;
    return out;
}

//---------------------------------------------------------------------------------------------------------------------
QDataStream &operator>>(QDataStream &in, VPieceNodeData &p)
{
    int typeTool     = 0;
    int angleType    = 0;
    int notchType    = 0;
    int notchSubType = 0;

    in >> p.m_id
       >> typeTool
       >> p.m_reverse
       >> p.m_excluded
       >> p.m_isNotch
       >> p.m_beforeWidthFormula
       >> p.m_afterWidthFormula
       >> angleType
       >> notchType
       >> notchSubType
       >> p.m_showNotch
       >> p.m_showSecondNotch
       >> p.m_notchLength
       >> p.m_notchWidth
       >> p.m_notchAngle
       >> p.m_notchCount;

    p.m_typeTool     = static_cast<Tool>(typeTool);
    p.m_angleType    = static_cast<PieceNodeAngle>(angleType);
    p.m_notchType    = static_cast<NotchType>(notchType);
    p.m_notchSubType = static_cast<NotchSubType>(notchSubType);

    return in;
}

QT_WARNING_POP

#endif // VPIECENODE_P_H

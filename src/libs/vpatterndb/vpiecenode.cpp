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

#include "vpiecenode.h"
#include "vpiecenode_p.h"
#include "vcontainer.h"
#include "calculator.h"

#include <QDataStream>
#include <QtNumeric>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
qreal EvalFormula(const VContainer *data, QString formula)
{
    if (formula.isEmpty())
    {
        return -1;
    }
    else
    {
        try
        {
            // Replace line return character with spaces for calc if exist
            formula.replace("\n", " ");
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(data->DataVariables(), formula);

            if (qIsInf(result) || qIsNaN(result))
            {
                return -1;
            }
            return result;
        }
        catch (qmu::QmuParserError &e)
        {
            Q_UNUSED(e)
            return -1;
        }
    }
}
}

//---------------------------------------------------------------------------------------------------------------------

#ifdef Q_COMPILER_RVALUE_REFS
VPieceNode &VPieceNode::operator=(VPieceNode &&node) Q_DECL_NOTHROW
{ Swap(node); return *this; }
#endif

void VPieceNode::Swap(VPieceNode &node) Q_DECL_NOTHROW
{ std::swap(d, node.d); }

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode()
    : d(new VPieceNodeData)
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode(quint32 id, Tool typeTool, bool reverse)
    : d(new VPieceNodeData(id, typeTool, reverse))
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::VPieceNode(const VPieceNode &node)
    : d (node.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode &VPieceNode::operator=(const VPieceNode &node)
{
    if ( &node == this )
    {
        return *this;
    }
    d = node.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VPieceNode::~VPieceNode()
{}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
QDataStream &operator<<(QDataStream &out, const VPieceNode &p)
{
    out << p.d;
    return out;
}

//---------------------------------------------------------------------------------------------------------------------
QDataStream &operator>>(QDataStream &in, VPieceNode &p)
{
    in >> *p.d;
    return in;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 VPieceNode::GetId() const
{
    return d->m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetId(quint32 id)
{
    d->m_id = id;
}

//---------------------------------------------------------------------------------------------------------------------
Tool VPieceNode::GetTypeTool() const
{
    return d->m_typeTool;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetTypeTool(Tool value)
{
    d->m_typeTool = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::GetReverse() const
{
    return d->m_reverse;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetReverse(bool reverse)
{
    if (d->m_typeTool != Tool::NodePoint)
    {
        d->m_reverse = reverse;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::GetSABefore(const VContainer *data) const
{
    if (d->m_beforeWidthFormula == currentSeamAllowance)
    {
        return -1;
    }

    return EvalFormula(data, d->m_beforeWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::GetSABefore(const VContainer *data, Unit unit) const
{
    if (d->m_beforeWidthFormula == currentSeamAllowance)
    {
        return -1;
    }

    qreal value = EvalFormula(data, d->m_beforeWidthFormula);
    if (value >= 0)
    {
        value = ToPixel(value, unit);
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPieceNode::GetFormulaSABefore() const
{
    return d->m_beforeWidthFormula;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaSABefore(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_beforeWidthFormula = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::GetSAAfter(const VContainer *data) const
{
    if (d->m_afterWidthFormula == currentSeamAllowance)
    {
        return -1;
    }

    return EvalFormula(data, d->m_afterWidthFormula);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::GetSAAfter(const VContainer *data, Unit unit) const
{
    if (d->m_afterWidthFormula == currentSeamAllowance)
    {
        return -1;
    }

    qreal value = EvalFormula(data, d->m_afterWidthFormula);
    if (value >= 0)
    {
        value = ToPixel(value, unit);
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPieceNode::GetFormulaSAAfter() const
{
    return d->m_afterWidthFormula;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetFormulaSAAfter(const QString &formula)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_afterWidthFormula = formula;
    }
}

//---------------------------------------------------------------------------------------------------------------------
PieceNodeAngle VPieceNode::GetAngleType() const
{
    return d->m_angleType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetAngleType(PieceNodeAngle type)
{
    if (d->m_typeTool == Tool::NodePoint)
    {
        d->m_angleType = type;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::isNotch() const
{
    return d->m_isNotch;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotch(bool notch)
{
    if (GetTypeTool() == Tool::NodePoint)
    {
        d->m_isNotch = notch;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::IsMainPathNode() const
{
    return d->m_isMainPathNode;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetMainPathNode(bool value)
{
    d->m_isMainPathNode = value;
}

//---------------------------------------------------------------------------------------------------------------------
NotchType VPieceNode::getNotchType() const
{
    return d->m_notchType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchType(NotchType lineType)
{
    d->m_notchType = lineType;
}

//---------------------------------------------------------------------------------------------------------------------
NotchSubType VPieceNode::getNotchSubType() const
{
    return d->m_notchSubType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchSubType(NotchSubType notchSubType)
{
    d->m_notchSubType = notchSubType;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::showNotch() const
{
    return d->m_showNotch;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setShowNotch(bool value)
{
    d->m_showNotch = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::showSecondNotch() const
{
    return d->m_showSecondNotch;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setShowSecondNotch(bool value)
{
    d->m_showSecondNotch = value;
}


//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchLength(qreal notchLength)
{
    d->m_notchLength = notchLength;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::getNotchLength() const
{
    return d->m_notchLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchWidth(qreal notchWidth)
{
    d->m_notchWidth = notchWidth;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::getNotchWidth() const
{
    return d->m_notchWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchAngle(qreal notchAngle)
{
    d->m_notchAngle = notchAngle;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPieceNode::getNotchAngle() const
{
    return d->m_notchAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::setNotchCount(int notchCount)
{
    d->m_notchCount = notchCount;
}

//---------------------------------------------------------------------------------------------------------------------
int VPieceNode::getNotchCount() const
{
    return d->m_notchCount;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPieceNode::isExcluded() const
{
    return d->m_excluded;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceNode::SetExcluded(bool exclude)
{
    d->m_excluded = exclude;
}

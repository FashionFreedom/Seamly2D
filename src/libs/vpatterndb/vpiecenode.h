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

#ifndef VPIECENODE_H
#define VPIECENODE_H

#include <QtGlobal>
#include <QSharedDataPointer>
#include <QMetaType>

#include "../vmisc/def.h"

class VPieceNodeData;
class VContainer;

class VPieceNode
{
public:
                       VPieceNode();
                       VPieceNode(quint32 id, Tool typeTool, bool reverse = false);
                       VPieceNode(const VPieceNode &node);

                      ~VPieceNode();

    VPieceNode         &operator=(const VPieceNode &node);

#ifdef Q_COMPILER_RVALUE_REFS
	VPieceNode           &operator=(VPieceNode &&node) Q_DECL_NOTHROW;
#endif

	void                  Swap(VPieceNode &node) Q_DECL_NOTHROW;

    friend QDataStream &operator<<(QDataStream &out, const VPieceNode &);
    friend QDataStream &operator>>(QDataStream &in, VPieceNode &p);

    quint32             GetId() const;
    void                SetId(quint32 id);

    Tool                GetTypeTool() const;
    void                SetTypeTool(Tool value);

    bool                GetReverse() const;
    void                SetReverse(bool reverse);

    bool                isExcluded() const;
    void                SetExcluded(bool exclude);

    qreal               GetSABefore(const VContainer *data) const;
    qreal               GetSABefore(const VContainer *data, Unit unit) const;

    QString             GetFormulaSABefore() const;
    void                SetFormulaSABefore(const QString &formula);

    qreal               GetSAAfter(const VContainer *data) const;
    qreal               GetSAAfter(const VContainer *data, Unit unit) const;

    QString             GetFormulaSAAfter() const;
    void                SetFormulaSAAfter(const QString &formula);

    PieceNodeAngle      GetAngleType() const;
    void                SetAngleType(PieceNodeAngle type);

    bool                isNotch() const;
    void                setNotch(bool notch);

    bool                IsMainPathNode() const;
    void                SetMainPathNode(bool value);

    NotchType           getNotchType() const;
    void                setNotchType(NotchType notchType);

    NotchSubType        getNotchSubType() const;
    void                setNotchSubType(NotchSubType notchSubType);

    bool                showSecondNotch() const;
    void                setShowSecondNotch(bool value);

    bool                showNotch() const;
    void                setShowNotch(bool value);

    qreal               getNotchLength() const;
    void                setNotchLength(qreal notchLength);

    qreal               getNotchWidth() const;
    void                setNotchWidth(qreal notchWidth);

    qreal               getNotchAngle() const;
    void                setNotchAngle(qreal notchAngle);

    int                 getNotchCount() const;
    void                setNotchCount(int notchCount);

private:
    QSharedDataPointer<VPieceNodeData> d;
};

Q_DECLARE_METATYPE(VPieceNode)
Q_DECLARE_TYPEINFO(VPieceNode, Q_MOVABLE_TYPE);

#endif // VPIECENODE_H

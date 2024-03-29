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
 **  @file   vabstractnode.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VABSTRACTNODE_H
#define VABSTRACTNODE_H

#include <qcompilerdetection.h>
#include <QColor>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vabstracttool.h"

enum class ParentType : bool {Scene, Item};

/**
 * @brief The VAbstractNode class parent class for all detail node.
 */
class VAbstractNode : public VAbstractTool
{
    Q_OBJECT
public:
    VAbstractNode(VAbstractPattern *doc, VContainer *data, const quint32 &id, const quint32 &idNode,
                  const QString &blockName = QString(), const quint32 &idTool = 0, QObject *parent = nullptr);
    virtual      ~VAbstractNode() Q_DECL_EQ_DEFAULT;
    static const QString AttrIdTool;
    virtual void ShowVisualization(bool show) Q_DECL_OVERRIDE;
    virtual void incrementReferens() Q_DECL_OVERRIDE;
    virtual void decrementReferens() Q_DECL_OVERRIDE;

    ParentType GetParentType() const;
    void       SetParentType(const ParentType &value);

    virtual void GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;

    bool IsExluded() const;
    void SetExluded(bool exluded);

protected:
    ParentType parentType;

    /** @brief idNodenode id. */
    quint32       idNode;

    /** @brief idTool id tool. */
    quint32       idTool;

    QString       m_blockName;

    bool          m_exluded;

    void         AddToModeling(const QDomElement &domElement);
    virtual void ToolCreation(const Source &typeCreation) Q_DECL_OVERRIDE;
    virtual void SetVisualization() Q_DECL_OVERRIDE {}

    virtual void ShowNode()=0;
    virtual void HideNode()=0;
private:
    Q_DISABLE_COPY(VAbstractNode)
};

#endif // VABSTRACTNODE_H

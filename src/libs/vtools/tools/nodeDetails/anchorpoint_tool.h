/***************************************************************************
 **  @file   anchorpoint_tool.h
 **  @author Douglas S Caskey
 **  @date   Jan 3, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vtoolpin.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef ANCHOR_POINT_TOOL_H
#define ANCHOR_POINT_TOOL_H

#include <QtGlobal>

#include "vabstractnode.h"

class DialogTool;

class AnchorPointTool : public VAbstractNode
{
    Q_OBJECT
public:
    static AnchorPointTool *Create(QSharedPointer<DialogTool> dialog, VAbstractPattern *doc, VContainer *data);
    static AnchorPointTool *Create(quint32 _id, quint32 pointId, quint32 pieceId, VAbstractPattern *doc, VContainer *data,
                            const Document &parse, const Source &typeCreation, const QString &blockName = QString(),
                            const quint32 &idTool = 0);

    static const QString  ToolType;
    virtual QString       getTagName() const Q_DECL_OVERRIDE;

public slots:
    virtual void            FullUpdateFromFile () Q_DECL_OVERRIDE {}
    virtual void            AllowHover(bool enabled) Q_DECL_OVERRIDE;
    virtual void            AllowSelecting(bool enabled) Q_DECL_OVERRIDE;

protected:
    virtual void            AddToFile() Q_DECL_OVERRIDE;
    virtual void            ShowNode() Q_DECL_OVERRIDE {}
    virtual void            HideNode() Q_DECL_OVERRIDE {}

private:
                            Q_DISABLE_COPY(AnchorPointTool)

    quint32                 m_pieceId;

                            AnchorPointTool(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 pointId,
                                            quint32 pieceId, const Source &typeCreation,
                                            const QString &blockName = QString(), const quint32 &idTool = 0,
                                            QObject *qoParent = nullptr);
};

#endif // ANCHOR_POINT_TOOL_H

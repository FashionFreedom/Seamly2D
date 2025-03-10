//---------------------------------------------------------------------------------------------------------------------
//  @file   internal_path_tool.h
//  @author Douglas S Caskey
//  @date   7 Dec, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
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

//---------------------------------------------------------------------------------------------------------------------
//  @file   vtoolinternalpath.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   24 11, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef INTERNAL_PATH_TOOL_H
#define INTERNAL_PATH_TOOL_H

#include <QtGlobal>

#include "vabstractnode.h"

class DialogTool;

class InternalPathTool : public VAbstractNode, public QGraphicsPathItem
{
    Q_OBJECT
public:
    static InternalPathTool *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                     VContainer *data);
    static InternalPathTool *Create(quint32 _id, const VPiecePath &path, quint32 pieceId, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data, const Document &parse,
                                     const Source &typeCreation, const QString &blockName = QString(),
                                     const quint32 &toolId = NULL_ID);

    virtual int      type() const override {return Type;}
    enum             {Type = UserType + static_cast<int>(Tool::InternalPath)};
    virtual QString  getTagName() const override;

    virtual void     paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                           QWidget *widget = nullptr) override;

    virtual void     incrementReferens() override;
    virtual void     decrementReferens() override;

    void             refreshGeometry();

    static void      addAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiecePath &path);

public slots:
    virtual void     FullUpdateFromFile () override;
    virtual void     AllowHover(bool enabled) override;
    virtual void     AllowSelecting(bool enabled) override;

protected:
    virtual void     AddToFile() override;
    virtual void     ShowNode() override;
    virtual void     HideNode() override;
    virtual void     ToolCreation(const Source &typeCreation) override;

private:
    Q_DISABLE_COPY(InternalPathTool)

    quint32          m_pieceId;

                     InternalPathTool(VAbstractPattern *doc, VContainer *data, quint32 id, quint32 pieceId,
                                       const Source &typeCreation, const QString &blockName = QString(),
                                       const quint32 &toolId = NULL_ID, QObject *objParent = nullptr,
                                       QGraphicsItem * parent = nullptr );


    void             incrementNodes(const VPiecePath &path) const;
    void             decrementNodes(const VPiecePath &path) const;
};

#endif // INTERNAL_PATH_TOOL_H

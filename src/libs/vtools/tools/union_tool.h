/***************************************************************************
 **  @file   union_tool.h
 **  @author Douglas S Caskey
 **  @date   Dec 27, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file  vtooluniondetails.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 12, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef UNION_TOOL_H
#define UNION_TOOL_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QDomNode>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiece.h"

class DialogTool;

struct UnionToolInitData
{
    UnionToolInitData()
        : piece1_Id(NULL_ID),
          piece2_Id(NULL_ID),
          piece1_Index(NULL_ID),
          piece2_Index(NULL_ID),
          scene(nullptr),
          doc(nullptr),
          data(nullptr),
          parse(Document::FullParse),
          typeCreation(Source::FromFile),
          retainPieces(false)
    {}

    quint32 piece1_Id;
    quint32 piece2_Id;
    quint32 piece1_Index;
    quint32 piece2_Index;
    VMainGraphicsScene *scene;
    VAbstractPattern *doc;
    VContainer *data;
    Document parse;
    Source typeCreation;
    bool retainPieces;
};

/**
 * @brief The UnionTool class tool union details.
 */
class UnionTool : public VAbstractTool
{
    Q_OBJECT
public:
    static UnionTool *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data);
    static UnionTool *Create(const quint32 _id, const UnionToolInitData &initData);

    static const QString ToolType;
    static const QString TagUnionPiece;
    static const QString TagNode;
    static const QString TagChildren;
    static const QString TagChild;
    static const QString AttrIndexD1;
    static const QString AttrIndexD2;
    static const QString AttrIdObject;
    static const QString AttrNodeType;
    static const QString NodeTypeContour;
    static const QString NodeTypeModeling;

    virtual QString getTagName() const Q_DECL_OVERRIDE;
    virtual void ShowVisualization(bool show) Q_DECL_OVERRIDE;
    virtual void incrementReferens() Q_DECL_OVERRIDE;
    virtual void decrementReferens() Q_DECL_OVERRIDE;
    virtual void GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;

public slots:
    /**
     * @brief FullUpdateFromFile update tool data form file.
     */
    virtual void FullUpdateFromFile () Q_DECL_OVERRIDE {}
    virtual void AllowHover(bool) Q_DECL_OVERRIDE {}
    virtual void AllowSelecting(bool) Q_DECL_OVERRIDE {}

protected:
    virtual void AddToFile() Q_DECL_OVERRIDE;
    virtual void SetVisualization() Q_DECL_OVERRIDE {}

private:
    Q_DISABLE_COPY(UnionTool)

    quint32 piece1_Id;    /** @brief d1 first detail id. */
    quint32 piece2_Id;    /** @brief d2 second detail id. */
    quint32 piece1_Index; /** @brief piece1_Index index edge in first detail. */
    quint32 piece2_Index; /** @brief piece2_Index index edge in second detail. */

    UnionTool(quint32 id, const UnionToolInitData &initData, QObject *parent = nullptr);

    void             addPiece(QDomElement &domElement, const VPiece &d) const;
    void             AddToModeling(const QDomElement &domElement);
    QVector<quint32> GetReferenceObjects() const;
    QVector<quint32> ReferenceObjects(const QDomElement &root, const QString &tag, const QString &attribute) const;
};

#endif // UNION_TOOL_H

/***************************************************************************
 **  @file   union_tool.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   vtooluniondetails.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 12, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2013 Valentina project
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

#include "union_tool.h"

#include <QByteArray>
#include <QDomNodeList>
#include <QFileInfo>
#include <QHash>
#include <QLineF>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QUndoStack>
#include <QtDebug>
#include <new>

#include "pattern_piece_tool.h"
#include "vdatatool.h"
#include "vnodedetail.h"
#include "nodeDetails/anchorpoint_tool.h"
#include "nodeDetails/vnodearc.h"
#include "nodeDetails/vnodeellipticalarc.h"
#include "nodeDetails/vnodepoint.h"
#include "nodeDetails/vnodespline.h"
#include "nodeDetails/vnodesplinepath.h"
#include "nodeDetails/vtoolinternalpath.h"
#include "../dialogs/tools/dialogtool.h"
#include "../dialogs/tools/union_dialog.h"
#include "../ifc/xml/abstract_converter.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vlabeltemplateconverter.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vformat/vlabeltemplate.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"

const QString UnionTool::ToolType         = QStringLiteral("union");
const QString UnionTool::TagUnionPiece    = QStringLiteral("unionPiece");
const QString UnionTool::TagNode          = QStringLiteral("node");
const QString UnionTool::TagChildren      = QStringLiteral("children");
const QString UnionTool::TagChild         = QStringLiteral("child");
const QString UnionTool::AttrIndexD1      = QStringLiteral("indexD1");
const QString UnionTool::AttrIndexD2      = QStringLiteral("indexD2");
const QString UnionTool::AttrIdObject     = QStringLiteral("idObject");
const QString UnionTool::AttrNodeType     = QStringLiteral("nodeType");
const QString UnionTool::NodeTypeContour  = QStringLiteral("Contour");
const QString UnionTool::NodeTypeModeling = QStringLiteral("Modeling");

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vToolUnion, "v.toolUnion")

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
VPiecePath GetPiecePath(int piece, VAbstractPattern *doc, quint32 id)
{
    const QDomElement tool = doc->elementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        VException e(QString("Can't get tool by id='%1'.").arg(id));
        throw e;
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        const QDomElement element = nodesList.at(i).toElement();
        if (not element.isNull() && element.tagName() == UnionTool::TagUnionPiece && i+1 == piece)
        {
            const QDomNodeList pieceList = element.childNodes();
            for (qint32 j = 0; j < pieceList.size(); ++j)
            {
                const QDomElement element = pieceList.at(j).toElement();
                if (not element.isNull() && element.tagName() == VAbstractPattern::TagNodes)
                {
                    return VAbstractPattern::ParsePieceNodes(element);
                }
            }
        }
    }

    return VPiecePath();
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath GetPiece1MainPath(VAbstractPattern *doc, quint32 id)
{
    return GetPiecePath(1, doc, id);
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath GetPiece2MainPath(VAbstractPattern *doc, quint32 id)
{
    return GetPiecePath(2, doc, id);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> GetPiece2CSAPaths(VAbstractPattern *doc, quint32 id)
{
    const QDomElement tool = doc->elementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        VException e(QString("Can't get tool by id='%1'.").arg(id));
        throw e;
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        const QDomElement element = nodesList.at(i).toElement();
        if (not element.isNull() && element.tagName() == UnionTool::TagUnionPiece && i+1 == 2)
        {
            const QDomNodeList pieceList = element.childNodes();
            for (qint32 j = 0; j < pieceList.size(); ++j)
            {
                const QDomElement element = pieceList.at(j).toElement();
                if (not element.isNull() && element.tagName() == PatternPieceTool::TagCSA)
                {
                    return VAbstractPattern::ParsePieceCSARecords(element);
                }
            }
        }
    }

    return QVector<CustomSARecord>();
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetPiece2InternalPaths(VAbstractPattern *doc, quint32 id)
{
    const QDomElement tool = doc->elementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        VException e(QString("Can't get tool by id='%1'.").arg(id));
        throw e;
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        const QDomElement element = nodesList.at(i).toElement();
        if (not element.isNull() && element.tagName() == UnionTool::TagUnionPiece && i+1 == 2)
        {
            const QDomNodeList pieceList = element.childNodes();
            for (qint32 j = 0; j < pieceList.size(); ++j)
            {
                const QDomElement element = pieceList.at(j).toElement();
                if (not element.isNull() && element.tagName() == PatternPieceTool::TagIPaths)
                {
                    return VAbstractPattern::ParsePieceInternalPaths(element);
                }
            }
        }
    }

    return QVector<quint32>();
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetPiece2Anchors(VAbstractPattern *doc, quint32 id)
{
    const QDomElement tool = doc->elementById(id, VAbstractPattern::TagTools);
    if (tool.isNull())
    {
        VException e(QString("Can't get tool by id='%1'.").arg(id));
        throw e;
    }

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        const QDomElement element = nodesList.at(i).toElement();
        if (not element.isNull() && element.tagName() == UnionTool::TagUnionPiece && i+1 == 2)
        {
            const QDomNodeList pieceList = element.childNodes();
            for (qint32 j = 0; j < pieceList.size(); ++j)
            {
                const QDomElement element = pieceList.at(j).toElement();
                if (not element.isNull() && element.tagName() == PatternPieceTool::TagAnchors)
                {
                    return VAbstractPattern::ParsePieceAnchors(element);
                }
            }
        }
    }

    return QVector<quint32>();
}

//---------------------------------------------------------------------------------------------------------------------
QString getBlockName(VAbstractPattern *doc, quint32 piece1_Id, quint32 piece2_Id)
{
    const QDomElement piece1 = doc->elementById(piece1_Id, VAbstractPattern::TagPiece);
    if (piece1.isNull())
    {
        return QString();
    }

    const QDomElement piece2 = doc->elementById(piece2_Id, VAbstractPattern::TagPiece);
    if (piece2.isNull())
    {
        return QString();
    }

    const QDomElement block1 = piece1.parentNode().parentNode().toElement();
    if (block1.isNull() || not block1.hasAttribute(VAbstractPattern::AttrName))
    {
        return QString();
    }

    const QDomElement block2 = piece2.parentNode().parentNode().toElement();
    if (block2.isNull() || not block2.hasAttribute(VAbstractPattern::AttrName))
    {
        return QString();
    }

    const QString block1Name = block1.attribute(VAbstractPattern::AttrName);
    const QString block2Name = block2.attribute(VAbstractPattern::AttrName);

    if (block1Name == block2Name)
    {
        return block1Name;
    }

    const QDomElement pattern = block1.parentNode().toElement();
    if (pattern.isNull())
    {
        return QString();
    }

    int piece1_Index = 0;
    int piece2_Index = 0;
    const QDomNodeList blockList = pattern.elementsByTagName(VAbstractPattern::TagDraftBlock);
    for (int i=0; i < blockList.size(); ++i)
    {
        const QDomElement draftBlockElement = blockList.at(i).toElement();
        if (draftBlockElement == block1)
        {
            piece1_Index = i;
        }

        if (draftBlockElement == block2)
        {
            piece2_Index = i;
        }
    }

    if (piece1_Index >= piece2_Index)
    {
        return block1Name;
    }
    else
    {
        return block2Name;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief BiasRotatePoint bias and rotate point.
 * @param point point.
 * @param dx bias x axis.
 * @param dy bias y axis.
 * @param pRotate point rotation.
 * @param angle angle rotation.
 */
void BiasRotatePoint(VPointF *point, qreal dx, qreal dy, const QPointF &pRotate, qreal angle)
{
    point->setX(point->x()+dx);
    point->setY(point->y()+dy);
    QLineF line(pRotate, static_cast<QPointF>(*point));
    line.setAngle(line.angle()+angle);
    point->setX(line.p2().x());
    point->setY(line.p2().y());
}

//---------------------------------------------------------------------------------------------------------------------
void PointsOnEdge(const VPiecePath &path, quint32 index, VPointF &p1, VPointF &p2, VContainer *data)
{
    VPieceNode piece2_Pt1;
    VPieceNode piece2_Pt2;
    path.NodeOnEdge(index, piece2_Pt1, piece2_Pt2);
    p1 = VPointF(*data->GeometricObject<VPointF>(piece2_Pt1.GetId()));
    p2 = VPointF(*data->GeometricObject<VPointF>(piece2_Pt2.GetId()));
}

//---------------------------------------------------------------------------------------------------------------------
void UnionInitParameters(const UnionToolInitData &initData, const VPiecePath &piece1Path, const VPiecePath &piece2Path,
                         VPieceNode &piece1_Pt1, qreal &dx, qreal &dy, qreal &angle)
{
    VPieceNode piece1_Pt2;
    piece1Path.NodeOnEdge(initData.piece1_Index, piece1_Pt1, piece1_Pt2);
    Q_UNUSED(piece1_Pt2)

    VPointF point1;
    VPointF point2;
    PointsOnEdge(piece1Path, initData.piece1_Index, point1, point2, initData.data);

    VPointF point3;
    VPointF point4;
    PointsOnEdge(piece2Path, initData.piece2_Index, point3, point4, initData.data);

    dx = point1.x() - point4.x();
    dy = point1.y() - point4.y();

    point3.setX(point3.x()+dx);
    point3.setY(point3.y()+dy);

    point4.setX(point4.x()+dx);
    point4.setY(point4.y()+dy);

    const QLineF p4p3 = QLineF(static_cast<QPointF>(point4), static_cast<QPointF>(point3));
    const QLineF p1p2 = QLineF(static_cast<QPointF>(point1), static_cast<QPointF>(point2));

    angle = p4p3.angleTo(p1p2);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddNodePoint(const VPieceNode &node, const UnionToolInitData &initData, quint32 idTool,
                     QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    QScopedPointer<VPointF> point(new VPointF(*initData.data->GeometricObject<VPointF>(node.GetId())));
    point->setMode(Draw::Modeling);

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.data(), dx, dy, static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)),
                        angle);
    }

    QScopedPointer<VPointF> point1(new VPointF(*point));

    const quint32 objectId = initData.data->AddGObject(point.take());
    children.append(objectId);
    point1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(point1.take());
    VNodePoint::Create(initData.doc, initData.data, initData.scene, id, objectId, Document::FullParse, Source::FromTool,
                       blockName, idTool);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddAnchorPoint(quint32 id, const UnionToolInitData &initData, quint32 idTool, QVector<quint32> &children,
               const QString &blockName, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    QScopedPointer<VPointF> point(new VPointF(*initData.data->GeometricObject<VPointF>(id)));
    point->setMode(Draw::Modeling);

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.data(), dx, dy, static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)),
                        angle);
    }

    QScopedPointer<VPointF> point1(new VPointF(*point));

    const quint32 objectId = initData.data->AddGObject(point.take());
    children.append(objectId);
    point1->setMode(Draw::Modeling);
    const quint32 anchorId = initData.data->AddGObject(point1.take());
    AnchorPointTool::Create(anchorId, objectId, 0, initData.doc, initData.data, Document::FullParse,
                            Source::FromTool, blockName, idTool);
    return anchorId;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddNodeArc(const VPieceNode &node, const UnionToolInitData &initData, quint32 idTool,
                   QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                   quint32 pRotate, qreal angle)
{
    const QSharedPointer<VArc> arc = initData.data->GeometricObject<VArc>(node.GetId());
    VPointF p1 = VPointF(arc->GetP1(), "A", 0, 0);
    VPointF p2 = VPointF(arc->GetP2(), "A", 0, 0);
    QScopedPointer<VPointF> center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));
    center->setMode(Draw::Modeling);
    VPointF *tmpCenter = center.take();
    const quint32 idCenter = initData.data->AddGObject(tmpCenter);
    Q_UNUSED(idCenter)
    QScopedPointer<VArc> arc1(new VArc(*tmpCenter, arc->GetRadius(), arc->GetFormulaRadius(), l1.angle(),
                                       QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle())));
    arc1->setMode(Draw::Modeling);

    QScopedPointer<VArc>arc2(new VArc(*arc1));

    const quint32 objectId = initData.data->AddGObject(arc1.take());
    children.append(objectId);

    arc2->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(arc2.take());

    VNodeArc::Create(initData.doc, initData.data, id, objectId, Document::FullParse, Source::FromTool, blockName,
                     idTool);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddNodeElArc(const VPieceNode &node, const UnionToolInitData &initData, quint32 idTool,
                     QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    const QSharedPointer<VEllipticalArc> arc = initData.data->GeometricObject<VEllipticalArc>(node.GetId());
    VPointF p1 = VPointF(arc->GetP1(), "A", 0, 0);
    VPointF p2 = VPointF(arc->GetP2(), "A", 0, 0);
    QScopedPointer<VPointF> center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));
    center->setMode(Draw::Modeling);
    VPointF *tmpCenter = center.take();
    quint32 idCenter = initData.data->AddGObject(tmpCenter);
    Q_UNUSED(idCenter)
    QScopedPointer<VEllipticalArc> arc1(new VEllipticalArc (*tmpCenter, arc->GetRadius1(), arc->GetRadius2(),
                                                            arc->GetFormulaRadius1(), arc->GetFormulaRadius2(),
                                                            l1.angle(), QString().setNum(l1.angle()), l2.angle(),
                                                            QString().setNum(l2.angle()), 0, "0"));
    arc1->setMode(Draw::Modeling);

    QScopedPointer<VEllipticalArc> arc2(new VEllipticalArc(*arc1));

    const quint32 objectId = initData.data->AddGObject(arc1.take());
    children.append(objectId);

    arc2->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(arc2.take());

    VNodeEllipticalArc::Create(initData.doc, initData.data, id, objectId, Document::FullParse, Source::FromTool,
                               blockName, idTool);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddNodeSpline(const VPieceNode &node, const UnionToolInitData &initData, quint32 idTool,
                      QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                      quint32 pRotate, qreal angle)
{
    const QSharedPointer<VAbstractCubicBezier> spline =
            initData.data->GeometricObject<VAbstractCubicBezier>(node.GetId());

    QScopedPointer<VPointF> p1(new VPointF(spline->GetP1()));
    VPointF p2 = VPointF(spline->GetP2());
    VPointF p3 = VPointF(spline->GetP3());
    QScopedPointer<VPointF> p4(new VPointF(spline->GetP4()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(p1.data(), dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(&p3, dx, dy, p, angle);
        BiasRotatePoint(p4.data(), dx, dy, p, angle);
    }

    VSpline *spl = new VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4, 0, Draw::Modeling);
    const quint32 objectId = initData.data->AddGObject(spl);
    children.append(objectId);

    VSpline *spl1 = new VSpline(*spl);
    spl1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(spl1);
    VNodeSpline::Create(initData.doc, initData.data, id, objectId, Document::FullParse, Source::FromTool, blockName,
                        idTool);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 AddNodeSplinePath(const VPieceNode &node, const UnionToolInitData &initData, quint32 idTool,
                          QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                          quint32 pRotate, qreal angle)
{
    QScopedPointer<VSplinePath> path(new VSplinePath());
    path->setMode(Draw::Modeling);
    const QSharedPointer<VAbstractCubicBezierPath> splinePath =
            initData.data->GeometricObject<VAbstractCubicBezierPath>(node.GetId());
    for (qint32 i = 1; i <= splinePath->CountSubSpl(); ++i)
    {
        const VSpline spline = splinePath->GetSpline(i);

        QScopedPointer<VPointF> p1(new VPointF(spline.GetP1()));
        VPointF p2 = VPointF(spline.GetP2());
        VPointF p3 = VPointF(spline.GetP3());
        QScopedPointer<VPointF> p4(new VPointF(spline.GetP4()));
        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            const QPointF p = static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate));

            BiasRotatePoint(p1.data(), dx, dy, p, angle);
            BiasRotatePoint(&p2, dx, dy, p, angle);
            BiasRotatePoint(&p3, dx, dy, p, angle);
            BiasRotatePoint(p4.data(), dx, dy, p, angle);
        }

        VSpline spl = VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4);
        if (i==1)
        {
            const qreal angle1 = spl.GetStartAngle()+180;
            const QString angle1F = QString().number(angle1);

            path->append(VSplinePoint(*p1, angle1, angle1F, spl.GetStartAngle(), spl.GetStartAngleFormula(),
                                      0, "0", spline.GetC1Length(), spline.GetC1LengthFormula()));
        }

        const qreal angle2 = spl.GetEndAngle()+180;
        const QString angle2F = QString().number(angle2);
        qreal pL2 = 0;
        QString pL2F("0");
        if (i+1 <= splinePath->CountSubSpl())
        {
            const VSpline nextSpline = splinePath->GetSpline(i+1);
            pL2 = nextSpline.GetC1Length();
            pL2F = nextSpline.GetC1LengthFormula();
        }

        path->append(VSplinePoint(*p4, spl.GetEndAngle(), spl.GetEndAngleFormula(), angle2, angle2F,
                                  spline.GetC2Length(), spline.GetC2LengthFormula(), pL2, pL2F));
    }
    QScopedPointer<VSplinePath> path1(new VSplinePath(*path));

    const quint32 objectId = initData.data->AddGObject(path.take());
    children.append(objectId);

    path1->setMode(Draw::Modeling);
    const quint32 id = initData.data->AddGObject(path1.take());

    VNodeSplinePath::Create(initData.doc, initData.data, id, objectId, Document::FullParse, Source::FromTool, blockName,
                            idTool);
    return id;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddNodeToNewPath create union adding one node at a  time.
 */
void AddNodeToNewPath(const UnionToolInitData &initData, VPiecePath &newPath, VPieceNode node,
                      quint32 idTool, QVector<quint32> &children, const QString &blockName, qreal dx = 0, qreal dy = 0,
                      quint32 pRotate = NULL_ID, qreal angle = 0);

void AddNodeToNewPath(const UnionToolInitData &initData, VPiecePath &newPath, VPieceNode node,
                      quint32 idTool, QVector<quint32> &children, const QString &blockName, qreal dx, qreal dy,
                      quint32 pRotate, qreal angle)
{
    quint32 id = 0;
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
            id = AddNodePoint(node, initData, idTool, children, blockName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeArc):
            id = AddNodeArc(node, initData, idTool, children, blockName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeElArc):
            id = AddNodeElArc(node, initData, idTool, children, blockName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSpline):
            id = AddNodeSpline(node, initData, idTool, children, blockName, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSplinePath):
            id = AddNodeSplinePath(node, initData, idTool, children, blockName, dx, dy, pRotate, angle);
            break;
        default:
            qWarning() << "May be wrong tool type!!! Ignoring." << Q_FUNC_INFO;
            break;
    }

    node.SetId(id);
    newPath.Append(node);
}

//---------------------------------------------------------------------------------------------------------------------
void FindIndexJ(qint32 piece2Points, const VPiecePath &piece2Path, quint32 piece2_Index, qint32 &j)
{
    if (piece2Points == 0)
    {
        VPieceNode node1;
        VPieceNode node2;
        piece2Path.NodeOnEdge(piece2_Index, node1, node2);
        const VPiecePath removedPiece = piece2Path.RemoveEdge(piece2_Index);
        const int k = removedPiece.indexOfNode(node2.GetId());
        SCASSERT(k != -1)
        if (k == removedPiece.CountNodes()-1)
        {//We have last node in piece, we wil begin from 0
            j = 0;
        }
        else
        {// Continue from next node
            j = k+1;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QDomElement GetTagChildren(VAbstractPattern *doc, quint32 id)
{
    QDomElement toolUnion = doc->elementById(id, VAbstractPattern::TagTools);
    if (toolUnion.isNull())
    {
        VException e(QString("Can't get tool by id='%1'.").arg(id));
        throw e;
    }

    QDomElement tagChildren = toolUnion.firstChildElement(UnionTool::TagChildren);

    if (tagChildren.isNull())
    {
        tagChildren = doc->createElement(UnionTool::TagChildren);
        toolUnion.appendChild(tagChildren);
    }

    return tagChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void SaveChildren(VAbstractPattern *doc, quint32 id, QDomElement section, const QVector<quint32> &children)
{
    if (children.size() > 0)
    {
        for (int i=0; i<children.size(); ++i)
        {
            QDomElement tagChild = doc->createElement(UnionTool::TagChild);
            tagChild.appendChild(doc->createTextNode(QString().setNum(children.at(i))));
            section.appendChild(tagChild);
        }

        GetTagChildren(doc, id).appendChild(section);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SaveNodesChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(VAbstractPattern::TagNodes), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveCSAChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(PatternPieceTool::TagCSA), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveInternalPathsChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(PatternPieceTool::TagIPaths), children);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveAnchorsChildren(VAbstractPattern *doc, quint32 id, const QVector<quint32> &children)
{
    SaveChildren(doc, id, doc->createElement(PatternPieceTool::TagAnchors), children);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetChildren(VAbstractPattern *doc, quint32 id, const QString &tagName)
{
    const QDomElement toolUnion = doc->elementById(id, VAbstractPattern::TagTools);
    if (toolUnion.isNull())
    {
        return QVector<quint32>();
    }

    const QDomElement tagChildren = toolUnion.firstChildElement(UnionTool::TagChildren);
    if (tagChildren.isNull())
    {
        return QVector<quint32>();
    }

    const QDomElement tagNodes = tagChildren.firstChildElement(tagName);
    if (tagNodes.isNull())
    {
        return QVector<quint32>();
    }

    QVector<quint32> childrenId;
    const QDomNodeList listChildren = tagNodes.elementsByTagName(UnionTool::TagChild);
    for (int i=0; i < listChildren.size(); ++i)
    {
        const QDomElement domElement = listChildren.at(i).toElement();
        if (not domElement.isNull())
        {
            childrenId.append(domElement.text().toUInt());
        }
    }
    return childrenId;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetNodesChildren(VAbstractPattern *doc, quint32 id)
{
    return GetChildren(doc, id, VAbstractPattern::TagNodes);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetCSAChildren(VAbstractPattern *doc, quint32 id)
{
    return GetChildren(doc, id, PatternPieceTool::TagCSA);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> GetInternalPathsChildren(VAbstractPattern *doc, quint32 id)
{
    return GetChildren(doc, id, PatternPieceTool::TagIPaths);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> getAnchorChildren(VAbstractPattern *doc, quint32 id)
{
    return GetChildren(doc, id, PatternPieceTool::TagAnchors);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 TakeNextId(QVector<quint32> &children)
{
    quint32 idChild = NULL_ID;
    if (not children.isEmpty())
    {
        idChild = children.takeFirst();
    }
    else
    {
        idChild = NULL_ID;
    }
    return idChild;
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodePoint(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    QScopedPointer<VPointF> point(new VPointF(*data->GeometricObject<VPointF>(node.GetId())));
    point->setMode(Draw::Modeling);
    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        BiasRotatePoint(point.data(), dx, dy, static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate)), angle);
    }
    data->UpdateGObject(TakeNextId(children), point.take());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeArc(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                   quint32 pRotate, qreal angle)
{
    const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(node.GetId());
    VPointF p1 = VPointF(arc->GetP1());
    VPointF p2 = VPointF(arc->GetP2());
    QScopedPointer<VPointF> center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));

    QScopedPointer<VArc> arc1(new VArc(*center, arc->GetRadius(), arc->GetFormulaRadius(), l1.angle(),
                                       QString().setNum(l1.angle()), l2.angle(), QString().setNum(l2.angle())));
    arc1->setMode(Draw::Modeling);
    data->UpdateGObject(TakeNextId(children), arc1.take());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeElArc(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                     quint32 pRotate, qreal angle)
{
    const QSharedPointer<VEllipticalArc> arc = data->GeometricObject<VEllipticalArc>(node.GetId());
    VPointF p1 = VPointF(arc->GetP1());
    VPointF p2 = VPointF(arc->GetP2());
    QScopedPointer<VPointF> center(new VPointF(arc->GetCenter()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(&p1, dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(center.data(), dx, dy, p, angle);
    }

    QLineF l1(static_cast<QPointF>(*center), static_cast<QPointF>(p1));
    QLineF l2(static_cast<QPointF>(*center), static_cast<QPointF>(p2));

    QScopedPointer<VEllipticalArc> arc1(new VEllipticalArc (*center, arc->GetRadius1(), arc->GetRadius2(),
                                                            arc->GetFormulaRadius1(), arc->GetFormulaRadius2(),
                                                            l1.angle(), QString().setNum(l1.angle()), l2.angle(),
                                                            QString().setNum(l2.angle()), 0, "0"));
    arc1->setMode(Draw::Modeling);
    data->UpdateGObject(TakeNextId(children), arc1.take());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeSpline(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                      quint32 pRotate, qreal angle)
{
    const QSharedPointer<VAbstractCubicBezier> spline =
            data->GeometricObject<VAbstractCubicBezier>(node.GetId());

    QScopedPointer<VPointF> p1(new VPointF(spline->GetP1()));
    VPointF p2 = VPointF(spline->GetP2());
    VPointF p3 = VPointF(spline->GetP3());
    QScopedPointer<VPointF> p4(new VPointF(spline->GetP4()));

    if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
    {
        const QPointF p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

        BiasRotatePoint(p1.data(), dx, dy, p, angle);
        BiasRotatePoint(&p2, dx, dy, p, angle);
        BiasRotatePoint(&p3, dx, dy, p, angle);
        BiasRotatePoint(p4.data(), dx, dy, p, angle);
    }

    QScopedPointer<VSpline> spl(new VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4, 0,
                                            Draw::Modeling));
    data->UpdateGObject(TakeNextId(children), spl.take());
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateNodeSplinePath(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                          quint32 pRotate, qreal angle)
{
    QScopedPointer<VSplinePath> path(new VSplinePath());
    path->setMode(Draw::Modeling);
    const QSharedPointer<VAbstractCubicBezierPath> splinePath =
            data->GeometricObject<VAbstractCubicBezierPath>(node.GetId());
    SCASSERT(splinePath != nullptr)
    for (qint32 i = 1; i <= splinePath->CountSubSpl(); ++i)
    {
        const VSpline spline = splinePath->GetSpline(i);

        QScopedPointer<VPointF> p1(new VPointF(spline.GetP1()));
        VPointF p2 = VPointF(spline.GetP2());
        VPointF p3 = VPointF(spline.GetP3());
        QScopedPointer<VPointF> p4(new VPointF(spline.GetP4()));

        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            const QPointF p = static_cast<QPointF>(*data->GeometricObject<VPointF>(pRotate));

            BiasRotatePoint(p1.data(), dx, dy, p, angle);
            BiasRotatePoint(&p2, dx, dy, p, angle);
            BiasRotatePoint(&p3, dx, dy, p, angle);
            BiasRotatePoint(p4.data(), dx, dy, p, angle);
        }

        VSpline spl = VSpline(*p1, static_cast<QPointF>(p2), static_cast<QPointF>(p3), *p4);
        if (i==1)
        {
            const qreal angle1 = spl.GetStartAngle()+180;
            const QString angle1F = QString().number(angle1);

            path->append(VSplinePoint(*p1, angle1, angle1F, spl.GetStartAngle(), spl.GetStartAngleFormula(),
                                      0, "0", spline.GetC1Length(), spline.GetC1LengthFormula()));
        }

        const qreal angle2 = spl.GetEndAngle()+180;
        const QString angle2F = QString().number(angle2);

        qreal pL2 = 0;
        QString pL2F("0");
        if (i+1 <= splinePath->CountSubSpl())
        {
            const VSpline nextSpline = splinePath->GetSpline(i+1);
            pL2 = nextSpline.GetC1Length();
            pL2F = nextSpline.GetC1LengthFormula();
        }

        path->append(VSplinePoint(*p4, spl.GetEndAngle(), spl.GetEndAngleFormula(), angle2, angle2F,
                                  spline.GetC2Length(), spline.GetC2LengthFormula(), pL2, pL2F));
    }
    data->UpdateGObject(TakeNextId(children), path.take());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateNodes update nodes of union.
 * @param data container with variables.
 * @param node piece's node.
 * @param children list ids of all children.
 * @param dx bias node x axis.
 * @param dy bias node y axis.
 * @param pRotate point rotation.
 * @param angle angle rotation.
 */
void UpdatePathNode(VContainer *data, const VPieceNode &node, QVector<quint32> &children,
                 qreal dx = 0, qreal dy = 0, quint32 pRotate = NULL_ID, qreal angle = 0);
void UpdatePathNode(VContainer *data, const VPieceNode &node, QVector<quint32> &children, qreal dx, qreal dy,
                 quint32 pRotate, qreal angle)
{
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
            UpdateNodePoint(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeArc):
            UpdateNodeArc(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeElArc):
            UpdateNodeElArc(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSpline):
            UpdateNodeSpline(data, node, children, dx, dy, pRotate, angle);
            break;
        case (Tool::NodeSplinePath):
            UpdateNodeSplinePath(data, node, children, dx, dy, pRotate, angle);
            break;
        default:
            qWarning() << "May be wrong tool type!!! Ignoring." << Q_FUNC_INFO;
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedNodes(VPiece &newPiece, const VPiece &piece1, const VPiece &piece2, quint32 id, const QString &blockName,
                       const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    const VPiecePath piece1Path = piece1.GetPath().RemoveEdge(initData.piece1_Index);
    const VPiecePath piece2Path = piece2.GetPath().RemoveEdge(initData.piece2_Index);

    const qint32 piece1NodeCount = piece1Path.CountNodes();
    const qint32 piece2NodeCount = piece2Path.CountNodes();

    qint32 piece2Points = 0; //Keeps number points the second piece, what we have already added.
    qint32 i = 0;
    QVector<quint32> children;
    VPiecePath newPath;
    const int piece1_Pt1_Index = piece1.GetPath().indexOfNode(pRotate);
    do
    {
        AddNodeToNewPath(initData, newPath, piece1Path.at(i), id, children, blockName);
        ++i;
        if (i > piece1_Pt1_Index && piece2Points < piece2NodeCount-1)
        {
            qint32 j = 0;
            FindIndexJ(piece2Points, piece2.GetPath(), initData.piece2_Index, j);
            do
            {
                if (j >= piece2NodeCount)
                {
                    j=0;
                }
                AddNodeToNewPath(initData, newPath, piece2Path.at(j), id, children, blockName, dx, dy, pRotate, angle);
                ++piece2Points;
                ++j;
            } while (piece2Points < piece2NodeCount-1);
        }
    } while (i < piece1NodeCount);

    newPiece.SetPath(newPath);

    SCASSERT(not children.isEmpty())
    SaveNodesChildren(initData.doc, id, children);
}

//---------------------------------------------------------------------------------------------------------------------
void createCSAUnion(VPiece &newPiece, const VPiece &d, QVector<quint32> &children, quint32 id,
                           const QString &blockName, const UnionToolInitData &initData, qreal dx, qreal dy,
                           quint32 pRotate, qreal angle)
{
    QVector<quint32> nodeChildren;
    for(int i=0; i < d.GetCustomSARecords().size(); ++i)
    {
        CustomSARecord record = d.GetCustomSARecords().at(i);
        const VPiecePath path = initData.data->GetPiecePath(record.path);
        VPiecePath newPath = path;
        newPath.Clear();//Clear nodes
        for (int i=0; i < path.CountNodes(); ++i)
        {
            AddNodeToNewPath(initData, newPath, path.at(i), id, nodeChildren, blockName, dx, dy, pRotate, angle);
        }
        const quint32 idPath = initData.data->AddPiecePath(newPath);
        VToolInternalPath::Create(idPath, newPath, NULL_ID, initData.scene, initData.doc, initData.data, initData.parse,
                               Source::FromTool, blockName, id);
        record.path = idPath;
        newPiece.GetCustomSARecords().append(record);
        nodeChildren.prepend(idPath);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedCSA(VPiece &newPiece, const VPiece &piece1, const VPiece &piece2, quint32 id, const QString &blockName,
                     const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    for (int i = 0; i < piece1.GetCustomSARecords().size(); ++i)
    {
        newPiece.GetCustomSARecords().append(piece1.GetCustomSARecords().at(i));
    }

    QVector<quint32> children;
    createCSAUnion(newPiece, piece2, children, id, blockName, initData, dx, dy, pRotate, angle);
    SaveCSAChildren(initData.doc, id, children);
}

//---------------------------------------------------------------------------------------------------------------------
void createUnionInternalPaths(VPiece &newPiece, const VPiece &d, QVector<quint32> &children, quint32 id,
                                     const QString &blockName, const UnionToolInitData &initData, qreal dx,
                                     qreal dy, quint32 pRotate, qreal angle)
{
    QVector<quint32> nodeChildren;
    for(int i=0; i < d.GetInternalPaths().size(); ++i)
    {
        const VPiecePath path = initData.data->GetPiecePath(d.GetInternalPaths().at(i));
        VPiecePath newPath = path;
        newPath.Clear();//Clear nodes

        for (int i=0; i < path.CountNodes(); ++i)
        {
            AddNodeToNewPath(initData, newPath, path.at(i), id, nodeChildren, blockName, dx, dy, pRotate, angle);
        }
        const quint32 idPath = initData.data->AddPiecePath(newPath);
        VToolInternalPath::Create(idPath, newPath, NULL_ID, initData.scene, initData.doc, initData.data, initData.parse,
                               Source::FromTool, blockName, id);
        newPiece.GetInternalPaths().append(idPath);
        nodeChildren.prepend(idPath);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void CreateUnitedInternalPaths(VPiece &newPiece, const VPiece &piece1, const VPiece &piece2, quint32 id,
                               const QString &blockName, const UnionToolInitData &initData, qreal dx, qreal dy,
                               quint32 pRotate, qreal angle)
{
    for (int i = 0; i < piece1.GetInternalPaths().size(); ++i)
    {
        newPiece.GetInternalPaths().append(piece1.GetInternalPaths().at(i));
    }

    QVector<quint32> children;
    createUnionInternalPaths(newPiece, piece2, children, id, blockName, initData, dx, dy, pRotate, angle);

    SaveInternalPathsChildren(initData.doc, id, children);
}

//---------------------------------------------------------------------------------------------------------------------
void createUnionAnchors(VPiece &newPiece, const VPiece &d, QVector<quint32> &children, quint32 idTool,
                            const QString &blockName, const UnionToolInitData &initData, qreal dx, qreal dy,
                            quint32 pRotate, qreal angle)
{
    QVector<quint32> nodeChildren;
    for(int i=0; i < d.getAnchors().size(); ++i)
    {
        const quint32 id = AddAnchorPoint(d.getAnchors().at(i), initData, idTool, children, blockName, dx, dy, pRotate, angle);
        newPiece.getAnchors().append(id);
        nodeChildren.prepend(id);
    }
    children += nodeChildren;
}

//---------------------------------------------------------------------------------------------------------------------
void createUnitedAnchors(VPiece &newPiece, const VPiece &piece1, const VPiece &piece2, quint32 id, const QString &blockName,
                      const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate, qreal angle)
{
    for (int i = 0; i < piece1.getAnchors().size(); ++i)
    {
        newPiece.getAnchors().append(piece1.getAnchors().at(i));
    }

    QVector<quint32> children;
    createUnionAnchors(newPiece, piece2, children, id, blockName, initData, dx, dy, pRotate, angle);
    SaveAnchorsChildren(initData.doc, id, children);
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateUnitedNodes(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                       qreal angle)
{
    const VPiecePath piece1Path = GetPiece1MainPath(initData.doc, id);
    const VPiecePath piece1REPath = piece1Path.RemoveEdge(initData.piece1_Index);

    const VPiecePath piece2Path = GetPiece2MainPath(initData.doc, id);
    const VPiecePath piece2REPath = piece2Path.RemoveEdge(initData.piece2_Index);

    const qint32 piece1NodeCount = piece1REPath.CountNodes();
    const qint32 piece2NodeCount = piece2REPath.CountNodes();

    QVector<quint32> children = GetNodesChildren(initData.doc, id);
    if (not children.isEmpty())
    {
        // This check need for backward compatibility
        // Remove check and "else" part if min version is 0.3.2
        Q_STATIC_ASSERT_X(VPatternConverter::PatternMinVer < CONVERTER_VERSION_CHECK(0, 3, 2),
                          "Time to refactor the code.");
        if (children.size() == piece1NodeCount + piece2NodeCount-1)
        {
            qint32 piece2Points = 0; //Keeps number points the second piece, what we have already added.
            qint32 i = 0;
            const int indexOfNode = piece1Path.indexOfNode(pRotate);
            do
            {
                UpdatePathNode(initData.data, piece1REPath.at(i), children);
                ++i;
                if (i > indexOfNode && piece2Points < piece2NodeCount-1)
                {
                    qint32 j = 0;
                    FindIndexJ(piece2Points, piece2Path, initData.piece2_Index, j);
                    do
                    {
                        if (j >= piece2NodeCount)
                        {
                            j=0;
                        }
                        UpdatePathNode(initData.data, piece2REPath.at(j), children, dx, dy, pRotate, angle);
                        ++piece2Points;
                        ++j;
                    } while (piece2Points < piece2NodeCount-1);
                }
            } while (i<piece1NodeCount);
        }
        else // remove if min version is 0.3.2
        {
            qint32 piece2Points = 0; //Keeps number points the second piece, what we have already added.
            qint32 i = 0;
            const int indexOfNode = piece1Path.indexOfNode(pRotate);
            do
            {
                ++i;
                if (i > indexOfNode)
                {
                    const int childrenCount = children.size();
                    qint32 j = 0;
                    FindIndexJ(piece2Points, piece2Path, initData.piece2_Index, j);
                    do
                    {
                        if (j >= piece2NodeCount)
                        {
                            j=0;
                        }
                        UpdatePathNode(initData.data, piece2REPath.at(j), children, dx, dy, pRotate, angle);
                        ++piece2Points;
                        ++j;
                    } while (piece2Points < childrenCount);
                    break;
                }
            } while (i<piece1NodeCount);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void createUnionPaths(const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                             qreal angle, const QVector<quint32> &records, QVector<quint32> children)
{
    for (int i=0; i < records.size(); ++i)
    {
        const VPiecePath path = initData.data->GetPiecePath(records.at(i));
        const quint32 updatedId = TakeNextId(children);

        VPiecePath updatedPath(path);
        updatedPath.Clear();

        for (int j=0; j < path.CountNodes(); ++j)
        {
            const VPieceNode &node = path.at(j);
            const quint32 id = TakeNextId(children);
            updatedPath.Append(VPieceNode(id, node.GetTypeTool(), node.GetReverse()));
            QVector<quint32> nodeChildren = {id};
            UpdatePathNode(initData.data, path.at(j), nodeChildren, dx, dy, pRotate, angle);
        }
        initData.data->UpdatePiecePath(updatedId, updatedPath);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void updateUnionCSA(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                           qreal angle, const QVector<CustomSARecord> &records)
{
    QVector<quint32> idRecords;
    for (int i = 0; i < records.size(); ++i)
    {
        idRecords.append(records.at(i).path);
    }
    createUnionPaths(initData, dx, dy, pRotate, angle, idRecords, GetCSAChildren(initData.doc, id));
}

//---------------------------------------------------------------------------------------------------------------------
void updateUnionInternalPaths(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy,
                                     quint32 pRotate, qreal angle, const QVector<quint32> &records)
{
    createUnionPaths(initData, dx, dy, pRotate, angle, records, GetInternalPathsChildren(initData.doc, id));
}

//---------------------------------------------------------------------------------------------------------------------
void updateUnionAnchors(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy,
                            quint32 pRotate, qreal angle, const QVector<quint32> &records)
{
    QVector<quint32> children = getAnchorChildren(initData.doc, id);

    for (int i = 0; i < records.size(); ++i)
    {
        QScopedPointer<VPointF> point(new VPointF(*initData.data->GeometricObject<VPointF>(records.at(i))));
        point->setMode(Draw::Modeling);
        if (not qFuzzyIsNull(dx) || not qFuzzyIsNull(dy) || pRotate != NULL_ID)
        {
            BiasRotatePoint(point.data(), dx, dy,
                            static_cast<QPointF>(*initData.data->GeometricObject<VPointF>(pRotate)), angle);
        }
        initData.data->UpdateGObject(TakeNextId(children), point.take());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void createUnion(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                        qreal angle)
{
    const QString blockName = getBlockName(initData.doc, initData.piece1_Id, initData.piece2_Id);
    SCASSERT(not blockName.isEmpty())

    const VPiece piece1 = initData.data->GetPiece(initData.piece1_Id);
    const VPiece piece2 = initData.data->GetPiece(initData.piece2_Id);

    VPiece newPiece;

    CreateUnitedNodes(newPiece, piece1, piece2, id, blockName, initData, dx, dy, pRotate, angle);
    CreateUnitedCSA(newPiece, piece1, piece2, id, blockName, initData, dx, dy, pRotate, angle);
    CreateUnitedInternalPaths(newPiece, piece1, piece2, id, blockName, initData, dx, dy, pRotate, angle);
    createUnitedAnchors(newPiece, piece1, piece2, id, blockName, initData, dx, dy, pRotate, angle);

    VPiecePath path = newPiece.GetPath();
    const QRectF rect = QPolygonF(path.PathPoints(initData.data)).boundingRect();

    newPiece.SetName(QObject::tr("Union piece"));
    newPiece.SetSeamAllowance(qApp->Settings()->getDefaultSeamAllowanceVisibilty());
    newPiece.GetPatternInfo().SetVisible(qApp->Settings()->showPatternLabels());
    newPiece.GetPatternInfo().SetLabelWidth(QString::number(qApp->Settings()->getDefaultLabelWidth()));
    newPiece.GetPatternInfo().SetLabelHeight(QString::number(qApp->Settings()->getDefaultLabelHeight()));
    qreal width =  newPiece.GetPatternInfo().GetLabelWidth().toDouble();
    qreal height = newPiece.GetPatternInfo().GetLabelHeight().toDouble();
    qreal xPos = rect.left() + (rect.width()/2.0 - width)/2.0;
    qreal yPos = rect.center().y() - height/2.0;
    newPiece.GetPatternInfo().SetPos(QPointF(xPos, yPos));

    newPiece.GetPatternPieceData().SetVisible(qApp->Settings()->showPieceLabels());
    newPiece.GetPatternPieceData().SetLabelWidth(QString::number(qApp->Settings()->getDefaultLabelWidth()));
    newPiece.GetPatternPieceData().SetLabelHeight(QString::number(qApp->Settings()->getDefaultLabelHeight()));
    QString filename = qApp->Settings()->getDefaultPieceTemplate();
    if (QFileInfo(filename).exists())
    {
        VLabelTemplate labelTemplate;
        labelTemplate.setXMLContent(VLabelTemplateConverter(filename).Convert());
        newPiece.GetPatternPieceData().SetLabelTemplate(labelTemplate.ReadLines());
    }

    width = newPiece.GetPatternPieceData().GetLabelWidth().toDouble();
    height = newPiece.GetPatternPieceData().GetLabelHeight().toDouble();
    xPos = rect.center().x() + (rect.width()/2.0 - width)/2.0;
    yPos = rect.center().y() - height/2.0;
    newPiece.GetPatternPieceData().SetPos(QPointF(xPos, yPos));

    newPiece.GetGrainlineGeometry().SetVisible(qApp->Settings()->getDefaultGrainlineVisibilty());
    newPiece.GetGrainlineGeometry().SetLength(QString::number(qApp->Settings()->getDefaultGrainlineLength()));
    qreal length =  newPiece.GetGrainlineGeometry().GetLength().toDouble();
    xPos = rect.center().x();
    yPos = rect.center().y();
    newPiece.GetGrainlineGeometry().SetPos(QPointF(xPos, yPos + length/2.0));

    QString formulaSAWidth = piece1.getSeamAllowanceWidthFormula();
    newPiece.setSeamAllowanceWidthFormula(formulaSAWidth, piece1.GetSAWidth());
    newPiece.SetMx(piece1.GetMx());
    newPiece.SetMy(piece1.GetMy());
    newPiece.SetUnited(true);
    PatternPieceTool::Create(0, newPiece, formulaSAWidth, initData.scene, initData.doc, initData.data,
                               initData.parse, Source::FromTool, blockName);

    auto removeUnionPiece = [initData](quint32 id)
    {
        PatternPieceTool *pieceTool = qobject_cast<PatternPieceTool*>(VAbstractPattern::getTool(id));
        SCASSERT(pieceTool != nullptr);
        bool ask = false;
        pieceTool->Remove(ask);
        // We do not call full parse, so will need more to do more cleaning than usually
        initData.doc->RemoveTool(id);
        initData.data->RemovePiece(id);
    };

    if (!initData.retainPieces && !piece1.isLocked() && !piece2.isLocked())
    {
        removeUnionPiece(initData.piece1_Id);
        removeUnionPiece(initData.piece2_Id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void updateUnion(quint32 id, const UnionToolInitData &initData, qreal dx, qreal dy, quint32 pRotate,
                        qreal angle)
{
    UpdateUnitedNodes(id, initData, dx, dy, pRotate, angle);
    updateUnionCSA(id, initData, dx, dy, pRotate, angle, GetPiece2CSAPaths(initData.doc, id));
    updateUnionInternalPaths(id, initData, dx, dy, pRotate, angle, GetPiece2InternalPaths(initData.doc, id));
    updateUnionAnchors(id, initData, dx, dy, pRotate, angle, GetPiece2Anchors(initData.doc, id));
}

//---------------------------------------------------------------------------------------------------------------------
void unitePieces(quint32 id, const UnionToolInitData &initData)
{
    VPieceNode piece1_Pt1;
    qreal dx = 0;
    qreal dy = 0;
    qreal angle = 0;

    if (initData.typeCreation == Source::FromGui)
    {
        const VPiece piece1 = initData.data->GetPiece(initData.piece1_Id);
        const VPiece piece2 = initData.data->GetPiece(initData.piece2_Id);
        UnionInitParameters(initData, piece1.GetPath(), piece2.GetPath(), piece1_Pt1, dx, dy, angle);
        createUnion(id, initData, dx, dy, piece1_Pt1.GetId(), angle);
    }
    else
    {
        const VPiecePath piece1Path = GetPiece1MainPath(initData.doc, id);
        const VPiecePath piece2Path = GetPiece2MainPath(initData.doc, id);
        UnionInitParameters(initData, piece1Path, piece2Path, piece1_Pt1, dx, dy, angle);
        updateUnion(id, initData, dx, dy, piece1_Pt1.GetId(), angle);
    }
}
} // static functions

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UnionTool costructor.
 * @param id object id in container.
 * @param initData global init data.
 * @param parent parent object.
 */
UnionTool::UnionTool(quint32 id, const UnionToolInitData &initData,
                                     QObject *parent)
    : VAbstractTool(initData.doc, initData.data, id, parent)
    , piece1_Id(initData.piece1_Id)
    , piece2_Id(initData.piece2_Id)
    , piece1_Index(initData.piece1_Index)
    , piece2_Index(initData.piece2_Index)
{
    _referens = 0;
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
QString UnionTool::getTagName() const
{
    return VAbstractPattern::TagTools;
}

//---------------------------------------------------------------------------------------------------------------------
void UnionTool::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
void UnionTool::incrementReferens()
{
    VDataTool::incrementReferens();
    if (_referens == 1)
    {
        const QVector<quint32> objects = GetReferenceObjects();
        for(int i = 0; i < objects.size(); ++i)
        {
            doc->IncrementReferens(objects.at(i));
        }

        QDomElement domElement = doc->elementById(m_id, getTagName());
        if (domElement.isElement())
        {
            doc->SetParametrUsage(domElement, AttrInUse, NodeUsage::InUse);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UnionTool::decrementReferens()
{
    VDataTool::decrementReferens();
    if (_referens == 0)
    {
        const QVector<quint32> objects = GetReferenceObjects();
        for(int i = 0; i < objects.size(); ++i)
        {
            doc->DecrementReferens(objects.at(i));
        }

        QDomElement domElement = doc->elementById(m_id, getTagName());
        if (domElement.isElement())
        {
            doc->SetParametrUsage(domElement, AttrInUse, NodeUsage::NotInUse);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UnionTool::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param doc dom document container.
 * @param data container with variables.
 */
UnionTool* UnionTool::Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                             VAbstractPattern *doc, VContainer *data)
{
    SCASSERT(not dialog.isNull())
    QSharedPointer<UnionDialog> dialogTool = dialog.objectCast<UnionDialog>();
    SCASSERT(not dialogTool.isNull())

    UnionToolInitData initData;
    initData.piece1_Id = dialogTool->getPiece1Id();
    initData.piece2_Id = dialogTool->getPiece2Id();
    initData.piece1_Index = static_cast<quint32>(dialogTool->getPiece1Index());
    initData.piece2_Index = static_cast<quint32>(dialogTool->getPiece2Index());
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.retainPieces = dialogTool->retainPieces();

    qApp->getUndoStack()->beginMacro(tr("union pieces"));
    UnionTool* tool = Create(0, initData);
    qApp->getUndoStack()->endMacro();
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param _id tool id, 0 if tool doesn't exist yet.
 * @param initData contains all init data.
 */
UnionTool* UnionTool::Create(const quint32 _id, const UnionToolInitData &initData)
{
    UnionTool *tool = nullptr;
    quint32 id = _id;
    if (initData.typeCreation == Source::FromGui)
    {
        id = VContainer::getNextId();
    }
    else
    {
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(id, initData.data);
        }
    }

    //First add tool to file
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(id, Tool::Union, initData.doc);
        //Scene doesn't show this tool, so doc will destroy this object.
        tool = new UnionTool(id, initData);
        VAbstractPattern::AddTool(id, tool);
        // Unfortunatelly doc will destroy all objects only in the end, but we should delete them before each FullParse
        initData.doc->AddToolOnRemove(tool);
    }
    unitePieces(id, initData);
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with Information about tool into file.
 */
void UnionTool::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());

    doc->SetAttribute(domElement, VDomDocument::AttrId, m_id);
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrIndexD1, piece1_Index);
    doc->SetAttribute(domElement, AttrIndexD2, piece2_Index);

    addPiece(domElement, data.GetPiece(piece1_Id));
    addPiece(domElement, data.GetPiece(piece2_Id));

    AddToModeling(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief addPiece add piece to xml file.
 * @param domElement tag in xml tree.
 * @param piece piece.
 */
void UnionTool::addPiece(QDomElement &domElement, const VPiece &piece) const
{
    QDomElement element = doc->createElement(TagUnionPiece);

    // nodes
    PatternPieceTool::AddNodes(doc, element, piece);
    //custom seam allowance
    PatternPieceTool::AddCSARecords(doc, element, piece.GetCustomSARecords());
    PatternPieceTool::AddInternalPaths(doc, element, piece.GetInternalPaths());
    PatternPieceTool::addAnchors(doc, element, piece.getAnchors());

    domElement.appendChild(element);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToModeling add tool to xml tree.
 * @param domElement tag in xml tree.
 */
void UnionTool::AddToModeling(const QDomElement &domElement)
{
    const QString blockName = getBlockName(doc, piece1_Id, piece2_Id);
    SCASSERT(not blockName.isEmpty())

    QDomElement modeling = doc->getDraftBlockElement(blockName).firstChildElement(VAbstractPattern::TagModeling);
    if (not modeling.isNull())
    {
        modeling.appendChild(domElement);
    }
    else
    {
        qCCritical(vToolUnion, "Can't find tag %s.", qUtf8Printable(VAbstractPattern::TagModeling));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> UnionTool::GetReferenceObjects() const
{
    QVector<quint32> list;
    const QDomElement tool = doc->elementById(m_id, getTagName());
    if (tool.isNull())
    {
        return list;
    }

    const QStringList parts = QStringList() << VAbstractPattern::TagNodes    /*0*/
                                            << PatternPieceTool::TagCSA      /*1*/
                                            << PatternPieceTool::TagIPaths   /*2*/
                                            << PatternPieceTool::TagAnchors; /*3*/

    const QDomNodeList nodesList = tool.childNodes();
    for (qint32 i = 0; i < nodesList.size(); ++i)
    {
        const QDomElement element = nodesList.at(i).toElement();
        if (not element.isNull() && element.tagName() == UnionTool::TagUnionPiece)
        {
            const QDomNodeList pieceList = element.childNodes();
            for (qint32 j = 0; j < pieceList.size(); ++j)
            {
                const QDomElement element = pieceList.at(j).toElement();
                if (not element.isNull())
                {
                    switch (parts.indexOf(element.tagName()))
                    {
                        case 0://VAbstractPattern::TagNodes
                            list += ReferenceObjects(element, TagNode, AttrIdObject);
                            break;
                        case 1://PatternPieceTool::TagCSA
                        case 2://PatternPieceTool::TagIPaths
                            list += ReferenceObjects(element, PatternPieceTool::TagRecord,
                                                     VAbstractPattern::AttrPath);
                            break;
                        case 3://PatternPieceTool::TagAnchors
                        {
                            const QDomNodeList children = element.childNodes();
                            for (qint32 i = 0; i < children.size(); ++i)
                            {
                                const QDomElement record = children.at(i).toElement();
                                if (not record.isNull() && record.tagName() == PatternPieceTool::TagRecord)
                                {
                                    list.append(record.text().toUInt());
                                }
                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
        }
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> UnionTool::ReferenceObjects(const QDomElement &root, const QString &tag,
                                                     const QString &attribute) const
{
    QVector<quint32> objects;

    const QDomNodeList list = root.childNodes();
    for (qint32 i = 0; i < list.size(); ++i)
    {
        const QDomElement element = list.at(i).toElement();
        if (not element.isNull() && element.tagName() == tag)
        {
            const quint32 id = doc->GetParametrUInt(element, attribute, NULL_ID_STR);
            if (id > NULL_ID)
            {
                objects.append(id);
            }
        }
    }

    return objects;
}

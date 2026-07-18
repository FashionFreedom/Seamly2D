//---------------------------------------------------------------------------------------------------------------------
//  @file   vlayoutpiece.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//  @file   vlayoutdetail.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   2 1, 2015
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

#include "vlayoutpiece.h"

#include <QBrush>
#include <QFlags>
#include <QFont>
#include <QFontMetrics>
#include <QGraphicsPathItem>
#include <QList>
#include <QTransform>
#include <QMessageLogger>
#include <QPainterPath>
#include <QPoint>
#include <QPolygonF>
#include <QTransform>
#include <Qt>
#include <QtDebug>

#include "global.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vmath.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/calculator.h"
#include "../vgeometry/vpointf.h"
#include "vlayoutdef.h"
#include "vlayoutpiece_p.h"
#include "vtextmanager.h"
#include "vgraphicsfillitem.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
QVector<VLayoutPiecePath> convertInternalPaths (const VPiece &piece, const VContainer *pattern, const bool isCut)
{
    SCASSERT(pattern != nullptr)

    QVector<VLayoutPiecePath> paths;
    const QVector<quint32> pathIds = piece.getInternalPaths();
    const QVector<QPointF> cutPath = piece.cutPathPoints(pattern);

    for (int i = 0; i < pathIds.size(); ++i)
    {
        const VPiecePath path = pattern->getPiecePath(pathIds.at(i));
        if (path.getType() == PiecePathType::InternalPath)
        {
            if (isCut && path.isCutPath())
            {
                paths.append(VLayoutPiecePath(path.PathPoints(pattern, cutPath),
                path.getLineColor(), path.getLineType(), path.getLineWeight(), path.isCutPath()));
            }
            else if (!isCut && !path.isCutPath())
            {
                paths.append(VLayoutPiecePath(path.PathPoints(pattern, cutPath),
                path.getLineColor(), path.getLineType(), path.getLineWeight(), path.isCutPath()));
            }
        }
    }
    return paths;
}

//---------------------------------------------------------------------------------------------------------------------
bool findLabelGeometry(const VPatternLabelData &labelData, const VContainer *pattern, qreal &rotationAngle,
                       qreal &labelWidth, qreal &labelHeight, QPointF &pos)
{
    SCASSERT(pattern != nullptr)

    try
    {
        Calculator cal1;
        rotationAngle = cal1.EvalFormula(pattern->DataVariables(), labelData.getRotation());
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return false;
    }

    const quint32 topLeftAnchorPoint = labelData.topLeftAnchorPoint();
    const quint32 bottomRightAnchorPoint = labelData.bottomRightAnchorPoint();

    if (topLeftAnchorPoint != NULL_ID && bottomRightAnchorPoint != NULL_ID)
    {
        try
        {
            const auto topLeftAnchorPointPoint = pattern->GeometricObject<VPointF>(topLeftAnchorPoint);
            const auto bottomRightAnchorPointPoint = pattern->GeometricObject<VPointF>(bottomRightAnchorPoint);

            const QRectF labelRect = QRectF(static_cast<QPointF>(*topLeftAnchorPointPoint),
                                            static_cast<QPointF>(*bottomRightAnchorPointPoint));
            labelWidth = qAbs(labelRect.width());
            labelHeight = qAbs(labelRect.height());

            pos = labelRect.topLeft();

            return true;
        }
        catch(const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        Calculator cal1;
        labelWidth = cal1.EvalFormula(pattern->DataVariables(), labelData.GetLabelWidth());
        labelWidth = ToPixel(labelWidth, *pattern->GetPatternUnit());

        Calculator cal2;
        labelHeight = cal2.EvalFormula(pattern->DataVariables(), labelData.GetLabelHeight());
        labelHeight = ToPixel(labelHeight, *pattern->GetPatternUnit());
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return false;
    }

    const quint32 centerAnchor = labelData.centerAnchorPoint();
    if (centerAnchor != NULL_ID)
    {
        try
        {
            const auto centerAnchorPoint = pattern->GeometricObject<VPointF>(centerAnchor);
            pos = static_cast<QPointF>(*centerAnchorPoint) - QRectF(0, 0, labelWidth, labelHeight).center();
        }
        catch(const VExceptionBadId &)
        {
            pos = labelData.GetPos();
        }
    }
    else
    {
        pos = labelData.GetPos();
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool findGrainlineGeometry(const VGrainlineData& data, const VContainer *pattern, qreal &length, qreal &rotationAngle,
                           qreal &arrowLength, QPointF &pos)
{
    SCASSERT(pattern != nullptr)

    const quint32 topAnchorPoint = data.topAnchorPoint();
    const quint32 bottomAnchorPoint = data.bottomAnchorPoint();

    if (topAnchorPoint != NULL_ID && bottomAnchorPoint != NULL_ID)
    {
        try
        {
            const auto topAnchor_Point = pattern->GeometricObject<VPointF>(topAnchorPoint);
            const auto bottomAnchor_Point = pattern->GeometricObject<VPointF>(bottomAnchorPoint);

            QLineF grainline(static_cast<QPointF>(*bottomAnchor_Point), static_cast<QPointF>(*topAnchor_Point));
            length = grainline.length();
            rotationAngle = grainline.angle();

            Calculator cal3;
            arrowLength = cal3.EvalFormula(pattern->DataVariables(), data.getArrowLength());
            arrowLength = ToPixel(arrowLength, *pattern->GetPatternUnit());

            if (!VFuzzyComparePossibleNulls(rotationAngle, 0))
            {
                grainline.setAngle(0);
            }

            pos = grainline.p1();
            rotationAngle = qDegreesToRadians(rotationAngle);

            return true;
        }
        catch(const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        Calculator cal1;
        rotationAngle = cal1.EvalFormula(pattern->DataVariables(), data.getRotation());
        rotationAngle = qDegreesToRadians(rotationAngle);

        Calculator cal2;
        length = cal2.EvalFormula(pattern->DataVariables(), data.getLength());
        length = ToPixel(length, *pattern->GetPatternUnit());

        Calculator cal3;
        arrowLength = cal3.EvalFormula(pattern->DataVariables(), data.getArrowLength());
        arrowLength = ToPixel(arrowLength, *pattern->GetPatternUnit());
    }
    catch(qmu::QmuParserError &error)
    {
        Q_UNUSED(error);
        return false;
    }

    const quint32 centerAnchor = data.centerAnchorPoint();
    if (centerAnchor != NULL_ID)
    {
        try
        {
            const auto centerAnchorPoint = pattern->GeometricObject<VPointF>(centerAnchor);

            QLineF grainline(centerAnchorPoint->x(), centerAnchorPoint->y(),
                             centerAnchorPoint->x() + length / 2.0, centerAnchorPoint->y());

            grainline.setAngle(qRadiansToDegrees(rotationAngle));
            grainline = QLineF(grainline.p2(), grainline.p1());
            grainline.setLength(length);

            pos = grainline.p2();
        }
        catch(const VExceptionBadId &)
        {
            pos = data.GetPos();
        }
    }
    else
    {
        pos = data.GetPos();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool isItemContained (const QRectF &parentBoundingRect, const QVector<QPointF> &shape, qreal &dX, qreal &dY)
{
    dX = 0;
    dY = 0;
    // single point differences
    bool bInside = true;

    for (int i = 0; i < shape.size(); ++i)
    {
        qreal dPtX = 0;
        qreal dPtY = 0;
        if (!parentBoundingRect.contains(shape.at(i)))
        {
            if (shape.at(i).x() < parentBoundingRect.left())
            {
                dPtX = parentBoundingRect.left() - shape.at(i).x();
            }
            else if (shape.at(i).x() > parentBoundingRect.right())
            {
                dPtX = parentBoundingRect.right() - shape.at(i).x();
            }

            if (shape.at(i).y() < parentBoundingRect.top())
            {
                dPtY = parentBoundingRect.top() - shape.at(i).y();
            }
            else if (shape.at(i).y() > parentBoundingRect.bottom())
            {
                dPtY = parentBoundingRect.bottom() - shape.at(i).y();
            }

            if (fabs(dPtX) > fabs(dX))
            {
                dX = dPtX;
            }

            if (fabs(dPtY) > fabs(dY))
            {
                dY = dPtY;
            }

            bInside = false;
        }
    }
    return bInside;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> correctPosition (const QRectF &parentBoundingRect, QVector<QPointF> points)
{
    qreal dX = 0;
    qreal dY = 0;
    if (!isItemContained (parentBoundingRect, points, dX, dY))
    {
        for (int i =0; i < points.size(); ++i)
        {
            points[i] = QPointF(points.at(i).x() + dX, points.at(i).y() + dY);
        }
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VSAPoint> prepareAllowance (const QVector<QPointF> &points)
{
    QVector<VSAPoint> allowancePoints;
    for(int i = 0; i < points.size(); ++i)
    {
        allowancePoints.append(VSAPoint(points.at(i)));
    }
    return allowancePoints;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief VLayoutPiece::rotatePoint  rotates a point around the center for given angle
/// @param ptCenter center around which the point is rotated
/// @param pt point, which is rotated around the center
/// @param rotationAngle angle of rotation
/// @return position of point pt after rotating it around the center for rotation radians
//---------------------------------------------------------------------------------------------------------------------
QPointF rotatePoint (const QPointF &ptCenter, const QPointF& pt, qreal rotationAngle)
{
    QPointF ptDest;
    QPointF ptRel = pt - ptCenter;
    ptDest.setX(cos(rotationAngle)*ptRel.x() - sin(rotationAngle)*ptRel.y());
    ptDest.setY(sin(rotationAngle)*ptRel.x() + cos(rotationAngle)*ptRel.y());

    return ptDest + ptCenter;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList getLabelText (const QVector<QPointF> &labelShape, const VTextManager &tm)
{
    QStringList text;
    if (labelShape.count() > 2)
    {
        for (int i = 0; i < tm.GetSourceLinesCount(); ++i)
        {
            text.append(tm.GetSourceLine(i).m_text);
        }
    }
    return text;
}
}

//---------------------------------------------------------------------------------------------------------------------

#ifdef Q_COMPILER_RVALUE_REFS
VLayoutPiece &VLayoutPiece::operator=(VLayoutPiece &&piece) noexcept { Swap(piece); return *this; }
#endif

void VLayoutPiece::Swap(VLayoutPiece &piece) noexcept
{ VAbstractPiece::Swap(piece); std::swap(d, piece.d); }

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece::VLayoutPiece()
    : VAbstractPiece()
    , d(new VLayoutPieceData)
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece::VLayoutPiece(const VLayoutPiece &piece)
    : VAbstractPiece(piece)
    , d(piece.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece &VLayoutPiece::operator=(const VLayoutPiece &piece)
{
    if ( &piece == this )
    {
        return *this;
    }
    VAbstractPiece::operator=(piece);
    d = piece.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece::~VLayoutPiece()
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiece VLayoutPiece::Create(const VPiece &piece, const VContainer *pattern)
{
    VLayoutPiece layoutPiece;

    layoutPiece.SetMx(piece.GetMx());
    layoutPiece.SetMy(piece.GetMy());
    layoutPiece.SetCountourPoints(piece.MainPathPoints(pattern), piece.isHideSeamLine());
    layoutPiece.setInternalPaths(convertInternalPaths (piece, pattern, false));
    layoutPiece.setCutoutPaths(convertInternalPaths (piece, pattern, true));
    layoutPiece.setNotches(piece.createNotchLines(pattern));
    layoutPiece.SetName(piece.GetName());
    // Keep the piece letter so the SVG exporter can emit it as the data-letter attribute.
    layoutPiece.setPieceLetter(piece.GetPatternPieceData().GetLetter());

    // Disable SA in exports and layouts
    if (qApp->Settings()->showSeamAllowances())
    {
        layoutPiece.setSeamAllowancePoints(piece.SeamAllowancePoints(pattern),
                                           piece.IsSeamAllowance(),
                                           piece.IsSeamAllowanceBuiltIn());
    };

    // Very important to set main path first!
    if (layoutPiece.createMainPath().isEmpty() && layoutPiece.createAllowancePath().isEmpty())
    {
        throw VException (tr("Piece %1 doesn't have shape.").arg(piece.GetName()));
    }

    // Disable piece labels in exports and layouts
    const VPieceLabelData& pieceLabelData = piece.GetPatternPieceData();
    if (pieceLabelData.IsVisible() & qApp->Settings()->showLabels())
    {
        layoutPiece.setPieceText(piece.GetName(), pieceLabelData, qApp->Settings()->getLabelFont(), pattern);
    }

    // Disable pattern labels in exports and layouts
    const VPatternLabelData& patternLabelData = piece.GetPatternInfo();
    if (patternLabelData.IsVisible() & qApp->Settings()->showLabels())
    {
        VAbstractPattern* pDoc = qApp->getCurrentDocument();
        layoutPiece.setPatternText(pDoc, patternLabelData, qApp->Settings()->getLabelFont(), pattern);
    }

    // Disable grainlines in exports and layouts
    const VGrainlineData& grainlineGeom = piece.GetGrainlineGeometry();
    if (grainlineGeom.IsVisible() & qApp->Settings()->showGrainlines())
    {
        layoutPiece.setGrainline(grainlineGeom, pattern);
    }

    layoutPiece.SetSAWidth(qApp->toPixel(piece.GetSAWidth()));
    layoutPiece.SetForbidFlipping(piece.IsForbidFlipping());

    return layoutPiece;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getPieceLetter returns the piece letter entered in the piece label data.
 * @return piece letter, or an empty string when none was set.
 */
QString VLayoutPiece::getPieceLetter() const
{
    return d->m_pieceLetter;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setPieceLetter stores the piece letter for later use by the SVG exporter (data-letter attribute).
 * @param letter piece letter from VPieceLabelData::GetLetter().
 */
void VLayoutPiece::setPieceLetter(const QString &letter)
{
    d->m_pieceLetter = letter;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
QVector<QPointF> VLayoutPiece::getContourPoints() const
{
    return Map(d->contour);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetCountourPoints(const QVector<QPointF> &points, bool hideMainPath)
{
    d->contour = RemoveDublicates(points, false);
    setHideSeamLine(hideMainPath);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
QVector<QPointF> VLayoutPiece::GetSeamAllowancePoints() const
{
    return Map(d->seamAllowance);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setSeamAllowancePoints(const QVector<QPointF> &points, bool seamAllowance, bool seamAllowanceBuiltIn)
{
    if (seamAllowance)
    {
        SetSeamAllowance(seamAllowance);
        SetSeamAllowanceBuiltIn(seamAllowanceBuiltIn);
        d->seamAllowance = points;
        if (!d->seamAllowance.isEmpty())
        {
            d->seamAllowance = RemoveDublicates(d->seamAllowance, false);
        }
        else if (!IsSeamAllowanceBuiltIn())
        {
            qWarning() << "Seam allowance is empty.";
            SetSeamAllowance(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VLayoutPiece::getLayoutAllowancePoints() const
{
    return Map(d->layoutAllowance);
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VLayoutPiece::getPieceTextPosition() const
{
    if (d->pieceLabel.count() > 2)
    {
        return d->transform.map(d->pieceLabel.first());
    }
    else
    {
        return QPointF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VLayoutPiece::getPieceText() const
{
    return getLabelText (d->pieceLabel, d->m_tmPiece);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setPieceText(const QString& qsName, const VPieceLabelData& data, const QFont &font,
                                const VContainer *pattern)
{
    QPointF ptPos;
    qreal labelWidth = 0;
    qreal labelHeight = 0;
    qreal labelAngle = 0;
    if (!findLabelGeometry(data, pattern, labelAngle, labelWidth, labelHeight, ptPos))
    {
        return;
    }

    QVector<QPointF> v;
    v << ptPos
      << QPointF(ptPos.x() + labelWidth, ptPos.y())
      << QPointF(ptPos.x() + labelWidth, ptPos.y() + labelHeight)
      << QPointF(ptPos.x(), ptPos.y() + labelHeight);

    const qreal rotationAngle = qDegreesToRadians(-labelAngle);
    const QPointF ptCenter(ptPos.x() + labelWidth/2, ptPos.y() + labelHeight/2);

    for (int i = 0; i < v.count(); ++i)
    {
        v[i] = rotatePoint (ptCenter, v.at(i), rotationAngle);
    }

    QScopedPointer<QGraphicsItem> item(getMainPathItem());
    d->pieceLabel = correctPosition (item->boundingRect(), v);

    // generate text
    d->m_tmPiece.setFont(font);
    d->m_tmPiece.SetFontSize(data.getFontSize());
    d->m_tmPiece.Update(qsName, data);

    // this will generate the lines of text
    d->m_tmPiece.SetFontSize(data.getFontSize());
    d->m_tmPiece.FitFontSize(labelWidth, labelHeight);
}

//---------------------------------------------------------------------------------------------------------------------
QPointF VLayoutPiece::getPatternTextPosition() const
{
    if (d->patternInfo.count() > 2)
    {
        return d->transform.map(d->patternInfo.first());
    }
    else
    {
        return QPointF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VLayoutPiece::getPatternText() const
{
    return getLabelText (d->patternInfo, d->m_tmPattern);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setPatternText(VAbstractPattern* pDoc, const VPatternLabelData& data, const QFont &font,
                                  const VContainer *pattern)
{
    QPointF ptPos;
    qreal labelWidth = 0;
    qreal labelHeight = 0;
    qreal labelAngle = 0;
    if (!findLabelGeometry(data, pattern, labelAngle, labelWidth, labelHeight, ptPos))
    {
        return;
    }

    QVector<QPointF> v;
    v << ptPos
      << QPointF(ptPos.x() + labelWidth, ptPos.y())
      << QPointF(ptPos.x() + labelWidth, ptPos.y() + labelHeight)
      << QPointF(ptPos.x(), ptPos.y() + labelHeight);

    const qreal rotationAngle = qDegreesToRadians(-labelAngle);
    const QPointF ptCenter(ptPos.x() + labelWidth/2, ptPos.y() + labelHeight/2);
    for (int i = 0; i < v.count(); ++i)
    {
        v[i] = rotatePoint (ptCenter, v.at(i), rotationAngle);
    }
    QScopedPointer<QGraphicsItem> item(getMainPathItem());
    d->patternInfo = correctPosition (item->boundingRect(), v);

    // Generate text
    d->m_tmPattern.setFont(font);
    d->m_tmPattern.SetFontSize(data.getFontSize());
    d->m_tmPattern.Update(pDoc);

    // generate lines of text
    d->m_tmPattern.SetFontSize(data.getFontSize());
    d->m_tmPattern.FitFontSize(labelWidth, labelHeight);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setGrainline(const VGrainlineData& data, const VContainer* pattern)
{
    SCASSERT(pattern != nullptr)

    QPointF pt1;
    qreal rotationAngle = 0;
    qreal length = 0;
    qreal arrowLength = 0;
    if (!findGrainlineGeometry(data, pattern, length, rotationAngle, arrowLength, pt1))
    {
        return;
    }

    QPointF pt2(pt1.x() + length * qCos(rotationAngle),
                pt1.y() - length * qSin(rotationAngle));

    QVector<QPointF> path;
    path << pt1;
    if (data.getArrowType() != ArrowType::Top)
    {
        path << QPointF(pt1.x() + arrowLength * cos(rotationAngle + M_PI / 9),
                        pt1.y() - arrowLength * sin(rotationAngle + M_PI / 9));

        path << QPointF(pt1.x() + arrowLength * cos(rotationAngle - M_PI / 9),
                        pt1.y() - arrowLength * sin(rotationAngle - M_PI / 9));
    }
    path << pt1 << pt2;

    if (data.getArrowType() != ArrowType::Bottom)
    {
        path << QPointF(pt2.x() + arrowLength * cos(M_PI + rotationAngle + M_PI / 9),
                        pt2.y() - arrowLength * sin(M_PI + rotationAngle + M_PI / 9));

        path << QPointF(pt2.x() + arrowLength * cos(M_PI + rotationAngle - M_PI / 9),
                        pt2.y() - arrowLength * sin(M_PI + rotationAngle - M_PI / 9));
    }
    path << pt2;

    QScopedPointer<QGraphicsItem> item(getMainPathItem());
    d->grainlinePoints = correctPosition (item->boundingRect(), path);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VLayoutPiece::getGrainline() const
{
    return Map(d->grainlinePoints);
}

//---------------------------------------------------------------------------------------------------------------------
QTransform VLayoutPiece::getTransform() const
{
    return d->transform;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setTransform(const QTransform &transform)
{
    d->transform = transform;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutPiece::getLayoutGap() const
{
    return d->layoutWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setLayoutGap(const qreal &value)
{
    d->layoutWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Translate(qreal dx, qreal dy)
{
    QTransform m;
    m.translate(dx, dy);
    d->transform *= m;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Rotate(const QPointF &originPoint, qreal degrees)
{
    QTransform m;
    m.translate(originPoint.x(), originPoint.y());
    m.rotate(-degrees);
    m.translate(-originPoint.x(), -originPoint.y());
    d->transform *= m;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::Mirror(const QLineF &edge)
{
    if (edge.isNull())
    {
        return;
    }

    const QLineF axis = QLineF(edge.x2(), edge.y2(), edge.x2() + 100, edge.y2()); // Ox axis

    const qreal angle = edge.angleTo(axis);
    const QPointF p2 = edge.p2();
    QTransform m;
    m.translate(p2.x(), p2.y());
    m.rotate(-angle);
    m.translate(-p2.x(), -p2.y());
    d->transform *= m;

    m.reset();
    m.translate(p2.x(), p2.y());
    m.scale(m.m11(), m.m22()*-1);
    m.translate(-p2.x(), -p2.y());
    d->transform *= m;

    m.reset();
    m.translate(p2.x(), p2.y());
    m.rotate(-(360-angle));
    m.translate(-p2.x(), -p2.y());
    d->transform *= m;

    d->mirror = !d->mirror;
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutPiece::pieceEdgesCount() const
{
    return piecePath().count();
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutPiece::LayoutEdgesCount() const
{
    return d->layoutAllowance.count();
}

//---------------------------------------------------------------------------------------------------------------------
QLineF VLayoutPiece::pieceEdge(int i) const
{
    return Edge(piecePath(), i);
}

//---------------------------------------------------------------------------------------------------------------------
QLineF VLayoutPiece::LayoutEdge(int i) const
{
    return Edge(d->layoutAllowance, i);
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutPiece::pieceEdgeByPoint(const QPointF &p1) const
{
    return EdgeByPoint(piecePath(), p1);
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutPiece::LayoutEdgeByPoint(const QPointF &p1) const
{
    return EdgeByPoint(d->layoutAllowance, p1);
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VLayoutPiece::pieceBoundingRect() const
{
    QVector<QPointF> points;
    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
    {
        points = GetSeamAllowancePoints();
    }
    else
    {
        points = getContourPoints();
    }

    points.append(points.first());
    return QPolygonF(points).boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VLayoutPiece::LayoutBoundingRect() const
{
    QVector<QPointF> points = getLayoutAllowancePoints();
    points.append(points.first());
    return QPolygonF(points).boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
qreal VLayoutPiece::Diagonal() const
{
    const QRectF rec = LayoutBoundingRect();
    return qSqrt(pow(rec.height(), 2) + pow(rec.width(), 2));
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPiece::isNull() const
{
    if (d->contour.isEmpty() == false && d->layoutWidth > 0)
    {
        if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn() && d->seamAllowance.isEmpty() == false)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qint64 VLayoutPiece::Square() const
{
    if (d->layoutAllowance.isEmpty()) //-V807
    {
        return 0;
    }

    const qreal res = sumTrapezoids(d->layoutAllowance);

    const qint64 sq = qFloor(qAbs(res/2.0));
    return sq;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::SetLayoutAllowancePoints()
{
    if (d->layoutWidth > 0)
    {
        if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
        {
            d->layoutAllowance = Equidistant(prepareAllowance (GetSeamAllowancePoints()), d->layoutWidth);
            if (d->layoutAllowance.isEmpty() == false)
            {
                d->layoutAllowance.removeLast();
            }
        }
        else
        {
            d->layoutAllowance = Equidistant(prepareAllowance (getContourPoints()), d->layoutWidth);
            if (d->layoutAllowance.isEmpty() == false)
            {
                d->layoutAllowance.removeLast();
            }
        }
    }
    else
    {
        d->layoutAllowance.clear();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VLayoutPiece::getNotches() const
{
    return Map(d->notches);
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setNotches(const QVector<QLineF> &notches)
{
    if (IsSeamAllowance())
    {
        d->notches = notches;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QVector<QPointF> > VLayoutPiece::internalPathsForCut (bool cut) const
{
    QVector<QVector<QPointF> > paths;

    for (int i=0;i < d->m_internalPaths.count(); ++i)
    {
        if (d->m_internalPaths.at(i).isCutPath() == cut)
        {
            paths.append(Map(d->m_internalPaths.at(i).Points()));
        }
    }

    return paths;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VLayoutPiecePath> VLayoutPiece::getInternalPaths() const
{
    return d->m_internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setInternalPaths(const QVector<VLayoutPiecePath> &internalPaths)
{
    d->m_internalPaths = internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VLayoutPiecePath> VLayoutPiece::getCutoutPaths() const
{
    return d->m_cutoutPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setCutoutPaths(const QVector<VLayoutPiecePath> &cutoutPaths)
{
    d->m_cutoutPaths = cutoutPaths;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
QVector<T> VLayoutPiece::Map(const QVector<T> &points) const
{
    QVector<T> p;
    for (int i = 0; i < points.size(); ++i)
    {
        p.append(d->transform.map(points.at(i)));
    }

    if (d->mirror)
    {
        QList<T> list = p.toList();
        for (int k=0, s=list.size(), max=(s/2); k<max; k++)
        {
			list.swapItemsAt(k, s-(1+k));
        }
        p = list.toVector();
    }
    return p;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VLayoutPiece::createMainPath() const
{
    QPainterPath path;

    // contour
    QVector<QPointF> points = getContourPoints();

    if (!isHideSeamLine() || not IsSeamAllowance() || IsSeamAllowanceBuiltIn())
    {
        path.moveTo(points.at(0));
        for (qint32 i = 1; i < points.count(); ++i)
        {
            path.lineTo(points.at(i));
        }
        path.lineTo(points.at(0));
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VLayoutPiece::createAllowancePath() const
{
    QPainterPath path;

    // seam allowance
    if (IsSeamAllowance())
    {
        if (!IsSeamAllowanceBuiltIn())
        {
            // Draw seam allowance
            QVector<QPointF>points = GetSeamAllowancePoints();

            if (points.last().toPoint() != points.first().toPoint())
            {
                points.append(points.at(0));// Should be always closed
            }

            QPainterPath ekv;
            ekv.moveTo(points.at(0));
            for (qint32 i = 1; i < points.count(); ++i)
            {
                ekv.lineTo(points.at(i));
            }

            path.addPath(ekv);
        }
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VLayoutPiece::createNotchesPath() const
{
    QPainterPath path;

    const QVector<QLineF> notches = getNotches();
    QPainterPath notchesPath;

    for (qint32 i = 0; i < notches.count(); ++i)
    {
        notchesPath.moveTo(notches.at(i).p1());
        notchesPath.lineTo(notches.at(i).p2());
    }

    path.addPath(notchesPath);
    path.setFillRule(Qt::WindingFill);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createInternalPathItem creates the child item for one internal path of the piece.
 * @param i      index into the internal paths list.
 * @param parent piece root item the path item is added to.
 */
void VLayoutPiece::createInternalPathItem(int i, QGraphicsItem *parent) const
{
    SCASSERT(parent != nullptr)
    QColor       color      = d->m_internalPaths.at(i).getLineColor();
    Qt::PenStyle lineType   = d->m_internalPaths.at(i).getLineType();
    qreal        lineWeight = ToPixel(d->m_internalPaths.at(i).getLineWeight().toDouble(), Unit::Mm);

    QGraphicsPathItem* item = new QGraphicsPathItem(parent);
    item->setData(PieceItemData::ItemType, QStringLiteral("internal_path"));
    item->setPath(d->transform.map(d->m_internalPaths.at(i).GetPainterPath()));
    item->setPen(QPen(color, lineWeight, lineType, Qt::RoundCap, Qt::RoundJoin));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createCutoutPathItem creates the child item for one internal cutout path of the piece.
 * @param i      index into the cutout paths list.
 * @param parent piece root item the path item is added to.
 */
void VLayoutPiece::createCutoutPathItem(int i, QGraphicsItem *parent) const
{
    SCASSERT(parent != nullptr)
    QColor       color      = d->m_cutoutPaths.at(i).getLineColor();
    Qt::PenStyle lineType   = d->m_cutoutPaths.at(i).getLineType();
    qreal        lineWeight = ToPixel(d->m_cutoutPaths.at(i).getLineWeight().toDouble(), Unit::Mm);

    QGraphicsPathItem* item = new QGraphicsPathItem(parent);
    // Cutouts share the internal_path data-type until the SVG spec defines a dedicated one.
    item->setData(PieceItemData::ItemType, QStringLiteral("internal_path"));
    item->setPath(d->transform.map(d->m_cutoutPaths.at(i).GetPainterPath()));
    item->setPen(QPen(color, lineWeight, lineType, Qt::RoundCap, Qt::RoundJoin));
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VLayoutPiece::LayoutAllowancePath() const
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    const QVector<QPointF> points = getLayoutAllowancePoints();
    path.moveTo(points.at(0));
    for (qint32 i = 1; i < points.count(); ++i)
    {
        path.lineTo(points.at(i));
    }
    path.lineTo(points.at(0));

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetItem builds the graphics item tree for one pattern piece.
 *
 * The root item is a pure container: it paints nothing itself. Every visual
 * component (seamline, cutline, notches, internal paths, labels, grainline)
 * is created as a direct child tagged with PieceItemData::ItemType so the SVG
 * exporter can render and identify each component independently.
 *
 * @param textAsPaths true to convert label text into vector paths.
 * @return root QGraphicsItem owning all component child items.
 */
QGraphicsItem *VLayoutPiece::GetItem(bool textAsPaths) const
{
    // Empty QGraphicsPathItem: no path and no pen, so the root draws nothing.
    QGraphicsPathItem *item = new QGraphicsPathItem();
    item->setData(PieceItemData::ObjectName, GetName());
    item->setData(PieceItemData::PieceLetter, getPieceLetter());

    // Child components, one taggable item (or item group) each.
    createSeamlineItem(item);
    createAllowanceItem(item);
    createNotchesItem(item);

    for (int i = 0; i < d->m_internalPaths.count(); ++i)
    {
        createInternalPathItem(i, item);
    }

    for (int i = 0; i < d->m_cutoutPaths.count(); ++i)
    {
        createCutoutPathItem(i, item);
    }

    createLabelItem(item, d->pieceLabel, d->m_tmPiece, QStringLiteral("piece_label"), textAsPaths);
    createLabelItem(item, d->patternInfo, d->m_tmPattern, QStringLiteral("pattern_label"), textAsPaths);
    createGrainlineItem(item, textAsPaths);

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createLabelItem creates the graphics items for one label (piece or pattern label).
 *
 * All text lines of the label are parented to one invisible group item tagged
 * with the given SVG data-type, so the SVG exporter can render the whole label
 * as a single identifiable component.
 *
 * @param parent      piece root item the label group is added to.
 * @param labelShape  label rectangle corners; fewer than 3 points means no label.
 * @param tm          text manager holding the label's text lines.
 * @param type        SVG data-type tag: "piece_label" or "pattern_label".
 * @param textAsPaths true to convert the text into vector paths.
 */
void VLayoutPiece::createLabelItem(QGraphicsItem *parent, const QVector<QPointF> &labelShape,
                                      const VTextManager &tm, const QString &type, bool textAsPaths) const
{
    SCASSERT(parent != nullptr)
    QColor color = QColor(qApp->Settings()->getDefaultLabelColor());

    if (labelShape.count() > 2)
    {
        // Invisible group item: every text line below becomes one SVG label component.
        QGraphicsPathItem *labelGroup = new QGraphicsPathItem(parent);
        labelGroup->setData(PieceItemData::ObjectName, QStringLiteral("label"));
        labelGroup->setData(PieceItemData::ItemType, type);

        const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
        const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
        const qreal angle = - QLineF(labelShape.at(0), labelShape.at(1)).angle();
        qreal dY = 0;
        qreal dX = 0;

        for (int i = 0; i < tm.GetSourceLinesCount(); ++i)
        {
            const TextLine& tl = tm.GetSourceLine(i);
            QFont fnt = tm.GetFont();
            fnt.setPixelSize(tm.GetFont().pixelSize() + tl.m_iFontSize);
            fnt.setBold(tl.bold);
            fnt.setItalic(tl.italic);

            QFontMetrics fm(fnt);

            if (textAsPaths)
            {
                dY += fm.height();
            }

            if (dY > dH)
            {
                break;
            }

            QString qsText = tl.m_text;
            if (fm.horizontalAdvance(qsText) > dW)
            {
                qsText = fm.elidedText(qsText, Qt::ElideMiddle, static_cast<int>(dW));
            }
            if ((tl.m_eAlign & Qt::AlignLeft) > 0)
            {
                dX = 0;
            }
            else if ((tl.m_eAlign & Qt::AlignHCenter) > 0)
            {
                dX = (dW - fm.horizontalAdvance(qsText))/2;
            }
            else
            {
                dX = dW - fm.horizontalAdvance(qsText);
            }

            // set up the rotation around top-left corner matrix
            QTransform labelTransform;
            labelTransform.translate(labelShape.at(0).x(), labelShape.at(0).y());
            if (d->mirror)
            {
                labelTransform.scale(-1, 1);
                labelTransform.rotate(-angle);
                labelTransform.translate(-dW, 0);
                labelTransform.translate(dX, dY); // Each string has own position
            }
            else
            {
                labelTransform.rotate(angle);
                labelTransform.translate(dX, dY); // Each string has own position
            }

            labelTransform *= d->transform;

            if (textAsPaths)
            {
                QPainterPath path;
                path.addText(0, - static_cast<qreal>(fm.ascent())/6., fnt, qsText);

                // Text line as vector path, parented to the label group.
                QGraphicsPathItem* item = new QGraphicsPathItem(labelGroup);
                item->setData(PieceItemData::ObjectName, QString("label"));
                item->setPath(path);
                item->setPen(QPen(color, widthHairLine));
                item->setBrush(QBrush(Qt::NoBrush));
                item->setTransform(labelTransform);

                dY += tm.GetSpacing();
            }
            else
            {
                // Text line as real text, parented to the label group.
                QGraphicsSimpleTextItem* item = new QGraphicsSimpleTextItem(labelGroup);
                item->setData(PieceItemData::ObjectName, QString("label"));
                item->setFont(fnt);
                item->setText(qsText);
                item->setTransform(labelTransform);
                item->setPen(QPen(color));
                item->setBrush(QBrush(color));

                dY += (fm.height() + tm.GetSpacing());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createGrainlineItem creates the grainline child item of the piece.
 * @param parent      piece root item the grainline item is added to.
 * @param textAsPaths true when the export renders text as vector paths.
 */
void VLayoutPiece::createGrainlineItem(QGraphicsItem *parent, bool textAsPaths) const
{
    SCASSERT(parent != nullptr)
    QColor color = QColor(qApp->Settings()->getDefaultGrainlineColor());

    if (d->grainlinePoints.count() < 2)
    {
        return;
    }
    VGraphicsFillItem* item = new VGraphicsFillItem(color, textAsPaths, parent);
    item->setData(PieceItemData::ObjectName, QString("grainline"));
    item->setData(PieceItemData::ItemType, QStringLiteral("grainline"));

    QPainterPath path;

    QVector<QPointF> gPoints = getGrainline();
    path.moveTo(gPoints.at(0));
    for (int i = 1; i < gPoints.count(); ++i)
    {
        path.lineTo(gPoints.at(i));
    }
    item->setPath(path);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VLayoutPiece::piecePath() const
{
    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
    {
        return d->seamAllowance;
    }
    else
    {
        return d->contour;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createSeamlineItem creates the seamline (main path) child item of a piece.
 *
 * Replaces the former createMainItem(): the seamline is now an ordinary child
 * of the piece container so the SVG exporter can render it independently.
 *
 * @param parent piece root item the seamline item is added to.
 */
void VLayoutPiece::createSeamlineItem(QGraphicsItem *parent) const
{
    SCASSERT(parent != nullptr)
    QColor  color;
    QString lineType;
    qreal   lineWeight;
    if (IsSeamAllowance() && !IsSeamAllowanceBuiltIn())
    {
        color      = QColor(qApp->Settings()->getDefaultSeamColor());
        lineType   = qApp->Settings()->getDefaultSeamLinetype();
        lineWeight = ToPixel(qApp->Settings()->getDefaultSeamLineweight(), Unit::Mm);
    }
    else
    {
        color      = QColor(qApp->Settings()->getDefaultCutColor());
        lineType   = qApp->Settings()->getDefaultCutLinetype();
        lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);
    }
    QGraphicsPathItem *item = new QGraphicsPathItem(parent);
    item->setData(PieceItemData::ObjectName, QString("seamline"));
    item->setData(PieceItemData::ItemType, QStringLiteral("seamline"));
    item->setPath(createMainPath());
    item->setPen(QPen(color, lineWeight, lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createAllowanceItem creates the seam allowance (cutline) child item of the piece.
 * @param parent piece root item the cutline item is added to.
 */
void VLayoutPiece::createAllowanceItem(QGraphicsItem *parent) const
{
    QColor  color      = QColor(qApp->Settings()->getDefaultCutColor());
    QString lineType   = qApp->Settings()->getDefaultCutLinetype();
    qreal   lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);

    QGraphicsPathItem *item = new QGraphicsPathItem(parent);
    item->setData(PieceItemData::ObjectName, QString("cutline"));
    item->setData(PieceItemData::ItemType, QStringLiteral("cutline"));
    item->setPath(createAllowancePath());
    item->setPen(QPen(color, lineWeight, lineTypeToPenStyle(lineType), Qt::RoundCap, Qt::RoundJoin));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief createNotchesItem creates the child item holding all notches of the piece.
 * @param parent piece root item the notches item is added to.
 */
void VLayoutPiece::createNotchesItem(QGraphicsItem *parent) const
{
    QColor color      = QColor(qApp->Settings()->getDefaultNotchColor());
    qreal  lineWeight = ToPixel(qApp->Settings()->getDefaultCutLineweight(), Unit::Mm);

    QGraphicsPathItem *item = new QGraphicsPathItem(parent);
    item->setData(PieceItemData::ObjectName, QString("notches"));
    item->setData(PieceItemData::ItemType, QStringLiteral("notch"));
    item->setPath(createNotchesPath());
    item->setPen(QPen(color, lineWeight, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
}

//---------------------------------------------------------------------------------------------------------------------
QGraphicsPathItem *VLayoutPiece::getMainPathItem() const
{
    QGraphicsPathItem *item = new QGraphicsPathItem();

    QPainterPath path;

    // contour
    QVector<QPointF> points = getContourPoints();

    path.moveTo(points.at(0));
    for (qint32 i = 1; i < points.count(); ++i)
    {
        path.lineTo(points.at(i));
    }
    path.lineTo(points.at(0));

    item->setPath(path);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
bool VLayoutPiece::isMirror() const
{
    return d->mirror;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiece::setMirror (bool value)
{
    d->mirror = value;
}

//---------------------------------------------------------------------------------------------------------------------
QLineF VLayoutPiece::Edge(const QVector<QPointF> &path, int i) const
{
    if (i < 1 || i > path.count())
    { // Doesn't exist such edge
        return QLineF();
    }

    int i1, i2;
    if (i < path.count())
    {
        i1 = i-1;
        i2 = i;
    }
    else
    {
        i1 = path.count()-1;
        i2 = 0;
    }

    if (d->mirror)
    {
        const int oldI1 = i1;
        const int size = path.size()-1; //-V807
        i1 = size - i2;
        i2 = size - oldI1;
        return QLineF(d->transform.map(path.at(i2)), d->transform.map(path.at(i1)));
    }
    else
    {
        return QLineF(d->transform.map(path.at(i1)), d->transform.map(path.at(i2)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VLayoutPiece::EdgeByPoint(const QVector<QPointF> &path, const QPointF &p1) const
{
    if (p1.isNull())
    {
        return 0;
    }

    if (path.count() < 3)
    {
        return 0;
    }

    const QVector<QPointF> points = Map(path);
    for (int i=0; i < points.size(); i++)
    {
        if (points.at(i) == p1)
        {
            return i+1;
        }
    }
    return 0; // Did not find edge
}

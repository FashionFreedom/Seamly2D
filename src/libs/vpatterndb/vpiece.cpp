/***************************************************************************
 **  @file   vpiece.cpp
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
 **  @file   vpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vpiece.h"
#include "vpiece_p.h"
#include "vcontainer.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vmisc/vabstractapplication.h"

#include <QSharedPointer>
#include <QDebug>
#include <QPainterPath>

namespace
{
QVector<quint32> PieceMissingNodes(const QVector<quint32> &d1Nodes, const QVector<quint32> &d2Nodes)
{
    if (d1Nodes.size() == d2Nodes.size()) //-V807
    {
        return QVector<quint32>();
    }

    QSet<quint32> set1;
    for (qint32 i = 0; i < d1Nodes.size(); ++i)
    {
        set1.insert(d1Nodes.at(i));
    }

    QSet<quint32> set2;
    for (qint32 j = 0; j < d2Nodes.size(); ++j)
    {
        set2.insert(d2Nodes.at(j));
    }

	const QList<quint32> set3 = set1.subtract(set2).values();
    QVector<quint32> r;
    for (qint32 i = 0; i < set3.size(); ++i)
    {
        r.append(set3.at(i));
    }

    return r;
}

//---------------------------------------------------------------------------------------------------------------------
bool notchesPossible(const QVector<VPieceNode> &path)
{
    int countPointNodes = 0;
    int countOthers = 0;

    for (int i = 0; i< path.size(); ++i)
    {
        const VPieceNode &node = path.at(i);
        if (node.isExcluded())
        {
            continue;// skip node
        }

        node.GetTypeTool() == Tool::NodePoint ? ++countPointNodes : ++countOthers;
    }

    return countPointNodes >= 3 || (countPointNodes >= 1 && countOthers >= 1);
}
}

//---------------------------------------------------------------------------------------------------------------------

#ifdef Q_COMPILER_RVALUE_REFS
VPiece &VPiece::operator=(VPiece &&piece) Q_DECL_NOTHROW
{ Swap(piece); return *this; }
#endif

void VPiece::Swap(VPiece &piece) Q_DECL_NOTHROW
{ VAbstractPiece::Swap(piece); std::swap(d, piece.d); }

//---------------------------------------------------------------------------------------------------------------------
VPiece::VPiece()
    : VAbstractPiece()
    , d(new VPieceData(PiecePathType::PiecePath))
{}

//---------------------------------------------------------------------------------------------------------------------
VPiece::VPiece(const VPiece &piece)
    : VAbstractPiece(piece)
    , d(piece.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VPiece &VPiece::operator=(const VPiece &piece)
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
VPiece::~VPiece()
{}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath VPiece::GetPath() const
{
    return d->m_path;
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath &VPiece::GetPath()
{
    return d->m_path;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetPath(const VPiecePath &path)
{
    d->m_path = path;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPiece::MainPathPoints(const VContainer *data) const
{
    QVector<QPointF> points = GetPath().PathPoints(data);
    points = CheckLoops(CorrectEquidistantPoints(points));//A path can contains loops
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VPointF> VPiece::MainPathNodePoints(const VContainer *data, bool showExcluded) const
{
    return GetPath().PathNodePoints(data, showExcluded);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QPointF> VPiece::SeamAllowancePoints(const VContainer *data) const
{
    SCASSERT(data != nullptr);


    if (not IsSeamAllowance() || IsSeamAllowanceBuiltIn())
    {
        return QVector<QPointF>();
    }

    const QVector<CustomSARecord> records = FilterRecords(GetValidRecords());
    int recordIndex = -1;
    bool insertingCSA = false;
    const qreal width = ToPixel(GetSAWidth(), *data->GetPatternUnit());
    const QVector<VPieceNode> unitedPath = GetUnitedPath(data);

    QVector<VSAPoint> pointsEkv;
    for (int i = 0; i< unitedPath.size(); ++i)
    {
        const VPieceNode &node = unitedPath.at(i);
        if (node.isExcluded())
        {
            continue;// skip excluded node
        }

        switch (node.GetTypeTool())
        {
            case (Tool::NodePoint):
            {
                if (not insertingCSA)
                {
                    pointsEkv.append(VPiecePath::PreparePointEkv(node, data));

                    recordIndex = IsCSAStart(records, node.GetId());
                    if (recordIndex != -1 && records.at(recordIndex).includeType == PiecePathIncludeType::AsCustomSA)
                    {
                        insertingCSA = true;

                        const VPiecePath path = data->GetPiecePath(records.at(recordIndex).path);
                        QVector<VSAPoint> r = path.SeamAllowancePoints(data, width, records.at(recordIndex).reverse);

                        for (int j = 0; j < r.size(); ++j)
                        {
                            r[j].SetAngleType(PieceNodeAngle::ByLength);
                            r[j].SetSABefore(0);
                            r[j].SetSAAfter(0);
                        }

                        pointsEkv += r;
                    }
                }
                else
                {
                    if (records.at(recordIndex).endPoint == node.GetId())
                    {
                        insertingCSA = false;
                        recordIndex = -1;

                        pointsEkv.append(VPiecePath::PreparePointEkv(node, data));
                    }
                }
            }
            break;
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                if (not insertingCSA)
                {
                    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());

                    pointsEkv += VPiecePath::CurveSeamAllowanceSegment(data, unitedPath, curve, i, node.GetReverse(),
                                                                       width);
                }
            }
            break;
            default:
                qDebug()<<"Get wrong tool type. Ignore."<< static_cast<char>(node.GetTypeTool());
                break;
        }
    }

    return Equidistant(pointsEkv, width);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createNotchLines(const VContainer *data, const QVector<QPointF> &seamAllowance) const
{
    const QVector<VPieceNode> unitedPath = GetUnitedPath(data);
    if (not notchesPossible(unitedPath))
    {
        return QVector<QLineF>();
    }

    QVector<QLineF> notches;

    for (int i = 0; i< unitedPath.size(); ++i)
    {
        const VPieceNode &node = unitedPath.at(i);
        if (node.isExcluded() || not node.isNotch())
        {
            continue;// skip node
        }

        const int previousIndex = VPiecePath::FindInLoopNotExcludedUp(i, unitedPath);
        const int nextIndex = VPiecePath::FindInLoopNotExcludedDown(i, unitedPath);

        notches += createNotch(unitedPath, previousIndex, i, nextIndex, data, seamAllowance);
    }

    return notches;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPiece::MainPathPath(const VContainer *data) const
{
    const QVector<QPointF> points = MainPathPoints(data);
    QPainterPath path;

    if (not points.isEmpty())
    {
        path.moveTo(points[0]);
        for (qint32 i = 1; i < points.count(); ++i)
        {
            path.lineTo(points.at(i));
        }
        path.lineTo(points.at(0));
        path.setFillRule(Qt::WindingFill);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPiece::SeamAllowancePath(const VContainer *data) const
{
    return SeamAllowancePath(SeamAllowancePoints(data));
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPiece::SeamAllowancePath(const QVector<QPointF> &points) const
{
    QPainterPath ekv;

    // seam allowence
    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn())
    {
        if (not points.isEmpty())
        {
            ekv.moveTo(points.at(0));
            for (qint32 i = 1; i < points.count(); ++i)
            {
                ekv.lineTo(points.at(i));
            }

            ekv.setFillRule(Qt::WindingFill);
        }
    }

    return ekv;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPiece::getNotchesPath(const VContainer *data, const QVector<QPointF> &pathPoints) const
{
    const QVector<QLineF> notches = createNotchLines(data, pathPoints);
    QPainterPath path;

    // seam allowence
    if (IsSeamAllowance())
    {
        if (not notches.isEmpty())
        {
            for (qint32 i = 0; i < notches.count(); ++i)
            {
                path.moveTo(notches.at(i).p1());
                path.lineTo(notches.at(i).p2());
            }

            path.setFillRule(Qt::WindingFill);
        }
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::isInLayout() const
{
    return d->m_inLayout;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetInLayout(bool inLayout)
{
    d->m_inLayout = inLayout;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::isLocked() const
{
    return d->m_isLocked;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::setIsLocked(bool isLocked)
{
    d->m_isLocked = isLocked;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::IsUnited() const
{
    return d->m_united;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetUnited(bool united)
{
    d->m_united = united;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPiece::getSeamAllowanceWidthFormula() const
{
    return d->m_formulaWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::setSeamAllowanceWidthFormula(const QString &formula, qreal value)
{
    SetSAWidth(value);
    const qreal width = GetSAWidth();
    width >= 0 ? d->m_formulaWidth = formula : d->m_formulaWidth = QLatin1String("0");
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::GetInternalPaths() const
{
    return d->m_internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> &VPiece::GetInternalPaths()
{
    return d->m_internalPaths;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetInternalPaths(const QVector<quint32> &iPaths)
{
    d->m_internalPaths = iPaths;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> VPiece::GetCustomSARecords() const
{
    return d->m_customSARecords;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> &VPiece::GetCustomSARecords()
{
    return d->m_customSARecords;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetCustomSARecords(const QVector<CustomSARecord> &records)
{
    d->m_customSARecords = records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::getAnchors() const
{
    return d->m_anchors;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> &VPiece::getAnchors()
{
    return d->m_anchors;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::setAnchors(const QVector<quint32> &anchors)
{
    d->m_anchors = anchors;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MissingNodes find missing nodes in piece. When we deleted object in piece and return this piece need
 * understand, what nodes need make invisible.
 * @param piece changed piece.
 * @return  list with missing nodes.
 */
QVector<quint32> VPiece::MissingNodes(const VPiece &piece) const
{
    return d->m_path.MissingNodes(piece.GetPath());
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::MissingCSAPath(const VPiece &piece) const
{
    QVector<quint32> oldCSARecords;
    for (qint32 i = 0; i < d->m_customSARecords.size(); ++i)
    {
        oldCSARecords.append(d->m_customSARecords.at(i).path);
    }

    QVector<quint32> newCSARecords;
    for (qint32 i = 0; i < piece.GetCustomSARecords().size(); ++i)
    {
        newCSARecords.append(piece.GetCustomSARecords().at(i).path);
    }

    return PieceMissingNodes(oldCSARecords, newCSARecords);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::MissingInternalPaths(const VPiece &piece) const
{
    return PieceMissingNodes(d->m_internalPaths, piece.GetInternalPaths());
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::missingAnchors(const VPiece &piece) const
{
    return PieceMissingNodes(d->m_anchors, piece.getAnchors());
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetPatternPieceData(const VPieceLabelData &data)
{
    d->m_ppData = data;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns full access to the pattern piece data object
 * @return pattern piece data object
 */
VPieceLabelData &VPiece::GetPatternPieceData()
{
    return d->m_ppData;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the read only reference to the pattern piece data object
 * @return pattern piece data object
 */
const VPieceLabelData &VPiece::GetPatternPieceData() const
{
    return d->m_ppData;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetPatternInfo(const VPatternLabelData &info)
{
    d->m_piPatternInfo = info;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns full access to the pattern info geometry object
 * @return pattern info geometry object
 */
VPatternLabelData &VPiece::GetPatternInfo()
{
    return d->m_piPatternInfo;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Returns the read only reference to the pattern info geometry object
 * @return pattern info geometry object
 */
const VPatternLabelData &VPiece::GetPatternInfo() const
{
    return d->m_piPatternInfo;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VDetail::GetGrainlineGeometry full access to the grainline geometry object
 * @return reference to grainline geometry object
 */
VGrainlineData &VPiece::GetGrainlineGeometry()
{
    return d->m_glGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VDetail::GetGrainlineGeometry returns the read-only reference to the grainline geometry object
 * @return reference to grainline geometry object
 */
const VGrainlineData &VPiece::GetGrainlineGeometry() const
{
    return d->m_glGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VPieceNode> VPiece::GetUnitedPath(const VContainer *data) const
{
    SCASSERT(data != nullptr)

    QVector<VPieceNode> united = d->m_path.GetNodes();

    if (IsSeamAllowance() && IsSeamAllowanceBuiltIn())
    {
        return united;
    }

    const QVector<CustomSARecord> records = FilterRecords(GetValidRecords());

    for (int i = 0; i < records.size(); ++i)
    {
        if (records.at(i).includeType == PiecePathIncludeType::AsMainPath)
        {
            const int indexStartPoint = VPiecePath::indexOfNode(united, records.at(i).startPoint);
            const int indexEndPoint = VPiecePath::indexOfNode(united, records.at(i).endPoint);

            if (indexStartPoint == -1 || indexEndPoint == -1)
            {
                continue;
            }

            const QVector<VPieceNode> midBefore = united.mid(0, indexStartPoint+1);
            const QVector<VPieceNode> midAfter = united.mid(indexEndPoint);

            QVector<VPieceNode> customNodes = data->GetPiecePath(records.at(i).path).GetNodes();
            if (records.at(i).reverse)
            {
                customNodes = VGObject::GetReversePoints(customNodes);
            }

            for (int j = 0; j < customNodes.size(); ++j)
            {
                // Additionally reverse all curves
                if (records.at(i).reverse)
                {
                    // don't make a check because node point will ignore the change
                    customNodes[j].SetReverse(not customNodes.at(j).GetReverse());
                }

                // If seam allowance is built in main path user will not see a notch provided by piece path
                if (IsSeamAllowanceBuiltIn())
                {
                    customNodes[j].setNotch(false);
                }
                else
                {
                    customNodes[j].SetMainPathNode(false);
                }
            }

            united = midBefore + customNodes + midAfter;
        }
    }
    return united;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> VPiece::GetValidRecords() const
{
    QVector<CustomSARecord> records;
    for (int i = 0; i < d->m_customSARecords.size(); ++i)
    {
        const CustomSARecord &record = d->m_customSARecords.at(i);
        const int indexStartPoint = d->m_path.indexOfNode(record.startPoint);
        const int indexEndPoint = d->m_path.indexOfNode(record.endPoint);

        if (record.startPoint > NULL_ID
                && record.path > NULL_ID
                && record.endPoint > NULL_ID
                && indexStartPoint != -1
                && not d->m_path.at(indexStartPoint).isExcluded()
                && indexEndPoint != -1
                && not d->m_path.at(indexEndPoint).isExcluded()
                && indexStartPoint < indexEndPoint)
        {
            records.append(record);
        }
    }
    return records;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<CustomSARecord> VPiece::FilterRecords(QVector<CustomSARecord> records) const
{
    if (records.size() < 2)
    {
        return records;
    }

    // cppcheck-suppress variableScope
    bool foundFilter = false;// Need in case "filter" will stay empty.
    CustomSARecord filter;
    int startIndex = d->m_path.CountNodes()-1;

    for (int i = 0; i < records.size(); ++i)
    {
        const int indexStartPoint = d->m_path.indexOfNode(records.at(i).startPoint);
        if (indexStartPoint < startIndex)
        {
            startIndex = i;
            filter = records.at(i);
            // cppcheck-suppress unreadVariable
            foundFilter = true;
        }
    }

    if (not foundFilter)
    {
        return records; // return as is
    }

    records.remove(startIndex);

    QVector<CustomSARecord> secondRound;
    for (int i = 0; i < records.size(); ++i)
    {
        const int indexStartPoint = d->m_path.indexOfNode(records.at(i).startPoint);
        const int indexEndPoint = d->m_path.indexOfNode(filter.endPoint);
        if (indexStartPoint > indexEndPoint)
        {
            secondRound.append(records.at(i));
        }
    }

    QVector<CustomSARecord> filtered;
    filtered.append(filter);

    filtered += FilterRecords(secondRound);

    return filtered;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<VSAPoint> VPiece::getNodeSAPoints(const QVector<VPieceNode> &path, int index, const VContainer *data) const
{
    SCASSERT(data != nullptr)

    if (index < 0 || index >= path.size())
    {
        return QVector<VSAPoint>();
    }

    const VPieceNode &node = path.at(index);
    QVector<VSAPoint> points;

    if (node.GetTypeTool() == Tool::NodePoint)
    {
        points.append(VPiecePath::PreparePointEkv(node, data));
    }
    else
    {
        const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());
        const qreal width = ToPixel(GetSAWidth(), *data->GetPatternUnit());

        points += VPiecePath::CurveSeamAllowanceSegment(data, path, curve, index, node.GetReverse(), width);
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::getNotchSAPoint(const QVector<VPieceNode> &path, int index, const VContainer *data,
                             VSAPoint &point) const
{
    SCASSERT(data != nullptr)

    const QVector<VSAPoint> points = getNodeSAPoints(path, index, data);

    if (points.isEmpty() || points.size() > 1)
    {
        return false;
    }

    point = points.first();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::getNotchPreviousSAPoints(const QVector<VPieceNode> &path, int index, const VSAPoint &notchSAPoint,
                                      const VContainer *data, VSAPoint &point) const
{
    SCASSERT(data != nullptr)

    const QVector<VSAPoint> points = getNodeSAPoints(path, index, data);

    if (points.isEmpty())
    {
        return false; // Something wrong
    }

    bool found = false;
    int nodeIndex = points.size()-1;
    do
    {
        const VSAPoint previous = points.at(nodeIndex);
        if (notchSAPoint.toPoint() != previous.toPoint())
        {
            point = previous;
            found = true;
        }
        --nodeIndex;
    } while (nodeIndex >= 0 && not found);

    if (not found)
    {
        return false; // Something wrong
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int VPiece::getNextNotchSAPoints(const QVector<VPieceNode> &path, int index, const VSAPoint &notchSAPoint,
                                 const VContainer *data, VSAPoint &point) const
{
    SCASSERT(data != nullptr)

    const QVector<VSAPoint> points = getNodeSAPoints(path, index, data);

    if (points.isEmpty())
    {
        return false; // Something wrong
    }

    bool found = false;
    int nodeIndex = 0;
    do
    {
        const VSAPoint next = points.at(nodeIndex);
        if (notchSAPoint.toPoint() != next.toPoint())
        {
            point = next;
            found = true;
        }
        ++nodeIndex;

    } while (nodeIndex < points.size() && not found);

    if (not found)
    {
        return false; // Something wrong
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::getSeamNotchSAPoint(const VSAPoint &previousSAPoint, const VSAPoint &notchSAPoint,
                                    const VSAPoint &nextSAPoint, const VContainer *data, QPointF &point) const
{
    SCASSERT(data != nullptr)

    QVector<QPointF> ekvPoints;
    const qreal width = ToPixel(GetSAWidth(), *data->GetPatternUnit());

    /* Because method VAbstractPiece::EkvPoint has troubles with edges on the same line we should specially treat such
       cases.
       First check if two edges and seam allowance create parallel lines.
       Second case check if two edges are on a same line geometrically and a notch point has equal SA width.*/
    if (IsEkvPointOnLine(notchSAPoint, previousSAPoint, nextSAPoint)// see issue #665
        || (IsEkvPointOnLine(static_cast<QPointF>(notchSAPoint), static_cast<QPointF>(previousSAPoint),
                             static_cast<QPointF>(nextSAPoint))
            && qAbs(notchSAPoint.GetSABefore(width)
                    - notchSAPoint.GetSAAfter(width)) < VGObject::accuracyPointOnLine))
    {
        QLineF line (notchSAPoint, nextSAPoint);
        line.setAngle(line.angle() + 90);
        line.setLength(VAbstractPiece::MaxLocalSA(notchSAPoint, width));
        ekvPoints.append(line.p2());
    }
    else
    {
        ekvPoints = EkvPoint(previousSAPoint, notchSAPoint, nextSAPoint, notchSAPoint, width);
    }

    if (ekvPoints.isEmpty())
    { // Just in case
        return false; // Something wrong
    }

    if (ekvPoints.size() == 1 || ekvPoints.size() > 2)
    {
        point = ekvPoints.first();
    }
    else if (ekvPoints.size() == 2)
    {
        QLineF line = QLineF(ekvPoints.at(0), ekvPoints.at(1));
        line.setLength(line.length()/2.);
        point = line.p2();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPiece::isNotchVisible(const QVector<VPieceNode> &path, int notchIndex) const
{
    if (notchIndex < 0 || notchIndex >= path.size())
    {
        return false;
    }

    const VPieceNode &node = path.at(notchIndex);
    if (node.GetTypeTool() != Tool::NodePoint || not node.isNotch() || node.isExcluded())
    {
        return false;
    }

    if (IsSeamAllowance() && IsSeamAllowanceBuiltIn())
    {
        return true;
    }

    const QVector<CustomSARecord> records = FilterRecords(GetValidRecords());
    if (records.isEmpty())
    {
        return true;
    }

    for (int i = 0; i < records.size(); ++i)
    {
        if (records.at(i).includeType == PiecePathIncludeType::AsCustomSA)
        {
            const int indexStartPoint = VPiecePath::indexOfNode(path, records.at(i).startPoint);
            const int indexEndPoint = VPiecePath::indexOfNode(path, records.at(i).endPoint);
            if (notchIndex > indexStartPoint && notchIndex < indexEndPoint)
            {
                return false;
            }
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createNotch(const QVector<VPieceNode> &path, int previousIndex, int notchIndex,
                                       int nextIndex, const VContainer *data,
                                       const QVector<QPointF> &pathPoints) const
{
    SCASSERT(data != nullptr);

    if (not isNotchVisible(path, notchIndex))
    {
        return QVector<QLineF>();
    }

    VSAPoint notchSAPoint;
    if (not getNotchSAPoint(path, notchIndex, data, notchSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    VSAPoint previousSAPoint;
    if (not getNotchPreviousSAPoints(path, previousIndex, notchSAPoint, data, previousSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    VSAPoint nextSAPoint;
    if (not getNextNotchSAPoints(path, nextIndex, notchSAPoint, data, nextSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    const QVector<QPointF> mainPathPoints = MainPathPoints(data);
    if (not IsSeamAllowanceBuiltIn())
    {
        QVector<QLineF> lines;
        if (path.at(notchIndex).showNotch())
        {
            lines += createSeamAllowanceNotch(path, previousSAPoint, notchSAPoint,  nextSAPoint,
                                              data, notchIndex, pathPoints);
        }
        if (not isHideSeamLine()
                && path.at(notchIndex).IsMainPathNode()
                && path.at(notchIndex).getNotchSubType() != NotchSubType::Intersection
                && path.at(notchIndex).showSeamlineNotch())
        {
            lines += createBuiltInSaNotch(path, previousSAPoint, notchSAPoint, nextSAPoint, data,
                                          notchIndex, mainPathPoints);
        }
        return lines;
    }
    else
    {
        return createBuiltInSaNotch(path, previousSAPoint, notchSAPoint, nextSAPoint, data, notchIndex, mainPathPoints);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createSeamAllowanceNotch(const QVector<VPieceNode> &path, VSAPoint &previousSAPoint,
                                                 const VSAPoint &notchSAPoint, VSAPoint &nextSAPoint,
                                                 const VContainer *data, int notchIndex,
                                                 const QVector<QPointF> &pathPoints) const
{
    QPointF seamNotchSAPoint;
    if (not getSeamNotchSAPoint(previousSAPoint, notchSAPoint, nextSAPoint, data, seamNotchSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    QVector<QLineF> notches;

    const VPieceNode &node = path.at(notchIndex);

    NotchData notchData;
    notchData.type    = node.getNotchType();
    notchData.subType = node.getNotchSubType();
    notchData.length  = ToPixel(node.getNotchLength(), *data->GetPatternUnit());
    notchData.width   = ToPixel(node.getNotchWidth(),  *data->GetPatternUnit());
    notchData.angle   = node.getNotchAngle();
    notchData.count   = node.getNotchCount();


    if (node.getNotchSubType() == NotchSubType::Straightforward)
    {
        QLineF line = QLineF(seamNotchSAPoint, notchSAPoint);
        line.setLength(notchData.length);
        notchData.line  = line;
        notches += createNotches(notchData, pathPoints);
    }
    else if (node.getNotchSubType() == NotchSubType::Bisector)
    {
        const QLineF bigLine1 = createParallelLine(previousSAPoint, notchSAPoint, notchData.width );
        const QLineF bigLine2 = createParallelLine(notchSAPoint, nextSAPoint, notchData.width );

        QLineF edge1 = QLineF(seamNotchSAPoint, bigLine1.p1());
        QLineF edge2 = QLineF(seamNotchSAPoint, bigLine2.p2());

        edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
        edge1.setLength(notchData.length);

        notchData.line  = edge1;
        notches += createNotches(notchData, pathPoints);
    }
    else if (node.getNotchSubType() == NotchSubType::Intersection)
    {
        QVector<QPointF> seamPoints;
        pathPoints.isEmpty() ? seamPoints = SeamAllowancePoints(data) : seamPoints = pathPoints;

        {
            // first notch
            QLineF line(previousSAPoint, notchSAPoint);
            line.setLength(line.length()*100); // Hope 100 is enough

            const QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(seamPoints, line);
            if (intersections.isEmpty())
            {
                return QVector<QLineF>(); // Something wrong
            }

            line = QLineF(intersections.first(), notchSAPoint);
            line.setLength(notchData.length);

            notchData.line = line;
            notches += createNotches(notchData, pathPoints);
        }

        {
            // second notch
            QLineF line(nextSAPoint, notchSAPoint);
            line.setLength(line.length()*100); // Hope 100 is enough

            const QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(seamPoints, line);

            if (intersections.isEmpty())
            {
                return QVector<QLineF>(); // Something wrong
            }

            line = QLineF(intersections.last(), notchSAPoint);
            line.setLength(notchData.length);

            notchData.line = line;
            notches += createNotches(notchData, pathPoints);
        }
    }

    return notches;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createBuiltInSaNotch(const QVector<VPieceNode> &path, const VSAPoint &previousSAPoint,
                                             const VSAPoint &notchSAPoint, const VSAPoint &nextSAPoint,
                                             const VContainer *data, int notchIndex,
                                             const QVector<QPointF> &pathPoints) const
{
    QVector<QLineF> notches;

    const VPieceNode &node = path.at(notchIndex);

    QLineF edge1 = QLineF(notchSAPoint, previousSAPoint);
    QLineF edge2 = QLineF(notchSAPoint, nextSAPoint);

    NotchData notchData;
    notchData.type    = node.getNotchType();
    notchData.subType = node.getNotchSubType();
    notchData.length  = ToPixel(node.getNotchLength(), *data->GetPatternUnit());
    notchData.width   = ToPixel(node.getNotchWidth(), *data->GetPatternUnit());
    notchData.angle   = node.getNotchAngle();
    notchData.count   = node.getNotchCount();

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
    edge1.setLength(notchData.length);
    notchData.line = edge1;
    notches += createNotches(notchData, pathPoints);

    return notches;
}

//---------------------------------------------------------------------------------------------------------------------
int VPiece::IsCSAStart(const QVector<CustomSARecord> &records, quint32 id)
{
    for (int i = 0; i < records.size(); ++i)
    {
        if (records.at(i).startPoint == id)
        {
            return i;
        }
    }

    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createSlitNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = QLineF(notchData.line);
    qreal  angle   = notchData.angle;
    qreal  offset  = notchData.offset;

    QLineF line = createParallelLine(refline.p1(), refline.p2(), offset);
    line.setAngle(line.angle() + angle);

    QVector<QLineF> lines;
    lines.append(findIntersection(pathPoints, QLineF(line.p2(), line.p1())));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createTNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = QLineF(notchData.line);
    qreal length   = notchData.length;
    qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    qreal offset   = notchData.offset;

    QLineF tempLine = createParallelLine(refline.p1(), refline.p2(), offset);
    tempLine.setAngle(tempLine.angle() + angle);
    tempLine = findIntersection(pathPoints, QLineF(tempLine.p2(), tempLine.p1()));
    QLineF line = QLineF(tempLine.p2(), tempLine.p1());
    line.setLength(length);

    QPointF p1;
    {
        QLineF tempLine = QLineF(line.p2(), line.p1());
        tempLine.setAngle(tempLine.angle() - 90);
        tempLine.setLength(width  * 0.75 / 2);
        p1 = tempLine.p2();
    }

    QPointF p2;
    {
        QLineF tempLine = QLineF(line.p2(), line.p1());
        tempLine.setAngle(tempLine.angle() + 90);
        tempLine.setLength(width * 0.75 / 2);
        p2 = tempLine.p2();
    }

    QVector<QLineF> lines;
    lines.append(line);
    lines.append(QLineF(p1, p2));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createUNotch(const NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = QLineF(notchData.line);
    qreal length   = notchData.length;
    qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    qreal offset   = notchData.offset;

    QLineF centerLine = createParallelLine(refline.p1(), refline.p2(), offset);

    QLineF line1 = createParallelLine(centerLine.p1(), centerLine.p2(), width / 2.);
    line1.setLength(length / 2.);
    line1.setAngle(line1.angle() + angle);

    QLineF line2 = createParallelLine(centerLine.p1(), centerLine.p2(), - width / 2.);
    line2.setLength(length / 2.);
    line2.setAngle(line2.angle() + angle);

    centerLine.setLength(length / 2.);
    centerLine.setAngle(centerLine.angle() + angle);

    VArc arc(VPointF(centerLine.p2()), width / 2., QLineF(centerLine.p2(), line2.p2()).angle(), QLineF(centerLine.p2(), line1.p2()).angle());

    QVector<QLineF> lines;
    lines.append(arc.getSegments());
    lines.append(findIntersection(pathPoints, QLineF(line1.p2(), line1.p1())));
    lines.append(findIntersection(pathPoints, QLineF(line2.p2(), line2.p1())));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createVInternalNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = QLineF(notchData.line);
    qreal length   = notchData.length;
    qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    qreal offset   = notchData.offset;

    QLineF tempLine = createParallelLine(refline.p1(), refline.p2(), offset);
    tempLine.setAngle(tempLine.angle() + angle);
    tempLine = findIntersection(pathPoints, QLineF(tempLine.p2(), tempLine.p1()));
    QLineF line = QLineF(tempLine.p2(), tempLine.p1());
    line.setLength(length);

    QPointF p1;
    {
        QLineF tempLine = QLineF(line.p2(), line.p1());
        tempLine.setAngle(tempLine.angle() - 90);
        tempLine.setLength(width / 2);
        p1 = tempLine.p2();
    }

    QPointF p2;
    {
        QLineF tempLine = QLineF(line.p2(), line.p1());
        tempLine.setAngle(tempLine.angle() + 90);
        tempLine.setLength(width / 2);
        p2 = tempLine.p2();
    }

    QVector<QLineF> lines;
    lines.append(QLineF(line.p1(), p2));
    lines.append(QLineF(line.p1(), p1));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createVExternalNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = QLineF(notchData.line);
    qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    qreal offset   = notchData.offset;

    QLineF line = createParallelLine(refline.p1(), refline.p2(), offset);
    line.setAngle(line.angle() + angle);

    QPointF p1;
    {
        QLineF tempLine = QLineF(line.p1(), line.p2());
        tempLine.setAngle(tempLine.angle() - 90);
        tempLine.setLength(width / 2);
        p1 = tempLine.p2();
    }

    QPointF p2;
    {
        QLineF tempLine = QLineF(line.p1(), line.p2());
        tempLine.setAngle(tempLine.angle() + 90);
        tempLine.setLength(width / 2);
        p2 = tempLine.p2();
    }

    QVector<QLineF> lines;
    lines.append(findIntersection(pathPoints, QLineF(line.p2(), p2)));
    lines.append(findIntersection(pathPoints, QLineF(line.p2(), p1)));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createCastleNotch(const NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    QLineF refline = notchData.line;
    qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    qreal offset   = notchData.offset;

    QLineF line = createParallelLine(refline.p1(), refline.p2(), offset);

    QLineF line1 = createParallelLine(line.p1(), line.p2(), width / 2.);
    line1.setAngle(line1.angle() + angle);
    QLineF line2 = createParallelLine(line.p1(), line.p2(), -width / 2.);
    line2.setAngle(line2.angle() + angle);

    QVector<QLineF> lines;
    lines.append(QLineF(line1.p2(), line2.p2()));
    lines.append(findIntersection(pathPoints, QLineF(line1.p2(), line1.p1())));
    lines.append(findIntersection(pathPoints, QLineF(line2.p2(), line2.p1())));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createDiamondNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    //QLineF refline = notchData.line;
    //qreal length    = notchData.length;
    //qreal width    = notchData.width;
    qreal angle    = notchData.angle;
    //qreal offset   = notchData.offset;

    QVector<QLineF> lines;
    lines += createVExternalNotch(notchData, pathPoints);
    notchData.angle = angle - 180;
    lines += createVExternalNotch(notchData, pathPoints);
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createNotches(NotchData notchData, const QVector<QPointF> &pathPoints) const
{
    int count            = notchData.count;
    NotchType type       = notchData.type;
    NotchSubType subType = notchData.subType;
    qreal width          = notchData.width;

    QVector<QLineF> notches;

    if (subType == NotchSubType::Straightforward)
    {
        switch (type)
        {
            case NotchType::TNotch:
            {
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createTNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createTNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createTNotch(notchData, pathPoints);
                        notchData.offset = width * 1.25;
                        notches += createTNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.25);
                        notches += createTNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createTNotch(notchData, pathPoints);
                        break;
                }
                break;
            case NotchType::UNotch:
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createUNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createUNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createUNotch(notchData, pathPoints);
                        notchData.offset = width * 1.375;
                        notches += createUNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.375);
                        notches += createUNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createUNotch(notchData, pathPoints);
                        break;
                }
                break;
            case NotchType::VInternal:
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createVInternalNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createVInternalNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createVInternalNotch(notchData, pathPoints);
                        notchData.offset = width * 1.25;
                        notches += createVInternalNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.25);
                        notches += createVInternalNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createVInternalNotch(notchData, pathPoints);
                        break;
                }
                break;
            case NotchType::VExternal:
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createVExternalNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createVExternalNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createVExternalNotch(notchData, pathPoints);
                        notchData.offset = width * 1.25;
                        notches += createVExternalNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.25);
                        notches += createVExternalNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createVExternalNotch(notchData, pathPoints);
                        break;
                }
                break;
            case NotchType::Castle:
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createCastleNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createCastleNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createCastleNotch(notchData, pathPoints);
                        notchData.offset = width * 1.375;
                        notches += createCastleNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.375);
                        notches += createCastleNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createCastleNotch(notchData, pathPoints);
                        break;
                }
                break;
                case NotchType::Diamond:
                    switch (count)
                    {
                        case 2:
                            notchData.offset = width * .625;
                            notches += createDiamondNotch(notchData, pathPoints);
                            notchData.offset = -(width * .625);
                            notches += createDiamondNotch(notchData, pathPoints);
                            break;
                        case 3:
                            notchData.offset = 0;
                            notches += createDiamondNotch(notchData, pathPoints);
                            notchData.offset = width * 1.375;
                            notches += createDiamondNotch(notchData, pathPoints);
                            notchData.offset = -(width * 1.375);
                            notches += createDiamondNotch(notchData, pathPoints);
                            break;
                        case 1:
                        default:
                            notchData.offset = 0;
                            notches += createDiamondNotch(notchData, pathPoints);
                            break;
                    }
                    break;
            case NotchType::Slit:
            default:
                switch (count)
                {
                    case 2:
                        notchData.offset = width * .625;
                        notches += createSlitNotch(notchData, pathPoints);
                        notchData.offset = -(width * .625);
                        notches += createSlitNotch(notchData, pathPoints);
                        break;
                    case 3:
                        notchData.offset = 0;
                        notches += createSlitNotch(notchData, pathPoints);
                        notchData.offset = width * 1.25;
                        notches += createSlitNotch(notchData, pathPoints);
                        notchData.offset = -(width * 1.25);
                        notches += createSlitNotch(notchData, pathPoints);
                        break;
                    case 1:
                    default:
                        notchData.offset = 0;
                        notches += createSlitNotch(notchData, pathPoints);
                        break;
                }
            }
        }
    }
    else
    {
        notchData.offset = 0;
        notches += createSlitNotch(notchData, pathPoints);
    }

    return notches;
}

QLineF VPiece::findIntersection(const QVector<QPointF> &pathPoints, const QLineF &line) const
{
    QLineF tempLine = line;
    tempLine.setLength(tempLine.length()*1.5);
    QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(pathPoints, tempLine);
    if (not intersections.isEmpty())
    {
        return QLineF(line.p1(), intersections.last());
    }

    return line;
}

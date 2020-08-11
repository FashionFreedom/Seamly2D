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

#include "vpiece.h"
#include "vpiece_p.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vabstractcurve.h"
#include "vcontainer.h"
#include "../vmisc/vabstractapplication.h"

#include <QSharedPointer>
#include <QDebug>
#include <QPainterPath>

const qreal notchFactor = 0.5;
const qreal maxNotchLength = (10/*mm*/ / 25.4) * PrintDPI;

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

const qreal notchGap = (1.5/*mm*/ / 25.4) * PrintDPI;

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> createTwoNotchLines(const QLineF &line)
{
    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(notchGap/2.);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(notchGap/2.);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(notchGap/2.);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(notchGap/2.);
        l2p2 = line2.p2();
    }

    QVector<QLineF> lines;
    lines.append(QLineF(l1p1, l1p2));
    lines.append(QLineF(l2p1, l2p2));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> createThreeNotchLines(const QLineF &line)
{
    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(notchGap);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(notchGap);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(notchGap);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(notchGap);
        l2p2 = line2.p2();
    }

    QVector<QLineF> lines;
    lines.append(QLineF(l1p1, l1p2));
    lines.append(line);
    lines.append(QLineF(l2p1, l2p2));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> createTMarkNotch(const QLineF &line)
{
    QPointF p1;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() - 90);
        tmpLine.setLength(line.length() * 0.75 / 2);
        p1 = tmpLine.p2();
    }

    QPointF p2;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() + 90);
        tmpLine.setLength(line.length() * 0.75 / 2);
        p2 = tmpLine.p2();
    }

    QVector<QLineF> lines;
    lines.append(line);
    lines.append(QLineF(p1, p2));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> createVMarkNotch(const QLineF &line)
{
    QLineF l1 = line;
    l1.setAngle(l1.angle() - 35);

    QLineF l2 = line;
    l2.setAngle(l2.angle() + 35);

    QVector<QLineF> lines;
    lines.append(l1);
    lines.append(l2);
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> createNotches(NotchType notchType, NotchSubType notchSubType, const QLineF &line)
{
    QVector<QLineF> notches;

    if (notchSubType == NotchSubType::Straightforward || notchSubType == NotchSubType::Intersection)
    {
        switch (notchType)
        {
            case NotchType::TwoLines:
                notches += createTwoNotchLines(line);
                break;
            case NotchType::ThreeLines:
                notches += createThreeNotchLines(line);
                break;
            case NotchType::TMark:
                notches += createTMarkNotch(line);
                break;
            case NotchType::VMark:
                notches += createVMarkNotch(line);
                break;
            case NotchType::OneLine:
            default:
                notches.append(line);
                break;
        }
    }
    else
    {
        switch (notchType)
        {
            case NotchType::TMark:
                notches += createTMarkNotch(line);
                break;
            case NotchType::OneLine:
            case NotchType::TwoLines:
            case NotchType::ThreeLines:
            case NotchType::VMark:
            default:
                notches.append(line);
                break;
        }
    }

    return notches;
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
    , d (piece.d)
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
    if (not IsSeamAllowance() || not notchesPossible(unitedPath))
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
QPainterPath VPiece::getNotchesPath(const VContainer *data, const QVector<QPointF> &seamAllowance) const
{
    const QVector<QLineF> notches = createNotchLines(data, seamAllowance);
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
bool VPiece::IsInLayout() const
{
    return d->m_inLayout;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetInLayout(bool inLayout)
{
    d->m_inLayout = inLayout;
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
QString VPiece::GetFormulaSAWidth() const
{
    return d->m_formulaWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetFormulaSAWidth(const QString &formula, qreal value)
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
QVector<quint32> VPiece::GetPins() const
{
    return d->m_pins;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> &VPiece::GetPins()
{
    return d->m_pins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiece::SetPins(const QVector<quint32> &pins)
{
    d->m_pins = pins;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MissingNodes find missing nodes in detail. When we deleted object in detail and return this detail need
 * understand, what nodes need make invisible.
 * @param det changed detail.
 * @return  list with missing nodes.
 */
QVector<quint32> VPiece::MissingNodes(const VPiece &det) const
{
    return d->m_path.MissingNodes(det.GetPath());
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::MissingCSAPath(const VPiece &det) const
{
    QVector<quint32> oldCSARecords;
    for (qint32 i = 0; i < d->m_customSARecords.size(); ++i)
    {
        oldCSARecords.append(d->m_customSARecords.at(i).path);
    }

    QVector<quint32> newCSARecords;
    for (qint32 i = 0; i < det.GetCustomSARecords().size(); ++i)
    {
        newCSARecords.append(det.GetCustomSARecords().at(i).path);
    }

    return PieceMissingNodes(oldCSARecords, newCSARecords);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::MissingInternalPaths(const VPiece &det) const
{
    return PieceMissingNodes(d->m_internalPaths, det.GetInternalPaths());
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> VPiece::MissingPins(const VPiece &det) const
{
    return PieceMissingNodes(d->m_pins, det.GetPins());
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
int VPiece::getNotchNextSAPoints(const QVector<VPieceNode> &path, int index, const VSAPoint &notchSAPoint,
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

    /* Because method VAbstractPiece::EkvPoint has troubles with edges on a same line we should specially treat such
       cases.
       First check if two edges and seam alowance create paralell lines.
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
                                       const QVector<QPointF> &seamAllowance) const
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
    if (not getNotchPreviousSAPoints(path, previousIndex, notchSAPoint, data,
                                        previousSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    VSAPoint nextSAPoint;
    if (not getNotchNextSAPoints(path, nextIndex, notchSAPoint, data, nextSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    if (not IsSeamAllowanceBuiltIn())
    {
        QVector<QLineF> lines;
        lines += createSaNotch(path, previousSAPoint, notchSAPoint, nextSAPoint, data, notchIndex, seamAllowance);
        if (qApp->Settings()->isDoubleNotch()
                && not IsHideMainPath()
                && path.at(notchIndex).IsMainPathNode()
                && path.at(notchIndex).getNotchSubType() != NotchSubType::Intersection
                && path.at(notchIndex).showSecondNotch())
        {
            lines += createBuiltInSaNotch(path, previousSAPoint, notchSAPoint, nextSAPoint, data, notchIndex);
        }
        return lines;
    }
    else
    {
        return createBuiltInSaNotch(path, previousSAPoint, notchSAPoint, nextSAPoint, data, notchIndex);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createSaNotch(const QVector<VPieceNode> &path, VSAPoint &previousSAPoint,
                                   const VSAPoint &notchSAPoint, VSAPoint &nextSAPoint, const VContainer *data,
                                   int notchIndex, const QVector<QPointF> &seamAllowance) const
{
    QPointF seamNotchSAPoint;
    if (not getSeamNotchSAPoint(previousSAPoint, notchSAPoint, nextSAPoint, data, seamNotchSAPoint))
    {
        return QVector<QLineF>(); // Something wrong
    }

    const qreal width = ToPixel(GetSAWidth(), *data->GetPatternUnit());

    QVector<QLineF> notches;

    qreal notchLength = VAbstractPiece::MaxLocalSA(notchSAPoint, width) * notchFactor;
    notchLength = qMin(notchLength, maxNotchLength);
    const VPieceNode &node = path.at(notchIndex);
    if (node.getNotchSubType() == NotchSubType::Straightforward)
    {
        QLineF line = QLineF(seamNotchSAPoint, notchSAPoint);
        line.setLength(notchLength);
        notches += createNotches(node.getNotchType(), node.getNotchSubType(), line);
    }
    else if (node.getNotchSubType() == NotchSubType::Bisector)
    {
        const QLineF bigLine1 = ParallelLine(previousSAPoint, notchSAPoint, width );
        const QLineF bigLine2 = ParallelLine(notchSAPoint, nextSAPoint, width );

        QLineF edge1 = QLineF(seamNotchSAPoint, bigLine1.p1());
        QLineF edge2 = QLineF(seamNotchSAPoint, bigLine2.p2());

        edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
        edge1.setLength(notchLength);

        notches += createNotches(node.getNotchType(), node.getNotchSubType(), edge1);
    }
    else if (node.getNotchSubType() == NotchSubType::Intersection)
    {
        QVector<QPointF> seamPoints;
        seamAllowance.isEmpty() ? seamPoints = SeamAllowancePoints(data) : seamPoints = seamAllowance;

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
            line.setLength(qMin(notchSAPoint.GetSAAfter(width) * notchFactor, maxNotchLength));

            notches += createNotches(node.getNotchType(), node.getNotchSubType(), line);
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
            line.setLength(qMin(notchSAPoint.GetSABefore(width) * notchFactor, maxNotchLength));

            notches += createNotches(node.getNotchType(), node.getNotchSubType(), line);
        }
    }

    return notches;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPiece::createBuiltInSaNotch(const QVector<VPieceNode> &path, const VSAPoint &previousSAPoint,
                                          const VSAPoint &notchSAPoint, const VSAPoint &nextSAPoint,
                                          const VContainer *data, int notchIndex) const
{
    QVector<QLineF> notches;

    const qreal width = ToPixel(GetSAWidth(), *data->GetPatternUnit());
    qreal notchLength = VAbstractPiece::MaxLocalSA(notchSAPoint, width) * notchFactor;
    notchLength = qMin(notchLength, maxNotchLength);

    QLineF edge1 = QLineF(notchSAPoint, previousSAPoint);
    QLineF edge2 = QLineF(notchSAPoint, nextSAPoint);

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
    edge1.setLength(notchLength);

    const VPieceNode &node = path.at(notchIndex);
    notches += createNotches(node.getNotchType(), node.getNotchSubType(), edge1);

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

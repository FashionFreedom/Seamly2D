/***************************************************************************
 **  @file   vpiece.h
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
 **  @file   vpiece.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
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

#ifndef VPIECE_H
#define VPIECE_H

#include <QtGlobal>
#include <QLineF>
#include <QSharedDataPointer>
#include <QSharedPointer>

#include "../vlayout/vabstractpiece.h"

struct NotchData
{
    QLineF       line {};
    NotchType    type {};
    NotchSubType subType {};
    qreal        length {};
    qreal        width {};
    qreal        angle {};
    qreal        offset {};
    int          count {};
};

class VPieceData;
class VPieceNode;
template <class T> class QVector;
template <class T>class QSharedPointer;
class VPiecePath;
class VPatternLabelData;
class VPieceLabelData;
class VGrainlineData;
class VContainer;
class QPainterPath;
class VPointF;

class VPiece : public VAbstractPiece
{
public:
                             VPiece();
                             VPiece(const VPiece &piece);

    virtual                 ~VPiece();

    VPiece                  &operator=(const VPiece &piece);

#ifdef Q_COMPILER_RVALUE_REFS
    VPiece                  &operator=(VPiece &&piece) Q_DECL_NOTHROW;
#endif

    void                     Swap(VPiece &piece) Q_DECL_NOTHROW;

    VPiecePath               GetPath() const;
    VPiecePath              &GetPath();
    void                     SetPath(const VPiecePath &path);

    QVector<QPointF>         MainPathPoints(const VContainer *data) const;
    QVector<VPointF>         MainPathNodePoints(const VContainer *data, bool showExcluded = false) const;
    QVector<QPointF>         SeamAllowancePoints(const VContainer *data) const;
    QVector<QLineF>          createNotchLines(const VContainer *data,
                                              const QVector<QPointF> &seamAllowance = QVector<QPointF>()) const;

    QPainterPath             MainPathPath(const VContainer *data) const;
    QPainterPath             SeamAllowancePath(const VContainer *data) const;
    QPainterPath             SeamAllowancePath(const QVector<QPointF> &points) const;
    QPainterPath             getNotchesPath(const VContainer *data,
                                           const QVector<QPointF> &seamAllowance = QVector<QPointF>()) const;

    bool                     isInLayout() const;
    void                     SetInLayout(bool inLayout);

    bool                     isLocked() const;
    void                     setIsLocked(bool isLocked);

    bool                     IsUnited() const;
    void                     SetUnited(bool united);

    QString                  getSeamAllowanceWidthFormula() const;
    void                     setSeamAllowanceWidthFormula(const QString &formula, qreal value);

    QVector<quint32>         GetInternalPaths() const;
    QVector<quint32>        &GetInternalPaths();
    void                     SetInternalPaths(const QVector<quint32> &iPaths);

    QVector<CustomSARecord>  GetCustomSARecords() const;
    QVector<CustomSARecord>  &GetCustomSARecords();
    void                     SetCustomSARecords(const QVector<CustomSARecord> &records);

    QVector<quint32>         getAnchors() const;
    QVector<quint32>        &getAnchors();
    void                     setAnchors(const QVector<quint32> &anchors);

    QVector<quint32>         MissingNodes(const VPiece &piece) const;
    QVector<quint32>         MissingCSAPath(const VPiece &piece) const;
    QVector<quint32>         MissingInternalPaths(const VPiece &piece) const;
    QVector<quint32>         missingAnchors(const VPiece &piece) const;

    void                     SetPatternPieceData(const VPieceLabelData &data);
    VPieceLabelData&         GetPatternPieceData();
    const VPieceLabelData&   GetPatternPieceData() const;

    void                     SetPatternInfo(const VPatternLabelData &info);
    VPatternLabelData&       GetPatternInfo();
    const VPatternLabelData &GetPatternInfo() const;

    VGrainlineData&          GetGrainlineGeometry();
    const VGrainlineData&    GetGrainlineGeometry() const;

private:
    QSharedDataPointer<VPieceData> d;

    QVector<VPieceNode>      GetUnitedPath(const VContainer *data) const;

    QVector<CustomSARecord>  GetValidRecords() const;
    QVector<CustomSARecord>  FilterRecords(QVector<CustomSARecord> records) const;

    QVector<VSAPoint>        getNodeSAPoints(const QVector<VPieceNode> &path, int index,
                                             const VContainer *data) const;

    bool                     getNotchSAPoint(const QVector<VPieceNode> &path, int index,
                                             const VContainer *data, VSAPoint &point) const;
    bool                     getNotchPreviousSAPoints(const QVector<VPieceNode> &path, int index,
                                                      const VSAPoint &notchSAPoint,
                                                      const VContainer *data, VSAPoint &point) const;
    int                      getNextNotchSAPoints(const QVector<VPieceNode> &path, int index,
                                                  const VSAPoint &notchSAPoint,
                                                  const VContainer *data, VSAPoint &point) const;
    bool                     getSeamNotchSAPoint(const VSAPoint &previousSAPoint, const VSAPoint &notchSAPoint,
                                                 const VSAPoint &nextSAPoint, const VContainer *data,
                                                 QPointF &point) const;

    bool                     isNotchVisible(const QVector<VPieceNode> &path, int notchIndex) const;

    QVector<QLineF>          createNotch(const QVector<VPieceNode> &path, int previousIndex, int notchIndex,
                                         int nextIndex, const VContainer *data,
                                         const QVector<QPointF> &pathPoints = QVector<QPointF>()) const;

    QVector<QLineF>          createSeamAllowanceNotch(const QVector<VPieceNode> &path, VSAPoint &previousSAPoint,
                                                      const VSAPoint &notchSAPoint, VSAPoint &nextSAPoint,
                                                      const VContainer *data, int notchIndex,
                                                      const QVector<QPointF> &pathPoints = QVector<QPointF>()) const;

    QVector<QLineF>          createBuiltInSaNotch(const QVector<VPieceNode> &path, const VSAPoint &previousSAPoint,
                                                  const VSAPoint &notchSAPoint, const VSAPoint &nextSAPoint,
                                                  const VContainer *data, int notchIndex,
                                                  const QVector<QPointF> &pathPoints) const;

    static int               IsCSAStart(const QVector<CustomSARecord> &records, quint32 id);

    QVector<QLineF>          createSlitNotch(NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QVector<QLineF>          createTNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const;
    QVector<QLineF>          createUNotch(const NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QVector<QLineF>          createVInternalNotch(NotchData notchData, const QVector<QPointF> &pathPoints) const;
    QVector<QLineF>          createVExternalNotch(NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QVector<QLineF>          createCastleNotch(const NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QVector<QLineF>          createDiamondNotch(NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QVector<QLineF>          createNotches(NotchData notchData, const QVector<QPointF> &seamAllowance) const;
    QLineF                   findIntersection(const QVector<QPointF> &seamAllowance, const QLineF &line) const;
};

Q_DECLARE_TYPEINFO(VPiece, Q_MOVABLE_TYPE);

#endif // VPIECE_H

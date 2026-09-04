//---------------------------------------------------------------------------------------------------------------------
//  @file   pattern_piece_dialog.cpp
//  @author Douglas S Caskey
//  @date   Dec 11, 2022
//
//  Copyright (C) 2017 - 2022 Seamly, LLC
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
//  @file   vtoolseamallowance.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   6 11, 2016
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
//  along with Valentina.  if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef PATTERNPIECE_TOOL_H
#define PATTERNPIECE_TOOL_H

#include <QtGlobal>
#include <qcompilerdetection.h>
#include <QHash>
#include <QObject>
#include <QGraphicsPathItem>

#include "vinteractivetool.h"

#include "../vpatterndb/vpiece.h"
#include "../vwidgets/vtextgraphicsitem.h"
#include "../vwidgets/vgrainlineitem.h"

class DialogTool;
class NonScalingFillPathItem;

class PatternPieceTool : public VInteractiveTool, public QGraphicsPathItem
{
    Q_OBJECT
public:
    virtual ~PatternPieceTool() Q_DECL_EQ_DEFAULT;
    static PatternPieceTool *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                      VAbstractPattern *doc, VContainer *data);

    static PatternPieceTool *Create(quint32 id, VPiece newPiece, QString &width, VMainGraphicsScene *scene,
                                      VAbstractPattern *doc, VContainer *data, const Document &parse,
                                      const Source &typeCreation, const QString &blockName = QString());

    static const quint8  pieceVersion;
    static const QString TagCSA;
    static const QString TagRecord;
    static const QString TagIPaths;
    static const QString TagAnchors;
    static const QString AttrVersion;
    static const QString AttrForbidFlipping;
    static const QString AttrPieceColor;
    static const QString AttrPieceFill;
    static const QString AttrPieceLock;
    static const QString AttrSeamAllowance;
    static const QString AttrHideSeamLine;
    static const QString AttrSeamAllowanceBuiltIn;
    static const QString AttrHeight;
    static const QString AttrUnited;
    static const QString AttrFont;
    static const QString AttrTopLeftAnchor;
    static const QString AttrBottomRightAnchor;
    static const QString AttrCenterAnchor;
    static const QString AttrTopAnchorPoint;
    static const QString AttrBottomAnchorPoint;

    void        Remove(bool ask);

    static void insertNodes(const QVector<VPieceNode> &nodes, quint32 pieceId, VMainGraphicsScene *scene,
                            VContainer *data, VAbstractPattern *doc);

    static void addAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece,
                              const QHash<QString, QString> &nameToIdToken);
    static void AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, const CustomSARecord &record);
    static void addCSARecords(VAbstractPattern *doc, QDomElement &domElement, const QVector<CustomSARecord> &records);
    static void addInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths);
    static void addAnchors(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &anchors);
    static void addPieceLabel(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                              const QHash<QString, QString> &nameToIdToken);
    static void addPatternLabel(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                                const QHash<QString, QString> &nameToIdToken);
    static void addGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece,
                             const QHash<QString, QString> &nameToIdToken);

    void                 updatePiece(const VPiece &piece);
    void                 RefreshGeometry();

    virtual int          type() const override {return Type;}
    enum                 { Type = UserType + static_cast<int>(Tool::Piece)};

    virtual QString      getTagName() const override;
    virtual void         ShowVisualization(bool show) override;
    virtual void         GroupVisibility(quint32 object, bool visible) override;
    virtual void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget) override;

    virtual QRectF       boundingRect() const override;
    virtual QPainterPath shape() const override;

public slots:
    virtual void         FullUpdateFromFile () override;
    void                 EnableToolMove(bool move);
    void                 pieceLockedChanged(quint32 id, bool lock);
    virtual void         AllowHover(bool enabled) override;
    virtual void         AllowSelecting(bool enabled) override;
    virtual void         resetChildren(QGraphicsItem* pItem);
    virtual void         retranslateUi();
    void                 highlight(quint32 id);
    void                 updatePieceDetails();
    void                 updatePieceLabel();
    void                 updatePatternLabel();
    void                 updateGrainline();
    void                 editPieceProperties();

protected slots:
    void                 saveMovePiece(const QPointF &ptPos);
    void                 saveResizePiece(qreal dLabelW, int iFontSize);
    void                 savePieceRotation(qreal dRot);
    void                 SaveMovePattern(const QPointF& ptPos);
    void                 SaveResizePattern(qreal dLabelW, int iFontSize);
    void                 SaveRotationPattern(qreal dRot);
    void                 SaveMoveGrainline(const QPointF& ptPos);
    void                 SaveResizeGrainline(qreal dLength);
    void                 SaveRotateGrainline(qreal dRot, const QPointF& ptPos);

private slots:
    void                 nodeAngleChanged(quint32 id, PieceNodeAngle notchData);
    void                 notchChanged(quint32 id, NotchData notchData);
    void                 nodeExcluded(quint32 id);
    void                 nodeDeleted(quint32 id);

protected:
    virtual void         AddToFile () override;
    virtual void         RefreshDataInFile() override;
    virtual QVariant     itemChange (GraphicsItemChange change, const QVariant &value) override;
    virtual void         mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    virtual void         mouseReleaseEvent (QGraphicsSceneMouseEvent * event) override;
    virtual void         hoverEnterEvent (QGraphicsSceneHoverEvent * event) override;
    virtual void         hoverLeaveEvent (QGraphicsSceneHoverEvent * event) override;
    virtual void         contextMenuEvent (QGraphicsSceneContextMenuEvent * event) override;
    virtual void         keyReleaseEvent(QKeyEvent * event) override;
    virtual void         SetVisualization() override {}
    virtual void         deleteTool(bool ask = true) override;
    virtual void         ToolCreation(const Source &typeCreation) override;
    virtual void         SetDialog() final;
    virtual void         SaveDialogChange() final;

private:
    Q_DISABLE_COPY(PatternPieceTool)

    QPainterPath            m_mainPath;    // Must be first to prevent crash
    QRectF                  m_pieceRect;
    QPainterPath            m_cutPath;
    VMainGraphicsScene     *m_pieceScene;  /// @brief pieceScene pointer to the scene. */
    QString                 m_blockName;
    NonScalingFillPathItem *m_cutLine;
    NonScalingFillPathItem *m_allowanceFill;
    VTextGraphicsItem      *m_dataLabel;
    VTextGraphicsItem      *m_patternInfo;
    VGrainlineItem         *m_grainLine;
    QGraphicsPathItem      *m_notches;

                           PatternPieceTool(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                              const Source &typeCreation, VMainGraphicsScene *scene,
                                              const QString &blockName, QGraphicsItem * parent = nullptr);

    void                  updateExcludeState();
    void                  updateInternalPaths();

    VPieceItem::MoveTypes findLabelGeometry(const VPatternLabelData &labelData, qreal &rotationAngle, qreal &labelWidth,
                                            qreal &labelHeight, QPointF &pos);

    VPieceItem::MoveTypes findGrainlineGeometry(const VGrainlineData &geom, qreal &length, qreal &rotationAngle,
                                                qreal &arrowLength, QPointF &pos);

    void                  initializeNodes(const VPiece &piece, VMainGraphicsScene *scene);
    static void           initializeNode(const VPieceNode &node, VMainGraphicsScene *scene, VContainer *data,
                                   VAbstractPattern *doc, PatternPieceTool *parent);

    static QVector<VPieceNode>   removeDuplicateNodePoints(const VPiece &piece, const QVector<VPieceNode> &nodes, VContainer *data);

    void                  InitCSAPaths(const VPiece &piece);
    void                  InitInternalPaths(const VPiece &piece);
    void                  initializeAnchorPoints(const VPiece &piece);
    bool                  PrepareLabelData(const VPatternLabelData &labelData, VTextGraphicsItem *labelItem,
                                           QPointF &pos, qreal &labelAngle);

    void                  UpdateLabelItem(VTextGraphicsItem *labelItem, QPointF pos, qreal labelAngle);

    void                  toggleInLayout(bool checked);
    void                  togglePieceLock(bool checked);
    void                  toggleFlipping(bool checked);
    void                  toggleSeamLine(bool checked);
    void                  toggleSeamAllowance(bool checked);
    void                  toggleGrainline(bool checked);
    void                  togglePatternLabel(bool checked);
    void                  togglePieceLabel(bool checked);
    void                  renamePiece(VPiece piece);
    void                  showStatus(QString message);
    void                  raiseItemToTop(QGraphicsItem *item);
    void                  lowerItemToBottom(QGraphicsItem *item);
};

#endif // PATTERNPIECE_TOOL_H

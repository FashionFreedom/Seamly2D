/***************************************************************************
 **  @file   pattern_piece_tool.h
 **  @author Douglas S Caskey
 **  @date   Dec 11, 2022
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
 **  @file   vtoolseamallowance.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2016
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

#ifndef PATTERNPIECE_TOOL_H
#define PATTERNPIECE_TOOL_H

#include <QtGlobal>
#include <qcompilerdetection.h>
#include <QObject>
#include <QGraphicsPathItem>

#include "vinteractivetool.h"

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

    static void AddAttributes(VAbstractPattern *doc, QDomElement &domElement, quint32 id, const VPiece &piece);
    static void AddCSARecord(VAbstractPattern *doc, QDomElement &domElement, const CustomSARecord &record);
    static void AddCSARecords(VAbstractPattern *doc, QDomElement &domElement, const QVector<CustomSARecord> &records);
    static void AddInternalPaths(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &paths);
    static void addAnchors(VAbstractPattern *doc, QDomElement &domElement, const QVector<quint32> &anchors);
    static void AddPatternPieceData(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);
    static void AddPatternInfo(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);
    static void AddGrainline(VAbstractPattern *doc, QDomElement &domElement, const VPiece &piece);

    void                 RefreshGeometry();

    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum                 { Type = UserType + static_cast<int>(Tool::Piece)};

    virtual QString      getTagName() const Q_DECL_OVERRIDE;
    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;
    virtual void         GroupVisibility(quint32 object, bool visible) Q_DECL_OVERRIDE;
    virtual void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                               QWidget *widget) Q_DECL_OVERRIDE;

    virtual QRectF       boundingRect() const Q_DECL_OVERRIDE;
    virtual QPainterPath shape() const Q_DECL_OVERRIDE;

public slots:
    virtual void         FullUpdateFromFile () Q_DECL_OVERRIDE;
    void                 EnableToolMove(bool move);
    void                 pieceLockedChanged(quint32 id, bool lock);
    virtual void         AllowHover(bool enabled) Q_DECL_OVERRIDE;
    virtual void         AllowSelecting(bool enabled) Q_DECL_OVERRIDE;
    virtual void         ResetChildren(QGraphicsItem* pItem);
    virtual void         UpdateAll();
    virtual void         retranslateUi();
    void                 Highlight(quint32 id);
    void                 updatePieceDetails();
    void                 UpdatePieceLabel();
    void                 UpdatePatternLabel();
    void                 UpdateGrainline();
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

protected:
    virtual void         AddToFile () Q_DECL_OVERRIDE;
    virtual void         RefreshDataInFile() Q_DECL_OVERRIDE;
    virtual QVariant     itemChange ( GraphicsItemChange change, const QVariant &value ) Q_DECL_OVERRIDE;
    virtual void         mousePressEvent( QGraphicsSceneMouseEvent * event) Q_DECL_OVERRIDE;
    virtual void         mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) Q_DECL_OVERRIDE;
    virtual void         hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) Q_DECL_OVERRIDE;
    virtual void         hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) Q_DECL_OVERRIDE;
    virtual void         contextMenuEvent (QGraphicsSceneContextMenuEvent * event ) Q_DECL_OVERRIDE;
    virtual void         keyReleaseEvent(QKeyEvent * event) Q_DECL_OVERRIDE;
    virtual void         SetVisualization() Q_DECL_OVERRIDE {}
    virtual void         deleteTool(bool ask = true) Q_DECL_OVERRIDE;
    virtual void         ToolCreation(const Source &typeCreation) Q_DECL_OVERRIDE;
    virtual void         SetDialog() Q_DECL_FINAL;
    virtual void         SaveDialogChange() Q_DECL_FINAL;

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

    void                  UpdateExcludeState();
    VPieceItem::MoveTypes FindLabelGeometry(const VPatternLabelData &labelData, qreal &rotationAngle, qreal &labelWidth,
                                            qreal &labelHeight, QPointF &pos);

    VPieceItem::MoveTypes FindGrainlineGeometry(const VGrainlineData &geom, qreal &length, qreal &rotationAngle,
                                                QPointF &pos);

    void                  initializeNodes(const VPiece &piece, VMainGraphicsScene *scene);
    static void           initializeNode(const VPieceNode &node, VMainGraphicsScene *scene, VContainer *data,
                                   VAbstractPattern *doc, PatternPieceTool *parent);

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
    void                  showStatus(QString toolTip);
    void                  raiseItemToTop(QGraphicsItem *item);
    void                  lowerItemToBottom(QGraphicsItem *item);
};

#endif // PATTERNPIECE_TOOL_H

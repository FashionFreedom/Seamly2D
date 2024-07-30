/***************************************************************************
 **  @file   vabstractpattern.h
 **  @author Douglas S Caskey
 **  @date   Mar 1, 2023
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vabstractpattern.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VABSTRACTPATTERN_H
#define VABSTRACTPATTERN_H

#include <qcompilerdetection.h>
#include <QHash>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QPair>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>
#include <QVector>

#include "vdomdocument.h"
#include "vtoolrecord.h"
#include "../vmisc/def.h"
#include "../vwidgets/pen_toolbar.h"
#include "../../tools/images/image_item.h"

class QDomElement;
class VPiecePath;
class VPieceNode;
class PenToolBar;

enum class Document : char { LiteParse, LiteBlockParse, FullParse };
enum class LabelType : char {NewPatternPiece, NewLabel};

// Don't touch values!!!. Same values stored in xml.
enum class CrossCirclesPoint : char {FirstPoint = 1, SecondPoint = 2};
enum class VCrossCurvesPoint : char {HighestPoint = 1, LowestPoint = 2};
enum class HCrossCurvesPoint : char {LeftmostPoint = 1, RightmostPoint = 2};
enum class AxisType : char {VerticalAxis = 1, HorizontalAxis = 2};

class VContainer;
class VDataTool;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")

struct VFormulaField
{
    QString     expression;
    QDomElement element;
    QString     attribute;
};

struct GroupAttributes
{
   QString  name;
   bool     visible;
   bool     locked;
   QString  color;
   QString  linetype;
   QString  lineweight;
};

QT_WARNING_POP

class VAbstractPattern : public QObject, public VDomDocument
{
    Q_OBJECT
public:
    explicit                       VAbstractPattern(QObject *parent = nullptr);
    virtual                       ~VAbstractPattern() Q_DECL_EQ_DEFAULT;

    QStringList                    ListMeasurements() const;
    QVector<VFormulaField>         ListExpressions() const;
    QVector<VFormulaField>         listVariableExpressions() const;

    virtual void                   CreateEmptyFile()=0;

    void                           changeActiveDraftBlock(const QString &name, const Document &parse = Document::FullParse);
    QString                        getActiveDraftBlockName() const;

    bool                           draftBlockNameExists(const QString &name) const;
    int                            draftBlockCount() const;
    QDomElement                    getDraftBlockElement(const QString &name);
    bool                           renameDraftBlock(const QString &oldName, const QString &newName);
    bool                           appendDraftBlock(const QString &name);

    bool                           getActiveNodeElement(const QString &name, QDomElement &element) const;

    quint32                        getCursor() const;
    void                           setCursor(const quint32 &value);

    void                           setDefaultPen(Pen pen);
    QString                        getDefaultLineColor() const;
    qreal                          getDefaultLineWeight() const;
    QString                        getDefaultLineType() const;

    void                           setDefaultBasePoint(QString basePoint);

    virtual void                   IncrementReferens(quint32 id) const=0;
    virtual void                   DecrementReferens(quint32 id) const=0;

    virtual QStringList            GetCurrentAlphabet() const=0;
    virtual QString                GenerateLabel(const LabelType &type, const QString &reservedName = QString())const=0;
    virtual QString                GenerateSuffix(const QString &type) const=0;

    virtual void                   UpdateToolData(const quint32 &id, VContainer *data)=0;

    static VDataTool              *getTool(quint32 id);
    static void                    AddTool(quint32 id, VDataTool *tool);
    static void                    RemoveTool(quint32 id);

    static VPiecePath              ParsePieceNodes(const QDomElement &domElement);
    static QVector<CustomSARecord> ParsePieceCSARecords(const QDomElement &domElement);
    static QVector<quint32>        ParsePieceInternalPaths(const QDomElement &domElement);
    static QVector<quint32>        ParsePieceAnchors(const QDomElement &domElement);

    void                           AddToolOnRemove(VDataTool *tool);

    QVector<VToolRecord>          *getHistory();
    QVector<VToolRecord>           getBlockHistory() const;
    QMap<quint32, Tool>            getGroupObjHistory() const;

    QString                        MPath() const;
    void                           SetMPath(const QString &path);

    quint32                        SiblingNodeId(const quint32 &nodeId) const;

    QStringList                    getPatternPieces() const;

    QMap<qint32, ImageItem *>      getBackgroundImageMap();
    ImageItem *                    getBackgroundImage(qint32 id);
    void                           addBackgroundImage(qint32 id, ImageItem *item);
    void                           removeBackgroundImage(qint32 id);
    void                           clearBackgroundImageMap();

    QMap<GHeights, bool>           GetGradationHeights() const;
    void                           SetGradationHeights(const QMap<GHeights, bool> &options);

    QMap<GSizes, bool>             GetGradationSizes() const;
    void                           SetGradationSizes(const QMap<GSizes, bool> &options);

    QString                        GetDescription() const;
    void                           SetDescription(const QString &text);

    QString                        GetNotes() const;
    void                           SetNotes(const QString &text);

    QString                        GetPatternName() const;
    void                           SetPatternName(const QString &qsName);

    QString                        GetCompanyName() const;
    void                           SetCompanyName(const QString &qsName);

    QString                        GetPatternNumber() const;
    void                           SetPatternNumber(const QString &qsNum);

    QString                        GetCustomerName() const;
    void                           SetCustomerName(const QString &qsName);

    QString                        GetLabelDateFormat() const;
    void                           SetLabelDateFormat(const QString &format);

    QString                        GetLabelTimeFormat() const;
    void                           SetLabelTimeFormat(const QString &format);

    void                           setPatternLabelTemplate(const QVector<VLabelTemplateLine> &lines);

    QVector<VLabelTemplateLine>    getPatternLabelTemplate() const;

    void                           SetPatternWasChanged(bool changed);
    bool                           GetPatternWasChanged() const;

    QString                        GetImage() const;
    QString                        GetImageExtension() const;
    void                           SetImage(const QString &text, const QString &extension);
    void                           DeleteImage();

    QString                        GetVersion() const;
    void                           setVersion();

    QVector<quint32>               getOpItems(const quint32 &toolId, const QString &itemType);
    QVector<quint32>               getDartItems(const quint32 &toolId);

    bool                           IsModified() const;
    void                           SetModified(bool modified);

    QDomElement                    getDraw(const QString &name) const;

    void                           parseGroups(const QDomElement &domElement);
    QDomElement                    createGroups();
    QDomElement                    createGroup(quint32 id, const QString &name, const QString &color, const QString &type,
                                                const QString &weight, const QMap<quint32, quint32> &groupData);

    QDomElement                    addGroupItems(const QString &name, const QMap<quint32, quint32> &groupData);
    QString                        getGroupName(quint32 id);
    void                           setGroupName(quint32 id, const QString &name);

    QMap<quint32, GroupAttributes> getGroups();

    QStringList                    groupListByName();
    QDomElement                    getGroupByName(const QString &name);
    quint32                        getGroupIdByName(const QString &name);
    QMap<quint32, QString>         getGroupsContainingItem(quint32 toolId, quint32 objectId, bool containsItem);

    void                           deleteToolFromGroup(quint32 toolId);
    void                           addToolToGroup(quint32 toolId, quint32 objectId, const QString &groupName );

    QDomElement                    addGroupItem(quint32 toolId, quint32 objectId, quint32 groupId);
    QDomElement                    removeGroupItem(quint32 toolId, quint32 objectId, quint32 groupId);
    bool                           isGroupEmpty(quint32 id);

    bool                           getGroupVisibility(quint32 id);
    void                           setGroupVisibility(quint32 id, bool visible);

    bool                           getGroupLock(quint32 id);
    void                           setGroupLock(quint32 id, bool locked);

    QString                        getGroupColor(quint32 id);
    void                           setGroupColor(quint32 id, QString color);

    QString                        getGroupLineType(quint32 id);
    void                           setGroupLineType(quint32 id, QString type);

    QString                        getGroupLineWeight(quint32 id);
    void                           setGroupLineWeight(quint32 id, QString weight);

    bool                           groupNameExists(const QString &groupName);

    QString                        useGroupColor(quint32 toolId, QString color);
    QString                        useGroupLineType(quint32 toolId, QString type);
    QString                        useGroupLineWeight(quint32 toolId, QString weight);

    QDomElement                    createDraftImages();

    QPair<bool, QMap<quint32, quint32> > parseItemElement(const QDomElement &domElement);

    static const QString TagPattern;
    static const QString TagCalculation;
    static const QString TagModeling;
    static const QString TagPieces;
    static const QString TagPiece;
    static const QString TagDescription;
    static const QString TagImage;
    static const QString TagNotes;
    static const QString TagMeasurements;
    static const QString TagVariables;
    static const QString TagVariable;
    static const QString TagDraftBlock;
    static const QString TagGroups;
    static const QString TagGroup;
    static const QString TagGroupItem;
    static const QString TagPoint;
    static const QString TagSpline;
    static const QString TagArc;
    static const QString TagElArc;
    static const QString TagTools;
    static const QString TagOperation;
    static const QString TagGradation;
    static const QString TagHeights;
    static const QString TagSizes;
    static const QString TagData;
    static const QString TagPatternInfo;
    static const QString TagPatternName;
    static const QString TagPatternNum;
    static const QString TagCompanyName;
    static const QString TagCustomerName;
    static const QString TagPatternLabel;
    static const QString TagGrainline;
    static const QString TagPath;
    static const QString TagNodes;
    static const QString TagNode;
    static const QString TagLine;

    static const QString TagDraftImages;
    static const QString TagDraftImage;
    static const QString AttrId;
    static const QString AttrFilename;
    static const QString AttrLocked;
    static const QString AttrAnchor;
    static const QString AttrXPos;
    static const QString AttrYPos;
    static const QString AttrHeight;
    static const QString AttrXScale;
    static const QString AttrYScale ;
    static const QString AttrUnits;
    static const QString AttrOpacity;
    static const QString AttrOrder;
    static const QString AttrAspectRatio;
    static const QString AttrSource;
    static const QString AttrXOffset;
    static const QString AttrYOffset;
    static const QString AttrBasepoint;


    static const QString AttrName;
    static const QString AttrVisible;
    static const QString AttrGroupLocked;
    static const QString AttrGroupColor;
    static const QString AttrObject;
    static const QString AttrTool;
    static const QString AttrType;
    static const QString AttrLetter;
    static const QString AttrAnnotation;
    static const QString AttrOrientation;
    static const QString AttrRotationWay;
    static const QString AttrTilt;
    static const QString AttrFoldPosition;
    static const QString AttrQuantity;
    static const QString AttrOnFold;
    static const QString AttrDateFormat;
    static const QString AttrTimeFormat;
    static const QString AttrArrows;
    static const QString AttrNodeReverse;
    static const QString AttrNodeExcluded;
    static const QString AttrNodeIsNotch;
    static const QString AttrNodeNotchType;
    static const QString AttrNodeNotchSubType;
    static const QString AttrNodeShowNotch;
    static const QString AttrNodeShowSecondNotch;
    static const QString AttrNodeNotchLength;
    static const QString AttrNodeNotchWidth;
    static const QString AttrNodeNotchAngle;
    static const QString AttrNodeNotchCount;
    static const QString AttrSABefore;
    static const QString AttrSAAfter;
    static const QString AttrStart;
    static const QString AttrPath;
    static const QString AttrEnd;
    static const QString AttrIncludeAs;
    static const QString AttrWidth;
    static const QString AttrRotation;

    static const QString AttrAll;

    static const QString AttrH50;
    static const QString AttrH56;
    static const QString AttrH62;
    static const QString AttrH68;
    static const QString AttrH74;
    static const QString AttrH80;
    static const QString AttrH86;
    static const QString AttrH92;
    static const QString AttrH98;
    static const QString AttrH104;
    static const QString AttrH110;
    static const QString AttrH116;
    static const QString AttrH122;
    static const QString AttrH128;
    static const QString AttrH134;
    static const QString AttrH140;
    static const QString AttrH146;
    static const QString AttrH152;
    static const QString AttrH158;
    static const QString AttrH164;
    static const QString AttrH170;
    static const QString AttrH176;
    static const QString AttrH182;
    static const QString AttrH188;
    static const QString AttrH194;
    static const QString AttrH200;

    static const QString AttrS22;
    static const QString AttrS24;
    static const QString AttrS26;
    static const QString AttrS28;
    static const QString AttrS30;
    static const QString AttrS32;
    static const QString AttrS34;
    static const QString AttrS36;
    static const QString AttrS38;
    static const QString AttrS40;
    static const QString AttrS42;
    static const QString AttrS44;
    static const QString AttrS46;
    static const QString AttrS48;
    static const QString AttrS50;
    static const QString AttrS52;
    static const QString AttrS54;
    static const QString AttrS56;
    static const QString AttrS58;
    static const QString AttrS60;
    static const QString AttrS62;
    static const QString AttrS64;
    static const QString AttrS66;
    static const QString AttrS68;
    static const QString AttrS70;
    static const QString AttrS72;

    static const QString AttrCustom;
    static const QString AttrDefHeight;
    static const QString AttrDefSize;
    static const QString AttrExtension;

    static const QString VariableName;
    static const QString VariableFormula;
    static const QString VariableDescription;

    static const QString NodeArc;
    static const QString NodeElArc;
    static const QString NodePoint;
    static const QString NodeSpline;
    static const QString NodeSplinePath;

signals:
    /**
     * @brief activeDraftBlockChanged change active draft block.
     * @param newName new pattern peace name.
     */
    void           activeDraftBlockChanged(const QString &newName);

    /**
     * @brief ChangedCursor change cursor position.
     * @param id tool id.
     */
    void           ChangedCursor(quint32 id);

    /**
     * @brief draftBlockNameChanged save new name draft block.
     * @param oldName old name.
     * @param newName new name.
     */
    void           draftBlockNameChanged(const QString &oldName, const QString &newName);
    /**
     * @brief FullUpdateFromFile update tool data form file.
     */
    void           FullUpdateFromFile();
    /**
     * @brief patternChanged emit if we have unsaved change.
     */
    void           patternChanged(bool saved);
    void           UpdatePatternLabel();
    /**
     * @brief ShowTool highlight tool.
     * @param id tool id.
     * @param enable enable or disable highlight.
     */
    void           ShowTool(quint32 id, bool enable);
    void           ClearMainWindow();
    void           UndoCommand();
    void           setGuiEnabled(bool enabled);
    void           patternParsed();
    void           UpdateInLayoutList(quint32 id);
    void           showPiece(quint32 id);
    void           setCurrentDraftBlock(const QString &draftblock);
    void           patternHasGroups(bool value);
    void           updateGroups();

public slots:
    virtual void   LiteParseTree(const Document &parse)=0;
    void           haveLiteChange();
    void           NeedFullParsing();
    void           ClearScene();
    void           updatePieceList(quint32 id);
    void           selectedPiece(quint32 id);

protected:
    /** @brief activeBlockName name current pattern peace. */
    QString        activeDraftBlock;

    QString        m_DefaultLineColor;
    qreal          m_DefaultLineWeight;
    QString        m_DefaultLineType;

    QString        defaultBasePoint;

    QString        lastSavedExportFormat;

    /** @brief cursor cursor keep id tool after which we will add new tool in file. */
    quint32        cursor;

    QVector<VDataTool*> toolsOnRemove;

    /** @brief history history records. */
    QVector<VToolRecord> history;

    /** @brief patternPieces list of patern pieces names for combobox*/
    QStringList    patternPieces;

    /** @brief m_imageMap stores the image items and their id*/
    QMap<qint32, ImageItem *>         m_imageMap{};

    /** @brief modified keep state of the document for cases that do not cover QUndoStack*/
    mutable bool   modified;

    /** @brief tools list with pointer on tools. */
    static QHash<quint32, VDataTool*> tools;
    /** @brief patternLabelLines list to speed up reading a template by many pieces. */
    static QVector<VLabelTemplateLine> patternLabelLines;
    static bool patternLabelWasChanged;

    static void       ToolExists(const quint32 &id);
    static VPiecePath ParsePathNodes(const QDomElement &domElement);
    static VPieceNode ParseSANode(const QDomElement &domElement);

    void              setActiveDraftBlock(const QString &name);

    QDomElement       CheckTagExists(const QString &tag);
    void              InsertTag(const QStringList &tags, const QDomElement &element);

    void              SetChildTag(const QString &qsParent, const QString &qsChild, const QString &qsValue);

    int               getActiveDraftBlockIndex() const;
    bool              getActiveDraftElement(QDomElement &element) const;

    bool              hasGroupItem(const QDomElement &domElement, quint32 toolId, quint32 objectId);

private:
    Q_DISABLE_COPY(VAbstractPattern)

    QStringList            listVariables() const;
    QVector<VFormulaField> ListPointExpressions() const;
    QVector<VFormulaField> ListArcExpressions() const;
    QVector<VFormulaField> ListElArcExpressions() const;
    QVector<VFormulaField> ListSplineExpressions() const;
    QVector<VFormulaField> ListPathPointExpressions() const;
    QVector<VFormulaField> ListOperationExpressions() const;
    QVector<VFormulaField> ListNodesExpressions(const QDomElement &nodes) const;
    QVector<VFormulaField> ListPathExpressions() const;
    QVector<VFormulaField> ListGrainlineExpressions(const QDomElement &element) const;
    QVector<VFormulaField> ListPieceExpressions() const;

    bool                   IsVariable(const QString &token) const;
    bool                   IsPostfixOperator(const QString &token) const;
    bool                   IsFunction(const QString &token) const;

    QPair<bool, QMap<quint32, quint32> > ParseItemElement(const QDomElement &domElement);

};

#endif // VABSTRACTPATTERN_H

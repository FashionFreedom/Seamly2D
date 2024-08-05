/***************************************************************************
 **  @file   pattern_piece_dialog.h
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
 **  @file   dialogseamallowance.h
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

#ifndef PATTERN_PIECE_DIALOG
#define PATTERN_PIECE_DIALOG

#include "../dialogtool.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"

#include <QSoundEffect>

enum TabOrder {Properties=0, Paths=1, SeamAllowance= 2, Labels=3, AnchorPoints=4, Grainline=5, Notches=6, Count=7};

namespace Ui
{
    class PatternPieceDialog;
    class PathsTab;
    class LabelsTab;
    class GrainlineTab;
    class AnchorPointsTab;
    class NotchesTab;
}

class PieceAnchorPointVisual;

class PatternPieceDialog : public DialogTool
{
    Q_OBJECT

public:
                                PatternPieceDialog(const VContainer *data, const quint32 &toolId,
                                                    QWidget *parent = nullptr);
                                //PatternPieceDialog(const VContainer *data, const quint32 &toolId,
                                //                    QWidget *parent = nullptr);
    virtual                    ~PatternPieceDialog();

    void                        pageChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void                        enableApply(bool enable);

    VPiece                      GetPiece() const;
    void                        SetPiece(const VPiece &piece);

    QString                     getSeamAllowanceWidthFormula() const;

public slots:
    virtual void                ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    virtual void                ShowDialog(bool click) Q_DECL_OVERRIDE;

protected:
    virtual void                SaveData() Q_DECL_OVERRIDE; //! @brief SaveData Put dialog data in local variables
    virtual void                CheckState() Q_DECL_FINAL;
    virtual bool                eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;

    virtual void                closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    virtual void                showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
    virtual void                resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
    void                        setErrorText(TabOrder tab, QString text);
    void                        clearErrorText(TabOrder tab, QString text);
    void                        pieceNameChanged();
    void                        pieceColorChanged();
    void                        showMainPathContextMenu(const QPoint &pos);
    void                        showCustomSAContextMenu(const QPoint &pos);
    void                        showInternalPathsContextMenu(const QPoint &pos);
    void                        showAnchorsContextMenu(const QPoint &pos);

    void                        nodeListChanged();
    void                        enableSeamAllowance(bool enable);
    void                        enableBuiltIn(bool enable);
    void                        nodeChanged(int index);
    void                        notchChanged(int index);
    void                        customSAStartPointChanged(int index);
    void                        customSAEndPointChanged(int index);
    void                        customSAIncludeTypeChanged(int index);
    void                        nodeAngleChanged(int index);
    void                        enableDefaultBeforeButton();
    void                        enableDefaultAfterButton();
    void                        customSeamAllowanceChanged(int row);
    void                        pathDialogClosed(int result);

    void                        notchTypeChanged(int id);
    void                        notchSubTypeChanged(int id);
    void                        showNotchChanged(int state);
    void                        showSeamlineNotchChanged(int state);
    void                        notchLengthChanged(qreal value);
    void                        resetNotchLength();
    void                        notchWidthChanged(qreal value);
    void                        resetNotchWidth();
    void                        notchAngleChanged(qreal value);
    void                        resetNotchAngle();
    void                        notchCountChanged(int value);

    void                        updateGrainlineValues();
    void                        updatePieceLabelValues();
    void                        updatePatternLabelValues();

    void                        editGrainlineFormula();
    void                        editPieceLabelFormula();
    void                        editPatternLabelFormula();

    void                        resetGrainlineWarning();
    void                        resetLabelsWarning();

    void                        enabledGrainline();
    void                        enabledPieceLabel();
    void                        enabledPatternLabel();

    void                        evaluateDefaultWidth();
    void                        evaluateBeforeWidth();
    void                        evaluateAfterWidth();

    void                        editDefaultSeamAllowanceWidth();
    void                        editBeforeSeamAllowanceWidth();
    void                        editAfterSeamAllowanceWidth();

    void                        defaultWidthChanged();
    void                        beforeWidthChanged();
    void                        afterWidthChanged();

    void                        grainlineAnchorChanged();
    void                        pieceLabelAnchorChanged();
    void                        patternLabelAnchorChanged();

    void                        editPieceLabel();
    void                        editPatternLabel();

private:
    Q_DISABLE_COPY(PatternPieceDialog)

    Ui::PatternPieceDialog     *ui;
    bool                        applyAllowed;
    bool                        flagGrainlineAnchor;
    bool                        flagPieceLabelAnchor;
    bool                        flagPatternLabelAnchor;
    bool                        flagGrainlineFormula;
    bool                        flagPieceLabelAngle;
    bool                        flagPieceLabelFormula;
    bool                        flagPatternLabelAngle;
    bool                        flagPatternLabelFormula;
    bool                        flagBeforeFormula;
    bool                        flagAfterFormula;
    bool                        flagMainPath;
    bool                        m_bAddMode;
    qreal                       m_mx;
    qreal                       m_my;
    bool                        m_inLayout;
    bool                        m_isLocked;

    QPointer<DialogTool>             m_dialog;
    QPointer<PieceAnchorPointVisual> m_anchorPoints;

    VPieceLabelData             m_oldData;
    VPatternLabelData           m_oldGeom;
    VGrainlineData              m_oldGrainline;
    QTimer                     *m_timerWidth;
    QTimer                     *m_timerWidthBefore;
    QTimer                     *m_timerWidthAfter;
    qreal                       m_saWidth;
    QVector<VLabelTemplateLine> m_patternLabelLines;
    QVector<VLabelTemplateLine> m_pieceLabelLines;
    QSoundEffect               *m_beep;

    VPiece                      CreatePiece() const;

    void                        newMainPathItem(const VPieceNode &node);
    void                        newCustomSeamAllowance(const CustomSARecord &record);
    void                        newInternalPath(quint32 path);
    void                        newAnchorPoint(quint32 anchorPoint);
    QString                     getPathName(quint32 path, bool reverse = false) const;

    QString                     getPieceColor() const;
    void                        setPieceColor(const QString &value);

    QString                     getPieceFill() const;
    void                        setPieceFill(const QString &value);

    bool                        isInLayout() const;
    void                        setInLayout(const bool &state);

    bool                        getPieceLock() const;
    void                        setPieceLock(const bool &state);

    bool                        isMainPathValid() const;
    void                        validateObjects(bool value);
    bool                        isMainPathClockwise() const;
    void                        updateCurrentCustomSARecord();
    void                        updateCurrentInternalPathRecord();

    QListWidgetItem            *getItemById(quint32 id);

    quint32                     getPreviousId() const;
    VPieceNode                  getLastNode() const;
    bool                        pointNodesExist() const;

    void                        setCurrentBeforeSeamAllowance(const QString &formula);
    void                        setCurrentAfterSeamAllowance(const QString &formula);

    void                        updateNodeBeforeSeamAllowance(const QString &formula);
    void                        updateNodeAfterSeamAllowance(const QString &formula);

    void                        initializeMainPathTab();
    void                        initializeSeamAllowanceTab();
    void                        initializeNodesList();
    void                        initializeNotchesList();
    void                        initializeCSAPoint(QComboBox *box);
    void                        initAnchorPoint(QComboBox *box);
    void                        initializeSAIncludeType();
    void                        initializeInternalPathsTab();
    void                        initializePropertiesTab();
    void                        initializeLabelsTab();
    void                        initializeGrainlineTab();
    void                        initializeAnchorsTab();
    void                        initializeNotchesTab();
    void                        initAnchorPointComboboxes();

    void                        setSeamAllowanceWidthFormula(const QString &formula);

    void                        setGrainlineAngle(QString angleFormula);
    void                        setGrainlineLength(QString lengthFormula);

    void                        setPieceLabelWidth(QString widthFormula);
    void                        setPieceLabelHeight(QString heightFormula);
    void                        setPieceLabelAngle(QString angleFormula);

    void                        setPatternLabelWidth(QString widthFormula);
    void                        setPatternLabelHeight(QString heightFormula);
    void                        setPatternLabelAngle(QString angleFormula);

    void                        showAnchorPoints();
    void                        setMoveExclusions();

    QString                     createPieceName() const;
    void                        reverseNode(QListWidgetItem *rowItem);
    void                        duplicateNode(QListWidgetItem *rowItem);
    void                        excludeNode(QListWidgetItem *rowItem);
    void                        setNotch(QListWidgetItem *rowItem, bool isNotch, NotchType notchType,
                                         NotchSubType notchSubType);
    void                        setCurrentText(QComboBox *box, const QString &text) const;
    qreal                       getFormulaValue(QPlainTextEdit *text) const;
};

#endif // PATTERN_PIECE_DIALOG

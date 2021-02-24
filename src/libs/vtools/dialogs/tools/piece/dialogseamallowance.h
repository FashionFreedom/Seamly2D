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
 **  @file   dialogseamallowance.h
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

#ifndef DIALOGSEAMALLOWANCE_H
#define DIALOGSEAMALLOWANCE_H

#include "../dialogtool.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/floatItemData/vpatternlabeldata.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"

namespace Ui
{
    class DialogSeamAllowance;
    class PathsTab;
    class LabelsTab;
    class GrainlineTab;
    class PinsTab;
    class NotchesTab;
}

class VisPiecePins;
class FancyTabBar;

class DialogSeamAllowance : public DialogTool
{
    Q_OBJECT

public:
                                DialogSeamAllowance(const VContainer *data, const quint32 &toolId,
                                                    QWidget *parent = nullptr);
    virtual                    ~DialogSeamAllowance();

    void                        EnableApply(bool enable);

    VPiece                      GetPiece() const;
    void                        SetPiece(const VPiece &piece);

    QString                     GetFormulaSAWidth() const;

public slots:
    virtual void                ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    virtual void                ShowDialog(bool click) Q_DECL_OVERRIDE;

protected:
    virtual void                SaveData() Q_DECL_OVERRIDE; //! @brief SaveData Put dialog data in local variables
    virtual void                CheckState() Q_DECL_FINAL;
    virtual void                closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    virtual void                showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
    virtual void                resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
    void                        NameDetailChanged();
    void                        ShowMainPathContextMenu(const QPoint &pos);
    void                        ShowCustomSAContextMenu(const QPoint &pos);
    void                        ShowInternalPathsContextMenu(const QPoint &pos);
    void                        ShowPinsContextMenu(const QPoint &pos);

    void                        ListChanged();
    void                        EnableSeamAllowance(bool enable);
    void                        enableBuiltIn(bool enable);
    void                        NodeChanged(int index);
    void                        notchChanged(int index);
    void                        CSAStartPointChanged(int index);
    void                        CSAEndPointChanged(int index);
    void                        CSAIncludeTypeChanged(int index);
    void                        NodeAngleChanged(int index);
    void                        ReturnDefBefore();
    void                        ReturnDefAfter();
    void                        customSeamAllowanceChanged(int row);
    void                        PathDialogClosed(int result);
    void                        FancyTabChanged(int index);
    void                        TabChanged(int index);
    void                        notchTypeChanged(int id);
    void                        notchSubTypeChanged(int id);
    void                        showNotchChanged(int state);
    void                        showSecondNotchChanged(int state);
    void                        notchLengthChanged(qreal value);
    void                        resetNotchLength();
    void                        notchWidthChanged(qreal value);
    void                        resetNotchWidth();
    void                        notchAngleChanged(qreal value);
    void                        resetNotchAngle();
    void                        notchCountChanged(int value);

    void                        UpdateGrainlineValues();
    void                        UpdateDetailLabelValues();
    void                        UpdatePatternLabelValues();

    void                        EditGrainlineFormula();
    void                        EditDLFormula();
    void                        EditPLFormula();

    void                        DeployGrainlineRotation();
    void                        DeployGrainlineLength();

    void                        DeployDLWidth();
    void                        DeployDLHeight();
    void                        DeployDLAngle();

    void                        DeployPLWidth();
    void                        DeployPLHeight();
    void                        DeployPLAngle();

    void                        ResetGrainlineWarning();
    void                        ResetLabelsWarning();

    void                        EnabledGrainline();
    void                        EnabledDetailLabel();
    void                        EnabledPatternLabel();

    void                        EvalWidth();
    void                        EvalWidthBefore();
    void                        EvalWidthAfter();

    void                        FXWidth();
    void                        FXWidthBefore();
    void                        FXWidthAfter();

    void                        WidthChanged();
    void                        WidthBeforeChanged();
    void                        WidthAfterChanged();

    void                        DeployWidthFormulaTextEdit();
    void                        DeployWidthBeforeFormulaTextEdit();
    void                        DeployWidthAfterFormulaTextEdit();

    void                        GrainlinePinPointChanged();
    void                        DetailPinPointChanged();
    void                        PatternPinPointChanged();

    void                        EditLabel();

private:
    Q_DISABLE_COPY(DialogSeamAllowance)

    Ui::DialogSeamAllowance    *ui;
    Ui::PathsTab               *uiPathsTab;
    Ui::LabelsTab              *uiLabelsTab;
    Ui::GrainlineTab           *uiGrainlineTab;
    Ui::PinsTab                *uiPinsTab;
    Ui::NotchesTab           *uiNotchesTab;

    QWidget                    *m_pathsTab;
    QWidget                    *m_labelsTab;
    QWidget                    *m_grainlineTab;
    QWidget                    *m_pinsTab;
    QWidget                    *m_notchesTab;

    FancyTabBar                *m_ftb;

    bool                        applyAllowed;
    bool                        flagGPin;
    bool                        flagDPin;
    bool                        flagPPin;
    bool                        flagGFormulas;
    bool                        flagDLAngle;
    bool                        flagDLFormulas;
    bool                        flagPLAngle;
    bool                        flagPLFormulas;
    bool                        m_bAddMode;
    qreal                       m_mx;
    qreal                       m_my;

    QPointer<DialogTool>        m_dialog;
    QPointer<VisPiecePins>      m_visPins;

    VPieceLabelData             m_oldData;
    VPatternLabelData           m_oldGeom;
    VGrainlineData              m_oldGrainline;
    int                         m_iRotBaseHeight;
    int                         m_iLenBaseHeight;
    int                         m_dLabelWidthBaseHeight;
    int                         m_dLabelHeightBaseHeight;
    int                         m_dLabelAngleBaseHeight;
    int                         m_pLabelWidthBaseHeight;
    int                         m_pLabelHeightBaseHeight;
    int                         m_pLabelAngleBaseHeight;
    int                         m_widthFormula;
    int                         m_beforeWidthFormula;
    int                         m_afterWidthFormula;

    QTimer                     *m_timerWidth;
    QTimer                     *m_timerWidthBefore;
    QTimer                     *m_timerWidthAfter;
    qreal                       m_saWidth;

    QVector<VLabelTemplateLine> m_templateLines;

    VPiece                      CreatePiece() const;

    void                        NewMainPathItem(const VPieceNode &node);
    void                        NewCustomSA(const CustomSARecord &record);
    void                        NewInternalPath(quint32 path);
    void                        NewPin(quint32 pinPoint);
    QString                     GetPathName(quint32 path, bool reverse = false) const;
    bool                        MainPathIsValid() const;
    void                        ValidObjects(bool value);
    bool                        MainPathIsClockwise() const;
    void                        UpdateCurrentCustomSARecord();
    void                        UpdateCurrentInternalPathRecord();

    QListWidgetItem            *GetItemById(quint32 id);

    quint32                     GetLastId() const;

    void                        SetCurrentSABefore(const QString &formula);
    void                        SetCurrentSAAfter(const QString &formula);

    void                        UpdateNodeSABefore(const QString &formula);
    void                        UpdateNodeSAAfter(const QString &formula);

    void                        InitFancyTabBar();
    void                        InitMainPathTab();
    void                        InitSeamAllowanceTab();
    void                        InitNodesList();
    void                        InitNotchesList();
    void                        InitCSAPoint(QComboBox *box);
    void                        InitPinPoint(QComboBox *box);
    void                        InitSAIncludeType();
    void                        InitInternalPathsTab();
    void                        InitPatternPieceDataTab();
    void                        InitLabelsTab();
    void                        InitGrainlineTab();
    void                        InitPinsTab();
    void                        InitNotchesTab();
    void                        InitAllPinComboboxes();

    void                        SetFormulaSAWidth(const QString &formula);

    void                        SetGrainlineAngle(QString angleFormula);
    void                        SetGrainlineLength(QString lengthFormula);

    void                        SetDLWidth(QString widthFormula);
    void                        SetDLHeight(QString heightFormula);
    void                        SetDLAngle(QString angleFormula);

    void                        SetPLWidth(QString widthFormula);
    void                        SetPLHeight(QString heightFormula);
    void                        SetPLAngle(QString angleFormula);

    void                        ShowPins();
    void                        setMoveExclusions();

};

#endif // DIALOGSEAMALLOWANCE_H

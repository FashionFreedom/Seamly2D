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
 **  @file   dialoginternalpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#ifndef DIALOGINTERNALPATH_H
#define DIALOGINTERNALPATH_H

#include "../dialogtool.h"

namespace Ui
{
    class DialogInternalPath;
}

class DialogInternalPath : public DialogTool
{
    Q_OBJECT
public:
    explicit                DialogInternalPath(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual                ~DialogInternalPath();

    void                    EnbleShowMode(bool disable);

    VPiecePath              GetPiecePath() const;
    void                    SetPiecePath(const VPiecePath &path);

    quint32                 GetPieceId() const;
    void                    SetPieceId(quint32 id);

    QString                 GetFormulaSAWidth() const;
    void                    SetFormulaSAWidth(const QString &formula);

    virtual void            SetPiecesList(const QVector<quint32> &list) Q_DECL_OVERRIDE;

public slots:
    virtual void            ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    virtual void            ShowDialog(bool click) Q_DECL_OVERRIDE;

protected:
    virtual void            CheckState() Q_DECL_FINAL;
    virtual void            ShowVisualization() Q_DECL_OVERRIDE;
    virtual void            closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void                    ShowContextMenu(const QPoint &pos);
    void                    ListChanged();
    void                    NameChanged();
    void                    NodeChanged(int index);
    void                    notchChanged(int index);
    void                    ReturnDefBefore();
    void                    ReturnDefAfter();
    void                    notchTypeChanged(int id);
    void                    notchSubTypeChanged(int id);

    void                    EvalWidth();
    void                    EvalWidthBefore();
    void                    EvalWidthAfter();

    void                    FXWidth();
    void                    FXWidthBefore();
    void                    FXWidthAfter();

    void                    WidthChanged();
    void                    WidthBeforeChanged();
    void                    WidthAfterChanged();

    void                    DeployWidthFormulaTextEdit();
    void                    DeployWidthBeforeFormulaTextEdit();
    void                    DeployWidthAfterFormulaTextEdit();

private:
    Q_DISABLE_COPY(DialogInternalPath)
    Ui::DialogInternalPath *ui;
    bool                    m_showMode;
    qreal                   m_saWidth;

    QTimer                 *m_timerWidth;
    QTimer                 *m_timerWidthBefore;
    QTimer                 *m_timerWidthAfter;

    int                     m_widthFormula;
    int                     m_beforeWidthFormula;
    int                     m_afterWidthFormula;

    void                    InitPathTab();
    void                    InitSeamAllowanceTab();
    void                    InitNotchesTab();
    void                    InitPathTypes();
    void                    InitNodesList();
    void                    InitNotchesList();
    void                    NodeAngleChanged(int index);

    VPiecePath              CreatePath() const;

    bool                    PathIsValid() const;
    void                    ValidObjects(bool value);
    void                    NewItem(const VPieceNode &node);

    PiecePathType           GetType() const;
    void                    SetType(PiecePathType type);

    Qt::PenStyle            GetPenType() const;
    void                    SetPenType(const Qt::PenStyle &type);

    bool                    IsCutPath() const;
    void                    SetCutPath(bool value);

    QListWidgetItem        *GetItemById(quint32 id);

    quint32                 GetLastId() const;

    void                    SetCurrentSABefore(const QString &formula);
    void                    SetCurrentSAAfter(const QString &formula);

    void                    UpdateNodeSABefore(const QString &formula);
    void                    UpdateNodeSAAfter(const QString &formula);

    QString                 GetFormulaSAWidthBefore() const;
    QString                 GetFormulaSAWidthAfter() const;

    void                    setMoveExclusions();
};

#endif // DIALOGINTERNALPATH_H

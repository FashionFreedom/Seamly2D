//---------------------------------------------------------------------------------------------------------------------
//  @file   internal_path_dialog.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
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
//  @file   dialoginternalpath.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   22 11, 2016
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

#ifndef INTERNAL_PATH_DIALOG_H
#define INTERNAL_PATH_DIALOG_H

#include "../dialogtool.h"

namespace Ui
{
    class InternalPathDialog;
}

class InternalPathDialog : public DialogTool
{
    Q_OBJECT
public:
    explicit                InternalPathDialog(const VContainer *data, quint32 toolId, QWidget *parent = nullptr);
    virtual                ~InternalPathDialog();

    void                    enableEditMode(bool disable);

    VPiecePath              getPiecePath() const;
    void                    setPiecePath(const VPiecePath &path);

    quint32                 getPieceId() const;
    void                    setPieceId(quint32 id);

    QString                 getSeamAllowanceWidthFormula() const;
    void                    setSeamAllowanceWidthFormula(const QString &formula);

    virtual void            SetPiecesList(const QVector<quint32> &list) override;

public slots:
    virtual void            ChosenObject(quint32 id, const SceneObject &type) override;
    virtual void            ShowDialog(bool click) override;

protected:
    virtual void            CheckState() final;
    virtual void            ShowVisualization() override;
    virtual void            closeEvent(QCloseEvent *event) override;

private slots:
    void                    showContextMenu(const QPoint &pos);
    void                    listChanged();
    void                    nameChanged();
    void                    setPenAttributes();
    void                    nodeChanged(int index);
    void                    notchChanged(int index);
    void                    enableDefaultBeforeButton();
    void                    enableDefaultAfterButton();
    void                    notchTypeChanged(int id);
    void                    notchSubTypeChanged(int id);

    void                    evaluateDefaultWidth();
    void                    evaluateBeforeWidth();
    void                    evaluateAfterWidth();

    void                    editDefaultSeamAllowanceWidth();
    void                    editBeforeSeamAllowanceWidth();
    void                    editAfterSeamAllowanceWidth();

    void                    defaultWidthChanged();
    void                    beforeWidthChanged();
    void                    afterWidthChanged();

    void                    expandWidthFormulaTextEdit();
    void                    expandWidthBeforeFormulaTextEdit();
    void                    expandWidthAfterFormulaTextEdit();

private:
    Q_DISABLE_COPY(InternalPathDialog)
    Ui::InternalPathDialog *ui;
    bool                    m_editMode;
    qreal                   m_saWidth;

    QTimer                 *m_timerWidth;
    QTimer                 *m_timerWidthBefore;
    QTimer                 *m_timerWidthAfter;

    int                     m_widthFormula;
    int                     m_beforeWidthFormula;
    int                     m_afterWidthFormula;

    void                    initializePathTab();
    void                    initializeSeamAllowanceTab();
    void                    initializeNotchesTab();
    void                    initializePathTypes();
    void                    initializeNodesList();
    void                    initializeNotchesList();
    void                    nodeAngleChanged(int index);

    VPiecePath              createPath() const;

    bool                    isValidPath() const;
    void                    validateObjects(bool value);
    void                    createNewItem(const VPieceNode &node);

    PiecePathType           getType() const;
    void                    setType(PiecePathType type);

    QString                 getLineColor() const;
    void                    setLineColor(const QString &value);

    Qt::PenStyle            getLineType() const;
    void                    setLineType(const Qt::PenStyle &type);

    QString                 getLineWeight() const;
    void                    setLineWeight(const QString &value);

    bool                    isCutPath() const;
    void                    setCutPath(bool value);

    QListWidgetItem        *getItemById(quint32 id);

    quint32                 getLastNodeId() const;

    void                    setCurrentSABefore(const QString &formula);
    void                    setCurrentAfterSeamAllowance(const QString &formula);

    void                    updateNodeBeforeSeamAllowance(const QString &formula);
    void                    updateNodeAfterSeamAllowance(const QString &formula);

    QString                 getSeamAllowanceWidthFormulaBefore() const;
    QString                 getSeamAllowanceWidthFormulaAfter() const;

    void                    setMoveExclusions();
    void                    validateName() const;
};

#endif // INTERNAL_PATH_DIALOG_H

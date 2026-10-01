//---------------------------------------------------------------------------------------------------------------------
//  @file   variables_dialog.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2023 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogvariables.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#ifndef VARIABLES_DIALOG_H
#define VARIABLES_DIALOG_H

#include "../vtools/dialogs/tools/dialogtool.h"
#include "../xml/vpattern.h"
#include "../vmisc/vtablesearch.h"

#include <QPair>
#include <QList>
#include <QTableWidget>

class VIndividualMeasurements;

namespace Ui
{
    class VariablesDialog;
}

/**
 * @brief The VariablesDialog class show Variables dialog. Tables of all variables in program will be here.
 */
class VariablesDialog : public DialogTool
{
    Q_OBJECT
public:
                                     VariablesDialog(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    virtual                         ~VariablesDialog() override;

signals:
    void                             updateProperties();

protected:
    virtual void                     closeEvent ( QCloseEvent * event ) override;
    virtual void                     changeEvent ( QEvent * event) override;
    virtual bool                     eventFilter(QObject *object, QEvent *event) override;
    virtual void                     showEvent( QShowEvent *event ) override;
    virtual void                     resizeEvent(QResizeEvent *event) override;

private slots:
    void                             showCustomVariableDetails();
    void                             filterVariables(const QString &filterString);
    void                             addCustomVariable();
    void                             removeCustomVariable();
    void                             moveUp();
    void                             moveDown();
    void                             saveCustomVariableName(const QString &text);
    void                             saveCustomVariableDescription();
    void                             saveCustomVariableFormula();
    void                             Fx();
    void                             FullUpdateFromFile();
    void                             refreshPattern();

private:
    Q_DISABLE_COPY(VariablesDialog)

    Ui::VariablesDialog             *ui;    /// @brief ui keeps information about user interface
    VContainer                      *m_data;  /// @brief m_data container with data
    VPattern                        *m_doc;   /// @brief m_doc dom document container
    QTableWidget                    *m_backup_table;
    QString                          m_reset_formula;
    QString                          m_reset_description;
    bool                             m_has_changes;

    QVector<QPair<QString, QString>> m_rename_list;

    QList<QSharedPointer<QTableWidget>> m_table_list;
    bool                             m_is_sorted;
    bool                             m_is_filtered;

    template <typename T>
    void                             fillTable(const QMap<QString, T> &varTable, QTableWidget *table);

    void                             fillCustomVariables(bool freshCall = false);
    void                             fillLineLengths();
    void                             fillLineAngles();
    void                             fillCurveLengths();
    void                             fillControlPointLengths();
    void                             fillArcsRadiuses();
    void                             fillCurveAngles();

    void                             showUnits();
    void                             showHeaderUnits(QTableWidget *table, int column, const QString &unit);

    void                             addCell(QTableWidget *table, const QString &text, int row, int column,
                                             int aligment, bool ok = true);

    QString                          getCustomVariableName() const;
    QString                          clearCustomVariableName(const QString &name) const;

    bool                             evalVariableFormula(const QString &formula, bool fromUser,
                                                          VContainer *data, QLabel *label);
    void                             setMoveControls();
    void                             enablePieces(bool enabled);

    void                             localUpdateTree();

    bool                             variableUsed(const QString &name) const;

    void                             renameCache(const QString &name, const QString &newName);

    void                             clearFormula();
    void                             resetFormula();
    void                             clearDescription();
    void                             resetDescription();

    void                             copyTable(const QTableWidget* source, QTableWidget* target);
    void                             undoButtonClicked();
};

#endif // VARIABLES_DIALOG_H

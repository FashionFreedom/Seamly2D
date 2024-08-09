/******************************************************************************
*   @file   edit_formula_dialog.h
**  @author DSCaskey <dscaskey@gmail.com>
**  @date   10 Jun, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   dialogeditwrongformula.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#ifndef EDIT_FORMULA_DIALOG_H
#define EDIT_FORMULA_DIALOG_H

#include <qcompilerdetection.h>
#include <QMap>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QTableWidgetItem>
#include <QtGlobal>

#include "../tools/dialogtool.h"

enum VariableTab {Measurements=0, Custom=1, LineLengths= 2, LineAngles=3, CurveLengths=4, CurveAngles=5,
               CPLengths=6, ArcRadii=7, Functions=8};

template <class T> class QSharedPointer;
class MeasurementVariable;

namespace Ui
{
class EditFormulaDialog;
}

/**
 * @brief The EditFormulaDialog class dialog for editing wrong formula.
 *
 * When math parser find in formula error user can try fix issue. Dialog will show all variables that user can use in
 * this formula. Dialog check fixed variant of formula.
 *
 * Don't implemant button "Apply" for this dialog!!
 */
class EditFormulaDialog : public DialogTool
{
    Q_OBJECT
public:
                 EditFormulaDialog(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual     ~EditFormulaDialog() Q_DECL_OVERRIDE;

    QString      GetFormula() const;
    void         SetFormula(const QString &value);
    void         setCheckZero(bool value);
    void         setCheckLessThanZero(bool value);
    void         setPostfix(const QString &value);

public slots:
    virtual void DialogAccepted() Q_DECL_OVERRIDE;
    virtual void DialogRejected() Q_DECL_OVERRIDE;
    virtual void EvalFormula() Q_DECL_OVERRIDE;
    void         valueChanged(int row);
    void         tabChanged(int row);
    void         insertVariable();
    void         insertValue(QTableWidgetItem * item);
    void         measurements();
    void         lineLengths();
    void         arcRadii();
    void         curveAngles();
    void         curveLengths();
    void         controlPointLengths();
    void         lineAngles();
    void         customVariables();
    void         functions();

protected:
    virtual void CheckState() Q_DECL_FINAL;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    virtual void showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
    virtual void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private slots:
    void         filterVariables(const QString &filter);

private:
    Q_DISABLE_COPY(EditFormulaDialog)
    Ui::EditFormulaDialog *ui;

    QString      m_formula;           //! @brief formula string with formula
    QString      m_undoFormula;       //! @brief copy of formula string used to perform undo
    bool         m_checkZero;
    bool         m_checkLessThanZero;
    QString      m_postfix;
    bool         m_restoreCursor;

    void         initializeVariables();

    template <class key, class val>
    void         showVariable(const QMap<key, val> &var);
    void         showMeasurements(const QMap<QString, QSharedPointer<MeasurementVariable> > &var);
    void         showFunctions();

    void         setDescription(const QString &name, qreal value, const QString &unit,
                                const QString &type, const QString &description);
    void         clearFormula();
    void         undoFormula();
};


#endif // EDIT_FORMULA_DIALOG_H

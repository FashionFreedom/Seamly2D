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
 **  @file   dialogvariables.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef DIALOG_VARIABLES_H
#define DIALOG_VARIABLES_H

#include "../vtools/dialogs/tools/dialogtool.h"
#include "../xml/vpattern.h"
#include "../vmisc/vtablesearch.h"

#include <QPair>
#include <QList>

class VIndividualMeasurements;

namespace Ui
{
    class DialogVariables;
}

/**
 * @brief The DialogVariables class show Variables dialog. Tables of all variables in program will be here.
 */
class DialogVariables : public DialogTool
{
    Q_OBJECT
public:
                                     DialogVariables(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    virtual                         ~DialogVariables() Q_DECL_OVERRIDE;

signals:
    void                             updateProperties();

protected:
    virtual void                     closeEvent ( QCloseEvent * event ) Q_DECL_OVERRIDE;
    virtual void                     changeEvent ( QEvent * event) Q_DECL_OVERRIDE;
    virtual bool                     eventFilter(QObject *object, QEvent *event) Q_DECL_OVERRIDE;
    virtual void                     showEvent( QShowEvent *event ) Q_DECL_OVERRIDE;
    virtual void                     resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

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
    Q_DISABLE_COPY(DialogVariables)

    /** @brief ui keeps information about user interface */
    Ui::DialogVariables             *ui;

    /** @brief data container with data */
    VContainer                      *data; // need because we must change data //-V703

    /** @brief doc dom document container */
    VPattern                        *doc;

    int                              formulaBaseHeight;

    bool                             hasChanges;

    QVector<QPair<QString, QString>> renameList;

    QList<QSharedPointer<QTableWidget>> tableList;
    bool                             isSorted;
    bool                             isFiltered;


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
};

#endif // DIALOG_VARIABLES_H

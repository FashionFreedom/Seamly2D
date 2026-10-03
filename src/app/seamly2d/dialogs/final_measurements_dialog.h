//---------------------------------------------------------------------------------------------------------------------
//  @file   final_measurements_dialog.h
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2026 Seamly2D project
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

#ifndef FINAL_MEASUREMENTS_DIALOG_H
#define FINAL_MEASUREMENTS_DIALOG_H

#include "../xml/vpattern.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"

#include <QDialog>
#include <QVector>

class VFormulaPropertyEditor;

namespace Ui
{
    class FinalMeasurementsDialog;
}

class FinalMeasurementsDialog : public QDialog
{
    Q_OBJECT

public:
                             FinalMeasurementsDialog(VContainer *data, VPattern *doc, QWidget *parent = nullptr);
    virtual                 ~FinalMeasurementsDialog();

    static VContainer        evaluationData(const VContainer *data, VPattern *doc);

signals:
    void                     dialogClosed();

protected:
    virtual void             closeEvent(QCloseEvent *event) override;
    virtual void             changeEvent(QEvent *event) override;
    virtual void             showEvent(QShowEvent *event) override;
    virtual void             resizeEvent(QResizeEvent *event) override;

private slots:
    void                     showMeasurementDetails();
    void                     filterMeasurements(const QString &filterString);
    void                     addMeasurement();
    void                     removeMeasurement();
    void                     moveUp();
    void                     moveDown();
    void                     validateName(const QString &text);
    void                     saveName();
    void                     saveDescription();
    void                     saveFormula();
    void                     saveFormulaFromEditor(const VFormula &formula);
    void                     fullUpdateFromFile();

private:
    Q_DISABLE_COPY(FinalMeasurementsDialog)
    Ui::FinalMeasurementsDialog *ui;
    VPattern                    *m_doc;
    VContainer                   m_evalData;
    QVector<VFinalMeasurement>   m_measurements;
    VFormulaPropertyEditor      *m_formulaEditor;
    bool                         m_isInitialized;

    void                     fillTable();
    void                     updateRow(int index);
    int                      rowForIndex(int index) const;
    int                      currentIndex() const;
    void                     selectIndex(int index);
    void                     enableDetails(bool enabled);
    void                     updateMoveButtons();
    void                     saveMeasurements();
    bool                     isNameValid(const QString &name, int index) const;
    QString                  uniqueName() const;
    VFormula                 makeFormula(const QString &formula);
};

#endif // FINAL_MEASUREMENTS_DIALOG_H

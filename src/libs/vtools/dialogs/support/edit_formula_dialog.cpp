//---------------------------------------------------------------------------------------------------------------------
//  @file   edit_formula_dialog.cpp
//  @author DSCaskey <dscaskey@gmail.com>
//  @date  10 Jun, 2023
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
//  @file   dialogeditwrongformula.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   29 5, 2014
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

#include "edit_formula_dialog.h"
#include "ui_edit_formula_dialog.h"

#include <qiterator.h>
#include <QAbstractItemView>
#include <QApplication>
#include <QCheckBox>
#include <QCursor>
#include <QDialog>
#include <QFont>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMapIterator>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSharedPointer>
#include <QShowEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QSize>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextCursor>
#include <QToolButton>
#include <QWidget>
#include <Qt>
#include <new>

#include "../ifc/xml/vdomdocument.h"
#include "../tools/dialogtool.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/variables/varcradius.h"
#include "../vpatterndb/variables/vcurveangle.h"
#include "../vpatterndb/variables/vcurvelength.h"
#include "../vpatterndb/variables/custom_variable.h"
#include "../vpatterndb/variables/vlineangle.h"
#include "../vpatterndb/variables/vlinelength.h"
#include "../vpatterndb/variables/measurement_variable.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../tools/dialogtool.h"
#include "ui_edit_formula_dialog.h"

template <class T> class QSharedPointer;

enum {ColumnName = 0, ColumnFullName};

//---------------------------------------------------------------------------------------------------------------------
EditFormulaDialog::EditFormulaDialog(const VContainer *data, const quint32 &toolId, const quint16 &source,
                                     QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::EditFormulaDialog)
    , m_formula(QString())
    , m_undoFormula(QString())
    , m_checkZero(false)
    , m_checkLessThanZero(false)
    , m_postfix(QString())
    , m_restoreCursor(false)
    , m_source(source)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    initializeVariables();
    initializeFormulaUi(ui);
    ui->plainTextEditFormula->installEventFilter(this);
    ui->menuTab_ListWidget->setCurrentRow(VariableTab::Measurements);

    connect(ui->filterFormulaInputs, &QLineEdit::textChanged, this, &EditFormulaDialog::filterVariables);

    initializeOkCancel(ui);
    flagFormula = false;
    CheckState();

    connect(ui->insert_PushButton,    &QPushButton::clicked,            this, &EditFormulaDialog::insertVariable);
    connect(ui->clear_PushButton,     &QPushButton::clicked,            this, &EditFormulaDialog::clearFormula);
    connect(ui->undo_PushButton,      &QPushButton::clicked,            this, &EditFormulaDialog::undoFormula);
    connect(ui->tableWidget,          &QTableWidget::itemDoubleClicked, this, &EditFormulaDialog::insertValue);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged,     this, &EditFormulaDialog::FormulaChanged);

    //Disable Qt::WaitCursor
#ifndef QT_NO_CURSOR
    if (QGuiApplication::overrideCursor() != nullptr)
    {
        if (QGuiApplication::overrideCursor()->shape() == Qt::WaitCursor)
        {
            m_restoreCursor = true;
            QGuiApplication::restoreOverrideCursor();
        }
    }
#endif

    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setEditTriggers(QTableWidget::NoEditTriggers);
    ui->tableWidget->verticalHeader()->hide();
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
}

//---------------------------------------------------------------------------------------------------------------------
EditFormulaDialog::~EditFormulaDialog()
{
#ifndef QT_NO_CURSOR
    if (m_restoreCursor)
    {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    }
#endif
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::DialogAccepted()
{
    m_formula = ui->plainTextEditFormula->toPlainText();
    m_formula.replace("\n", " ");
    emit DialogClosed(QDialog::Accepted);
    accepted();
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::DialogRejected()
{
    emit DialogClosed(QDialog::Rejected);
    rejected();
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::EvalFormula()
{
    SCASSERT(plainTextEditFormula != nullptr)
    SCASSERT(labelResultCalculation != nullptr)
    Eval(plainTextEditFormula->toPlainText(), flagFormula, labelResultCalculation, m_postfix,
         m_checkZero, m_checkLessThanZero);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief valueChanged show description when current variable changed
/// @param row number of row
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::valueChanged(int row)
{
    if (ui->tableWidget->rowCount() == 0)
    {
        ui->description_Label->setText("");
        return;
    }
    QTableWidgetItem *item = ui->tableWidget->item( row, ColumnName );

    switch (ui->menuTab_ListWidget->currentRow())
    {
        case VariableTab::Measurements:
        {
            const QString name = qApp->translateVariables()->VarFromUser(item->text());
            const QSharedPointer<MeasurementVariable> measurements = data->getVariable<MeasurementVariable>(name);
            const QString desc = (measurements->getGuiText() == "") ? "" : QString("\nDescription: %1").arg(measurements->getGuiText());
            setDescription(item->text(), *data->DataVariables()->value(name)->GetValue(),
                           UnitsToStr(qApp->patternUnit(), true), tr("Measurement"), desc);
            break;
        }
        case VariableTab::Custom:
        {
            const QSharedPointer<CustomVariable> variables = data->getVariable<CustomVariable>(item->text());
            const QString desc =(variables->GetDescription() == "") ? "" : QString("\nDescription: %1").arg(variables->GetDescription());
            setDescription(item->text(), *data->DataVariables()->value(item->text())->GetValue(),
                           UnitsToStr(qApp->patternUnit(), true), tr("Custom Variable"), desc);
            break;
        }
        case VariableTab::LineLengths:
            {
                setDescription(item->text(),
                        *data->getVariable<VLengthLine>(qApp->translateVariables()->VarFromUser(item->text()))->GetValue(),
                        UnitsToStr(qApp->patternUnit(), true), tr("Line length"), "");
                break;
            }
        case VariableTab::CurveLengths:
        {
            setDescription(item->text(),
                           *data->getVariable<VCurveLength>(qApp->translateVariables()->VarFromUser(item->text()))->GetValue(),
                           UnitsToStr(qApp->patternUnit(), true), tr("Curve length"), "");
            break;
        }
        case VariableTab::LineAngles:
        {
            setDescription(item->text(),
                           *data->getVariable<VLineAngle>(qApp->translateVariables()->VarFromUser(item->text()))->GetValue(),
                           degreeSymbol, tr("Line Angle"), "");
            break;
        }
        case VariableTab::ArcRadii:
        {
            setDescription(item->text(),
                           *data->getVariable<VArcRadius>(qApp->translateVariables()->VarFromUser(item->text()))->GetValue(),
                           UnitsToStr(qApp->patternUnit(), true), tr("Arc radius"), "");
            break;
        }
        case VariableTab::CurveAngles:
        {
            setDescription(item->text(),
                           *data->getVariable<VCurveAngle>(qApp->translateVariables()->VarFromUser(item->text()))->GetValue(),
                           degreeSymbol, tr("Curve angle"), "");
        break;
        }
        case VariableTab::Functions:
        {
            ui->description_Label->setText(item->toolTip());
            break;
        }
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief menu tabChanged
/// @param row number of row
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::tabChanged(int row)
{
    switch (row)
    {
        case VariableTab::Measurements:
        {
            measurements();
            break;
        }
        case VariableTab::Custom:
        {
            customVariables();
            break;
        }
        case VariableTab::LineLengths:
            {
                lineLengths();
                break;
            }
        case VariableTab::LineAngles:
        {
            lineAngles();
            break;
        }
        case VariableTab::CurveLengths:
        {
            curveLengths();
            break;
        }
        case VariableTab::CurveAngles:
        {
            curveAngles();
            break;
        }
        case VariableTab::CPLengths:
        {
            controlPointLengths();
            break;
        }
        case VariableTab::ArcRadii:
        {
            arcRadii();
            break;
        }
        case VariableTab::Functions:
        {
            functions();
            break;
        }
    }
    return;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief insertVariable insert variable into line edit
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::insertVariable()
{
    insertValue(ui->tableWidget->currentItem());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief insertValue insert variable into line edit
/// @param item chosen item of table widget
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::insertValue(QTableWidgetItem *item)
{
    if (item != nullptr)
    {
        QTextCursor cursor = ui->plainTextEditFormula->textCursor();
        if (ui->menuTab_ListWidget->currentRow() == VariableTab::Functions)
        {
            if (cursor.hasSelection())
            {
                QString selected = cursor.selectedText();
                cursor.insertText(ui->tableWidget->item(item->row(), ColumnName)->text() +
                                  QStringLiteral("(") + selected + QStringLiteral(")"));
                cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
                ui->plainTextEditFormula->setTextCursor(cursor);
            }
            else
            {
                cursor.insertText(ui->tableWidget->item(item->row(), ColumnName)->text() + QStringLiteral("()"));
                cursor.setPosition(cursor.position() - 1);
            }
        }
        else
        {
            cursor.insertText(ui->tableWidget->item(item->row(), ColumnName)->text());
        }
        ui->plainTextEditFormula->setTextCursor(cursor);
        ui->plainTextEditFormula->setFocus();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief measurements show measurements in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::measurements()
{
    ui->checkBoxHideEmpty->setEnabled(true);
    showMeasurements(data->DataMeasurements());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief lineLengths show lengths of line variables in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::lineLengths()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->lineLengthsData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief arcRadii show radii of arc variables in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::arcRadii()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->arcRadiusesData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief curveAngles show angles of curve variables in list
//---------------------------------------------------------------------------------------------------------------------
 void EditFormulaDialog::curveAngles()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->curveAnglesData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief curveLengths show lengths of curve variables in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::curveLengths()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->curveLengthsData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief controlPointLengths show lengths of control point variables in list
//---------------------------------------------------------------------------------------------------------------------
 void EditFormulaDialog::controlPointLengths()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->controlPointLengthsData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief lineAngles show angles of line variables in list
//---------------------------------------------------------------------------------------------------------------------
 void EditFormulaDialog::lineAngles()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->lineAnglesData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief customVariables show custom variables in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::customVariables()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showVariable(data->variablesData());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief functions show functions in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::functions()
{
    ui->checkBoxHideEmpty->setEnabled(false);
    showFunctions();
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    const QSize size = qApp->Settings()->GetFormulaWizardDialogSize();
    if (!size.isEmpty())
    {
        resize(size);
    }

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized.
    if (isInitialized)
    {
        qApp->Settings()->SetFormulaWizardDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::SetFormula(const QString &value)
{
    m_formula = qApp->translateVariables()->FormulaToUser(value, qApp->Settings()->getOsSeparator());
    m_undoFormula = m_formula;
    ui->plainTextEditFormula->setPlainText(m_formula);
    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::setCheckZero(bool value)
{
    m_checkZero = value;
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::setCheckLessThanZero(bool value)
{
    m_checkLessThanZero = value;
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::setPostfix(const QString &value)
{
    m_postfix = value;
}

//---------------------------------------------------------------------------------------------------------------------
QString EditFormulaDialog::GetFormula() const
{
    return qApp->translateVariables()->TryFormulaFromUser(m_formula, qApp->Settings()->getOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::initializeVariables()
{
    connect(ui->tableWidget, &QTableWidget::currentCellChanged, this, &EditFormulaDialog::valueChanged);

    measurements();

    // clear text filter whenever list widget row changes
    auto ClearFilterFormulaInputs = [this] () { ui->filterFormulaInputs->clear(); };

    connect(ui->menuTab_ListWidget,  &QListWidget::currentRowChanged, this, &EditFormulaDialog::tabChanged);
    connect(ui->menuTab_ListWidget,  &QListWidget::currentRowChanged, this, ClearFilterFormulaInputs);
    connect(ui->checkBoxHideEmpty,   &QCheckBox::stateChanged,        this, &EditFormulaDialog::measurements);


    for (int i = 0; i < ui->menuTab_ListWidget->count(); ++i)
    {
        // Set the selection highlight rect larger than just the item text
        ui->menuTab_ListWidget->item(i)->setSizeHint(QSize(ui->menuTab_ListWidget->width(), 50));

        // Set the visibility of tab item depending on source i.e., the parent dialog.
        ui->menuTab_ListWidget->item(i)->setHidden(!(m_source & (1 << i)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::setDescription(const QString &name, qreal value, const QString &unit,
                                            const QString &type, const QString &description)
{
    const QString desc = QString("%5: %1(%2 %3)%4").arg(name).arg(value).arg(unit).arg(description).arg(type);
    ui->description_Label->setText(desc);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showVariable show variables in list
/// @param var container with variables
//---------------------------------------------------------------------------------------------------------------------
template <class key, class val>
void EditFormulaDialog::showVariable(const QMap<key, val> &var)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnHidden(ColumnFullName, true);
    ui->description_Label->setText("");

    QMapIterator<key, val> iMap(var);
    while (iMap.hasNext())
    {
        iMap.next();
        if (ui->checkBoxHideEmpty->isEnabled() && ui->checkBoxHideEmpty->isChecked() && iMap.value()->isNotUsed())
        {
            continue; //skip this measurement
        }
        if (iMap.value()->Filter(toolId) == false)
        {// If we create this variable don't show
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
            QTableWidgetItem *item = new QTableWidgetItem(iMap.key());
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, ColumnName, item);
        }
    }
    ui->tableWidget->blockSignals(false);
    ui->tableWidget->selectRow(0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showMeasurements show measurements in table
/// @param var container with measurements
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::showMeasurements(const QMap<QString, QSharedPointer<MeasurementVariable> > &var)
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnHidden(ColumnFullName, false);
    ui->description_Label->setText("");

    QMapIterator<QString, QSharedPointer<MeasurementVariable>> iMap(var);
    while (iMap.hasNext())
    {
        iMap.next();
        if (ui->checkBoxHideEmpty->isEnabled() && ui->checkBoxHideEmpty->isChecked() && iMap.value()->isNotUsed())
        {
            continue; //skip this measurement
        }
        if (iMap.value()->Filter(toolId) == false)
        {// If we create this variable don't show
            ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
            QTableWidgetItem *itemName = new QTableWidgetItem(iMap.key());
            itemName->setToolTip(itemName->text());

            QTableWidgetItem *itemFullName = new QTableWidgetItem();
            if (iMap.value()->isCustom())
            {
                itemFullName->setText(iMap.value()->getGuiText());
            }
            else
            {
                itemFullName->setText(qApp->translateVariables()->guiText(iMap.value()->GetName()));
            }

            itemFullName->setToolTip(itemFullName->text());
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, ColumnName, itemName);
            ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, ColumnFullName, itemFullName);
        }
    }
    ui->tableWidget->blockSignals(false);
    ui->tableWidget->selectRow(0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showFunctions show functions in list
//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::showFunctions()
{
    ui->tableWidget->blockSignals(true);
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget->setColumnHidden(ColumnFullName, true);
    ui->description_Label->setText("");

    QMap<QString, qmu::QmuTranslation>::const_iterator i = qApp->translateVariables()->getFunctions().constBegin();
    while (i != qApp->translateVariables()->getFunctions().constEnd())
    {
        ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);
        QTableWidgetItem *item = new QTableWidgetItem(i.value().translate());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, ColumnName, item);
        item->setToolTip(i.value().getMdisambiguation());
        ++i;
    }

    ui->tableWidget->blockSignals(false);
    ui->tableWidget->selectRow(0);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

//---------------------------------------------------------------------------------------------------------------------
void EditFormulaDialog::filterVariables(const QString &filter)
{
    ui->tableWidget->blockSignals(true);

    // If filter is empty findItems() for unknown reason returns nullptr items.
    if (filter.isEmpty())
    {
        // show all rows
        for (auto i = 0; i < ui->tableWidget->rowCount(); ++i)
        {
            ui->tableWidget->showRow(i);
        }
    }
    else
    {
        // hide all rows
        for (auto i = 0; i < ui->tableWidget->rowCount(); i++)
        {
            ui->tableWidget->hideRow(i);
        }

        // show rows with matched filter
        for (auto item : ui->tableWidget->findItems(filter, Qt::MatchContains))
        {
            // If filter is empty findItems() for unknown reason returns nullptr items.
            if (item)
            {
                ui->tableWidget->showRow(item->row());
            }
        }
    }

    ui->tableWidget->blockSignals(false);
}

void EditFormulaDialog::clearFormula()
{
     ui->plainTextEditFormula->clear();
}

void EditFormulaDialog::undoFormula()
{
     ui->plainTextEditFormula->setPlainText(m_undoFormula);
}

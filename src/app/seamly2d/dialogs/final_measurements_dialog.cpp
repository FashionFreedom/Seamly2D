//---------------------------------------------------------------------------------------------------------------------
//  @file   final_measurements_dialog.cpp
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

#include "final_measurements_dialog.h"
#include "ui_final_measurements_dialog.h"
#include "../core/application_2d.h"
#include "../core/vformulapropertyeditor.h"
#include "../ifc/ifcdef.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/xml/vtoolrecord.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vsettings.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/tools/vdatatool.h"

#include <QCloseEvent>
#include <QGuiApplication>
#include <QHeaderView>
#include <QLayout>
#include <QRegularExpressionValidator>
#include <QScreen>
#include <QTableWidgetItem>
#include <QTimer>

namespace
{
enum FinalMeasurementsColumn
{
    ColumnName = 0,
    ColumnDescription,
    ColumnValue,
    ColumnFormula
};
}

//---------------------------------------------------------------------------------------------------------------------
FinalMeasurementsDialog::FinalMeasurementsDialog(VContainer *data, VPattern *doc, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FinalMeasurementsDialog)
    , m_doc(doc)
    , m_evalData(*data)
    , m_measurements(doc->getFinalMeasurements())
    , m_formulaEditor(nullptr)
    , m_isInitialized(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    m_formulaEditor = new VFormulaPropertyEditor(ui->formulaEditor_Widget);
    ui->formulaEditor_Widget->layout()->addWidget(m_formulaEditor);

    ui->name_LineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(
                                                                    QLatin1String("^$|") + NameRegExp()), this));

    ui->measurements_TableWidget->horizontalHeader()->setSectionResizeMode(ColumnFormula, QHeaderView::Stretch);

    connect(ui->measurements_TableWidget, &QTableWidget::itemSelectionChanged, this,
            &FinalMeasurementsDialog::showMeasurementDetails);
    connect(ui->filter_LineEdit,           &QLineEdit::textChanged,    this,
            &FinalMeasurementsDialog::filterMeasurements);
    connect(ui->add_ToolButton,            &QToolButton::clicked,      this, &FinalMeasurementsDialog::addMeasurement);
    connect(ui->remove_ToolButton,         &QToolButton::clicked,      this,
            &FinalMeasurementsDialog::removeMeasurement);
    connect(ui->moveUp_ToolButton,         &QToolButton::clicked,      this, &FinalMeasurementsDialog::moveUp);
    connect(ui->moveDown_ToolButton,       &QToolButton::clicked,      this, &FinalMeasurementsDialog::moveDown);
    connect(ui->name_LineEdit,             &QLineEdit::textEdited,     this, &FinalMeasurementsDialog::validateName);
    connect(ui->name_LineEdit,             &QLineEdit::editingFinished, this, &FinalMeasurementsDialog::saveName);
    connect(ui->description_PlainTextEdit, &QPlainTextEdit::textChanged, this,
            &FinalMeasurementsDialog::saveDescription);
    connect(ui->formula_PlainTextEdit,     &QPlainTextEdit::textChanged, this, &FinalMeasurementsDialog::saveFormula);
    connect(m_formulaEditor, &VFormulaPropertyEditor::dataChangedByUser, this,
            &FinalMeasurementsDialog::saveFormulaFromEditor);
    connect(ui->measurements_TableWidget->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this,
            &FinalMeasurementsDialog::updateMoveButtons);

    connect(m_doc, &VPattern::FullUpdateFromFile, this, &FinalMeasurementsDialog::fullUpdateFromFile);
    connect(m_doc, &VPattern::patternClosed,      this, [this](){ close(); });

    m_evalData = evaluationData(data, m_doc);
    fillTable();
    ui->measurements_TableWidget->sortByColumn(-1, Qt::AscendingOrder);
    selectIndex(m_measurements.isEmpty() ? -1 : 0);
    showMeasurementDetails();
}

//---------------------------------------------------------------------------------------------------------------------
FinalMeasurementsDialog::~FinalMeasurementsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::closeEvent(QCloseEvent *event)
{
    ui->name_LineEdit->blockSignals(true);
    ui->formula_PlainTextEdit->blockSignals(true);
    ui->description_PlainTextEdit->blockSignals(true);

    emit dialogClosed();
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        fullUpdateFromFile();
    }
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    if (event->spontaneous() || m_isInitialized)
    {
        return;
    }

    const QSize sz = qApp->Settings()->getFinalMeasurementsDialogSize();
    if (!sz.isEmpty())
    {
        resize(sz);
    }

    m_isInitialized = true;
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::resizeEvent(QResizeEvent *event)
{
    if (m_isInitialized)
    {
        qApp->Settings()->setFinalMeasurementsDialogSize(size());
    }
    QDialog::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::showMeasurementDetails()
{
    const int index = currentIndex();
    const bool hasSelection = index >= 0;

    ui->remove_ToolButton->setEnabled(hasSelection);
    updateMoveButtons();
    enableDetails(hasSelection);

    ui->name_LineEdit->blockSignals(true);
    ui->formula_PlainTextEdit->blockSignals(true);
    ui->description_PlainTextEdit->blockSignals(true);

    if (hasSelection)
    {
        const VFinalMeasurement &measurement = m_measurements.at(index);
        const VFormula formula = makeFormula(measurement.formula);

        ui->name_LineEdit->setText(measurement.name);
        ui->name_LineEdit->setStyleSheet(QString());
        ui->formula_PlainTextEdit->setPlainText(formula.GetFormula(FormulaType::ToUser));
        ui->description_PlainTextEdit->setPlainText(measurement.description);
        m_formulaEditor->SetFormula(formula);
    }
    else
    {
        ui->name_LineEdit->clear();
        ui->formula_PlainTextEdit->clear();
        ui->description_PlainTextEdit->clear();
        m_formulaEditor->SetFormula(makeFormula(QString()));
    }

    ui->name_LineEdit->blockSignals(false);
    ui->formula_PlainTextEdit->blockSignals(false);
    ui->description_PlainTextEdit->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::filterMeasurements(const QString &filterString)
{
    for (int row = 0; row < ui->measurements_TableWidget->rowCount(); ++row)
    {
        bool match = filterString.isEmpty();
        for (int column = 0; column < ui->measurements_TableWidget->columnCount() && !match; ++column)
        {
            const QTableWidgetItem *item = ui->measurements_TableWidget->item(row, column);
            match = item != nullptr && item->text().contains(filterString, Qt::CaseInsensitive);
        }
        ui->measurements_TableWidget->setRowHidden(row, !match);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::addMeasurement()
{
    VFinalMeasurement measurement;
    measurement.name = uniqueName();
    measurement.formula = QStringLiteral("0");

    const int current = currentIndex();
    const int index = current >= 0 ? current + 1 : m_measurements.size();
    m_measurements.insert(index, measurement);

    saveMeasurements();
    fillTable();
    selectIndex(index);
    ui->name_LineEdit->setFocus();
    ui->name_LineEdit->selectAll();
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::removeMeasurement()
{
    const int index = currentIndex();
    if (index < 0)
    {
        return;
    }

    m_measurements.remove(index);

    saveMeasurements();
    fillTable();
    selectIndex(m_measurements.isEmpty() ? -1 : qMin(index, m_measurements.size() - 1));
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::moveUp()
{
    const int index = currentIndex();
    if (index <= 0)
    {
        return;
    }

    m_measurements.move(index, index - 1);

    saveMeasurements();
    fillTable();
    selectIndex(index - 1);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::moveDown()
{
    const int index = currentIndex();
    if (index < 0 || index >= m_measurements.size() - 1)
    {
        return;
    }

    m_measurements.move(index, index + 1);

    saveMeasurements();
    fillTable();
    selectIndex(index + 1);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::validateName(const QString &text)
{
    const bool valid = isNameValid(text, currentIndex());
    ui->name_LineEdit->setStyleSheet(valid ? QString() : QStringLiteral("QLineEdit { color: red; }"));
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::saveName()
{
    const int index = currentIndex();
    if (index < 0)
    {
        return;
    }

    const QString name = ui->name_LineEdit->text();
    if (name == m_measurements.at(index).name)
    {
        return;
    }

    if (!isNameValid(name, index))
    {
        ui->name_LineEdit->blockSignals(true);
        ui->name_LineEdit->setText(m_measurements.at(index).name);
        ui->name_LineEdit->setStyleSheet(QString());
        ui->name_LineEdit->blockSignals(false);
        return;
    }

    m_measurements[index].name = name;
    saveMeasurements();
    updateRow(index);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::saveDescription()
{
    const int index = currentIndex();
    if (index < 0)
    {
        return;
    }

    m_measurements[index].description = ui->description_PlainTextEdit->toPlainText();
    saveMeasurements();
    updateRow(index);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::saveFormula()
{
    const int index = currentIndex();
    if (index < 0)
    {
        return;
    }

    QString text = ui->formula_PlainTextEdit->toPlainText();
    text.replace(QLatin1String("\n"), QLatin1String(" "));

    m_measurements[index].formula = VTranslateVars::TryFormulaFromUser(text, qApp->Settings()->getOsSeparator());
    saveMeasurements();
    updateRow(index);
    m_formulaEditor->SetFormula(makeFormula(m_measurements.at(index).formula));
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::saveFormulaFromEditor(const VFormula &formula)
{
    const int index = currentIndex();
    if (index < 0)
    {
        return;
    }

    m_measurements[index].formula = formula.GetFormula(FormulaType::FromUser);
    saveMeasurements();
    updateRow(index);

    ui->formula_PlainTextEdit->blockSignals(true);
    ui->formula_PlainTextEdit->setPlainText(formula.GetFormula(FormulaType::ToUser));
    ui->formula_PlainTextEdit->blockSignals(false);

    // The formula wizard is parented to the main window, which gets activated when the wizard closes.
    QTimer::singleShot(0, this, [this]()
    {
        raise();
        activateWindow();
    });
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::fullUpdateFromFile()
{
    const int index = currentIndex();

    m_measurements = m_doc->getFinalMeasurements();
    m_evalData = evaluationData(&m_evalData, m_doc);
    fillTable();
    selectIndex(m_measurements.isEmpty() ? -1 : qBound(0, index, m_measurements.size() - 1));
    showMeasurementDetails();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief evaluationData returns a copy of the data of the last tool in the pattern history, so formulas have access
/// to every object of the pattern: line lengths and angles, curve lengths and angles, custom variables, measurements,
/// etc. Falls back to @p data when the history has no tool.
//---------------------------------------------------------------------------------------------------------------------
VContainer FinalMeasurementsDialog::evaluationData(const VContainer *data, VPattern *doc)
{
    VContainer evalData(*data);
    const QVector<VToolRecord> *history = doc->getHistory();
    for (int i = history->size() - 1; i >= 0; --i)
    {
        try
        {
            const VDataTool *tool = VAbstractPattern::getTool(history->at(i).getId());
            if (tool != nullptr)
            {
                evalData = tool->getData();
                break;
            }
        }
        catch (const VExceptionBadId &)
        {
            continue;
        }
    }

    evalData.RemoveVariable(currentLength);
    evalData.RemoveVariable(currentSeamAllowance);
    return evalData;
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::fillTable()
{
    QTableWidget *table = ui->measurements_TableWidget;
    const bool sortingEnabled = table->isSortingEnabled();

    table->blockSignals(true);
    table->setSortingEnabled(false);
    table->clearContents();
    table->setRowCount(m_measurements.size());

    for (int i = 0; i < m_measurements.size(); ++i)
    {
        for (int column = ColumnName; column <= ColumnFormula; ++column)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setData(Qt::UserRole, i);
            if (column == ColumnValue)
            {
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
            }
            table->setItem(i, column, item);
        }
        updateRow(i);
    }

    table->setSortingEnabled(sortingEnabled);
    table->blockSignals(false);

    table->resizeColumnToContents(ColumnName);
    table->resizeColumnToContents(ColumnValue);
    table->resizeRowsToContents();

    filterMeasurements(ui->filter_LineEdit->text());
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::updateRow(int index)
{
    const int row = rowForIndex(index);
    if (row < 0)
    {
        return;
    }

    QTableWidget *table = ui->measurements_TableWidget;
    const bool sortingEnabled = table->isSortingEnabled();
    table->blockSignals(true);
    table->setSortingEnabled(false);

    const VFinalMeasurement &measurement = m_measurements.at(index);
    const VFormula formula = makeFormula(measurement.formula);

    table->item(row, ColumnName)->setText(measurement.name);
    table->item(row, ColumnName)->setToolTip(measurement.name);
    table->item(row, ColumnDescription)->setText(measurement.description);
    table->item(row, ColumnDescription)->setToolTip(measurement.description);
    table->item(row, ColumnValue)->setText(formula.getStringValue());
    table->item(row, ColumnFormula)->setText(formula.GetFormula(FormulaType::ToUser));
    table->item(row, ColumnFormula)->setToolTip(formula.GetFormula(FormulaType::ToUser));

    table->setSortingEnabled(sortingEnabled);
    table->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
int FinalMeasurementsDialog::rowForIndex(int index) const
{
    const QTableWidget *table = ui->measurements_TableWidget;
    for (int row = 0; row < table->rowCount(); ++row)
    {
        const QTableWidgetItem *item = table->item(row, ColumnName);
        if (item != nullptr && item->data(Qt::UserRole).toInt() == index)
        {
            return row;
        }
    }
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
int FinalMeasurementsDialog::currentIndex() const
{
    const QTableWidget *table = ui->measurements_TableWidget;
    const int row = table->currentRow();
    if (row < 0 || table->selectedItems().isEmpty())
    {
        return -1;
    }

    const QTableWidgetItem *item = table->item(row, ColumnName);
    if (item == nullptr)
    {
        return -1;
    }

    const int index = item->data(Qt::UserRole).toInt();
    return (index >= 0 && index < m_measurements.size()) ? index : -1;
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::selectIndex(int index)
{
    QTableWidget *table = ui->measurements_TableWidget;
    const int row = rowForIndex(index);
    if (row < 0)
    {
        table->clearSelection();
        table->setCurrentCell(-1, -1);
        showMeasurementDetails();
        return;
    }
    table->selectRow(row);
    table->scrollToItem(table->item(row, ColumnName));
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::enableDetails(bool enabled)
{
    ui->name_LineEdit->setEnabled(enabled);
    ui->formulaEditor_Widget->setEnabled(enabled);
    m_formulaEditor->setVisible(enabled);
    ui->formula_PlainTextEdit->setEnabled(enabled);
    ui->description_PlainTextEdit->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::updateMoveButtons()
{
    const int index = currentIndex();
    const bool isSorted = ui->measurements_TableWidget->horizontalHeader()->sortIndicatorSection() >= 0;

    ui->moveUp_ToolButton->setEnabled(!isSorted && index > 0);
    ui->moveDown_ToolButton->setEnabled(!isSorted && index >= 0 && index < m_measurements.size() - 1);
}

//---------------------------------------------------------------------------------------------------------------------
void FinalMeasurementsDialog::saveMeasurements()
{
    m_doc->setFinalMeasurements(m_measurements);
}

//---------------------------------------------------------------------------------------------------------------------
bool FinalMeasurementsDialog::isNameValid(const QString &name, int index) const
{
    if (name.isEmpty() || !QRegularExpression(QStringLiteral("^") + NameRegExp() + QStringLiteral("$")).match(name)
                                                                                                        .hasMatch())
    {
        return false;
    }

    if (m_evalData.DataVariables()->contains(name))
    {
        return false;
    }

    for (int i = 0; i < m_measurements.size(); ++i)
    {
        if (i != index && m_measurements.at(i).name == name)
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
QString FinalMeasurementsDialog::uniqueName() const
{
    int number = m_measurements.size() + 1;
    QString name;
    do
    {
        name = QStringLiteral("M_%1").arg(number++);
    } while (!isNameValid(name, -1));

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
VFormula FinalMeasurementsDialog::makeFormula(const QString &formula)
{
    VFormula result(formula, &m_evalData);
    result.setCheckZero(false);
    result.setToolId(NULL_ID);
    result.setPostfix(UnitsToStr(qApp->patternUnit()));
    result.Eval();
    return result;
}

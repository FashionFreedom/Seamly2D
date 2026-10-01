//---------------------------------------------------------------------------------------------------------------------
//  @file   variables_dialog.cpp
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
//  @file   dialogvariables.cpp
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

#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/vsettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/dialogs/support/edit_formula_dialog.h"
#include "../vwidgets/vwidgetpopup.h"
#include "ui_variables_dialog.h"
#include "variables_dialog.h"

#include <QFileDialog>
#include <QDir>
#include <QGuiApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QTabBar>
#include <QTableWidget>
#include <QScreen>
#include <QSettings>
#include <QTableWidgetItem>
#include <QtNumeric>

#define DIALOG_MAX_FORMULA_HEIGHT 64

//---------------------------------------------------------------------------------------------------------------------
/// @brief VariablesDialog create dialog
/// @param data container with data
/// @param doc dom document container
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
VariablesDialog::VariablesDialog(VContainer *data, VPattern *doc, QWidget *parent)
    : DialogTool(data, NULL_ID, parent)
    , ui(new Ui::VariablesDialog)
    , m_data(data)
    , m_doc(doc)
    , m_backup_table(new QTableWidget(this))
    , m_reset_formula(QString())
    , m_reset_description(QString())
    , m_has_changes(false)
    , m_rename_list()
    , m_table_list()
    , m_is_sorted(false)
    , m_is_filtered(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    ui->filter_LineEdit->installEventFilter(this);
    ui->formula_PlainTextEdit->installEventFilter(this);

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    qCDebug(vDialog, "Showing variables.");
    showUnits();

    const bool freshCall = true;
    fillCustomVariables(freshCall);
    fillLineLengths();
    fillLineAngles();
    fillCurveLengths();
    fillControlPointLengths();
    fillArcsRadiuses();
    fillCurveAngles();

    m_table_list.append(QSharedPointer<QTableWidget>(ui->variables_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->lineLengths_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->lineAngles_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->curveLengths_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->curveAngles_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->controlPointLengths_TableWidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->arcRadiuses_TableWidget));

    connect(m_doc, &VPattern::FullUpdateFromFile, this, &VariablesDialog::FullUpdateFromFile);
    connect(m_doc, &VPattern::patternClosed,      this, [this](){ close(); });

    ui->tabWidget->setCurrentIndex(0);
    ui->name_LineEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(
                                                                        QLatin1String("^$|")+NameRegExp()), this));

    connect(ui->variables_TableWidget, &QTableWidget::itemSelectionChanged, this,
            &VariablesDialog::showCustomVariableDetails);

    connect(ui->addCustomVariable_ToolButton, &QPushButton::clicked, this, &VariablesDialog::addCustomVariable);
    connect(ui->removeCustomVariable_ToolButton, &QToolButton::clicked, this, &VariablesDialog::removeCustomVariable);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &VariablesDialog::moveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &VariablesDialog::moveDown);
    connect(ui->formula_ToolButton, &QToolButton::clicked, this, &VariablesDialog::Fx);
    connect(ui->name_LineEdit, &QLineEdit::textEdited, this, &VariablesDialog::saveCustomVariableName);
    connect(ui->description_PlainTextEdit, &QPlainTextEdit::textChanged, this, &VariablesDialog::saveCustomVariableDescription);
    connect(ui->formula_PlainTextEdit, &QPlainTextEdit::textChanged, this, &VariablesDialog::saveCustomVariableFormula);

    connect(ui->filter_LineEdit, &QLineEdit::textChanged, this, &VariablesDialog::filterVariables);

    connect(ui->refresh_PushButton,  &QPushButton::clicked, this, &VariablesDialog::refreshPattern);
    connect(ui->undo_all_pushbutton, &QPushButton::clicked, this, &VariablesDialog::undoButtonClicked);

    connect(ui->variables_TableWidget->horizontalHeader(), &QHeaderView::sectionClicked, [this]()
    {
        m_is_sorted = true;
        setMoveControls();
    });



    if (ui->variables_TableWidget->rowCount() > 0)
    {
        ui->variables_TableWidget->selectRow(0);
    }

    // clear text filter string every time a new tab is selected
    auto clearFilterString = [this] ()
    {
        ui->filter_LineEdit->clear();
        m_is_filtered = false;

        if (ui->tabWidget->currentIndex() == 0)
        {
            filterVariables("");
            ui->variables_TableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
            m_is_sorted = false;
            setMoveControls();
        }
    };
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, clearFilterString);

    connect(ui->clear_fx_pushbutton,   &QPushButton::clicked, this, &VariablesDialog::clearFormula);
    connect(ui->reset_pushbutton,      &QPushButton::clicked, this, &VariablesDialog::resetFormula);
    connect(ui->clear_desc_pushbutton, &QPushButton::clicked, this, &VariablesDialog::clearDescription);
    connect(ui->reset_desc_pushbutton, &QPushButton::clicked, this, &VariablesDialog::resetDescription);

}

//---------------------------------------------------------------------------------------------------------------------
VariablesDialog::~VariablesDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCustomVariables fill data for variables table
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCustomVariables(bool freshCall)
{
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->clearContents();

    const QMap<QString, QSharedPointer<CustomVariable> > variables = m_data->variablesData();
    QMap<QString, QSharedPointer<CustomVariable> >::const_iterator i;
    QMap<quint32, QString> map;
    //Sorting QHash by id
    for (i = variables.constBegin(); i != variables.constEnd(); ++i)
    {
        QSharedPointer<CustomVariable> variable = i.value();
        map.insert(variable->getIndex(), i.key());
    }

    qint32 currentRow = -1;
    QMapIterator<quint32, QString> iMap(map);
    ui->variables_TableWidget->setRowCount ( variables.size() );
    while (iMap.hasNext())
    {
        iMap.next();
        QSharedPointer<CustomVariable> variable = variables.value(iMap.value());
        currentRow++;

        addCell(ui->variables_TableWidget, variable->GetName(), currentRow, 0, Qt::AlignVCenter); // name
        addCell(ui->variables_TableWidget, variable->GetDescription(), currentRow, 1, Qt::AlignVCenter); // description
        addCell(ui->variables_TableWidget, qApp->LocaleToString(*variable->GetValue()), currentRow, 2,
                Qt::AlignHCenter | Qt::AlignVCenter, variable->IsFormulaOk()); // calculated value

        QString formula;
        try
        {
            formula = qApp->translateVariables()->FormulaToUser(variable->GetFormula(), qApp->Settings()->getOsSeparator());
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            formula = variable->GetFormula();
        }

        addCell(ui->variables_TableWidget, formula, currentRow, 3, Qt::AlignVCenter); // formula
    }

    if (freshCall)
    {
        ui->variables_TableWidget->resizeColumnsToContents();
        ui->variables_TableWidget->resizeRowsToContents();
        ui->variables_TableWidget->setColumnWidth(1, 350);
    }

    ui->variables_TableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->variables_TableWidget->blockSignals(false);

    copyTable(ui->variables_TableWidget, m_backup_table);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VariablesDialog::fillTable(const QMap<QString, T> &varTable, QTableWidget *table)
{
    SCASSERT(table != nullptr)

    qint32 currentRow = -1;
    QMapIterator<QString, T> i(varTable);
    while (i.hasNext())
    {
        i.next();
        qreal length = *i.value()->GetValue();
        currentRow++;
        table->setRowCount ( varTable.size() );

        QTableWidgetItem *item = new QTableWidgetItem(i.key());
        item->setTextAlignment(Qt::AlignLeft);
        table->setItem(currentRow, 0, item);

        item = new QTableWidgetItem(qApp->LocaleToString(length));
        item->setTextAlignment(Qt::AlignHCenter);
        table->setItem(currentRow, 1, item);
    }
    table->resizeColumnsToContents();
    table->resizeRowsToContents();
    table->verticalHeader()->setDefaultSectionSize(20);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief FillLengthLines fill variables table with data for line lengths
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillLineLengths()
{
    fillTable(m_data->lineLengthsData(), ui->lineLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillLineAngles fill variables table with data for line angles.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillLineAngles()
{
    fillTable(m_data->lineAnglesData(), ui->lineAngles_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCurveLengths fill variables table with data for curve lengths.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCurveLengths()
{
    fillTable(m_data->curveLengthsData(), ui->curveLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillControlPointLengths fill variables table with data for control point lengths.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillControlPointLengths()
{
    fillTable(m_data->controlPointLengthsData(), ui->controlPointLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillArcsRadiuses fill variables table with data for arc radii.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillArcsRadiuses()
{
    fillTable(m_data->arcRadiusesData(), ui->arcRadiuses_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCurveAngles fill variables table with data for curve angles.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCurveAngles()
{
    fillTable(m_data->curveAnglesData(), ui->curveAngles_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showUnits()
{
    const QString unit = UnitsToStr(qApp->patternUnit());

    showHeaderUnits(ui->variables_TableWidget, 2, unit);           // calculated value
    showHeaderUnits(ui->variables_TableWidget, 3, unit);           // formula

    showHeaderUnits(ui->lineLengths_TableWidget, 1, unit);         // line lengths
    showHeaderUnits(ui->lineAngles_TableWidget, 1, degreeSymbol);  // line angle
    showHeaderUnits(ui->curveLengths_TableWidget, 1, unit);        // curve lengths
    showHeaderUnits(ui->curveAngles_TableWidget, 1, degreeSymbol); // curve angle
    showHeaderUnits(ui->controlPointLengths_TableWidget, 1, unit); // CP lengths
    showHeaderUnits(ui->arcRadiuses_TableWidget, 1, unit);         // arc radii
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
    SCASSERT(table != nullptr)

    QString header = table->horizontalHeaderItem(column)->text();
    // Need to strip text of any umits so we don't recursively add units to the header string
    header = header.section('(', 0, 0);
    const QString unitHeader = QString("%1 (%2)").arg(header).arg(unit);
    table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::addCell(QTableWidget *table, const QString &text, int row, int column, int aligment, bool ok)
{
    SCASSERT(table != nullptr)

    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(aligment);

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);
    item->setToolTip(text);

    if (!ok)
    {
        QBrush brush = item->foreground();
        brush.setColor(Qt::red);
        item->setForeground(brush);
    }

    table->setItem(row, column, item);
}

//---------------------------------------------------------------------------------------------------------------------
QString VariablesDialog::getCustomVariableName() const
{
    qint32 num = 1;
    QString name;
    do
    {
        name = CustomIncrSign + qApp->translateVariables()->InternalVarToUser(variable_) + QString().number(num);
        num++;
    } while (!m_data->IsUnique(name));
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
QString VariablesDialog::clearCustomVariableName(const QString &name) const
{
    QString clear = name;
    const int index = clear.indexOf(CustomIncrSign);
    if (index == 0)
    {
        clear.remove(0, 1);
    }
    return clear;
}

//---------------------------------------------------------------------------------------------------------------------
bool VariablesDialog::evalVariableFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label)
{
    const QString postfix = UnitsToStr(qApp->patternUnit());//Show unit in dialog label (cm, mm or inch)
    if (formula.isEmpty())
    {
        label->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        label->setToolTip(tr("Empty field"));
        return false;
    }
    else
    {
        try
        {
            QString f;
            // Replace line return character with spaces for calc if exist
            if (fromUser)
            {
                f = qApp->translateVariables()->FormulaFromUser(formula, qApp->Settings()->getOsSeparator());
            }
            else
            {
                f = formula;
            }
            f.replace("\n", " ");
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(m_data->DataVariables(), f);

            if (qIsInf(result) || qIsNaN(result))
            {
                label->setText(tr("Error") + " (" + postfix + ").");
                label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
                return false;
            }

            label->setText(qApp->LocaleToString(result) + " " + postfix);
            label->setToolTip(tr("Value"));
            return true;
        }
        catch (qmu::QmuParserError &error)
        {
            label->setText(tr("Error") + " (" + postfix + "). " + tr("Parser error: %1").arg(error.GetMsg()));
            label->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
            return false;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::setMoveControls()
{
    if (m_is_sorted || m_is_filtered)
    {
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
        return;
    }

    if (ui->variables_TableWidget->rowCount() > 0)
    {
        const QTableWidgetItem *name = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        SCASSERT(name != nullptr)

        ui->removeCustomVariable_ToolButton->setEnabled(!variableUsed(name->text()));
    }
    else
    {
        ui->removeCustomVariable_ToolButton->setEnabled(false);
    }

    if (ui->variables_TableWidget->rowCount() >= 2)
    {
        if (ui->variables_TableWidget->currentRow() == 0)
        {
            ui->toolButtonUp->setEnabled(false);
            ui->toolButtonDown->setEnabled(true);
        }
        else if (ui->variables_TableWidget->currentRow() == ui->variables_TableWidget->rowCount()-1)
        {
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(false);
        }
        else
        {
            ui->toolButtonUp->setEnabled(true);
            ui->toolButtonDown->setEnabled(true);
        }
    }
    else
    {
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::enablePieces(bool enabled)
{
    if (enabled)
    {
        setMoveControls();
    }
    else
    {
        ui->removeCustomVariable_ToolButton->setEnabled(enabled);

        ui->toolButtonUp->setEnabled(enabled);
        ui->toolButtonDown->setEnabled(enabled);
    }

    if (!enabled)
    { // Clear
        ui->name_LineEdit->blockSignals(true);
        ui->name_LineEdit->clear();
        ui->name_LineEdit->blockSignals(false);

        ui->description_PlainTextEdit->blockSignals(true);
        ui->description_PlainTextEdit->clear();
        ui->description_PlainTextEdit->blockSignals(false);

        ui->calculatedValue_Label->blockSignals(true);
        ui->calculatedValue_Label->clear();
        ui->calculatedValue_Label->blockSignals(false);

        ui->formula_PlainTextEdit->blockSignals(true);
        ui->formula_PlainTextEdit->clear();
        ui->formula_PlainTextEdit->blockSignals(false);
    }

    ui->formula_ToolButton->setEnabled(enabled);
    ui->name_LineEdit->setEnabled(enabled);
    ui->description_PlainTextEdit->setEnabled(enabled);
    ui->formula_PlainTextEdit->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::localUpdateTree()
{
    m_doc->LiteParseVariables();
    fillCustomVariables();
}

//---------------------------------------------------------------------------------------------------------------------
bool VariablesDialog::variableUsed(const QString &name) const
{
    const QVector<VFormulaField> expressions = m_doc->ListExpressions();

    for(int i = 0; i < expressions.size(); ++i)
    {
        if (expressions.at(i).expression.indexOf(name) != -1)
        {
            // Eval formula
            try
            {
                QScopedPointer<qmu::QmuTokenParser> cal(new qmu::QmuTokenParser(expressions.at(i).expression, false,
                                                                                false));

                // Tokens (variables, measurements)
                if (cal->GetTokens().values().contains(name))
                {
                    return true;
                }
            }
            catch (const qmu::QmuParserError &)
            {
                // Do nothing. Because we not sure if used. A formula is broken.
            }
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::renameCache(const QString &name, const QString &newName)
{
    for (int i = 0; i < m_rename_list.size(); ++i)
    {
        if (m_rename_list.at(i).second == name)
        {
            m_rename_list[i].second = newName;
            return;
        }
    }

    m_rename_list.append(qMakePair(name, newName));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief FullUpdateFromFile update information in tables form file
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::FullUpdateFromFile()
{
    m_has_changes = false;

    ui->lineLengths_TableWidget->clearContents();
    ui->curveLengths_TableWidget->clearContents();
    ui->curveAngles_TableWidget->clearContents();
    ui->lineAngles_TableWidget->clearContents();
    ui->arcRadiuses_TableWidget->clearContents();

    fillCustomVariables();
    fillLineLengths();
    fillLineAngles();
    fillCurveLengths();
    fillControlPointLengths();
    fillArcsRadiuses();
    fillCurveAngles();
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::refreshPattern()
{
    if (m_has_changes)
    {
        QVector<VFormulaField> expressions = m_doc->ListExpressions();
        for (int i = 0; i < m_rename_list.size(); ++i)
        {
            m_doc->replaceNameInFormula(expressions, m_rename_list.at(i).first, m_rename_list.at(i).second);
        }
        m_rename_list.clear();

        const int row = ui->variables_TableWidget->currentRow();

        m_doc->LiteParseTree(Document::LiteParse);

        ui->variables_TableWidget->blockSignals(true);
        ui->variables_TableWidget->selectRow(row);
        ui->variables_TableWidget->blockSignals(false);

        m_has_changes = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief clickedToolButtonAdd create new row in table
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::addCustomVariable()
{
    qCDebug(vDialog, "Add a new custom variable");

    const QString name = getCustomVariableName();
    qint32 currentRow = -1;

    if (ui->variables_TableWidget->currentRow() == -1)
    {
        currentRow  = ui->variables_TableWidget->rowCount();
        m_doc->addEmptyCustomVariable(name);
    }
    else
    {
        currentRow  = ui->variables_TableWidget->currentRow()+1;
        const QTableWidgetItem *item = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        m_doc->addEmptyCustomVariableAfter(item->text(), name);
    }

    m_has_changes = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(currentRow);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief clickedToolButtonRemove remove one row from table
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::removeCustomVariable()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    m_doc->removeCustomVariable(name->text());

    m_has_changes = true;
    localUpdateTree();

    if (ui->variables_TableWidget->rowCount() > 0)
    {
        ui->variables_TableWidget->selectRow(0);
    }
    else
    {
        enablePieces(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::moveUp()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    m_doc->moveVariableUp(name->text());

    m_has_changes = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(row-1);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::moveDown()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    m_doc->moveVariableDown(name->text());

    m_has_changes = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(row+1);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveCustomVariableName(const QString &text)
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);

    QString newName = text.isEmpty() ? getCustomVariableName() : CustomIncrSign + text;

    if (!m_data->IsUnique(newName))
    {
        qint32 num = 2;
        QString tempName = newName;
        do
        {
            tempName = tempName + QLatin1String("_") + QString().number(num);
            num++;
        } while (!m_data->IsUnique(tempName));
        newName = tempName;
    }

    m_doc->setVariableName(name->text(), newName);
    QVector<VFormulaField> expressions = m_doc->listVariableExpressions();
    m_doc->replaceNameInFormula(expressions, name->text(), newName);
    renameCache(name->text(), newName);

    m_has_changes = true;
    localUpdateTree();

    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveCustomVariableDescription()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    m_doc->setVariableDescription(name->text(), ui->description_PlainTextEdit->toPlainText());

    localUpdateTree();

    const QTextCursor cursor = ui->description_PlainTextEdit->textCursor();
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
    ui->description_PlainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveCustomVariableFormula()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);

    // Replace line return character with spaces for calc if exist
    QString text = ui->formula_PlainTextEdit->toPlainText();
    text.replace("\n", " ");

    QTableWidgetItem *formula = ui->variables_TableWidget->item(row, 2);
    if (formula->text() == text)
    {
        QTableWidgetItem *result = ui->variables_TableWidget->item(row, 1);
        //Show unit in dialog label (cm, mm or inch)
        const QString postfix = UnitsToStr(qApp->patternUnit());
        ui->calculatedValue_Label->setText(result->text() + " " +postfix);
        return;
    }

    if (text.isEmpty())
    {
        //Show unit in dialog label (cm, mm or inch)
        const QString postfix = UnitsToStr(qApp->patternUnit());
        ui->calculatedValue_Label->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        return;
    }

    QSharedPointer<CustomVariable> variable = m_data->getVariable<CustomVariable>(name->text());
    if (!evalVariableFormula(text, true, variable->GetData(), ui->calculatedValue_Label))
    {
        return;
    }

    try
    {
        const QString formula = qApp->translateVariables()->FormulaFromUser(text, qApp->Settings()->getOsSeparator());
        m_doc->setVariableFormula(name->text(), formula);
    }
    catch (qmu::QmuParserError &error) // Just in case something bad will happen
    {
        Q_UNUSED(error)
        return;
    }

    m_has_changes = true;
    localUpdateTree();

    const QTextCursor cursor = ui->formula_PlainTextEdit->textCursor();
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
    ui->formula_PlainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::Fx()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    QSharedPointer<CustomVariable> variable = m_data->getVariable<CustomVariable>(name->text());

    EditFormulaDialog *dialog = new EditFormulaDialog(variable->GetData(), NULL_ID, VariableDialog, this);
    dialog->setWindowTitle(tr("Edit variable"));
    dialog->SetFormula(qApp->translateVariables()->TryFormulaFromUser(ui->formula_PlainTextEdit->toPlainText().replace("\n", " "),
                                                          qApp->Settings()->getOsSeparator()));
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    dialog->setPostfix(postfix);//Show unit in dialog label (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
        m_doc->setVariableFormula(name->text(), dialog->GetFormula());

        m_has_changes = true;
        localUpdateTree();

        ui->variables_TableWidget->selectRow(row);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::closeEvent(QCloseEvent *event)
{
    refreshPattern();

    ui->formula_PlainTextEdit->blockSignals(true);
    ui->name_LineEdit->blockSignals(true);
    ui->description_PlainTextEdit->blockSignals(true);

    emit updateProperties();
    emit DialogClosed(QDialog::Accepted);
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        showUnits();
        FullUpdateFromFile();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
bool VariablesDialog::eventFilter(QObject *object, QEvent *event)
{
    if (QLineEdit *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                QString separator = qApp->Settings()->getOsSeparator()
                                  ? QString(localeDecimalPoint(QLocale()))
                                  : QString(localeDecimalPoint(QLocale::c()));

                textEdit->insert(separator);
                return true;
            }
        }
    }

    return DialogTool::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showEvent(QShowEvent *event)
{
    // Skip DialogTool implementation
    QDialog::showEvent(event);
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }

    // Initialize Dialog to last used size.
    const QSize sz = qApp->Settings()->getVariablesDialogSize();
    if (!sz.isEmpty())
    {
        resize(sz);
    }

    isInitialized = true; //first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::resizeEvent(QResizeEvent *event)
{
    // Save the size for the next time this dialog is opened, but only
    // if dialog was already initialized, which rules out the resize at
    // dialog creation.
    if (isInitialized)
    {
        qApp->Settings()->setVariablesDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showCustomVariableDetails()
{
    if (ui->variables_TableWidget->rowCount() > 0)
    {
        enablePieces(true);

        // name
        const QTableWidgetItem *name = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        QSharedPointer<CustomVariable> variable;

        try
        {
            variable = m_data->getVariable<CustomVariable>(name->text());
        }
        catch(const VExceptionBadId &error)
        {
            Q_UNUSED(error)
            enablePieces(false);
            return;
        }

        ui->name_LineEdit->setText(clearCustomVariableName(variable->GetName()));

        ui->description_PlainTextEdit->blockSignals(true);
        ui->description_PlainTextEdit->setPlainText(variable->GetDescription());
        m_reset_description = variable->GetDescription();
        ui->description_PlainTextEdit->blockSignals(false);

        evalVariableFormula(variable->GetFormula(), false, variable->GetData(), ui->calculatedValue_Label);
        ui->formula_PlainTextEdit->blockSignals(true);

        QString formula;
        try
        {
            formula = qApp->translateVariables()->FormulaToUser(variable->GetFormula(), qApp->Settings()->getOsSeparator());
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            formula = variable->GetFormula();
        }

        ui->formula_PlainTextEdit->setPlainText(formula);
        m_reset_formula = formula;
        ui->formula_PlainTextEdit->blockSignals(false);
    }
    else
    {
        enablePieces(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::filterVariables(const QString &filterString)
{
    QSharedPointer<QTableWidget> currentTable = m_table_list.value(ui->tabWidget->currentIndex());
    currentTable->blockSignals(true);

    if (filterString.isEmpty())
    {
        m_is_filtered = false;
        for (auto i = 0; i < currentTable->rowCount(); ++i)
        {
            currentTable->showRow(i);
        }
        ui->variables_TableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    }
    else
    {
        m_is_filtered = true;
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
        for (auto i = 0; i < currentTable->rowCount(); i++)
        {
            currentTable->hideRow(i);
        }

        for (auto item : currentTable->findItems(filterString, Qt::MatchContains))
        {
            if (item)
            {
                currentTable->showRow(item->row());
            }
        }
    }

    currentTable->blockSignals(false);
}

void VariablesDialog::clearFormula()
{
     ui->formula_PlainTextEdit->clear();
     ui->reset_pushbutton->setEnabled(true);
}

void VariablesDialog::resetFormula()
{
     ui->formula_PlainTextEdit->setPlainText(m_reset_formula);
     ui->reset_pushbutton->setEnabled(false);
}

void VariablesDialog::clearDescription()
{
     ui->description_PlainTextEdit->clear();
     ui->reset_desc_pushbutton->setEnabled(true);
}

void VariablesDialog::resetDescription()
{
     ui->description_PlainTextEdit->setPlainText(m_reset_description);
     ui->reset_desc_pushbutton->setEnabled(false);
}

void  VariablesDialog::copyTable(const QTableWidget* source, QTableWidget* target)
{
    int rows = source->rowCount();
    int cols = source->columnCount();

    target->setRowCount(rows);
    target->setColumnCount(cols);

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            QTableWidgetItem* sourceItem = source->item(r, c);
            if (sourceItem != nullptr)
            {
                // Create a fresh item using only the text from the source cell
                target->setItem(r, c, new QTableWidgetItem(sourceItem->text()));
            }
            else
            {
                // Clear the destination cell if the source is empty
                target->setItem(r, c, nullptr);
            }
        }
    }
}

void VariablesDialog::undoButtonClicked()
{
    copyTable(m_backup_table, ui->variables_TableWidget);
}

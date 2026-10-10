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
    , m_has_changes(false)
    , m_rename_list()
    , m_table_list()
    , m_is_sorted(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    ui->filter_line_edit->installEventFilter(this);
    ui->formula_plaintextedit->installEventFilter(this);

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    qCDebug(vDialog, "Showing variables.");
    showUnits();

    const bool fresh_call = true;
    fillCustomVariables(fresh_call);
    fillLineLengths();
    fillLineAngles();
    fillCurveLengths();
    fillControlPointLengths();
    fillArcsRadiuses();
    fillCurveAngles();

    m_table_list.append(QSharedPointer<QTableWidget>(ui->variables_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->line_lengths_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->line_angles_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->curve_lengths_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->curve_angles_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->handle_lengths_tablewidget));
    m_table_list.append(QSharedPointer<QTableWidget>(ui->arc_radii_tablewidget));

    connect(m_doc, &VPattern::FullUpdateFromFile, this, &VariablesDialog::FullUpdateFromFile);
    connect(m_doc, &VPattern::patternClosed,      this, [this](){ close(); });

    ui->tab_widget->setCurrentIndex(0);
    QString expression = QLatin1String("^$|") + NameRegExp();
    ui->name_line_edit->setValidator(new QRegularExpressionValidator(QRegularExpression(expression), this));

    connect(ui->variables_tablewidget, &QTableWidget::itemSelectionChanged, this,
            &VariablesDialog::showCustomVariables);

    connect(ui->add_variable_toolbutton,    &QPushButton::clicked,        this, &VariablesDialog::addVariable);
    connect(ui->remove_variable_toolbutton, &QToolButton::clicked,        this, &VariablesDialog::removeVariable);
    connect(ui->up_toolbutton,              &QToolButton::clicked,        this, &VariablesDialog::moveUp);
    connect(ui->down_toolbutton,            &QToolButton::clicked,        this, &VariablesDialog::moveDown);
    connect(ui->formula_toolbutton,         &QToolButton::clicked,        this, &VariablesDialog::editFormula);
    connect(ui->name_line_edit,             &QLineEdit::textEdited,       this, &VariablesDialog::saveVariableName);
    connect(ui->description_plaintextedit,  &QPlainTextEdit::textChanged, this, &VariablesDialog::saveDescription);
    connect(ui->formula_plaintextedit,      &QPlainTextEdit::textChanged, this, &VariablesDialog::saveVariableFormula);
    connect(ui->filter_line_edit,           &QLineEdit::textChanged,      this, &VariablesDialog::filterVariables);
    connect(ui->refresh_pushbutton,         &QPushButton::clicked,        this, &VariablesDialog::refreshPattern);

    connect(ui->variables_tablewidget->horizontalHeader(), &QHeaderView::sectionClicked, [this]()
    {
        m_is_sorted = true;
        setMoveControls();
    });

    if (ui->variables_tablewidget->rowCount() > 0)
    {
        ui->variables_tablewidget->selectRow(0);
    }

    // clear text filter string every time a new tab is selected
    auto clearFilterString = [this] ()
    {
        ui->filter_line_edit->clear();
        m_is_filtered = false;

        if (ui->tab_widget->currentIndex() == 0)
        {
            filterVariables("");
            ui->variables_tablewidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
            m_is_sorted = false;
            setMoveControls();
        }
    };
    connect(ui->tab_widget, &QTabWidget::currentChanged, this, clearFilterString);
}

//---------------------------------------------------------------------------------------------------------------------
VariablesDialog::~VariablesDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCustomVariables fill data for variables table
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCustomVariables(bool fresh_call)
{
    ui->variables_tablewidget->blockSignals(true);
    ui->variables_tablewidget->clearContents();

    const QMap<QString, QSharedPointer<CustomVariable> > variables = m_data->variablesData();
    QMap<QString, QSharedPointer<CustomVariable> >::const_iterator i;
    QMap<quint32, QString> map;
    //Sorting QHash by id
    for (i = variables.constBegin(); i != variables.constEnd(); ++i)
    {
        QSharedPointer<CustomVariable> variable = i.value();
        map.insert(variable->getIndex(), i.key());
    }

    qint32 current_row = -1;
    QMapIterator<quint32, QString> iMap(map);
    ui->variables_tablewidget->setRowCount ( variables.size() );
    while (iMap.hasNext())
    {
        iMap.next();
        QSharedPointer<CustomVariable> variable = variables.value(iMap.value());
        current_row++;

        addCell(ui->variables_tablewidget, variable->GetName(), current_row, 0, Qt::AlignVCenter); // name
        addCell(ui->variables_tablewidget, variable->GetDescription(), current_row, 1, Qt::AlignVCenter); // description
        addCell(ui->variables_tablewidget, qApp->LocaleToString(*variable->GetValue()), current_row, 2,
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

        addCell(ui->variables_tablewidget, formula, current_row, 3, Qt::AlignVCenter); // formula
    }

    if (fresh_call)
    {
        ui->variables_tablewidget->resizeColumnsToContents();
        ui->variables_tablewidget->resizeRowsToContents();
        ui->variables_tablewidget->setColumnWidth(1, 350);
    }

    ui->variables_tablewidget->horizontalHeader()->setStretchLastSection(true);
    ui->variables_tablewidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VariablesDialog::fillTable(const QMap<QString, T> &varTable, QTableWidget *table)
{
    SCASSERT(table != nullptr)

    qint32 current_row = -1;
    QMapIterator<QString, T> i(varTable);
    while (i.hasNext())
    {
        i.next();
        qreal length = *i.value()->GetValue();
        current_row++;
        table->setRowCount ( varTable.size() );

        QTableWidgetItem *item = new QTableWidgetItem(i.key());
        item->setTextAlignment(Qt::AlignLeft);
        table->setItem(current_row, 0, item);

        item = new QTableWidgetItem(qApp->LocaleToString(length));
        item->setTextAlignment(Qt::AlignHCenter);
        table->setItem(current_row, 1, item);
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
    fillTable(m_data->lineLengthsData(), ui->line_lengths_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillLineAngles fill variables table with data for line angles.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillLineAngles()
{
    fillTable(m_data->lineAnglesData(), ui->line_angles_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCurveLengths fill variables table with data for curve lengths.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCurveLengths()
{
    fillTable(m_data->curveLengthsData(), ui->curve_lengths_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillControlPointLengths fill variables table with data for control point lengths.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillControlPointLengths()
{
    fillTable(m_data->controlPointLengthsData(), ui->handle_lengths_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillArcsRadiuses fill variables table with data for arc radii.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillArcsRadiuses()
{
    fillTable(m_data->arcRadiusesData(), ui->arc_radii_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillCurveAngles fill variables table with data for curve angles.
//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::fillCurveAngles()
{
    fillTable(m_data->curveAnglesData(), ui->curve_angles_tablewidget);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showUnits()
{
    const QString unit = UnitsToStr(qApp->patternUnit());

    showHeaderUnits(ui->variables_tablewidget, 2, unit);            // calculated value
    showHeaderUnits(ui->variables_tablewidget, 3, unit);            // formula

    showHeaderUnits(ui->line_lengths_tablewidget, 1, unit);         // line lengths
    showHeaderUnits(ui->line_angles_tablewidget, 1, degreeSymbol);  // line angle
    showHeaderUnits(ui->curve_lengths_tablewidget, 1, unit);        // curve lengths
    showHeaderUnits(ui->curve_angles_tablewidget, 1, degreeSymbol); // curve angle
    showHeaderUnits(ui->handle_lengths_tablewidget, 1, unit);       // CP lengths
    showHeaderUnits(ui->arc_radii_tablewidget, 1, unit);            // arc radii
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::showHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
    SCASSERT(table != nullptr)

    QString header = table->horizontalHeaderItem(column)->text();
    // Need to strip text of any umits so we don't recursively add units to the header string
    header = header.section('(', 0, 0);
    const QString unit_header = QString("%1 (%2)").arg(header).arg(unit);
    table->horizontalHeaderItem(column)->setText(unit_header);
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
bool VariablesDialog::evalVariableFormula(const QString &formula, bool from_user, VContainer *data, QLabel *label)
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
            if (from_user)
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
        ui->up_toolbutton->setEnabled(false);
        ui->down_toolbutton->setEnabled(false);
        return;
    }

    if (ui->variables_tablewidget->rowCount() > 0)
    {
        const QTableWidgetItem *name = ui->variables_tablewidget->item(ui->variables_tablewidget->currentRow(), 0);
        SCASSERT(name != nullptr)

        ui->remove_variable_toolbutton->setEnabled(!variableUsed(name->text()));
    }
    else
    {
        ui->remove_variable_toolbutton->setEnabled(false);
    }

    if (ui->variables_tablewidget->rowCount() >= 2)
    {
        if (ui->variables_tablewidget->currentRow() == 0)
        {
            ui->up_toolbutton->setEnabled(false);
            ui->down_toolbutton->setEnabled(true);
        }
        else if (ui->variables_tablewidget->currentRow() == ui->variables_tablewidget->rowCount()-1)
        {
            ui->up_toolbutton->setEnabled(true);
            ui->down_toolbutton->setEnabled(false);
        }
        else
        {
            ui->up_toolbutton->setEnabled(true);
            ui->down_toolbutton->setEnabled(true);
        }
    }
    else
    {
        ui->up_toolbutton->setEnabled(false);
        ui->down_toolbutton->setEnabled(false);
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
        ui->remove_variable_toolbutton->setEnabled(enabled);

        ui->up_toolbutton->setEnabled(enabled);
        ui->down_toolbutton->setEnabled(enabled);
    }

    if (!enabled)
    { // Clear
        ui->name_line_edit->blockSignals(true);
        ui->name_line_edit->clear();
        ui->name_line_edit->blockSignals(false);

        ui->description_plaintextedit->blockSignals(true);
        ui->description_plaintextedit->clear();
        ui->description_plaintextedit->blockSignals(false);

        ui->calculation_label->blockSignals(true);
        ui->calculation_label->clear();
        ui->calculation_label->blockSignals(false);

        ui->formula_plaintextedit->blockSignals(true);
        ui->formula_plaintextedit->clear();
        ui->formula_plaintextedit->blockSignals(false);
    }

    ui->formula_toolbutton->setEnabled(enabled);
    ui->name_line_edit->setEnabled(enabled);
    ui->description_plaintextedit->setEnabled(enabled);
    ui->formula_plaintextedit->setEnabled(enabled);
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
void VariablesDialog::renameCache(const QString &name, const QString &new_name)
{
    for (int i = 0; i < m_rename_list.size(); ++i)
    {
        if (m_rename_list.at(i).second == name)
        {
            m_rename_list[i].second = new_name;
            return;
        }
    }

    m_rename_list.append(qMakePair(name, new_name));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief FullUpdateFromFile update information in tables form file
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::FullUpdateFromFile()
{
    m_has_changes = false;

    ui->line_lengths_tablewidget->clearContents();
    ui->curve_lengths_tablewidget->clearContents();
    ui->curve_angles_tablewidget->clearContents();
    ui->line_angles_tablewidget->clearContents();
    ui->arc_radii_tablewidget->clearContents();

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

        const int row = ui->variables_tablewidget->currentRow();

        m_doc->LiteParseTree(Document::LiteParse);

        ui->variables_tablewidget->blockSignals(true);
        ui->variables_tablewidget->selectRow(row);
        ui->variables_tablewidget->blockSignals(false);

        m_has_changes = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief clickedToolButtonAdd create new row in table
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::addVariable()
{
    qCDebug(vDialog, "Add a new custom variable");

    const QString name = getCustomVariableName();
    qint32 current_row = -1;

    if (ui->variables_tablewidget->currentRow() == -1)
    {
        current_row  = ui->variables_tablewidget->rowCount();
        m_doc->addEmptyCustomVariable(name);
    }
    else
    {
        current_row  = ui->variables_tablewidget->currentRow()+1;
        const QTableWidgetItem *item = ui->variables_tablewidget->item(ui->variables_tablewidget->currentRow(), 0);
        m_doc->addEmptyCustomVariableAfter(item->text(), name);
    }

    m_has_changes = true;
    localUpdateTree();

    ui->variables_tablewidget->selectRow(current_row);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief clickedToolButtonRemove remove one row from table
//---------------------------------------------------------------------------------------------------------------------

void VariablesDialog::removeVariable()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
    m_doc->removeCustomVariable(name->text());

    m_has_changes = true;
    localUpdateTree();

    if (ui->variables_tablewidget->rowCount() > 0)
    {
        ui->variables_tablewidget->selectRow(0);
    }
    else
    {
        enablePieces(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::moveUp()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
    m_doc->moveVariableUp(name->text());

    m_has_changes = true;
    localUpdateTree();

    ui->variables_tablewidget->selectRow(row-1);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::moveDown()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
    m_doc->moveVariableDown(name->text());

    m_has_changes = true;
    localUpdateTree();

    ui->variables_tablewidget->selectRow(row+1);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveVariableName(const QString &text)
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);

    QString new_name = text.isEmpty() ? getCustomVariableName() : CustomIncrSign + text;

    if (!m_data->IsUnique(new_name))
    {
        qint32 num = 2;
        QString temp_name = new_name;
        do
        {
            temp_name = temp_name + QLatin1String("_") + QString().number(num);
            num++;
        } while (!m_data->IsUnique(temp_name));
        new_name = temp_name;
    }

    m_doc->setVariableName(name->text(), new_name);
    QVector<VFormulaField> expressions = m_doc->listVariableExpressions();
    m_doc->replaceNameInFormula(expressions, name->text(), new_name);
    renameCache(name->text(), new_name);

    m_has_changes = true;
    localUpdateTree();

    ui->variables_tablewidget->blockSignals(true);
    ui->variables_tablewidget->selectRow(row);
    ui->variables_tablewidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveDescription()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
    m_doc->setVariableDescription(name->text(), ui->description_plaintextedit->toPlainText());

    localUpdateTree();

    const QTextCursor cursor = ui->description_plaintextedit->textCursor();
    ui->variables_tablewidget->blockSignals(true);
    ui->variables_tablewidget->selectRow(row);
    ui->variables_tablewidget->blockSignals(false);
    ui->description_plaintextedit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::saveVariableFormula()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);

    // Replace line return character with spaces for calc if exist
    QString text = ui->formula_plaintextedit->toPlainText();
    text.replace("\n", " ");

    QTableWidgetItem *formula = ui->variables_tablewidget->item(row, 3);
    if (formula->text() == text)
    {
        QTableWidgetItem *result = ui->variables_tablewidget->item(row, 2);
        //Show unit in dialog label (cm, mm or inch)
        const QString postfix = UnitsToStr(qApp->patternUnit());
        ui->calculation_label->setText(result->text() + " " + postfix);
        return;
    }

    if (text.isEmpty())
    {
        //Show unit in dialog label (cm, mm or inch)
        const QString postfix = UnitsToStr(qApp->patternUnit());
        ui->calculation_label->setText(tr("Error") + " (" + postfix + "). " + tr("Empty field."));
        return;
    }

    QSharedPointer<CustomVariable> variable = m_data->getVariable<CustomVariable>(name->text());
    if (!evalVariableFormula(text, true, variable->GetData(), ui->calculation_label))
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

    const QTextCursor cursor = ui->formula_plaintextedit->textCursor();
    ui->variables_tablewidget->blockSignals(true);
    ui->variables_tablewidget->selectRow(row);
    ui->variables_tablewidget->blockSignals(false);
    ui->formula_plaintextedit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::editFormula()
{
    const int row = ui->variables_tablewidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
    QSharedPointer<CustomVariable> variable = m_data->getVariable<CustomVariable>(name->text());

    EditFormulaDialog *dialog = new EditFormulaDialog(variable->GetData(), NULL_ID, VariableDialog, this);
    dialog->setWindowTitle(tr("Edit variable"));
    QString formula = ui->formula_plaintextedit->toPlainText().replace("\n", " ");
    dialog->SetFormula(qApp->translateVariables()->TryFormulaFromUser(formula, qApp->Settings()->getOsSeparator()));
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    dialog->setPostfix(postfix);//Show unit in dialog label (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        const QTableWidgetItem *name = ui->variables_tablewidget->item(row, 0);
        m_doc->setVariableFormula(name->text(), dialog->GetFormula());

        m_has_changes = true;
        localUpdateTree();

        ui->variables_tablewidget->selectRow(row);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::closeEvent(QCloseEvent *event)
{
    refreshPattern();

    ui->formula_plaintextedit->blockSignals(true);
    ui->name_line_edit->blockSignals(true);
    ui->description_plaintextedit->blockSignals(true);

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
    if (QLineEdit *text_edit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                QString separator = qApp->Settings()->getOsSeparator()
                                  ? QString(localeDecimalPoint(QLocale()))
                                  : QString(localeDecimalPoint(QLocale::c()));

                text_edit->insert(separator);
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
void VariablesDialog::showCustomVariables()
{
    if (ui->variables_tablewidget->rowCount() > 0)
    {
        enablePieces(true);

        // name
        const QTableWidgetItem *name = ui->variables_tablewidget->item(ui->variables_tablewidget->currentRow(), 0);
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

        ui->name_line_edit->setText(clearCustomVariableName(variable->GetName()));

        //ui->description_plaintextedit->blockSignals(true);
        ui->description_plaintextedit->setPlainText(variable->GetDescription());
        //ui->description_plaintextedit->blockSignals(false);

        evalVariableFormula(variable->GetFormula(), false, variable->GetData(), ui->calculation_label);
        //ui->formula_plaintextedit->blockSignals(true);

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

        ui->formula_plaintextedit->setPlainText(formula);
        //ui->formula_plaintextedit->blockSignals(false);
    }
    else
    {
        enablePieces(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VariablesDialog::filterVariables(const QString &filterString)
{
    QSharedPointer<QTableWidget> current_table = m_table_list.value(ui->tab_widget->currentIndex());
    current_table->blockSignals(true);

    if (filterString.isEmpty())
    {
        m_is_filtered = false;
        for (auto i = 0; i < current_table->rowCount(); ++i)
        {
            current_table->showRow(i);
        }
        ui->variables_tablewidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    }
    else
    {
        m_is_filtered = true;
        ui->up_toolbutton->setEnabled(false);
        ui->down_toolbutton->setEnabled(false);
        for (auto i = 0; i < current_table->rowCount(); i++)
        {
            current_table->hideRow(i);
        }

        for (auto item : current_table->findItems(filterString, Qt::MatchContains))
        {
            if (item)
            {
                current_table->showRow(item->row());
            }
        }
    }

    current_table->blockSignals(false);
}

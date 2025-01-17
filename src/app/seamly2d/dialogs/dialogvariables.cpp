/***************************************************************************
 **  @file   dialogvariables.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   dialogvariables.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
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
#include "dialogvariables.h"
#include "ui_dialogvariables.h"
#include "../vwidgets/vwidgetpopup.h"
#include "../vmisc/vsettings.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/calculator.h"
#include "../vtools/dialogs/support/edit_formula_dialog.h"

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
/**
 * @brief DialogVariables create dialog
 * @param data container with data
 * @param doc dom document container
 * @param parent parent widget
 */
DialogVariables::DialogVariables(VContainer *data, VPattern *doc, QWidget *parent)
    : DialogTool(data, NULL_ID, parent)
    , ui(new Ui::DialogVariables)
    , data(data)
    , doc(doc)
    , formulaBaseHeight(0)
    , hasChanges(false)
    , renameList()
    , tableList()
    , isSorted(false)
    , isFiltered(false)
{
    ui->setupUi(this);

    setWindowFlags(Qt::Window);
    setWindowFlags((windowFlags() | Qt::WindowStaysOnTopHint) & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    ui->name_LineEdit->setClearButtonEnabled(true);
    ui->filter_LineEdit->installEventFilter(this);

    formulaBaseHeight = ui->formula_PlainTextEdit->height();
    ui->formula_PlainTextEdit->installEventFilter(this);

    qApp->Settings()->GetOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

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


    tableList.append(QSharedPointer<QTableWidget>(ui->variables_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->lineLengths_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->lineAngles_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->curveLengths_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->curveAngles_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->controlPointLengths_TableWidget));
    tableList.append(QSharedPointer<QTableWidget>(ui->arcRadiuses_TableWidget));

    connect(this->doc, &VPattern::FullUpdateFromFile, this, &DialogVariables::FullUpdateFromFile);

    ui->tabWidget->setCurrentIndex(0);
    ui->name_LineEdit->setValidator( new QRegularExpressionValidator(QRegularExpression(
                                                                        QLatin1String("^$|")+NameRegExp()), this));

    connect(ui->variables_TableWidget, &QTableWidget::itemSelectionChanged, this,
            &DialogVariables::showCustomVariableDetails);

    connect(ui->addCustomVariable_ToolButton, &QPushButton::clicked, this, &DialogVariables::addCustomVariable);
    connect(ui->removeCustomVariable_ToolButton, &QToolButton::clicked, this, &DialogVariables::removeCustomVariable);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &DialogVariables::moveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &DialogVariables::moveDown);
    connect(ui->formula_ToolButton, &QToolButton::clicked, this, &DialogVariables::Fx);
    connect(ui->name_LineEdit, &QLineEdit::textEdited, this, &DialogVariables::saveCustomVariableName);
    connect(ui->description_PlainTextEdit, &QPlainTextEdit::textChanged, this, &DialogVariables::saveCustomVariableDescription);
    connect(ui->formula_PlainTextEdit, &QPlainTextEdit::textChanged, this, &DialogVariables::saveCustomVariableFormula);

    connect(ui->filter_LineEdit, &QLineEdit::textChanged, this, &DialogVariables::filterVariables);

    connect(ui->refresh_PushButton, &QPushButton::clicked, this, &DialogVariables::refreshPattern);

    connect(ui->variables_TableWidget->horizontalHeader(), &QHeaderView::sectionClicked, [this]()
    {
        isSorted = true;
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
        isFiltered = false;

        if (ui->tabWidget->currentIndex() == 0)
        {
            filterVariables("");
            ui->variables_TableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
            isSorted = false;
            setMoveControls();
        }
    };
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, clearFilterString);
}

//---------------------------------------------------------------------------------------------------------------------
DialogVariables::~DialogVariables()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief fillCustomVariables fill data for variables table
 */
void DialogVariables::fillCustomVariables(bool freshCall)
{
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->clearContents();

    const QMap<QString, QSharedPointer<VIncrement> > variables = data->variablesData();
    QMap<QString, QSharedPointer<VIncrement> >::const_iterator i;
    QMap<quint32, QString> map;
    //Sorting QHash by id
    for (i = variables.constBegin(); i != variables.constEnd(); ++i)
    {
        QSharedPointer<VIncrement> variable = i.value();
        map.insert(variable->getIndex(), i.key());
    }

    qint32 currentRow = -1;
    QMapIterator<quint32, QString> iMap(map);
    ui->variables_TableWidget->setRowCount ( variables.size() );
    while (iMap.hasNext())
    {
        iMap.next();
        QSharedPointer<VIncrement> variable = variables.value(iMap.value());
        currentRow++;

        addCell(ui->variables_TableWidget, variable->GetName(), currentRow, 0, Qt::AlignVCenter); // name
        addCell(ui->variables_TableWidget, qApp->LocaleToString(*variable->GetValue()), currentRow, 1,
                Qt::AlignHCenter | Qt::AlignVCenter, variable->IsFormulaOk()); // calculated value

        QString formula;
        try
        {
            formula = qApp->TrVars()->FormulaToUser(variable->GetFormula(), qApp->Settings()->GetOsSeparator());
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            formula = variable->GetFormula();
        }

        addCell(ui->variables_TableWidget, formula, currentRow, 2, Qt::AlignVCenter); // formula
    }

    if (freshCall)
    {
        ui->variables_TableWidget->resizeColumnsToContents();
        ui->variables_TableWidget->resizeRowsToContents();
    }

    ui->variables_TableWidget->setColumnWidth(0, 350);
    ui->variables_TableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->variables_TableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void DialogVariables::fillTable(const QMap<QString, T> &varTable, QTableWidget *table)
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
/**
 * @brief FillLengthLines fill data for table of lines lengths
 */
void DialogVariables::fillLineLengths()
{
    fillTable(data->lineLengthsData(), ui->lineLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::fillLineAngles()
{
    fillTable(data->lineAnglesData(), ui->lineAngles_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillLengthSplines fill data for table of splines lengths
 */
void DialogVariables::fillCurveLengths()
{
    fillTable(data->curveLengthsData(), ui->curveLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::fillControlPointLengths()
{
    fillTable(data->controlPointLengthsData(), ui->controlPointLengths_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::fillArcsRadiuses()
{
    fillTable(data->arcRadiusesData(), ui->arcRadiuses_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::fillCurveAngles()
{
    fillTable(data->curveAnglesData(), ui->curveAngles_TableWidget);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::showUnits()
{
    const QString unit = UnitsToStr(qApp->patternUnit());

    showHeaderUnits(ui->variables_TableWidget, 1, unit);// calculated value
    showHeaderUnits(ui->variables_TableWidget, 2, unit);// formula

    showHeaderUnits(ui->lineLengths_TableWidget, 1, unit);// lengths
    showHeaderUnits(ui->curveLengths_TableWidget, 1, unit);// lengths
    showHeaderUnits(ui->controlPointLengths_TableWidget, 1, unit);// lengths
    showHeaderUnits(ui->lineAngles_TableWidget, 1, degreeSymbol);// angle
    showHeaderUnits(ui->arcRadiuses_TableWidget, 1, unit);// radius
    showHeaderUnits(ui->curveAngles_TableWidget, 1, degreeSymbol);// angle
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::showHeaderUnits(QTableWidget *table, int column, const QString &unit)
{
    SCASSERT(table != nullptr)

    const QString header = table->horizontalHeaderItem(column)->text();
    const QString unitHeader = QString("%1 (%2)").arg(header).arg(unit);
    table->horizontalHeaderItem(column)->setText(unitHeader);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::addCell(QTableWidget *table, const QString &text, int row, int column, int aligment, bool ok)
{
    SCASSERT(table != nullptr)

    QTableWidgetItem *item = new QTableWidgetItem(text);
    item->setTextAlignment(aligment);

    // set the item non-editable (view only), and non-selectable
    Qt::ItemFlags flags = item->flags();
    flags &= ~(Qt::ItemIsEditable); // reset/clear the flag
    item->setFlags(flags);

    if (not ok)
    {
        QBrush brush = item->foreground();
        brush.setColor(Qt::red);
        item->setForeground(brush);
    }

    table->setItem(row, column, item);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogVariables::getCustomVariableName() const
{
    qint32 num = 1;
    QString name;
    do
    {
        name = CustomIncrSign + qApp->TrVars()->InternalVarToUser(variable_) + QString().number(num);
        num++;
    } while (not data->IsUnique(name));
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogVariables::clearCustomVariableName(const QString &name) const
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
bool DialogVariables::evalVariableFormula(const QString &formula, bool fromUser, VContainer *data, QLabel *label)
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
                f = qApp->TrVars()->FormulaFromUser(formula, qApp->Settings()->GetOsSeparator());
            }
            else
            {
                f = formula;
            }
            f.replace("\n", " ");
            QScopedPointer<Calculator> cal(new Calculator());
            const qreal result = cal->EvalFormula(data->DataVariables(), f);

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
void DialogVariables::setMoveControls()
{
    if (isSorted || isFiltered)
    {
        ui->toolButtonUp->setEnabled(false);
        ui->toolButtonDown->setEnabled(false);
        return;
    }

    if (ui->variables_TableWidget->rowCount() > 0)
    {
        const QTableWidgetItem *name = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        SCASSERT(name != nullptr)

        ui->removeCustomVariable_ToolButton->setEnabled(not variableUsed(name->text()));
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
void DialogVariables::enablePieces(bool enabled)
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

    if (not enabled)
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
void DialogVariables::localUpdateTree()
{
    doc->LiteParseIncrements();
    fillCustomVariables();
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogVariables::variableUsed(const QString &name) const
{
    const QVector<VFormulaField> expressions = doc->ListExpressions();

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
void DialogVariables::renameCache(const QString &name, const QString &newName)
{
    for (int i = 0; i < renameList.size(); ++i)
    {
        if (renameList.at(i).second == name)
        {
            renameList[i].second = newName;
            return;
        }
    }

    renameList.append(qMakePair(name, newName));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update information in tables form file
 */
void DialogVariables::FullUpdateFromFile()
{
    hasChanges = false;

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
void DialogVariables::refreshPattern()
{
    if (hasChanges)
    {
        QVector<VFormulaField> expressions = doc->ListExpressions();
        for (int i = 0; i < renameList.size(); ++i)
        {
            doc->replaceNameInFormula(expressions, renameList.at(i).first, renameList.at(i).second);
        }
        renameList.clear();

        const int row = ui->variables_TableWidget->currentRow();

        doc->LiteParseTree(Document::LiteParse);

        ui->variables_TableWidget->blockSignals(true);
        ui->variables_TableWidget->selectRow(row);
        ui->variables_TableWidget->blockSignals(false);

        hasChanges = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief clickedToolButtonAdd create new row in table
 */
void DialogVariables::addCustomVariable()
{
    qCDebug(vDialog, "Add a new custom variable");

    const QString name = getCustomVariableName();
    qint32 currentRow = -1;

    if (ui->variables_TableWidget->currentRow() == -1)
    {
        currentRow  = ui->variables_TableWidget->rowCount();
        doc->addEmptyCustomVariable(name);
    }
    else
    {
        currentRow  = ui->variables_TableWidget->currentRow()+1;
        const QTableWidgetItem *item = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        doc->addEmptyCustomVariableAfter(item->text(), name);
    }

    hasChanges = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(currentRow);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief clickedToolButtonRemove remove one row from table
 */
void DialogVariables::removeCustomVariable()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    doc->removeCustomVariable(name->text());

    hasChanges = true;
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
void DialogVariables::moveUp()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    doc->MoveUpIncrement(name->text());

    hasChanges = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(row-1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::moveDown()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    doc->MoveDownIncrement(name->text());

    hasChanges = true;
    localUpdateTree();

    ui->variables_TableWidget->selectRow(row+1);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::saveCustomVariableName(const QString &text)
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);

    QString newName = text.isEmpty() ? getCustomVariableName() : CustomIncrSign + text;

    if (not data->IsUnique(newName))
    {
        qint32 num = 2;
        QString tempName = newName;
        do
        {
            tempName = tempName + QLatin1String("_") + QString().number(num);
            num++;
        } while (not data->IsUnique(tempName));
        newName = tempName;
    }

    doc->setIncrementName(name->text(), newName);
    QVector<VFormulaField> expressions = doc->ListIncrementExpressions();
    doc->replaceNameInFormula(expressions, name->text(), newName);
    renameCache(name->text(), newName);

    hasChanges = true;
    localUpdateTree();

    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::saveCustomVariableDescription()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    doc->setIncrementDescription(name->text(), ui->description_PlainTextEdit->toPlainText());

    localUpdateTree();

    const QTextCursor cursor = ui->description_PlainTextEdit->textCursor();
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
    ui->description_PlainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::saveCustomVariableFormula()
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

    QSharedPointer<VIncrement> variable = data->GetVariable<VIncrement>(name->text());
    if (not evalVariableFormula(text, true, variable->GetData(), ui->calculatedValue_Label))
    {
        return;
    }

    try
    {
        const QString formula = qApp->TrVars()->FormulaFromUser(text, qApp->Settings()->GetOsSeparator());
        doc->SetIncrementFormula(name->text(), formula);
    }
    catch (qmu::QmuParserError &error) // Just in case something bad will happen
    {
        Q_UNUSED(error)
        return;
    }

    hasChanges = true;
    localUpdateTree();

    const QTextCursor cursor = ui->formula_PlainTextEdit->textCursor();
    ui->variables_TableWidget->blockSignals(true);
    ui->variables_TableWidget->selectRow(row);
    ui->variables_TableWidget->blockSignals(false);
    ui->formula_PlainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::Fx()
{
    const int row = ui->variables_TableWidget->currentRow();

    if (row == -1)
    {
        return;
    }

    const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
    QSharedPointer<VIncrement> variable = data->GetVariable<VIncrement>(name->text());

    EditFormulaDialog *dialog = new EditFormulaDialog(variable->GetData(), NULL_ID, this);
    dialog->setWindowTitle(tr("Edit variable"));
    dialog->SetFormula(qApp->TrVars()->TryFormulaFromUser(ui->formula_PlainTextEdit->toPlainText().replace("\n", " "),
                                                          qApp->Settings()->GetOsSeparator()));
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    dialog->setPostfix(postfix);//Show unit in dialog label (cm, mm or inch)

    if (dialog->exec() == QDialog::Accepted)
    {
        // Because of the bug need to take QTableWidgetItem twice time. Previous update "killed" the pointer.
        const QTableWidgetItem *name = ui->variables_TableWidget->item(row, 0);
        doc->SetIncrementFormula(name->text(), dialog->GetFormula());

        hasChanges = true;
        localUpdateTree();

        ui->variables_TableWidget->selectRow(row);
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::closeEvent(QCloseEvent *event)
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
void DialogVariables::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        FullUpdateFromFile();
    }
    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogVariables::eventFilter(QObject *object, QEvent *event)
{
    if (QLineEdit *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (qApp->Settings()->GetOsSeparator())
                {
                    textEdit->insert(QLocale().decimalPoint());
                }
                else
                {
                    textEdit->insert(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }
    else
    {
        // pass the event on to the parent class
        return DialogTool::eventFilter(object, event);
    }
    return false;// pass the event to the widget
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::showEvent(QShowEvent *event)
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
    // do your init stuff here

    const QSize sz = qApp->Settings()->GetIncrementsDialogSize();
    if (not sz.isEmpty())
    {
        resize(sz);
    }

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize at
    // dialog creating, which would
    if (isInitialized)
    {
        qApp->Settings()->SetIncrementsDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::showCustomVariableDetails()
{
    if (ui->variables_TableWidget->rowCount() > 0)
    {
        enablePieces(true);

        // name
        const QTableWidgetItem *name = ui->variables_TableWidget->item(ui->variables_TableWidget->currentRow(), 0);
        QSharedPointer<VIncrement> variable;

        try
        {
            variable = data->GetVariable<VIncrement>(name->text());
        }
        catch(const VExceptionBadId &error)
        {
            Q_UNUSED(error)
            enablePieces(false);
            return;
        }

        ui->name_LineEdit->blockSignals(true);
        ui->name_LineEdit->setText(clearCustomVariableName(variable->GetName()));
        ui->name_LineEdit->blockSignals(false);

        ui->description_PlainTextEdit->blockSignals(true);
        ui->description_PlainTextEdit->setPlainText(variable->GetDescription());
        ui->description_PlainTextEdit->blockSignals(false);

        evalVariableFormula(variable->GetFormula(), false, variable->GetData(), ui->calculatedValue_Label);
        ui->formula_PlainTextEdit->blockSignals(true);

        QString formula;
        try
        {
            formula = qApp->TrVars()->FormulaToUser(variable->GetFormula(), qApp->Settings()->GetOsSeparator());
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            formula = variable->GetFormula();
        }

        ui->formula_PlainTextEdit->setPlainText(formula);
        ui->formula_PlainTextEdit->blockSignals(false);
    }
    else
    {
        enablePieces(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogVariables::filterVariables(const QString &filterString)
{
    QSharedPointer<QTableWidget> currentTable = tableList.value(ui->tabWidget->currentIndex());
    currentTable->blockSignals(true);

    if (filterString.isEmpty())
    {
        isFiltered = false;
        for (auto i = 0; i < currentTable->rowCount(); ++i)
        {
            currentTable->showRow(i);
        }
        ui->variables_TableWidget->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    }
    else
    {
        isFiltered = true;
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

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
 **  @file   dialogmove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#include "dialogmove.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QPlainTextEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QTimer>
#include <QToolButton>
#include <Qt>
#include <new>

#include "../../visualization/visualization.h"
#include "../../visualization/line/operation/vistoolmove.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../support/dialogeditwrongformula.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogmove.h"

//---------------------------------------------------------------------------------------------------------------------
DialogMove::DialogMove(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    ,  ui(new Ui::DialogMove)
    , angleFlag(false)
    , angleFormula()
    , angleTimer(nullptr)
    , lengthFlag(false)
    , lengthFormula()
    , lengthTimer(nullptr)
    , objects()
    , stage1(true)
    , m_suffix()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/move.png"));

    ui->angle_PlainTextEdit->installEventFilter(this);
    ui->length_PlainTextEdit->installEventFilter(this);

    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix());

    angleTimer = new QTimer(this);
    connect(angleTimer, &QTimer::timeout, this, &DialogMove::evaluateAngle);

    lengthTimer = new QTimer(this);
    connect(lengthTimer, &QTimer::timeout, this, &DialogMove::evaluateLength);

    InitOkCancelApply(ui);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit,          &QLineEdit::textChanged,      this, &DialogMove::suffixChanged);
    connect(ui->angleFormula_ToolButton,  &QPushButton::clicked,        this, &DialogMove::editAngleFormula);
    connect(ui->lengthFormula_ToolButton, &QPushButton::clicked,        this, &DialogMove::editLengthFormula);
    connect(ui->angle_PlainTextEdit,      &QPlainTextEdit::textChanged, this, &DialogMove::angleChanged);
    connect(ui->length_PlainTextEdit,     &QPlainTextEdit::textChanged, this, &DialogMove::lengthChanged);

    vis = new VisToolMove(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogMove::~DialogMove()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMove::GetAngle() const
{
    return qApp->TrVars()->TryFormulaFromUser(angleFormula, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetAngle(const QString &value)
{
    angleFormula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    ui->angle_PlainTextEdit->setPlainText(angleFormula);

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetAngle(angleFormula);

    MoveCursorToEnd(ui->angle_PlainTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMove::GetLength() const
{
    return qApp->TrVars()->TryFormulaFromUser(lengthFormula, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SetLength(const QString &value)
{
    lengthFormula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    ui->length_PlainTextEdit->setPlainText(lengthFormula);

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetLength(lengthFormula);

    MoveCursorToEnd(ui->length_PlainTextEdit);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMove::getSuffix() const
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::setSuffix(const QString &value)
{
    m_suffix = value;
    ui->suffix_LineEdit->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> DialogMove::getObjects() const
{
    return objects.toVector();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (objects.isEmpty())
        {
            return;
        }

        stage1 = false;
        prepare = true;

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)
        operation->setObjects(objects.toVector());
        operation->VisualMode();

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        connect(operation, &VisToolMove::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

        scene->ToggleArcSelection(false);
        scene->ToggleElArcSelection(false);
        scene->ToggleSplineSelection(false);
        scene->ToggleSplinePathSelection(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);
    }
    else if (not stage1 && prepare && click)
    {
        VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        if (operation->LengthValue() > 0)
        {
            SetAngle(operation->Angle());//Show in dialog angle that a user choose
            SetLength(operation->Length());
            setModal(true);
            emit ToolTip("");
            angleTimer->start();
            lengthTimer->start();
            show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ChosenObject(quint32 id, const SceneObject &type)
{
    Q_UNUSED(id)
    Q_UNUSED(type)
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(tool)
    if (stage1)
    {
        if (selected)
        {
            if (not objects.contains(object))
            {
                objects.append(object);
            }
        }
        else
        {
            objects.removeOne(object);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::angleChanged()
{
    labelEditFormula = ui->editAngle_Label;
    labelResultCalculation = ui->angleResult_Label;
    ValFormulaChanged(angleFlag, ui->angle_PlainTextEdit, angleTimer, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::lengthChanged()
{
    labelEditFormula = ui->editLength_Label;
    labelResultCalculation = ui->lengthResult_Label;
    ValFormulaChanged(lengthFlag, ui->length_PlainTextEdit, lengthTimer, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::editAngleFormula()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit angle"));
    dialog->SetFormula(GetAngle());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        SetAngle(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::editLengthFormula()
{
    DialogEditWrongFormula *dialog = new DialogEditWrongFormula(data, toolId, this);
    dialog->setWindowTitle(tr("Edit length"));
    dialog->SetFormula(GetLength());
    dialog->setPostfix(UnitsToStr(qApp->patternUnit(), true));
    if (dialog->exec() == QDialog::Accepted)
    {
        SetLength(dialog->GetFormula());
    }
    delete dialog;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::suffixChanged()
{
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
    {
        const QString suffix = edit->text();
        if (suffix.isEmpty())
        {
            flagName = false;
            ChangeColor(ui->suffix_Label, Qt::red);
            CheckState();
            return;
        }
        else
        {
            if (m_suffix != suffix)
            {
                QRegularExpression rx(NameRegExp());
                const QStringList uniqueNames = VContainer::AllUniqueNames();
                for (int i=0; i < uniqueNames.size(); ++i)
                {
                    const QString name = uniqueNames.at(i) + suffix;
                    if (not rx.match(name).hasMatch() || not data->IsUnique(name))
                    {
                        flagName = false;
                        ChangeColor(ui->suffix_Label, Qt::red);
                        CheckState();
                        return;
                    }
                }
            }
        }

        flagName = true;
        ChangeColor(ui->suffix_Label, okColor);
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::CheckState()
{
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(angleFlag && lengthFlag && flagName);
    SCASSERT(bApply != nullptr)
    bApply->setEnabled(bOk->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowVisualization()
{
    AddVisualization<VisToolMove>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SaveData()
{
    m_suffix = ui->suffix_LineEdit->text();

    angleFormula = ui->angle_PlainTextEdit->toPlainText();
    angleFormula.replace("\n", " ");

    lengthFormula = ui->length_PlainTextEdit->toPlainText();
    lengthFormula.replace("\n", " ");

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)

    operation->setObjects(objects.toVector());
    operation->SetAngle(angleFormula);
    operation->SetLength(lengthFormula);
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::closeEvent(QCloseEvent *event)
{
    ui->angle_PlainTextEdit->blockSignals(true);
    ui->length_PlainTextEdit->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::evaluateAngle()
{
    labelEditFormula = ui->editAngle_Label;
    Eval(ui->angle_PlainTextEdit->toPlainText(), angleFlag, ui->angleResult_Label, degreeSymbol, false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::evaluateLength()
{
    labelEditFormula = ui->editLength_Label;
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);
    Eval(ui->length_PlainTextEdit->toPlainText(), lengthFlag, ui->lengthResult_Label, postfix);
}

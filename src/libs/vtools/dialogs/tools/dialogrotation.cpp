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
 **  @file   dialogrotation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
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

#include "dialogrotation.h"

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
#include "../../visualization/line/operation/vistoolrotation.h"
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
#include "ui_dialogrotation.h"

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::DialogRotation(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogRotation)
    , angleFlag(false)
    , angleTimer(nullptr)
    , angleFormula()
    , m_objects()
    , stage1(true)
    , m_suffix()
    , m_firstRelease(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/rotation.png"));

    ui->plainTextEditFormula->installEventFilter(this);
    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix(qApp->Settings()->getRotateSuffix()));

    angleTimer = new QTimer(this);
    connect(angleTimer, &QTimer::timeout, this, &DialogRotation::evaluateAngle);

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->rotation_ComboBox);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit,      &QLineEdit::textChanged,        this, &DialogRotation::suffixChanged);
    connect(ui->formula_ToolButton,   &QPushButton::clicked,          this, &DialogRotation::editAngleFormula);
    connect(ui->plainTextEditFormula, &QPlainTextEdit::textChanged,   this, &DialogRotation::angleChanged);
    connect(ui->rotation_ComboBox,    &QComboBox::currentTextChanged, this, &DialogRotation::pointChanged);

    vis = new VisToolRotation(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogRotation::~DialogRotation()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogRotation::getOriginPointId() const
{
    return getCurrentObjectId(ui->rotation_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::setOriginPointId(const quint32 &value)
{
    ChangeCurrentData(ui->rotation_ComboBox, value);
    VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogRotation::GetAngle() const
{
    return qApp->TrVars()->TryFormulaFromUser(angleFormula, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SetAngle(const QString &value)
{
    angleFormula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    ui->plainTextEditFormula->setPlainText(angleFormula);

    VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->SetAngle(angleFormula);

    MoveCursorToEnd(ui->plainTextEditFormula);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogRotation::getSuffix() const
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::setSuffix(const QString &value)
{
    m_suffix = value;
    ui->suffix_LineEdit->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<SourceItem> DialogRotation::getSourceObjects() const
{
    return m_objects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::setSourceObjects(const QVector<SourceItem> &value)
{
    m_objects = value;

    VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)
    operation->setObjects(sourceToObjects(m_objects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (m_objects.isEmpty())
        {
            return;
        }

        stage1 = false;

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)
        operation->setObjects(sourceToObjects(m_objects));
        operation->VisualMode();

        scene->ToggleArcSelection(false);
        scene->ToggleElArcSelection(false);
        scene->ToggleSplineSelection(false);
        scene->ToggleSplinePathSelection(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        qApp->getSceneView()->allowRubberBand(false);

        emit ToolTip(tr("Select rotation point"));
    }
    else if (not stage1 && prepare && click)
    {
        // The check need to ignore first release of mouse button.
        // User can select point by clicking on a label.
        if (not m_firstRelease)
        {
            m_firstRelease = true;
            return;
        }

        /*We will ignore click if pointer is in point circle*/
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        try
        {
            const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(getOriginPointId());
            const QLineF line = QLineF(static_cast<QPointF>(*point), scene->getScenePos());

            //Radius of point circle, but little bigger. Need handle with hover sizes.
            if (line.length() <= scaledRadius(sceneScale(qApp->getCurrentScene())) * 1.5)
            {
                return;
            }
        }
        catch (const VExceptionBadId &)
        {
            return;
        }

        VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(operation != nullptr)

        SetAngle(operation->Angle());//Show in dialog angle that a user choose
        setModal(true);
        emit ToolTip("");
        angleTimer->start();
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
            SCASSERT(operation != nullptr)

            auto object = std::find_if(m_objects.begin(), m_objects.end(),
                                         [id](const SourceItem &item) { return item.id == id; });

            if (object != m_objects.end())
            {
                if (m_objects.size() > 1)
                {
                    // It's not really logical for a user that a center of rotation no need to select.
                    // To fix this issue we just silently remove it from the list.
                    m_objects.erase(object);
                    operation->setObjects(sourceToObjects(m_objects));
                }
                else
                {
                    emit ToolTip(tr("Select rotation point that is not part of the list of objects"));
                    return;
                }
            }

            if (SetObject(id, ui->rotation_ComboBox, ""))
            {

                VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
                SCASSERT(window != nullptr)
                connect(operation, &Visualization::ToolTip, window, &VAbstractMainWindow::ShowToolTip);

                operation->SetOriginPointId(id);
                operation->RefreshGeometry();

                prepare = true;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SelectedObject(bool selected, quint32 id, quint32 tool)
{
    Q_UNUSED(tool)
    if (stage1)
    {
        auto object = std::find_if(m_objects.begin(), m_objects.end(),
                                [id](const SourceItem &item) { return item.id == id; });

        if (selected)
        {
            if (object == m_objects.cend())
            {
                SourceItem item;
                item.id = id;
                m_objects.append(item);
            }
        }
        else
        {
            if (object != m_objects.end())
            {
                m_objects.erase(object);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::angleChanged()
{
    labelEditFormula = ui->editAngle_Label;
    labelResultCalculation = ui->resultAngle_Label;
    ValFormulaChanged(angleFlag, ui->plainTextEditFormula, angleTimer, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::editAngleFormula()
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
void DialogRotation::suffixChanged()
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
void DialogRotation::CheckState()
{
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(angleFlag && flagName && flagError);
    SCASSERT(bApply != nullptr)
    bApply->setEnabled(bOk->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::ShowVisualization()
{
    AddVisualization<VisToolRotation>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::SaveData()
{
    m_suffix = ui->suffix_LineEdit->text();

    angleFormula = ui->plainTextEditFormula->toPlainText();
    angleFormula.replace("\n", " ");

    VisToolRotation *operation = qobject_cast<VisToolRotation *>(vis);
    SCASSERT(operation != nullptr)

    operation->setObjects(sourceToObjects(m_objects));
    operation->SetOriginPointId(getOriginPointId());
    operation->SetAngle(angleFormula);
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::closeEvent(QCloseEvent *event)
{
    ui->plainTextEditFormula->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::pointChanged()
{
    quint32 id = getCurrentObjectId(ui->rotation_ComboBox);

    auto objectId = std::find_if(m_objects.begin(), m_objects.end(),
                            [id](const SourceItem &item) { return item.id == id; });

    QColor color = okColor;
    if (objectId != m_objects.end())
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->originPoint_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogRotation::evaluateAngle()
{
    labelEditFormula = ui->editAngle_Label;
    Eval(ui->plainTextEditFormula->toPlainText(), angleFlag, ui->resultAngle_Label, degreeSymbol, false);
}

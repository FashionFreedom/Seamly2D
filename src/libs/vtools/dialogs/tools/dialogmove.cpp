/***************************************************************************
 **  @file   dialogmove.cpp
 **  @author Douglas S Caskey
 **  @date   18 Nov, 2023
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
 **
 **  @file   dialogmove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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
#include "../support/edit_formula_dialog.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogmove.h"
#include "../../tools/drawTools/operation/vabstractoperation.h"

//---------------------------------------------------------------------------------------------------------------------
DialogMove::DialogMove(const VContainer *data, quint32 toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogMove)
    , angleFlag(false)
    , angleFormula()
    , angleTimer(nullptr)
    , lengthFlag(false)
    , lengthFormula()
    , lengthTimer(nullptr)
    , rotationFlag(false)
    , rotationFormula()
    , rotationTimer(nullptr)
    , m_objects()
    , stage1(true)
    , stage2(false)
    , m_suffix()
    , useOriginPoint(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/move.png"));

    ui->angle_PlainTextEdit->installEventFilter(this);
    ui->length_PlainTextEdit->installEventFilter(this);
    ui->rotation_PlainTextEdit->installEventFilter(this);

    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix(qApp->Settings()->getMoveSuffix()));

    angleTimer = new QTimer(this);
    connect(angleTimer, &QTimer::timeout, this, &DialogMove::evaluateAngle);

    lengthTimer = new QTimer(this);
    connect(lengthTimer, &QTimer::timeout, this, &DialogMove::evaluateLength);

    rotationTimer = new QTimer(this);
    connect(rotationTimer, &QTimer::timeout, this, &DialogMove::evaluateRotation);

    initializeOkCancelApply(ui);

    FillComboBoxPoints(ui->rotationPoint_ComboBox);
    ui->rotationPoint_ComboBox->blockSignals(true);
    ui->rotationPoint_ComboBox->addItem(tr("Center point"), NULL_ID);
    ui->rotationPoint_ComboBox->blockSignals(false);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit,            &QLineEdit::textChanged,        this, &DialogMove::suffixChanged);
    connect(ui->angleFormula_ToolButton,    &QPushButton::clicked,          this, &DialogMove::editAngleFormula);
    connect(ui->lengthFormula_ToolButton,   &QPushButton::clicked,          this, &DialogMove::editLengthFormula);
    connect(ui->rotationFormula_ToolButton, &QPushButton::clicked,          this, &DialogMove::editRotationFormula);

    connect(ui->angle_PlainTextEdit,        &QPlainTextEdit::textChanged,   this, &DialogMove::angleChanged);
    connect(ui->length_PlainTextEdit,       &QPlainTextEdit::textChanged,   this, &DialogMove::lengthChanged);
    connect(ui->rotation_PlainTextEdit,     &QPlainTextEdit::textChanged,   this, &DialogMove::rotationChanged);
    connect(ui->rotationPoint_ComboBox,     &QComboBox::currentTextChanged, this, &DialogMove::originChanged);

    vis = new VisToolMove(data);

    setOriginPointId(NULL_ID);
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
QString DialogMove::getRotation() const
{
    return qApp->TrVars()->TryFormulaFromUser(rotationFormula, qApp->Settings()->GetOsSeparator());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::setRotation(const QString &value)
{
    rotationFormula = qApp->TrVars()->FormulaToUser(value, qApp->Settings()->GetOsSeparator());
    ui->rotation_PlainTextEdit->setPlainText(rotationFormula);

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->setRotation(rotationFormula);

    MoveCursorToEnd(ui->rotation_PlainTextEdit);
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
quint32 DialogMove::getOriginPointId() const
{
    return getCurrentObjectId(ui->rotationPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::setOriginPointId(const quint32 &value)
{
    ChangeCurrentData(ui->rotationPoint_ComboBox, value);
    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->setOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (m_objects.isEmpty())
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
        operation->setObjects(sourceToObjects(m_objects));
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

        qApp->getSceneView()->allowRubberBand(false);
    }
    else if (!stage2 && !stage1 && prepare && click)
    {
        VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        if (operation->LengthValue() > 0)
        {
            angleFormula = qApp->TrVars()->FormulaToUser(operation->Angle(), qApp->Settings()->GetOsSeparator());
            lengthFormula = qApp->TrVars()->FormulaToUser(operation->Length(), qApp->Settings()->GetOsSeparator());
            operation->SetAngle(angleFormula);
            operation->SetLength(lengthFormula);

            operation->RefreshGeometry();
            emit ToolTip(operation->CurrentToolTip());
            stage2 = true;
        }
    }
    else if (!stage1 && stage2 && prepare && click)
    {
        VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
        SCASSERT(operation != nullptr)

        if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
        {
            if (!useOriginPoint)
            {
                operation->setOriginPointId(NULL_ID);
                SetObject(NULL_ID, ui->rotationPoint_ComboBox, QString());
                operation->RefreshGeometry();
            }
            useOriginPoint = false;
        }
        else
        {
            SetAngle(operation->Angle());
            SetLength(operation->Length());
            setRotation(operation->Rotation());
            setModal(true);
            emit ToolTip("");
            angleTimer->start();
            lengthTimer->start();
            rotationTimer->start();
            show();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::ChosenObject(quint32 id, const SceneObject &type)
{
    if (!stage1 && stage2 && prepare)
    {
        if (type == SceneObject::Point && QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
        {
            if (SetObject(id, ui->rotationPoint_ComboBox, QString()))
            {
                VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
                SCASSERT(operation != nullptr)

                operation->setOriginPointId(id);
                operation->RefreshGeometry();
                useOriginPoint = true;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::SelectedObject(bool selected, quint32 id, quint32 tool)
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
void DialogMove::rotationChanged()
{
    labelEditFormula = ui->editRotation_Label;
    labelResultCalculation = ui->rotationResult_Label;
    ValFormulaChanged(rotationFlag, ui->rotation_PlainTextEdit, rotationTimer, degreeSymbol);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::editAngleFormula()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
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
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
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
void DialogMove::editRotationFormula()
{
    EditFormulaDialog *dialog = new EditFormulaDialog(data, toolId, this);
    dialog->setWindowTitle(tr("Edit rotation"));
    dialog->SetFormula(getRotation());
    dialog->setPostfix(degreeSymbol);
    if (dialog->exec() == QDialog::Accepted)
    {
        setRotation(dialog->GetFormula());
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
void DialogMove::originChanged(const QString &text)
{
    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    if (text == tr("Center point"))
    {
        operation->setOriginPointId(NULL_ID);
        useOriginPoint = false;
    }
    else
    {
        operation->setOriginPointId(getCurrentObjectId(ui->rotationPoint_ComboBox));
        useOriginPoint = true;
    }
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(angleFlag && lengthFlag && rotationFlag && flagName);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(ok_Button->isEnabled());
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

    rotationFormula = ui->rotation_PlainTextEdit->toPlainText();
    rotationFormula.replace("\n", " ");

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)

    operation->setObjects(sourceToObjects(m_objects));
    operation->SetAngle(angleFormula);
    operation->SetLength(lengthFormula);
    operation->setRotation(rotationFormula);
    operation->setOriginPointId(getOriginPointId());
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::closeEvent(QCloseEvent *event)
{
    ui->angle_PlainTextEdit->blockSignals(true);
    ui->length_PlainTextEdit->blockSignals(true);
    ui->rotation_PlainTextEdit->blockSignals(true);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<SourceItem> DialogMove::getSourceObjects() const
{
    return m_objects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::setSourceObjects(const QVector<SourceItem> &value)
{
    m_objects = value;

    VisToolMove *operation = qobject_cast<VisToolMove *>(vis);
    SCASSERT(operation != nullptr)
    operation->setObjects(sourceToObjects(m_objects));
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

//---------------------------------------------------------------------------------------------------------------------
void DialogMove::evaluateRotation()
{
    labelEditFormula = ui->editRotation_Label;
    Eval(ui->rotation_PlainTextEdit->toPlainText(), rotationFlag, ui->rotationResult_Label, degreeSymbol, false);
}

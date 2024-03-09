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
 **  @file   dialogmirrorbyline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#include "dialogmirrorbyline.h"

#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPointF>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QToolButton>
#include <Qt>
#include <new>

#include "../../visualization/visualization.h"
#include "../../visualization/line/operation/vistoolmirrorbyline.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogmirrorbyline.h"

//---------------------------------------------------------------------------------------------------------------------
DialogMirrorByLine::DialogMirrorByLine(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogMirrorByLine)
    , m_objects()
    , stage1(true)
    , m_suffix()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/mirror_by_line.png"));

    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix(qApp->Settings()->getMirrorByLineSuffix()));

    initializeOkCancelApply(ui);

    FillComboBoxPoints(ui->firstLinePoint_ComboBox);
    FillComboBoxPoints(ui->secondLinePoint_ComboBox);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit,          &QLineEdit::textChanged,        this, &DialogMirrorByLine::suffixChanged);
    connect(ui->firstLinePoint_ComboBox,  &QComboBox::currentTextChanged, this, &DialogMirrorByLine::pointChanged);
    connect(ui->secondLinePoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogMirrorByLine::pointChanged);

    vis = new VisToolMirrorByLine(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogMirrorByLine::~DialogMirrorByLine()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogMirrorByLine::getFirstLinePointId() const
{
    return getCurrentObjectId(ui->firstLinePoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::setFirstLinePointId(quint32 value)
{
    ChangeCurrentData(ui->firstLinePoint_ComboBox, value);
    VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->setFirstLinePointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogMirrorByLine::getSecondLinePointId() const
{
    return getCurrentObjectId(ui->secondLinePoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::setSecondLinePointId(quint32 value)
{
    ChangeCurrentData(ui->secondLinePoint_ComboBox, value);
    VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->setSecondLinePointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMirrorByLine::getSuffix() const
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::setSuffix(const QString &value)
{
    m_suffix = value;
    ui->suffix_LineEdit->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<SourceItem> DialogMirrorByLine::getSourceObjects() const
{
    return m_objects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::setSourceObjects(const QVector<SourceItem> &value)
{
    m_objects = value;

    VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
    SCASSERT(operation != nullptr)
    operation->setObjects(sourceToObjects(m_objects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::ShowDialog(bool click)
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

        VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
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

        emit ToolTip(tr("Select first mirror line point"));
    }
    else if (not stage1 && prepare && click)
    {
        CheckState();
        setModal(true);
        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto object = std::find_if(m_objects.begin(), m_objects.end(),
                                    [id](const SourceItem &item) { return item.id == id; });

            switch (number)
            {
                case 0:
                    if (object != m_objects.end())
                    {
                        emit ToolTip(tr("Select first mirror line point that is not part of the list of objects"));
                        return;
                    }

                    if (SetObject(id, ui->firstLinePoint_ComboBox, tr("Select second mirror line point")))
                    {
                        number++;
                        VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
                        SCASSERT(operation != nullptr)
                        operation->setFirstLinePointId(id);
                        operation->RefreshGeometry();
                    }
                    break;
                case 1:
                    if (object != m_objects.end())
                    {
                        emit ToolTip(tr("Select second mirror line point that is not part of the list of objects"));
                        return;
                    }

                    if (getCurrentObjectId(ui->firstLinePoint_ComboBox) != id)
                    {
                        if (SetObject(id, ui->secondLinePoint_ComboBox, ""))
                        {
                            if (flagError)
                            {
                                number = 0;
                                prepare = true;

                                VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
                                SCASSERT(operation != nullptr)
                                operation->setSecondLinePointId(id);
                                operation->RefreshGeometry();
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::SelectedObject(bool selected, quint32 id, quint32 tool)
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
void DialogMirrorByLine::suffixChanged()
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
void DialogMirrorByLine::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagError && flagName);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(ok_Button->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::ShowVisualization()
{
    AddVisualization<VisToolMirrorByLine>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::SaveData()
{
    m_suffix = ui->suffix_LineEdit->text();

    VisToolMirrorByLine *operation = qobject_cast<VisToolMirrorByLine *>(vis);
    SCASSERT(operation != nullptr)

    operation->setObjects(sourceToObjects(m_objects));
    operation->setFirstLinePointId(getFirstLinePointId());
    operation->setSecondLinePointId(getSecondLinePointId());
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByLine::pointChanged()
{
    QColor color = okColor;
    flagError = true;
    ChangeColor(ui->firstLinePoint_Label, color);
    ChangeColor(ui->secondLinePoint_Label, color);

    quint32 id1 = getCurrentObjectId(ui->firstLinePoint_ComboBox);
    auto objectId1 = std::find_if(m_objects.begin(), m_objects.end(),
                                 [id1](const SourceItem &item) { return item.id == id1;});

    quint32 id2 = getCurrentObjectId(ui->secondLinePoint_ComboBox);
    auto objectId2 = std::find_if(m_objects.begin(), m_objects.end(),
                                 [id2](const SourceItem &item) { return item.id == id2;});

    if (getCurrentObjectId(ui->firstLinePoint_ComboBox) == getCurrentObjectId(ui->secondLinePoint_ComboBox))
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->firstLinePoint_Label, color);
        ChangeColor(ui->secondLinePoint_Label, color);
    }
    else if (objectId1 != m_objects.end())
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->firstLinePoint_Label, color);
    }
    else if (objectId2 != m_objects.end())
    {
        flagError = false;
        color = errorColor;
        ChangeColor(ui->secondLinePoint_Label, color);
    }

    CheckState();
}

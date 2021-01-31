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
 **  @file   dialogmirrorbyaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
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

#include "dialogmirrorbyaxis.h"

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
#include "../../visualization/line/operation/vistoolmirrorbyaxis.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "ui_dialogmirrorbyaxis.h"

//---------------------------------------------------------------------------------------------------------------------
DialogMirrorByAxis::DialogMirrorByAxis(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::DialogMirrorByAxis)
    , objects()
    , stage1(true)
    , m_suffix()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/mirror_by_axis.png"));

    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix());

    InitOkCancelApply(ui);

    FillComboBoxPoints(ui->originPoint_ComboBox);
    fillComboBoxAxisType(ui->axisType_ComboBox);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit, &QLineEdit::textChanged, this, &DialogMirrorByAxis::suffixChanged);
    connect(ui->originPoint_ComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &DialogMirrorByAxis::pointChanged);

    vis = new VisToolMirrorByAxis(data);
}

//---------------------------------------------------------------------------------------------------------------------
DialogMirrorByAxis::~DialogMirrorByAxis()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogMirrorByAxis::getOriginPointId() const
{
    return getCurrentObjectId(ui->originPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::setOriginPointId(quint32 value)
{
    ChangeCurrentData(ui->originPoint_ComboBox, value);
    VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
    SCASSERT(operation != nullptr)
    operation->setOriginPointId(value);
}

//---------------------------------------------------------------------------------------------------------------------
AxisType DialogMirrorByAxis::getAxisType() const
{
    return getCurrentCrossPoint<AxisType>(ui->axisType_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::setAxisType(AxisType type)
{
    auto index = ui->axisType_ComboBox->findData(static_cast<int>(type));
    if (index != -1)
    {
        ui->axisType_ComboBox->setCurrentIndex(index);

        auto operation = qobject_cast<VisToolMirrorByAxis *>(vis);
        SCASSERT(operation != nullptr)
        operation->setAxisType(type);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMirrorByAxis::getSuffix() const
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::setSuffix(const QString &value)
{
    m_suffix = value;
    ui->suffix_LineEdit->setText(value);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> DialogMirrorByAxis::getObjects() const
{
    return objects.toVector();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::ShowDialog(bool click)
{
    if (stage1 && not click)
    {
        if (objects.isEmpty())
        {
            return;
        }

        stage1 = false;

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
        SCASSERT(operation != nullptr)
        operation->setObjects(objects.toVector());
        operation->VisualMode();

        scene->ToggleArcSelection(false);
        scene->ToggleElArcSelection(false);
        scene->ToggleSplineSelection(false);
        scene->ToggleSplinePathSelection(false);

        scene->ToggleArcHover(false);
        scene->ToggleElArcHover(false);
        scene->ToggleSplineHover(false);
        scene->ToggleSplinePathHover(false);

        emit ToolTip(tr("Select axis rotation point"));
    }
    else if (not stage1 && prepare && click)
    {
        setModal(true);
        emit ToolTip("");
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            if (objects.contains(id))
            {
                emit ToolTip(tr("Select axis rotation point that is not part of the list of objects"));
                return;
            }

            if (SetObject(id, ui->originPoint_ComboBox, ""))
            {
                VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
                SCASSERT(operation != nullptr)
                operation->setOriginPointId(id);
                operation->RefreshGeometry();

                prepare = true;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::SelectedObject(bool selected, quint32 object, quint32 tool)
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
void DialogMirrorByAxis::suffixChanged()
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
void DialogMirrorByAxis::CheckState()
{
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(flagError && flagName);
    SCASSERT(bApply != nullptr)
    bApply->setEnabled(bOk->isEnabled());
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::ShowVisualization()
{
    AddVisualization<VisToolMirrorByAxis>();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::SaveData()
{
    m_suffix = ui->suffix_LineEdit->text();

    VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
    SCASSERT(operation != nullptr)

    operation->setObjects(objects.toVector());
    operation->setOriginPointId(getOriginPointId());
    operation->setAxisType(getAxisType());
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::pointChanged()
{
    QColor color = okColor;
    if (objects.contains(getCurrentObjectId(ui->originPoint_ComboBox)))
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
void DialogMirrorByAxis::fillComboBoxAxisType(QComboBox *box)
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Vertical axis"), QVariant(static_cast<int>(AxisType::VerticalAxis)));
    box->addItem(tr("Horizontal axis"), QVariant(static_cast<int>(AxisType::HorizontalAxis)));
}

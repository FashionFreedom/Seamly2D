//-----------------------------------------------------------------------------
//  @file   dialogmirrorbyaxis.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   dialogmirrorbyaxis.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   16 Sep, 2016
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//-----------------------------------------------------------------------------

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
    , m_objects()
    , stage1(true)
    , m_suffix()
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/mirror_by_axis.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->suffix_LineEdit->setText(qApp->getCurrentDocument()->GenerateSuffix(qApp->Settings()->getMirrorByAxisSuffix()));

    initializeOkCancelApply(ui);

    fillComboBoxPoints(ui->originPoint_ComboBox);
    fillComboBoxAxisType(ui->axisType_ComboBox);

    flagName = true;
    CheckState();

    connect(ui->suffix_LineEdit,      &QLineEdit::textChanged,        this, &DialogMirrorByAxis::suffixChanged);
    connect(ui->originPoint_ComboBox, &QComboBox::currentTextChanged, this, &DialogMirrorByAxis::pointChanged);

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
QVector<SourceItem> DialogMirrorByAxis::getSourceObjects() const
{
    return m_objects;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::setSourceObjects(const QVector<SourceItem> &value)
{
    m_objects = value;

    VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
    SCASSERT(operation != nullptr)
    operation->setObjects(sourceToObjects(m_objects));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::ShowDialog(bool click)
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

        VisToolMirrorByAxis *operation = qobject_cast<VisToolMirrorByAxis *>(vis);
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

        emit ToolTip(tr("Select axis rotation point"));
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
void DialogMirrorByAxis::ChosenObject(quint32 id, const SceneObject &type)
{
    if (not stage1 && not prepare)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            auto object = std::find_if(m_objects.begin(), m_objects.end(),
                                    [id](const SourceItem &item) { return item.id == id; });

            if (object != m_objects.end())
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
void DialogMirrorByAxis::SelectedObject(bool selected, quint32 id, quint32 tool)
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
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagError && flagName);
    SCASSERT(apply_Button != nullptr)
    apply_Button->setEnabled(ok_Button->isEnabled());
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

    operation->setObjects(sourceToObjects(m_objects));
    operation->setOriginPointId(getOriginPointId());
    operation->setAxisType(getAxisType());
    operation->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMirrorByAxis::pointChanged()
{
    QColor color = okColor;
    quint32 id = getCurrentObjectId(ui->originPoint_ComboBox);
    auto objectId = std::find_if(m_objects.begin(), m_objects.end(),
                                 [id](const SourceItem &item) { return item.id == id;});

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
void DialogMirrorByAxis::fillComboBoxAxisType(QComboBox *box)
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Vertical axis"), QVariant(static_cast<int>(AxisType::VerticalAxis)));
    box->addItem(tr("Horizontal axis"), QVariant(static_cast<int>(AxisType::HorizontalAxis)));
}

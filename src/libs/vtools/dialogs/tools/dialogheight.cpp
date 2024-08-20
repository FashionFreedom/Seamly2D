//-----------------------------------------------------------------------------
//  @file   dialogheight.cpp
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
//  @file   dialogheight.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
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

#include "dialogheight.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QLineF>
#include <QPoint>
#include <QPointF>
#include <QPointer>
#include <QSet>
#include <QSharedPointer>
#include <new>

#include "../../visualization/line/vistoolheight.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/visline.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "dialogtool.h"
#include "ui_dialogheight.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogHeight create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogHeight::DialogHeight(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent), ui(new Ui::DialogHeight)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/height.png"));

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    ui->lineEditNamePoint->setClearButtonEnabled(true);

    ui->lineEditNamePoint->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->labelEditNamePoint;
    initializeOkCancelApply(ui);
    DialogTool::CheckState();

    fillComboBoxPoints(ui->comboBoxBasePoint);
    fillComboBoxPoints(ui->comboBoxP1Line);
    fillComboBoxPoints(ui->comboBoxP2Line);

    int index = ui->lineColor_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineColor());
    if (index != -1)
    {
        ui->lineColor_ComboBox->setCurrentIndex(index);
    }

    index = ui->lineWeight_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineWeight());
    if (index != -1)
    {
        ui->lineWeight_ComboBox->setCurrentIndex(index);
    }

    index = ui->lineType_ComboBox->findData(qApp->getCurrentDocument()->getDefaultLineType());
    if (index != -1)
    {
        ui->lineType_ComboBox->setCurrentIndex(index);
    }

    connect(ui->lineEditNamePoint, &QLineEdit::textChanged,        this, &DialogHeight::NamePointChanged);
    connect(ui->comboBoxBasePoint, &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);
    connect(ui->comboBoxP1Line,    &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);
    connect(ui->comboBoxP2Line,    &QComboBox::currentTextChanged, this, &DialogHeight::PointNameChanged);

    vis = new VisToolHeight(data);
    // Call after visual initialized.
    // If true current pen overides the default tool pen
    if(!qApp->Settings()->useCurrentPen())
    {
        setLineType(LineTypeDashLine);
        setLineWeight("0.35");
    }
}

//---------------------------------------------------------------------------------------------------------------------
DialogHeight::~DialogHeight()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetPointName set name of point
 * @param value name
 */
void DialogHeight::SetPointName(const QString &value)
{
    pointName = value;
    ui->lineEditNamePoint->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineType return type of line
 * @return type
 */
QString DialogHeight::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeSolidLine);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineType set type of line
 * @param value type
 */
void DialogHeight::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString DialogHeight::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void DialogHeight::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogHeight::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetBasePointId set id base point of height
 * @param value id
 */
void DialogHeight::SetBasePointId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxBasePoint, value);

    VisToolHeight *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->setObject1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP1LineId set id first point of line
 * @param value id
 */
void DialogHeight::SetP1LineId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP1Line, value);

    VisToolHeight *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->setLineP1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP2LineId set id second point of line
 * @param value id
 */
void DialogHeight::SetP2LineId(const quint32 &value)
{
    setCurrentPointId(ui->comboBoxP2Line, value);

    VisToolHeight *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)
    line->setLineP2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogHeight::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            VisToolHeight *line = qobject_cast<VisToolHeight *>(vis);
            SCASSERT(line != nullptr)

            switch (number)
            {
                case (0):
                    if (SetObject(id, ui->comboBoxBasePoint, tr("Select first point of line")))
                    {
                        number++;
                        line->VisualMode(id);
                    }
                    break;
                case (1):
                    if (getCurrentObjectId(ui->comboBoxBasePoint) != id)
                    {
                        if (SetObject(id, ui->comboBoxP1Line, tr("Select second point of line")))
                        {
                            number++;
                            line->setLineP1Id(id);
                            line->RefreshGeometry();
                        }
                    }
                    break;
                case (2):
                {
                    QSet<quint32> set;
                    set.insert(getCurrentObjectId(ui->comboBoxBasePoint));
                    set.insert(getCurrentObjectId(ui->comboBoxP1Line));
                    set.insert(id);

                    if (set.size() == 3)
                    {
                        if (SetObject(id, ui->comboBoxP2Line, ""))
                        {
                            line->setLineP2Id(id);
                            line->RefreshGeometry();
                            prepare = true;
                            DialogAccepted();
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
void DialogHeight::SaveData()
{
    pointName = ui->lineEditNamePoint->text();

    VisToolHeight *line = qobject_cast<VisToolHeight *>(vis);
    SCASSERT(line != nullptr)

    line->setObject1Id(GetBasePointId());
    line->setLineP1Id(GetP1LineId());
    line->setLineP2Id(GetP2LineId());
    line->setLineStyle(lineTypeToPenStyle(getLineType()));
    line->setLineWeight(getLineWeight());
    line->RefreshGeometry();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::PointNameChanged()
{
    QSet<quint32> set;
    const quint32 basePointId = getCurrentObjectId(ui->comboBoxBasePoint);
    const quint32 p1LineId    = getCurrentObjectId(ui->comboBoxP1Line);
    const quint32 p2LineId    = getCurrentObjectId(ui->comboBoxP2Line);

    set.insert(basePointId);
    set.insert(p1LineId);
    set.insert(p2LineId);

    const QPointF basePoint = static_cast<QPointF>(*data->GeometricObject<VPointF>(basePointId));
    const QPointF p1Line    = static_cast<QPointF>(*data->GeometricObject<VPointF>(p1LineId));
    const QPointF p2Line    = static_cast<QPointF>(*data->GeometricObject<VPointF>(p2LineId));

    QColor color = okColor;
    if (set.size() != 3 || VGObject::ClosestPoint(QLineF(p1Line, p2Line), basePoint) == QPointF())
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->labelBasePoint, color);
    ChangeColor(ui->labelFirstLinePoint, color);
    ChangeColor(ui->labelSecondLinePoint, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogHeight::ShowVisualization()
{
    AddVisualization<VisLine>();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBasePointId return id base point of height
 * @return id
 */
quint32 DialogHeight::GetBasePointId() const
{
    return getCurrentObjectId(ui->comboBoxBasePoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1LineId return id first point of line
 * @return id id
 */
quint32 DialogHeight::GetP1LineId() const
{
    return getCurrentObjectId(ui->comboBoxP1Line);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2LineId return id second point of line
 * @return id
 */
quint32 DialogHeight::GetP2LineId() const
{
    return getCurrentObjectId(ui->comboBoxP2Line);
}

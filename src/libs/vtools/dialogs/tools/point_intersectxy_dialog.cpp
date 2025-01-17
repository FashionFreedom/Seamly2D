/**************************************************************************
 **
 **  @file   point_intersectxy_dialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @author Douglas S Caskey
 **  @date   7.20.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#include "point_intersectxy_dialog.h"
#include "ui_point_intersectxy_dialog.h"

#include "dialogtool.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../../visualization/visualization.h"
#include "../../visualization/line/point_intersectxy_visual.h"

#include <QColor>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PointIntersectXYDialog create dialog
 * @param data container with data
 * @param parent parent widget
 */
PointIntersectXYDialog::PointIntersectXYDialog(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : DialogTool(data, toolId, parent)
    , ui(new Ui::PointIntersectXYDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowIcon(QIcon(":/toolicon/32x32/point_intersectxy_icon.png"));

    ui->pointName_LineEdit->setClearButtonEnabled(true);

    ui->pointName_LineEdit->setText(qApp->getCurrentDocument()->GenerateLabel(LabelType::NewLabel));
    labelEditNamePoint = ui->pointName_Label;

    initializeOkCancelApply(ui);
    CheckState();

    FillComboBoxPoints(ui->firstPoint_ComboBox);
    FillComboBoxPoints(ui->secondPoint_ComboBox);

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

    connect(ui->pointName_LineEdit,   &QLineEdit::textChanged,        this, &PointIntersectXYDialog::NamePointChanged);
    connect(ui->firstPoint_ComboBox,  &QComboBox::currentTextChanged, this, &PointIntersectXYDialog::pointChanged);
    connect(ui->secondPoint_ComboBox, &QComboBox::currentTextChanged, this, &PointIntersectXYDialog::pointChanged);

    vis = new PointIntersectXYVisual(data);
    vis->VisualMode(NULL_ID);//Show vertical axis

    // Call after initialization vis!!!!
    setLineType(LineTypeDashLine);
    setLineWeight("0.35");
}

//---------------------------------------------------------------------------------------------------------------------
PointIntersectXYDialog::~PointIntersectXYDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setPointName set name of point
 * @param value name
 */
void PointIntersectXYDialog::setPointName(const QString &value)
{
    pointName = value;
    ui->pointName_LineEdit->setText(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getFirstPointId return id of first point
 * @return id
 */
quint32 PointIntersectXYDialog::getFirstPointId() const
{
    return getCurrentObjectId(ui->firstPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setFirstPointId set id of first point
 * @param value id
 */
void PointIntersectXYDialog::setFirstPointId(const quint32 &value)
{
    setCurrentPointId(ui->firstPoint_ComboBox, value);

    PointIntersectXYVisual *visual = qobject_cast<PointIntersectXYVisual *>(vis);
    SCASSERT(visual != nullptr)
    visual->setPoint1Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getSecondPointId return id of second point
 * @return id
 */
quint32 PointIntersectXYDialog::getSecondPointId() const
{
    return getCurrentObjectId(ui->secondPoint_ComboBox);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setSecondPointId set id of second point
 * @param value id
 */
void PointIntersectXYDialog::setSecondPointId(const quint32 &value)
{
    setCurrentPointId(ui->secondPoint_ComboBox, value);

    PointIntersectXYVisual *visual = qobject_cast<PointIntersectXYVisual *>(vis);
    SCASSERT(visual != nullptr)
    visual->setPoint2Id(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineType return type of the lines
 * @return type
 */
QString PointIntersectXYDialog::getLineType() const
{
    return GetComboBoxCurrentData(ui->lineType_ComboBox, LineTypeDashLine);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineType set type of the lines
 * @param value type
 */
void PointIntersectXYDialog::setLineType(const QString &value)
{
    ChangeCurrentData(ui->lineType_ComboBox, value);
    vis->setLineStyle(lineTypeToPenStyle(value));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineWeight return weight of the lines
 * @return type
 */
QString PointIntersectXYDialog::getLineWeight() const
{
        return GetComboBoxCurrentData(ui->lineWeight_ComboBox, "0.35");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineWeight set weight of the lines
 * @param value type
 */
void PointIntersectXYDialog::setLineWeight(const QString &value)
{
    ChangeCurrentData(ui->lineWeight_ComboBox, value);
    vis->setLineWeight(value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getLineColor get the color of lines
 * @param value type
 */
//---------------------------------------------------------------------------------------------------------------------
QString PointIntersectXYDialog::getLineColor() const
{
    return GetComboBoxCurrentData(ui->lineColor_ComboBox, qApp->Settings()->getSecondarySupportColor());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setLineColor set color of the lines
 * @param value type
 */
//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYDialog::setLineColor(const QString &value)
{
    ChangeCurrentData(ui->lineColor_ComboBox, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void PointIntersectXYDialog::ChosenObject(quint32 id, const SceneObject &type)
{
    if (prepare == false)// After first choose we ignore all objects
    {
        if (type == SceneObject::Point)
        {
            PointIntersectXYVisual *visual = qobject_cast<PointIntersectXYVisual *>(vis);
            SCASSERT(visual != nullptr)

            switch (number)
            {
                case 0:
                    if (SetObject(id, ui->firstPoint_ComboBox, tr("Select point for Y value (horizontal)")))
                    {
                        number++;
                        visual->setPoint1Id(id);
                        visual->RefreshGeometry();
                    }
                    break;
                case 1:
                    if (getCurrentObjectId(ui->firstPoint_ComboBox) != id)
                    {
                        if (SetObject(id, ui->secondPoint_ComboBox, ""))
                        {
                            visual->setPoint2Id(id);
                            visual->RefreshGeometry();
                            prepare = true;
                            DialogAccepted();
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
void PointIntersectXYDialog::pointChanged()
{
    QColor color = okColor;
    if (getFirstPointId() == getSecondPointId())
    {
        flagError = false;
        color = errorColor;
    }
    else
    {
        flagError = true;
        color = okColor;
    }
    ChangeColor(ui->firstPoint_Label, color);
    ChangeColor(ui->secondPoint_Label, color);
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void PointIntersectXYDialog::ShowVisualization()
{
    AddVisualization<PointIntersectXYVisual>();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveData Put dialog data in local variables
 */
 void PointIntersectXYDialog::SaveData()
{
    pointName = ui->pointName_LineEdit->text();

    PointIntersectXYVisual *visual = qobject_cast<PointIntersectXYVisual *>(vis);
    SCASSERT(visual != nullptr)

    visual->setPoint1Id(getFirstPointId());
    visual->setPoint2Id(getSecondPointId());
    visual->setLineStyle(lineTypeToPenStyle(getLineType()));
    visual->setLineWeight(getLineWeight());
    visual->RefreshGeometry();
}

//  @file   new_measurements_dialog.cpp
//  @author Douglas S Caskey
//  @date   3 Jan, 2024
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2024 Seamly2D project
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.

/************************************************************************
 **
 **  @file   dialognewmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "new_measurements_dialog.h"
#include "ui_new_measurements_dialog.h"

#include "../vpatterndb/variables/measurement_variable.h"
#include "../vmisc/vseamlymesettings.h"
#include "../mapplication.h"

#include <QShowEvent>

//---------------------------------------------------------------------------------------------------------------------
NewMeasurementsDialog::NewMeasurementsDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewMeasurementsDialog)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    const VSeamlyMeSettings *settings = qApp->SeamlyMeSettings();

    initializeMeasurementTypes();
    initializeUnits(MeasurementsType::Individual);
    initializeHeightsList();
    initializeSizesList();

    const int height = static_cast<int>(UnitConvertor(settings->GetDefHeight(), Unit::Cm, measurementUnits()));
    int index = ui->comboBoxBaseHeight->findText(QString().setNum(height));
    if (index != -1)
    {
        ui->comboBoxBaseHeight->setCurrentIndex(index);
    }

    const int size = static_cast<int>(UnitConvertor(settings->GetDefSize(), Unit::Cm, measurementUnits()));
    index = ui->comboBoxBaseSize->findText(QString().setNum(size));
    if (index != -1)
    {
        ui->comboBoxBaseSize->setCurrentIndex(index);
    }

    connect(ui->comboBoxMType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &NewMeasurementsDialog::currentTypeChanged);

    connect(ui->comboBoxUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this,
            &NewMeasurementsDialog::currentUnitChanged);
}

//---------------------------------------------------------------------------------------------------------------------
NewMeasurementsDialog::~NewMeasurementsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementsType NewMeasurementsDialog::type() const
{
    return static_cast<MeasurementsType>(ui->comboBoxMType->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
Unit NewMeasurementsDialog::measurementUnits() const
{
    return static_cast<Unit>(ui->comboBoxUnit->currentData().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
int NewMeasurementsDialog::baseSize() const
{
    return ui->comboBoxBaseSize->currentText().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
int NewMeasurementsDialog::baseHeight() const
{
    return ui->comboBoxBaseHeight->currentText().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        initializeMeasurementTypes();
        initializeUnits(static_cast<MeasurementsType>(ui->comboBoxMType->currentData().toInt()));
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::currentTypeChanged(int index)
{
    const MeasurementsType type = static_cast<MeasurementsType>(ui->comboBoxMType->itemData(index).toInt());
    initializeUnits(type);
    initializeHeightsList();
    initializeSizesList();
    if (type == MeasurementsType::Multisize)
    {
        ui->comboBoxBaseSize->setEnabled(true);
        ui->comboBoxBaseHeight->setEnabled(true);
    }
    else
    {
        ui->comboBoxBaseSize->clear();
        ui->comboBoxBaseHeight->clear();
        ui->comboBoxBaseSize->setEnabled(false);
        ui->comboBoxBaseHeight->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::currentUnitChanged(int index)
{
    Q_UNUSED(index)
    int i = ui->comboBoxBaseHeight->currentIndex();
    initializeHeightsList();
    ui->comboBoxBaseHeight->setCurrentIndex(i);

    i = ui->comboBoxBaseSize->currentIndex();
    initializeSizesList();
    ui->comboBoxBaseSize->setCurrentIndex(i);
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::initializeMeasurementTypes()
{
    ui->comboBoxMType->blockSignals(true);
    ui->comboBoxMType->clear();
    ui->comboBoxMType->addItem(tr("Individual"), static_cast<int>(MeasurementsType::Individual));
    ui->comboBoxMType->addItem(tr("Multisize"), static_cast<int>(MeasurementsType::Multisize));
    ui->comboBoxMType->setCurrentIndex(ui->comboBoxMType->findData(static_cast<int>(MeasurementsType::Individual)));
    ui->comboBoxMType->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::initializeHeightsList()
{
    ui->comboBoxBaseHeight->clear();
    if (measurementUnits() != Unit::Inch)
    {
        const QStringList list = MeasurementVariable::WholeListHeights(measurementUnits());
        ui->comboBoxBaseHeight->addItems(list);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::initializeSizesList()
{
    ui->comboBoxBaseSize->clear();
    if (measurementUnits() != Unit::Inch)
    {
        const QStringList list = MeasurementVariable::WholeListSizes(measurementUnits());
        ui->comboBoxBaseSize->addItems(list);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void NewMeasurementsDialog::initializeUnits(const MeasurementsType &type)
{
    ui->comboBoxUnit->blockSignals(true);
    ui->comboBoxUnit->clear();
    qint32 index;

    ui->comboBoxUnit->addItem(tr("Centimeters"), static_cast<int>(Unit::Cm));
    ui->comboBoxUnit->addItem(tr("Millimeters"), static_cast<int>(Unit::Mm));
    if (type == MeasurementsType::Individual)
    {
        ui->comboBoxUnit->addItem(tr("Inches"), static_cast<int>(Unit::Inch));
        index = ui->comboBoxUnit->findData(static_cast<int>(StrToUnits(qApp->SeamlyMeSettings()->GetUnit())));
    }
    else
    {
        index = ui->comboBoxUnit->findData(static_cast<int>(Unit::Cm));
    }

    if (index != -1)
    {
        ui->comboBoxUnit->setCurrentIndex(index);
    }
    else
    {
        index = ui->comboBoxUnit->findData(static_cast<int>(Unit::Cm));
        ui->comboBoxUnit->setCurrentIndex(index);
    }
    ui->comboBoxUnit->blockSignals(false);
}

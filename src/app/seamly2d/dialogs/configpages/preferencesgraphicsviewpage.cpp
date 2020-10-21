/************************************************************************
 **
 **  @file   PreferencesGraphicsViewPage.cpp
 **  @author DS Caskey
 **  @date   10.17.2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Seamly2D project
 **  All Rights Reserved.
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

#include "preferencesgraphicsviewpage.h"
#include "ui_preferencesgraphicsviewpage.h"
#include "../../core/vapplication.h"
#include "../vpatterndb/pmsystems.h"
#include "../vmisc/logging.h"
#include "../vwidgets/vmaingraphicsview.h"

#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTimer>
#include <QtDebug>
#include <QDoubleSpinBox>
#include <QCheckBox>


Q_LOGGING_CATEGORY(vGraphicsViewConfig, "vgraphicsviewconfig")
//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::PreferencesGraphicsViewPage (QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferencesGraphicsViewPage )
    , m_zrbPositiveColorChanged(false)
    , m_zrbNegativeColorChanged(false)
{
    ui->setupUi(this);
    // Appearance preferences
    // Toolbar
    ui->toolBarStyle_CheckBox->setChecked(qApp->Seamly2DSettings()->getToolBarStyle());

    // Antialiasing
    ui->graphicsOutput_CheckBox->setChecked(qApp->Seamly2DSettings()->GetGraphicalOutput());

    // Color preferences
    // Zoom Rubberband colors
    int index = ui->zrbPositiveColor_ComboBox->findText(qApp->Seamly2DSettings()->getZoomRBPositiveColor());
    if (index != -1)
    {
        ui->zrbPositiveColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->zrbPositiveColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_zrbPositiveColorChanged = true;
    });

    index = ui->zrbNegativeColor_ComboBox->findText(qApp->Seamly2DSettings()->getZoomRBNegativeColor());
    if (index != -1)
    {
        ui->zrbNegativeColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->zrbNegativeColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_zrbNegativeColorChanged = true;
    });

    // Navigation preferences
    // Show Scroll Bars
    ui->showScrollBars_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowScrollBars());

    // Scroll Bar Width
    ui->scrollBarWidth_SpinBox->setValue(qApp->Seamly2DSettings()->getScrollBarWidth());

    ui->scrollDuration_SpinBox->setValue(qApp->Seamly2DSettings()->getScrollDuration());
    ui->scrollUpdateInterval_SpinBox->setValue(qApp->Seamly2DSettings()->getScrollUpdateInterval());
    ui->scrollSpeedFactor_Slider->setValue(qApp->Seamly2DSettings()->getScrollSpeedFactor());

    // Zoom Modifier Key
    ui->zoomModKey_CheckBox->setChecked(qApp->Seamly2DSettings()->getZoomModKey());

    // Zoom Speed
    ui->zoomSpeedFactor_Slider->setValue(qApp->Seamly2DSettings()->getZoomSpeedFactor());

    // Behavior preferences
    // Constrain Angle Value & Modifier Key
    ui->constrainValue_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getConstrainValue());
    ui->constrainModKey_CheckBox->setChecked(qApp->Seamly2DSettings()->getConstrainModKey());

    // Zoom double mouse click to selected IsTestModeEnabled
    ui->zoomDoubleClick_CheckBox->setChecked(qApp->Seamly2DSettings()->isZoomDoubleClick());
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::~PreferencesGraphicsViewPage ()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesGraphicsViewPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();

    settings->setToolBarStyle(ui->toolBarStyle_CheckBox->isChecked());

    // Appearance preferences
    // Toolbar
    // Scene antialiasing
    settings->SetGraphicalOutput(ui->graphicsOutput_CheckBox->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::Antialiasing, ui->graphicsOutput_CheckBox->isChecked());
    qApp->getSceneView()->setRenderHint(QPainter::SmoothPixmapTransform, ui->graphicsOutput_CheckBox->isChecked());

    // Color preferences
    // Zoom Rubberband colors
    if (m_zrbPositiveColorChanged)
    {
      settings->setZoomRBPositiveColor(ui->zrbPositiveColor_ComboBox->currentText());
      m_zrbPositiveColorChanged = false;
    }

    if (m_zrbNegativeColorChanged)
    {
      settings->setZoomRBNegativeColor(ui->zrbNegativeColor_ComboBox->currentText());
      m_zrbNegativeColorChanged = false;
    }

    // Navigation preferences
    // Scroll Bars
    settings->setShowScrollBars(ui->showScrollBars_CheckBox->isChecked());
    settings->setScrollBarWidth(ui->scrollBarWidth_SpinBox->value());
    settings->setScrollDuration(ui->scrollDuration_SpinBox->value());
    settings->setScrollUpdateInterval(ui->scrollUpdateInterval_SpinBox->value());
    settings->setScrollSpeedFactor(ui->scrollSpeedFactor_Slider->value());

    // Zoom
    settings->setZoomModKey(ui->zoomModKey_CheckBox->isChecked());
    settings->setZoomSpeedFactor(ui->zoomSpeedFactor_Slider->value());

    // Behavior preferences
    // Constrain Angle Value & Modifier Key
    settings->setConstrainValue(ui->constrainValue_DoubleSpinBox->value());
    settings->setConstrainModKey(ui->constrainModKey_CheckBox->isChecked());

    // Zoom double mouse click to selected IsTestModeEnabled
    settings->setZoomDoubleClick(ui->zoomDoubleClick_CheckBox->isChecked());
}

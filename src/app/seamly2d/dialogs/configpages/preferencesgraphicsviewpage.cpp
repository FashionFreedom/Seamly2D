//-----------------------------------------------------------------------------
//  @file   PreferencesGraphicsViewPage.cpp
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2022 Seamly, LLC
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

#include "preferencesgraphicsviewpage.h"
#include "ui_preferencesgraphicsviewpage.h"
#include "../../core/application_2d.h"
#include "../vpatterndb/pmsystems.h"
#include "../vmisc/logging.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vwidgets/vmaingraphicsview.h"

#include <Qt>
#include <QAbstractButton>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDir>
#include <QDirIterator>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QtDebug>

Q_LOGGING_CATEGORY(vGraphicsViewConfig, "vgraphicsviewconfig")
//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::PreferencesGraphicsViewPage (QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferencesGraphicsViewPage)
    , m_zrbPositiveColorChanged(false)
    , m_zrbNegativeColorChanged(false)
    , m_pointNameColorChanged(false)
    , m_pointNameHoverColorChanged(false)
    , m_orginAxisColorChanged(false)
    , m_primarySupportColorChanged(false)
    , m_secondarySupportColorChanged(false)
    , m_tertiarySupportColorChanged(false)
{
    ui->setupUi(this);
    // Appearance preferences
    // Toolbar
    ui->toolBarStyle_CheckBox->setChecked(qApp->Seamly2DSettings()->getToolBarStyle());
    ui->toolsToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowToolsToolBar());
    ui->pointToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowPointToolBar());
    ui->lineToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowLineToolBar());
    ui->curveToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowCurveToolBar());
    ui->arcToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowArcToolBar());
    ui->operationToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowOpsToolBar());
    ui->pieceToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowPieceToolBar());
    ui->detailsToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowDetailsToolBar());
    ui->layoutToolbar_CheckBox->setChecked(qApp->Seamly2DSettings()->getShowLayoutToolBar());

    ui->useSecondMonitor_CheckBox->setChecked(qApp->Seamly2DSettings()->useSecondMonitor());

    int id = qApp->Seamly2DSettings()->getDialogPosition();
    foreach (QAbstractButton *button, ui->position_ButtonGroup->buttons())
    {
        if (ui->position_ButtonGroup->id(button) == id)
        {
            button->setChecked(true);
            break;
        }
    }
    ui->xOffset_SpinBox->setValue(qApp->Seamly2DSettings()->getXOffset());
    ui->yOffset_SpinBox->setValue(qApp->Seamly2DSettings()->getYOffset());
    enableOffsets();

    connect(ui->position_ButtonGroup, &QButtonGroup::idClicked, this, [this]()
    {
        enableOffsets();
    });

    // Antialiasing
    ui->graphicsOutput_CheckBox->setChecked(qApp->Seamly2DSettings()->GetGraphicalOutput());

    ui->primarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());
    ui->secondarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());
    ui->tertiarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());

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

    index = ui->pointNameColor_ComboBox->findText(qApp->Seamly2DSettings()->getPointNameColor());
    if (index != -1)
    {
        ui->pointNameColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->pointNameColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_pointNameColorChanged = true;
    });

    index = ui->pointNameHoverColor_ComboBox->findText(qApp->Seamly2DSettings()->getPointNameHoverColor());
    if (index != -1)
    {
        ui->pointNameHoverColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->pointNameHoverColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_pointNameHoverColorChanged = true;
    });

    //----------------------- Axis Orgin Color
    index = ui->axisOrginColor_ComboBox->findText(qApp->Seamly2DSettings()->getAxisOrginColor());
    if (index != -1)
    {
        ui->axisOrginColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->axisOrginColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_orginAxisColorChanged = true;
    });

    //----------------------- Selection Support Colors
    index = ui->primarySupportColor_ComboBox->findText(qApp->Seamly2DSettings()->getPrimarySupportColor());
    if (index != -1)
    {
        ui->primarySupportColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->primarySupportColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_primarySupportColorChanged = true;
    });

    index = ui->secondarySupportColor_ComboBox->findText(qApp->Seamly2DSettings()->getSecondarySupportColor());
    if (index != -1)
    {
        ui->secondarySupportColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->secondarySupportColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_secondarySupportColorChanged = true;
    });

    index = ui->tertiarySupportColor_ComboBox->findText(qApp->Seamly2DSettings()->getTertiarySupportColor());
    if (index != -1)
    {
        ui->tertiarySupportColor_ComboBox->setCurrentIndex(index);
    }
    connect(ui->tertiarySupportColor_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        m_tertiarySupportColorChanged = true;
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

    // Export Quality
    ui->quality_Slider->setValue(qApp->Seamly2DSettings()->getExportQuality());

    // Behavior preferences
    // Constrain Angle Value & Modifier Key
    ui->constrainValue_DoubleSpinBox->setValue(qApp->Seamly2DSettings()->getConstrainValue());
    ui->constrainModKey_CheckBox->setChecked(qApp->Seamly2DSettings()->getConstrainModKey());

    // Zoom double mouse click to selected IsTestModeEnabled
    ui->zoomDoubleClick_CheckBox->setChecked(qApp->Seamly2DSettings()->isZoomDoubleClick());

    // Pan Zoom while Space Key pressed
    ui->panActiveSpacePressed_CheckBox->setChecked(qApp->Seamly2DSettings()->isPanActiveSpaceKey());

    // Always use current pen
    ui->useCurrentPen_checkBox->setChecked(qApp->Seamly2DSettings()->useCurrentPen());

    // Font preferences
    // Pattern piece labels font
    ui->labelFont_ComboBox->setCurrentFont(qApp->Seamly2DSettings()->getLabelFont());

    // Point name font
    QFont nameFont = qApp->Seamly2DSettings()->getPointNameFont();
    ui->pointNameFont_ComboBox->setCurrentFont(nameFont);
    nameFont.setPointSize(12);
    ui->pointName_Label->setFont(nameFont);

    connect(ui->pointNameFont_ComboBox,
            static_cast<void(QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont nameFont)
    {
        nameFont.setPointSize(12);
        ui->pointName_Label->setFont(nameFont);
    });

    index = ui->pointNameFontSize_ComboBox->findText(QString().setNum(qApp->Seamly2DSettings()->getPointNameSize()));
    if (index != -1)
    {
        ui->pointNameFontSize_ComboBox->setCurrentIndex(index);
    }

    // GUI font
    QFont guiFont = qApp->Seamly2DSettings()->getGuiFont();
    ui->guiFont_ComboBox->setCurrentFont(guiFont);
    guiFont.setPointSize(12);
    ui->gui_Label->setFont(guiFont);

    connect(ui->guiFont_ComboBox,
            static_cast<void(QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont guiFont)
    {
        guiFont.setPointSize(12);
        ui->gui_Label->setFont(guiFont);
    });

    index = ui->guiFontSize_ComboBox->findText(QString().setNum(qApp->Seamly2DSettings()->getGuiFontSize()));
    if (index != -1)
    {
        ui->guiFontSize_ComboBox->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::~PreferencesGraphicsViewPage ()
{
    delete ui;
}

// @brief enableOffsets() enable offset spinboxes.
//
// This method enables / disables the offset spinboxes based on the radio button checked.
//
// @Details
//  - Enables spinboxes when the Offset radio button is checked.
//  - Disables spinboxes when any other radio button is checked.
void PreferencesGraphicsViewPage::enableOffsets()
{
    if (ui->offset_RadioButton->isChecked())
    {
        ui->xOffset_SpinBox->setEnabled(true);
        ui->yOffset_SpinBox->setEnabled(true);
    }
    else
    {
        ui->xOffset_SpinBox->setEnabled(false);
        ui->yOffset_SpinBox->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesGraphicsViewPage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void PreferencesGraphicsViewPage::Apply()
{
    VSettings *settings = qApp->Seamly2DSettings();

    settings->setToolBarStyle(ui->toolBarStyle_CheckBox->isChecked());
    settings->setShowToolsToolBar(ui->toolsToolbar_CheckBox->isChecked());
    settings->setShowPointToolBar(ui->pointToolbar_CheckBox->isChecked());
    settings->setShowLineToolBar(ui->lineToolbar_CheckBox->isChecked());
    settings->setShowCurveToolBar(ui->curveToolbar_CheckBox->isChecked());
    settings->setShowArcToolBar(ui->arcToolbar_CheckBox->isChecked());
    settings->setShowOpsToolBar(ui->operationToolbar_CheckBox->isChecked());
    settings->setShowPieceToolBar(ui->pieceToolbar_CheckBox->isChecked());
    settings->setShowDetailsToolBar(ui->detailsToolbar_CheckBox->isChecked());
    settings->setShowLayoutToolBar(ui->layoutToolbar_CheckBox->isChecked());

    settings->setUseSecondMonitor(ui->useSecondMonitor_CheckBox->isChecked());
    settings->setDialogPosition(ui->position_ButtonGroup->checkedId());
    settings->setXOffset(ui->xOffset_SpinBox->value());
    settings->setYOffset(ui->yOffset_SpinBox->value());

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

    // Point Name colors
    if (m_pointNameColorChanged)
    {
      settings->setPointNameColor(ui->pointNameColor_ComboBox->currentText());
      m_pointNameColorChanged = false;
    }

    if (m_pointNameHoverColorChanged)
    {
      settings->setPointNameHoverColor(ui->pointNameHoverColor_ComboBox->currentText());
      m_pointNameHoverColorChanged = false;
    }

    if (m_orginAxisColorChanged)
    {
      settings->setAxisOrginColor(ui->axisOrginColor_ComboBox->currentText());
      m_orginAxisColorChanged = false;
    }

    if (m_primarySupportColorChanged)
    {
      settings->setPrimarySupportColor(ui->primarySupportColor_ComboBox->currentText());
      m_primarySupportColorChanged = false;
    }

    if (m_secondarySupportColorChanged)
    {
      settings->setSecondarySupportColor(ui->secondarySupportColor_ComboBox->currentText());
      m_secondarySupportColorChanged = false;
    }

    if (m_tertiarySupportColorChanged)
    {
      settings->setTertiarySupportColor(ui->tertiarySupportColor_ComboBox->currentText());
      m_tertiarySupportColorChanged = false;
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

    // Export Quality
    settings->setExportQuality(ui->quality_Slider->value());

    // Behavior preferences
    // Constrain Angle Value & Modifier Key
    settings->setConstrainValue(ui->constrainValue_DoubleSpinBox->value());
    settings->setConstrainModKey(ui->constrainModKey_CheckBox->isChecked());

    // Zoom double mouse click to selected IsTestModeEnabled
    settings->setZoomDoubleClick(ui->zoomDoubleClick_CheckBox->isChecked());

    // Pan Zoom while Space key pressed
    settings->setPanActiveSpaceKey(ui->panActiveSpacePressed_CheckBox->isChecked());

    // Always use current pen
    settings->setUseCurrentPen(ui->useCurrentPen_checkBox->isChecked());

    //Fonts
    settings->setLabelFont(ui->labelFont_ComboBox->currentFont());

    settings->setGuiFont(ui->guiFont_ComboBox->currentFont());
    settings->setGuiFontSize(ui->guiFontSize_ComboBox->currentText().toInt());

    settings->setPointNameFont(ui->pointNameFont_ComboBox->currentFont());
    settings->setPointNameSize(ui->pointNameFontSize_ComboBox->currentText().toInt());
}

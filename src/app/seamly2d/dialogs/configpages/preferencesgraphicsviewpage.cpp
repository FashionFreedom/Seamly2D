//-----------------------------------------------------------------------------
//  @file   PreferencesGraphicsViewPage.cpp
//  @author Douglas S Caskey
//  @date   26 Oct, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program to create and model patterns of clothing.
//  Copyright (C) 2017-2025 Seamly2D project
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
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
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QDoubleSpinBox>
#include <QFontComboBox>
#include <QMessageBox>
#include <QPixmap>
#include <QTimer>
#include <QtDebug>
#include <exception>

Q_LOGGING_CATEGORY(vGraphicsViewConfig, "vgraphicsviewconfig")

//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::PreferencesGraphicsViewPage (QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PreferencesGraphicsViewPage)
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

    // Color preferences
    // Background Color
    ui->bgColor_ComboBox->setItems(VAbstractTool::backgroundColorsList());
    setIndex(ui->bgColor_ComboBox, qApp->Seamly2DSettings()->getBackgroundColor());

    // Zoom Rubberband colors
    setIndex(ui->zrbPositiveColor_ComboBox, qApp->Seamly2DSettings()->getZoomRBPositiveColor());
    setIndex(ui->zrbNegativeColor_ComboBox, qApp->Seamly2DSettings()->getZoomRBNegativeColor());

    // Point name colors
    setIndex(ui->pointNameColor_ComboBox, qApp->Seamly2DSettings()->getPointNameColor());
    setIndex(ui->pointNameHoverColor_ComboBox, qApp->Seamly2DSettings()->getPointNameHoverColor());

    // Axis Orgin Color
    setIndex(ui->axisOrginColor_ComboBox, qApp->Seamly2DSettings()->getAxisOrginColor());

    // Selection Support Colors
    ui->primarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());
    ui->secondarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());
    ui->tertiarySupportColor_ComboBox->setItems(VAbstractTool::supportColorsList());
    setIndex(ui->primarySupportColor_ComboBox, qApp->Seamly2DSettings()->getPrimarySupportColor());
    setIndex(ui->secondarySupportColor_ComboBox, qApp->Seamly2DSettings()->getSecondarySupportColor());
    setIndex(ui->tertiarySupportColor_ComboBox, qApp->Seamly2DSettings()->getTertiarySupportColor());

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
    ui->showOnlyIso_CheckBox->setChecked(qApp->Seamly2DSettings()->showOnlyIso());

    // Autoclear FX formula
    ui->autoClearFx_CheckBox->setChecked(qApp->Seamly2DSettings()->autoClearFx());

    // Font preferences
    // Pattern piece labels font
    QFont labelFont = qApp->Seamly2DSettings()->getLabelFont();
    labelFont.setPointSize(12);
    setFontComboBox(ui->labelFont_ComboBox, labelFont);
    ui->label_Label->setFont(labelFont);

    connect(ui->labelFont_ComboBox,
            static_cast<void(QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont labelFont)
    {
        labelFont.setPointSize(12);
        ui->label_Label->setFont(labelFont);
    });

    // Point name font
    QFont nameFont = qApp->Seamly2DSettings()->getPointNameFont();
    setFontComboBox(ui->pointNameFont_ComboBox, nameFont);

    int index = ui->pointNameFontSize_ComboBox->findText(QString().setNum(qApp->Seamly2DSettings()->getPointNameSize()));
    if (index != -1)
    {
        ui->pointNameFontSize_ComboBox->setCurrentIndex(index);
    }

    nameFont.setPointSize(ui->pointNameFontSize_ComboBox->currentText().toInt());
    ui->pointName_Label->setFont(nameFont);

    connect(ui->pointNameFont_ComboBox,
            static_cast<void(QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont nameFont)
    {
        nameFont.setPointSize(ui->pointNameFontSize_ComboBox->currentText().toInt());
        ui->pointName_Label->setFont(nameFont);
    });

    connect(ui->pointNameFontSize_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        QFont labelFont = ui->pointName_Label->font();
        labelFont.setPointSize(ui->pointNameFontSize_ComboBox->currentText().toInt());
        ui->pointName_Label->setFont(labelFont);
    });

    // GUI font
    QFont guiFont = qApp->Seamly2DSettings()->getGuiFont();
    setFontComboBox(ui->guiFont_ComboBox, guiFont);

    index = ui->guiFontSize_ComboBox->findText(QString().setNum(qApp->Seamly2DSettings()->getGuiFontSize()));
    if (index != -1)
    {
        ui->guiFontSize_ComboBox->setCurrentIndex(index);
    }

    guiFont.setPointSize(ui->guiFontSize_ComboBox->currentText().toInt());
    ui->gui_Label->setFont(guiFont);

    connect(ui->guiFont_ComboBox,
            static_cast<void(QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont guiFont)
    {
        guiFont.setPointSize(ui->guiFontSize_ComboBox->currentText().toInt());
        ui->gui_Label->setFont(guiFont);
    });

    connect(ui->guiFontSize_ComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]()
    {
        QFont guiFont = ui->gui_Label->font();
        guiFont.setPointSize(ui->guiFontSize_ComboBox->currentText().toInt());
        ui->gui_Label->setFont(guiFont);
    });
}

//---------------------------------------------------------------------------------------------------------------------
PreferencesGraphicsViewPage::~PreferencesGraphicsViewPage ()
{
    delete ui;
}

/// @brief enableOffsets() enable offset spinboxes.
///
/// This method enables / disables the offset spinboxes based on the radio button checked.
///
/// @Details
///  - Enables spinboxes when the Offset radio button is checked.
///  - Disables spinboxes when any other radio button is checked.
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
    // Background color
    settings->setBackgroundColor(ui->bgColor_ComboBox->currentData().toString());


    // Zoom Rubberband colors
    settings->setZoomRBPositiveColor(ui->zrbPositiveColor_ComboBox->currentData().toString());
    settings->setZoomRBNegativeColor(ui->zrbNegativeColor_ComboBox->currentData().toString());

    // Point Name colors
    settings->setPointNameColor(ui->pointNameColor_ComboBox->currentData().toString());
    settings->setPointNameHoverColor(ui->pointNameHoverColor_ComboBox->currentData().toString());

    // Avxi Origin color
    settings->setAxisOrginColor(ui->axisOrginColor_ComboBox->currentData().toString());

    // Support colors
    settings->setPrimarySupportColor(ui->primarySupportColor_ComboBox->currentData().toString());
    settings->setSecondarySupportColor(ui->secondarySupportColor_ComboBox->currentData().toString());
    settings->setTertiarySupportColor(ui->tertiarySupportColor_ComboBox->currentData().toString());

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

    // Pen
    settings->setUseCurrentPen(ui->useCurrentPen_checkBox->isChecked());
    settings->setShowIsoOnly(ui->showOnlyIso_CheckBox->isChecked());

    // Formula Editor
    settings->setAutoClearFx(ui->autoClearFx_CheckBox->isChecked());

    //Fonts
    settings->setLabelFont(ui->labelFont_ComboBox->currentFont());

    settings->setGuiFont(ui->guiFont_ComboBox->currentFont());
    settings->setGuiFontSize(ui->guiFontSize_ComboBox->currentText().toInt());

    settings->setPointNameFont(ui->pointNameFont_ComboBox->currentFont());
    settings->setPointNameSize(ui->pointNameFontSize_ComboBox->currentText().toInt());
}

void PreferencesGraphicsViewPage::setIndex(QComboBox *box, const QString &text)
{
    int index;
    index = box->findData(text);
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
    else
    {
        box->setCurrentIndex(box->findText(text));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setFontComboBox() safely set the selected font in a QFontComboBox.
///
/// @Details
///  - On macOS, font enumeration in QFontComboBox can cause exceptions during initialization.
///  - This method defers initialization by 100ms to avoid crashes during dialog setup.
///  - Includes exception handling with fallback to setFont() if setCurrentFont() fails.
///  - Logs warnings when exceptions occur for debugging purposes.
///
/// @param[in] combo The QFontComboBox to configure.
/// @param[in] font The font to set as current.
void PreferencesGraphicsViewPage::setFontComboBox(QFontComboBox *combo, const QFont &font)
{
    if (!combo)
        return;

    try
    {
#if defined(Q_OS_MAC)
        QTimer::singleShot(100, combo, [combo, font]()
        {
            try
            {
                combo->setCurrentFont(font);
            }
            catch (const std::exception &e)
            {
                qCWarning(vGraphicsViewConfig) << "Exception setting font on macOS:" << e.what();
                combo->setCurrentText(font.family());
                combo->setFont(font);
            }
            catch (...)
            {
                qCWarning(vGraphicsViewConfig) << "Unknown exception setting font on macOS";
                combo->setCurrentText(font.family());
                combo->setFont(font);
            }
        });
#else
        combo->setCurrentFont(font);
#endif
    }
    catch (const std::exception &e)
    {
        qCWarning(vGraphicsViewConfig) << "Exception in setFontComboBox:" << e.what();
        if (combo)
            combo->setFont(font);
    }
    catch (...)
    {
        qCWarning(vGraphicsViewConfig) << "Unknown exception in setFontComboBox";
        if (combo)
            combo->setFont(font);
    }
}

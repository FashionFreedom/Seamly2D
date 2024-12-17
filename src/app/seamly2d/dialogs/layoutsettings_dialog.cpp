/******************************************************************************
 *   @file   layoutsettings_dialog.cpp
 **  @author Douglas S Caskey
 **  @date   19 Oct, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

/************************************************************************
 **
 **  @file   LayoutSettingsDialog.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "layoutsettings_dialog.h"
#include "ui_layoutsettings_dialog.h"
#include "../core/application_2d.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/vmath.h"
#include "../vlayout/vlayoutgenerator.h"
#include "../vwidgets/page_format_combobox.h"

#include <QMessageBox>
#include <QPushButton>
#include <QPrinterInfo>

//---------------------------------------------------------------------------------------------------------------------
LayoutSettingsDialog::LayoutSettingsDialog(VLayoutGenerator *generator, QWidget *parent, bool disableSettings)
    : AbstractLayoutDialog(parent)
    , ui(new Ui::LayoutSettingsDialog)
    , disableSettings(disableSettings)
    , oldPaperUnit(Unit::Mm)
    , oldLayoutUnit(Unit::Mm)
    , generator(generator)
    , isInitialized(false)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    qApp->Seamly2DSettings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    //moved from ReadSettings - well...it seems it can be done once only (i.e. constructor) because Init funcs dont
    //even cleanse lists before adding
    InitPaperUnits();
    InitLayoutUnits();
    MinimumPaperSize();
    MinimumLayoutSize();
    InitPrinter();

    //in export console mode going to use defaults
    if (disableSettings == false)
    {
        ReadSettings();
    }
    else
    {
        RestoreDefaults();
    }

    connect(ui->comboBoxPrinter, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LayoutSettingsDialog::PrinterMargins);

    connect(ui->comboBoxTemplates, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LayoutSettingsDialog::TemplateSelected);
    connect(ui->comboBoxPaperSizeUnit, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LayoutSettingsDialog::ConvertPaperSize);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::PaperSizeChanged);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::PaperSizeChanged);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::FindTemplate);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::FindTemplate);

    connect(ui->doubleSpinBoxPaperWidth, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::CorrectMaxFileds);
    connect(ui->doubleSpinBoxPaperHeight, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &LayoutSettingsDialog::CorrectMaxFileds);

    connect(ui->checkBoxIgnoreFileds, &QCheckBox::stateChanged, this, &LayoutSettingsDialog::IgnoreAllFields);

    connect(ui->portrait_ToolButton, &QToolButton::toggled, this, &LayoutSettingsDialog::Swap);
    connect(ui->landscape_ToolButton, &QToolButton::toggled, this, &LayoutSettingsDialog::Swap);
    connect(ui->comboBoxLayoutUnit,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &LayoutSettingsDialog::ConvertLayoutSize);

    QPushButton *ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(ok_Button, &QPushButton::clicked, this, &LayoutSettingsDialog::DialogAccepted);

    QPushButton *bRestoreDefaults = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults);
    connect(bRestoreDefaults, &QPushButton::clicked, this, &LayoutSettingsDialog::RestoreDefaults);
}

//---------------------------------------------------------------------------------------------------------------------
LayoutSettingsDialog::~LayoutSettingsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::GetPaperHeight() const
{
    return UnitConvertor(ui->doubleSpinBoxPaperHeight->value(), oldPaperUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetPaperHeight(qreal value)
{
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, PaperUnit()));
    ui->doubleSpinBoxPaperHeight->setValue(UnitConvertor(value, Unit::Px, PaperUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::GetPaperWidth() const
{
    return UnitConvertor(ui->doubleSpinBoxPaperWidth->value(), oldPaperUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetPaperWidth(qreal value)
{
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, PaperUnit()));
    ui->doubleSpinBoxPaperWidth->setValue(UnitConvertor(value, Unit::Px, PaperUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::GetShift() const
{
    return UnitConvertor(ui->doubleSpinBoxShift->value(), oldLayoutUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetShift(qreal value)
{
    ui->doubleSpinBoxShift->setValue(UnitConvertor(value, Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::getLayoutGap() const
{
    return UnitConvertor(ui->doubleSpinBoxLayoutWidth->value(), oldLayoutUnit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::setLayoutGap(qreal value)
{
    ui->doubleSpinBoxLayoutWidth->setValue(UnitConvertor(value, Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF LayoutSettingsDialog::GetFields() const
{
    QMarginsF fields;
    fields.setLeft(UnitConvertor(ui->leftField_DoubleSpinBox->value(), oldLayoutUnit, Unit::Px));
    fields.setRight(UnitConvertor(ui->rightField_DoubleSpinBox->value(), oldLayoutUnit, Unit::Px));
    fields.setTop(UnitConvertor(ui->topField_DoubleSpinBox->value(), oldLayoutUnit, Unit::Px));
    fields.setBottom(UnitConvertor(ui->bottomField_DoubleSpinBox->value(), oldLayoutUnit, Unit::Px));
    return fields;
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetFields(const QMarginsF &value)
{
    ui->leftField_DoubleSpinBox->setValue(UnitConvertor(value.left(), Unit::Px, LayoutUnit()));
    ui->rightField_DoubleSpinBox->setValue(UnitConvertor(value.right(), Unit::Px, LayoutUnit()));
    ui->topField_DoubleSpinBox->setValue(UnitConvertor(value.top(), Unit::Px, LayoutUnit()));
    ui->bottomField_DoubleSpinBox->setValue(UnitConvertor(value.bottom(), Unit::Px, LayoutUnit()));
}

//---------------------------------------------------------------------------------------------------------------------
Cases LayoutSettingsDialog::GetGroup() const
{
    if (ui->radioButtonThreeGroups->isChecked())
    {
        return Cases::CaseThreeGroup;
    }
    else if (ui->radioButtonTwoGroups->isChecked())
    {
        return Cases::CaseTwoGroup;
    }
    else
    {
        return Cases::CaseDesc;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void LayoutSettingsDialog::SetGroup(const Cases &value)
{
    switch (value)
    {
        case Cases::CaseThreeGroup:
            ui->radioButtonThreeGroups->setChecked(true);
            break;
        case Cases::CaseTwoGroup:
            ui->radioButtonTwoGroups->setChecked(true);
            break;
        case Cases::CaseDesc:
        default:
            ui->radioButtonDescendingArea->setChecked(true);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::GetRotate() const
{
    return ui->groupBoxRotate->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetRotate(bool state)
{
    ui->groupBoxRotate->setChecked(state);
}

//---------------------------------------------------------------------------------------------------------------------
int LayoutSettingsDialog::GetIncrease() const
{
    return ui->comboBoxIncrease->currentText().toInt();
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
bool LayoutSettingsDialog::SetIncrease(int increase)
{
    int index = ui->comboBoxIncrease->findText(QString::number(increase));
    bool failed = (index == -1);
    if (failed)
    {
        const QString def = QString::number(VSettings::GetDefLayoutRotationIncrease());// Value by default
        index = ui->comboBoxIncrease->findText(def);
    }

    ui->comboBoxIncrease->setCurrentIndex(index);
    return failed;
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::GetAutoCrop() const
{
    return ui->checkBoxAutoCrop->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetAutoCrop(bool autoCrop)
{
    ui->checkBoxAutoCrop->setChecked(autoCrop);
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::IsSaveLength() const
{
    return ui->checkBoxSaveLength->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetSaveLength(bool save)
{
    ui->checkBoxSaveLength->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::IsUnitePages() const
{
    return ui->checkBoxUnitePages->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetUnitePages(bool save)
{
    ui->checkBoxUnitePages->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::useStripOptimization() const
{
    return ui->groupBoxStrips->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::setStripOptimization(bool save)
{
    ui->groupBoxStrips->setChecked(save);
}

//---------------------------------------------------------------------------------------------------------------------
quint8 LayoutSettingsDialog::GetMultiplier() const
{
    return static_cast<quint8>(ui->spinBoxMultiplier->value());
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetMultiplier(const quint8 &value)
{
    ui->spinBoxMultiplier->setValue(static_cast<int>(value));
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::IsIgnoreAllFields() const
{
    return ui->checkBoxIgnoreFileds->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetIgnoreAllFields(bool value)
{
    ui->checkBoxIgnoreFileds->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::isTextAsPaths() const
{
    return ui->textAsPaths_Checkbox->isChecked();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::setTextAsPaths(bool value)
{
    ui->textAsPaths_Checkbox->setChecked(value);
}

//---------------------------------------------------------------------------------------------------------------------
QString LayoutSettingsDialog::SelectedPrinter() const
{
    return ui->comboBoxPrinter->currentText();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::TemplateSelected()
{
    SheetSize(Template());
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::FindTemplate()
{
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();
    QSizeF size(width, height);

    const Unit paperUnit = PaperUnit();

    const int max = static_cast<int>(PaperSizeFormat::Custom);
    for (int i=0; i < max; ++i)
    {
        const QSizeF tmplSize = getTemplateSize(static_cast<PaperSizeFormat>(i), paperUnit);
        if (size == tmplSize)
        {
            ui->comboBoxTemplates->blockSignals(true);
            const int index = ui->comboBoxTemplates->findData(i);
            if (index != -1)
            {
                ui->comboBoxTemplates->setCurrentIndex(index);
            }
            ui->comboBoxTemplates->blockSignals(false);
            return;
        }
    }

    ui->comboBoxTemplates->blockSignals(true);
    const int index = ui->comboBoxTemplates->findData(max);
    if (index != -1)
    {
        ui->comboBoxTemplates->setCurrentIndex(index);
    }
    ui->comboBoxTemplates->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::ConvertPaperSize()
{
    const Unit paperUnit = PaperUnit();
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();

    const qreal left = ui->leftField_DoubleSpinBox->value();
    const qreal right = ui->rightField_DoubleSpinBox->value();
    const qreal top = ui->topField_DoubleSpinBox->value();
    const qreal bottom = ui->bottomField_DoubleSpinBox->value();

    ui->doubleSpinBoxPaperWidth->blockSignals(true);
    ui->doubleSpinBoxPaperHeight->blockSignals(true);
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, paperUnit));
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, paperUnit));
    ui->doubleSpinBoxPaperWidth->blockSignals(false);
    ui->doubleSpinBoxPaperHeight->blockSignals(false);

    const qreal newWidth = UnitConvertor(width, oldPaperUnit, paperUnit);
    const qreal newHeight = UnitConvertor(height, oldPaperUnit, paperUnit);

    const qreal newLeft = UnitConvertor(left, oldPaperUnit, paperUnit);
    const qreal newRight = UnitConvertor(right, oldPaperUnit, paperUnit);
    const qreal newTop = UnitConvertor(top, oldPaperUnit, paperUnit);
    const qreal newBottom = UnitConvertor(bottom, oldPaperUnit, paperUnit);

    oldPaperUnit = paperUnit;
    CorrectPaperDecimals();
    MinimumPaperSize();

    ui->doubleSpinBoxPaperWidth->setValue(newWidth);
    ui->doubleSpinBoxPaperHeight->setValue(newHeight);

    ui->leftField_DoubleSpinBox->setValue(newLeft);
    ui->rightField_DoubleSpinBox->setValue(newRight);
    ui->topField_DoubleSpinBox->setValue(newTop);
    ui->bottomField_DoubleSpinBox->setValue(newBottom);
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::SelectPaperUnit(const QString& units)
{
    qint32 indexUnit = ui->comboBoxPaperSizeUnit->findData(units);
    if (indexUnit != -1)
    {
        ui->comboBoxPaperSizeUnit->setCurrentIndex(indexUnit);
    }
    return indexUnit != -1;
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::SelectLayoutUnit(const QString &units)
{
    qint32 indexUnit = ui->comboBoxLayoutUnit->findData(units);
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
    return indexUnit != -1;
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::LayoutToPixels(qreal value) const
{
    return UnitConvertor(value, LayoutUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal LayoutSettingsDialog::PageToPixels(qreal value) const
{
    return UnitConvertor(value, PaperUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QString LayoutSettingsDialog::MakeGroupsHelp()
{
    //that is REALLY dummy ... can't figure fast how to automate generation... :/
    return tr("\n\tThree groups: big, middle, small = 0;\n\tTwo groups: big, small = 1;\n\tDescending area = 2");
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::showEvent(QShowEvent *event)
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
void LayoutSettingsDialog::ConvertLayoutSize()
{
    const Unit unit = LayoutUnit();
    const qreal layoutWidth = ui->doubleSpinBoxLayoutWidth->value();
    const qreal shift = ui->doubleSpinBoxShift->value();

    ui->doubleSpinBoxLayoutWidth->setMaximum(FromPixel(QIMAGE_MAX, unit));
    ui->doubleSpinBoxShift->setMaximum(FromPixel(QIMAGE_MAX, unit));

    const qreal newLayoutWidth = UnitConvertor(layoutWidth, oldLayoutUnit, unit);
    const qreal newShift = UnitConvertor(shift, oldLayoutUnit, unit);

    oldLayoutUnit = unit;
    CorrectLayoutDecimals();
    MinimumLayoutSize();

    ui->doubleSpinBoxLayoutWidth->setValue(newLayoutWidth);
    ui->doubleSpinBoxShift->setValue(newShift);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::PaperSizeChanged()
{
    if (ui->doubleSpinBoxPaperHeight->value() > ui->doubleSpinBoxPaperWidth->value())
    {
        ui->portrait_ToolButton->blockSignals(true);
        ui->portrait_ToolButton->setChecked(true);
        ui->portrait_ToolButton->blockSignals(false);
    }
    else
    {
        ui->landscape_ToolButton->blockSignals(true);
        ui->landscape_ToolButton->setChecked(true);
        ui->landscape_ToolButton->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool LayoutSettingsDialog::SelectTemplate(const PaperSizeFormat& id)
{
    int index = ui->comboBoxTemplates->findData(static_cast<int>(id));
    if (index > -1)
    {
        ui->comboBoxTemplates->setCurrentIndex(index);
    }

    return (index > -1);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::Swap(bool checked)
{
    if (checked)
    {
        const qreal width = ui->doubleSpinBoxPaperWidth->value();
        const qreal height = ui->doubleSpinBoxPaperHeight->value();

        ui->doubleSpinBoxPaperWidth->blockSignals(true);
        ui->doubleSpinBoxPaperWidth->setValue(height);
        ui->doubleSpinBoxPaperWidth->blockSignals(false);

        ui->doubleSpinBoxPaperHeight->blockSignals(true);
        ui->doubleSpinBoxPaperHeight->setValue(width);
        ui->doubleSpinBoxPaperHeight->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::DialogAccepted()
{
    SCASSERT(generator != nullptr)
    generator->setLayoutGap(getLayoutGap());
    generator->setCaseType(GetGroup());
    generator->SetPaperHeight(GetPaperHeight());
    generator->SetPaperWidth(GetPaperWidth());
    generator->SetShift(static_cast<quint32>(qFloor(GetShift())));
    generator->SetRotate(GetRotate());
    generator->SetRotationIncrease(GetIncrease());
    generator->SetAutoCrop(GetAutoCrop());
    generator->SetSaveLength(IsSaveLength());
    generator->SetUnitePages(IsUnitePages());
    generator->setStripOptimization(useStripOptimization());
    generator->SetMultiplier(GetMultiplier());
    generator->setTextAsPaths(isTextAsPaths());

    if (IsIgnoreAllFields())
    {
        generator->SetPrinterFields(false, QMarginsF());
    }
    else
    {
        QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
        if (printer.isNull())
        {
            generator->SetPrinterFields(true, GetFields());
        }
        else
        {
            const QMarginsF minFields = MinPrinterFields();
            const QMarginsF fields = GetFields();
            if (fields.left() < minFields.left() || fields.right() < minFields.right() ||
                fields.top() < minFields.top() || fields.bottom() < minFields.bottom())
            {
                QMessageBox::StandardButton answer;
                answer = QMessageBox::question(this, tr("Wrong fields."),
                                               tr("Margins go beyond printing. \n\nApply settings anyway?"),
                                               QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                if (answer == QMessageBox::No)
                {
                    if (fields.left() < minFields.left())
                    {
                        ui->leftField_DoubleSpinBox->setValue(UnitConvertor(minFields.left(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.right() < minFields.right())
                    {
                        ui->rightField_DoubleSpinBox->setValue(UnitConvertor(minFields.right(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.top() < minFields.top())
                    {
                        ui->topField_DoubleSpinBox->setValue(UnitConvertor(minFields.top(), Unit::Px, LayoutUnit()));
                    }

                    if (fields.bottom() < minFields.bottom())
                    {
                        ui->bottomField_DoubleSpinBox->setValue(UnitConvertor(minFields.bottom(), Unit::Px,
                                                                             LayoutUnit()));
                    }

                    generator->SetPrinterFields(true, GetFields());
                }
                else
                {
                    generator->SetPrinterFields(false, GetFields());
                }
            }
            else
            {
                generator->SetPrinterFields(true, GetFields());
            }
        }
    }

    //don't want to break visual settings when cmd used
    if (disableSettings == false)
    {
        WriteSettings();
    }
    accepted();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::RestoreDefaults()
{
    ui->comboBoxTemplates->blockSignals(true);
    ui->comboBoxTemplates->setCurrentIndex(0);//A0
    TemplateSelected();
    ui->comboBoxTemplates->blockSignals(false);

    ui->comboBoxPrinter->blockSignals(true);
    InitPrinter();
    ui->comboBoxPrinter->blockSignals(false);

    setLayoutGap(VSettings::getDefLayoutGap());
    SetShift(VSettings::GetDefLayoutShift());
    SetGroup(VSettings::GetDefLayoutGroup());
    SetRotate(VSettings::GetDefLayoutRotate());
    SetIncrease(VSettings::GetDefLayoutRotationIncrease());
    SetFields(GetDefPrinterFields());
    SetIgnoreAllFields(VSettings::GetDefIgnoreAllFields());
    SetMultiplier(VSettings::GetDefMultiplier());

    CorrectMaxFileds();
    IgnoreAllFields(ui->checkBoxIgnoreFileds->isChecked());

    ui->textAsPaths_Checkbox->setChecked(false);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::PrinterMargins()
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        SetFields(GetPrinterFields(QSharedPointer<QPrinter>(new QPrinter(printer))));
    }
    else
    {
        SetFields(QMarginsF());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::CorrectMaxFileds()
{
    const qreal width = ui->doubleSpinBoxPaperWidth->value();
    const qreal height = ui->doubleSpinBoxPaperHeight->value();

    // 80%/2 of paper size for each field
    const qreal widthField = (width*80.0/100.0)/2.0;
    const qreal heightField = (height*80.0/100.0)/2.0;

    ui->leftField_DoubleSpinBox->setMaximum(widthField);
    ui->rightField_DoubleSpinBox->setMaximum(widthField);
    ui->topField_DoubleSpinBox->setMaximum(heightField);
    ui->bottomField_DoubleSpinBox->setMaximum(heightField);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::IgnoreAllFields(int state)
{
    ui->leftField_DoubleSpinBox->setDisabled(state);
    ui->rightField_DoubleSpinBox->setDisabled(state);
    ui->topField_DoubleSpinBox->setDisabled(state);
    ui->bottomField_DoubleSpinBox->setDisabled(state);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::InitPaperUnits()
{
    ui->comboBoxPaperSizeUnit->addItem(tr("Millimeters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));
    ui->comboBoxPaperSizeUnit->addItem(tr("Pixels"), QVariant(UnitsToStr(Unit::Px)));

    // set default unit
    oldPaperUnit = StrToUnits(qApp->Seamly2DSettings()->getUnit());
    const qint32 indexUnit = ui->comboBoxPaperSizeUnit->findData(qApp->Seamly2DSettings()->getUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxPaperSizeUnit->setCurrentIndex(indexUnit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::InitLayoutUnits()
{
    ui->comboBoxLayoutUnit->addItem(tr("Centimeters"), QVariant(UnitsToStr(Unit::Cm)));
    ui->comboBoxLayoutUnit->addItem(tr("Millimeters"), QVariant(UnitsToStr(Unit::Mm)));
    ui->comboBoxLayoutUnit->addItem(tr("Inches"), QVariant(UnitsToStr(Unit::Inch)));

    // set default unit
    oldLayoutUnit = StrToUnits(qApp->Seamly2DSettings()->getUnit());
    const qint32 indexUnit = ui->comboBoxLayoutUnit->findData(qApp->Seamly2DSettings()->getUnit());
    if (indexUnit != -1)
    {
        ui->comboBoxLayoutUnit->setCurrentIndex(indexUnit);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::InitPrinter()
{
    ui->comboBoxPrinter->clear();
    QStringList printerNames;
    printerNames = QPrinterInfo::availablePrinterNames();

    ui->comboBoxPrinter->addItems(printerNames);

    if (ui->comboBoxPrinter->count() == 0)
    {
        ui->comboBoxPrinter->addItem(tr("None", "Printer"));
    }
    else
    {
        QString defPrinterName;
        defPrinterName = QPrinterInfo::defaultPrinterName();

        const int index = ui->comboBoxPrinter->findText(defPrinterName);
        if(index != -1)
        {
            ui->comboBoxPrinter->setCurrentIndex(index);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF LayoutSettingsDialog::Template()
{
    PaperSizeFormat temp;
    temp = static_cast<PaperSizeFormat>(ui->comboBoxTemplates->currentData().toInt());

    const Unit paperUnit = PaperUnit();

    switch (temp)
    {
        case PaperSizeFormat::A0:
        case PaperSizeFormat::A1:
        case PaperSizeFormat::A2:
        case PaperSizeFormat::A3:
        case PaperSizeFormat::A4:
        case PaperSizeFormat::Letter:
        case PaperSizeFormat::Legal:
        case PaperSizeFormat::Tabloid:
        case PaperSizeFormat::AnsiC:
        case PaperSizeFormat::AnsiD:
        case PaperSizeFormat::AnsiE:
            SetAdditionalOptions(false);
            return getTemplateSize(temp, paperUnit);
        case PaperSizeFormat::Roll24in:
        case PaperSizeFormat::Roll30in:
        case PaperSizeFormat::Roll36in:
        case PaperSizeFormat::Roll42in:
        case PaperSizeFormat::Roll44in:
            SetAdditionalOptions(true);
            return getTemplateSize(temp, paperUnit);
        case PaperSizeFormat::Custom:
            return getTemplateSize(temp, paperUnit);
        default:
            break;
    }
    return QSizeF();
}

/**
 * @brief LayoutSettingsDialog::TemplateSize
 * @param tmpl
 * @param unit
 * @return
 */
QSizeF LayoutSettingsDialog::getTemplateSize(const PaperSizeFormat &tmpl, const Unit &unit) const
{
    qreal width = 0;
    qreal height = 0;

    switch (tmpl)
    {
        case PaperSizeFormat::Custom:
            width = ui->doubleSpinBoxPaperWidth->value();
            height = ui->doubleSpinBoxPaperHeight->value();
            return RoundTemplateSize(width, height, unit);
        default:
            return AbstractLayoutDialog::getTemplateSize(tmpl, unit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF LayoutSettingsDialog::MinPrinterFields() const
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        QSharedPointer<QPrinter> pr = QSharedPointer<QPrinter>(new QPrinter(printer));
        return GetMinPrinterFields(pr);
    }
    else
    {
        return QMarginsF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF LayoutSettingsDialog::GetDefPrinterFields() const
{
    QPrinterInfo printer = QPrinterInfo::printerInfo(ui->comboBoxPrinter->currentText());
    if (not printer.isNull())
    {
        return GetPrinterFields(QSharedPointer<QPrinter>(new QPrinter(printer)));
    }
    else
    {
        return QMarginsF();
    }
}

//---------------------------------------------------------------------------------------------------------------------
Unit LayoutSettingsDialog::PaperUnit() const
{
    return StrToUnits(ui->comboBoxPaperSizeUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
Unit LayoutSettingsDialog::LayoutUnit() const
{
    return StrToUnits(ui->comboBoxLayoutUnit->currentData().toString());
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::CorrectPaperDecimals()
{
    switch (oldPaperUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxPaperWidth->setDecimals(2);
            ui->doubleSpinBoxPaperHeight->setDecimals(2);

            ui->leftField_DoubleSpinBox->setDecimals(4);
            ui->rightField_DoubleSpinBox->setDecimals(4);
            ui->topField_DoubleSpinBox->setDecimals(4);
            ui->bottomField_DoubleSpinBox->setDecimals(4);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxPaperWidth->setDecimals(5);
            ui->doubleSpinBoxPaperHeight->setDecimals(5);

            ui->leftField_DoubleSpinBox->setDecimals(5);
            ui->rightField_DoubleSpinBox->setDecimals(5);
            ui->topField_DoubleSpinBox->setDecimals(5);
            ui->bottomField_DoubleSpinBox->setDecimals(5);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::CorrectLayoutDecimals()
{
    switch (oldLayoutUnit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            ui->doubleSpinBoxLayoutWidth->setDecimals(2);
            ui->doubleSpinBoxShift->setDecimals(2);
            break;
        case Unit::Inch:
            ui->doubleSpinBoxLayoutWidth->setDecimals(5);
            ui->doubleSpinBoxShift->setDecimals(5);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::MinimumPaperSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, oldPaperUnit);
    ui->doubleSpinBoxPaperWidth->setMinimum(value);
    ui->doubleSpinBoxPaperHeight->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::MinimumLayoutSize()
{
    const qreal value = UnitConvertor(1, Unit::Px, oldLayoutUnit);
    ui->doubleSpinBoxLayoutWidth->setMinimum(value);
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::ReadSettings()
{
    const VSettings *settings = qApp->Seamly2DSettings();
    setLayoutGap(settings->getLayoutGap());
    SetShift(settings->GetLayoutShift());

    const qreal width = UnitConvertor(settings->GetLayoutPaperWidth(), Unit::Px, LayoutUnit());
    const qreal height = UnitConvertor(settings->GetLayoutPaperHeight(), Unit::Px, LayoutUnit());
    SheetSize(QSizeF(width, height));
    SetGroup(settings->GetLayoutGroup());
    SetRotate(settings->GetLayoutRotate());
    SetIncrease(settings->GetLayoutRotationIncrease());
    SetAutoCrop(settings->GetLayoutAutoCrop());
    SetSaveLength(settings->GetLayoutSaveLength());
    SetUnitePages(settings->GetLayoutUnitePages());
    SetFields(settings->GetFields(GetDefPrinterFields()));
    SetIgnoreAllFields(settings->GetIgnoreAllFields());
    setStripOptimization(settings->useStripOptimization());
    SetMultiplier(settings->GetMultiplier());
    setTextAsPaths(settings->GetTextAsPaths());

    FindTemplate();

    CorrectMaxFileds();
    IgnoreAllFields(ui->checkBoxIgnoreFileds->isChecked());
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::WriteSettings() const
{
    VSettings *settings = qApp->Seamly2DSettings();
    settings->setLayoutGap(getLayoutGap());
    settings->SetLayoutGroup(GetGroup());
    settings->SetLayoutPaperHeight(GetPaperHeight());
    settings->SetLayoutPaperWidth(GetPaperWidth());
    settings->SetLayoutShift(GetShift());
    settings->SetLayoutRotate(GetRotate());
    settings->SetLayoutRotationIncrease(GetIncrease());
    settings->SetLayoutAutoCrop(GetAutoCrop());
    settings->SetLayoutSaveLength(IsSaveLength());
    settings->SetLayoutUnitePages(IsUnitePages());
    settings->SetFields(GetFields());
    settings->SetIgnoreAllFields(IsIgnoreAllFields());
    settings->setStripOptimization(useStripOptimization());
    settings->SetMultiplier(GetMultiplier());
    settings->setTextAsPaths(isTextAsPaths());
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SheetSize(const QSizeF &size)
{
    oldPaperUnit = PaperUnit();
    ui->doubleSpinBoxPaperWidth->setMaximum(FromPixel(QIMAGE_MAX, oldPaperUnit));
    ui->doubleSpinBoxPaperHeight->setMaximum(FromPixel(QIMAGE_MAX, oldPaperUnit));

    ui->doubleSpinBoxPaperWidth->setValue(size.width());
    ui->doubleSpinBoxPaperHeight->setValue(size.height());

    CorrectPaperDecimals();
    PaperSizeChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void LayoutSettingsDialog::SetAdditionalOptions(bool value)
{
    SetAutoCrop(value);
    SetSaveLength(value);
    SetUnitePages(value);
    setStripOptimization(value);
}

/******************************************************************************
 *   @file   grade_breaks_dialog.cpp
 **  @author Claude Code
 **  @date   29 Mar, 2026
 **
 **  @brief  Dialog for editing piecewise (non-linear) grade breaks per
 **          measurement, for size or height dimension.
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2026 Seamly2D project
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

#include "grade_breaks_dialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QHeaderView>

#include "../vpatterndb/variables/measurement_variable.h"
#include "../vpatterndb/variables/grade_break.h"

//---------------------------------------------------------------------------------------------------------------------
GradeBreaksDialog::GradeBreaksDialog(Dimension dim,
                                     const QString &measurementName,
                                     const QStringList &activeValues,
                                     qreal baseValue,
                                     qreal defaultIncrement,
                                     qreal baseMeasurement,
                                     const QVector<GradeBreak> &existingBreaks,
                                     Unit unit,
                                     const QMap<int, QString> &aliases,
                                     QWidget *parent)
    : QDialog(parent)
    , m_dimension(dim)
    , m_activeValues(activeValues)
    , m_baseValue(baseValue)
    , m_defaultIncrement(defaultIncrement)
    , m_baseMeasurement(baseMeasurement)
    , m_unit(unit)
    , m_aliases(aliases)
    , m_spinBoxes()
    , m_previewLabel(nullptr)
{
    setupUI(measurementName, existingBreaks);
}

//---------------------------------------------------------------------------------------------------------------------
GradeBreaksDialog::~GradeBreaksDialog()
{
}

//---------------------------------------------------------------------------------------------------------------------
int GradeBreaksDialog::decimals() const
{
    switch (m_unit)
    {
        case Unit::Mm:
            return 2;
        case Unit::Cm:
            return 1;
        case Unit::Inch:
            return 5;
        default:
            return 1;
    }
}

//---------------------------------------------------------------------------------------------------------------------
qreal GradeBreaksDialog::singleStep() const
{
    switch (m_unit)
    {
        case Unit::Mm:
            return 0.01;
        case Unit::Cm:
            return 0.1;
        case Unit::Inch:
            return 0.00001;
        default:
            return 0.1;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GradeBreaksDialog::setupUI(const QString &measurementName,
                                const QVector<GradeBreak> &existingBreaks)
{
    const QString dimLabel = (m_dimension == Dimension::Size) ? tr("Size") : tr("Height");
    setWindowTitle(tr("Grade Rules — %1 — %2").arg(measurementName, dimLabel));
    setMinimumWidth(380);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Header
    QLabel *header = new QLabel(tr("Set the increment per step for each %1. "
                                   "Values must be non-decreasing.").arg(dimLabel.toLower()));
    header->setWordWrap(true);
    mainLayout->addWidget(header);

    // Scroll area for the table
    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    QWidget *scrollWidget = new QWidget;
    QGridLayout *grid = new QGridLayout(scrollWidget);

    // Column headers
    grid->addWidget(new QLabel(tr("<b>%1</b>").arg(dimLabel)), 0, 0);
    grid->addWidget(new QLabel(tr("<b>Increment per Step</b>")), 0, 1);

    const QString unitStr = UnitsToStr(m_unit);
    const int dec = decimals();
    const qreal step = singleStep();

    for (int row = 0; row < m_activeValues.size(); ++row)
    {
        const qreal val = m_activeValues[row].toDouble();
        const bool isBase = qFuzzyCompare(val, m_baseValue);

        // Size/height label
        QString labelText = MeasurementDoc::sizeDisplayAlias(static_cast<int>(val), m_aliases);
        if (isBase)
        {
            labelText += tr(" (base)");
        }
        QLabel *label = new QLabel(labelText);
        if (isBase)
        {
            QFont f = label->font();
            f.setBold(true);
            label->setFont(f);
        }
        grid->addWidget(label, row + 1, 0);

        // Spin box for increment
        QDoubleSpinBox *spin = new QDoubleSpinBox;
        spin->setDecimals(dec);
        spin->setSingleStep(step);
        spin->setMinimum(0.0);
        spin->setMaximum(10000.0);
        spin->setSuffix(QString(" %1").arg(unitStr));

        // Find applicable increment from existing breaks
        qreal incrementForRow = m_defaultIncrement;
        for (int b = existingBreaks.size() - 1; b >= 0; --b)
        {
            if (val >= existingBreaks[b].threshold - 0.001)
            {
                incrementForRow = existingBreaks[b].increment;
                break;
            }
        }
        spin->setValue(incrementForRow);

        if (isBase)
        {
            spin->setEnabled(false);
            spin->setToolTip(tr("Base size — increment defines the default rate"));
        }

        connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                this, [this, row]() { incrementChanged(row); });

        grid->addWidget(spin, row + 1, 1);
        m_spinBoxes.append(spin);
    }

    scrollWidget->setLayout(grid);
    scrollArea->setWidget(scrollWidget);
    mainLayout->addWidget(scrollArea, 1);

    // Preview
    m_previewLabel = new QLabel;
    updatePreview();
    mainLayout->addWidget(m_previewLabel);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    QPushButton *resetButton = new QPushButton(tr("Reset All"));
    connect(resetButton, &QPushButton::clicked, this, &GradeBreaksDialog::resetAll);
    buttonLayout->addWidget(resetButton);

    buttonLayout->addStretch();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    buttonLayout->addWidget(buttonBox);

    mainLayout->addLayout(buttonLayout);
}

//---------------------------------------------------------------------------------------------------------------------
void GradeBreaksDialog::incrementChanged(int row)
{
    Q_UNUSED(row)
    updatePreview();
}

//---------------------------------------------------------------------------------------------------------------------
void GradeBreaksDialog::resetAll()
{
    for (QDoubleSpinBox *spin : m_spinBoxes)
    {
        if (spin->isEnabled())
        {
            spin->blockSignals(true);
            spin->setValue(m_defaultIncrement);
            spin->blockSignals(false);
        }
    }
    updatePreview();
}

//---------------------------------------------------------------------------------------------------------------------
void GradeBreaksDialog::updatePreview()
{
    if (m_activeValues.isEmpty())
    {
        return;
    }

    // Show preview for the last active value
    const qreal lastVal = m_activeValues.last().toDouble();
    const qreal previewResult = calcPreviewValue(lastVal);
    const QString unitStr = UnitsToStr(m_unit);

    m_previewLabel->setText(tr("Preview: %1 %2 = %3 %4")
                            .arg((m_dimension == Dimension::Size) ? tr("size") : tr("height"))
                            .arg(m_activeValues.last())
                            .arg(QLocale().toString(previewResult, 'f', decimals()))
                            .arg(unitStr));
}

//---------------------------------------------------------------------------------------------------------------------
qreal GradeBreaksDialog::calcPreviewValue(qreal targetValue) const
{
    // Build temporary breaks vector from current spinbox values
    QVector<GradeBreak> breaks = gradeBreaks();

    const qreal stepSize = (m_dimension == Dimension::Size)
                           ? UnitConvertor(2.0, Unit::Cm, m_unit)
                           : UnitConvertor(6.0, Unit::Cm, m_unit);

    const qreal offset = MeasurementVariable::calcPiecewiseOffset(
        targetValue, m_baseValue, stepSize, m_defaultIncrement, breaks);

    return m_baseMeasurement + offset;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<GradeBreak> GradeBreaksDialog::gradeBreaks() const
{
    QVector<GradeBreak> breaks;
    qreal prevIncrement = m_defaultIncrement;

    for (int i = 0; i < m_spinBoxes.size(); ++i)
    {
        const qreal increment = m_spinBoxes[i]->value();
        const qreal threshold = m_activeValues[i].toDouble();

        // Only store a break where the increment differs from the previous
        if (!qFuzzyCompare(increment, prevIncrement))
        {
            breaks.append(GradeBreak(threshold, increment));
            prevIncrement = increment;
        }
    }

    return breaks;
}

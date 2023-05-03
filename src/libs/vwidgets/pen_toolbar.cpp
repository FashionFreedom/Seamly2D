/******************************************************************************
 *   @file   pen_toolbar.cpp
 **  @author DS Caskey
 **  @date   Jan 16, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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


#include "../vmisc/logging.h"
#include "../vmisc/vabstractapplication.h"

#include "pen_toolbar.h"

Q_LOGGING_CATEGORY(penToolBar, "pentoolbar")

/**
 * Constructor.
 */
PenToolBar::PenToolBar( const QString &title, QWidget *parent )
		: QToolBar(title, parent)
		, currentPen(Pen{})
		, colorBox(new ColorComboBox{40, 14, this, "colorbox"})
		, lineTypeBox(new LineTypeComboBox{40, 14, this, "lineTypebox"})
        , lineWeightBox(new LineWeightComboBox{40, 14, this, "lineWeightbox"})
{
    currentPen.color      = qApp->Settings()->getDefaultLineColor();
    currentPen.lineType   = qApp->Settings()->getDefaultLineType();
    currentPen.lineWeight = qApp->Settings()->getDefaultLineWeight();

    int index = colorBox->findData(currentPen.color);
    if (index != -1)
    {
        colorBox->setCurrentIndex(index);
    }
    index = lineTypeBox->findData(currentPen.lineType);
    if (index != -1)
    {
        lineTypeBox->setCurrentIndex(index);
    }
    index = lineWeightBox->findData(currentPen.lineWeight);
    if (index != -1)
    {
        lineWeightBox->setCurrentIndex(index);
    }

  	addWidget(colorBox);
    colorBox->setToolTip(tr("Current line color"));
    connect(colorBox, &ColorComboBox::colorChangedSignal, this, &PenToolBar::colorChanged);

    addWidget(lineTypeBox);
    lineTypeBox->setToolTip(tr("Current line type"));
    connect(lineTypeBox, &LineTypeComboBox::lineTypeChanged,  this, &PenToolBar::lineTypeChanged);

	addWidget(lineWeightBox);
    lineWeightBox->setToolTip(tr("Current line weight"));
    connect(lineWeightBox, &LineWeightComboBox::lineWeightChanged, this, &PenToolBar::lineWeightChanged);

    QToolButton *resetButton = new QToolButton;
    resetButton->setIcon(QIcon(":/icons/win.icon.theme/24x24/actions/edit-undo.png"));
    resetButton->setToolTip ("Reset current pen to defaults");
    addWidget(resetButton);
    connect(resetButton, &QToolButton::clicked, this, &PenToolBar::penReset);

    QToolButton *savePresetButton = new QToolButton;
    savePresetButton->setIcon(QIcon(":/icons/win.icon.theme/24x24/actions/document-save-as.png"));
    savePresetButton->setToolTip ("Save current pen preset");
    addWidget(savePresetButton);
    connect(savePresetButton, &QToolButton::clicked, this, &PenToolBar::savePreset);
}


/**
 * Destructor
 */
PenToolBar::~PenToolBar() = default;

Pen PenToolBar::getPen() const
{
	return currentPen;
}

/**
 * Called when the linetype was changed by the user.
 */
void PenToolBar::lineTypeChanged(const QString &type)
{
    qCDebug(penToolBar, "PenToolBar::lineTypeChanged - Pen type changed\n");
	currentPen.lineType = type;

	emit penChanged(currentPen);
}

/**
 * Called when the color was changed by the user.
 */
void PenToolBar::colorChanged(const QString &color)
{
	qCDebug(penToolBar, "PenToolBar::colorChanged - Pen color changed\n");
	currentPen.color = color;

	emit penChanged(currentPen);
}

/**
 * Called when the width was changed by the user.
 */
void PenToolBar::lineWeightChanged(const qreal &weight)
{
    qCDebug(penToolBar, "PenToolBar::lineWeightChanged - Pen width changed\n");
	currentPen.lineWeight = weight;

	emit penChanged(currentPen);
}

/**
 * @brief penReset resets the current pen to the preferred defaults.
 */
void PenToolBar::penReset()
{
    qCDebug(penToolBar, "Pen Reset");
    blockSignals(true);

    int index = colorBox->findData(qApp->Settings()->getDefaultLineColor());
	if (index != -1)
	{
		colorBox->setCurrentIndex(index);
        currentPen.color = colorBox->getColor();
	}
	index = lineTypeBox->findData(qApp->Settings()->getDefaultLineType());
	if (index != -1)
	{
		lineTypeBox->setCurrentIndex(index);
        currentPen.lineType = lineTypeBox->getLineType();
	}
    index = lineWeightBox->findData(qApp->Settings()->getDefaultLineWeight());
    if (index != -1)
    {
        lineWeightBox->setCurrentIndex(index);
        currentPen.lineWeight = lineWeightBox->getLineWeight();
    }

    blockSignals(false);
	emit penChanged(currentPen);
}

/**
 * savePreset() Save the current pen to the preferences.
 */
void PenToolBar::savePreset()
{
    qApp->Settings()->setDefaultLineColor(currentPen.color);
    qApp->Settings()->setDefaultLineType(currentPen.lineType);
    qApp->Settings()->setDefaultLineWeight(currentPen.lineWeight);
}

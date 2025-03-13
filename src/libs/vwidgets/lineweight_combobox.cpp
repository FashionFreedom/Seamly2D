//-----------------------------------------------------------------------------
//  @file   lineweight_combobox.cpp
//  @author Douglas S Caskey
//  @date   14 Mar, 2025
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

#include "lineweight_combobox.h"

#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

#include <QAbstractItemView>
#include <QPen>
#include <Qt>
#include <QPainter>
#include <QPixmap>

/// @brief LineWeightComboBox constructor
///
/// This constructor provides a name, and parent widget.
///
/// @param name of combobox.
/// @param parent parent widget of combobox, default is null if not provided.
LineWeightComboBox::LineWeightComboBox(QWidget *parent, const char *name)
		: QComboBox(parent)
      	, m_currentWeight(0.35)
		, m_iconWidth(40)
		, m_iconHeight(12)
{
		 setObjectName(name);
		 setEditable ( false );
		 init();
}

/// @brief LineWeightComboBox constructor
///
/// This constructor provides a width and height for the icon, name, and parent widget.
///
/// @param width width of icon.
/// @param height width of icon.
/// @param name of combobox.
/// @param parent parent widget of combobox, default is null if not provided.
LineWeightComboBox::LineWeightComboBox(int width, int height, QWidget *parent, const char *name)
		: QComboBox(parent)
		, m_currentWeight(0.35)
		, m_iconWidth(width)
		, m_iconHeight(height)
{
		 setObjectName(name);
		 setEditable ( false );
		 init();
}

/// Destructor
LineWeightComboBox::~LineWeightComboBox() {}


/// @brief init initalise combo box
///
/// This method intializes the combobox items list.
///
/// @Details
///  - Initialisation called from constructor or manually, but only once.
///  - Sets size of item icon and creates an icon for a given lineweight.
///  - Adds the full item list of all pen weights, unless the showOnlyIso option is set in prefs
///    which then omly adds the ISO pen weights.
void LineWeightComboBox::init()
{
    // Block signals while we initialize combobox.
    this->blockSignals(true);

#if defined(Q_OS_MAC)
    // Mac
    setIconSize(QSize(m_iconWidth-= 2, m_iconHeight-= 2)); // On Mac pixmap should be little bit smaller.
#else
    // Windows
    setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif

    this->view()->setTextElideMode(Qt::ElideNone);

    if (qApp->Settings()->showOnlyIso())
    {
        addItem(createIcon(2.00), "0.13mm (ISO)", 0.13);
        addItem(createIcon(2.00), "0.18mm (ISO)", 0.18);
        addItem(createIcon(3.00), "0.25mm (ISO)", 0.25);
        addItem(createIcon(3.00), "0.35mm (ISO)", 0.35);
        addItem(createIcon(4.00), "0.50mm (ISO)", 0.50);
        addItem(createIcon(4.00), "0.70mm (ISO)", 0.70);
        addItem(createIcon(6.00), "1.00mm (ISO)", 1.00);
        addItem(createIcon(7.00), "1.40mm (ISO)", 1.40);
        addItem(createIcon(8.00), "2.00mm (ISO)", 2.00);
        setMaxVisibleItems(9);
    }
    else
    {
        addItem(createIcon(0.00), "0.00mm",       0.00);
        addItem(createIcon(1.00), "0.05mm",       0.05);
        addItem(createIcon(1.00), "0.09mm",       0.09);
        addItem(createIcon(2.00), "0.13mm (ISO)", 0.13);
        addItem(createIcon(2.00), "0.15mm",       0.15);
        addItem(createIcon(2.00), "0.18mm (ISO)", 0.18);
        addItem(createIcon(3.00), "0.20mm",       0.20);
        addItem(createIcon(3.00), "0.25mm (ISO)", 0.25);
        addItem(createIcon(3.00), "0.30mm",       0.30);
        addItem(createIcon(3.00), "0.35mm (ISO)", 0.35);
        addItem(createIcon(3.00), "0.40mm",       0.40);
        addItem(createIcon(4.00), "0.50mm (ISO)", 0.50);
        addItem(createIcon(4.00), "0.53mm",       0.53);
        addItem(createIcon(4.00), "0.60mm",       0.60);
        addItem(createIcon(4.00), "0.70mm (ISO)", 0.70);
        addItem(createIcon(5.00), "0.80mm",       0.80);
        addItem(createIcon(5.00), "0.90mm",       0.90);
        addItem(createIcon(6.00), "1.00mm (ISO)", 1.00);
        addItem(createIcon(6.00), "1.06mm",       1.06);
        addItem(createIcon(6.00), "1.20mm",       1.20);
        addItem(createIcon(7.00), "1.40mm (ISO)", 1.40);
        addItem(createIcon(7.00), "1.58mm",       1.58);
        addItem(createIcon(8.00), "2.00mm (ISO)", 2.00);
        addItem(createIcon(8.00), "2.11mm",       2.11);
        addItem(createIcon(9.00), "3.00mm",       3.00);
        setMaxVisibleItems(25);
    }

    this->blockSignals(false);
		connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineWeightComboBox::updateLineWeight);

    setCurrentIndex(0);
    updateLineWeight(currentIndex());
}

/// @brief getLineWeight get the current lineweight item.
///
/// This method gets the currently selected weight.
///
/// @return current line weight.
qreal LineWeightComboBox::getLineWeight() const
{
    return m_currentWeight;
}

/// @brief setLineWeight set the current lineweight item.
///
/// This method sets the currently selected weight item to the given weight.
///
/// @param weight line weight to set as current item.
void LineWeightComboBox::setLineWeight(const qreal &weight)
{
    m_currentWeight = weight;

    int index = findData(weight);

    if (index = -1)
    {
        index = 6;
    }

    setCurrentIndex(index);

    if (currentIndex()!= count() -1 )
    {
        updateLineWeight(currentIndex());
    }
}

/// @brief setLineWeight set current lineweight item by index.
///
/// This method is called when the width has changed, and sets the current width to the value chosen
/// or even offers a dialog to the user that allows him/ her to choose an individual width.
///
/// @param index index to set the current item to.
///
/// @Details
///  - emits lineWeightChanged signal with the curren line weight.
void LineWeightComboBox::updateLineWeight(int index)
{
    QVariant weight = itemData(index);
    if(weight != QVariant::Invalid )
    {
       m_currentWeight = QVariant(weight).toReal();
    }

    emit lineWeightChanged(m_currentWeight);
}

QIcon LineWeightComboBox::createIcon(const qreal &width)
{
    QPixmap pixmap(m_iconWidth, m_iconHeight);
    pixmap.fill(Qt::black);

    QPen pen(Qt::black, width, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);

    QPainter painter(&pixmap);
    painter.fillRect(QRect(1, 1, m_iconWidth-2, m_iconHeight-2), QColor(Qt::white));
    painter.setPen(pen);
    painter.drawLine(0, m_iconHeight/2, m_iconWidth, m_iconHeight/2);

    return QIcon(pixmap);
}

//---------------------------------------------------------------------------------------------------------------------
///  @file   color_comboox.cpp
///  @author Douglas S Caskey
///  @date   11 Apr, 2025
///
/// @copyright
/// Copyright (C) 2017 - 2026 Seamly, LLC
/// https://github.com/fashionfreedom/seamly2d
///
/// @brief
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
///
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#include <QColor>
#include <QPixmap>
#include <QMap>
#include <QAbstractItemView>
#include <QComboBox>

#include "../vtools/tools/vabstracttool.h"
#include "../vmisc/logging.h"
#include "../ifc/ifcdef.h"

#include "color_combobox.h"

//---------------------------------------------------------------------------------------------------------------------
/// @brief ColorComboBox Color comnbo box.
///
/// This is the constructor that provides just a name for the box.
///
/// @param parent parent widget.
/// @param name name of combobox.
//---------------------------------------------------------------------------------------------------------------------
ColorComboBox::ColorComboBox(QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_current_color("black")
    , m_icon_width(40)
    , m_icon_height(14)
{
    setObjectName(name);
    setEditable (false);
    init();
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ColorComboBox Color comnbo box.
///
/// This is the constructor that provides width and height for icon, and a name.
///
/// @param width width of icon.
/// @param height height of icon.
/// @param parent parent widget.
/// @param name name of combobox.
//---------------------------------------------------------------------------------------------------------------------
ColorComboBox::ColorComboBox(int width, int height, QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_current_color("black")
    , m_icon_width(width)
    , m_icon_height(height)
{
    setObjectName(name);
    setEditable (false);
    init();
}

//---------------------------------------------------------------------------------------------------------------------*
/// @brief ~ColorComboBox Color comnbo box.
///
/// This is the destructor.
//---------------------------------------------------------------------------------------------------------------------
ColorComboBox::~ColorComboBox(){}

//---------------------------------------------------------------------------------------------------------------------
/// @brief init Initialisation called from constructor or manually but only once.
//---------------------------------------------------------------------------------------------------------------------
void ColorComboBox::init()
{
    setItems(VAbstractTool::ColorsList());
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColorComboBox::colorChanged);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief setItems Sets the items shown in the combobox and sets index to the 1st color.
///
/// This method sets the item list for the combobox.
///
/// @param map QMap of the text & data pair for combobox.
//---------------------------------------------------------------------------------------------------------------------
void ColorComboBox::setItems(QMap<QString, QString> map)
{
    this->blockSignals(true);

    clear();

#if defined(Q_OS_MAC)
    // Mac pixmap should be little bit smaller
    setIconSize(QSize(m_icon_width -= 2 , m_icon_height -= 2));
#else
    // Windows
    setIconSize(QSize(m_icon_width, m_icon_height));
#endif

    this->view()->setTextElideMode(Qt::ElideNone);
    //map.remove(ColorByGroup);
    QMap<QString, QString>::const_iterator i = map.constBegin();
    while (i != map.constEnd())
    {
        QPixmap pixmap = VAbstractTool::createColorIcon(m_icon_width, m_icon_height, i.key());
        addItem(QIcon(pixmap), i.value(), QVariant(i.key()));
        ++i;
    }

    setMaxVisibleItems(map.size());
    this->model()->sort(1, Qt::AscendingOrder);
    setCurrentIndex(0);
    colorChanged(currentIndex());

    this->blockSignals(false);
}
//---------------------------------------------------------------------------------------------------------------------
///@brief setColor Set boc color
///
/// This method sets the color shown in the combobox to the given color.
///
/// @param color color to set item index to.
//---------------------------------------------------------------------------------------------------------------------
void ColorComboBox::setColor(const QString &color)
{
    m_current_color = color;

    setCurrentIndex(findData(color));

    if (currentIndex()!= count() -1 )
    {
        colorChanged(currentIndex());
    }
}

//---------------------------------------------------------------------------------------------------------------------
///@brief colorChanged Handle color index chamge
///
/// This Called when the color has changed. This method sets the current color to the
/// value chosen.
///
/// @param index Item index to set box to.
//---------------------------------------------------------------------------------------------------------------------
void ColorComboBox::colorChanged(int index)
{
    QVariant color = itemData(index);
    if(color != QVariant::Invalid )
    {
       m_current_color = QVariant(color).toString();
    }

    emit colorChangedSignal(m_current_color);
}

QString ColorComboBox::getColor() const
{
    return m_current_color;
}

int ColorComboBox::getIconWidth()
{
    return m_icon_width;
}

int ColorComboBox::getIconHeight()
{
    return m_icon_height;
}

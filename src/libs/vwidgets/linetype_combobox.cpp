/******************************************************************************
 *   @file   linetypecombobox.cpp
 **  @author DS Caskey
 **  @date   April 6, 2021
 **
 **  @brief
 **  @copyright
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
 *****************************************************************************/

#include "linetype_combobox.h"

#include "../ifc/ifcdef.h"

#include <QStandardItemModel>
#include <QAbstractItemView>
#include <QPixmap>
#include <QComboBox>
#include <QPainter>

/*
 * Default Constructor.
 */
LineTypeComboBox::LineTypeComboBox(QWidget *parent)
    : QComboBox(parent)
    , m_currentLineType(LineTypeSolidLine)
    , m_iconWidth(40)
    , m_iconHeight(14)
{
    setEditable (false);
    init();
}

/**
 * Constructor that provides a width & height for the icon, and a name for the combobox.
 */
LineTypeComboBox::LineTypeComboBox(int  width, int height, QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_currentLineType(LineTypeSolidLine)
    , m_iconWidth(width)
    , m_iconHeight(height)
{
    setObjectName(name);
    init();
}

/**
 * Destructor
 */
LineTypeComboBox::~LineTypeComboBox() {}


/**
 * Initialisation called from constructor or manually but only once.
 */
void LineTypeComboBox::init()
{
    blockSignals(true);

#if defined(Q_OS_MAC)
    setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2)); // On Mac pixmap should be little bit smaller.
#else // Windows
    setIconSize(QSize(m_iconWidth,m_iconHeight));
#endif

    view()->setTextElideMode(Qt::ElideNone);
    setSizeAdjustPolicy(QComboBox::AdjustToContents);

    addItem(createIcon(LineTypeNone),           tr("No Pen"),       LineTypeNone);
    addItem(createIcon(LineTypeSolidLine),      tr("Solidline"),    LineTypeSolidLine);
    addItem(createIcon(LineTypeDashLine),       tr("Dash"),         LineTypeDashLine);
    addItem(createIcon(LineTypeDotLine),        tr("Dot"),          LineTypeDotLine);
    addItem(createIcon(LineTypeDashDotLine),    tr("Dash Dot"),     LineTypeDashDotLine);
    addItem(createIcon(LineTypeDashDotDotLine), tr("Dash Dot Dot"), LineTypeDashDotDotLine);

    const int index = findData(QVariant(LineTypeSolidLine));
    if (index != -1)
    {
        setCurrentIndex(index);
    }
    else
    {
        setCurrentIndex(1);
    }

    blockSignals(false);
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LineTypeComboBox::updateLineType);


    updateLineType(currentIndex());
}

/**
 * Sets the currently selected linetype item to the given linetype.
 */
void LineTypeComboBox::setLineType(const QString &type)
{
    m_currentLineType = type;

    setCurrentIndex(findData(type));

    if (currentIndex()!= count() -1 )
    {
        updateLineType(currentIndex());
    }
}


/* Called when the linetype has changed. This method sets the current linetype
 * to the value chosen or even offers a dialog to the user that allows one to
 * choose an individual linetype.
 */
void LineTypeComboBox::updateLineType(int index)
{
    QVariant type = itemData(index);
    if(type != QVariant::Invalid )
    {
       m_currentLineType = QVariant(type).toString();
    }

    emit lineTypeChanged(m_currentLineType);
}

QIcon LineTypeComboBox::createIcon(const QString &type)
{
    const Qt::PenStyle style = lineTypeToPenStyle(type);
    QPixmap pixmap(m_iconWidth, m_iconHeight);
    pixmap.fill(Qt::black);

    QBrush brush(Qt::black);
    QPen pen(brush, 2.5, style);

    QPainter painter(&pixmap);
    painter.fillRect(QRect(1, 1, m_iconWidth-2, m_iconHeight-2), QColor(Qt::white));
    if (style != Qt::NoPen)
    {
        painter.setPen(pen);
        painter.drawLine(0, m_iconHeight/2, m_iconWidth, m_iconHeight/2);
    }

    return QIcon(pixmap);
}

/***************************************************************************
 **  @file   fill_combobox.cpp
 **  @author Douglas S Caskey
 **  @date   Dec 28, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include "fill_combobox.h"

#include <QAbstractItemView>
#include <Qt>
#include <QPainter>
#include <QPixmap>

#include "../vtools/tools/vabstracttool.h"
#include "../vmisc/logging.h"

Q_LOGGING_CATEGORY(fillComboBox, "fill_combobox")

/*
 * Default Constructor. You must call init manually if you choose
 * to use this constructor.
 */
PieceFillComboBox::PieceFillComboBox(QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_currentFill(FillNone)
    , m_iconWidth(40)
    , m_iconHeight(14)
{
    setObjectName(name);
    setEditable (false);
    init();
}

/*
 * Constructor that calls init and provides a fully functional
 * comboBox for choosing fills.
 */
PieceFillComboBox::PieceFillComboBox(int width, int height, QWidget *parent, const char *name)
    : QComboBox(parent)
    , m_currentFill(FillNone)
    , m_iconWidth(width)
    , m_iconHeight(height)
{
    setObjectName(name);
    setEditable (false);
    init();
}

/**
 * Destructor
 */
PieceFillComboBox::~PieceFillComboBox(){}

/*
 * Initialisation called from constructor or manually but only once.
 */
void PieceFillComboBox::init()
{
    qCDebug(fillComboBox, "PieceFillComboBox::init");
    this->blockSignals(true);

#if defined(Q_OS_MAC)
    // Mac pixmap should be little bit smaller
    setIconSize(QSize(m_iconWidth-= 2 ,m_iconHeight-= 2));
#else
    // Windows
    setIconSize(QSize(m_iconWidth, m_iconHeight));
#endif

    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::NoBrush)),          tr("No Fill"),          FillNone);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::SolidPattern)),     tr("Solid"),            FillSolid);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense1Pattern)),    tr("Density 1"),        FillDense1);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense2Pattern)),    tr("Density 2"),        FillDense2);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense3Pattern)),    tr("Density 3"),        FillDense3);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense4Pattern)),    tr("Density 4"),        FillDense4);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense5Pattern)),    tr("Density 5"),        FillDense5);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense6Pattern)),    tr("Density 6"),        FillDense6);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::Dense7Pattern)),    tr("Density 7"),        FillDense7);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::HorPattern)),       tr("Horizontal Line"),  FillHorizLines);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::VerPattern)),       tr("Vertical Line"),    FillVertLines);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::CrossPattern)),     tr("Cross"),            FillCross);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::BDiagPattern)),     tr("Backward Diagonal"),FillBackwardDiagonal);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::FDiagPattern)),     tr("Forward Diagonal"), FillForwardDiagonal);
    addItem(QIcon(createFillIcon(m_iconWidth, m_iconHeight, Qt::DiagCrossPattern)), tr("Diagonal Cross"),   FilldDiagonalCross);

    setMaxVisibleItems(15);
    this->model()->sort(1, Qt::AscendingOrder);
    setCurrentIndex(0);

    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PieceFillComboBox::fillChanged);

    fillChanged(currentIndex());
    this->blockSignals(false);
}

/*
 * Sets the fill shown in the combobox to the given fill.
 */
void PieceFillComboBox::setFill(const QString &fill)
{
    m_currentFill = fill;

    setCurrentIndex(findData(fill));

    if (currentIndex()!= count() -1 )
    {
        fillChanged(currentIndex());
    }
}

QString PieceFillComboBox::getFill() const
{
    return m_currentFill;
}

//* generate icon from fill style, then add to fill box
QPixmap PieceFillComboBox::createFillIcon(const int w, const int h, Qt::BrushStyle style)
{
    QPixmap pixmap(w, h);
    pixmap.fill(QColor(Qt::black));
    QPainter painter(&pixmap);
    painter.fillRect(1, 1, w-2, h-2, QColor(Qt::white));
    painter.fillRect(1, 1, w-2, h-2, style);

    return pixmap;
}

/*
 * Called when the fill has changed. This method sets the current fill to the
 * value chosen or even offers a dialog to the user that allows one to
 * choose an individual fill.
 */
void PieceFillComboBox::fillChanged(int index)
{
    QVariant fill = itemData(index);
    if(fill != QVariant::Invalid )
    {
       m_currentFill = QVariant(fill).toString();
    }

    emit fillChangedSignal(m_currentFill);
}

int PieceFillComboBox::getIconWidth()
{
    return m_iconWidth;
}

int PieceFillComboBox::getIconHeight()
{
    return m_iconHeight;
}

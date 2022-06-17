/******************************************************************************
 **  @file   image_toolbar.cpp
 **  @author DS Caskey
 **  @date   May 21, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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
 *****************************************************************************/

#include "image_toolbar.h"
#include "ui_image_toolbar.h"

#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"

#include <QtDebug>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QWidget>

/**
 * Constructor.
 */
ImageToolbarWidget::ImageToolbarWidget(QWidget *parent)
		: QWidget(parent)
        , ui(new Ui::ImageToolbarWidget)
        , m_item(nullptr)
        , m_image()
        , m_pixmapWidth()
        , m_pixmapHeight()
        , m_maxScale(160.0)
{
    ui->setupUi(this);

    updateUnits();

    connect(ui->importImage_ToolButton,  &QToolButton::clicked, this, &ImageToolbarWidget::addImage);
    connect(ui->deleteImage_ToolButton,  &QToolButton::clicked, this, &ImageToolbarWidget::deleteImage);
    connect(ui->locked_ToolButton,       &QToolButton::clicked, this, &ImageToolbarWidget::lockChanged);
    connect(ui->setAlignment_ToolButton, &QToolButton::clicked, this, &ImageToolbarWidget::alignmentChanged);

    connect(ui->xPosition_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
     this, &ImageToolbarWidget::xPosChanged);
     connect(ui->yPosition_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
     this, &ImageToolbarWidget::yPosChanged);
     connect(ui->width_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this, &ImageToolbarWidget::widthChanged);
     connect(ui->height_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
     this, &ImageToolbarWidget::heightChanged);
     connect(ui->xScale_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
     this, &ImageToolbarWidget::xScaleChanged);
     connect(ui->yScale_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
     this, &ImageToolbarWidget::yScaleChanged);

    connect(ui->aspectLocked_ToolButton, &QToolButton::clicked, this, &ImageToolbarWidget::lockAspectChanged);
    connect(ui->units_ToolButton,        &QToolButton::clicked, this, &ImageToolbarWidget::unitsChanged);
    connect(ui->rotation_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
    this, &ImageToolbarWidget::rotationChanged);
    connect(ui->visible_ToolButton,      &QToolButton::clicked, this, &ImageToolbarWidget::visibilityChanged);



    connect(ui->opacity_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageToolbarWidget::opacityChanged);
}

/**
 * Destructor
 */
ImageToolbarWidget::~ImageToolbarWidget(){}

void ImageToolbarWidget::setImage(DraftImage image)
{
    m_image        = image;
    m_pixmapWidth  = m_image.pixmap.width();
    m_pixmapHeight = m_image.pixmap.height();

    updateImage();
}

void ImageToolbarWidget::updateImage()
{
    qDebug("Id = %d", m_image.id);
    qDebug("Name = %s", qUtf8Printable(m_image.name));
    qDebug("Filename = %s", qUtf8Printable(m_image.filename));
    qDebug("lock Image = %s", m_image.locked ? "True" : "False");
    qDebug("Anchor = %d", static_cast<int>(m_image.anchor));
    qDebug("Xpos = %f", m_image.xPos);
    qDebug("YPos = %f", m_image.yPos);
    qDebug("Width = %f", m_image.width);
    qDebug("Height = %f", m_image.height);
    qDebug("XScale = %f", m_image.xScale);
    qDebug("YScale = %f", m_image.yScale);
    qDebug("lock Image Aspect = %s",  m_image.aspectLocked ? "True" : "False");
    qDebug("Units = %d", static_cast<int>(m_image.units));
    qDebug("Rotation = %f", m_image.rotation);
    qDebug("Visible = %s",  m_image.visible ? "True" : "False");
    qDebug("Opacity = %f", m_image.opacity);
    qDebug("Order = %d\n", static_cast<int>(m_image.order));

    ui->locked_ToolButton->setChecked(m_image.locked);
    if (m_image.units == Unit::Px)
    {
        ui->xPosition_DoubleSpinBox->setValue(m_image.xPos);
        ui->yPosition_DoubleSpinBox->setValue(m_image.yPos);
        ui->width_DoubleSpinBox->setValue(m_image.width);
        ui->height_DoubleSpinBox->setValue(m_image.height);
    }
    else
    {
        ui->xPosition_DoubleSpinBox->setValue(qApp->fromPixel(m_image.xPos));
        ui->yPosition_DoubleSpinBox->setValue(qApp->fromPixel(m_image.yPos));
        ui->width_DoubleSpinBox->setValue(qApp->fromPixel(m_image.width));
        ui->height_DoubleSpinBox->setValue(qApp->fromPixel(m_image.height));
    }
    ui->xScale_DoubleSpinBox->setValue(m_image.xScale);
    ui->yScale_DoubleSpinBox->setValue(m_image.yScale);
    ui->aspectLocked_ToolButton->setChecked(m_image.aspectLocked);
    ui->units_ToolButton->setChecked(m_image.units == Unit::Px ? true : false);
    ui->rotation_DoubleSpinBox->setValue(m_image.rotation);
    ui->visible_ToolButton->setChecked(m_image.visible);
    ui->opacity_DoubleSpinBox->setValue(m_image.opacity);

    enableWidgets();
}

void ImageToolbarWidget::setLocked(const bool &checked)
{
    m_image.locked = checked;
    ui->locked_ToolButton->setChecked(m_image.locked);
}

bool ImageToolbarWidget::isLocked() const
{
    return m_image.locked;
}

void ImageToolbarWidget::setXPos(const qreal &pos)
{
    ui->xPosition_DoubleSpinBox->blockSignals(true);
    m_image.xPos = pos;
    ui->xPosition_DoubleSpinBox->setValue(m_image.xPos);
    ui->xPosition_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getXPos() const
{
    return m_image.xPos;
}

void ImageToolbarWidget::setYPos(const qreal &pos)
{
    ui->yPosition_DoubleSpinBox->blockSignals(true);
    m_image.yPos = pos;
    ui->yPosition_DoubleSpinBox->setValue(m_image.yPos);
    ui->yPosition_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getYPos() const
{
    return m_image.yPos;
}

void ImageToolbarWidget::setWidth(const qreal &value)
{
    ui->width_DoubleSpinBox->blockSignals(true);
    m_image.width = value;
    ui->width_DoubleSpinBox->setValue(m_image.width);
    ui->width_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::geWidth() const
{
    return m_image.width;
}

void ImageToolbarWidget::setHeight(const qreal &value)
{
    ui->height_DoubleSpinBox->blockSignals(true);
    m_image.height = value;
    ui->height_DoubleSpinBox->setValue(m_image.height);
    ui->height_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getHeight() const
{
    return m_image.height;
}

void ImageToolbarWidget::setXScale(const qreal &scale)
{
    ui->xScale_DoubleSpinBox->blockSignals(true);
    m_image.xScale = scale;
    ui->xScale_DoubleSpinBox->setValue(m_image.xScale);
    ui->xScale_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getXScale() const
{
    return m_image.xScale;
}

void ImageToolbarWidget::setYScale(const qreal &scale)
{
    ui->yScale_DoubleSpinBox->blockSignals(true);
    m_image.yScale = scale;
    ui->yScale_DoubleSpinBox->setValue(m_image.yScale);
    ui->yScale_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getYScale() const
{
    return m_image.yScale;
}

void ImageToolbarWidget::setAspectLocked(const bool &checked)
{
     ui->aspectLocked_ToolButton->blockSignals(true);
     m_image.aspectLocked = checked;
     ui->aspectLocked_ToolButton->setChecked(m_image.aspectLocked);
     ui->aspectLocked_ToolButton->blockSignals(false);

}

bool ImageToolbarWidget::isAspectLocked() const
{
    return m_image.aspectLocked;
}

void ImageToolbarWidget::setUnits(const Unit &unit)
{
    m_image.units = unit;
}

Unit ImageToolbarWidget::getUnits() const
{
    return m_image.units;
}

void ImageToolbarWidget::setRotation(qreal angle)
{
    ui->rotation_DoubleSpinBox->blockSignals(true);
    m_image.rotation = angle;
    ui->rotation_DoubleSpinBox->setValue(m_image.rotation);
    ui->rotation_DoubleSpinBox->blockSignals(false);
}

qreal ImageToolbarWidget::getRotation() const
{
    return m_image.rotation;
}

void ImageToolbarWidget::setVisibiltity(const bool &checked)
{
    m_image.visible = checked;
    ui->visible_ToolButton->setChecked(m_image.visible);
}

bool ImageToolbarWidget::getVisibiltity() const
{
    return m_image.visible;
}

void ImageToolbarWidget::setOpacity(const qreal &opacity)
{
    m_image.opacity = opacity;
    ui->opacity_DoubleSpinBox->setValue(m_image.opacity);
}

qreal ImageToolbarWidget::getOpacity() const
{
    return m_image.opacity;
}

void ImageToolbarWidget::addImage()
{
    emit addClicked();
}

void ImageToolbarWidget::deleteImage()
{
    emit deleteClicked(m_image.id);
}

void ImageToolbarWidget::lockChanged(bool checked)
{
    m_image.locked = checked;
    ui->locked_ToolButton->setChecked(m_image.locked);
    enableWidgets();

    emit lockClicked(m_image.locked);
}

void ImageToolbarWidget::alignmentChanged()
{
    emit alignClicked();
}

void ImageToolbarWidget::xPosChanged(qreal value)
{
    m_image.xPos = value;
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::yPosChanged(qreal value)
{
    m_image.yPos = value;
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::widthChanged(qreal value)
{
    m_image.width = value;
    setXScale(m_image.width / m_pixmapWidth * 100);
    if (m_image.aspectLocked)
    {
        setYScale(m_image.xScale);
        setHeight(m_pixmapHeight * m_image.yScale / 100);
    }
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::heightChanged(qreal value)
{
    m_image.height = value;
    setYScale(m_image.height / m_pixmapHeight * 100);
    if (m_image.aspectLocked)
    {
        setXScale(m_image.yScale);
        setWidth(m_pixmapWidth * m_image.xScale / 100);
    }
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::xScaleChanged(qreal value)
{
    m_image.xScale = value;
    setWidth(m_pixmapWidth * m_image.xScale / 100);
    if (m_image.aspectLocked)
    {
        setYScale(m_image.xScale);
        setHeight(m_pixmapHeight * m_image.yScale / 100);
    }
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::yScaleChanged(qreal value)
{
    m_image.yScale = value;
    setHeight(m_pixmapHeight * m_image.yScale / 100);
    if (m_image.aspectLocked)
    {
        setXScale(m_image.yScale);
        setWidth(m_pixmapWidth * m_image.xScale / 100);
    }
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::lockAspectChanged(bool checked)
{
    m_image.aspectLocked = checked;
    ui->aspectLocked_ToolButton->setChecked(m_image.aspectLocked);
    emit lockAspectClicked(m_image.aspectLocked);
}

void ImageToolbarWidget::unitsChanged(bool checked)
{
    blockSignals(true);
    ui->units_ToolButton->setChecked(checked);
    if (ui->units_ToolButton->isChecked())
    {
        m_image.units = Unit::Px;
    }
    else
    {
        m_image.units = qApp->patternUnit();
    }
    updateUnits();
    updateImage();
    blockSignals(false);
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::rotationChanged(qreal value)
{
    m_image.rotation = value;
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::visibilityChanged(bool checked)
{
    m_image.visible = checked;
    ui->visible_ToolButton->setChecked(m_image.visible);
    ui->opacity_DoubleSpinBox->setEnabled(ui->visible_ToolButton->isChecked());
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::opacityChanged(qreal value)
{
    m_image.opacity = value;
    emit imageUpdated(m_image);
}

void ImageToolbarWidget::updateUnits()
{
    switch (m_image.units)
    {
        case Unit::Cm:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_cm_on.png"));
            m_maxScale = FromPixel(16000.0, Unit::Cm);
            ui->xScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            ui->yScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            setSuffix("cm");
            setDecimals(2);
            break;

        case Unit::Mm:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_cm_on.png"));
            m_maxScale = FromPixel(16000.0, Unit::Mm);
            ui->xScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            ui->yScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            setSuffix("cm");
            setDecimals(2);
            break;

        case Unit::Inch:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_in_on.png"));
            m_maxScale = FromPixel(16000.0, Unit::Inch);
            ui->xScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            ui->yScale_DoubleSpinBox->setMaximum(m_maxScale * 100);
            setSuffix("in");
            setDecimals(3);
            break;

        case Unit::Px:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_px_on.png"));
            ui->xScale_DoubleSpinBox->setMaximum(16000.0 / m_pixmapWidth * 100);
            ui->yScale_DoubleSpinBox->setMaximum(16000.0 / m_pixmapHeight * 100);
            setSuffix("px");
            setDecimals(0);
        default:
            break;
    }
}

void ImageToolbarWidget::enableWidgets()
{
    ui->locked_ToolButton->setEnabled(true);
    ui->xPosition_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->yPosition_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->width_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->height_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->xScale_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->yScale_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->aspectLocked_ToolButton->setEnabled(!m_image.locked);
    ui->units_ToolButton->setEnabled(!m_image.locked);
    ui->rotation_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->visible_ToolButton->setEnabled(!m_image.locked);
    ui->opacity_DoubleSpinBox->setEnabled(!m_image.locked);
}

void ImageToolbarWidget::setSuffix(QString suffix)
{
    ui->xPosition_DoubleSpinBox->setSuffix(suffix);
    ui->yPosition_DoubleSpinBox->setSuffix(suffix);
    ui->width_DoubleSpinBox->setSuffix(suffix);
    ui->height_DoubleSpinBox->setSuffix(suffix);
}

void ImageToolbarWidget::setDecimals(int precision)
{
    ui->xPosition_DoubleSpinBox->setDecimals(precision);
    ui->yPosition_DoubleSpinBox->setDecimals(precision);
    ui->width_DoubleSpinBox->setDecimals(precision);
    ui->height_DoubleSpinBox->setDecimals(precision);
    //ui->xScale_DoubleSpinBox->setDecimals(precision);
    //ui->yScale_DoubleSpinBox->setDecimals(precision);
}

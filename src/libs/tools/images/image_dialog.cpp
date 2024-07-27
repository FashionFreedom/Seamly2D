/******************************************************************************
 **  @file   image_dialog.cpp
 **  @author DS Caskey
 **  @date   Jun 3, 2022
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

#include "image_dialog.h"
#include "ui_image_dialog.h"


#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../vmisc/vabstractapplication.h"
//#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/tools/drawTools/operation/vabstractoperation.h"
#include "../vtools/visualization/visualization.h"
#include "../vtools/visualization/line/operation/vistoolmove.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"

#include <QString>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPointer>
#include <QPushButton>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSharedPointer>
#include <QStringList>
#include <QToolButton>
#include <Qt>
#include <new>

//---------------------------------------------------------------------------------------------------------------------
ImageDialog::ImageDialog(DraftImage image, qreal minDimension, qreal maxDimension, qreal pixmapWidth, qreal pixmapHeight, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ImageDialog)
    , m_image(image)
    , m_pixmapWidth(pixmapWidth)
    , m_pixmapHeight(pixmapHeight)
    , m_maxDimension(maxDimension)
    , m_minDimension(minDimension)
    , m_minOpacity(5)
    , m_flagName(true)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    updateUnits();

    setXScale(m_image.width / m_pixmapWidth * 100);
    setYScale(m_image.height / m_pixmapHeight * 100);

    updateImage();

    connect(ui->name_LineEdit, &QLineEdit::textChanged, this, &ImageDialog::nameChanged);
    connect(ui->xPosition_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::xPosChanged);
    connect(ui->yPosition_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::yPosChanged);
    connect(ui->width_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::widthChanged);
    connect(ui->height_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::heightChanged);
    connect(ui->xScale_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::xScaleChanged);
    connect(ui->yScale_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::yScaleChanged);
    connect(ui->rotation_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::rotationChanged);
    connect(ui->aspectLocked_ToolButton, &QToolButton::clicked, this, &ImageDialog::lockAspectChanged);
    connect(ui->units_ToolButton, &QToolButton::clicked, this, &ImageDialog::unitsChanged);
    connect(ui->lockImage_ToolButton,  &QCheckBox::toggled, this, &ImageDialog::lockChanged);
    connect(ui->opacity_DoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ImageDialog::opacityChanged);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &ImageDialog::dialogApply);
}

//---------------------------------------------------------------------------------------------------------------------
ImageDialog::~ImageDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
DraftImage ImageDialog::getImage() const
{
    return m_image;
}

void ImageDialog::setImage(DraftImage image)
{
    m_image = image;
    updateImage();
}

void ImageDialog::updateImage()
{
    blockSignals(true);
    ui->idText_Label->setNum(static_cast<double>(m_image.id));
    ui->name_LineEdit->setText(m_image.name);
    ui->lockImage_ToolButton->setChecked(m_image.locked);
    setXPos(m_image.xPos);
    setYPos(m_image.yPos);
    setWidth(m_image.width);
    setHeight(m_image.height);
    setAspectLocked(m_image.aspectLocked);
    setRotation(m_image.rotation);
    setVisibility(m_image.visible);
    setOpacity(m_image.opacity);
    enableWidgets();
    blockSignals(false);

    qDebug("Id = %d", m_image.id);
    qDebug("Name = %s", qUtf8Printable(m_image.name));
    qDebug("Filename = %s", qUtf8Printable(m_image.filename));
    qDebug("lock Image = %s", m_image.locked ? "True" : "False");
    qDebug("Xpos = %f", m_image.xPos);
    qDebug("YPos = %f", m_image.yPos);
    qDebug("Width = %f", m_image.width);
    qDebug("Height = %f", m_image.height);
    qDebug("lock Image Aspect = %s",  m_image.aspectLocked ? "True" : "False");
    qDebug("Units = %d", static_cast<int>(m_image.units));
    qDebug("Rotation = %f", m_image.rotation);
    qDebug("Visible = %s",  m_image.visible ? "True" : "False");
    qDebug("Opacity = %f", m_image.opacity);
    qDebug("Order = %d\n", static_cast<qint32>(m_image.order));
}

void ImageDialog::enableWidgets()
{
    ui->lockImage_ToolButton->setEnabled(true);
    ui->idText_Label->setEnabled(!m_image.locked);
    ui->name_LineEdit->setEnabled(!m_image.locked);
    ui->xPosition_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->yPosition_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->width_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->height_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->xScale_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->yScale_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->aspectLocked_ToolButton->setEnabled(!m_image.locked);
    ui->units_ToolButton->setEnabled(!m_image.locked);
    ui->rotation_DoubleSpinBox->setEnabled(!m_image.locked);
    ui->opacity_DoubleSpinBox->setEnabled(!m_image.locked);
}
//---------------------------------------------------------------------------------------------------------------------
QString ImageDialog::getName() const
{
    return m_image.name;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setName(const QString &name)
{
    m_image.name = name;
    ui->name_LineEdit->setText(name);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setOriginPoint(const int &index)
{
    Q_UNUSED(index);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getXPos() const
{
    return m_image.xPos;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setXPos(const qreal &value)
{
    ui->xPosition_DoubleSpinBox->blockSignals(true);
    m_image.xPos = value;
    if (m_image.units == Unit::Px)
    {
        ui->xPosition_DoubleSpinBox->setValue(value);
    }
    else
    {
        ui->xPosition_DoubleSpinBox->setValue(qApp->fromPixel(value));
    }
    ui->xPosition_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getYPos() const
{
    return m_image.yPos;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setYPos(const qreal &value)
{
    ui->yPosition_DoubleSpinBox->blockSignals(true);
    m_image.yPos = value;
    if (m_image.units == Unit::Px)
    {
        ui->yPosition_DoubleSpinBox->setValue(value);
    }
    else
    {
        ui->yPosition_DoubleSpinBox->setValue(qApp->fromPixel(value));
    }
    ui->yPosition_DoubleSpinBox->blockSignals(true);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getWidth() const
{
    return m_image.width;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setWidth(const qreal &value)
{
    ui->yPosition_DoubleSpinBox->blockSignals(true);

    m_image.width = value;

    if (m_image.units == Unit::Px)
    {
        if (ui->width_DoubleSpinBox->value() != value) {ui->width_DoubleSpinBox->setValue(value);};
    }
    else
    {
        if (ui->width_DoubleSpinBox->value() != qApp->fromPixel(value)) {ui->width_DoubleSpinBox->setValue(qApp->fromPixel(value));};
    }

    ui->yPosition_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getHeight() const
{
    return m_image.height;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setHeight(const qreal &value)
{
    ui->height_DoubleSpinBox->blockSignals(true);

    m_image.height = value;
    if (m_image.units == Unit::Px)
    {
        if (ui->height_DoubleSpinBox->value() != value) {ui->height_DoubleSpinBox->setValue(value);};
    }
    else
    {
        if (ui->height_DoubleSpinBox->value() != qApp->fromPixel(value)) {ui->height_DoubleSpinBox->setValue(qApp->fromPixel(value));};
    }

    ui->height_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getXScale() const
{
    return m_xScale;
}
//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setXScale(const qreal &scale)
{
    ui->xScale_DoubleSpinBox->blockSignals(true);
    m_xScale = scale;
    if (ui->xScale_DoubleSpinBox->value() != scale) {ui->xScale_DoubleSpinBox->setValue(scale);};
    ui->xScale_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getYScale() const
{
    return m_yScale;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setYScale(const qreal &scale)
{
    ui->yScale_DoubleSpinBox->blockSignals(true);
    m_yScale = scale;
    if (ui->yScale_DoubleSpinBox->value() != scale) {ui->yScale_DoubleSpinBox->setValue(scale);};
    ui->yScale_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
bool ImageDialog::isAspectLocked() const
{
    return m_image.aspectLocked;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setAspectLocked(const bool &checked)
{
    ui->aspectLocked_ToolButton->blockSignals(true);
    m_image.aspectLocked = checked;
    ui->aspectLocked_ToolButton->setChecked(checked);
    ui->aspectLocked_ToolButton->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
bool ImageDialog::isLocked() const
{
    return m_image.locked;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setLocked(const bool &checked)
{
    ui->lockImage_ToolButton->blockSignals(true);
    m_image.locked = checked;
    ui->lockImage_ToolButton->setChecked(checked);
    ui->lockImage_ToolButton->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getRotation() const
{
    return m_image.rotation;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setRotation(const qreal &angle)
{
    ui->rotation_DoubleSpinBox->blockSignals(true);
    m_image.rotation = angle;
    ui->rotation_DoubleSpinBox->setValue(angle);
    ui->rotation_DoubleSpinBox->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
bool ImageDialog::isVisible() const
{
    return m_image.visible;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setVisibility(const bool &checked)
{
    m_image.visible = checked;
}

//---------------------------------------------------------------------------------------------------------------------
qreal ImageDialog::getOpacity() const
{
    return m_image.opacity;
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::setOpacity(const qreal &opacity)
{
    ui->opacity_DoubleSpinBox->blockSignals(true);
    m_image.opacity = opacity;
    ui->opacity_DoubleSpinBox->setValue(opacity);
    ui->opacity_DoubleSpinBox->blockSignals(false);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::nameChanged(const QString &name)
{
    QPushButton *bOk = ui->buttonBox->button(QDialogButtonBox::Ok);
    SCASSERT(bOk != nullptr)
    bOk->setEnabled(true);

    QPalette palette = ui->name_LineEdit->palette();
    palette.setColor(ui->name_Label->foregroundRole(), Qt::black);
    //alette.setColor(ui->name_LineEdit->foregroundRole(), Qt::black);

    if (name.isEmpty())
    {
        bOk->setEnabled(false);
        palette.setColor(ui->name_Label->foregroundRole(), Qt::red);
        //palette.setColor(ui->name_LineEdit->foregroundRole(), Qt::red);
    }
    else
    {
        if (m_image.name != name)
        {
            QRegularExpression rx(NameRegExp());
            //if (!rx.match(name).hasMatch() || !VContainer::IsUnique(name))
            if (!VContainer::IsUnique(name))
            {
                bOk->setEnabled(false);
                palette.setColor(ui->name_Label->foregroundRole(), Qt::red);
                //palette.setColor(ui->name_LineEdit->foregroundRole(), Qt::red);
            }
            else
            {
                setName(name);
            }
        }
    }
    ui->name_Label->setPalette(palette);
    //ui->name_LineEdit->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::originChanged(const QString &text)
{
    Q_UNUSED(text);
    blockSignals(true);
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::alignmentChanged()
{

}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::xPosChanged(qreal value)
{
    blockSignals(true);
    if (m_image.units == Unit::Px)
    {
        m_image.xPos = value;
    }
    else
    {
        m_image.xPos = qApp->toPixel(value);
    }
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::yPosChanged(qreal value)
{
    blockSignals(true);
    if (m_image.units == Unit::Px)
    {
        m_image.yPos = value;
    }
    else
    {
        m_image.yPos = qApp->toPixel(value);
    }
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::widthChanged(qreal width)
{
    blockSignals(true);
    qreal oldWidth = m_image.width;
    qreal height = m_image.height;

    if (m_image.units != Unit::Px)
    {
        width = qApp->toPixel(width);
    }

    width = width < m_minDimension ? m_minDimension : width;
    width = width > m_maxDimension ? m_maxDimension : width;

    if (m_image.aspectLocked)
    {
        height = m_image.height * width / oldWidth;

        if (height > m_maxDimension)
        {
            width = m_maxDimension * oldWidth / m_image.height;
            height = m_maxDimension;
        }
        else if (height < m_minDimension)
        {
            width = m_minDimension * oldWidth / m_image.height;
            height = m_minDimension;
        }
    }

    setWidth(width);
    setXScale(m_image.width / m_pixmapWidth * 100);
    setHeight(height);
    setYScale(m_image.height / m_pixmapHeight * 100);

    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::heightChanged(qreal height)
{
    blockSignals(true);
    qreal oldHeight = m_image.height;
    qreal width = m_image.width;

    if (m_image.units != Unit::Px)
    {
        height = qApp->toPixel(height);
    }

    height = height < m_minDimension ? m_minDimension : height;
    height = height > m_maxDimension ? m_maxDimension : height;

    if (m_image.aspectLocked)
    {
        width = m_image.width * height / oldHeight;

        if (width > m_maxDimension)
        {
            height = m_maxDimension * oldHeight / m_image.width;
            width = m_maxDimension;
        }
        else if (width < m_minDimension)
        {
            height = m_minDimension * oldHeight / m_image.width;
            width = m_minDimension;
        }
    }

    setWidth(width);
    setXScale(m_image.width / m_pixmapWidth * 100);
    setHeight(height);
    setYScale(m_image.height / m_pixmapHeight * 100);

    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::xScaleChanged(qreal xScale)
{
    blockSignals(true);
    qreal oldYScale = m_xScale;
    qreal yScale = m_yScale;

    qreal maxXScale = m_maxDimension / m_pixmapWidth * 100;
    qreal minXScale = m_minDimension / m_pixmapWidth * 100;
    qreal maxYScale = m_maxDimension / m_pixmapHeight * 100;
    qreal minYScale = m_minDimension / m_pixmapHeight * 100;

    xScale = xScale < minXScale ? minXScale : xScale;
    xScale = xScale > maxXScale ? maxXScale : xScale;

    if (m_image.aspectLocked)
    {
        yScale = m_yScale * xScale / oldYScale;
        if (yScale > maxYScale)
        {
            xScale = maxYScale * oldYScale / m_yScale;
            yScale = maxYScale;
        }
        else if (yScale < minYScale)
        {
            xScale = minYScale * oldYScale / m_yScale;
            yScale = minYScale;
        }
    }

    setXScale(xScale);
    setWidth(m_pixmapWidth * m_xScale / 100);
    setYScale(yScale);
    setHeight(m_pixmapHeight * m_yScale / 100);

    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::yScaleChanged(qreal scale)
{
    blockSignals(true);
    qreal oldXScale = m_yScale;
    qreal xScale = m_xScale;

    qreal maxXScale = m_maxDimension / m_pixmapWidth * 100;
    qreal minXScale = m_minDimension / m_pixmapWidth * 100;
    qreal maxYScale = m_maxDimension / m_pixmapHeight * 100;
    qreal minYScale = m_minDimension / m_pixmapHeight * 100;

    scale = scale < minYScale ? minYScale : scale;
    scale = scale > maxYScale ? maxYScale : scale;

    if (m_image.aspectLocked)
    {
        xScale = m_xScale * scale / oldXScale;
        if (xScale > maxXScale)
        {
            scale = maxXScale * oldXScale / m_xScale;
            xScale = maxXScale;
        }
        else if (xScale < minXScale)
        {
            scale = minXScale * oldXScale / m_xScale;
            xScale = minXScale;
        }
    }

    setYScale(scale);
    setHeight(m_pixmapHeight * m_yScale / 100);
    setXScale(xScale);
    setWidth(m_pixmapWidth * m_xScale / 100);

    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::lockChanged(bool checked)
{
    blockSignals(true);
    m_image.locked = checked;
    ui->lockImage_ToolButton->setChecked(m_image.locked);
    enableWidgets();
    blockSignals(false);
    emit lockClicked(m_image.locked);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::lockAspectChanged(bool checked)
{
    blockSignals(true);
    m_image.aspectLocked = checked;
    ui->aspectLocked_ToolButton->setChecked(m_image.aspectLocked);
    blockSignals(false);
    emit lockAspectClicked(m_image.aspectLocked);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::updateSpinboxesRanges()
{
    if (m_image.units == Unit::Px)
    {
        ui->width_DoubleSpinBox->setMinimum(m_minDimension);
        ui->height_DoubleSpinBox->setMinimum(m_minDimension);

        ui->xScale_DoubleSpinBox->setMinimum(m_minDimension / m_pixmapWidth * 100);
        ui->yScale_DoubleSpinBox->setMinimum(m_minDimension / m_pixmapHeight * 100);

        ui->width_DoubleSpinBox->setMaximum(m_maxDimension);
        ui->height_DoubleSpinBox->setMaximum(m_maxDimension);

        ui->xScale_DoubleSpinBox->setMaximum(m_maxDimension / m_pixmapWidth * 100);
        ui->yScale_DoubleSpinBox->setMaximum(m_maxDimension / m_pixmapHeight * 100);

    }
    else
    {
        ui->width_DoubleSpinBox->setMinimum(qApp->fromPixel(m_minDimension));
        ui->height_DoubleSpinBox->setMinimum(qApp->fromPixel(m_minDimension));

        ui->xScale_DoubleSpinBox->setMinimum(qApp->fromPixel(m_minDimension / m_pixmapWidth * 100));
        ui->yScale_DoubleSpinBox->setMinimum(qApp->fromPixel(m_minDimension / m_pixmapHeight * 100));

        ui->width_DoubleSpinBox->setMaximum(qApp->fromPixel(m_maxDimension));
        ui->height_DoubleSpinBox->setMaximum(qApp->fromPixel(m_maxDimension));

        ui->xScale_DoubleSpinBox->setMaximum(qApp->fromPixel(m_maxDimension / m_pixmapWidth * 100));
        ui->yScale_DoubleSpinBox->setMaximum(qApp->fromPixel(m_maxDimension / m_pixmapHeight * 100));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::unitsChanged()
{
    blockSignals(true);
    qreal tempHeight = m_image.height;
    qreal tempWidth = m_image.width;
    qreal tempXPos = m_image.xPos;
    qreal tempYPos = m_image.yPos;

    if (m_image.units != Unit::Px)
    {
        m_image.units = Unit::Px;
        updateUnits();

        ui->width_DoubleSpinBox->setValue(tempWidth);
        ui->height_DoubleSpinBox->setValue(tempHeight);
        ui->xPosition_DoubleSpinBox->setValue(tempXPos);
        ui->yPosition_DoubleSpinBox->setValue(tempYPos);
    }
    else
    {
        m_image.units = qApp->patternUnit();
        updateUnits();

        ui->width_DoubleSpinBox->setValue(qApp->fromPixel(tempWidth));
        ui->height_DoubleSpinBox->setValue(qApp->fromPixel(tempHeight));
        ui->xPosition_DoubleSpinBox->setValue(qApp->fromPixel(tempXPos));
        ui->yPosition_DoubleSpinBox->setValue(qApp->fromPixel(tempYPos));
    }
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::rotationChanged(qreal angle)
{
    blockSignals(true);
    m_image.rotation = angle;
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::visibilityChanged(bool checked)
{
    blockSignals(true);
    m_image.visible = checked;
    blockSignals(false);
    emit imageUpdated(m_image);
}

//---------------------------------------------------------------------------------------------------------------------
void ImageDialog::opacityChanged(qreal opacity)
{
    blockSignals(true);
    if (opacity < m_minOpacity){
        m_image.opacity = m_minOpacity;
    }
    else{
        m_image.opacity = opacity;
    }
    blockSignals(false);
    emit imageUpdated(m_image);
}

void ImageDialog::updateUnits()
{
    switch (m_image.units)
    {
        case Unit::Cm:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_cm_on.png"));
            setSuffix("cm");
            setDecimals(2);
            break;

        case Unit::Mm:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_mm_on.png"));
            setSuffix("mm");
            setDecimals(2);
            break;

        case Unit::Inch:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_in_on.png"));
            setSuffix("in");
            setDecimals(3);
            break;

        case Unit::Px:
            ui->units_ToolButton->setIcon(QIcon("://icon/32x32/units_px_on.png"));
            setSuffix("px");
            setDecimals(0);
        default:
            break;
    }
}

void ImageDialog::setSuffix(QString suffix)
{
    ui->xPosition_DoubleSpinBox->setSuffix(suffix);
    ui->yPosition_DoubleSpinBox->setSuffix(suffix);
    ui->width_DoubleSpinBox->setSuffix(suffix);
    ui->height_DoubleSpinBox->setSuffix(suffix);
}

void ImageDialog::setDecimals(int precision)
{
    ui->xPosition_DoubleSpinBox->setDecimals(precision);
    ui->yPosition_DoubleSpinBox->setDecimals(precision);
    ui->width_DoubleSpinBox->setDecimals(precision);
    ui->height_DoubleSpinBox->setDecimals(precision);
}


void ImageDialog::dialogApply(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
    {
    updateImage();
    emit applyClicked(m_image);
    }
}

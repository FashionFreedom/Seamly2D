/******************************************************************************
 **  @file   image_toolbar.h
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

#ifndef IMAGE_TOOLBAR_H
#define IMAGE_TOOLBAR_H

#include "../tools/image_item.h"
#include "../vmisc/def.h"

#include <memory>
#include <QToolBar>

namespace Ui
{
    class ImageToolbarWidget;
}

/**
 * A toolbar that offers all widgets for choosing a pen.
 */
class ImageToolbarWidget: public QWidget
{
    Q_OBJECT

public:
	           ImageToolbarWidget(QWidget *parent = 0);
    virtual   ~ImageToolbarWidget();

public slots:
    void        setImage(DraftImage image);
    void        updateImage();

    bool        isLocked() const;
    void        setLocked(const bool &checked);

    qreal       getXPos() const;
    void        setXPos(const qreal &pos);

    qreal       getYPos() const;
    void        setYPos(const qreal &pos);

    qreal       geWidth() const;
    void        setWidth(const qreal &value);

    qreal       getHeight() const;
    void        setHeight(const qreal &value);

    qreal       getXScale() const;
    void        setXScale(const qreal &scale);

    qreal       getYScale() const;
    void        setYScale(const qreal &scale);

    bool        isAspectLocked() const;
    void        setAspectLocked(const bool &checked);

    void        setUnits(const Unit &unit);
    Unit        getUnits() const;

    qreal       getRotation() const;
    void        setRotation(qreal angle);

    bool        getVisibiltity() const;
    void        setVisibiltity(const bool &checked);

    qreal       getOpacity() const;
    void        setOpacity(const qreal &opacity);

    void        addImage();
    void        deleteImage();
    void        lockChanged(bool checked);
    void        alignmentChanged();
    void        xPosChanged(qreal value);
    void        yPosChanged(qreal value);
    void        widthChanged(qreal value);
    void        heightChanged(qreal value);
    void        xScaleChanged(qreal value);
    void        yScaleChanged(qreal value);
    void        lockAspectChanged(bool checked);
    void        unitsChanged(bool checked);
    void        rotationChanged(qreal value);
    void        visibilityChanged(bool checked);
    void        opacityChanged(qreal value);

signals:
    void        addClicked();
    void        deleteClicked(quint32 id);
    void        lockClicked(bool state);
    void        alignClicked();
    void        lockAspectClicked(bool state);
    void        imageUpdated(DraftImage image);

private:
    Q_DISABLE_COPY(ImageToolbarWidget)
    Ui::ImageToolbarWidget *ui;
    ImageItem  *m_item;
    DraftImage  m_image;
    qreal       m_pixmapWidth;
    qreal       m_pixmapHeight;
    qreal       m_maxScale;

    void        updateUnits();
    void        setSuffix(QString suffix);
    void        setDecimals(int precision);
    void        enableWidgets();
};

#endif

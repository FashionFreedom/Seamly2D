/******************************************************************************
 **  @file   image_dialog.h
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

#ifndef IMAGE_DIALOG_H
#define IMAGE_DIALOG_H

#include "../vmisc/def.h"
#include "../vtools/dialogs/tools/dialogtool.h"
#include "../vtools/tools/drawTools/operation/vabstractoperation.h"

#include <qcompilerdetection.h>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

namespace Ui
{
    class ImageDialog;
}

class ImageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit         ImageDialog(DraftImage image, qreal minDimension, qreal maxDimension, qreal pixmapWidth, qreal pixmapHeight, QWidget *parent);
    virtual         ~ImageDialog();

    DraftImage       getImage() const;
    void             setImage(DraftImage image);
    void             updateImage();
    void             enableWidgets();

    QString          getName() const;
    void             setName(const QString &name);

    void             setOriginPoint(const int &index);

    qreal            getXPos() const;
    void             setXPos(const qreal &value);

    qreal            getYPos() const;
    void             setYPos(const qreal &value);

    qreal            getWidth() const;
    void             setWidth(const qreal &value);

    qreal            getHeight() const;
    void             setHeight(const qreal &value);

    qreal            getXScale() const;
    void             setXScale(const qreal &value);

    qreal            getYScale() const;
    void             setYScale(const qreal &value);

    bool             isAspectLocked() const;
    void             setAspectLocked(const bool &state);

    bool             isLocked() const;
    void             setLocked(const bool &state);

    qreal            getRotation() const;
    void             setRotation(const qreal &value);

    bool             isVisible() const;
    void             setVisibility(const bool &checked);

    qreal            getOpacity() const;
    void             setOpacity(const qreal &opacity);

private slots:
    void             nameChanged(const QString &name);
    void             originChanged(const QString &text);
    void             lockChanged(bool checked);
    void             alignmentChanged();
    void             xPosChanged(qreal value);
    void             yPosChanged(qreal value);
    void             widthChanged(qreal width);
    void             heightChanged(qreal height);
    void             xScaleChanged(qreal value);
    void             yScaleChanged(qreal value);
    void             lockAspectChanged(bool checked);
    void             unitsChanged();
    void             rotationChanged(qreal value);
    void             visibilityChanged(bool checked);
    void             opacityChanged(qreal value);

    void             updateUnits();
    void             updateSpinboxesRanges();

    void             setSuffix(QString suffix);
    void             setDecimals(int precision);

    void             dialogApply(QAbstractButton *button);

signals:
    void             lockClicked(bool state);
    //void             alignClicked();
    void             lockAspectClicked(bool state);
    void             imageUpdated(DraftImage image);
    void             applyClicked(DraftImage image);

private:
    Q_DISABLE_COPY(ImageDialog)
    Ui::ImageDialog *ui;
    DraftImage       m_image;
    qreal            m_pixmapWidth;
    qreal            m_pixmapHeight;
    qreal            m_maxDimension;
    qreal            m_minDimension;
    qreal            m_minOpacity;
    qreal            m_xScale;
    qreal            m_yScale;
    bool             m_flagName;
};

#endif // IMAGE_DIALOG_H

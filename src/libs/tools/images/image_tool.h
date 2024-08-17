/******************************************************************************
 **  @file   image_tool.h
 **  @author Evans PERRET
 **  @date   April 21, 2024
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

#ifndef IMAGE_TOOL_H
#define IMAGE_TOOL_H

#include <QObject>
#include <QString>
#include <QDomElement>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vmaingraphicsscene.h"

QString          getImageFilename(QWidget *parent);


class ImageTool : public QObject
{
    Q_OBJECT

public:
    explicit ImageTool(QObject *parent, VAbstractPattern *doc, VMainGraphicsScene *draftScene, QString filename);
    explicit ImageTool(QObject *parent, VAbstractPattern *doc, VMainGraphicsScene *draftScene, DraftImage image);

    bool        creationWasSuccessful=false;

    DraftImage  image;
    ImageItem*  imageItem;

    void             addToFile();

signals:
    void             setStatusMessage(QString message);

protected slots:
    void        handleDeleteImage(quint32 id);
    void        handleImageSelected(quint32 id);

private:
    VAbstractPattern    *m_doc;
    VMainGraphicsScene  *m_draftScene;
    QDomElement         m_oldDomElement;

    void        addImage();

    void             saveOptions(QDomElement &tag);
    void             saveChanges();
};

#endif // IMAGE_TOOL_H

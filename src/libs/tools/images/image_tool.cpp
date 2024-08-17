/******************************************************************************
 **  @file   image_tool.cpp
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

#include "image_tool.h"

#include <QImageReader>
#include <QMessageBox>
#include <QLoggingCategory>
#include <QWidget>

#include "../../../app/seamly2d/core/application_2d.h"
#include "image_item.h"
#include "../vpropertyexplorer/checkablemessagebox.h"

#include "../../vtools/undocommands/deltool.h"
#include "../../vtools/undocommands/savetooloptions.h"
#include "../../vtools/undocommands/addimage.h"


Q_LOGGING_CATEGORY(vImageTool, "v.imageTool")


//---------------------------------------------------------------------------------------------------------------------
QString getImageFilename(QWidget *parent)
{
    const QString filter = QObject::tr("Images") + QLatin1String(" (*.bmp *.gif *.jpeg *.jpg *.pbm *.pgm *.png *.ppm *.svg *.tif *.xbm *.xpm);;") +
                           "BMP" + QLatin1String(" (*.bmp);;") +
                           "GIF" + QLatin1String(" (*.gif);;") +
                           "JPEG" + QLatin1String(" (*.jpeg);;") +
                           "JPG" + QLatin1String(" (*.jpg);;") +
                           "PBM" + QLatin1String(" (*.pbm);;") +
                           "PGM" + QLatin1String(" (*.pgm);;") +
                           "PNG" + QLatin1String(" (*.png);;") +
                           "PPM" + QLatin1String(" (*.ppm);;") +
                           "SVG" + QLatin1String(" (*.svg);;") +
                           "TIF" + QLatin1String(" (*.tif);;") +
                           "XBM" + QLatin1String(" (*.xbm);;") +
                           "XPM" + QLatin1String(" (*.xpm)");

    const QString path = qApp->Seamly2DSettings()->getImageFilePath();

    bool usedNotExistedDir = false;
    QDir directory(path);
    if (!directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    const QString filename = QFileDialog::getOpenFileName(parent, QObject::tr("Open Image File"), path, filter, nullptr,
                                                          QFileDialog::DontUseNativeDialog);

    return filename;
}


//Constructor called from GUI
ImageTool::ImageTool(QObject *parent, VAbstractPattern *doc, VMainGraphicsScene *draftScene, QString filename)
    : QObject(parent),
    m_doc(doc),
    m_draftScene(draftScene)
{
    qCDebug(vImageTool, "Image filename: %s", qUtf8Printable(filename));

    if (filename.isEmpty())
    {
        qCDebug(vImageTool, "Can't load image");
        QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not load the image."), QMessageBox::Ok);
        return;
    }

    QFileInfo f(filename);

    image.name = f.baseName();
    image.filename = filename;
    image.units = qApp->patternUnit();

    image.id = VContainer::getNextId();

    addImage();
}

//Constructor called when file is full-parsed
ImageTool::ImageTool(QObject *parent, VAbstractPattern *doc, VMainGraphicsScene *draftScene, DraftImage img)
    : QObject(parent),
    image(img),
    m_doc(doc),
    m_draftScene(draftScene)
{
    qCDebug(vImageTool, "Image filename: %s", qUtf8Printable(image.filename));

    QFileInfo fileInfo(image.filename);

    if (!fileInfo.exists())
    {
        const QString text = tr("The image <br/><br/> <b>%1</b> <br/><br/> could not be found. Do you "
                                "want to update the file location?").arg(image.filename);
        QMessageBox::StandardButton result = QMessageBox::question(nullptr, tr("Loading image"), text,
                                                                   QMessageBox::Yes | QMessageBox::No,
                                                                   QMessageBox::Yes);
        if (result == QMessageBox::No)
        {
            return;
        }
        else
        {
            bool askAgain = true;
            while(askAgain)
            {
                image.filename = getImageFilename(nullptr);
                if (image.filename.isEmpty())
                {
                    qCDebug(vImageTool, "No image selected");
                    QMessageBox::critical(nullptr, tr("Import Image"), tr("No image was selected..."), QMessageBox::Ok);
                    continue;
                }

                QImageReader imageReader(image.filename);

                if(!imageReader.canRead())
                {
                    qCDebug(vImageTool, "Can't read image");
                    QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not read the image.") + "\n" + tr("File may be corrupted..."), QMessageBox::Ok);
                    continue;
                }
                askAgain = false;
            }
            image.name = fileInfo.baseName();

            QDomElement domElement = m_doc->elementById(image.id, VAbstractPattern::TagDraftImage);
            QDomElement newDomElement = domElement.cloneNode().toElement();
            m_doc->SetAttribute(newDomElement, VAbstractPattern::AttrName,  image.name);
            m_doc->SetAttribute(newDomElement, VAbstractPattern::AttrSource, image.filename);
            domElement.parentNode().replaceChild(newDomElement, domElement);
        }
    }

    if (image.name.isEmpty())
    {
        image.name = fileInfo.baseName();
    }

    addImage();
}


void  ImageTool::addImage()
{
    QImageReader imageReader(image.filename);

    if(!imageReader.canRead())
    {
        qCDebug(vImageTool, "Can't read image");
        QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not read the image.") + "\n" + tr("File may be corrupted..."), QMessageBox::Ok);
        return;
    }

    imageItem = new ImageItem(this, m_doc, image);
    m_doc->addBackgroundImage(image.id, imageItem);
    m_draftScene->addItem(imageItem);
    //Need error dialog

    //connect(this, &MainWindow::EnableImageSelection, item, &ImageItem::enableSelection);
    //connect(this, &MainWindow::EnableImageHover, item, &ImageItem::enableHovering);
    connect(imageItem, &ImageItem::deleteImage, this, &ImageTool::handleDeleteImage);
    //connect(item, &ImageItem::imageSelected, this, &MainWindow::handleImageSelected);
    connect(imageItem, &ImageItem::setStatusMessage, this, [this](QString message) {emit setStatusMessage(message);});
    connect(imageItem, &ImageItem::imageNeedsSave, this, &ImageTool::saveChanges);

    creationWasSuccessful = true;
}


//---------------------------------------------------------------------------------------------------------------------
void ImageTool::handleDeleteImage(quint32 id)
{
    Q_UNUSED(id);

    qCDebug(vImageTool, "delete Image");
    bool deleteConfirmed = true;

    if (qApp->Settings()->getConfirmItemDelete())
    {
        Utils::CheckableMessageBox msgBox(qApp->getMainWindow());
        msgBox.setWindowTitle(tr("Confirm deletion"));
        msgBox.setText(tr("Do you really want to delete?"));
        msgBox.setStandardButtons(QDialogButtonBox::Yes | QDialogButtonBox::No);
        msgBox.setDefaultButton(QDialogButtonBox::No);
        msgBox.setIconPixmap(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion).pixmap(32, 32) );

        int dialogResult = msgBox.exec();

        if (dialogResult != QDialog::Accepted)
        {
            deleteConfirmed = false;
        }

        qApp->Settings()->setConfirmItemDelete(not msgBox.isChecked());

    }

    if (deleteConfirmed)
    {
        imageItem->deleteImageItem();

        DelTool *delTool = new DelTool(m_doc, id);
        connect(delTool, &DelTool::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
        qApp->getUndoStack()->push(delTool);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void ImageTool::handleImageSelected(quint32 id)
{
    ImageItem *item = m_doc->getBackgroundImage(id);
    if (item)
    {
        // May be useful in the development of the background-image feature
    }
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief addToFile add tag with informations about image into file.
 */
void ImageTool::addToFile()
{
    QDomElement domElement = m_doc->createElement(VAbstractPattern::TagDraftImage);

    saveOptions(domElement);

    AddImage *cmd = new AddImage(domElement, m_doc);
    connect(cmd, &AddImage::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
    qApp->getUndoStack()->push(cmd);
}


//---------------------------------------------------------------------------------------------------------------------
void ImageTool::saveOptions(QDomElement &tag)
{
    image = imageItem->getImage();

    m_doc->SetAttribute(tag, VDomDocument::AttrId, image.id);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrName,  image.name);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrSource, image.filename);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrWidth,  image.width);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrHeight, image.height);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrUnits,  UnitsToStr(image.units));
    m_doc->SetAttribute(tag, VAbstractPattern::AttrXPos,   image.xPos);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrYPos,   image.yPos);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrOpacity, image.opacity);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrRotation, image.rotation);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrAspectRatio, image.aspectLocked);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrVisible, image.visible);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrLocked, image.locked);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrBasepoint, image.basepoint);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrOrder, qreal(image.order));
    m_doc->SetAttribute(tag, VAbstractPattern::AttrXOffset, image.xOrigin);
    m_doc->SetAttribute(tag, VAbstractPattern::AttrYOffset, image.yOrigin);
}

void ImageTool::saveChanges()
{
    qCDebug(vImageTool, "Saving image options");

    quint32 id = imageItem->getImage().id;
    QDomElement domElement = m_doc->elementById(id, VAbstractPattern::TagDraftImage);
    if (domElement.isElement())
    {
        QDomElement newDomElement = domElement.cloneNode().toElement();

        saveOptions(newDomElement);

        SaveToolOptions *saveOptions = new SaveToolOptions(domElement, newDomElement, m_doc, id);
        connect(saveOptions, &SaveToolOptions::NeedLiteParsing, m_doc, &VAbstractPattern::LiteParseTree);
        qApp->getUndoStack()->push(saveOptions);
    }
    else
    {
        qCDebug(vUndo, "Can't get node by id = %u.", id);
        return;
    }

}

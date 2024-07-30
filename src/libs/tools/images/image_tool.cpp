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

bool ImageTool::m_firstImportImage = true;


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

    addImage(Source::FromGui);
}


ImageTool::ImageTool(QObject *parent, VAbstractPattern *doc, VMainGraphicsScene *draftScene, DraftImage image)
    : QObject(parent),
    image(image),
    m_doc(doc),
    m_draftScene(draftScene)
{
    qCDebug(vImageTool, "Image filename: %s", qUtf8Printable(image.filename));

    if (image.filename.isEmpty())
    {
        qCDebug(vImageTool, "Can't load image");
        QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not load the image."), QMessageBox::Ok);
        return;
    }

    QFileInfo f(image.filename);

    if (image.name.isEmpty())
    {
        image.name = f.baseName();
    }

    addImage(Source::FromFile);
}


void  ImageTool::addImage(const Source &typeCreation)
{
    QImageReader imageReader(image.filename);

    if (typeCreation == Source::FromGui)
    {
        image.id = VContainer::getNextId();
    }

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


    if(m_firstImportImage)
    {
        qCDebug(vImageTool, "This is the first time an image is loaded.");
        InfoUnsavedImages();
    }

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
 * @brief InfoUnsavedImages is called when the user imports his first image.
 */
void ImageTool::InfoUnsavedImages()
{
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(QMessageBox::Information,
                                                               tr("Images will not be saved"),
                                                               tr("Please note that the images can not be saved and that they are not affected "
                                                                  "by the undo and redo functions in the current version of the software.\n\n"
                                                                  "You may want to take a screenshot of the image properties dialog before closing the software "
                                                                  "to be able to recreate identically the image when opening the software again."),
                                                               QMessageBox::NoButton,
                                                               nullptr,Qt::Sheet));

        messageBox->setWindowModality(Qt::ApplicationModal);
        messageBox->setWindowFlags(Qt::WindowFlags() & ~Qt::WindowContextHelpButtonHint
                                   & ~Qt::WindowMaximizeButtonHint
                                   & ~Qt::WindowMinimizeButtonHint);

        messageBox->exec();
        m_firstImportImage = false;
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

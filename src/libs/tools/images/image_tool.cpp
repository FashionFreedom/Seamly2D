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

    addImage();
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

    addImage();
}


void  ImageTool::addImage()
{
    QImageReader imageReader(image.filename);

    image.id = VContainer::getNextId();

    if(!imageReader.canRead())
    {
        qCDebug(vImageTool, "Can't read image");
        QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not read the image.") + "\n" + tr("File may be corrupted..."), QMessageBox::Ok);
        return;
    }

    image.pixmap = QPixmap::fromImageReader(&imageReader);

    if(image.pixmap.isNull())
    {
        qCDebug(vImageTool, "Can't read image");
        QMessageBox::critical(nullptr, tr("Import Image"), tr("Could not read the image.") + "\n" + tr("File may be corrupted or empty..."), QMessageBox::Ok);
        return;
    }

    if (image.width == 0 || image.height == 0)
    {
        image.width  = image.pixmap.width();
        image.height = image.pixmap.height();
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
        m_doc->removeBackgroundImage(id);
        imageItem->deleteImageItem();
        deleteLater();
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

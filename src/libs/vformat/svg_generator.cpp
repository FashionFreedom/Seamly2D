/******************************************************************************
 **  @file   svg_generator.cpp
 **  @author Evans PERRET
 **  @date   September 21, 2024
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

#include "svg_generator.h"
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QSvgGenerator>
#include <QGraphicsItem>
#include <QPainter>
#include <QFileInfo>

SvgGenerator::SvgGenerator(QGraphicsRectItem *paper, QString name, QString description, int resolution):
    m_svgCounter(0),
    m_paper(paper),
    m_filepath(name),
    m_description(description),
    m_resolution(resolution),
    m_tempSvgPrefix("tempSvg")
{
    QFileInfo fileInfo(m_filepath);
    m_folderPath = fileInfo.absolutePath();
    m_name = fileInfo.baseName();
    m_tempSvgFolderName = m_tempSvgPrefix + "_" + m_name;
}


bool SvgGenerator::loadSvgIntoDom(QDomDocument &domDocument, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Error : Impossible to open the SVG file :" << filePath;
        return false;
    }
    if (!domDocument.setContent(&file)) {
        qDebug() << "Error : Impossible to load the SVG content in the QDomDocument.";
        file.close();
        return false;
    }
    file.close();
    return true;
}

QDomDocument SvgGenerator::mergeSvgDoms(const QList<QDomDocument> &domList)
{
    /*domList contains DOM representations of multiple SVG
    Assuming each svg contains a main group containing every graphical item of the svg,
    this function adds to the first svg of the list all the main groups of the other svgs,
    thus creating a single svg with each svg of the list in it, every svg being in its own group.
    This function is used in order to create svgs containing groups*/

    if (domList.isEmpty()) {
        qDebug() << "Error : the SVG list is empty";
        return QDomDocument();
    }

    QDomDocument mergedSvg = domList.at(0);

    QDomElement mergedSvgRoot = mergedSvg.documentElement();
    if (mergedSvgRoot.tagName() != "svg") {
        qDebug() << "Error : the first SVG does not contain a <svg> tag.";
        return QDomDocument();
    }

    for (int i = 1; i < domList.size(); ++i) {
        QDomDocument domSvg = domList.at(i);
        QDomElement svgRoot = domSvg.documentElement();
        if (svgRoot.tagName() != "svg") {
            qDebug() << "Error : the SVG does not contain a <svg> tag.";
            return QDomDocument();
        }
        QDomNodeList svgGroups = svgRoot.elementsByTagName("g");
        if (svgGroups.isEmpty()) {
            qDebug() << "Error : the SVG does not contain a <g> tag.";
            return QDomDocument();
        }
        QDomElement mainGroup = svgGroups.at(0).toElement();
        mergedSvgRoot.appendChild(mainGroup);
    }

    return mergedSvg;
}

void SvgGenerator::addSvgFromScene(QGraphicsScene *scene)
{
    m_svgCounter++;

    QSvgGenerator svgGenerator;
    svgGenerator.setFileName(getTempFilePath(m_svgCounter));
    svgGenerator.setSize(m_paper->rect().size().toSize());
    svgGenerator.setViewBox(m_paper->rect());
    svgGenerator.setTitle(QObject::tr("Pattern"));
    svgGenerator.setDescription(m_description);
    svgGenerator.setResolution(m_resolution);

    QPainter painter;
    painter.begin(&svgGenerator);
    painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter, m_paper->rect(), m_paper->rect(), Qt::IgnoreAspectRatio);
    painter.end();
}


void SvgGenerator::generate()
{
    QList<QDomDocument> domList;

    for(int i = 1; i <= m_svgCounter; i++)
    {
        QDomDocument domDoc;
        QString path = getTempFilePath(i);
        QFile file(path);
        if (!file.exists()) {
            qDebug() << "Error : the SVG file does not exist.";
            continue;
        }
        loadSvgIntoDom(domDoc, path);
        domList.append(domDoc);

        if (!file.remove()) {
            qDebug() << "Error : unable to remove " << path;
        }
    }

    QDomDocument mergedSvg = mergeSvgDoms(domList);

    QFile outputFile(m_filepath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error : Couldn't write the output file.";
    }

    QTextStream stream(&outputFile);
    stream << mergedSvg.toString();
    outputFile.close();

    QDir dir(m_folderPath);
    if(!dir.rmdir(m_tempSvgFolderName))
    {
        qDebug() << "Error : Couldn't remove the temp SVG folder.";
    }

    qDebug() << "Merged SVG Generated!";
}


QString SvgGenerator::getTempFilePath(int id)
{
    QDir dir(m_folderPath);
    if (!dir.cd(m_tempSvgFolderName))
    {
        dir.mkdir(m_tempSvgFolderName);
        dir.cd(m_tempSvgFolderName);
    }

    return dir.path() + "/" + m_tempSvgPrefix + "_" + m_name + "_" + QString::number(id) + ".svg";
}

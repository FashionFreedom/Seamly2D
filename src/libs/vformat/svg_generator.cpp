/******************************************************************************
 **  @file   svg_generator.cpp
 **  @author Evans PERRET
 **  @date   September 21, 2024
 **
 **  @brief
 **  Custom SVG generator to handle groups in SVGs
 **
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
#include <QDebug>
#include <QSvgGenerator>
#include <QGraphicsItem>
#include <QPainter>
#include <QBuffer>


//---------------------------------------------------------------------------------------------------------------------
SvgGenerator::SvgGenerator(QGraphicsRectItem *paper, QString name, QString description, int resolution):
    m_paper(paper),
    m_filepath(name),
    m_description(description),
    m_resolution(resolution)
{
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Merge all the SVGs in the m_domList list into a single SVG
 * @return The merged SVG as a DOM document
 * @details m_domList contains DOM representations of multiple SVGs
            Assuming each svg contains a main group containing every graphical item of the svg,
            this function adds to the first svg of the list all the main groups of the other svgs,
            thus creating a single svg with each svg of the list in it, every svg being in its own group.
            This function is used in order to create svgs containing groups
 */
QDomDocument SvgGenerator::mergeSvgDoms()
{
    if (m_domList.isEmpty()) {
        qDebug() << "Error : the SVG list is empty";
        return QDomDocument();
    }

    QDomDocument mergedSvg = m_domList.at(0);

    QDomElement mergedSvgRoot = mergedSvg.documentElement();
    if (mergedSvgRoot.tagName() != "svg") {
        qDebug() << "Error : the first SVG does not contain a <svg> tag.";
        return QDomDocument();
    }

    for (int i = 1; i < m_domList.size(); ++i) {
        QDomDocument domSvg = m_domList.at(i);
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


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add a new SVG to the list of SVGs to be merged into a single SVG
 * @param scene : the scene that must be converted to SVG
 * @return void
 * @details This function creates a SVG from the given scene and converts it into
            a DOM that is added to the m_domList list of SVGs to be merged.
*/
void SvgGenerator::addSvgFromScene(QGraphicsScene *scene)
{


    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);

    QSvgGenerator svgGenerator;
    svgGenerator.setOutputDevice(&buffer);
    svgGenerator.setSize(m_paper->rect().size().toSize());
    svgGenerator.setViewBox(m_paper->rect());
    svgGenerator.setTitle(QObject::tr("Pattern"));
    svgGenerator.setResolution(m_resolution);

    QPainter painter;
    painter.begin(&svgGenerator);
    painter.setFont( QFont( "Arial", 8, QFont::Normal ) );
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setBrush ( QBrush ( Qt::NoBrush ) );
    scene->render(&painter, m_paper->rect(), m_paper->rect(), Qt::IgnoreAspectRatio);
    painter.end();

    QDomDocument domDoc;
    if (domDoc.setContent(byteArray)) {
        m_domList.append(domDoc);
    } else {
        qDebug() << "Error : Impossible to load the SVG content in the QDomDocument.";
    }

    buffer.close();
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Generate the merged SVG where each previously given scene is grouped separately.
 * @return void
 * @details This function merges the SVGs of the m_domList list and writes the result
            in a file at the path given in the constructor.
*/
void SvgGenerator::generate()
{


    QDomDocument mergedSvg = mergeSvgDoms();

    QFile outputFile(m_filepath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error : Couldn't write the output file.";
        return;
    }

    QTextStream stream(&outputFile);
    stream << mergedSvg.toString();
    outputFile.close();

    qDebug() << "Merged SVG Generated!";
}

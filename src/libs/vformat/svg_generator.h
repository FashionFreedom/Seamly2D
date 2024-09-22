/******************************************************************************
 **  @file   svg_generator.h
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

#ifndef SVG_GENERATOR_H
#define SVG_GENERATOR_H

#include <QDomDocument>
#include <QGraphicsScene>


class SvgGenerator
{
public:
    SvgGenerator(QGraphicsRectItem *paper, QString name, QString description, int resolution);
    void addSvgFromScene(QGraphicsScene *scene);
    void generate();


private:
    bool loadSvgIntoDom(QDomDocument &domDocument, const QString &filePath);
    QDomDocument mergeSvgDoms(const QList<QDomDocument> &domList);
    QString getTempFilePath(int id);

    int m_svgCounter;
    QGraphicsRectItem *m_paper;
    QString m_filepath;
    QString m_folderPath;
    QString m_name;
    QString m_description;
    int m_resolution;
    QString m_tempSvgFolderName;
    QString m_tempSvgPrefix;
};

#endif // SVG_GENERATOR_H

/******************************************************************************
 **  @file   svg_generator.h
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
    QDomDocument mergeSvgDoms();

    void removeEmptyGroups(QDomElement &mainGroup);
    void removeEmptyOriginPath(QDomElement &mainGroup);
    void cleanSvg(QDomElement &mainGroup);

    QGraphicsRectItem *m_paper;
    QString m_filepath;
    QString m_description;
    int m_resolution;

    QList<QDomDocument> m_domList;
};

#endif // SVG_GENERATOR_H

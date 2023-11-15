/******************************************************************************
 *   @file   measurement_variable_p.h
 **  @author Douglas S Caskey
 **  @date   16 Jul, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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
 *************************************************************************/

/************************************************************************
 **
 **  @file   vmeasurement_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef MEASUREMENT_VARIABLE_P_H
#define MEASUREMENT_VARIABLE_P_H

#include <QSharedData>

#include "../vcontainer.h"
#include "../vmisc/diagnostic.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class MeasurementVariableData : public QSharedData
{
public:

    MeasurementVariableData(quint32 index, const QString &gui_text, const QString &tagName, qreal baseSize, qreal baseHeight,
                     qreal base, qreal ksize, qreal kheight )
        : data(VContainer(nullptr, nullptr)),
          index(index),
          formula(),
          gui_text(gui_text),
          _tagName(tagName),
          formulaOk(true),
          currentSize(nullptr),
          currentHeight(nullptr),
          currentUnit(nullptr),
          base(base),
          ksize(ksize),
          kheight(kheight),
          baseSize(baseSize),
          baseHeight(baseHeight)
    {}

    MeasurementVariableData(VContainer *data, quint32 index, const QString &formula, bool ok, const QString &gui_text,
                     const QString &tagName, qreal base)
        : data(*data),
          index(index),
          formula(formula),
          gui_text(gui_text),
          _tagName(tagName),
          formulaOk(ok),
          currentSize(nullptr),
          currentHeight(nullptr),
          currentUnit(nullptr),
          base(base),
          ksize(0),
          kheight(0),
          baseSize(0),
          baseHeight(0)
    {}

    MeasurementVariableData(const MeasurementVariableData &m)
        : QSharedData(m),
          data(m.data),
          index(m.index),
          formula(m.formula),
          gui_text(m.gui_text),
          _tagName(m._tagName),
          formulaOk(m.formulaOk),
          currentSize(m.currentSize),
          currentHeight(m.currentHeight),
          currentUnit(m.currentUnit),
          base(m.base),
          ksize(m.ksize),
          kheight(m.kheight),
          baseSize(m.baseSize),
          baseHeight(m.baseHeight)
    {}

    virtual    ~MeasurementVariableData();

    VContainer  data;
    quint32     index;
    QString     formula;
    QString     gui_text;
    QString     _tagName;
    bool        formulaOk;
    qreal      *currentSize;
    qreal      *currentHeight;
    const Unit *currentUnit;
    qreal       base;           //! @brief base value in base size and height */
    qreal       ksize;          //! @brief ksize increment in sizes */
    qreal       kheight;        //! @brief kgrowth increment in heights */
    qreal       baseSize;
    qreal       baseHeight;

private:
    MeasurementVariableData &operator=(const MeasurementVariableData &) Q_DECL_EQ_DELETE;
};

MeasurementVariableData::~MeasurementVariableData()
{}

QT_WARNING_POP

#endif // MEASUREMENT_VARIABLE_P_H

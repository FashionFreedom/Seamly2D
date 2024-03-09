/******************************************************************************
 *   @file   pen.h
 **  @author DS Caskey
 **  @date   Jan 14, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
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


#ifndef PEN_H
#define PEN_H

/**
 * A pen stores attributes for painting such as line weight,
 * linetype, color, ...
 */
class Pen
{
public:
/**
* Creates a default pen (black, solid, weight 0).
*/

Pen ()
{
    setColor(QString("black"));
    setLineWeight(0.00);
    setLineType(QString("solidLine"));
}

/**
* Creates a pen with the given attributes.
*/
Pen (const QString& color, const qreal &weight, const QString &type)
{
    setColor(color);
    setLineWeight(weight);
    setLineType(type);
}

virtual ~Pen () {}

QString getLineType() const
{
    return m_lineType;
}

void setLineType(const QString &type)
{
    m_lineType = type;
}

qreal getLineWeight() const
{
    return m_lineWeight;
}

void setLineWeight(const qreal &weight)
{
    m_lineWeight = weight;
}

QString getColor() const
{
    return m_color;
}

void setColor(const QString &color)
{
    m_color = color;
}

bool operator == (const Pen &pen) const
{
    return (m_lineType==pen.m_lineType && m_lineWeight==pen.m_lineWeight && m_color==pen.m_color);
}

bool operator != (const Pen &pen) const
{
    return !(*this==pen);
}

friend std::ostream &operator << (std::ostream &stream, const Pen &pen);

protected:
    QString m_color;
    qreal   m_lineWeight;
    QString m_lineType;
};

#endif

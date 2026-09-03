/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2026  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************/

#include "vlinevariablelookup.h"

#include "vlinelength.h"
#include "vlineangle.h"
#include "../vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
QSharedPointer<VLengthLine> FindLineLength(const VContainer &data, quint32 lineId)
{
    const QMap<QString, QSharedPointer<VLengthLine>> lines = data.lineLengthsData();
    for (auto i = lines.constBegin(); i != lines.constEnd(); ++i)
    {
        if (i.value()->GetLineId() == lineId)
        {
            return i.value();
        }
    }
    return QSharedPointer<VLengthLine>();
}

//---------------------------------------------------------------------------------------------------------------------
QSharedPointer<VLineAngle> FindLineAngle(const VContainer &data, quint32 lineId)
{
    const QMap<QString, QSharedPointer<VLineAngle>> angles = data.lineAnglesData();
    for (auto i = angles.constBegin(); i != angles.constEnd(); ++i)
    {
        if (i.value()->GetLineId() == lineId)
        {
            return i.value();
        }
    }
    return QSharedPointer<VLineAngle>();
}

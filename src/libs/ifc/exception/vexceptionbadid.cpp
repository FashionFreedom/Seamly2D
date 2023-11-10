/***************************************************************************
 **  @file   vexceptionbadid.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2017 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **  @file   vexceptionbadid.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "vexceptionbadid.h"

#include "vexception.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VExceptionBadId exception bad id
 * @param error string with error
 * @param id id
 */
VExceptionBadId::VExceptionBadId(const QString &error, const quint32 &id)
    : VException(error)
    , id(id)
    , key(QString()){}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VExceptionBadId exception bad id
 * @param error string with error
 * @param key string key
 */
VExceptionBadId::VExceptionBadId(const QString &error, const QString &key)
    : VException(error), id(NULL_ID), key(key){}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VExceptionBadId copy constructor
 * @param e exception
 */
VExceptionBadId::VExceptionBadId(const VExceptionBadId &error)
    : VException(error)
    , id(error.BadId())
    , key(error.BadKey()){}

//---------------------------------------------------------------------------------------------------------------------
VExceptionBadId &VExceptionBadId::operator=(const VExceptionBadId &error)
{
    if ( &error == this )
    {
        return *this;
    }
    VException::operator=(error);
    this->id = error.BadId();
    this->key = error.BadKey();
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ErrorMessage return main error message
 * @return main error message
 */
QString VExceptionBadId::ErrorMessage() const
{
    QString error;
    if (key.isEmpty())
    {
        error = QString("ExceptionBadId: %1, id = %2").arg(this->error).arg(id);
    }
    else
    {
        error = QString("ExceptionBadId: %1, id = %2").arg(this->error).arg(key);
    }
    return error;
}

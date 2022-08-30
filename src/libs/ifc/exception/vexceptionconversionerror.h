/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   vexceptionconversionerror.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VEXCEPTIONCONVERSIONERROR_H
#define VEXCEPTIONCONVERSIONERROR_H

#include <qcompilerdetection.h>
#include <QString>

#include "../ifcdef.h"
#include "vexception.h"

/**
 * @brief The VExceptionConversionError class for exception of conversion error
 */
class VExceptionConversionError : public VException
{
public:
    VExceptionConversionError(const QString &error, const QString &str);
    VExceptionConversionError(const VExceptionConversionError &e);
    VExceptionConversionError &operator=(const VExceptionConversionError &e);
    virtual         ~VExceptionConversionError() V_NOEXCEPT_EXPR (true) Q_DECL_EQ_DEFAULT;
    virtual QString ErrorMessage() const Q_DECL_OVERRIDE;
    QString         String() const;
protected:
    /** @brief str string, where the error happened */
    QString         str;
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief String return string, where the error happened
 * @return string
 */
inline QString VExceptionConversionError::String() const
{
    return str;
}

#endif // VEXCEPTIONCONVERSIONERROR_H

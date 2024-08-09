/***************************************************************************
 **  @file   vexceptionwrongparameterid.cpp
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
 **
 **  @file   vexceptionwrongparameterid.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef VEXCEPTIONWRONGID_H
#define VEXCEPTIONWRONGID_H

#include <qcompilerdetection.h>
#include <QString>
#include <QtGlobal>

#include "../ifcdef.h"
#include "vexception.h"

class QDomElement;

/**
 * @brief The VExceptionWrongId class for exception wrong id
 */
class VExceptionWrongId : public VException
{
public:
                    VExceptionWrongId(const QString &what, const QDomElement &domElement);
                    VExceptionWrongId(const VExceptionWrongId &error);
                    VExceptionWrongId &operator=(const VExceptionWrongId &error);
    virtual        ~VExceptionWrongId() V_NOEXCEPT_EXPR (true) Q_DECL_EQ_DEFAULT;
    virtual QString ErrorMessage() const Q_DECL_OVERRIDE;
    virtual QString DetailedInformation() const Q_DECL_OVERRIDE;
    QString         TagText() const;
    QString         TagName() const;
    qint32          LineNumber() const;
    
protected:
    /** @brief tagText tag text */
    QString         tagText;

    /** @brief tagName tag name */
    QString         tagName;

    /** @brief lineNumber line number */
    qint32          lineNumber;
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TagText return tag text
 * @return tag text
 */
inline QString VExceptionWrongId::TagText() const
{
    return tagText;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief TagName return tag name
 * @return tag name
 */
inline QString VExceptionWrongId::TagName() const
{
    return tagName;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LineNumber return line number in file
 * @return line number
 */
inline qint32 VExceptionWrongId::LineNumber() const
{
    return lineNumber;
}

#endif // VEXCEPTIONWRONGID_H

/******************************************************************************
 *   @file   vlabeltemplateconverter.h
 **  @author Douglas S Caskey
 **   @date   Jul 8, 2023
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
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 8, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VLABELTEMPLATECONVERTER_H
#define VLABELTEMPLATECONVERTER_H

#include "abstract_converter.h"

class VLabelTemplateConverter : public VAbstractConverter
{
public:
    explicit VLabelTemplateConverter(const QString &fileName);
    virtual ~VLabelTemplateConverter() Q_DECL_EQ_DEFAULT;

    static const QString LabelTemplateMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const int LabelTemplateMinVer = CONVERTER_VERSION_CHECK(1, 0, 0);
    static Q_DECL_CONSTEXPR const int LabelTemplateMaxVer = CONVERTER_VERSION_CHECK(1, 0, 0);

protected:
    virtual int     minVer() const Q_DECL_OVERRIDE;
    virtual int     maxVer() const Q_DECL_OVERRIDE;

    virtual QString minVerStr() const Q_DECL_OVERRIDE;
    virtual QString maxVerStr() const Q_DECL_OVERRIDE;

    virtual QString getSchema(int ver) const Q_DECL_OVERRIDE;
    virtual void    applyPatches() Q_DECL_OVERRIDE;
    virtual void    downgradeToCurrentMaxVersion() Q_DECL_OVERRIDE;

    virtual bool isReadOnly() const Q_DECL_OVERRIDE {return false;}

private:
    Q_DISABLE_COPY(VLabelTemplateConverter)
    static const QString LabelTemplateMinVerStr;
};

#endif // VLABELTEMPLATECONVERTER_H

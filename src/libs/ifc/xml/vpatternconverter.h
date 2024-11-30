/******************************************************************************
 *   @file   vpatternconverter.h
 **  @author Douglas S Caskey
 **  @date   14 Jul, 2023
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

/**************************************************************************
 **
 **  @file   vpatternconverter.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 12, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2014 Valentina project
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

#ifndef VPATTERNCONVERTER_H
#define VPATTERNCONVERTER_H

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QDomElement>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

#include "abstract_converter.h"

class QDomElement;

class VPatternConverter : public VAbstractConverter
{
    Q_DECLARE_TR_FUNCTIONS(VPatternConverter)
public:
    explicit VPatternConverter(const QString &fileName);
    virtual ~VPatternConverter() Q_DECL_EQ_DEFAULT;

    static const QString PatternMaxVerStr;
    static const QString CurrentSchema;
    static Q_DECL_CONSTEXPR const int PatternMinVer = CONVERTER_VERSION_CHECK(0, 1, 0);
    static Q_DECL_CONSTEXPR const int PatternMaxVer = CONVERTER_VERSION_CHECK(0, 7, 2);

protected:
    virtual int     minVer() const override;
    virtual int     maxVer() const override;

    virtual QString minVerStr() const override;
    virtual QString maxVerStr() const override;

    virtual QString getSchema(int ver) const override;
    virtual void    applyPatches() override;
    virtual void    downgradeToCurrentMaxVersion() override;

    virtual bool    isReadOnly() const override;

private:
    Q_DISABLE_COPY(VPatternConverter)
    static const QString PatternMinVerStr;

    void          toVersion0_1_1();
    void          toVersion0_1_2();
    void          toVersion0_1_3();
    void          toVersion0_1_4();
    void          toVersion0_2_0();
    void          toVersion0_2_1();
    void          toVersion0_2_2();
    void          toVersion0_2_3();
    void          toVersion0_2_4();
    void          toVersion0_2_5();
    void          toVersion0_2_6();
    void          toVersion0_2_7();
    void          toVersion0_3_0();
    void          toVersion0_3_1();
    void          toVersion0_3_2();
    void          toVersion0_3_3();
    void          toVersion0_3_4();
    void          toVersion0_3_5();
    void          toVersion0_3_6();
    void          toVersion0_3_7();
    void          toVersion0_3_8();
    void          toVersion0_3_9();
    void          toVersion0_4_0();
    void          toVersion0_4_1();
    void          toVersion0_4_2();
    void          toVersion0_4_3();
    void          toVersion0_4_4();
    void          toVersion0_4_5();
    void          toVersion0_4_6();
    void          toVersion0_4_7();
    void          toVersion0_4_8();
    void          toVersion0_5_0();
    void          toVersion0_5_1();
    void          toVersion0_6_0();
    void          toVersion0_6_1();
    void          toVersion0_6_2();
    void          toVersion0_6_3();
    void          toVersion0_6_4();
    void          toVersion0_6_5();
    void          toVersion0_6_6();
    void          toVersion0_6_7();
    void          toVersion0_6_8();
    void          toVersion0_6_9();
    void          toVersion0_7_0();
    void          toVersion0_7_1();
    void          toVersion0_7_2();


    void          TagUnitToV0_2_0();
    void          TagIncrementToV0_2_0();
    void          ConvertMeasurementsToV0_2_0();
    void          TagMeasurementsToV0_2_0();

    void          ConvertMeasurementsToV0_2_1();

    void          RemoveColorToolCutV0_3_1();

    QSet<QString> FixIncrementsToV0_2_0();
    QString       FixIncrementInFormulaToV0_2_0(const QString &formula, const QSet<QString> &names);
    void          FixPointExpressionsToV0_2_0(const QSet<QString> &names);
    void          FixArcExpressionsToV0_2_0(const QSet<QString> &names);
    void          FixPathPointExpressionsToV0_2_0(const QSet<QString> &names);

    void          ConvertPointExpressionsToV0_2_0(const QMap<QString, QString> &names);
    void          ConvertArcExpressionsToV0_2_0(const QMap<QString, QString> &names);
    void          ConvertPathPointExpressionsToV0_2_0(const QMap<QString, QString> &names);
    QString       FixMeasurementInFormulaToV0_2_0(const QString &formula, const QMap<QString, QString> &names);

    QString       MUnitV0_1_4() const;
    QDomElement   TagMeasurementsV0_1_4() const;
    QDomElement   TagIncrementsV0_1_4() const;

    void          FixToolUnionToV0_2_4();
    void          ParseModelingToV0_2_4(const QDomElement &modeling);
    void          SaveChildrenToolUnionToV0_2_4(quint32 id, const QVector<quint32> &children);

    static QMap<QString, QString> OldNamesToNewNames_InV0_2_0();
    static QMap<QString, QString> OldNamesToNewNames_InV0_2_1();

    void          FixCutPoint();
    void          FixSubPaths(int i, quint32 id, quint32 baseCurve);

    void          TagRemoveAttributeTypeObjectInV0_4_0();
    void          TagDetailToV0_4_0();
    void          TagUnionDetailsToV0_4_0();
    QDomElement   GetUnionDetailNodesV0_4_0(const QDomElement &detail);
    QDomElement   GetUnionChildrenNodesV0_4_0(const QDomElement &detail);

    void          LabelTagToV0_4_4(const QString &tagName);

    QDomElement   AddTagPatternLabelV0_5_1();
    void          PortPatternLabeltoV0_6_0(QDomElement &label);
    void          AddLabelTemplateLineV0_6_0(QDomElement &label, const QString &text, bool bold,
                                             bool italic, int alignment, int fontSizeIncrement);
    void          PortPieceLabelstoV0_6_0();
    void          RemoveUnusedTagsV0_6_0();
    void          RemoveUniqueTagV0_6_0(const QString &tag);
    void          renameAttribute(QDomElement &element, const QString &oldName,
                                  const QString &newName, const QString &value) const;
};

//---------------------------------------------------------------------------------------------------------------------
inline int VPatternConverter::minVer() const
{
    return PatternMinVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VPatternConverter::maxVer() const
{
    return PatternMaxVer;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VPatternConverter::minVerStr() const
{
    return PatternMinVerStr;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VPatternConverter::maxVerStr() const
{
    return PatternMaxVerStr;
}

#endif // VPATTERNCONVERTER_H

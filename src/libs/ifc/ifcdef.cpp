/******************************************************************************
*   @file   ifcdef.cpp
**  @author Douglas S Caskey
**  @date   30 Apr, 2023
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
 **  @file   ifcdef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
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

#include "ifcdef.h"

#include <QBrush>
#include <QIcon>
#include <QMap>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>

#include "../vmisc/diagnostic.h"

const QString CustomMSign    = QStringLiteral("@");
const QString CustomIncrSign = QStringLiteral("#");

const QString AttrType        = QStringLiteral("type");
const QString AttrMx          = QStringLiteral("mx");
const QString AttrMy          = QStringLiteral("my");
const QString AttrName        = QStringLiteral("name");
const QString AttrMx1         = QStringLiteral("mx1");
const QString AttrMy1         = QStringLiteral("my1");
const QString AttrName1       = QStringLiteral("name1");
const QString AttrMx2         = QStringLiteral("mx2");
const QString AttrMy2         = QStringLiteral("my2");
const QString AttrName2       = QStringLiteral("name2");
const QString AttrBaseLineP1  = QStringLiteral("baseLineP1");
const QString AttrBaseLineP2  = QStringLiteral("baseLineP2");
const QString AttrDartP1      = QStringLiteral("dartP1");
const QString AttrDartP2      = QStringLiteral("dartP2");
const QString AttrDartP3      = QStringLiteral("dartP3");
const QString AttrX           = QStringLiteral("x");
const QString AttrY           = QStringLiteral("y");
const QString AttrLineType    = QStringLiteral("lineType");
const QString AttrLineWeight  = QStringLiteral("lineWeight");
const QString AttrObjName     = QStringLiteral("objName");
const QString AttrCut         = QStringLiteral("cut");
const QString AttrLength      = QStringLiteral("length");
const QString AttrBasePoint   = QStringLiteral("basePoint");
const QString AttrFirstPoint  = QStringLiteral("firstPoint");
const QString AttrSecondPoint = QStringLiteral("secondPoint");
const QString AttrThirdPoint  = QStringLiteral("thirdPoint");
const QString AttrCenter      = QStringLiteral("center");
const QString AttrRadius      = QStringLiteral("radius");
const QString AttrRadius1     = QStringLiteral("radius1");
const QString AttrRadius2     = QStringLiteral("radius2");
const QString AttrAngle       = QStringLiteral("angle");
const QString AttrAngle1      = QStringLiteral("angle1");
const QString AttrAngle2      = QStringLiteral("angle2");
const QString AttrLength1     = QStringLiteral("length1");
const QString AttrLength2     = QStringLiteral("length2");
const QString AttrP1Line      = QStringLiteral("p1Line");
const QString AttrP2Line      = QStringLiteral("p2Line");
const QString AttrP1Line1     = QStringLiteral("p1Line1");
const QString AttrP2Line1     = QStringLiteral("p2Line1");
const QString AttrP1Line2     = QStringLiteral("p1Line2");
const QString AttrP2Line2     = QStringLiteral("p2Line2");
const QString AttrPShoulder   = QStringLiteral("pShoulder");
const QString AttrPoint1      = QStringLiteral("point1");
const QString AttrPoint2      = QStringLiteral("point2");
const QString AttrPoint3      = QStringLiteral("point3");
const QString AttrPoint4      = QStringLiteral("point4");
const QString AttrKAsm1       = QStringLiteral("kAsm1");
const QString AttrKAsm2       = QStringLiteral("kAsm2");
const QString AttrKCurve      = QStringLiteral("kCurve");
const QString AttrDuplicate   = QStringLiteral("duplicate");
const QString AttrPathPoint   = QStringLiteral("pathPoint");
const QString AttrPSpline     = QStringLiteral("pSpline");
const QString AttrAxisP1      = QStringLiteral("axisP1");
const QString AttrAxisP2      = QStringLiteral("axisP2");
const QString AttrCurve       = QStringLiteral("curve");
const QString AttrCurve1      = QStringLiteral("curve1");
const QString AttrCurve2      = QStringLiteral("curve2");
const QString AttrLineColor   = QStringLiteral("lineColor");
const QString AttrColor       = QStringLiteral("color");
const QString AttrPenStyle    = QStringLiteral("penStyle");
const QString AttrFirstArc    = QStringLiteral("firstArc");
const QString AttrSecondArc   = QStringLiteral("secondArc");
const QString AttrCrossPoint  = QStringLiteral("crossPoint");
const QString AttrVCrossPoint = QStringLiteral("vCrossPoint");
const QString AttrHCrossPoint = QStringLiteral("hCrossPoint");
const QString AttrAxisType    = QStringLiteral("axisType");
const QString AttrC1Center    = QStringLiteral("c1Center");
const QString AttrC2Center    = QStringLiteral("c2Center");
const QString AttrC1Radius    = QStringLiteral("c1Radius");
const QString AttrC2Radius    = QStringLiteral("c2Radius");
const QString AttrCCenter     = QStringLiteral("cCenter");
const QString AttrTangent     = QStringLiteral("tangent");
const QString AttrCRadius     = QStringLiteral("cRadius");
const QString AttrArc         = QStringLiteral("arc");
const QString AttrSuffix      = QStringLiteral("suffix");
const QString AttrIdObject    = QStringLiteral("idObject");
const QString AttrInLayout    = QStringLiteral("inLayout");
const QString AttrPieceLocked = QStringLiteral("locked");
const QString AttrRotationAngle   = QStringLiteral("rotationAngle");
const QString AttrClosed          = QStringLiteral("closed");
const QString AttrShowPointName   = QStringLiteral("showPointName");
const QString AttrShowPointName1  = QStringLiteral("showPointName1");
const QString AttrShowPointName2  = QStringLiteral("showPointName2");
const QString AttrAlias           = QStringLiteral("alias");

const QString LineTypeNone           = QStringLiteral("none");
const QString LineTypeSolidLine      = QStringLiteral("solidLine");
const QString LineTypeDashLine       = QStringLiteral("dashLine");
const QString LineTypeDotLine        = QStringLiteral("dotLine");
const QString LineTypeDashDotLine    = QStringLiteral("dashDotLine");
const QString LineTypeDashDotDotLine = QStringLiteral("dashDotDotLine");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LineTypeList return list of all line types.
 * @return list of all line types.
 */
QStringList LineTypes()
{
    const QStringList lineTypes = QStringList() << LineTypeNone    << LineTypeSolidLine << LineTypeDashLine
                                                << LineTypeDotLine << LineTypeDashDotLine
                                                << LineTypeDashDotDotLine;
    return lineTypes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LineStyle return pen style for current line style.
 * @return pen style.
 */
Qt::PenStyle lineTypeToPenStyle(const QString &lineType)
{
    const QStringList lineTypes = LineTypes();
    switch (lineTypes.indexOf(lineType))
    {
        case 0: // LineTypeNone
            return Qt::NoPen;
        case 2: // LineTypeDashLine
            return Qt::DashLine;
        case 3: // LineTypeDotLine
            return Qt::DotLine;
        case 4: // LineTypeDashDotLine
            return Qt::DashDotLine;
        case 5: // LineTypeDashDotDotLine
            return Qt::DashDotDotLine;
        default:
            return Qt::SolidLine;
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString PenStyleToLineType(Qt::PenStyle penStyle)
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")

    switch (penStyle)
    {
        case Qt::NoPen:
            return LineTypeNone;
        case Qt::DashLine:
            return LineTypeDashLine;
        case Qt::DotLine:
            return LineTypeDotLine;
        case Qt::DashDotLine:
            return LineTypeDashDotLine;
        case Qt::DashDotDotLine:
            return LineTypeDashDotDotLine;
        default:
            break;
    }

    QT_WARNING_POP

    return LineTypeSolidLine;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, QString> curveLineTypeList()
{
    QMap<QString, QString> map = lineTypeList();
    map.remove(LineTypeNone);
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, QString> lineTypeList()
{
    QMap<QString, QString> map;

    const QStringList lineTypeNames = LineTypes();
    for (int i = 0; i < lineTypeNames.size(); ++i)
    {
        QString name;
        switch (i)
        {
            case 1: // LineTypeSolidLine
                name = QObject::tr("Solidline");
                break;
            case 2: // LineTypeDashLine
                name = QObject::tr("Dash");
                break;
            case 3: // LineTypeDotLine
                name = QObject::tr("Dot");
                break;
            case 4: // LineTypeDashDotLine
                name = QObject::tr("Dash Dot");
                break;
            case 5: // LineTypeDashDotDotLine
                name = QObject::tr("Dash Dot Dot");
                break;
            case 0: // LineTypeNone
            default:
                name = QObject::tr("No Pen");
                break;
        }

        map.insert(lineTypeNames.at(i), name);
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, QString> lineWeightList()
{
    QMap<QString, QString> map;

    const QStringList lineWeights = QStringList() << "0"    << "0.05" << "0.09" << "0.13" << "0.15" << "0.18"
                                                  << "0.2"  << "0.25" << "0.3"  << "0.35" << "0.4"  << "0.5"
                                                  << "0.53" << "0.6"  << "0.7"  << "0.8"  << "0.9"  << "1"
                                                  << "1.06" << "1.2"  << "1.4"  << "1.58" << "2"    << "2.11";

    for (int i = 0; i < lineWeights.size(); ++i)
    {
        QString name;
        switch (i)
        {
            case 1:
                name = "0.05mm";
                break;
            case 2:
                name = "0.09";
                break;
            case 3:
                name = "0.13mm (ISO)";
                break;
            case 4:
                name = "0.15mm";
                break;
            case 5:
                name = "0.18mm (ISO)";
                break;
            case 6:
                name ="0.20mm";
                break;
            case 7:
                name = "0.25mm (ISO)";
                break;
            case 8:
                name = "0.30mm";
                break;
            case 9:
                name = "0.35mm (ISO)";
                break;
            case 10:
                name = "0.40mm";
                break;
            case 11:
                name = "0.50mm (ISO)";
                break;
            case 12:
                name ="0.53mm";
                break;
            case 13:
                name = "0.60mm";
                break;
            case 14:
                name = "0.70mm (ISO)";
                break;
            case 15:
                name = "0.80mm";
                break;
            case 16:
                name ="0.90mm";
                break;
            case 17:
                name = "1.00mm (ISO)";
                break;
            case 18:
                name = "1.06mm";
                break;
            case 19:
                name = "1.20mm";
                break;
            case 20:
                name = "1.40mm (ISO)";
                break;
            case 21:
                name = "1.58mm";
                break;
            case 22:
                name = "2.00mm (ISO)";
                break;
            case 23:
                name = "2.11mm";
                break;
            case 0:
            default:
                name = "0.00mm";
                break;
        }

        map.insert(lineWeights.at(i), name);
    }
    return map;
}
const QString LineWeightByGroup     = QStringLiteral("byGroup");
const QString LineTypeByGroup       = QStringLiteral("byGroup");
const QString ColorByGroup          = QStringLiteral("byGroup");
const QString ColorNone             = QStringLiteral("none");
const QString ColorBlack            = QStringLiteral("black");
const QString ColorGreen            = QStringLiteral("green");
const QString ColorBlue             = QStringLiteral("blue");
const QString ColorDarkRed          = QStringLiteral("darkRed");
const QString ColorDarkGreen        = QStringLiteral("darkGreen");
const QString ColorDarkBlue         = QStringLiteral("darkBlue");
const QString ColorYellow           = QStringLiteral("yellow");
const QString ColorLightSalmon      = QStringLiteral("lightsalmon");
const QString ColorGoldenRod        = QStringLiteral("goldenrod");
const QString ColorOrange           = QStringLiteral("orange");
const QString ColorDeepPink         = QStringLiteral("deeppink");
const QString ColorViolet           = QStringLiteral("violet");
const QString ColorDarkViolet       = QStringLiteral("darkviolet");
const QString ColorMediumSeaGreen   = QStringLiteral("mediumseagreen");
const QString ColorLime             = QStringLiteral("lime");
const QString ColorDeepSkyBlue      = QStringLiteral("deepskyblue");
const QString ColorCornFlowerBlue   = QStringLiteral("cornflowerblue");
const QString ColorGray             = QStringLiteral("gray");
const QString ColorDarkGray         = QStringLiteral("darkgray");
const QString ColorLightGray        = QStringLiteral("lightgray");
const QString ColorWhite            = QStringLiteral("white");

const QString FillNone              = QStringLiteral("nobrush");
const QString FillSolid             = QStringLiteral("solidpattern");
const QString FillDense1            = QStringLiteral("dense1pattern");
const QString FillDense2            = QStringLiteral("dense2pattern");
const QString FillDense3            = QStringLiteral("dense3pattern");
const QString FillDense4            = QStringLiteral("dense4pattern");
const QString FillDense5            = QStringLiteral("dense5pattern");
const QString FillDense6            = QStringLiteral("dense6pattern");
const QString FillDense7            = QStringLiteral("dense7pattern");
const QString FillHorizLines        = QStringLiteral("horpattern");
const QString FillVertLines         = QStringLiteral("verpattern");
const QString FillCross             = QStringLiteral("crosspattern");
const QString FillBackwardDiagonal  = QStringLiteral("bdiagpattern");
const QString FillForwardDiagonal   = QStringLiteral("fdiagpattern");
const QString FilldDiagonalCross    = QStringLiteral("diagcrosspattern");

//variables
const QString measurement_         = QStringLiteral("M_");
const QString variable_           = QStringLiteral("Variable_");
const QString line_                = QStringLiteral("Line_");
const QString angleLine_           = QStringLiteral("AngleLine_");
const QString spl_                 = QStringLiteral(SPL_);
const QString arc_                 = QStringLiteral(ARC_);
const QString elarc_               = QStringLiteral(ELARC_);
const QString splPath              = QStringLiteral("SplPath");
const QString radius_V             = QStringLiteral("Radius");
const QString radiusArc_           = radius_V + arc_;
const QString radius1ElArc_        = radius_V + QLatin1Char('1') + elarc_;
const QString radius2ElArc_        = radius_V + QLatin1Char('2') + elarc_;
const QString angle1_V             = QStringLiteral("Angle1");
const QString angle2_V             = QStringLiteral("Angle2");
const QString c1Length_V           = QStringLiteral("C1Length");
const QString c2Length_V           = QStringLiteral("C2Length");
const QString c1LengthSpl_         = c1Length_V + spl_;
const QString c2LengthSpl_         = c2Length_V + spl_;
const QString c1LengthSplPath      = c1Length_V + splPath;
const QString c2LengthSplPath      = c2Length_V + splPath;
const QString angle1Arc_           = angle1_V + arc_;
const QString angle2Arc_           = angle2_V + arc_;
const QString angle1ElArc_         = angle1_V + elarc_;
const QString angle2ElArc_         = angle2_V + elarc_;
const QString angle1Spl_           = angle1_V + spl_;
const QString angle2Spl_           = angle2_V + spl_;
const QString angle1SplPath        = angle1_V + splPath;
const QString angle2SplPath        = angle2_V + splPath;
const QString seg_                 = QStringLiteral("Seg_");
const QString currentLength        = QStringLiteral("CurrentLength");
const QString currentSeamAllowance = QStringLiteral("CurrentSeamAllowance");

const QStringList builInVariables = QStringList() << measurement_
                                                  << variable_
                                                  << line_
                                                  << angleLine_
                                                  << arc_
                                                  << elarc_
                                                  << spl_
                                                  << splPath
                                                  << radiusArc_
                                                  << radius1ElArc_
                                                  << radius2ElArc_
                                                  << angle1Arc_
                                                  << angle2Arc_
                                                  << angle1ElArc_
                                                  << angle2ElArc_
                                                  << angle1Spl_
                                                  << angle2Spl_
                                                  << angle1SplPath
                                                  << angle2SplPath
                                                  << seg_
                                                  << currentLength
                                                  << currentSeamAllowance
                                                  << c1LengthSpl_
                                                  << c2LengthSpl_
                                                  << c1LengthSplPath
                                                  << c2LengthSplPath;

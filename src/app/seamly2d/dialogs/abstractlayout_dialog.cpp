/******************************************************************************
 *   @file   abstractlayout_dialog.cpp
 **  @author Douglas S Caskey
 **  @date   19 Oct, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/
 #include "abstractlayout_dialog.h"

#include "../core/vapplication.h"
#include "../vmisc/def.h"

AbstractLayoutDialog::AbstractLayoutDialog(QWidget *parent) : QDialog(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LayoutSettingsDialog::TemplateSize
 * @param tmpl
 * @param unit
 * @return
 */
QSizeF AbstractLayoutDialog::getTemplateSize(const PaperSizeFormat &tmpl, const Unit &unit) const
{
    qreal width = 0;
    qreal height = 0;

    switch (tmpl)
    {
        case PaperSizeFormat::A0:
            width = UnitConvertor(841, Unit::Mm, unit);
            height = UnitConvertor(1189, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::A1:
            width = UnitConvertor(594, Unit::Mm, unit);
            height = UnitConvertor(841, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::A2:
            width = UnitConvertor(420, Unit::Mm, unit);
            height = UnitConvertor(594, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::A3:
            width = UnitConvertor(297, Unit::Mm, unit);
            height = UnitConvertor(420, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::A4:
            width = UnitConvertor(210, Unit::Mm, unit);
            height = UnitConvertor(297, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Letter:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(11, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Legal:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(14, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Tabloid:
             width = UnitConvertor(11, Unit::Inch, unit);
             height = UnitConvertor(17, Unit::Inch, unit);
             return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::AnsiC:
             width = UnitConvertor(17, Unit::Inch, unit);
             height = UnitConvertor(22, Unit::Inch, unit);
             return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::AnsiD:
             width = UnitConvertor(22, Unit::Inch, unit);
             height = UnitConvertor(34, Unit::Inch, unit);
             return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::AnsiE:
             width = UnitConvertor(34, Unit::Inch, unit);
             height = UnitConvertor(44, Unit::Inch, unit);
             return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Roll24in:
            width = UnitConvertor(24, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Roll30in:
            width = UnitConvertor(30, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Roll36in:
            width = UnitConvertor(36, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Roll42in:
            width = UnitConvertor(42, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeFormat::Roll44in:
            width = UnitConvertor(44, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        default:
            break;
    }
    return QSizeF();
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF AbstractLayoutDialog::RoundTemplateSize(qreal width, qreal height, Unit unit) const
{
    qreal w = 0;
    qreal h = 0;

    switch (unit)
    {
        case Unit::Cm:
        case Unit::Mm:
        case Unit::Px:
            w = qRound(width * 100.0) / 100.0;
            h = qRound(height * 100.0) / 100.0;
            return QSizeF(w, h);
        case Unit::Inch:
            w = qRound(width * 100000.0) / 100000.0;
            h = qRound(height * 100000.0) / 100000.0;
            return QSizeF(w, h);
        default:
            break;
    }

    return QSizeF(width, height);
}

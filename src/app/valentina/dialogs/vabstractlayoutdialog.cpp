#include "vabstractlayoutdialog.h"

#include "../core/vapplication.h"


//must be the same order as PaperSizeTemplate constants
const VAbstractLayoutDialog::FormatsVector VAbstractLayoutDialog::pageFormatNames =
        VAbstractLayoutDialog::FormatsVector () << QLatin1String("A0")
                                               << QLatin1String("A1")
                                               << QLatin1String("A2")
                                               << QLatin1String("A3")
                                               << QLatin1String("A4")
                                               << QApplication::translate("DialogLayoutSettings", "Letter")
                                               << QApplication::translate("DialogLayoutSettings", "Legal")
                                               << QApplication::translate("DialogLayoutSettings", "Roll 24in")
                                               << QApplication::translate("DialogLayoutSettings", "Roll 30in")
                                               << QApplication::translate("DialogLayoutSettings", "Roll 36in")
                                               << QApplication::translate("DialogLayoutSettings", "Roll 42in")
                                               << QApplication::translate("DialogLayoutSettings", "Roll 44in")
                                               << QApplication::translate("DialogLayoutSettings", "Custom");

//---------------------------------------------------------------------------------------------------------------------

VAbstractLayoutDialog::VAbstractLayoutDialog(QWidget *parent) : QDialog(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------

void VAbstractLayoutDialog::InitTemplates(QComboBox *comboBoxTemplates)
{
    SCASSERT(comboBoxTemplates != nullptr)
    const QIcon icoPaper("://icon/16x16/template.png");
    const QIcon icoRoll("://icon/16x16/roll.png");
    const QString pdi = QString("(%1ppi)").arg(PrintDPI);

    auto cntr = static_cast<VIndexType>(PaperSizeTemplate::A0);
    foreach(const auto& v, VAbstractLayoutDialog::pageFormatNames)
    {
        if (cntr <= static_cast<int>(PaperSizeTemplate::Legal))
        {
            comboBoxTemplates->addItem(icoPaper, v+" "+pdi, QVariant(cntr++));
        }
        else if (cntr <= static_cast<int>(PaperSizeTemplate::Roll44in))
        {
            comboBoxTemplates->addItem(icoRoll, v+" "+pdi, QVariant(cntr++));
        }
        else
        {
            comboBoxTemplates->addItem(v+" "+pdi, QVariant(cntr++));
        }
    }
    comboBoxTemplates->setCurrentIndex(-1);
}


//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogLayoutSettings::TemplateSize
 * @param tmpl
 * @param unit
 * @return
 */
QSizeF VAbstractLayoutDialog::GetTemplateSize(const PaperSizeTemplate &tmpl, const Unit &unit) const
{
    qreal width = 0;
    qreal height = 0;

    switch (tmpl)
    {
        case PaperSizeTemplate::A0:
            width = UnitConvertor(841, Unit::Mm, unit);
            height = UnitConvertor(1189, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A1:
            width = UnitConvertor(594, Unit::Mm, unit);
            height = UnitConvertor(841, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A2:
            width = UnitConvertor(420, Unit::Mm, unit);
            height = UnitConvertor(594, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A3:
            width = UnitConvertor(297, Unit::Mm, unit);
            height = UnitConvertor(420, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::A4:
            width = UnitConvertor(210, Unit::Mm, unit);
            height = UnitConvertor(297, Unit::Mm, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Letter:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(11, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Legal:
            width = UnitConvertor(8.5, Unit::Inch, unit);
            height = UnitConvertor(14, Unit::Inch, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll24in:
            width = UnitConvertor(24, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll30in:
            width = UnitConvertor(30, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll36in:
            width = UnitConvertor(36, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll42in:
            width = UnitConvertor(42, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        case PaperSizeTemplate::Roll44in:
            width = UnitConvertor(44, Unit::Inch, unit);
            height = UnitConvertor(QIMAGE_MAX, Unit::Px, unit);
            return RoundTemplateSize(width, height, unit);
        default:
            break;
    }
    return QSizeF();
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VAbstractLayoutDialog::RoundTemplateSize(qreal width, qreal height, Unit unit) const
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



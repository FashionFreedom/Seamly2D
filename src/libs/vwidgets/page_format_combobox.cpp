/******************************************************************************
 *   @file   page_format_combobox.cpp
 **  @author DS Caskey
 **  @date   19, Oct, 2023
  **
  **  @brief
  **  @copyright
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
  *****************************************************************************/

#include <QAbstractItemView>
#include <Qt>
#include <QDebug>
#include <QVariant>

#include "page_format_combobox.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

/**
 * Constructor with name.
 */
PageFormatCombobox::PageFormatCombobox(QWidget *parent, const char *name)
		: QComboBox(parent)
      	, m_currentFormat()
{
		 setObjectName(name);
		 setEditable (false);
		 init();
}

/**
 * Destructor
 */
PageFormatCombobox::~PageFormatCombobox() {}

/**
 * Initialisation called from constructor or manually but only once.
 */
void PageFormatCombobox::init()
{
    this->blockSignals(true);

    const QIcon icoPaper("://icon/16x16/template.png");
    const QIcon icoRoll("://icon/16x16/roll.png");

    addItem(QIcon(icoPaper), tr("A0"),        QVariant(static_cast<int>(PaperSizeFormat::A0)));
    addItem(QIcon(icoPaper), tr("A1"),        QVariant(static_cast<int>(PaperSizeFormat::A1)));
    addItem(QIcon(icoPaper), tr("A2"),        QVariant(static_cast<int>(PaperSizeFormat::A2)));
    addItem(QIcon(icoPaper), tr("A3"),        QVariant(static_cast<int>(PaperSizeFormat::A3)));
    addItem(QIcon(icoPaper), tr("A4"),        QVariant(static_cast<int>(PaperSizeFormat::A4)));
    addItem(QIcon(icoPaper), tr("Letter"),    QVariant(static_cast<int>(PaperSizeFormat::Letter)));
    addItem(QIcon(icoPaper), tr("Legal"),     QVariant(static_cast<int>(PaperSizeFormat::Legal)));
    addItem(QIcon(icoPaper), tr("Tabloid"),   QVariant(static_cast<int>(PaperSizeFormat::Tabloid)));
    addItem(QIcon(icoPaper), tr("ANSI C"),    QVariant(static_cast<int>(PaperSizeFormat::AnsiC)));
    addItem(QIcon(icoPaper), tr("ANSI D"),    QVariant(static_cast<int>(PaperSizeFormat::AnsiD)));
    addItem(QIcon(icoPaper), tr("ANSI E"),    QVariant(static_cast<int>(PaperSizeFormat::AnsiE)));
    addItem(QIcon(icoRoll),  tr("Roll 24in"), QVariant(static_cast<int>(PaperSizeFormat::Roll24in)));
    addItem(QIcon(icoRoll),  tr("Roll 30in"), QVariant(static_cast<int>(PaperSizeFormat::Roll30in)));
    addItem(QIcon(icoRoll),  tr("Roll 36in"), QVariant(static_cast<int>(PaperSizeFormat::Roll36in)));
    addItem(QIcon(icoRoll),  tr("Roll 42in"), QVariant(static_cast<int>(PaperSizeFormat::Roll42in)));
    addItem(QIcon(icoRoll),  tr("Roll 44in"), QVariant(static_cast<int>(PaperSizeFormat::Roll44in)));
    addItem(QIcon(),         tr("Custom"),    QVariant(static_cast<int>(PaperSizeFormat::Custom)));

    setCurrentIndex(-1);
    setMaxVisibleItems(17);
    this->view()->setTextElideMode(Qt::ElideNone);

    this->blockSignals(false);
		connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PageFormatCombobox::updatePageFormat);

    updatePageFormat(currentIndex());
}

PaperSizeFormat PageFormatCombobox::getPageFormat() const
{
    return m_currentFormat;
}

/**
 * Sets the currently selected format item to the given format.
 */
void PageFormatCombobox::setPageFormat(PaperSizeFormat &format)
{
    m_currentFormat = format;

    setCurrentIndex(findData(QVariant(static_cast<int>(format))));

    if (currentIndex()!= count() -1 )
    {
        updatePageFormat(currentIndex());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<std::pair<QString, PaperSizeFormat>> PageFormatCombobox::initFormats()
{
    QVector<std::pair<QString, PaperSizeFormat>> list;

    auto InitFormat = [&list](PaperSizeFormat format)
    {
        list.append(std::make_pair(getFormatName(format), format));
    };

    InitFormat(PaperSizeFormat::A0);
    InitFormat(PaperSizeFormat::A1);
    InitFormat(PaperSizeFormat::A2);
    InitFormat(PaperSizeFormat::A3);
    InitFormat(PaperSizeFormat::A4);
    InitFormat(PaperSizeFormat::Letter);
    InitFormat(PaperSizeFormat::Legal);
    InitFormat(PaperSizeFormat::Tabloid);
    InitFormat(PaperSizeFormat::AnsiC);
    InitFormat(PaperSizeFormat::AnsiD);
    InitFormat(PaperSizeFormat::AnsiE);
    InitFormat(PaperSizeFormat::Roll24in);
    InitFormat(PaperSizeFormat::Roll30in);
    InitFormat(PaperSizeFormat::Roll36in);
    InitFormat(PaperSizeFormat::Roll42in);
    InitFormat(PaperSizeFormat::Roll44in);
    InitFormat(PaperSizeFormat::Custom);
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
QString PageFormatCombobox::getFormatName(PaperSizeFormat format)
{
    switch(format)
    {
        case PaperSizeFormat::A0:
            return QString("A0");
        case PaperSizeFormat::A1:
            return QString("A1");
        case PaperSizeFormat::A2:
            return QString("A2");
        case PaperSizeFormat::A3:
            return QString("A3");
        case PaperSizeFormat::A4:
            return QString("A4");
        case PaperSizeFormat::Letter:
            return QString("Letter");
        case PaperSizeFormat::Legal:
            return QString("Legal");
        case PaperSizeFormat::Tabloid:
            return QString("Tabloid");
        case PaperSizeFormat::AnsiC:
            return QString("ANSI C");
        case PaperSizeFormat::AnsiD:
            return QString("ANSI D");
        case PaperSizeFormat::AnsiE:
            return QString("ANSI E");
        case PaperSizeFormat::Roll24in:
            return QString("Roll 24in");
        case PaperSizeFormat::Roll30in:
            return QString("Roll 30in");
        case PaperSizeFormat::Roll36in:
            return QString("Roll 36in");
        case PaperSizeFormat::Roll42in:
            return QString("Roll 42in");
        case PaperSizeFormat::Roll44in:
            return QString("Roll 44in");
        case PaperSizeFormat::Custom:
            return QString("Custom");
        default:
            return QString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString PageFormatCombobox::MakeHelpTemplateList()
{
    QString out = "\n";

    auto cntr = static_cast<int>(PaperSizeFormat::A0);
    foreach(auto& v,  initFormats())
    {
        if (cntr <= static_cast<int>(PaperSizeFormat::Roll44in))// Don't include custom template
        {
            out += QLatin1String("\t") + v.first + QLatin1String(" = ") +
                   QString::number(static_cast<int>(v.second)) + QLatin1String("\n");
        }
    }
    return out;
}

/**
 * Called when the width has changed. This method sets the current width to the value chosen or even
 * offers a dialog to the user that allows him/ her to choose an individual width.
 */
void PageFormatCombobox::updatePageFormat(int index)
{
    QVariant format = itemData(index);
    if(format != QVariant::Invalid )
    {
       m_currentFormat = static_cast<PaperSizeFormat>(this->currentData().toInt());
    }

    emit pageFormatChanged(m_currentFormat);
}

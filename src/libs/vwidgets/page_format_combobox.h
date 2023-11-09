/******************************************************************************
 *   @file   page_format_combobox.h
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


#ifndef PAGE_FORMAT_COMBOBOX_H
#define PAGE_FORMAT_COMBOBOX_H

#include <QComboBox>
#include <QWidget>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"

/**
 * A comboBox for choosing an export format type.
 */
class PageFormatCombobox: public QComboBox
{
    Q_OBJECT

public:
                        PageFormatCombobox(QWidget *parent = nullptr, const char *name = nullptr);
    virtual            ~PageFormatCombobox();

    void                init();
    PaperSizeFormat     getPageFormat() const;
    void                setPageFormat(PaperSizeFormat &format);
    static QString      MakeHelpTemplateList();

private slots:
    void                updatePageFormat(int index);

signals:
    void                pageFormatChanged(const PaperSizeFormat &format);

private:
    static QVector<std::pair<QString, PaperSizeFormat>> initFormats();
    static QString      getFormatName(PaperSizeFormat format);
    PaperSizeFormat     m_currentFormat;
};

#endif //PAGE_FORMAT_COMBOBOX_H

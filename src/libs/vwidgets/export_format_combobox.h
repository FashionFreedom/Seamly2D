///---------------------------------------------------------------------------------------------------------------------
/// @file   export_format_combobox.h
/// @author DS Caskey
/// @date   @date 11 Nov, 2026
///
/// @brief
/// @copyright
/// This source code is part of the Seamly2D project, a pattern making
/// program, whose allow create and modeling patterns of clothing.
/// Copyright (C) 2013-2026 Seamly2D project
/// <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
///
/// Seamly2D is free software: you can redistribute it and/or modify
/// it under the terms of the GNU General Public License as published by
/// the Free Software Foundation, either version 3 of the License, or
/// (at your option) any later version.
//
/// Seamly2D is distributed in the hope that it will be useful,
/// but WITHOUT ANY WARRANTY; without even the implied warranty of
/// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
/// GNU General Public License for more details.
///
/// You should have received a copy of the GNU General Public License
/// along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef EXPORT_FORMAT_COMBOBOX_H
#define EXPORT_FORMAT_COMBOBOX_H

#include <QComboBox>
#include <QWidget>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"

class ExportFormatCombobox: public QComboBox
{
    Q_OBJECT

public:
                          ExportFormatCombobox(QWidget *parent = nullptr, const char *name = nullptr);
    virtual              ~ExportFormatCombobox();

    void                  init();

    LayoutExportFormat    getExportFormat() const;
    void                  setExportFormat(LayoutExportFormat &format);
    static QString        exportFormatDescription(LayoutExportFormat format, bool is_source = false);
    static QString        makeHelpFormatList();
    static QList<LayoutExportFormat> initFormatTypes();
    QString               getCurrentSource();
    void                  setCurrentToDefault();

private slots:
    void                  updateExportFormat(int index);

signals:
    void                  exportFormatChanged(const LayoutExportFormat &format);

private:
    static bool           supportPSTest();
    static bool           testPdf();
    void                  initFormats();
    static bool           have_pdf;
    static bool           tested;
    LayoutExportFormat    m_current_format_type;
    QVector<ExportFormat> m_formats;
};

#endif //EXPORT_FORMAT_COMBOBOX_H

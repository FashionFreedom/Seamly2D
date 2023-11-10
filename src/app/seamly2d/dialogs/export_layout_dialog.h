/************************************************************************
 **
 **  @file   export_layout_dialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @author Douglas S Caskey
 **  @date   Nov 4, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2022 Seamly2D project
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

#ifndef EXPORT_LAYOUT_DIALOG_H
#define EXPORT_LAYOUT_DIALOG_H

#include <QDialog>
#include <QMarginsF>

#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "abstractlayout_dialog.h"

namespace Ui
{
    class ExportLayoutDialog;
}


class ExportLayoutDialog : public  AbstractLayoutDialog
{
    Q_OBJECT

public:
    explicit              ExportLayoutDialog(int count, Draw mode = Draw::Layout, const QString &fileName = QString(),
                                           QWidget *parent = nullptr);

    virtual              ~ExportLayoutDialog();

    QString               path() const;
    QString               fileName() const;
    QString               modeString() const;

    LayoutExportFormat    format() const;
    QString               formatText() const;
    void                  selectFormat(LayoutExportFormat format);

    void                  setBinaryDXFFormat(bool binary);
    bool                  isBinaryDXFFormat() const;

    void                  setDestinationPath(const QString& cmdDestinationPath);

    Draw                  mode() const;

    static QString        exportFormatSuffix(LayoutExportFormat format);

    bool                  isTextAsPaths() const;
    void                  setTextAsPaths(bool textAsPaths);

protected:
    virtual void          showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void                  initTemplates(QComboBox *templates);

private slots:
    void                  save();
    void                  pathChanged(const QString &text);
    void                  showExportFiles();

private:
    Q_DISABLE_COPY(ExportLayoutDialog)
    Ui::ExportLayoutDialog *ui;
    int                   m_count;
    bool                  m_isInitialized;
    Draw                  m_mode;
    QPushButton          *m_SaveButton;

    void                  removeFormatFromList(LayoutExportFormat format);

    void                  readSettings();
    void                  writeSettings() const;
};

#endif // EXPORT_LAYOUT_DIALOG_H

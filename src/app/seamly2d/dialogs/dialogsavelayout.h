/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
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
 **************************************************************************

 ************************************************************************
 **
 **  @file   dialogsavelayout.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef DIALOGSAVELAYOUT_H
#define DIALOGSAVELAYOUT_H

#include <QDialog>
#include <QMarginsF>

#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "vabstractlayoutdialog.h"

namespace Ui
{
    class DialogSaveLAyout;
}


class DialogSaveLayout : public  VAbstractLayoutDialog
{
    Q_OBJECT

public:
    explicit DialogSaveLayout(int count, Draw mode = Draw::Layout, const QString &fileName = QString(),
                              QWidget *parent = nullptr);
    virtual ~DialogSaveLayout();

    QString            Path() const;
    QString            FileName() const;

    LayoutExportFormat Format() const;
    QString            formatText() const;
    void               SelectFormat(LayoutExportFormat format);

    void               SetBinaryDXFFormat(bool binary);
    bool               IsBinaryDXFFormat() const;


    void               SetDestinationPath(const QString& cmdDestinationPath);

    Draw               Mode() const;


    static QString exportFormatSuffix(LayoutExportFormat format);

    bool IsTextAsPaths() const;
    void SetTextAsPaths(bool textAsPaths);

protected:
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    void InitTemplates(QComboBox *comboBoxTemplates);

private slots:
    void Save();
    void PathChanged(const QString &text);
    void ShowExample();


private:
    Q_DISABLE_COPY(DialogSaveLayout)
    Ui::DialogSaveLAyout *ui;
    int count;
    bool isInitialized;
    Draw m_mode;

    void RemoveFormatFromList(LayoutExportFormat format);

    void ReadSettings();
    void WriteSettings() const;
};

#endif // DIALOGSAVELAYOUT_H

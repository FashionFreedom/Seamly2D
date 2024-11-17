//---------------------------------------------------------------------------------------------------------------------
//  @file   editlabeltemplate_dialog.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2022 Seamly, LLC
//  https://github.com/fashionfreedom/seamly2d
//
//  @brief
//  Seamly2D is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Seamly2D is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Seamly2D. if not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogeditlabel.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   11 8, 2017
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2017 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------


#ifndef EDITLABELTEMPLATE_DIALOG_H
#define EDITLABELTEMPLATE_DIALOG_H

#include <QDialog>
#include <QMap>

namespace Ui
{
    class EditLabelTemplateDialog;
}

struct VLabelTemplateLine;

class QMenu;
class VAbstractPattern;
class VPiece;

class EditLabelTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditLabelTemplateDialog(VAbstractPattern *doc, QWidget *parent = nullptr);
    virtual ~EditLabelTemplateDialog();

    QVector<VLabelTemplateLine> GetTemplate() const;
    void SetTemplate(const QVector<VLabelTemplateLine> &lines);

    void SetPiece(const VPiece &piece);

private slots:
    void ShowLineDetails();
    void AddLine();
    void RemoveLine();
    void SaveLineText(const QString &text);
    void SaveFontStyle(bool checked);
    void SaveTextFormating(bool checked);
    void NewTemplate();
    void ExportTemplate();
    void ImportTemplate();
    void InsertPlaceholder();
    void TabChanged(int index);
    void SaveAdditionalFontSize(int i);

private:
    Q_DISABLE_COPY(EditLabelTemplateDialog)
    Ui::EditLabelTemplateDialog *ui;
    QMenu               *m_placeholdersMenu;
    VAbstractPattern    *m_doc;

    QMap<QString, QPair<QString, QString>> m_placeholders;

    void SetupControls();
    void InitPlaceholdersMenu();
    void InitPlaceholders();

    QString ReplacePlaceholders(QString line) const;

    void InitPreviewLines(const QVector<VLabelTemplateLine> &lines);
};

#endif // EDITLABELTEMPLATE_DIALOG_H

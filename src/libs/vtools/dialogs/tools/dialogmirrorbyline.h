/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file   dialogmirrorbyline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#ifndef DIALOGMIRRORBYLINE_H
#define DIALOGMIRRORBYLINE_H

#include "dialogtool.h"

#include <qcompilerdetection.h>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../../tools/drawTools/operation/vabstractoperation.h"

namespace Ui
{
    class DialogMirrorByLine;
}

class DialogMirrorByLine : public DialogTool
{
    Q_OBJECT

public:
    explicit               DialogMirrorByLine(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual               ~DialogMirrorByLine();

    quint32                getFirstLinePointId() const;
    void                   setFirstLinePointId(quint32 value);

    quint32                getSecondLinePointId() const;
    void                   setSecondLinePointId(quint32 value);

    QString                getSuffix() const;
    void                   setSuffix(const QString &value);

    QVector<SourceItem>    getSourceObjects() const;
    void                   setSourceObjects(const QVector<SourceItem> &value);

    virtual void           ShowDialog(bool click) Q_DECL_OVERRIDE;

public slots:
    virtual void           ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    virtual void           SelectedObject(bool selected, quint32 id, quint32 tool) Q_DECL_OVERRIDE;

private slots:
    void                   suffixChanged();

protected:
    virtual void           CheckState() Q_DECL_FINAL;
    virtual void           ShowVisualization() Q_DECL_OVERRIDE;

    /** @brief SaveData Put dialog data in local variables */
    virtual void           SaveData() Q_DECL_OVERRIDE;

private slots:
    void                   pointChanged();

private:
    Q_DISABLE_COPY(DialogMirrorByLine)

    Ui::DialogMirrorByLine *ui;

    QVector<SourceItem>     m_objects;

    bool                    stage1;

    QString                 m_suffix;
};

#endif // DIALOGMIRRORBYLINE_H

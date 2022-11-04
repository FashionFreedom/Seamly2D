/**************************************************************************
 **
 **  @file   point_intersectxy_dialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @author Douglas S Caskey
 **  @date   7.21.2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
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

#ifndef POINT_INTERSECTXY_DIALOG_H
#define POINT_INTERSECTXY_DIALOG_H

#include "dialogtool.h"
#include "../vmisc/def.h"

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

namespace Ui
{
    class PointIntersectXYDialog;
}

/**
 * @brief The PointIntersectXYDialog class dialog for ToolPointOfIntersection. Help create point and edit option.
 */
class PointIntersectXYDialog : public DialogTool
{
    Q_OBJECT

public:
                   PointIntersectXYDialog(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual       ~PointIntersectXYDialog() Q_DECL_OVERRIDE;

    void           setPointName(const QString &value);

    quint32        getFirstPointId() const;
    void           setFirstPointId(const quint32 &value);

    quint32        getSecondPointId() const;
    void           setSecondPointId(const quint32 &value);

    QString        getLineType() const;
    void           setLineType(const QString &value);

    QString        getLineWeight() const;
    void           setLineWeight(const QString &value);

    QString        getLineColor() const;
    void           setLineColor(const QString &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    void           pointChanged();

protected:
    virtual void   ShowVisualization() Q_DECL_OVERRIDE;
    virtual void   SaveData() Q_DECL_OVERRIDE;

private:
                   Q_DISABLE_COPY(PointIntersectXYDialog)

    Ui::PointIntersectXYDialog *ui; /** @brief ui keeps information about user interface */
};

#endif // POINT_INTERSECTXY_DIALOG_H

//---------------------------------------------------------------------------------------------------------------------
//  @file   basepoint_dialog.h
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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
//  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogsinglepoint.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   15 Nov, 2013
//
//  @copyright
//  Copyright (C) 2013 Valentina project.
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published
//  by the Free Software Foundation, either version 3 of the License,
//  or (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef BASEPOINT_DIALOG_H
#define BASEPOINT_DIALOG_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "dialogtool.h"

namespace Ui
{
    class BasePointDialog;
}

class BasePointDialog : public DialogTool
{
    Q_OBJECT
public:
                         BasePointDialog(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual             ~BasePointDialog() override;

    void                 SetData(const QString &name, const QPointF &point);
    QPointF              GetPoint()const;

public slots:
    void                 mousePress(const QPointF &scenePos);

protected:
    virtual void         SaveData() override; ///@brief SaveData Put dialog data in local variables

private:
    Q_DISABLE_COPY(BasePointDialog)

    Ui::BasePointDialog *ui;      ///@brief ui keeps information about user interface
    QPointF              m_point; ///@brief point data of point
};

#endif // BASEPOINT_DIALOG_H

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogtriangle.cpp
//  @author Douglas S Caskey
//  @date   14 Aug, 2024
//
//  @copyright
//  Copyright (C) 2017 - 2026 Seamly, LLC
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
//  @file   dialogtriangle.cpp
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

#ifndef DIALOGTRIANGLE_H
#define DIALOGTRIANGLE_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogTriangle;
}

class DialogTriangle : public DialogTool
{
    Q_OBJECT

public:
                   DialogTriangle(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual       ~DialogTriangle() override;

    quint32        GetAxisP1Id() const;
    void           SetAxisP1Id(const quint32 &value);

    quint32        GetAxisP2Id() const;
    void           SetAxisP2Id(const quint32 &value);

    quint32        GetFirstPointId() const;
    void           SetFirstPointId(const quint32 &value);

    quint32        GetSecondPointId() const;
    void           SetSecondPointId(const quint32 &value);

    void           SetPointName(const QString &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) override;
    virtual void   PointNameChanged() override;

protected:
    virtual void   ShowVisualization() override;
    virtual void   SaveData() override; /// @brief SaveData Put dialog data in local variables

private:
    Q_DISABLE_COPY(DialogTriangle)

    Ui::DialogTriangle *ui;            ///@brief ui keeps information about user interface
};

#endif // DIALOGTRIANGLE_H

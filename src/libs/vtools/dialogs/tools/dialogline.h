//---------------------------------------------------------------------------------------------------------------------
// @file   dialogline.cpp
// @author Douglas S Caskey
// @date   21 Mar, 2023
//
// @copyright
// Copyright (C) 2017 - 2026 Seamly, LLC
// https://github.com/fashionfreedom/seamly2d
//
// @brief
// Seamly2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Seamly2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

// ---------------------------------------------------------------------------------------------------------------------
//  @file   dialogline.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
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

#ifndef DIALOGLINE_H
#define DIALOGLINE_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogLine;
}

class DialogLine : public DialogTool
{
    Q_OBJECT
public:
                   DialogLine(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual       ~DialogLine() override;

    quint32        getFirstPoint() const;
    void           setFirstPoint(const quint32 &value);

    quint32        getSecondPoint() const;
    void           setSecondPoint(const quint32 &value);

    void           setLineName();

    QString        getLineType() const;
    void           setLineType(const QString &value);

    QString        getLineWeight() const;
    void           setLineWeight(const QString &value);

    QString        getLineColor() const;
    void           setLineColor(const QString &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) override;
    virtual void   PointNameChanged() override;

protected:
    virtual void   ShowVisualization() override;
    virtual void   SaveData() override; /// SaveData Put dialog data in local variables

private:
    Q_DISABLE_COPY(DialogLine)
    Ui::DialogLine *ui; ///@brief ui keeps information about user interface
};

#endif // DIALOGLINE_H

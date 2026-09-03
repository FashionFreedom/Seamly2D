//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogpointofcontact.cpp
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
//  @file   dialogpointofcontact.cpp
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

#ifndef DIALOGPOINTOFCONTACT_H
#define DIALOGPOINTOFCONTACT_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"
#include "ui_dialogpointofcontact.h"

namespace Ui
{
    class DialogPointOfContact;
}

class DialogPointOfContact : public DialogTool
{
    Q_OBJECT
public:
                   DialogPointOfContact(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual       ~DialogPointOfContact() override;

    void           SetPointName(const QString &value);

    QString        getRadius() const;
    void           setRadius(const QString &value);

    quint32        getCenter() const;
    void           setCenter(const quint32 &value);

    quint32        GetFirstPoint() const;
    void           SetFirstPoint(const quint32 &value);

    quint32        GetSecondPoint() const;
    void           SetSecondPoint(const quint32 &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) override;
    void           FormulaTextChanged(); /// @brief FormulaTextChanged when formula text changes for validation and calc
    virtual void   PointNameChanged() override;
    void           FXRadius();

protected:
    virtual void   ShowVisualization() override;
    virtual void   SaveData() override; /// @brief SaveData Put dialog data in local variables
    virtual void   closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(DialogPointOfContact)

    Ui::DialogPointOfContact *ui; /// @brief ui keeps information about user interface
    QString        radius;        /// @brief radius radius of arc 
};

#endif // DIALOGPOINTOFCONTACT_H

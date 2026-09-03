//---------------------------------------------------------------------------------------------------------------------
//  @file   dialoglineintersectaxis.cpp
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
//  @file   dialoglineintersectaxis.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   19 Oct, 2014
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

#ifndef DIALOGLINEINTERSECTAXIS_H
#define DIALOGLINEINTERSECTAXIS_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogLineIntersectAxis;
}

class DialogLineIntersectAxis : public DialogTool
{
    Q_OBJECT

public:
                 DialogLineIntersectAxis(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual     ~DialogLineIntersectAxis() override;

    void         SetPointName(const QString &value);

    QString      getLineType() const;
    void         setLineType(const QString &value);

    QString      getLineWeight() const;
    void         setLineWeight(const QString &value);

    QString      getLineColor() const;
    void         setLineColor(const QString &value);

    QString      GetAngle() const;
    void         SetAngle(const QString &value);

    quint32      GetBasePointId() const;
    void         SetBasePointId(const quint32 &value);

    quint32      GetFirstPointId() const;
    void         SetFirstPointId(const quint32 &value);

    quint32      GetSecondPointId() const;
    void         SetSecondPointId(const quint32 &value);

    virtual void ShowDialog(bool click) override;

public slots:
    virtual void ChosenObject(quint32 id, const SceneObject &type) override;
    void         EvalAngle();
    void         AngleTextChanged();
    virtual void PointNameChanged() override;

    void         FXAngle();

protected:
    virtual void ShowVisualization() override;
    virtual void SaveData() override; ///@brief SaveData Put dialog data in local variables
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(DialogLineIntersectAxis)
    Ui::DialogLineIntersectAxis *ui;
    QString     formulaAngle;
    bool        m_firstRelease;
};

#endif // DIALOGLINEINTERSECTAXIS_H

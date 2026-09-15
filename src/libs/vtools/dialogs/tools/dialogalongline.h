//-----------------------------------------------------------------------------
//  @file   dialogalongline.h
//  @author Douglas S Caskey
//  @date   30 Apr, 2023
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
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  @file   dialogalongline.h
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
//-----------------------------------------------------------------------------

#ifndef DIALOGALONGLINE_H
#define DIALOGALONGLINE_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogAlongLine;
}

/**
 * @brief The DialogAlongLine class dialog for ToolAlongLine. Help create point and edit option.
 */
class DialogAlongLine : public DialogTool
{
    Q_OBJECT
public:
    DialogAlongLine(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual ~DialogAlongLine() override;

    void                SetPointName(const QString &value);

    QString             getLineType() const;
    void                setLineType(const QString &value);

    QString             getLineWeight() const;
    void                setLineWeight(const QString &value);

    QString             getLineColor() const;
    void                setLineColor(const QString &value);

    QString             GetFormula() const;
    void                SetFormula(const QString &value);

    quint32             GetFirstPointId() const;
    void                SetFirstPointId(const quint32 &value);

    quint32             GetSecondPointId() const;
    void                SetSecondPointId(const quint32 &value);

    virtual void        Build(const Tool &type) override;
public slots:
    virtual void        ChosenObject(quint32 id, const SceneObject &type) override;

    /**
     * @brief FormulaTextChanged when formula text changes for validation and calc
     */
    void                FormulaTextChanged();
    void                PointChanged();

    void                FXLength();

protected:
    virtual void        ShowVisualization() override;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void        SaveData() override;
    virtual void        closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(DialogAlongLine)

    /** @brief ui keeps information about user interface */
    Ui::DialogAlongLine *ui;

    /** @brief formula formula */
    QString             formula;

    bool buildMidpoint;

    void SetCurrentLength();
};

#endif // DIALOGALONGLINE_H

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
 **  @file   dialogrotation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
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

#ifndef DIALOGROTATION_H
#define DIALOGROTATION_H

#include <qcompilerdetection.h>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogRotation;
}

class DialogRotation : public DialogTool
{
    Q_OBJECT
public:
    explicit            DialogRotation(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual            ~DialogRotation();

    quint32             getRotationPointId() const;
    void                setRotationPointId(const quint32 &value);

    QString             GetAngle() const;
    void                SetAngle(const QString &value);

    QString             getSuffix() const;
    void                setSuffix(const QString &value);

    QVector<quint32>    getObjects() const;

    virtual void        ShowDialog(bool click) Q_DECL_OVERRIDE;

public slots:
    virtual void        ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    virtual void        SelectedObject(bool selected, quint32 object, quint32 tool) Q_DECL_OVERRIDE;

private slots:
    void                angleChanged();
    void                editAngleFormula();
    void                suffixChanged();

protected:
    virtual void        CheckState() Q_DECL_FINAL;
    virtual void        ShowVisualization() Q_DECL_OVERRIDE;

    /** @brief SaveData Put dialog data in local variables */
    virtual void        SaveData() Q_DECL_OVERRIDE;
    virtual void        closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private slots:
    void                pointChanged();

private:
    Q_DISABLE_COPY(DialogRotation)
    Ui::DialogRotation *ui;
    bool                angleFlag;        //! @brief angleFlag true if value of angle is correct */
    QTimer             *angleTimer;       //! @brief angleTimer timer of check formula of angle */
    QString             angleFormula;     //! @brief angle formula of angle
    QList<quint32>      objects;
    bool                stage1;
    QString             m_suffix;
    bool                m_firstRelease;
    
    void                evaluateAngle();
};

#endif // DIALOGROTATION_H

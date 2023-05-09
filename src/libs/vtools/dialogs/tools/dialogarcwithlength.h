/******************************************************************************
 *   @file   dialogarcwithlength.h
 **  @author Douglas S Caskey
 **  @date   21 Mar, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *****************************************************************************/

/************************************************************************
 **
 **  @file   dialogarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Seamly2D project
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

#ifndef DIALOGARCWITHLENGTH_H
#define DIALOGARCWITHLENGTH_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "dialogtool.h"
#include "../vgeometry/varc.h"
#include "../vmisc/def.h"

namespace Ui
{
    class DialogArcWithLength;
}

class DialogArcWithLength : public DialogTool
{
    Q_OBJECT

public:
    DialogArcWithLength(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    ~DialogArcWithLength();

    VArc          getArc() const;
    void          setArc(const VArc &arc);

    quint32       GetCenter() const;
    void          SetCenter(const quint32 &value);

    QString       GetRadius() const;
    void          SetRadius(const QString &value);

    QString       GetF1() const;
    void          SetF1(const QString &value);

    QString       GetLength() const;
    void          SetLength(const QString &value);

    QString       getPenStyle() const;
    void          setPenStyle(const QString &value);

    QString       getLineWeight() const;
    void          setLineWeight(const QString &value);

    QString       getLineColor() const;
    void          setLineColor(const QString &value);

public slots:
    virtual void  ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    /**
     * @brief DeployFormulaTextEdit grow or shrink formula input
     */
    void          DeployRadiusTextEdit();
    void          DeployF1TextEdit();
    void          DeployLengthTextEdit();

    void          RadiusChanged();
    void          F1Changed();
    void          LengthChanged();

    void          FXRadius();
    void          FXF1();
    void          FXLength();

protected:
    void          pointNameChanged();
    virtual void  CheckState() Q_DECL_FINAL;
    virtual void  ShowVisualization() Q_DECL_OVERRIDE;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void  SaveData() Q_DECL_OVERRIDE;
    virtual void  closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DialogArcWithLength)
    Ui::DialogArcWithLength *ui;

    /** @brief flagRadius true if value of radius is correct */
    bool          flagRadius;

    /** @brief flagF1 true if value of first angle is correct */
    bool          flagF1;

    bool          flagLength;

    /** @brief timerRadius timer of check formula of radius */
    QTimer        *timerRadius;

    /** @brief timerF1 timer of check formula of first angle */
    QTimer        *timerF1;

    QTimer        *timerLength;

    /** @brief radius formula of radius */
    QString       radius;

    /** @brief f1 formula of first angle */
    QString       f1;

    QString       length;

    /** @brief formulaBaseHeight base height defined by dialogui */
    int           formulaBaseHeightRadius;
    int           formulaBaseHeightF1;
    int           formulaBaseHeightLength;

    qreal         angleF1;
    VArc          m_arc;
    qint32        m_Id;
    qint32        newDuplicate;

    void          Radius();
    void          Length();
    void          EvalF();
};

#endif // DIALOGARCWITHLENGTH_H

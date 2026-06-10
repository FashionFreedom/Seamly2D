//-----------------------------------------------------------------------------
//  @file   dialogarc.h
//  @author Douglas S Caskey
//  @date   21 Mar, 2023
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
//  @file   dialogarc.h
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

#ifndef DIALOG_ARC_H
#define DIALOG_ARC_H

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
    class DialogArc;
}

class DialogArc : public DialogTool
{
    Q_OBJECT
public:
                  DialogArc(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual      ~DialogArc() override;

    VArc          getArc() const;
    void          setArc(const VArc &arc);

    quint32       getCenter() const;
    void          setCenter(const quint32 &value);

    QString       getRadius() const;
    void          setRadius(const QString &value);

    QString       getF1() const;
    void          setF1(const QString &value);

    QString       getF2() const;
    void          setF2(const QString &value);

    QString       getPenStyle() const;
    void          setPenStyle(const QString &value);

    QString       getLineWeight() const;
    void          setLineWeight(const QString &value);

    QString       getLineColor() const;
    void          setLineColor(const QString &value);

public slots:
    virtual void  ChosenObject(quint32 id, const SceneObject &type) override;

    void          deployRadiusTextEdit();
    void          deployAngle1TextEdit();
    void          deployAngle2TextEdit();

    void          radiusFX();
    void          angle1FX();
    void          angle2FX();

protected:
    void          pointNameChanged();
    virtual void  CheckState() final;
    virtual void  ShowVisualization() override;

    virtual void  SaveData() override;
    virtual void  closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(DialogArc)

    Ui::DialogArc *ui;            /// @brief ui keeps information about user interface
    bool          m_flagRadius;   /// @brief m_flagRadius true if value of radius is correct
    bool          m_flagAngle1;   /// @brief m_flagAngle1 true if value of first angle is correct
    bool          m_flagAngle2;   /// @brief flagF2 true if value of second angle is correct

    QTimer        *m_timerRadius; /// @brief m_timerRadius timer of check formula of radius
    QTimer        *m_timerAngle1; /// @brief m_timerAngle1 timer of check formula of first angle
    QTimer        *m_timerAngle2; /// @brief m_timerAngle2 timer of check formula of second angle

    QString       m_radiusFx;     /// @brief m_radiusFx formula of radius
    QString       m_angle1Fx;     /// @brief m_angle1Fx formula of first angle
    QString       m_angle2Fx;     /// @brief m_angle2Fx formula of second angle

    int           m_baseHeightRadius; /// @brief m_baseHeightRadius base height defined by dialogui
    int           m_baseHeightAngle1; /// @brief m_baseHeightAngle1 base height defined by dialogui
    int           m_baseHeightAngle2; /// @brief m_baseHeightAngle2 base height defined by dialogui

    qreal         m_angle1;
    qreal         m_angle2;
    VArc          m_arc;
    qint32        m_Id;
    qint32        m_newDuplicate;

    void          evalRadius();
    void          evalAngles();
    void          checkAngles();
};

#endif // DIALOG_ARC_H

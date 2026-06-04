//-----------------------------------------------------------------------------
//  @file   dialogellipticalarc.h
//  @author Douglas S Caskey
//  @date   14 Aug, 2023
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
//  @file   dialogellipticalarc.h
//  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
//  @date   15 Sep, 2013
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

#ifndef DIALOGELLIPTICALARC_H
#define DIALOGELLIPTICALARC_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vgeometry/vellipticalarc.h"
#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui
{
    class DialogEllipticalArc;
}

class DialogEllipticalArc : public DialogTool
{
    Q_OBJECT

public:
    DialogEllipticalArc(const VContainer *data, const quint32 &toolId, QWidget *parent = nullptr);
    virtual ~DialogEllipticalArc() override;

    VEllipticalArc getArc() const;
    void           setArc(const VEllipticalArc &arc);

    quint32        GetCenter() const;
    void           SetCenter(const quint32 &value);

    QString        GetRadius1() const;
    void           SetRadius1(const QString &value);

    QString        GetRadius2() const;
    void           SetRadius2(const QString &value);

    QString        GetF1() const;
    void           SetF1(const QString &value);

    QString        GetF2() const;
    void           SetF2(const QString &value);

    QString        getRotationAngle() const;
    void           SetRotationAngle(const QString &value);

    QString        getPenStyle() const;
    void           setPenStyle(const QString &value);

    QString        getLineWeight() const;
    void           setLineWeight(const QString &value);

    QString        getLineColor() const;
    void           setLineColor(const QString &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) override;

    void           deployRadius1TextEdit();
    void           deployRadius2TextEdit();
    void           deployF1TextEdit();
    void           deployF2TextEdit();
    void           deployRotationAngleTextEdit();

    void           FXRadius1();
    void           FXRadius2();
    void           FXF1();
    void           FXF2();
    void           FXRotationAngle();

protected:
    void           pointNameChanged();
    virtual void   CheckState() final;
    virtual void   ShowVisualization() override;

    virtual void   SaveData() override;
    virtual void   closeEvent(QCloseEvent *event) override;

private:
    Q_DISABLE_COPY(DialogEllipticalArc)

    Ui::DialogEllipticalArc *ui;     /// @brief ui keeps information about user interface

    bool           m_flagRadius1;    /// @brief m_flagRadius1 true if value of radius1 is correct
    bool           m_flagRadius2;    /// @brief m_flagRadius2 true if value of radius2 is correct
    bool           m_flagAngle1;     /// @brief m_flagAngle1 true if value of first angle is correct
    bool           m_flagAngle2;     /// @brief m_flagAngle2 true if value of second angle is correct
    bool           m_flagRotation;   /// @brief m_flagRotation true if value of rotation angle is correct

    QTimer         *m_timerRadius1;  /// @brief m_timerRadius1 timer of check formula of radius1
    QTimer         *m_timerRadius2;  /// @brief m_timerRadius2 timer of check formula of radius2
    QTimer         *m_timerAngle1;   /// @brief m_timerAngle1 timer of check formula of first angle
    QTimer         *m_timerAngle2;   /// @brief m_timerAngle2 timer of check formula of second angle
    QTimer         *m_timerRotation; /// @brief m_timerRotation timer of check formula of rotation angle


    QString        m_radius1Fx;      /// @brief m_radius1Fx formula of radius1
    QString        m_radius2Fx;      /// @brief radius2 formula of radius2
    QString        m_angle1Fx;       /// @brief m_angle1Fx formula of first angle
    QString        m_angle2Fx;       /// @brief m_angle2Fx formula of second angle
    QString        m_rotationFx;     /// @brief m_rotationFx formula of rotation angle

    /// @brief formulaBaseHeight base height defined by dialogui
    int            m_baseHeightRadius1;
    int            m_baseHeightRadius2;
    int            m_baseHeightAngle1;
    int            m_baseHeightAngle2;
    int            m_baseHeightRotation;

    qreal          m_angleF1;
    qreal          m_angleF2;
    qreal          m_rotationAngle;
    VEllipticalArc m_arc;
    qint32         m_Id;
    qint32         m_newDuplicate;

    void           evalRadiuses();
    void           evalAngles();
    void           checkAngles();
    void           collapseFormula(QPlainTextEdit *textEdit, QPushButton *pushButton, int height);
};

#endif // DIALOGELLIPTICALARC_H

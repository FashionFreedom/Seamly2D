/***************************************************************************
 **  @file   dialogcubicbezierlength.h
 **  @author Seamly2D contributors
 **
 **  @brief  Dialog for the cubic Bézier curve with matched length tool.
 **
 **  The user selects start point P1, end point P4, then enters four formula
 **  fields: Angle1, C1Length, Angle2, TargetLength.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#ifndef DIALOGCUBICBEZIERLENGTH_H
#define DIALOGCUBICBEZIERLENGTH_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui { class DialogCubicBezierLength; }

class DialogCubicBezierLength : public DialogTool
{
    Q_OBJECT
public:
    explicit DialogCubicBezierLength(const VContainer *data, const quint32 &toolId,
                                      QWidget *parent = nullptr);
    virtual ~DialogCubicBezierLength();

    quint32        GetPoint1() const;
    void           SetPoint1(const quint32 &value);

    quint32        GetPoint4() const;
    void           SetPoint4(const quint32 &value);

    QString        GetAngle1() const;
    void           SetAngle1(const QString &value);

    QString        GetAngle2() const;
    void           SetAngle2(const QString &value);

    QString        GetC1Length() const;
    void           SetC1Length(const QString &value);

    QString        GetTargetLength() const;
    void           SetTargetLength(const QString &value);

    QString        getPenStyle() const;
    void           setPenStyle(const QString &value);

    QString        getLineWeight() const;
    void           setLineWeight(const QString &value);

    QString        getLineColor() const;
    void           setLineColor(const QString &value);

public slots:
    virtual void   ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;

    void           DeployAngle1TextEdit();
    void           DeployC1LengthTextEdit();
    void           DeployAngle2TextEdit();
    void           DeployTargetLengthTextEdit();

    void           Angle1Changed();
    void           C1LengthChanged();
    void           Angle2Changed();
    void           TargetLengthChanged();

    void           FXAngle1();
    void           FXC1Length();
    void           FXAngle2();
    void           FXTargetLength();

protected:
    void           pointNameChanged();
    virtual void   CheckState() Q_DECL_FINAL;
    virtual void   ShowVisualization() Q_DECL_OVERRIDE;
    virtual void   SaveData() Q_DECL_OVERRIDE;
    virtual void   closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DialogCubicBezierLength)
    Ui::DialogCubicBezierLength *ui;

    bool           flagAngle1;
    bool           flagC1Length;
    bool           flagAngle2;
    bool           flagTargetLength;

    QTimer        *timerAngle1;
    QTimer        *timerC1Length;
    QTimer        *timerAngle2;
    QTimer        *timerTargetLength;

    QString        m_angle1;
    QString        m_c1Length;
    QString        m_angle2;
    QString        m_targetLength;

    int            formulaBaseHeightAngle1;
    int            formulaBaseHeightC1Length;
    int            formulaBaseHeightAngle2;
    int            formulaBaseHeightTargetLength;

    void           EvalAngle1();
    void           EvalAngle2();
    void           EvalC1Length();
    void           EvalTargetLength();
};

#endif // DIALOGCUBICBEZIERLENGTH_H

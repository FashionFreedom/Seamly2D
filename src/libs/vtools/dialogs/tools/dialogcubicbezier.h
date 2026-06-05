/***************************************************************************
 *  @file   dialogcubicbezier.h
 *  @author Douglas S Caskey / Seamly2D contributors
 *
 *  @brief  Dialog for the parametric cubic Bézier tool.
 *
 *  Supports four computation modes selected by two checkboxes:
 *    State 1 (both off):  manual angle1 + c1 + angle2 + c2
 *    State 2 (A only):    bisection – c2 set so arc-length == targetLength
 *    State 3 (B only):    Hobby algorithm computes both c1 and c2
 *    State 4 (A + B):     Hobby proportions + bisection for global scale
 *
 *  Seamly2D is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 ***************************************************************************/

#ifndef DIALOGCUBICBEZIER_H
#define DIALOGCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "dialogtool.h"

namespace Ui { class DialogCubicBezier; }

class DialogCubicBezier : public DialogTool
{
    Q_OBJECT
public:
    explicit DialogCubicBezier(const VContainer *data, const quint32 &toolId,
                                QWidget *parent = nullptr);
    virtual ~DialogCubicBezier();

    // Point selectors
    quint32        GetPoint1() const;
    void           SetPoint1(const quint32 &value);

    quint32        GetPoint4() const;
    void           SetPoint4(const quint32 &value);

    // Formula fields
    QString        GetAngle1() const;
    void           SetAngle1(const QString &value);

    QString        GetAngle2() const;
    void           SetAngle2(const QString &value);

    QString        GetC1Length() const;
    void           SetC1Length(const QString &value);

    QString        GetC2Length() const;
    void           SetC2Length(const QString &value);

    QString        GetTargetLength() const;
    void           SetTargetLength(const QString &value);

    bool           GetAutoSmooth() const;
    void           SetAutoSmooth(bool value);

    // Line style
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
    void           DeployC2LengthTextEdit();
    void           DeployTargetLengthTextEdit();

    void           Angle1Changed();
    void           C1LengthChanged();
    void           Angle2Changed();
    void           C2LengthChanged();
    void           TargetLengthChanged();

    void           FXAngle1();
    void           FXC1Length();
    void           FXAngle2();
    void           FXC2Length();
    void           FXTargetLength();

    void           OnTargetLengthToggled(bool checked);
    void           OnAutoSmoothToggled(bool checked);

protected:
    void           pointNameChanged();
    virtual void   CheckState() Q_DECL_FINAL;
    virtual void   ShowVisualization() Q_DECL_OVERRIDE;
    virtual void   SaveData() Q_DECL_OVERRIDE;
    virtual void   closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DialogCubicBezier)
    Ui::DialogCubicBezier *ui;

    bool           flagAngle1;
    bool           flagC1Length;
    bool           flagAngle2;
    bool           flagC2Length;
    bool           flagTargetLength;

    QTimer        *timerAngle1;
    QTimer        *timerC1Length;
    QTimer        *timerAngle2;
    QTimer        *timerC2Length;
    QTimer        *timerTargetLength;

    QString        m_angle1;
    QString        m_c1Length;
    QString        m_angle2;
    QString        m_c2Length;
    QString        m_targetLength;

    int            formulaBaseHeightAngle1;
    int            formulaBaseHeightC1Length;
    int            formulaBaseHeightAngle2;
    int            formulaBaseHeightC2Length;
    int            formulaBaseHeightTargetLength;

    void           EvalAngle1();
    void           EvalAngle2();
    void           EvalC1Length();
    void           EvalC2Length();
    void           EvalTargetLength();

    void           updateC2LengthEnabled();
    void           updateTargetLengthVisible(bool visible);
};

#endif // DIALOGCUBICBEZIER_H

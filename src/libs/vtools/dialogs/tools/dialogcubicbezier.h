/***************************************************************************
 **  @file   dialogcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com> / Seamly2D contributors
 **
 **  Dialog for "Kurve Fixiert" — the user clicks 4 canvas points.
 **  Two optional checkboxes modify the curve computation:
 **    Checkbox A: scale both handles to hit a target arc length
 **    Checkbox B: Hobby algorithm sets optimal handle lengths (direction from P2/P3)
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

    // The 4 original canvas point IDs
    quint32       GetP1Id() const;
    void          SetP1Id(const quint32 &value);

    quint32       GetP2Id() const;
    void          SetP2Id(const quint32 &value);

    quint32       GetP3Id() const;
    void          SetP3Id(const quint32 &value);

    quint32       GetP4Id() const;
    void          SetP4Id(const quint32 &value);

    // Optional target length formula (empty = Checkbox A off)
    QString       GetTargetLength() const;
    void          SetTargetLength(const QString &value);

    bool          GetAutoSmooth() const;
    void          SetAutoSmooth(bool value);

    // Line attributes
    QString       getPenStyle() const;
    void          setPenStyle(const QString &value);

    QString       getLineWeight() const;
    void          setLineWeight(const QString &value);

    QString       getLineColor() const;
    void          setLineColor(const QString &value);

public slots:
    virtual void  ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;

    void          DeployTargetLengthTextEdit();
    void          TargetLengthChanged();
    void          FXTargetLength();
    void          OnTargetLengthToggled(bool checked);
    void          OnAutoSmoothToggled(bool checked);

protected:
    void          PointNameChanged();
    virtual void  CheckState() Q_DECL_FINAL;
    virtual void  ShowVisualization() Q_DECL_OVERRIDE;
    virtual void  SaveData() Q_DECL_OVERRIDE;
    virtual void  closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(DialogCubicBezier)
    Ui::DialogCubicBezier *ui;

    bool          flagTargetLength;
    bool          flagError;

    QTimer       *timerTargetLength;
    QString       m_targetLength;
    int           formulaBaseHeightTargetLength;

    qint32        newDuplicate;

    void          EvalTargetLength();
    void          updateTargetLengthVisible(bool visible);
};

#endif // DIALOGCUBICBEZIER_H

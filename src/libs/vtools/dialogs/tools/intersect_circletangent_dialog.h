/**************************************************************************
 **
 **  @file   intersect_circletangent_dialog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
 **
 **  @author Douglas S. Caskey
 **  @date   7.16.2022
 **
 **  @copyright
 **  Copyright (C) 2013-2022 Seamly2D project.
 **  This source code is part of the Seamly2D project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published
 **  by the Free Software Foundation, either version 3 of the License,
 **  or (at your option) any later version.
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

#ifndef INTERSECT_CIRCLETANGENT_DIALOG_H
#define INTERSECT_CIRCLETANGENT_DIALOG_H

#include "dialogtool.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"

#include <qcompilerdetection.h>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

namespace Ui
{
    class IntersectCircleTangentDialog;
}

class IntersectCircleTangentDialog : public DialogTool
{
    Q_OBJECT

public:
                      IntersectCircleTangentDialog(const VContainer *data, const quint32 &toolId,
                                                   QWidget *parent = nullptr);
                     ~IntersectCircleTangentDialog();

    void              SetPointName(const QString &value);

    quint32           GetCircleCenterId() const;
    void              SetCircleCenterId(const quint32 &value);

    QString           GetCircleRadius() const;
    void              SetCircleRadius(const QString &value);

    quint32           GetTangentPointId() const;
    void              SetTangentPointId(const quint32 &value);

    CrossCirclesPoint GetCrossCirclesPoint() const;
    void              SetCrossCirclesPoint(const CrossCirclesPoint &p);

public slots:
    virtual void      ChosenObject(quint32 id, const SceneObject &type) Q_DECL_OVERRIDE;
    void              PointChanged();

    void              DeployCircleRadiusTextEdit();
    void              CircleRadiusChanged();
    void              FXCircleRadius();
    void              EvalCircleRadius();

protected:
    virtual void      ShowVisualization() Q_DECL_OVERRIDE;
    /**
     * @brief SaveData Put dialog data in local variables
     */
    virtual void      SaveData() Q_DECL_OVERRIDE;
    virtual void      closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    virtual void      CheckState() Q_DECL_FINAL;

private:
                      Q_DISABLE_COPY(IntersectCircleTangentDialog)

    Ui::IntersectCircleTangentDialog *ui;

    bool             flagCircleRadius;
    QTimer           *timerCircleRadius;
    QString          circleRadius;
    int              formulaBaseHeightCircleRadius;
};

#endif // INTERSECT_CIRCLETANGENT_DIALOG_H

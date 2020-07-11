/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
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
 **  @file   dialogmdatabase.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 7, 2015
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

#ifndef DIALOGMDATABASE_H
#define DIALOGMDATABASE_H

#include <QDialog>

namespace Ui
{
    class DialogMDataBase;
}

class QTreeWidgetItem;

class DialogMDataBase : public QDialog
{
    Q_OBJECT

public:
    explicit DialogMDataBase(const QStringList &measurements, QWidget *parent = nullptr);
    explicit DialogMDataBase(QWidget *parent = nullptr);
    virtual ~DialogMDataBase() Q_DECL_OVERRIDE;

    void RetranslateGroups();

    QStringList GetNewMeasurements() const;
    static QString ImgTag(const QString &number);

protected:
    virtual void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *target, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void UpdateChecks(QTreeWidgetItem *item, int column);
    void ShowDescription(QTreeWidgetItem *item, int column);
    void TreeMenu(const QPoint &pos);
    void Recheck();

private:
    Q_DISABLE_COPY(DialogMDataBase)
    Ui::DialogMDataBase *ui;
    bool selectMode;
    QStringList measurements;
    QStringList newMeasurements;

    const QString groupAText = "A. " + DialogMDataBase::tr("Direct Height", "Measurement section");
    const QString groupBText = "B. " + DialogMDataBase::tr("Direct Width", "Measurement section");
    const QString groupCText = "C. " + DialogMDataBase::tr("Indentation", "Measurement section");
    const QString groupDText = "D. " + DialogMDataBase::tr("Hand", "Measurement section");
    const QString groupEText = "E. " + DialogMDataBase::tr("Foot", "Measurement section");
    const QString groupFText = "F. " + DialogMDataBase::tr("Head", "Measurement section");
    const QString groupGText = "G. " + DialogMDataBase::tr("Circumference and Arc", "Measurement section");
    const QString groupHText = "H. " + DialogMDataBase::tr("Vertical", "Measurement section");
    const QString groupIText = "I. " + DialogMDataBase::tr("Horizontal", "Measurement section");
    const QString groupJText = "J. " + DialogMDataBase::tr("Bust", "Measurement section");
    const QString groupKText = "K. " + DialogMDataBase::tr("Balance", "Measurement section");
    const QString groupLText = "L. " + DialogMDataBase::tr("Arm", "Measurement section");
    const QString groupMText = "M. " + DialogMDataBase::tr("Leg", "Measurement section");
    const QString groupNText = "N. " + DialogMDataBase::tr("Crotch and Rise", "Measurement section");
    const QString groupOText = "O. " + DialogMDataBase::tr("Men & Tailoring", "Measurement section");
    const QString groupPText = "P. " + DialogMDataBase::tr("Historical & Specialty", "Measurement section");
    const QString groupQText = "Q. " + DialogMDataBase::tr("Patternmaking measurements", "Measurement section");

    QTreeWidgetItem *groupA;
    QTreeWidgetItem *groupB;
    QTreeWidgetItem *groupC;
    QTreeWidgetItem *groupD;
    QTreeWidgetItem *groupE;
    QTreeWidgetItem *groupF;
    QTreeWidgetItem *groupG;
    QTreeWidgetItem *groupH;
    QTreeWidgetItem *groupI;
    QTreeWidgetItem *groupJ;
    QTreeWidgetItem *groupK;
    QTreeWidgetItem *groupL;
    QTreeWidgetItem *groupM;
    QTreeWidgetItem *groupN;
    QTreeWidgetItem *groupO;
    QTreeWidgetItem *groupP;
    QTreeWidgetItem *groupQ;

    void InitDataBase(const QStringList &measurements = QStringList());
    void InitGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &groupList,
                   const QStringList &newMeasurements = QStringList());

    Q_REQUIRED_RESULT QTreeWidgetItem *AddGroup(const QString &text);

    void AddMeasurement(QTreeWidgetItem *group, const QString &groupName, const QStringList &newMeasurements);

    void ReadSettings();
    void WriteSettings();
    void Filter(const QString term);

    void RetranslateGroup(QTreeWidgetItem *group, const QString &groupText, const QStringList &measurements);
    void RetranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name);

    void ChangeCheckState(QTreeWidgetItem *group, Qt::CheckState check);
    Qt::CheckState GlobalCheckState() const;
};

#endif // DIALOGMDATABASE_H

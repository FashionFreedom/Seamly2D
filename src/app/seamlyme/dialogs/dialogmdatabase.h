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

class MeasurementDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MeasurementDatabaseDialog(const QStringList &measurements, QWidget *parent = nullptr);
    explicit MeasurementDatabaseDialog(QWidget *parent = nullptr);
    virtual ~MeasurementDatabaseDialog() Q_DECL_OVERRIDE;

    void RetranslateGroups();

    QStringList getNewMeasurementNames() const;
    static QString imgTag(const QString &number);

protected:
    virtual void changeEvent(QEvent* event) Q_DECL_OVERRIDE;
    virtual bool eventFilter(QObject *target, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void UpdateChecks(QTreeWidgetItem *item, int column);
    void ShowDescription(QTreeWidgetItem *item, int column);
    void TreeMenu(const QPoint &pos);
    void Recheck();

private:
    Q_DISABLE_COPY(MeasurementDatabaseDialog)
    Ui::DialogMDataBase *ui;
    bool selectMode;
    QStringList measurements;
    QStringList newMeasurements;

    const QString groupAText = "A. " + MeasurementDatabaseDialog::tr("Direct Height", "Measurement section");
    const QString groupBText = "B. " + MeasurementDatabaseDialog::tr("Direct Width", "Measurement section");
    const QString groupCText = "C. " + MeasurementDatabaseDialog::tr("Indentation", "Measurement section");
    const QString groupDText = "D. " + MeasurementDatabaseDialog::tr("Hand", "Measurement section");
    const QString groupEText = "E. " + MeasurementDatabaseDialog::tr("Foot", "Measurement section");
    const QString groupFText = "F. " + MeasurementDatabaseDialog::tr("Head", "Measurement section");
    const QString groupGText = "G. " + MeasurementDatabaseDialog::tr("Circumference and Arc", "Measurement section");
    const QString groupHText = "H. " + MeasurementDatabaseDialog::tr("Vertical", "Measurement section");
    const QString groupIText = "I. " + MeasurementDatabaseDialog::tr("Horizontal", "Measurement section");
    const QString groupJText = "J. " + MeasurementDatabaseDialog::tr("Bust", "Measurement section");
    const QString groupKText = "K. " + MeasurementDatabaseDialog::tr("Balance", "Measurement section");
    const QString groupLText = "L. " + MeasurementDatabaseDialog::tr("Arm", "Measurement section");
    const QString groupMText = "M. " + MeasurementDatabaseDialog::tr("Leg", "Measurement section");
    const QString groupNText = "N. " + MeasurementDatabaseDialog::tr("Crotch and Rise", "Measurement section");
    const QString groupOText = "O. " + MeasurementDatabaseDialog::tr("Men & Tailoring", "Measurement section");
    const QString groupPText = "P. " + MeasurementDatabaseDialog::tr("Historical & Specialty", "Measurement section");
    const QString groupQText = "Q. " + MeasurementDatabaseDialog::tr("Patternmaking measurements", "Measurement section");

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

    void initDataBase(const QStringList &measurements = QStringList());
    void initGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &groupList,
                   const QStringList &newMeasurements = QStringList());

    Q_REQUIRED_RESULT QTreeWidgetItem *AddGroup(const QString &text);

    void addMeasurement(QTreeWidgetItem *group, const QString &groupName, const QStringList &newMeasurements);

    void readSettings();
    void writeSettings();
    void filter(const QString term);

    void retranslateGroup(QTreeWidgetItem *group, const QString &groupText, const QStringList &measurements);
    void retranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name);

    void changeCheckState(QTreeWidgetItem *group, Qt::CheckState check);
    Qt::CheckState GlobalCheckState() const;
};

#endif // DIALOGMDATABASE_H

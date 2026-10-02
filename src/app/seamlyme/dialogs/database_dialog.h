//---------------------------------------------------------------------------------------------------------------------
//  @file   database_dialog.h
//  @author Douglas S Caskey
//  @date   3 Sep, 2023
//
//  @brief
//  @copyright
//  This source code is part of the Seamly2D project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2026 Seamly2D project
//  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
//
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
//  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   dialogmdatabase.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   26 Jul, 2015
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2015 Valentina project
//  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
//
//  Valentina is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  Valentina is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
//---------------------------------------------------------------------------------------------------------------------

#ifndef DATABASE_DIALOG_H
#define DATABASE_DIALOG_H

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
    explicit             MeasurementDatabaseDialog(const QStringList &measurements, QWidget *parent = nullptr);
    explicit             MeasurementDatabaseDialog(QWidget *parent = nullptr);
    virtual             ~MeasurementDatabaseDialog() override;

    void                 retranslateGroups();

    QStringList          getNewMeasurementNames() const;

protected:
    virtual void         changeEvent(QEvent* event) override;
    virtual bool         eventFilter(QObject *target, QEvent *event) override;

private slots:
    void                 updateChecks(QTreeWidgetItem *item, int column);
    void                 showDescription(QTreeWidgetItem *item, int column);
    void                 treeContextMenu(const QPoint &pos);
    void                 recheckState();

private:
    Q_DISABLE_COPY(MeasurementDatabaseDialog)
    Ui::DialogMDataBase *ui;
    bool                 m_selectMode;
    QStringList          m_measurements;
    QStringList          m_newMeasurements;
    QString              m_currentSvgPath;   // Stores the current "://diagrams/..." path
    QString              m_currentNumber;    // Stores the current measurement number (e.g. "L13")
    QString              m_currentName;      // Stores the translated name text
    QString              m_currentDescription; 

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

    QTreeWidgetItem     *m_groupA;
    QTreeWidgetItem     *m_groupB;
    QTreeWidgetItem     *m_groupC;
    QTreeWidgetItem     *m_groupD;
    QTreeWidgetItem     *m_groupE;
    QTreeWidgetItem     *m_groupF;
    QTreeWidgetItem     *m_groupG;
    QTreeWidgetItem     *m_groupH;
    QTreeWidgetItem     *m_groupI;
    QTreeWidgetItem     *m_groupJ;
    QTreeWidgetItem     *m_groupK;
    QTreeWidgetItem     *m_groupL;
    QTreeWidgetItem     *m_groupM;
    QTreeWidgetItem     *m_groupN;
    QTreeWidgetItem     *m_groupO;
    QTreeWidgetItem     *m_groupP;
    QTreeWidgetItem     *m_groupQ;

    void                 initDataBase(const QStringList &measurements = QStringList());
    void                 initGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &groupList,
                                   const QStringList &measurementList = QStringList());

    Q_REQUIRED_RESULT QTreeWidgetItem *addMeasurementGroup(const QString &text);

    void                addMeasurement(QTreeWidgetItem *group, const QString &groupName,
                                       const QStringList &newMeasurements);

    void                readSettings();
    void                writeSettings();
    void                filterGroups(const QString searchString);
    void                filterGroup(QTreeWidgetItem *group, const QString &searchString);

    void                retranslateGroup(QTreeWidgetItem *group, const QString &groupName,
                                         const QStringList &measurements);
    void                retranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name);

    void                changeCheckState(QTreeWidgetItem *group, Qt::CheckState state);
    Qt::CheckState      globalCheckState() const;
    void                renderScaledDiagram();
};

#endif // DATABASE_DIALOG_H

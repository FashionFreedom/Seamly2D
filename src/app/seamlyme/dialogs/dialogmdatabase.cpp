/******************************************************************************
*   @file   dialogmdatabase.cpp
**  @author Douglas S Caskey
**  @date   3 Sep, 2023
**
**  @brief
**  @copyright
**  This source code is part of the Seamly2D project, a pattern making
**  program to create and model patterns of clothing.
**  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   dialogmdatabase.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#include "dialogmdatabase.h"
#include "ui_dialogmdatabase.h"
#include "../application_me.h"
#include "../vpatterndb/measurements_def.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QScreen>
#include <QSvgRenderer>
#include <QtSvg>

//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::MeasurementDatabaseDialog(const QStringList &list, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMDataBase)
    , selectMode(true)
    , measurements(list)
    , newMeasurements()
    , groupA(nullptr)
    , groupB(nullptr)
    , groupC(nullptr)
    , groupD(nullptr)
    , groupE(nullptr)
    , groupF(nullptr)
    , groupG(nullptr)
    , groupH(nullptr)
    , groupI(nullptr)
    , groupJ(nullptr)
    , groupK(nullptr)
    , groupL(nullptr)
    , groupM(nullptr)
    , groupN(nullptr)
    , groupO(nullptr)
    , groupP(nullptr)
    , groupQ(nullptr)
{
    ui->setupUi(this);
    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    initDataBase();

    ui->treeWidget->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged,                this, &MeasurementDatabaseDialog::filterGroups);
    connect(ui->treeWidget, &QTreeWidget::itemChanged,                this, &MeasurementDatabaseDialog::updateChecks);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,                this, &MeasurementDatabaseDialog::showDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MeasurementDatabaseDialog::treeContextMenu);

    readSettings();
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::MeasurementDatabaseDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogMDataBase)
    , selectMode(false)
    , measurements()
    , newMeasurements()
    , groupA(nullptr)
    , groupB(nullptr)
    , groupC(nullptr)
    , groupD(nullptr)
    , groupE(nullptr)
    , groupF(nullptr)
    , groupG(nullptr)
    , groupH(nullptr)
    , groupI(nullptr)
    , groupJ(nullptr)
    , groupK(nullptr)
    , groupL(nullptr)
    , groupM(nullptr)
    , groupN(nullptr)
    , groupO(nullptr)
    , groupP(nullptr)
    , groupQ(nullptr)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

#if defined(Q_OS_MAC)
    setWindowFlags(Qt::Window);
#endif

    initDataBase();

    ui->treeWidget->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged,                this, &MeasurementDatabaseDialog::filterGroups);
    connect(ui->treeWidget, &QTreeWidget::itemClicked,                this, &MeasurementDatabaseDialog::showDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &MeasurementDatabaseDialog::treeContextMenu);
    connect(ui->treeWidget, &QTreeWidget::itemActivated,              this, &MeasurementDatabaseDialog::showDescription);

    readSettings();
}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDatabaseDialog::~MeasurementDatabaseDialog()
{
    writeSettings();
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList MeasurementDatabaseDialog::getNewMeasurementNames() const
{
    return newMeasurements;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateGroups()
{
    retranslateGroup(groupA, groupAText, ListGroupA());
    retranslateGroup(groupB, groupBText, ListGroupB());
    retranslateGroup(groupC, groupCText, ListGroupC());
    retranslateGroup(groupD, groupDText, ListGroupD());
    retranslateGroup(groupE, groupEText, ListGroupE());
    retranslateGroup(groupF, groupFText, ListGroupF());
    retranslateGroup(groupG, groupGText, ListGroupG());
    retranslateGroup(groupH, groupHText, ListGroupH());
    retranslateGroup(groupI, groupIText, ListGroupI());
    retranslateGroup(groupJ, groupJText, ListGroupJ());
    retranslateGroup(groupK, groupKText, ListGroupK());
    retranslateGroup(groupL, groupLText, ListGroupL());
    retranslateGroup(groupM, groupMText, ListGroupM());
    retranslateGroup(groupN, groupNText, ListGroupN());
    retranslateGroup(groupO, groupOText, ListGroupO());
    retranslateGroup(groupP, groupPText, ListGroupP());
    retranslateGroup(groupQ, groupQText, ListGroupQ());

    showDescription(ui->treeWidget->currentItem(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
QString MeasurementDatabaseDialog::imageUrl(const QString &number)
{
    QString imgUrl("<img src=\"wrong.png\" align=\"center\"/>"); // In case of error
    const QString filePath = QString("://diagrams/%1.svg").arg(MapDiagrams(qApp->translateVariables(), number));
    if (QFileInfo::exists(filePath))
    {
        // Load your SVG
        QSvgRenderer renderer;
        const bool ok = renderer.load(filePath);
        if (ok)
        {
            const QScreen *screen = QGuiApplication::screens().at(0);
            if (screen)
            {
                const QSize defSize = renderer.defaultSize();

                // Prepare a QImage with desired characteritisc
                QImage image(defSize, QImage::Format_RGB32);
                image.fill(Qt::white);

                const QRect geometry = screen->geometry();
                const int baseHeight = 1440;
                const int imgHeight = geometry.height() * defSize.height() / baseHeight;

                QImage scaledImg = image.scaledToHeight(imgHeight);

                // Get QPainter that paints to the image
                QPainter painter(&scaledImg);
                renderer.render(&painter);

                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                scaledImg.save(&buffer, "PNG");
                imgUrl = QString("<img src=\"data:image/png;base64,") + byteArray.toBase64() + "\" align=\"center\"/>";
            }
        }
    }

    return imgUrl;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
bool MeasurementDatabaseDialog::eventFilter(QObject *target, QEvent *event)
{
    if (target == ui->treeWidget && event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        switch(keyEvent->key())
        {
            case Qt::Key_Up:
            {
                const QModelIndex model = ui->treeWidget->indexAbove(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemAbove(ui->treeWidget->currentItem());
                showDescription(item, model.column());
                break;
            }
            case Qt::Key_Down:
            {
                const QModelIndex model = ui->treeWidget->indexBelow(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemBelow(ui->treeWidget->currentItem());
                showDescription(item, model.column());
                break;
            }
            default:
                break;
        }
    }
    return QDialog::eventFilter(target, event);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::updateChecks(QTreeWidgetItem *item, int column)
{
    bool diff = false;
    if (column != 0 && column != -1)
    {
        return;
    }

    if (item->childCount() != 0 && item->checkState(0) != Qt::PartiallyChecked && column != -1)
    {
        bool flag = false; // Check if we could change atleast one children
        Qt::CheckState state = item->checkState(0);
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (!measurements.contains(item->child(i)->data(0, Qt::UserRole).toString()))
            {
                item->child(i)->setCheckState(0, state);
                flag = true;
            }

            if (flag == false) // All child in the list
            {
                item->setCheckState(0, Qt::Checked);
            }
        }
    }
    else if (item->childCount() == 0 || column == -1)
    {

        QString itemName = item->data(0, Qt::UserRole).toString();
        Qt::CheckState state = item->checkState(0);

        if (state == Qt::CheckState::Unchecked)
        {
            newMeasurements.removeOne(itemName);
        }
        else if (state == Qt::CheckState::Checked &&
                 !measurements.contains(itemName) &&
                 !newMeasurements.contains(itemName) &&
                 !itemName.isEmpty())
        {
            newMeasurements.append(itemName);
        }

        QTreeWidgetItem *parent = item->parent();
        if (parent == nullptr)
        {
            return;
        }
        for (int j = 0; j < parent->childCount(); ++j)
        {
            if (j != parent->indexOfChild(item)
                    && item->checkState(0) != parent->child(j)->checkState(0))
            {
                diff = true;
            }
        }
        if (diff)
        {
            parent->setCheckState(0, Qt::PartiallyChecked);
        }
        else
        {
            parent->setCheckState(0, item->checkState(0));
        }

        updateChecks(parent, -1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::showDescription(QTreeWidgetItem *item, int column)
{
    if ((column != 0 && column != -1) || item == nullptr || (item->childCount() != 0 ))
    {
        ui->textEdit->clear();
        return;
    }

    const QString name = item->data(0, Qt::UserRole).toString();
    const VTranslateVars *trv = qApp->translateVariables();
    const QString number = trv->MNumber(name);

    const QString text = QString("<p align=\"center\" style=\"font-variant: normal; font-style: normal; font-weight: "
                                 "normal\"> %1 <br clear=\"left\"><b>%2</b>. <i>%3</i></p>"
                                 "<p align=\"left\" style=\"font-variant: normal; font-style: normal; font-weight: "
                                 "normal\">%4</p>")
                                 .arg(imageUrl(number))
                                 .arg(number)
                                 .arg(trv->guiText(name))
                                 .arg(trv->Description(name));

    ui->textEdit->setHtml(text);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::treeContextMenu(const QPoint &pos)
{
    // Because item also will be selected need to show description
    const QModelIndex model = ui->treeWidget->currentIndex();
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    showDescription(item, model.column());

    QAction *actionCollapseAll = new QAction(tr("Collapse All"), this);
    connect(actionCollapseAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::collapseAll);

    QAction *actionExpandeAll = new QAction(tr("Expand All"), this);
    connect(actionExpandeAll, &QAction::triggered, ui->treeWidget, &QTreeWidget::expandAll);

    QMenu menu(this);
    menu.addAction(actionCollapseAll);
    menu.addAction(actionExpandeAll);

    if (selectMode)
    {
        QString actionName;
        globalCheckState() == Qt::Checked ? actionName = tr("Check all") : actionName = tr("Uncheck all");

        QAction *actionRecheckState = new QAction(actionName, this);
        connect(actionRecheckState, &QAction::triggered, this, &MeasurementDatabaseDialog::recheckState);

        menu.addAction(actionRecheckState);
    }
    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::recheckState()
{
    if (selectMode)
    {
        const Qt::CheckState check = globalCheckState();

        changeCheckState(groupA, check);
        changeCheckState(groupB, check);
        changeCheckState(groupC, check);
        changeCheckState(groupD, check);
        changeCheckState(groupE, check);
        changeCheckState(groupF, check);
        changeCheckState(groupG, check);
        changeCheckState(groupH, check);
        changeCheckState(groupI, check);
        changeCheckState(groupJ, check);
        changeCheckState(groupK, check);
        changeCheckState(groupL, check);
        changeCheckState(groupM, check);
        changeCheckState(groupN, check);
        changeCheckState(groupO, check);
        changeCheckState(groupP, check);
        changeCheckState(groupQ, check);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::initDataBase(const QStringList &list)
{
    initGroup(&groupA, groupAText, ListGroupA(), list);
    initGroup(&groupB, groupBText, ListGroupB(), list);
    initGroup(&groupC, groupCText, ListGroupC(), list);
    initGroup(&groupD, groupDText, ListGroupD(), list);
    initGroup(&groupE, groupEText, ListGroupE(), list);
    initGroup(&groupF, groupFText, ListGroupF(), list);
    initGroup(&groupG, groupGText, ListGroupG(), list);
    initGroup(&groupH, groupHText, ListGroupH(), list);
    initGroup(&groupI, groupIText, ListGroupI(), list);
    initGroup(&groupJ, groupJText, ListGroupJ(), list);
    initGroup(&groupK, groupKText, ListGroupK(), list);
    initGroup(&groupL, groupLText, ListGroupL(), list);
    initGroup(&groupM, groupMText, ListGroupM(), list);
    initGroup(&groupN, groupNText, ListGroupN(), list);
    initGroup(&groupO, groupOText, ListGroupO(), list);
    initGroup(&groupP, groupPText, ListGroupP(), list);
    initGroup(&groupQ, groupQText, ListGroupQ(), list);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::initGroup(QTreeWidgetItem **group, const QString &groupName,
                                          const QStringList &groupList, const QStringList &measurementList)
{
    *group = addMeasurementGroup(groupName);
    for (int i=0; i < groupList.size(); ++i)
    {
        addMeasurement(*group, groupList.at(i), measurementList);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem *MeasurementDatabaseDialog::addMeasurementGroup(const QString &groupName)
{
    QTreeWidgetItem *group = new QTreeWidgetItem(ui->treeWidget);
    group->setText(0, groupName);
    group->setToolTip(0, groupName);
    group->setExpanded(true);
    if (selectMode)
    {
        group->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        group->setCheckState(0, Qt::Unchecked);
        group->setBackground(0, QBrush(Qt::lightGray));
    }
    return group;
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::addMeasurement(QTreeWidgetItem *group, const QString &name, const QStringList &list)
{
    SCASSERT(group != nullptr)

    QTreeWidgetItem *item = new QTreeWidgetItem(group);

    if (selectMode)
    {
        if (measurements.contains(name))
        {
            item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            item->setCheckState(0, Qt::Checked);
            item->setBackground(0, QBrush(QColor(231, 231, 0, 127)));
        }
        else if (list.contains(name))
        {
            item->setCheckState(0, Qt::Checked);
        }
        else
        {
            item->setCheckState(0, Qt::Unchecked);
        }

        updateChecks(item, 0);
    }

    const QString text = qApp->translateVariables()->MNumber(name) + ". " + qApp->translateVariables()->MToUser(name);
    item->setText(0, text);
    item->setToolTip(0, text);
    item->setData(0, Qt::UserRole, name);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::readSettings()
{
    restoreGeometry(qApp->seamlyMeSettings()->getDataBaseGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::writeSettings()
{
    qApp->seamlyMeSettings()->setDataBaseGeometry(saveGeometry());
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::filterGroups(const QString searchString)
{
    filterGroup(groupA, searchString);
    filterGroup(groupB, searchString);
    filterGroup(groupC, searchString);
    filterGroup(groupD, searchString);
    filterGroup(groupE, searchString);
    filterGroup(groupF, searchString);
    filterGroup(groupG, searchString);
    filterGroup(groupH, searchString);
    filterGroup(groupI, searchString);
    filterGroup(groupJ, searchString);
    filterGroup(groupK, searchString);
    filterGroup(groupL, searchString);
    filterGroup(groupM, searchString);
    filterGroup(groupN, searchString);
    filterGroup(groupO, searchString);
    filterGroup(groupP, searchString);
    filterGroup(groupQ, searchString);

    const QList<QTreeWidgetItem *> selected = ui->treeWidget->selectedItems();
    selected.isEmpty() ? showDescription(nullptr, -1) : showDescription(selected.first(), 0);
}

void MeasurementDatabaseDialog::filterGroup(QTreeWidgetItem *group, const QString &searchString)
{
    SCASSERT(group != nullptr)

    bool isMatch = false;
    for (int i=0; i < group->childCount(); ++i)
    {
        QTreeWidgetItem *item = group->child(i);
        //const QString description = QTextDocumentFragment::fromHtml(ItemFullDescription(item, false))
        //        .toPlainText();

        //const bool isHidden = !item->text(0).contains(searchString, Qt::CaseInsensitive)
        //        && !description.contains(searchString, Qt::CaseInsensitive);

        const bool isHidden = !item->text(0).contains(searchString, Qt::CaseInsensitive);

        item->setHidden(isHidden);
        if (!isHidden)
        {
            isMatch = true;
        }
    }

    group->setHidden(!group->text(0).contains(searchString, Qt::CaseInsensitive) && !isMatch);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateGroup(QTreeWidgetItem *group, const QString &groupName,
                                                 const QStringList &list)
{
    group->setText(0, groupName);
    group->setToolTip(0, groupName);

    for (int i=0; i<list.size(); ++i)
    {
        retranslateMeasurement(group, i, list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::retranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name)
{
    const QString text = qApp->translateVariables()->MNumber(name) + ". " + qApp->translateVariables()->MToUser(name);

    QTreeWidgetItem *item = group->child(index);
    item->setText(0, text);
    item->setToolTip(0, text);
}

//---------------------------------------------------------------------------------------------------------------------
void MeasurementDatabaseDialog::changeCheckState(QTreeWidgetItem *group, Qt::CheckState state)
{
    SCASSERT(group != nullptr)
    group->setCheckState(0, state);
}

//---------------------------------------------------------------------------------------------------------------------
Qt::CheckState MeasurementDatabaseDialog::globalCheckState() const
{
    SCASSERT(groupA != nullptr)
    SCASSERT(groupB != nullptr)
    SCASSERT(groupC != nullptr)
    SCASSERT(groupD != nullptr)
    SCASSERT(groupE != nullptr)
    SCASSERT(groupF != nullptr)
    SCASSERT(groupG != nullptr)
    SCASSERT(groupH != nullptr)
    SCASSERT(groupI != nullptr)
    SCASSERT(groupJ != nullptr)
    SCASSERT(groupK != nullptr)
    SCASSERT(groupL != nullptr)
    SCASSERT(groupM != nullptr)
    SCASSERT(groupN != nullptr)
    SCASSERT(groupO != nullptr)
    SCASSERT(groupP != nullptr)
    SCASSERT(groupQ != nullptr)

    if (selectMode)
    {
        if (groupA->checkState(0) == Qt::Unchecked &&
            groupB->checkState(0) == Qt::Unchecked &&
            groupC->checkState(0) == Qt::Unchecked &&
            groupD->checkState(0) == Qt::Unchecked &&
            groupE->checkState(0) == Qt::Unchecked &&
            groupF->checkState(0) == Qt::Unchecked &&
            groupG->checkState(0) == Qt::Unchecked &&
            groupH->checkState(0) == Qt::Unchecked &&
            groupI->checkState(0) == Qt::Unchecked &&
            groupJ->checkState(0) == Qt::Unchecked &&
            groupK->checkState(0) == Qt::Unchecked &&
            groupL->checkState(0) == Qt::Unchecked &&
            groupM->checkState(0) == Qt::Unchecked &&
            groupN->checkState(0) == Qt::Unchecked &&
            groupO->checkState(0) == Qt::Unchecked &&
            groupP->checkState(0) == Qt::Unchecked &&
            groupQ->checkState(0) == Qt::Unchecked)
        {
            return Qt::Checked;
        }
        else if (groupA->checkState(0) == Qt::Checked &&
                 groupB->checkState(0) == Qt::Checked &&
                 groupC->checkState(0) == Qt::Checked &&
                 groupD->checkState(0) == Qt::Checked &&
                 groupE->checkState(0) == Qt::Checked &&
                 groupF->checkState(0) == Qt::Checked &&
                 groupG->checkState(0) == Qt::Checked &&
                 groupH->checkState(0) == Qt::Checked &&
                 groupI->checkState(0) == Qt::Checked &&
                 groupJ->checkState(0) == Qt::Checked &&
                 groupK->checkState(0) == Qt::Checked &&
                 groupL->checkState(0) == Qt::Checked &&
                 groupM->checkState(0) == Qt::Checked &&
                 groupN->checkState(0) == Qt::Checked &&
                 groupO->checkState(0) == Qt::Checked &&
                 groupP->checkState(0) == Qt::Checked &&
                 groupQ->checkState(0) == Qt::Checked)
        {
            return Qt::Unchecked;
        }
        return Qt::Checked;
    }

    return Qt::Unchecked;
}

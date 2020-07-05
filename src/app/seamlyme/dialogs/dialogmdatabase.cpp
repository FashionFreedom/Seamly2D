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
 **  @file   dialogmdatabase.cpp
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

#include "dialogmdatabase.h"
#include "ui_dialogmdatabase.h"
#include "../mapplication.h"
#include "../vpatterndb/measurements.h"

#include <QKeyEvent>
#include <QMenu>
#include <QSvgRenderer>
#include <QtSvg>

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

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::DialogMDataBase(const QStringList &list, QWidget *parent)
    :QDialog(parent),
      ui(new Ui::DialogMDataBase),
      selectMode(true),
      list(list),
      groupA(nullptr),
      groupB(nullptr),
      groupC(nullptr),
      groupD(nullptr),
      groupE(nullptr),
      groupF(nullptr),
      groupG(nullptr),
      groupH(nullptr),
      groupI(nullptr),
      groupJ(nullptr),
      groupK(nullptr),
      groupL(nullptr),
      groupM(nullptr),
      groupN(nullptr),
      groupO(nullptr),
      groupP(nullptr),
      groupQ(nullptr)
{
    ui->setupUi(this);
    InitDataBase(list);

    ui->treeWidget->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged, [this] (const QString &term){Filter(term);});
    connect(ui->treeWidget, &QTreeWidget::itemChanged, this, &DialogMDataBase::UpdateChecks);
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DialogMDataBase::ShowDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogMDataBase::TreeMenu);

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::DialogMDataBase(QWidget *parent)
    :QDialog(parent),
      ui(new Ui::DialogMDataBase),
      selectMode(false),
      list(),
      groupA(nullptr),
      groupB(nullptr),
      groupC(nullptr),
      groupD(nullptr),
      groupE(nullptr),
      groupF(nullptr),
      groupG(nullptr),
      groupH(nullptr),
      groupI(nullptr),
      groupJ(nullptr),
      groupK(nullptr),
      groupL(nullptr),
      groupM(nullptr),
      groupN(nullptr),
      groupO(nullptr),
      groupP(nullptr),
      groupQ(nullptr)

{
    ui->setupUi(this);
    InitDataBase();

    ui->treeWidget->installEventFilter(this);

    connect(ui->lineEditFind, &QLineEdit::textChanged, [this] (const QString &term){Filter(term);});
    connect(ui->treeWidget, &QTreeWidget::itemClicked, this, &DialogMDataBase::ShowDescription);
    connect(ui->treeWidget, &QTreeWidget::customContextMenuRequested, this, &DialogMDataBase::TreeMenu);
    connect(ui->treeWidget, &QTreeWidget::itemActivated, this, &DialogMDataBase::ShowDescription);

    ReadSettings();
}

//---------------------------------------------------------------------------------------------------------------------
DialogMDataBase::~DialogMDataBase()
{
    WriteSettings();
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList DialogMDataBase::GetNewNames() const
{
    if (selectMode)
    {
        QStringList newNames;
        QTreeWidgetItemIterator it(ui->treeWidget,
                                   QTreeWidgetItemIterator::NoChildren | QTreeWidgetItemIterator::Checked );
        while (*it)
        {
            const QString name = (*it)->data(0, Qt::UserRole).toString();
            if (not list.contains(name))
            {
                newNames.append(name);
            }
            ++it;
        }
        return newNames;
    }
    else
    {
        return QStringList();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::RetranslateGroups()
{
    RetranslateGroup(groupA, groupAText, ListGroupA());
    RetranslateGroup(groupB, groupBText, ListGroupB());
    RetranslateGroup(groupC, groupCText, ListGroupC());
    RetranslateGroup(groupD, groupDText, ListGroupD());
    RetranslateGroup(groupE, groupEText, ListGroupE());
    RetranslateGroup(groupF, groupFText, ListGroupF());
    RetranslateGroup(groupG, groupGText, ListGroupG());
    RetranslateGroup(groupH, groupHText, ListGroupH());
    RetranslateGroup(groupI, groupIText, ListGroupI());
    RetranslateGroup(groupJ, groupJText, ListGroupJ());
    RetranslateGroup(groupK, groupKText, ListGroupK());
    RetranslateGroup(groupL, groupLText, ListGroupL());
    RetranslateGroup(groupM, groupMText, ListGroupM());
    RetranslateGroup(groupN, groupNText, ListGroupN());
    RetranslateGroup(groupO, groupOText, ListGroupO());
    RetranslateGroup(groupP, groupPText, ListGroupP());
    RetranslateGroup(groupQ, groupQText, ListGroupQ());

    ShowDescription(ui->treeWidget->currentItem(), 0);
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogMDataBase::ImgTag(const QString &number)
{
    QString imgUrl("<img src=\"wrong.png\" align=\"center\"/>"); // In case of error
    const QString filePath = QString("://diagrams/%1.svg").arg(MapDiagrams(qApp->TrVars(), number));
    if (QFileInfo(filePath).exists())
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
void DialogMDataBase::changeEvent(QEvent *event)
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
bool DialogMDataBase::eventFilter(QObject *target, QEvent *event)
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
                ShowDescription(item, model.column());
                break;
            }
            case Qt::Key_Down:
            {
                const QModelIndex model = ui->treeWidget->indexBelow(ui->treeWidget->currentIndex());
                QTreeWidgetItem *item = ui->treeWidget->itemBelow(ui->treeWidget->currentItem());
                ShowDescription(item, model.column());
                break;
            }
            default:
                break;
        }
    }
    return QDialog::eventFilter(target, event);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::UpdateChecks(QTreeWidgetItem *item, int column)
{
    bool diff = false;
    if (column != 0 && column != -1)
    {
        return;
    }

    if (item->childCount() != 0 && item->checkState(0) != Qt::PartiallyChecked && column != -1)
    {
        bool flag = false; // Check if we could change atleast one children
        Qt::CheckState checkState = item->checkState(0);
        for (int i = 0; i < item->childCount(); ++i)
        {
            if (not list.contains(item->child(i)->data(0, Qt::UserRole).toString()))
            {
                item->child(i)->setCheckState(0, checkState);
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

        UpdateChecks(parent, -1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ShowDescription(QTreeWidgetItem *item, int column)
{
    if (column != 0 && column != -1)
    {
        ui->textEdit->clear();
        return;
    }

    if (item == nullptr)
    {
        ui->textEdit->clear();
        return;
    }

    if (item->childCount() != 0)
    {
        ui->textEdit->clear();
        return;
    }

    const QString name = item->data(0, Qt::UserRole).toString();
    const VTranslateVars *trv = qApp->TrVars();
    const QString number = trv->MNumber(name);

    const QString text = QString("<p align=\"center\" style=\"font-variant: normal; font-style: normal; font-weight: "
                                 "normal\"> %1 <br clear=\"left\"><b>%2</b>. <i>%3</i></p>"
                                 "<p align=\"left\" style=\"font-variant: normal; font-style: normal; font-weight: "
                                 "normal\">%4</p>")
            .arg(ImgTag(number))
            .arg(number)
            .arg(trv->GuiText(name))
            .arg(trv->Description(name));

    ui->textEdit->setHtml(text);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::TreeMenu(const QPoint &pos)
{
    // Because item also will be selected need to show description
    const QModelIndex model = ui->treeWidget->currentIndex();
    QTreeWidgetItem *item = ui->treeWidget->currentItem();
    ShowDescription(item, model.column());

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
        GlobalCheckState() == Qt::Checked ? actionName = tr("Check all") : actionName = tr("Uncheck all");

        QAction *actionRecheck = new QAction(actionName, this);
        connect(actionRecheck, &QAction::triggered, this, &DialogMDataBase::Recheck);

        menu.addAction(actionRecheck);
    }
    menu.exec(ui->treeWidget->mapToGlobal(pos));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::Recheck()
{
    if (selectMode)
    {
        const Qt::CheckState check = GlobalCheckState();

        ChangeCheckState(groupA, check);
        ChangeCheckState(groupB, check);
        ChangeCheckState(groupC, check);
        ChangeCheckState(groupD, check);
        ChangeCheckState(groupE, check);
        ChangeCheckState(groupF, check);
        ChangeCheckState(groupG, check);
        ChangeCheckState(groupH, check);
        ChangeCheckState(groupI, check);
        ChangeCheckState(groupJ, check);
        ChangeCheckState(groupK, check);
        ChangeCheckState(groupL, check);
        ChangeCheckState(groupM, check);
        ChangeCheckState(groupN, check);
        ChangeCheckState(groupO, check);
        ChangeCheckState(groupP, check);
        ChangeCheckState(groupQ, check);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::InitDataBase(const QStringList &list)
{
    InitGroup(&groupA, groupAText, ListGroupA(), list);
    InitGroup(&groupB, groupBText, ListGroupB(), list);
    InitGroup(&groupC, groupCText, ListGroupC(), list);
    InitGroup(&groupD, groupDText, ListGroupD(), list);
    InitGroup(&groupE, groupEText, ListGroupE(), list);
    InitGroup(&groupF, groupFText, ListGroupF(), list);
    InitGroup(&groupG, groupGText, ListGroupG(), list);
    InitGroup(&groupH, groupHText, ListGroupH(), list);
    InitGroup(&groupI, groupIText, ListGroupI(), list);
    InitGroup(&groupJ, groupJText, ListGroupJ(), list);
    InitGroup(&groupK, groupKText, ListGroupK(), list);
    InitGroup(&groupL, groupLText, ListGroupL(), list);
    InitGroup(&groupM, groupMText, ListGroupM(), list);
    InitGroup(&groupN, groupNText, ListGroupN(), list);
    InitGroup(&groupO, groupOText, ListGroupO(), list);
    InitGroup(&groupP, groupPText, ListGroupP(), list);
    InitGroup(&groupQ, groupQText, ListGroupQ(), list);
}                                  
                                   
//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::InitGroup(QTreeWidgetItem **group, const QString &groupName, const QStringList &mList,
                                const QStringList &list)
{                                  
    *group = AddGroup(groupName);  
    for (int i=0; i < mList.size(); ++i)
    {                              
        AddMeasurement(*group, mList.at(i), list);
    }                              
}                                  
                                   
//---------------------------------------------------------------------------------------------------------------------
QTreeWidgetItem *DialogMDataBase::AddGroup(const QString &text)
{                                  
    QTreeWidgetItem *group = new QTreeWidgetItem(ui->treeWidget);
    group->setText(0, text);       
    group->setToolTip(0, text);    
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
void DialogMDataBase::AddMeasurement(QTreeWidgetItem *group, const QString &name, const QStringList &list)
{                                  
    SCASSERT(group != nullptr)     
                                   
    QTreeWidgetItem *m = new QTreeWidgetItem(group);
                                   
    if (selectMode)                
    {                              
        if (list.contains(name))   
        {                          
            m->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            m->setCheckState(0, Qt::Checked);
            m->setBackground(0, QBrush(Qt::yellow));
        }                          
        else                       
        {                          
            m->setCheckState(0, Qt::Unchecked);
        }                          
                                   
        UpdateChecks(m, 0);        
    }                              
                                   
    const QString text = qApp->TrVars()->MNumber(name) + ". " + qApp->TrVars()->MToUser(name);
    m->setText(0, text);           
    m->setToolTip(0, text);        
    m->setData(0, Qt::UserRole, name);
}                                  
                                   
//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ReadSettings()
{                                  
    restoreGeometry(qApp->SeamlyMeSettings()->GetDataBaseGeometry());
}                                  
                                   
//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::WriteSettings()
{                                  
    qApp->SeamlyMeSettings()->SetDataBaseGeometry(saveGeometry());
}                                  

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::Filter(const QString term)
{

    if (term.isEmpty())
    {
        InitDataBase(list);
        return;
    }

    if (ListGroupA().filter(term).count() > 0)
    {
        InitGroup(&groupA, groupAText, ListGroupA().filter(term), list);
    }

    if (ListGroupB().filter(term).count() > 0)
    {
        InitGroup(&groupB, groupBText, ListGroupB().filter(term), list);
    }

    if (ListGroupC().filter(term).count() > 0)
    {
        InitGroup(&groupC, groupCText, ListGroupC().filter(term), list);
    }

    if (ListGroupD().filter(term).count() > 0)
    {
        InitGroup(&groupD, groupDText, ListGroupD().filter(term), list);
    }

    if (ListGroupE().filter(term).count() > 0)
    {
        InitGroup(&groupE, groupEText, ListGroupE().filter(term), list);
    }

    if (ListGroupF().filter(term).count() > 0)
    {
        InitGroup(&groupF, groupFText, ListGroupF().filter(term), list);
    }

    if (ListGroupG().filter(term).count() > 0)
    {
        InitGroup(&groupG, groupGText, ListGroupG().filter(term), list);
    }

    if (ListGroupH().filter(term).count() > 0)
    {
        InitGroup(&groupH, groupHText, ListGroupH().filter(term), list);
    }

    if (ListGroupI().filter(term).count() > 0)
    {
        InitGroup(&groupI, groupIText, ListGroupI().filter(term), list);
    }

    if (ListGroupJ().filter(term).count() > 0)
    {
        InitGroup(&groupJ, groupJText, ListGroupJ().filter(term), list);
    }

    if (ListGroupK().filter(term).count() > 0)
    {
        InitGroup(&groupK, groupKText, ListGroupK().filter(term), list);
    }

    if (ListGroupL().filter(term).count() > 0)
    {
        InitGroup(&groupL, groupLText, ListGroupL().filter(term), list);
    }

    if (ListGroupM().filter(term).count() > 0)
    {
        InitGroup(&groupM, groupMText, ListGroupM().filter(term), list);
    }

    if (ListGroupN().filter(term).count() > 0)
    {
        InitGroup(&groupN, groupNText, ListGroupN().filter(term), list);
    }

    if (ListGroupO().filter(term).count() > 0)
    {
        InitGroup(&groupO, groupOText, ListGroupO().filter(term), list);
    }

    if (ListGroupP().filter(term).count() > 0)
    {
        InitGroup(&groupP, groupPText, ListGroupP().filter(term), list);
    }

    if (ListGroupQ().filter(term).count() > 0)
    {
        InitGroup(&groupQ, groupQText, ListGroupQ().filter(term), list);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::RetranslateGroup(QTreeWidgetItem *group, const QString &groupText, const QStringList &list)
{
    group->setText(0, groupText);
    group->setToolTip(0, groupText);

    for (int i=0; i<list.size(); ++i)
    {
        RetranslateMeasurement(group, i, list.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::RetranslateMeasurement(QTreeWidgetItem *group, int index, const QString &name)
{
    const QString text = qApp->TrVars()->MNumber(name) + ". " + qApp->TrVars()->MToUser(name);

    QTreeWidgetItem *m = group->child(index);
    m->setText(0, text);
    m->setToolTip(0, text);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogMDataBase::ChangeCheckState(QTreeWidgetItem *group, Qt::CheckState check)
{
    SCASSERT(group != nullptr)
    group->setCheckState(0, check);
}

//---------------------------------------------------------------------------------------------------------------------
Qt::CheckState DialogMDataBase::GlobalCheckState() const
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

//-----------------------------------------------------------------------------
//  @file   dialogtool.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2022 Seamly, LLC
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
//
//  @file   dialogtool.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
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

#include "dialogtool.h"

#include <limits.h>
#include <qiterator.h>
#include <qnumeric.h>
#include <QCloseEvent>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QEvent>
#include <QGuiApplication>
#include <QHash>
#include <QIcon>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMapIterator>
#include <QMessageLogger>
#include <QPalette>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRect>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QScopedPointer>
#include <QScreen>
#include <QSharedPointer>
#include <QShowEvent>
#include <QSize>
#include <QTextCursor>
#include <QTimer>
#include <QWidget>
#include <Qt>
#include <QtDebug>
#include <new>
#include <QBuffer>
#include <QFont>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vpatterndb/vpiecenode.h"
#include "../../tools/vabstracttool.h"

template <class T> class QSharedPointer;

Q_LOGGING_CATEGORY(vDialog, "v.dialog")

#define DIALOG_MAX_FORMULA_HEIGHT 64
#define DIALOG_MIN_WIDTH 260

namespace
{

//---------------------------------------------------------------------------------------------------------------------
quint32 RowId(QListWidget *listWidget, int i)
{
    SCASSERT(listWidget != nullptr);

    if (i < 0 || i >= listWidget->count())
    {
        return NULL_ID;
    }

    const QListWidgetItem *rowItem = listWidget->item(i);
    SCASSERT(rowItem != nullptr);
    const VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
    return rowNode.GetId();
}
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogTool create dialog
 * @param data container with data
 * @param parent parent widget
 */
DialogTool:: DialogTool(const VContainer *data, const quint32 &toolId, QWidget *parent)
    : QDialog(parent)
    , data(data)
    , isInitialized(false)
    , flagName(true)
    , flagFormula(true)
    , flagError(true)
    , timerFormula(nullptr)
    , ok_Button(nullptr)
    , apply_Button(nullptr)
    , spinBoxAngle(nullptr)
    , plainTextEditFormula(nullptr)
    , labelResultCalculation(nullptr)
    , labelEditNamePoint(nullptr)
    , labelEditFormula(nullptr)
    , okColor(this->palette().color(QPalette::Active, QPalette::WindowText))
    , errorColor(Qt::red)
    , associatedTool(nullptr)
    , toolId(toolId)
    , prepare(false)
    , pointName()
    , number(0)
    , vis(nullptr)
    , m_screen(QGuiApplication::primaryScreen())
{
    SCASSERT(data != nullptr)
    timerFormula = new QTimer(this);
    connect(timerFormula, &QTimer::timeout, this, &DialogTool::EvalFormula);

    QList<QScreen *> screenList = QGuiApplication::screens();

    // If second monitor exists and user pref is to use second monitor, open dialogs on second monitor.
    if (qApp->Settings()->useSecondMonitor() && (screenList.count() > 1))
    {
        m_screen = screenList.at(1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
DialogTool::~DialogTool()
{
    emit ToolTip("");

    if (!vis.isNull())
    {
        delete vis;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            DialogRejected();
            return; // After reject the dialog will be destroyed, exit imidiately
        default:
            break;
    }
    QDialog::keyPressEvent ( event );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle when dialog cloded
 * @param event event
 */
void DialogTool::closeEvent(QCloseEvent *event)
{
    DialogRejected();
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showEvent handle when window show
 * @param event event
 */
void DialogTool::showEvent(QShowEvent *event)
{
    QDialog::showEvent( event );
    if ( event->spontaneous() )
    {
        return;
    }
    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    setMaximumSize(size());
    setMinimumSize(size());

    isInitialized = true;//first show windows are held
    ShowVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxPiecesList(QComboBox *box, const QVector<quint32> &list)
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);
    box->clear();
    for (int i=0; i < list.size(); ++i)
    {
        box->addItem(data->GetPiece(list.at(i)).GetName(), list.at(i));
    }
    box->blockSignals(false);
    box->setCurrentIndex(-1); // Force a user to choose
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief fillComboBoxPoints fill comboBox list of points
 * @param box comboBox
 */
void DialogTool::fillComboBoxPoints(QComboBox *box, FillComboBox rule, const quint32 &ch1, const quint32 &ch2) const
{
    FillCombo<VPointF>(box, GOType::Point, rule, ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxArcs(QComboBox *box, FillComboBox rule, const quint32 &ch1, const quint32 &ch2) const
{
    FillCombo<VAbstractCurve>(box, GOType::Arc, rule, ch1, ch2);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxSplines(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const auto objs = data->DataGObjects();
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;
    QMap<QString, quint32> list;
    for (i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId)
        {
            if (IsSpline(i.value()))
            {
                PrepareList<VAbstractCurve>(list, i.key());
            }
        }
    }
    FillList(box, list);

    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxSplinesPath(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const auto objs = data->DataGObjects();
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;
    QMap<QString, quint32> list;
    for (i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId)
        {
            if (IsSplinePath(i.value()))
            {
                PrepareList<VAbstractCurve>(list, i.key());
            }
        }
    }
    FillList(box, list);

    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxCurves(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    const auto objs = data->DataGObjects();
    QMap<QString, quint32> list;
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;
    for (i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (i.key() != toolId)
        {
            QSharedPointer<VGObject> obj = i.value();
            if ((obj->getType() == GOType::Arc
                 || obj->getType() == GOType::EllipticalArc
                 || obj->getType() == GOType::Spline
                 || obj->getType() == GOType::SplinePath
                 || obj->getType() == GOType::CubicBezier
                 || obj->getType() == GOType::CubicBezierPath) && obj->getMode() == Draw::Calculation)
            {
                PrepareList<VAbstractCurve>(list, i.key());
            }
        }
    }
    FillList(box, list);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillComboBoxTypeLine fill comboBox list of type lines
 * @param box comboBox
 */
void DialogTool::FillComboBoxTypeLine(QComboBox *box, const QMap<QString, QIcon> &stylesPics) const
{
    SCASSERT(box != nullptr)
    QMap<QString, QIcon>::const_iterator i = stylesPics.constBegin();
    while (i != stylesPics.constEnd())
    {
        box->addItem(i.value(), "", QVariant(i.key()));
        ++i;
    }

    const int index = box->findData(QVariant(LineTypeSolidLine));
    if (index != -1)
    {
        box->setCurrentIndex(index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxLineColors(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->clear();
    int size = box->iconSize().height();
    // On Mac pixmap should be little bit smaller.
#if defined(Q_OS_MAC)
    size -= 2; // Two pixels should be enough.
#endif //defined(Q_OS_MAC)

    const QMap<QString, QString> map = VAbstractTool::ColorsList();
    QMap<QString, QString>::const_iterator i = map.constBegin();
    while (i != map.constEnd())
    {
        QPixmap pix(size, size);
        pix.fill(QColor(i.key()));
        box->addItem(QIcon(pix), i.value(), QVariant(i.key()));
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxCrossCirclesPoints(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("First point"), QVariant(static_cast<int>(CrossCirclesPoint::FirstPoint)));
    box->addItem(tr("Second point"), QVariant(static_cast<int>(CrossCirclesPoint::SecondPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxVCrossCurvesPoint(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Highest point"), QVariant(static_cast<int>(VCrossCurvesPoint::HighestPoint)));
    box->addItem(tr("Lowest point"), QVariant(static_cast<int>(VCrossCurvesPoint::LowestPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FillComboBoxHCrossCurvesPoint(QComboBox *box) const
{
    SCASSERT(box != nullptr)

    box->addItem(tr("Leftmost point"), QVariant(static_cast<int>(HCrossCurvesPoint::LeftmostPoint)));
    box->addItem(tr("Rightmost point"), QVariant(static_cast<int>(HCrossCurvesPoint::RightmostPoint)));
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogTool::GetComboBoxCurrentData(const QComboBox *box, const QString &def) const
{
    SCASSERT(box != nullptr)
    QString value;
    value = box->currentData().toString();

    if (value.isEmpty())
    {
        value = def;
    }
    return value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangeCurrentData select item in combobox by id
 * @param box combobox
 * @param value id of item
 */
void DialogTool::ChangeCurrentData(QComboBox *box, const QVariant &value) const
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findData(value);
    if (index != -1)
    {
        box->blockSignals(true);
        box->setCurrentIndex(index);
        box->blockSignals(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::MoveCursorToEnd(QPlainTextEdit *plainTextEdit) const
{
    SCASSERT(plainTextEdit != nullptr)
    QTextCursor cursor = plainTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    plainTextEdit->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::eventFilter(QObject *object, QEvent *event)
{
    if (QPlainTextEdit *plainTextEdit = qobject_cast<QPlainTextEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return))
            {
                // Ignore Enter key
                return true;
            }
            else if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (qApp->Settings()->getOsSeparator())
                {
                    plainTextEdit->insertPlainText(QLocale().decimalPoint());
                }
                else
                {
                    plainTextEdit->insertPlainText(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }
    else
    {
        // pass the event on to the parent class
        return QDialog::eventFilter(object, event);
    }
    return false;// pass the event to the widget
}

//---------------------------------------------------------------------------------------------------------------------
quint32 DialogTool::DNumber(const QString &baseName) const
{
    quint32 num = 0;
    QString name;
    do
    {
        ++num;
        name = baseName + QString("_%1").arg(num);
    } while (!data->IsUnique(name));

    return num;
}

//---------------------------------------------------------------------------------------------------------------------
int DialogTool::FindNotExcludedNodeDown(QListWidget *listWidget, int candidate)
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr);
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (!rowNode.isExcluded())
        {
            index = i;
        }

        ++i;
    }
    while (rowNode.isExcluded() && i < listWidget->count());

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
int DialogTool::FindNotExcludedNodeUp(QListWidget *listWidget, int candidate)
{
    SCASSERT(listWidget != nullptr);

    int index = -1;
    if (candidate < 0 || candidate >= listWidget->count())
    {
        return index;
    }

    int i = candidate;
    VPieceNode rowNode;
    do
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr);
        rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));

        if (!rowNode.isExcluded())
        {
            index = i;
        }

        --i;
    }
    while (rowNode.isExcluded() && i > -1);

    return index;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::isFirstPointSameAsLast(QListWidget *listWidget)
{
    SCASSERT(listWidget != nullptr);
    if (listWidget->count() > 1)
    {
        const quint32 topId = RowId(listWidget, FindNotExcludedNodeDown(listWidget, 0));
        const quint32 bottomId = RowId(listWidget, FindNotExcludedNodeUp(listWidget, listWidget->count()-1));
        return topId == bottomId;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::doublePointsExist(QListWidget *listWidget)
{
    SCASSERT(listWidget != nullptr);
    for (int i=0, sz = listWidget->count()-1; i<sz; ++i)
    {
        const int firstIndex = FindNotExcludedNodeDown(listWidget, i);
        const quint32 firstId = RowId(listWidget, firstIndex);
        const quint32 secondId = RowId(listWidget, FindNotExcludedNodeDown(listWidget, firstIndex+1));

        if (firstId == secondId)
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::isEachPointNameUnique(QListWidget *listWidget)
{
    SCASSERT(listWidget != nullptr);
    QSet<quint32> pointLabels;
    int countPoints = 0;
    for (int i=0; i < listWidget->count(); ++i)
    {
        const QListWidgetItem *rowItem = listWidget->item(i);
        SCASSERT(rowItem != nullptr);
        const VPieceNode rowNode = qvariant_cast<VPieceNode>(rowItem->data(Qt::UserRole));
        if (rowNode.GetTypeTool() == Tool::NodePoint)
        {
            ++countPoints;
            pointLabels.insert(rowNode.GetId());
        }
    }

    return countPoints == pointLabels.size();
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogTool::DialogWarningIcon()
{
    const QIcon icon = QIcon::fromTheme("dialog-warning",
                                  QIcon(":/icons/win.icon.theme/16x16/status/dialog-warning.png"));

    const QPixmap pixmap = icon.pixmap(QSize(16, 16));
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    pixmap.save(&buffer, "PNG");
    const QString url = QString("<img src=\"data:image/png;base64,") + byteArray.toBase64() + QLatin1String("\"/> ");
    return url;
}

//---------------------------------------------------------------------------------------------------------------------
QFont DialogTool::NodeFont(bool nodeExcluded)
{
    QFont font = qApp->Settings()->getGuiFont();
    font.setPointSize(qApp->Settings()->getGuiFontSize());
    font.setBold(true);
    font.setStrikeOut(nodeExcluded);
    return font;
}

//---------------------------------------------------------------------------------------------------------------------
NodeInfo DialogTool::getNodeInfo(const VPieceNode &node, bool showNotch) const
{
    NodeInfo info;
    const QSharedPointer<VGObject> obj = data->GetGObject(node.GetId());
    info.name = obj->name();
    info.icon = "://icon/24x24/spacer.png";

    if (node.GetTypeTool() != Tool::NodePoint)
    {
        int bias = 0;
        qApp->translateVariables()->VariablesToUser(info.name, 0, obj->name(), bias);

        if (node.GetReverse())
        {
            info.icon = "://icon/24x24/reverse.png";
        }
    }
    else if (showNotch && node.isNotch())
    {
        switch(node.getNotchType())
        {
            case NotchType::TNotch:
                info.icon = "://icon/24x24/t_notch.png";
                break;
            case NotchType::UNotch:
                info.icon = "://icon/24x24/u_notch.png";
                break;
            case NotchType::VInternal:
                info.icon = "://icon/24x24/internal_v_notch.png";
                break;
            case NotchType::VExternal:
                info.icon = "://icon/24x24/external_v_notch.png";
                break;
            case NotchType::Castle:
                info.icon = "://icon/24x24/castle_notch.png";
                break;
            case NotchType::Diamond:
                info.icon = "://icon/24x24/diamond_notch.png";
                break;
            case NotchType::Slit:
                info.icon = "://icon/24x24/slit_notch.png";
                break;
            default:
                break;
        }
    }

    return info;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::newNodeItem(QListWidget *listWidget, const VPieceNode &node, bool nodeExcluded, bool isDuplicate)
{
    SCASSERT(listWidget != nullptr);
    SCASSERT(node.GetId() > NULL_ID);
    NodeInfo info;
    switch (node.GetTypeTool())
    {
        case (Tool::NodePoint):
        case (Tool::NodeArc):
        case (Tool::NodeElArc):
        case (Tool::NodeSpline):
        case (Tool::NodeSplinePath):
            info = getNodeInfo(node, true);
            break;
        default:
            qWarning() << "Got wrong tools. Ignore.";
            return;
    }

    bool newNodeAllowed = false;

    if(listWidget->count() == 0 || isDuplicate || RowId(listWidget, listWidget->count()-1) != node.GetId())
    {
        newNodeAllowed = true;
    }

    if(newNodeAllowed)
    {
        QListWidgetItem *item = new QListWidgetItem(info.name);
        item->setFont(NodeFont(nodeExcluded ? node.isExcluded() : false));
        item->setData(Qt::UserRole, QVariant::fromValue(node));
        item->setIcon(QIcon(info.icon));
        listWidget->addItem(item);
        listWidget->setCurrentRow(listWidget->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::initializeNodeAngles(QComboBox *box)
{
    SCASSERT(box != nullptr);
    box->clear();

    box->addItem(tr("by length"), static_cast<unsigned char>(PieceNodeAngle::ByLength));
    box->addItem(tr("by points intersetions"), static_cast<unsigned char>(PieceNodeAngle::ByPointsIntersection));
    box->addItem(tr("by first edge symmetry"), static_cast<unsigned char>(PieceNodeAngle::ByFirstEdgeSymmetry));
    box->addItem(tr("by second edge symmetry"), static_cast<unsigned char>(PieceNodeAngle::BySecondEdgeSymmetry));
    box->addItem(tr("by first edge right angle"), static_cast<unsigned char>(PieceNodeAngle::ByFirstEdgeRightAngle));
    box->addItem(tr("by second edge right angle"), static_cast<unsigned char>(PieceNodeAngle::BySecondEdgeRightAngle));
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::IsSplinePath(const QSharedPointer<VGObject> &obj) const
{
    return (obj->getType() == GOType::SplinePath || obj->getType() == GOType::CubicBezierPath) &&
            obj->getMode() == Draw::Calculation;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ValFormulaChanged handle change formula
 * @param flag flag state of formula
 * @param edit LineEdit
 * @param timer timer of formula
 */
void DialogTool::ValFormulaChanged(bool &flag, QLineEdit *edit, QTimer *timer, const QString& postfix)
{
    SCASSERT(edit != nullptr)
    SCASSERT(timer != nullptr)
    SCASSERT(labelEditFormula != nullptr)
    SCASSERT(labelResultCalculation != nullptr)
    if (edit->text().isEmpty())
    {
        flag = false;
        CheckState();
        ChangeColor(labelEditFormula, Qt::red);
        if (postfix.isEmpty())
        {
            labelResultCalculation->setText(tr("Error"));
        }
        else
        {
            labelResultCalculation->setText(tr("Error") + " (" + postfix + ")");
        }
        labelResultCalculation->setToolTip(tr("Empty field"));
        return;
    }
    timer->start(1000);
}
//---------------------------------------------------------------------------------------------------------------------
void DialogTool::ValFormulaChanged(bool &flag, QPlainTextEdit *edit, QTimer *timer, const QString& postfix)
{
    SCASSERT(edit != nullptr)
    SCASSERT(timer != nullptr)
    SCASSERT(labelEditFormula != nullptr)
    SCASSERT(labelResultCalculation != nullptr)
    if (edit->toPlainText().isEmpty())
    {
        flag = false;
        CheckState();
        ChangeColor(labelEditFormula, Qt::red);
        if (postfix.isEmpty())
        {
            labelResultCalculation->setText(tr("Error"));
        }
        else
        {
            labelResultCalculation->setText(tr("Error") + " (" + postfix + ")");
        }

        labelResultCalculation->setToolTip(tr("Empty field"));
        return;
    }
    timer->setSingleShot(true);
    timer->start(300);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Eval evaluate formula and show result
 * @param text expresion that we parse
 * @param flag flag state of eval formula
 * @param label label for signal error
 * @param postfix unit name
 * @param checkZero true - if formula can't be equal zero
 */
qreal DialogTool::Eval(const QString &text, bool &flag, QLabel *label, const QString& postfix, bool checkZero,
                       bool checkLessThanZero)
{
    SCASSERT(label != nullptr)
    SCASSERT(labelEditFormula != nullptr)

    qreal result = INT_MIN;//Value can be 0, so use max imposible value

    if (text.isEmpty())
    {
        flag = false;
        ChangeColor(labelEditFormula, Qt::red);
        label->setText(tr("Error") + " (" + postfix + ")");
        label->setToolTip(tr("Empty field"));
    }
    else
    {
        try
        {
            // Replace line return character with spaces for calc if exist
            QString formula = text;
            formula.replace("\n", " ");
            // Translate to internal look.
            formula = qApp->translateVariables()->FormulaFromUser(formula, qApp->Settings()->getOsSeparator());
            QScopedPointer<Calculator> cal(new Calculator());
            result = cal->EvalFormula(data->DataVariables(), formula);

            if (qIsInf(result) || qIsNaN(result))
            {
                flag = false;
                ChangeColor(labelEditFormula, Qt::red);
                label->setText(tr("Error") + " (" + postfix + ")");
                label->setToolTip(tr("Invalid result. Value is infinite or NaN. Please, check your calculations."));
            }
            else
            {
                //if result equal 0
                if (checkZero && qFuzzyIsNull(result))
                {
                    flag = false;
                    ChangeColor(labelEditFormula, Qt::red);
                    label->setText(tr("Error") + " (" + postfix + ")");
                    label->setToolTip(tr("Value can't be 0"));
                }
                else if (checkLessThanZero && result < 0)
                {
                    flag = false;
                    ChangeColor(labelEditFormula, Qt::red);
                    label->setText(tr("Error") + " (" + postfix + ")");
                    label->setToolTip(tr("Value can't be less than 0"));
                }
                else
                {
                    label->setText(qApp->LocaleToString(result) + " " +postfix);
                    flag = true;
                    ChangeColor(labelEditFormula, okColor);
                    label->setToolTip(tr("Result Value"));
                    emit ToolTip("");
                }
            }
        }
        catch (qmu::QmuParserError &error)
        {
            label->setText(tr("Error") + " (" + postfix + ")");
            flag = false;
            ChangeColor(labelEditFormula, Qt::red);
            emit ToolTip(tr("Parser error: %1").arg(error.GetMsg()));
            label->setToolTip(tr("Parser error: %1").arg(error.GetMsg()));
            qDebug() << "\nMath parser error:\n"
                       << "--------------------------------------\n"
                       << "Message:     " << error.GetMsg()  << "\n"
                       << "Expression:  " << error.GetExpr() << "\n"
                       << "--------------------------------------";
        }
    }
    CheckState(); // Disable Ok and Apply buttons if something wrong.
    return result;
}

// Normalizes any number to an arbitrary range
// by assuming the range wraps around when going below min or above max
qreal DialogTool::normalize( const qreal value, const qreal start, const qreal end )
{
  const qreal range       = end - start   ;   //
  const qreal offsetValue = value - start ;   // value relative to 0

  return ( offsetValue - ( floor( offsetValue / range ) * range ) ) + start ;
  // + start to reset back to start of original range
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::setCurrentPointId(QComboBox *box, const quint32 &value, FillComboBox rule,
                                   const quint32 &ch1, const quint32 &ch2) const
{
    SCASSERT(box != nullptr)

    box->blockSignals(true);

    fillComboBoxPoints(box, rule, ch1, ch2);
    ChangeCurrentData(box, value);

    box->blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentSplineId set current spline id in combobox
 */
void DialogTool::setCurrentSplineId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxSplines(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentArcId
 */
void DialogTool::setCurrentArcId(QComboBox *box, const quint32 &value, FillComboBox rule,
                                 const quint32 &ch1, const quint32 &ch2) const
{
    SCASSERT(box != nullptr)
    FillComboBoxArcs(box, rule, ch1, ch2);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setCurrentSplinePathId set current splinePath id in combobox
 * @param box combobox
 * @param value splinePath id
 */
void DialogTool::setCurrentSplinePathId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxSplinesPath(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::setCurrentCurveId(QComboBox *box, const quint32 &value) const
{
    SCASSERT(box != nullptr)
    FillComboBoxCurves(box);
    ChangeCurrentData(box, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getCurrentPointId return current point id stored in combobox
 * @param box combobox
 * @return id or 0 if combobox is empty
 */
quint32 DialogTool::getCurrentObjectId(QComboBox *box) const
{
    SCASSERT(box != nullptr)
    qint32 index = box->currentIndex();
    if (index != -1)
    {
        return qvariant_cast<quint32>(box->itemData(index));
    }
    else
    {
        return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::SetObject(const quint32 &id, QComboBox *box, const QString &toolTip)
{
    SCASSERT(box != nullptr)
    const qint32 index = box->findData(id);
    if ( index != -1 )
    { // -1 for not found
        box->setCurrentIndex(index);
        emit ToolTip(toolTip);
        return true;
    }
    else
    {
        qWarning() << "Can't find object by id"<<id;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::DeployFormula(QPlainTextEdit *formula, QPushButton *buttonGrowLength, int formulaBaseHeight)
{
    SCASSERT(formula != nullptr)
    SCASSERT(buttonGrowLength != nullptr)

    const QTextCursor cursor = formula->textCursor();

    if (formula->height() < DIALOG_MAX_FORMULA_HEIGHT)
    {
        setMaximumWidth(QWIDGETSIZE_MAX);
        formula->setFixedHeight(DIALOG_MAX_FORMULA_HEIGHT);
        //Set icon from theme (internal for Windows system)
        buttonGrowLength->setIcon(QIcon::fromTheme("go-up",
                                                   QIcon(":/icons/win.icon.theme/16x16/actions/go-up.png")));
    }
    else
    {
        setMaximumWidth(DIALOG_MIN_WIDTH);
        formula->setFixedHeight(formulaBaseHeight);
        //Set icon from theme (internal for Windows system)
        buttonGrowLength->setIcon(QIcon::fromTheme("go-down",
                                                   QIcon(":/icons/win.icon.theme/16x16/actions/go-down.png")));
    }

    // I found that after change size of formula field, it was filed for angle formula, field for formula became black.
    // This code prevent this.
    setUpdatesEnabled(false);
    repaint();
    setUpdatesEnabled(true);

    formula->setFocus();
    formula->setTextCursor(cursor);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FillList fill combobox list
 * @param box combobox
 * @param list list with ids and names
 */
void DialogTool::FillList(QComboBox *box, const QMap<QString, quint32> &list) const
{
    SCASSERT(box != nullptr)
    box->clear();

    QMapIterator<QString, quint32> iter(list);
    while (iter.hasNext())
    {
        iter.next();
        box->addItem(iter.key(), iter.value());
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void DialogTool::PrepareList(QMap<QString, quint32> &list, quint32 id) const
{
    const auto obj = data->GeometricObject<T>(id);
    SCASSERT(obj != nullptr)

    QString newName = obj->name();
    int bias = 0;
    if (qApp->translateVariables()->VariablesToUser(newName, 0, obj->name(), bias))
    {
        list[newName] = id;
    }
    else
    {
        list[obj->name()] = id;
    }
}

//---------------------------------------------------------------------------------------------------------------------
bool DialogTool::IsSpline(const QSharedPointer<VGObject> &obj) const
{
    return (obj->getType() == GOType::Spline || obj->getType() == GOType::CubicBezier) &&
            obj->getMode() == Draw::Calculation;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckState enable, when all is correct, or disable, when something wrong, button ok
 */
void DialogTool::CheckState()
{
    SCASSERT(ok_Button != nullptr)
    ok_Button->setEnabled(flagFormula && flagName && flagError);
    // In case dialog does not have an apply button
    if (apply_Button != nullptr)
    {
        apply_Button->setEnabled(ok_Button->isEnabled());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChosenObject gets id and type of selected object. Save right data and ignore wrong.
 * @param id id of point or detail
 * @param type type of object
 */
void DialogTool::ChosenObject(quint32 id, const SceneObject &type)
{
    Q_UNUSED(id)
    Q_UNUSED(type)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SelectedObject(bool selected, quint32 object, quint32 tool)
{
    Q_UNUSED(selected)
    Q_UNUSED(object)
    Q_UNUSED(tool)
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NamePointChanged check name of point
 */
void DialogTool::NamePointChanged()
{
    SCASSERT(labelEditNamePoint != nullptr)
    QLineEdit* edit = qobject_cast<QLineEdit*>(sender());
    if (edit)
    {
        QString name = edit->text();
        QRegularExpression rx(NameRegExp());
        if (name.isEmpty() || (pointName != name && data->IsUnique(name) == false) ||
            rx.match(name).hasMatch() == false)
        {
            flagName = false;
            ChangeColor(labelEditNamePoint, Qt::red);
        }
        else
        {
            flagName = true;
            ChangeColor(labelEditNamePoint, okColor);
        }
    }
    CheckState();
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::ChangeColor(QWidget *widget, const QColor &color)
{
    SCASSERT(widget != nullptr)
    QPalette palette = widget->palette();
    palette.setColor(QPalette::Active, widget->foregroundRole(), color);
    widget->setPalette(palette);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogAccepted save data and emit signal about closed dialog.
 */
void DialogTool::DialogAccepted()
{
    SaveData();
    emit DialogClosed(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::DialogApply()
{
    SaveData();
    emit DialogApplied();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogRejected emit signal dialog rejected
 */
void DialogTool::DialogRejected()
{
    emit DialogClosed(QDialog::Rejected);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief formula check formula
 */
void DialogTool::FormulaChanged()
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(sender());
    if (edit)
    {
        ValFormulaChanged(flagFormula, edit, timerFormula, UnitsToStr(qApp->patternUnit()));
    }
}
//---------------------------------------------------------------------------------------------------------------------
void DialogTool::FormulaChangedPlainText() //-V524
{
    QPlainTextEdit* edit = qobject_cast<QPlainTextEdit*>(sender());
    if (edit)
    {
        ValFormulaChanged(flagFormula, edit, timerFormula, UnitsToStr(qApp->patternUnit()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowUp set angle value 90 degree
 */
void DialogTool::ArrowUp()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(90);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowDown set angle value 270 degree
 */
void DialogTool::ArrowDown()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(270);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowLeft set angle value 180 degree
 */
void DialogTool::ArrowLeft()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(180);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowRight set angle value 0 degree
 */
void DialogTool::ArrowRight()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(0);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowLeftUp set angle value 135 degree
 */
void DialogTool::ArrowLeftUp()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(135);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowLeftDown set angle value 225 degree
 */
void DialogTool::ArrowLeftDown()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(225);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowRightUp set angle value 45 degree
 */
void DialogTool::ArrowRightUp()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(45);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ArrowRightDown set angle value 315 degree
 */
void DialogTool::ArrowRightDown()
{
    SCASSERT(spinBoxAngle != nullptr)
    spinBoxAngle->setValue(315);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EvalFormula evaluate formula
 */
void DialogTool::EvalFormula()
{
    SCASSERT(plainTextEditFormula != nullptr)
    SCASSERT(labelResultCalculation != nullptr)
    const QString postfix = UnitsToStr(qApp->patternUnit(), true);//Show unit in dialog lable (cm, mm or inch)
    Eval(plainTextEditFormula->toPlainText(), flagFormula, labelResultCalculation, postfix, false);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
quint32 DialogTool::GetToolId() const
{
    return toolId;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetToolId(const quint32 &value)
{
    toolId = value;
}

//---------------------------------------------------------------------------------------------------------------------
QString DialogTool::getPointName() const
{
    return pointName;
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::moveListRowTop(QListWidget *list)
{
    SCASSERT(list != nullptr)
    const int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex))
    {
        list->insertItem(0, currentItem);
        list->setCurrentRow(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::moveListRowUp(QListWidget *list)
{
    SCASSERT(list != nullptr)
    int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex--))
    {
        if (currentIndex < 0)
        {
            currentIndex = 0;
        }
        list->insertItem(currentIndex, currentItem);
        list->setCurrentRow(currentIndex);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::moveListRowDown(QListWidget *list)
{
    SCASSERT(list != nullptr)
    int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex++))
    {
        if (currentIndex > list->count())
        {
            currentIndex = list->count();
        }
        list->insertItem(currentIndex, currentItem);
        list->setCurrentRow(currentIndex);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::moveListRowBottom(QListWidget *list)
{
    SCASSERT(list != nullptr)
    const int currentIndex = list->currentRow();
    if (QListWidgetItem *currentItem = list->takeItem(currentIndex))
    {
        list->insertItem(list->count(), currentItem);
        list->setCurrentRow(list->count()-1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::ShowDialog(bool click)
{
    Q_UNUSED(click)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::Build(const Tool &type)
{
    Q_UNUSED(type)
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetPiecesList(const QVector<quint32> &list)
{
    Q_UNUSED(list);
}

//---------------------------------------------------------------------------------------------------------------------
void DialogTool::SetAssociatedTool(VAbstractTool *tool)
{
    if (tool != nullptr)
    {
        associatedTool = tool;
        SetToolId(tool->getId());
        data = tool->getData();
        if (!vis.isNull())
        {
            vis->setData(data);
        }
    }
    else
    {
        associatedTool = nullptr;
        SetToolId(NULL_ID);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename GObject>
void DialogTool::FillCombo(QComboBox *box, GOType gType, FillComboBox rule, const quint32 &ch1,
                           const quint32 &ch2) const
{
    SCASSERT(box != nullptr)
    box->blockSignals(true);

    const QHash<quint32, QSharedPointer<VGObject> > *objs = data->DataGObjects();
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;
    QMap<QString, quint32> list;
    for (i = objs->constBegin(); i != objs->constEnd(); ++i)
    {
        if (rule == FillComboBox::NoChildren)
        {
            if (i.key() != toolId && i.value()->getIdTool() != toolId && i.key() != ch1 && i.key() != ch2)
            {
                QSharedPointer<VGObject> obj = i.value();
                if (obj->getType() == gType && obj->getMode() == Draw::Calculation)
                {
                    PrepareList<GObject>(list, i.key());
                }
            }
        }
        else
        {
            if (i.key() != toolId && i.value()->getIdTool() != toolId)
            {
                QSharedPointer<VGObject> obj = i.value();
                if (obj->getType() == gType && obj->getMode() == Draw::Calculation)
                {
                    PrepareList<GObject>(list, i.key());
                }
            }
        }
    }
    FillList(box, list);

    box->blockSignals(false);
}

// @brief setDialogPosition set position to open dialogs
//
// This method sets the screen position of dialogs based on the user preference setting
//
// @details
//  - Get dialog prefered possition from settings.
//  - Determine geometry of screen and dialog.
//  - Move dialog to prefered screen position with margin applied.
//  - Positions include Top left, Top right, Center, Bottom Left, and Botton right corner of screen.
void  DialogTool::setDialogPosition()
{
    int position = qApp->Settings()->getDialogPosition();
    QRect screenRect = m_screen->availableGeometry();
    QRect dialogRect = frameGeometry();

    switch(static_cast<DialogPosition>(position))
    {
        case DialogPosition::TopLeft:
        {
            move(screenRect.topLeft() + QPoint(10, 10));
            break;
        }
        case DialogPosition::TopRight:
        {
            move(screenRect.topRight() - dialogRect.topRight() + QPoint(-10 , 10));
            break;
        }
        case DialogPosition::BottomLeft:
        {
            move(screenRect.bottomLeft() - dialogRect.bottomLeft() + QPoint(10, -40));
            break;
        }
        case DialogPosition::BottomRight:
        {
            move(screenRect.bottomRight() - dialogRect.bottomRight() + QPoint(-10, -40));
            break;
        }
        case DialogPosition::Offset:
        {
            int xOffset = qApp->Settings()->getXOffset();
            if (xOffset > screenRect.width() - dialogRect.width())
            {
                xOffset = screenRect.width() - dialogRect.width();
            }

            int yOffset = qApp->Settings()->getYOffset();
            if (yOffset > screenRect.height() - dialogRect.height())
            {
                yOffset = screenRect.height() - dialogRect.height();
            }
            move(screenRect.topLeft() + QPoint(xOffset, yOffset - 40));
            break;
        }
        case DialogPosition::Center:
        default:
        {
            move(screenRect.center() - dialogRect.center());
            break;
        }
    }
}

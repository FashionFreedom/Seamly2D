/***************************************************************************
 **  @file   historyDialog.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @copyright
 **  Copyright (C) 2015 - 2023 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

 /************************************************************************
 **
 **  @file   dialoghistory.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "history_dialog.h"
#include "ui_history_dialog.h"

#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/diagnostic.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../xml/vpattern.h"

#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief HistoryDialog create dialog
 * @param data container with data
 * @param doc dom document container
 * @param parent parent widget
 */
HistoryDialog::HistoryDialog(VContainer *data, VPattern *doc, QWidget *parent)
    : DialogTool(data, 0, parent)
    , ui(new Ui::HistoryDialog)
    , m_doc(doc)
    , m_cursorRow(0)
    , m_cursorToolRecordRow(0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setWindowFlags((windowFlags() | Qt::WindowStaysOnTopHint | Qt::WindowMaximizeButtonHint) & ~Qt::WindowContextHelpButtonHint);

    //Limit dialog height to 80% of screen size
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    ui->find_LineEdit->installEventFilter(this);

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    fillTable();
    initializeTable();

    ok_Button = ui->buttonBox->button(QDialogButtonBox::Ok);
    connect(ok_Button,                &QPushButton::clicked,           this,  &HistoryDialog::DialogAccepted);
    connect(ui->clipboard_ToolButton, &QToolButton::clicked,           this,  &HistoryDialog::copyToClipboard);
    connect(ui->tableWidget,          &QTableWidget::cellClicked,      this,  &HistoryDialog::cellClicked);
    connect(m_doc,                    &VPattern::ChangedCursor,        this,  &HistoryDialog::changedCursor);
    connect(m_doc,                    &VPattern::patternChanged,       this,  &HistoryDialog::updateHistory);
    connect(ui->find_LineEdit,        &QLineEdit::textEdited,          this,  &HistoryDialog::findText);
    connect(this,                     &HistoryDialog::showHistoryTool, m_doc, [doc](quint32 id, bool enable)
    {
        emit doc->ShowTool(id, enable);
    });

    showTool();
}

//---------------------------------------------------------------------------------------------------------------------
HistoryDialog::~HistoryDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief DialogAccepted save data and emit signal about closed dialog.
 */
void HistoryDialog::DialogAccepted()
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
    quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
    emit showHistoryTool(id, false);
    emit DialogClosed(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief cellClicked changed history record
 * @param row number row in table
 * @param column number column in table
 */
void HistoryDialog::cellClicked(int row, int column)
{
    if (column == 0)
    {
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
        item->setIcon(QIcon());

        item = ui->tableWidget->item(row, 0);
        m_cursorRow = row;
        item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));
        const quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        m_doc->blockSignals(true);
        row == ui->tableWidget->rowCount()-1 ? m_doc->setCursor(0) : m_doc->setCursor(id);
        m_doc->blockSignals(false);
    }
    else
    {
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
        quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, false);

        m_cursorToolRecordRow = row;
        item = ui->tableWidget->item(m_cursorToolRecordRow, 0);
        id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief changedCursor changed cursor of input. Cursor show after which record we will insert new object
 * @param id id of object
 */
void HistoryDialog::changedCursor(quint32 id)
{
    for (qint32 i = 0; i< ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        quint32 rId = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (rId == id)
        {
            QTableWidgetItem *oldCursorItem = ui->tableWidget->item(m_cursorRow, 0);
            oldCursorItem->setIcon(QIcon());
            m_cursorRow = i;
            item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief updateHistory update history table
 */
void HistoryDialog::updateHistory()
{
    fillTable();
    initializeTable();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief fillTable fill table
 */
void HistoryDialog::fillTable()
{
    ui->tableWidget->clear();
    QVector<VToolRecord> history = m_doc->getBlockHistory();
    qint32 currentRow = -1;
    qint32 count = 0;
    ui->tableWidget->setRowCount(history.size());//Set Row count to number of Tool history records

    for (qint32 i = 0; i< history.size(); ++i)
    {
        const VToolRecord tool = history.at(i);
        const RowData rowData = record(tool);
        if (!rowData.tool.isEmpty())
        {
            currentRow++;

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString().setNum(rowData.id));
                item->setTextAlignment(Qt::AlignHCenter);
                item->setSizeHint(QSize(80, 24));
                item->setData(Qt::UserRole, rowData.id);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentRow, 0, item);//2nd column is Tool id
            }

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString(rowData.name));
                item->setTextAlignment(Qt::AlignHCenter);
                item->setSizeHint(QSize(100, 24));
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentRow, 1, item);//3rd column is Tool name
            }

            QTableWidgetItem *item = new QTableWidgetItem(rowData.tool);
            item->setTextAlignment(Qt::AlignLeft);
            item->setFlags(item->flags() ^ Qt::ItemIsEditable);
            item->setIcon(QIcon(rowData.icon));
            ui->tableWidget->setItem(currentRow, 2, item);//4th column is Tool description
            ++count;
        }
    }
    ui->tableWidget->setRowCount(count);//Current history records count
    if (count>0)
    {
        m_cursorRow = cursorRow();
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
        SCASSERT(item != nullptr)
        item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));//place arrow in 1st column of most recent history record
    }

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(24);//Set all row heights to 20px
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief Record return description for record
 * @param tool record data
 * @return RowData
 */
RowData HistoryDialog::record(const VToolRecord &tool)
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 54, "Not all tools were used in history.");

    RowData rowData;
    const quint32 &toolId = tool.getId();
    rowData.id = toolId;
    const QDomElement domElement = m_doc->elementById(toolId);
    if (domElement.isElement() == false)
    {
        qDebug() << "Can't find element by id" << Q_FUNC_INFO;
        return rowData;
    }
    try
    {
        switch (tool.getTypeTool())
        {
            case Tool::Arrow:
            case Tool::SinglePoint:
            case Tool::DoublePoint:
            case Tool::LinePoint:
            case Tool::AbstractSpline:
            case Tool::Cut:
            case Tool::Midpoint:// Same as Tool::AlongLine, but tool will never has such type
            case Tool::ArcIntersectAxis:// Same as Tool::CurveIntersectAxis, but tool will never has such type
            case Tool::BackgroundImage:
            case Tool::LAST_ONE_DO_NOT_USE:
                Q_UNREACHABLE(); //-V501
                break;

            case Tool::BasePoint:
                rowData.icon = ":/toolicon/32x32/point_basepoint_icon.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Base Point");
                break;

            case Tool::EndLine:
                rowData.icon = ":/toolicon/32x32/segment.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Length and Angle from point %1")
                                  .arg(getPointName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::Line:
                rowData.icon = ":/toolicon/32x32/line.png";
                rowData.name = tr("Line_%1_%2")
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                rowData.tool = tr("Line from %1 to %2")
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::AlongLine:
                rowData.icon = ":/toolicon/32x32/along_line.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Line %1_%2")
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::ShoulderPoint:
                rowData.icon = ":/toolicon/32x32/shoulder.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Length to Line");
                break;

            case Tool::Normal:
                rowData.icon = ":/toolicon/32x32/normal.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Perpendicular %1_%2")
                                 .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                 .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::Bisector:
                rowData.icon = ":/toolicon/32x32/bisector.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Bisector %1_%2_%3")
                                 .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                 .arg(getPointName(attrUInt(domElement, AttrSecondPoint)))
                                 .arg(getPointName(attrUInt(domElement, AttrThirdPoint)));
                break;

            case Tool::LineIntersect:
                rowData.icon = ":/toolicon/32x32/intersect.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Lines %1_%2 and %3_%4")
                                 .arg(getPointName(attrUInt(domElement, AttrP1Line1)))
                                 .arg(getPointName(attrUInt(domElement, AttrP2Line1)))
                                 .arg(getPointName(attrUInt(domElement, AttrP1Line2)))
                                 .arg(getPointName(attrUInt(domElement, AttrP2Line2)));
                break;

            case Tool::Spline:
            {
                const QSharedPointer<VSpline> spl = data->GeometricObject<VSpline>(toolId);
                SCASSERT(!spl.isNull())
                rowData.icon = ":/toolicon/32x32/spline.png";
                rowData.name = tr("%1").arg(spl->NameForHistory(tr("Spl_")));
                rowData.tool = tr("Curve Interactive");
                break;
            }

            case Tool::CubicBezier:
            {
                const QSharedPointer<VCubicBezier> spl = data->GeometricObject<VCubicBezier>(toolId);
                SCASSERT(!spl.isNull())
                rowData.icon = ":/toolicon/32x32/cubic_bezier.png";
                rowData.name = tr("%1").arg(spl->NameForHistory(tr("Spl_")));
                rowData.tool = tr("Curve Fixed");
                break;
            }

            case Tool::Arc:
            {
                const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(toolId);
                SCASSERT(!arc.isNull())
                rowData.icon = ":/toolicon/32x32/arc.png";
                rowData.name = tr("%1").arg(arc->NameForHistory(tr("Arc_")));
                rowData.tool = tr("Arc Radius & Angles");
                break;
            }

            case Tool::ArcWithLength:
            {
                const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(toolId);
                SCASSERT(!arc.isNull())
                rowData.icon = ":/toolicon/32x32/arc_with_length.png";
                rowData.name =tr("%1").arg(arc->NameForHistory(tr("Arc_")));
                rowData.tool =tr("Arc Radius & Length %1") .arg(arc->NameForHistory(tr("Arc_")));
                break;
            }

            case Tool::SplinePath:
            {
                const QSharedPointer<VSplinePath> splPath = data->GeometricObject<VSplinePath>(toolId);
                SCASSERT(!splPath.isNull())
                rowData.icon = ":/toolicon/32x32/splinePath.png";
                rowData.name = tr("%1").arg(splPath->NameForHistory(tr("SplPath_")));
                rowData.tool = tr("Spline Interactive");
                break;
            }

            case Tool::CubicBezierPath:
            {
                const QSharedPointer<VCubicBezierPath> splPath = data->GeometricObject<VCubicBezierPath>(toolId);
                SCASSERT(!splPath.isNull())
                rowData.icon = ":/toolicon/32x32/cubic_bezier_path.png";
                rowData.name = tr("%1").arg(splPath->NameForHistory(tr("SplPath_")));
                rowData.tool = tr("Spline Fixed");
                break;
            }

            case Tool::PointOfContact:
                rowData.icon = ":/toolicon/32x32/point_intersect_arc_line.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Arc with center %1 & Line %2_%3")
                                  .arg(getPointName(attrUInt(domElement, AttrCenter)))
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::Height:
                rowData.icon = ":/toolicon/32x32/height.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Line %1_%2 & Perpendicular %3")
                                  .arg(getPointName(attrUInt(domElement, AttrP1Line)))
                                  .arg(getPointName(attrUInt(domElement, AttrP2Line)))
                                  .arg(getPointName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::Triangle:
                rowData.icon = ":/toolicon/32x32/triangle.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Axis %1_%2 & Triangle points %3 and %4")
                                  .arg(getPointName(attrUInt(domElement, AttrAxisP1)))
                                  .arg(getPointName(attrUInt(domElement, AttrAxisP2)))
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::PointOfIntersection:
                rowData.icon = ":/toolicon/32x32/point_intersectxy_icon.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect XY of points %1 and %2")
                                  .arg(getPointName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getPointName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::CutArc:
            {
                const QSharedPointer<VArc> arc = data->GeometricObject<VArc>(attrUInt(domElement, AttrArc));
                SCASSERT(!arc.isNull())
                rowData.icon = ":/toolicon/32x32/arc_cut.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Arc");
                break;
            }

            case Tool::CutSpline:
            {
                const quint32 splineId = attrUInt(domElement, VToolCutSpline::AttrSpline);
                const QSharedPointer<VAbstractCubicBezier> spl = data->GeometricObject<VAbstractCubicBezier>(splineId);
                SCASSERT(!spl.isNull())
                rowData.icon = ":/toolicon/32x32/spline_cut_point.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Curve");
                break;
            }

            case Tool::CutSplinePath:
            {
                const quint32 splinePathId = attrUInt(domElement, VToolCutSplinePath::AttrSplinePath);
                const QSharedPointer<VAbstractCubicBezierPath> splPath =
                data->GeometricObject<VAbstractCubicBezierPath>(splinePathId);
                SCASSERT(!splPath.isNull())
                rowData.icon = ":/toolicon/32x32/splinePath_cut_point.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point On Spline");
                break;
            }

            case Tool::LineIntersectAxis:
                rowData.icon = ":/toolicon/32x32/line_intersect_axis.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("%Point Intersect Line & %1_%2 and Axis through point %3")
                                 .arg(getPointName(attrUInt(domElement, AttrP1Line)))
                                 .arg(getPointName(attrUInt(domElement, AttrP2Line)))
                                 .arg(getPointName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::CurveIntersectAxis:
                rowData.icon = ":/toolicon/32x32/arc_intersect_axis.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Curve & Axis through point %1")
                                  .arg(getPointName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::PointOfIntersectionArcs:
                rowData.icon = ":/toolicon/32x32/point_of_intersection_arcs.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Arcs");
                break;

            case Tool::PointOfIntersectionCircles:
                rowData.icon = ":/toolicon/32x32/point_of_intersection_circles.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("%1 - Point Intersect Circles");
                break;

            case Tool::PointOfIntersectionCurves:
                rowData.icon = ":/toolicon/32x32/intersection_curves.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Curves");
                break;

            case Tool::PointFromCircleAndTangent:
                rowData.icon = ":/toolicon/32x32/point_from_circle_and_tangent.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Circle & Tangent");
                break;

            case Tool::PointFromArcAndTangent:
                rowData.icon = ":/toolicon/32x32/point_from_arc_and_tangent.png";
                rowData.name = tr("%1").arg(getPointName(toolId));
                rowData.tool = tr("Point Intersect Arc & Tangent");
                break;

            case Tool::TrueDarts:
                rowData.icon = ":/toolicon/32x32/true_darts.png";
                rowData.name = tr("");
                rowData.tool = tr("True Dart %1_%2_%3")
                                 .arg(getPointName(attrUInt(domElement, AttrDartP1)))
                                 .arg(getPointName(attrUInt(domElement, AttrDartP2)))
                                 .arg(getPointName(attrUInt(domElement, AttrDartP2)));
                break;

            case Tool::EllipticalArc:
            {
                const QSharedPointer<VEllipticalArc> elArc = data->GeometricObject<VEllipticalArc>(toolId);
                SCASSERT(!elArc.isNull())
                rowData.icon = ":/toolicon/32x32/el_arc.png";
                rowData.name = tr("%1").arg(elArc->NameForHistory(tr("ElArc_")));
                rowData.tool = tr("Arc Elliptical with length %1").arg(elArc->GetLength());
                break;

            }
            case Tool::Rotation:
                rowData.icon = ":/toolicon/32x32/rotation.png";
                rowData.name = tr("");
                rowData.tool = tr("Rotation around point %1. Suffix %2")
                                  .arg(getPointName(attrUInt(domElement, AttrCenter)),
                                       m_doc->GetParametrString(domElement, AttrSuffix, QString()));
                break;

            case Tool::MirrorByLine:
                rowData.icon = ":/toolicon/32x32/mirror_by_line.png";
                rowData.name = tr("");
                rowData.tool = tr("Mirror by Line %1_%2. Suffix %3")
                                  .arg(getPointName(attrUInt(domElement, AttrP1Line)))
                                  .arg(getPointName(attrUInt(domElement, AttrP2Line)),
                                       m_doc->GetParametrString(domElement, AttrSuffix, QString()));
                break;

            case Tool::MirrorByAxis:
                rowData.icon = ":/toolicon/32x32/mirror_by_axis.png";
                rowData.name = tr("");
                rowData.tool = tr("Mirror by Axis through %1 point. Suffix %2")
                                  .arg(getPointName(attrUInt(domElement, AttrCenter)),
                                       m_doc->GetParametrString(domElement, AttrSuffix, QString()));
                break;

            case Tool::Move:
                rowData.icon = ":/toolicon/32x32/move.png";
                rowData.name = tr("");
                rowData.tool = tr("Move - rotate around point %1. Suffix %2")
                                  .arg(getPointName(attrUInt(domElement, AttrCenter)),
                                       m_doc->GetParametrString(domElement, AttrSuffix, QString()));
                break;

            //Because "history" not only shows history of pattern, but helps restore current data for each pattern's
            //piece, we need add record about details and nodes, but don't show them.
            case Tool::Piece:
            case Tool::Union:
            case Tool::NodeArc:
            case Tool::NodeElArc:
            case Tool::NodePoint:
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            case Tool::Group:
            case Tool::InternalPath:
            case Tool::AnchorPoint:
            case Tool::InsertNodes:
                break;
        }
        return rowData;
    }
    catch (const VExceptionBadId &error)
    {
        qDebug() << error.ErrorMessage() << Q_FUNC_INFO;
        return rowData;
    }
    qWarning() << "Can't create history record for the tool.";
    return rowData;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initializeTable set initial option of table
 */
void HistoryDialog::initializeTable()
{
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem("Id"));
    ui->tableWidget->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Name")));
    ui->tableWidget->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Description")));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showTool show selected point
 */
void HistoryDialog::showTool()
{
    const QVector<VToolRecord> *history = m_doc->getHistory();
    if (!history->empty())
    {
        QTableWidgetItem *item = ui->tableWidget->item(0, 1);
        item->setSelected(true);
        m_cursorToolRecordRow = 0;
        item = ui->tableWidget->item(0, 0);
        quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief HistoryDialog::PointName return point name by id.
 *
 * Refacoring what hide ugly string getting point name by id.
 * @param pointId point if in data.
 * @return point name.
 */
QString HistoryDialog::getPointName(quint32 pointId)
{
    return data->GeometricObject<VPointF>(pointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
quint32 HistoryDialog::attrUInt(const QDomElement &domElement, const QString &name)
{
    return m_doc->GetParametrUInt(domElement, name, "0");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle when windows is closing
 * @param event event
 */
void HistoryDialog::closeEvent(QCloseEvent *event)
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow,1);
    quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
    emit showHistoryTool(id, false);
    DialogTool::closeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate Creator form (single inheritance approach)
        ui->retranslateUi(this);
        retranslateUi();
    }

    // remember to call base class implementation
    QDialog::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
bool HistoryDialog::eventFilter(QObject *object, QEvent *event)
{
    if (QLineEdit *textEdit = qobject_cast<QLineEdit *>(object))
    {
        if (event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                if (qApp->Settings()->getOsSeparator())
                {
                    textEdit->insert(QLocale().decimalPoint());
                }
                else
                {
                    textEdit->insert(QLocale::c().decimalPoint());
                }
                return true;
            }
        }
    }
    else
    {
        // pass the event on to the parent class
        return DialogTool::eventFilter(object, event);
    }
    return false;// pass the event to the widget
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::retranslateUi()
{
    qint32 currentRow = m_cursorRow;
    updateHistory();

    QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, 0);
    SCASSERT(item != nullptr)
    item->setIcon(QIcon(""));

    m_cursorRow = currentRow;
    cellClicked(m_cursorRow, 0);
}

//---------------------------------------------------------------------------------------------------------------------
int HistoryDialog::cursorRow() const
{
    const quint32 cursor = m_doc->getCursor();
    if (cursor == 0)
    {
        return ui->tableWidget->rowCount()-1;
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        const quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (cursor == id)
        {
            return i;
        }
    }
    return ui->tableWidget->rowCount()-1;
}

void HistoryDialog::findText(const QString &text)
{
    updateHistory();
    if (text.isEmpty())
    {
        return;
    }

    QList<QTableWidgetItem *> items = ui->tableWidget->findItems(text, Qt::MatchContains);

    for (int i = 0; i < items.count(); ++i)
    {
        items.at(i)->setBackground(QColor("#b2cbe5"));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief copyToClipboard copy dialog selection to clipboard as comma separated values.
 */
void HistoryDialog::copyToClipboard()
{
    QItemSelectionModel *model = ui->tableWidget->selectionModel();
    QModelIndexList selectedIndexes = model->selectedIndexes();

    QString clipboardString;

    for (int i = 0; i < selectedIndexes.count(); ++i)
    {
        QModelIndex current = selectedIndexes[i];
        QString displayText = current.data(Qt::DisplayRole).toString();

        // Check if another column exists beyond this one.
        if (i + 1 < selectedIndexes.count())
        {
            QModelIndex next = selectedIndexes[i+1];

            // If the column is on different row, the clipboard should take note.
            if (next.row() != current.row())
            {
                displayText.append("\n");
            }
            else
            {
                // Otherwise append a comma separator.
                displayText.append(" , ");
            }
        }
        clipboardString.append(displayText);
    }

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(clipboardString);
}

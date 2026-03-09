//-----------------------------------------------------------------------------
//  @file   historyDialog.cpp
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2015 - 2023 Seamly, LLC
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
//  @file   dialoghistory.cpp
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   November 15, 2013
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2013-2015 Valentina project
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
//
//-----------------------------------------------------------------------------

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
#include "../vmisc/vtablesearch.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/tools/drawTools/operation/vabstractoperation.h"
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
#include <QScrollBar>

// We need this enum in case we will add or delete a column. It also makes code more readable.
enum {CursorColumn = 0, IdColumn, NameColumn, DescColumn, LengthColumn, AngleColumn};

//---------------------------------------------------------------------------------------------------------------------
/// @brief HistoryDialog create dialog
/// @param data container with data
/// @param doc dom document container
/// @param parent parent widget
//---------------------------------------------------------------------------------------------------------------------
HistoryDialog::HistoryDialog(VContainer *data, VPattern *doc, QWidget *parent)
    : DialogTool(data, 0, parent)
    , ui(new Ui::HistoryDialog)
    , m_doc(doc)
    , m_cursorRow(0)
    , m_cursorToolRecordRow(0)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint & ~Qt::WindowContextHelpButtonHint);

    // Set the position that the dialog opens based on user preference.
    setDialogPosition();

    // Limit dialog height to 80% of screen size.
    setMaximumHeight(qRound(QGuiApplication::primaryScreen()->availableGeometry().height() * .8));

    /// Install event filter to handle key presses and mouse events for the search line edit field.
    ui->find_LineEdit->installEventFilter(this);

    qApp->Settings()->getOsSeparator() ? setLocale(QLocale()) : setLocale(QLocale::c());

    fillTable();
    initializeTable();

    QPushButton *close_Button = ui->buttonBox->button(QDialogButtonBox::Close);
    close_Button->setDefault(false);
    close_Button->setAutoDefault(false);

    m_search = QSharedPointer<VTableSearch>(new VTableSearch(ui->tableWidget));

    connect(ui->clipboard_ToolButton,    &QToolButton::clicked,       this,  &HistoryDialog::copyToClipboard);
    connect(ui->find_LineEdit,           &QLineEdit::textEdited,      this,  &HistoryDialog::findText);
    connect(ui->prev_PushButton,         &QToolButton::clicked,       this,  &HistoryDialog::searchPrev);
    connect(ui->next_PushButton,         &QToolButton::clicked,       this,  &HistoryDialog::searchNext);
    connect(ui->regex_ToolButton,        &QToolButton::toggled,       this,  &HistoryDialog::regexToggled);
    connect(ui->case_ToolButton,         &QToolButton::toggled,       this,  &HistoryDialog::caseToggled);
    connect(ui->word_ToolButton,         &QToolButton::toggled,       this,  &HistoryDialog::wordToggled);
    connect(ui->moveToTop_ToolButton,    &QToolButton::clicked,       this,  &HistoryDialog::moveToTop);
    connect(ui->moveToCursor_ToolButton, &QToolButton::clicked,       this,  &HistoryDialog::moveToCursor);
    connect(ui->moveToBottom_ToolButton, &QToolButton::clicked,       this,  &HistoryDialog::moveToBottom);
    connect(ui->tableWidget,             &QTableWidget::cellClicked,  this,  &HistoryDialog::cellClicked);
    connect(close_Button,                &QPushButton::clicked,       this,  &HistoryDialog::DialogAccepted);
    connect(m_doc,                       &VPattern::ChangedCursor,    this,  &HistoryDialog::cursorChanged);
    connect(m_doc,                       &VPattern::patternChanged,   this,  &HistoryDialog::updateHistory);

    connect(this, &HistoryDialog::showHistoryTool, m_doc, [doc](quint32 id, bool enable)
    {
        emit doc->ShowTool(id, enable);
    });
    connect(m_search.data(), &VTableSearch::hasResult,  this,  [this] (bool state)
    {
        ui->prev_PushButton->setEnabled(state);
    });
    connect(m_search.data(), &VTableSearch::hasResult, this, [this] (bool state)
    {
        ui->next_PushButton->setEnabled(state);
    });

    showTool();
}

//---------------------------------------------------------------------------------------------------------------------
HistoryDialog::~HistoryDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief DialogAccepted save data and emit signal about closed dialog.
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::DialogAccepted()
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, IdColumn);
    quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
    emit showHistoryTool(id, false);

    // Reset cursor to bottom of table before closing
    m_doc->setCursorId(NULL_ID);

    emit DialogClosed(QDialog::Accepted);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief cellClicked changed history record
/// @param row number row in table
/// @param column number column in table
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::cellClicked(int row, int column)
{
    if (column == CursorColumn)
    {
        // clear current cursor row icon
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, CursorColumn);
        item->setIcon(QIcon());

        // set new cursor row icon
        m_cursorRow = row;
        item = ui->tableWidget->item(m_cursorRow, CursorColumn);
        item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));

        // set new cursor position in pattern.
        // if cursor is at last row set id to 0, else use tool id in row.
        item = ui->tableWidget->item(m_cursorRow, IdColumn);
        const quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        m_doc->blockSignals(true);
        row == ui->tableWidget->rowCount()-1 ? m_doc->setCursorId(NULL_ID) : m_doc->setCursorId(id);
        m_doc->blockSignals(false);
    }
    else
    {
        // Un Highlight old selected tool in draft block
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, IdColumn);
        quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, false);

        // Highlight new selected tool in draft block
        m_cursorToolRecordRow = row;
        item = ui->tableWidget->item(m_cursorToolRecordRow, IdColumn);
        id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief cursorChanged changed cursor of input. Cursor show after which record we will insert new object
/// @param id id of object
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::cursorChanged(quint32 id)
{
    for (qint32 row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, IdColumn);
        quint32 rowId = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (rowId == id)
        {
            QTableWidgetItem *oldCursorItem = ui->tableWidget->item(m_cursorRow, CursorColumn);
            oldCursorItem->setIcon(QIcon());
            m_cursorRow = row;
            item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));

            QScrollBar *verScrollBar = ui->tableWidget->verticalScrollBar();
            verScrollBar->setSliderPosition(verScrollBar->maximum());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief updateHistory update history table
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::updateHistory(bool saved)
{
    if (saved)
    {
        fillTable();
        initializeTable();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief fillTable fill table
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::fillTable()
{
    ui->draftblockName_Label->setText(m_doc->getActiveDraftBlockName());
    ui->tableWidget->clear();

    // Set the description column to stretch
    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(DescColumn, QHeaderView::Stretch);

    QVector<VToolRecord> history = m_doc->getBlockHistory();
    qint32 currentRow = -1;
    qint32 count = 0;
    ui->tableWidget->setRowCount(history.size());//Set Row count to number of Tool history records

    qint32 size = history.size();
    for (qint32 i = 0; i < size; ++i)
    {
        const VToolRecord tool = history.at(i);
        const RowData rowData = record(tool);
        if (!rowData.tool.isEmpty())
        {
            currentRow++;

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString(""));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                ui->tableWidget->setItem(currentRow, CursorColumn, item); //1st column is Cursor position
            }

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString().setNum(rowData.id));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setData(Qt::UserRole, rowData.id);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setItem(currentRow, IdColumn, item); //2nd column is Tool id
            }

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString(rowData.name));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setColumnWidth(NameColumn, 150);
                ui->tableWidget->setItem(currentRow, NameColumn, item); //3rd column is Tool name or Obj names
                ui->tableWidget->resizeRowToContents(currentRow);
            }

                QTableWidgetItem *item = new QTableWidgetItem(rowData.tool);
                item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setColumnWidth(DescColumn, 400);
                item->setIcon(QIcon(rowData.icon));
                ui->tableWidget->setItem(currentRow, DescColumn, item); //4th column is Tool description

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString(rowData.length));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setColumnWidth(LengthColumn, 120);
                ui->tableWidget->setItem(currentRow, LengthColumn, item); //5th column is Tool length formula
                ui->tableWidget->resizeRowToContents(currentRow);
            }

            {
                QTableWidgetItem *item = new QTableWidgetItem(QString(rowData.angle));
                item->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setColumnWidth(AngleColumn, 120);
                ui->tableWidget->setItem(currentRow, AngleColumn, item); //6th column is Tool angle formula
                ui->tableWidget->resizeRowToContents(currentRow);
            }
            ++count;
        }
    }

    ui->tableWidget->setRowCount(count);//Current history records count

    if (count>0)
    {
        m_cursorRow = cursorRow();
        QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, CursorColumn);
        SCASSERT(item != nullptr)
        item->setIcon(QIcon("://icon/24x24/left_to_right_arrow.png"));//place arrow in 1st column of most recent history record
    }

}


QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
//---------------------------------------------------------------------------------------------------------------------
/// @brief Record return description for record
/// @param tool record data
/// @return RowData
//---------------------------------------------------------------------------------------------------------------------
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
        rowData.icon = ":/icons/win.icon.theme/16x16/status/dialog-warning.png";
        rowData.name = QString();
        rowData.tool = tr("Can't create record.");
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
                rowData.name = getName(toolId);
                rowData.tool = tr("Base Point");
                break;

            case Tool::EndLine:
                rowData.icon   = ":/toolicon/32x32/segment.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrLength, QString());
                rowData.angle  = m_doc->GetParametrString(domElement, AttrAngle, QString());
                rowData.tool   = tr("Point Length and Angle from point %1")
                                    .arg(getName(attrUInt(domElement, AttrBasePoint)));
                                break;

            case Tool::Line:
                rowData.icon = ":/toolicon/32x32/line.png";
                rowData.name = tr("Line_%1_%2")
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                rowData.tool = tr("Line from %1 to %2")
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::AlongLine:
                rowData.icon = ":/toolicon/32x32/along_line.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point On Line %1_%2")
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::ShoulderPoint:
                rowData.icon = ":/toolicon/32x32/shoulder.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Length to Line");
                break;

            case Tool::Normal:
                rowData.icon   = ":/toolicon/32x32/normal.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrLength, QString());
                rowData.angle  = m_doc->GetParametrString(domElement, AttrAngle, QString());
                rowData.tool   = tr("Point On Perpendicular %1_%2")
                                .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::Bisector:
                rowData.icon = ":/toolicon/32x32/bisector.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point On Bisector %1_%2_%3")
                                 .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                 .arg(getName(attrUInt(domElement, AttrSecondPoint)))
                                 .arg(getName(attrUInt(domElement, AttrThirdPoint)));
                break;

            case Tool::LineIntersect:
                rowData.icon = ":/toolicon/32x32/intersect.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Lines %1_%2 and %3_%4")
                                 .arg(getName(attrUInt(domElement, AttrP1Line1)))
                                 .arg(getName(attrUInt(domElement, AttrP2Line1)))
                                 .arg(getName(attrUInt(domElement, AttrP1Line2)))
                                 .arg(getName(attrUInt(domElement, AttrP2Line2)));
                break;

            case Tool::Spline:
            {
                rowData.icon   = ":/toolicon/32x32/spline.png";
                rowData.name   = getName(toolId);
                rowData.length = QString("%1\n%2")
                                   .arg(m_doc->GetParametrString(domElement, AttrLength1, QString()))
                                   .arg(m_doc->GetParametrString(domElement, AttrLength2, QString()));
                rowData.angle  = QString("%1\n%2")
                                   .arg(m_doc->GetParametrString(domElement, AttrAngle1, QString()))
                                   .arg(m_doc->GetParametrString(domElement, AttrAngle2, QString()));
                rowData.tool   = tr("Curve Interactive");
                break;
            }

            case Tool::CubicBezier:
            {
                rowData.icon = ":/toolicon/32x32/cubic_bezier.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Curve Fixed");
                break;
            }

            case Tool::Arc:
            {
                rowData.icon   = ":/toolicon/32x32/arc.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrRadius, QString());
                rowData.angle  = QString("%1\n %2")
                                 .arg(m_doc->GetParametrString(domElement, AttrAngle1, QString()))
                                 .arg(m_doc->GetParametrString(domElement, AttrAngle2, QString()));
                rowData.tool   = tr("Arc Radius & Angles");
                break;
            }

            case Tool::ArcWithLength:
            {
                rowData.icon   = ":/toolicon/32x32/arc_with_length.png";
                rowData.name   = getName(toolId);
                rowData.length = QString("%1\n %2").arg(m_doc->GetParametrString(domElement, AttrRadius, ""))
                                                   .arg(m_doc->GetParametrString(domElement, AttrLength, ""));
                rowData.angle  = m_doc->GetParametrString(domElement, AttrAngle1, QString());
                rowData.tool   = tr("Arc with Radius, Length, and Angle");
                break;
            }

            case Tool::SplinePath:
            {
                rowData.icon = ":/toolicon/32x32/splinePath.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Spline Interactive");
                break;
            }

            case Tool::CubicBezierPath:
            {
                rowData.icon = ":/toolicon/32x32/cubic_bezier_path.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Spline Fixed");
                break;
            }

            case Tool::PointOfContact:
                rowData.icon = ":/toolicon/32x32/point_intersect_arc_line.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Arc with center %1 & Line %2_%3")
                                  .arg(getName(attrUInt(domElement, AttrCenter)))
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::Height:
                rowData.icon = ":/toolicon/32x32/height.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Line %1_%2 & Perpendicular %3")
                                  .arg(getName(attrUInt(domElement, AttrP1Line)))
                                  .arg(getName(attrUInt(domElement, AttrP2Line)))
                                  .arg(getName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::Triangle:
                rowData.icon = ":/toolicon/32x32/triangle.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Axis %1_%2 & Triangle points %3 and %4")
                                  .arg(getName(attrUInt(domElement, AttrAxisP1)))
                                  .arg(getName(attrUInt(domElement, AttrAxisP2)))
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::PointOfIntersection:
                rowData.icon = ":/toolicon/32x32/point_intersectxy_icon.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect XY of points %1 and %2")
                                  .arg(getName(attrUInt(domElement, AttrFirstPoint)))
                                  .arg(getName(attrUInt(domElement, AttrSecondPoint)));
                break;

            case Tool::CutArc:
            {
                rowData.icon   = ":/toolicon/32x32/arc_cut.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrLength, QString());
                rowData.tool   = tr("Point On Arc");
                break;
            }

            case Tool::CutSpline:
            {
                rowData.icon   = ":/toolicon/32x32/spline_cut_point.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrLength, QString());
                rowData.tool   = tr("Point On Curve");
                break;
            }

            case Tool::CutSplinePath:
            {
                rowData.icon   = ":/toolicon/32x32/splinePath_cut_point.png";
                rowData.name   = getName(toolId);
                rowData.length = m_doc->GetParametrString(domElement, AttrLength, QString());
                rowData.tool   = tr("Point On Spline");
                break;
            }

            case Tool::LineIntersectAxis:
                rowData.icon = ":/toolicon/32x32/line_intersect_axis.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("%Point Intersect Line & %1_%2 and Axis through point %3")
                                 .arg(getName(attrUInt(domElement, AttrP1Line)))
                                 .arg(getName(attrUInt(domElement, AttrP2Line)))
                                 .arg(getName(attrUInt(domElement, AttrBasePoint)));
                break;

            case Tool::CurveIntersectAxis:
            rowData.icon   = ":/toolicon/32x32/arc_intersect_axis.png";
            rowData.name   = getName(toolId);
            rowData.angle  = m_doc->GetParametrString(domElement, AttrAngle, QString());
            rowData.tool   = tr("Point Intersect Curve & Axis through point %1")
                                .arg(getName(attrUInt(domElement, AttrBasePoint)));
            break;

            case Tool::PointOfIntersectionArcs:
                rowData.icon = ":/toolicon/32x32/point_of_intersection_arcs.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Arcs");
                break;

            case Tool::PointOfIntersectionCircles:
                rowData.icon = ":/toolicon/32x32/point_of_intersection_circles.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("%1 - Point Intersect Circles");
                break;

            case Tool::PointOfIntersectionCurves:
                rowData.icon = ":/toolicon/32x32/intersection_curves.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Curves");
                break;

            case Tool::PointFromCircleAndTangent:
                rowData.icon = ":/toolicon/32x32/point_from_circle_and_tangent.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Circle & Tangent");
                break;

            case Tool::PointFromArcAndTangent:
                rowData.icon = ":/toolicon/32x32/point_from_arc_and_tangent.png";
                rowData.name = getName(toolId);
                rowData.tool = tr("Point Intersect Arc & Tangent");
                break;

            case Tool::TrueDarts:
                rowData.icon = ":/toolicon/32x32/true_darts.png";
                rowData.name = getName(attrUInt(domElement, AttrPoint1)) + QStringLiteral("\n") +
                            getName(attrUInt(domElement, AttrPoint2));
                rowData.tool = tr("True Dart %1_%2_%3")
                                .arg(getName(attrUInt(domElement, AttrDartP1)))
                                .arg(getName(attrUInt(domElement, AttrDartP2)))
                                .arg(getName(attrUInt(domElement, AttrDartP3)));
                break;

            case Tool::EllipticalArc:
            {
                const QSharedPointer<VEllipticalArc> elArc = data->GeometricObject<VEllipticalArc>(toolId);
                SCASSERT(!elArc.isNull())
                rowData.icon = ":/toolicon/32x32/el_arc.png";
                rowData.name = elArc->NameForHistory(tr("ElArc_"));
                rowData.tool = tr("Arc Elliptical with length %1").arg(elArc->GetLength());
                break;

            }
            case Tool::Rotation:
            {
                const QString suffix = m_doc->GetParametrString(domElement, AttrSuffix, QString());
                rowData.icon = ":/toolicon/32x32/rotation.png";
                rowData.name = getDestinationNames(domElement, suffix);
                rowData.tool = tr("Rotation around point %1. Suffix %2")
                                .arg(getName(attrUInt(domElement, AttrCenter)), suffix);
                break;
            }

            case Tool::MirrorByLine:
            {
                const QString suffix = m_doc->GetParametrString(domElement, AttrSuffix, QString());
                rowData.icon = ":/toolicon/32x32/mirror_by_line.png";
                rowData.name = getDestinationNames(domElement, suffix);
                rowData.tool = tr("Mirror by Line %1_%2. Suffix %3")
                                  .arg(getName(attrUInt(domElement, AttrP1Line)))
                                  .arg(getName(attrUInt(domElement, AttrP2Line)), suffix);
                break;
            }

            case Tool::MirrorByAxis:
            {
                const QString suffix = m_doc->GetParametrString(domElement, AttrSuffix, QString());
                rowData.icon = ":/toolicon/32x32/mirror_by_axis.png";
                rowData.name = getDestinationNames(domElement, suffix);
                rowData.tool = tr("Mirror by Axis through %1 point. Suffix %2")
                                  .arg(getName(attrUInt(domElement, AttrCenter)), suffix);
                break;
            }

            case Tool::Move:
            {
                const QString suffix = m_doc->GetParametrString(domElement, AttrSuffix, QString());
                rowData.icon = ":/toolicon/32x32/move.png";
                rowData.name = getDestinationNames(domElement, suffix);

                quint32 id = attrUInt(domElement, AttrCenter);
                QString pointName = tr("Center point");
                if (id != NULL)
                {
                    pointName = getName(id);
                }
                rowData.tool = tr("Move - rotate around point %1. Suffix %2")
                                .arg(pointName, suffix);
                break;
            }

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
/// @brief initializeTable set initial option of table
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::initializeTable()
{
    ui->tableWidget->setSortingEnabled(false);
    ui->tableWidget->verticalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->horizontalHeader()->setDefaultAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableWidget->setHorizontalHeaderItem(CursorColumn, new QTableWidgetItem(""));
    ui->tableWidget->setHorizontalHeaderItem(IdColumn, new QTableWidgetItem("Id"));
    ui->tableWidget->setHorizontalHeaderItem(NameColumn, new QTableWidgetItem(tr("Name")));
    ui->tableWidget->setHorizontalHeaderItem(DescColumn, new QTableWidgetItem(tr("Description")));
    ui->tableWidget->setHorizontalHeaderItem(LengthColumn, new QTableWidgetItem(tr("Radius / Length")));
    ui->tableWidget->setHorizontalHeaderItem(AngleColumn, new QTableWidgetItem(tr("Angle")));
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief showTool show selected point
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::showTool()
{
    const QVector<VToolRecord> *history = m_doc->getHistory();
    if (history->size()>0)
    {
        // Select 1st tool and highlight it in draftblock
        QTableWidgetItem *item = ui->tableWidget->item(0, CursorColumn);
        m_cursorToolRecordRow = 0;
        item = ui->tableWidget->item(0, IdColumn);
        quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        emit showHistoryTool(id, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief HistoryDialog::getName get object name by id.
///
/// This method gets the name of the VObject with the given tool id.
///
/// @param toolId id of object.
/// @return name of object.
//---------------------------------------------------------------------------------------------------------------------
QString HistoryDialog::getName(const quint32 &toolId)
{
    const QSharedPointer<VGObject> object = data->GetGObject(toolId);
    return object->name();
}

QString HistoryDialog::getDestinationNames(const QDomElement &domElement, const QString &suffix)
{
    QString name = QString("");
    QVector<SourceItem> source;
    QVector<DestinationItem> destination;
    VAbstractOperation::ExtractData(domElement, source, destination);

    for(int i = 0; i < source.size(); i++)
    {
        if ((i > 0) && (i != source.size()))

        {
            name = name + QStringLiteral("\n") + getName(source[i].id) + suffix;
        }
        else
        {
            name = name + getName(source[i].id) + suffix;
        }
    }
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
quint32 HistoryDialog::attrUInt(const QDomElement &domElement, const QString &name)
{
    return m_doc->GetParametrUInt(domElement, name, "0");
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief closeEvent handle when windows is closing
/// @param event event
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::closeEvent(QCloseEvent *event)
{
    // Highlight tool in draft block
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorToolRecordRow, IdColumn);
    quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
    emit showHistoryTool(id, false);

    // Reset cursor to bottom of table before closing
    m_doc->setCursorId(NULL_ID);

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

    if (event->type() == QEvent::ApplicationPaletteChange)
    {
        updateHistory(true);
        m_search->find(ui->find_LineEdit->text());
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
            if ((keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return))
            {
                // Ignore Enter key
                return true;
            }
            else if ((keyEvent->key() == Qt::Key_Period) && (keyEvent->modifiers() & Qt::KeypadModifier))
            {
                QString separator = qApp->Settings()->getOsSeparator()
                                  ? QString(localeDecimalPoint(QLocale()))
                                  : QString(localeDecimalPoint(QLocale::c()));

                textEdit->insert(separator);
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
/// @brief showEvent Reimplement show event.
///
/// This method reimplemnts the showevent and resizes the dialog based on the last saved size.
///
/// @param event event
//---------------------------------------------------------------------------------------------------------------------

void HistoryDialog::showEvent(QShowEvent *event)
{
    // Skip DialogTool implementation
    QDialog::showEvent(event);
    if ( event->spontaneous() )
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }

    const QSize size = qApp->Settings()->getHistoryDialogSize();
    if (! size.isEmpty())
    {
        resize(size);
    }

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief resizeEvent Reimplement rsize event.
/// @param event event
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::resizeEvent(QResizeEvent *event)
{
    // remember the size for the next time this dialog is opened, but only
    // if widget was already initialized, which rules out the resize when
    // dialog is created.
    if (isInitialized)
    {
        qApp->Settings()->setHistoryDialogSize(size());
    }
    DialogTool::resizeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::retranslateUi()
{
    qint32 currentRow = m_cursorRow;
    updateHistory(true);

    QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, CursorColumn);
    SCASSERT(item != nullptr)
    item->setIcon(QIcon(""));

    m_cursorRow = currentRow;
    cellClicked(m_cursorRow, CursorColumn);
}

//---------------------------------------------------------------------------------------------------------------------
int HistoryDialog::cursorRow() const
{
    const quint32 cursorId = m_doc->getCursorId();
    if (cursorId == NULL_ID)
    {
        return ui->tableWidget->rowCount()-1;
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QTableWidgetItem *item = ui->tableWidget->item(row, IdColumn);
        const quint32 id = qvariant_cast<quint32>(item->data(Qt::UserRole));
        if (cursorId == id)
        {
            return row;
        }
    }
    return ui->tableWidget->rowCount() - 1;
}

void HistoryDialog::findText(const QString &text)
{
    updateHistory(true);
    m_search->find(text);
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief copyToClipboard copy dialog selection to clipboard as comma separated values.
//---------------------------------------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::searchPrev()
{
    m_search->findPrevious();
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::searchNext()
{
    m_search->findNext();
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::regexToggled(bool checked)
{
    if (checked)
    {
        ui->case_ToolButton->blockSignals(true);
        ui->case_ToolButton->setChecked(false);
        ui->case_ToolButton->blockSignals(false);
        m_search->setMatchCase(false);

        ui->word_ToolButton->blockSignals(true);
        ui->word_ToolButton->setChecked(false);
        ui->word_ToolButton->blockSignals(false);
        m_search->setMatchWord(false);
    }

    m_search->setMatchRegEx(checked);
    m_search->find(ui->find_LineEdit->text());
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::caseToggled(bool checked)
{
    if (checked)
    {
        ui->regex_ToolButton->blockSignals(true);
        ui->regex_ToolButton->setChecked(false);
        ui->regex_ToolButton->blockSignals(false);
        m_search->setMatchRegEx(false);
    }

    m_search->setMatchCase(checked);
    m_search->find(ui->find_LineEdit->text());
}

//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::wordToggled(bool checked)
{
    if (checked)
    {
        ui->regex_ToolButton->blockSignals(true);
        ui->regex_ToolButton->setChecked(false);
        ui->regex_ToolButton->blockSignals(false);
        m_search->setMatchRegEx(false);
    }

    m_search->setMatchWord(checked);
    m_search->find(ui->find_LineEdit->text());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief moveToTop  Scroll to top of table.
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::moveToTop()
{
    ui->tableWidget->verticalScrollBar()->setValue(ui->tableWidget->verticalScrollBar()->minimum());
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief moveToCursor Scroll to cursor osition in table.
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::moveToCursor()
{
    QTableWidgetItem *item = ui->tableWidget->item(m_cursorRow, CursorColumn);
    if (item)
    {
        ui->tableWidget->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief moveToBottom Scroll to bottom of table.
//---------------------------------------------------------------------------------------------------------------------
void HistoryDialog::moveToBottom()
{
    ui->tableWidget->verticalScrollBar()->setValue(ui->tableWidget->verticalScrollBar()->maximum());
}

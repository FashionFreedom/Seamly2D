//---------------------------------------------------------------------------------------------------------------------
//  @file   vdrawtool.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2015 - 2026 Seamly, LLC
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
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
//  @file   vdrawtool.h
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
//---------------------------------------------------------------------------------------------------------------------

#ifndef VDRAWTOOL_H
#define VDRAWTOOL_H


#include "../vinteractivetool.h"
#include "../ifc/exception/vexceptionbadid.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vdatatool.h"
#include "../vgeometry/vabstractarc.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vwidgets/vabstractmainwindow.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vtools/undocommands/addgroup.h"
#include "../vtools/undocommands/add_groupitem.h"
#include "../vtools/undocommands/remove_groupitem.h"
#include "../vtools/undocommands/move_groupitem.h"

#include <qcompilerdetection.h>
#include <QAction>
#include <QActionGroup>
#include <QByteArray>
#include <QClipboard>
#include <QColor>
#include <QDomElement>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QMetaObject>
#include <QObject>
#include <QPair>
#include <QString>
#include <QtGlobal>
#include <QVariant>

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/// @brief The VDrawTool abstract class for all draw tool.
//---------------------------------------------------------------------------------------------------------------------
class VDrawTool : public VInteractiveTool
{
    Q_OBJECT

public:
                     VDrawTool(VAbstractPattern *doc, VContainer *data, quint32 id, QObject *parent = nullptr);
    virtual         ~VDrawTool() Q_DECL_EQ_DEFAULT;

    QString          getLineType() const;
    virtual void     setLineType(const QString &value);

    QString          getLineWeight() const;
    virtual void     setLineWeight(const QString &value);

    virtual bool     isPointNameVisible(quint32 id) const;
    virtual bool     isUsed() const override;

signals:
    void             ChangedToolSelection(bool selected, quint32 object, quint32 tool);

public slots:
    virtual void     ShowTool(quint32 id, bool enable);
    virtual void     activeBlockChanged(const QString &new_name);
    void             blockNameChanged(const QString &old_name, const QString &new_name);
    virtual void     EnableToolMove(bool move);
    virtual void     Disable(bool disable, const QString &draft_block_name)=0;
    virtual void     piecesMode(bool mode);

protected slots:
    virtual void     showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID)=0;

protected:
    enum class       RemoveOption : bool {Disable = false, Enable = true};
    enum class       Referens : bool {Follow = true, Ignore = false};


    QString          m_active_block_name;   /// @brief m_active_block_name name of tool's pattern peace. */
    QString          m_lineType;           /// @brief typeLine line type. */
    QString          m_lineWeight;         /// @brief typeLine line weight. */

    void             AddToCalculation(const QDomElement &dom_element);
    void             addDependence(QList<quint32> &list, quint32 object_id) const;

    /// @brief SaveDialog save options into file after change in dialog. */
    virtual void     SaveDialog(QDomElement &dom_element)=0;
    virtual void     SaveDialogChange() final;
    virtual void     AddToFile() override;
    void             SaveOption(QSharedPointer<VGObject> &obj);
    virtual void     SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj);
    virtual QString  makeToolTip() const;

    bool             CorrectDisable(bool disable, const QString &draft_block_name) const;

    void             ReadAttributes();
    virtual void     ReadToolAttributes(const QDomElement &dom_element)=0;
    virtual void     updatePointNameVisibility(quint32 id, bool visible);

    template <typename Dialog>
    void             ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 objecct_id = NULL_ID,
                                 const RemoveOption &show_remove = RemoveOption::Enable,
                                 const Referens &ref = Referens::Follow);

    template <typename Item>
    void             ShowItem(Item *item, quint32 id, bool enable);

    template <typename T>
    QString          ObjectName(quint32 id) const;

    template <typename T>
    static void      InitDrawToolConnections(VMainGraphicsScene *scene, T *tool);

    QString          makeName(const QString &attribute1, const QString &attribute2,
                              const QString &prefix, const quint32 &tool_id);

private:
    Q_DISABLE_COPY(VDrawTool)

};

//---------------------------------------------------------------------------------------------------------------------
/// @brief ContextMenu show context menu for tool.
/// @param objecct_id id of point.
/// @param event context menu event.
/// @param show_remove true - tool enable option delete.
/// @param ref true - do not ignore referens value.
//---------------------------------------------------------------------------------------------------------------------
template <typename Dialog>
void VDrawTool::ContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 objecct_id,
                            const RemoveOption &show_remove, const Referens &ref)
{
    SCASSERT(event != nullptr)

    if (m_suppressContextMenu)
    {
        return;
    }

    GOType item_type =  GOType::Unknown;
    if(objecct_id != NULL_ID)
    {
        try
        {
            item_type = data.GetGObject(objecct_id)->getType();
        }
        catch (const VExceptionBadId &error)
        { // Possible case. Parent was deleted, but the node object is still here.
            qWarning() << qUtf8Printable(error.ErrorMessage());
        }
    }

    quint32 tool_id = this->getId();
    const QDomElement tool_element = doc->elementById(tool_id);
    if (tool_element.isNull())
    {
        return;
    }

    QMap<quint32, Tool> history = doc->getGroupObjHistory();
    Tool tool_type = history.value(tool_id);

    qCDebug(vTool, "Creating tool context menu.");
    QMenu menu;

    QAction *properties_action = menu.addAction(QIcon::fromTheme("preferences-other"), VDrawTool::tr("Properties"));

    // Show object name menu item
    QAction *show_point_name_action = menu.addAction(QIcon("://icon/16x16/open_eye.png"), VDrawTool::tr("Show Point Name"));
    show_point_name_action->setCheckable(true);

    if (item_type == GOType::Point)
    {
        show_point_name_action->setChecked(isPointNameVisible(objecct_id));
    }
    else
    {
       show_point_name_action->setVisible(false);
    }

    // Add Copy menu
    QMenu   *copy_memu   = nullptr;

    if (tool_type != Tool::BasePoint &&
        tool_type != Tool::Triangle &&
        tool_type != Tool::PointOfIntersectionArcs &&
        tool_type != Tool::PointOfIntersectionCircles &&
        tool_type != Tool::PointOfIntersectionCurves &&
        tool_type != Tool::PointFromCircleAndTangent &&
        tool_type != Tool::PointFromArcAndTangent &&
        tool_type != Tool::Rotation &&
        tool_type != Tool::Move &&
        tool_type != Tool::MirrorByLine &&
        tool_type != Tool::MirrorByAxis)
    {
        copy_memu = menu.addMenu(QIcon("://icon/32x32/clipboard_icon.png"), tr("Copy"));
    }
    switch (tool_type)
    {
        case Tool::Line:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/line.png"),
                                 makeName(AttrFirstPoint, AttrSecondPoint, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrFirstPoint, AttrSecondPoint, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::LineIntersect:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/intersect.png"),
                                 makeName(AttrP1Line2, AttrName,    tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrName,    AttrP2Line1, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrName,    AttrP2Line2, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrP1Line1, AttrName,    tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrP1Line2, AttrName,    tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrName,    AttrP2Line1, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrName,    AttrP2Line2, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrP1Line1, AttrName,    tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::EndLine:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/segment.png"),
                                 makeName(AttrBasePoint, AttrName, tr("Line_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrBasePoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::CurveIntersectAxis:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/arc_intersect_axis.png"),
                                 makeName(AttrBasePoint, AttrName, tr("Line_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrBasePoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::AlongLine:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/along_line.png"),
                                 makeName(AttrFirstPoint, AttrName, tr("Line_"), tool_id));
            copy_memu->addAction(makeName(AttrName, AttrSecondPoint, tr("Line_"), tool_id));
            copy_memu->addAction(makeName(AttrFirstPoint, AttrSecondPoint, tr("Line_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrFirstPoint, AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrName, AttrSecondPoint, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrFirstPoint, AttrSecondPoint, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::Height:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/height.png"),
                                 makeName(AttrBasePoint, AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrP1Line,    AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrP2Line,    AttrName, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrP1Line,    AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrName,      AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrP2Line,    AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::Bisector:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/bisector.png"),
                                 makeName(AttrSecondPoint, AttrName, tr("Line_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrSecondPoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::Normal:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/normal.png"),
                                 makeName(AttrFirstPoint, AttrName, tr("Line_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrFirstPoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::LineIntersectAxis:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/line_intersect_axis.png"),
                                 makeName(AttrBasePoint, AttrName,   tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrP1Line,    AttrName,   tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrName,      AttrP2Line, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrBasePoint, AttrName,   tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrP1Line,    AttrName,   tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrName,      AttrP2Line, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::IntersectArcLine:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/point_intersect_arc_line.png"),
                                 makeName(AttrCenter,      AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrFirstPoint,  AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrSecondPoint, AttrName, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter,      AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrFirstPoint,  AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrSecondPoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::ShoulderPoint:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/shoulder.png"),
                                 makeName(AttrP1Line, AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrP2Line, AttrName, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrP1Line, AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrP2Line, AttrName, tr("AngleLine_"), tool_id));

            break;
        }
        case Tool::PointOfIntersection:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/point_intersectxy_icon.png"),
                                 makeName(AttrFirstPoint,  AttrName, tr("Line_"),      tool_id));
            copy_memu->addAction(makeName(AttrSecondPoint, AttrName, tr("Line_"),      tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrFirstPoint,  AttrName, tr("AngleLine_"), tool_id));
            copy_memu->addAction(makeName(AttrSecondPoint, AttrName, tr("AngleLine_"), tool_id));

            break;
        }

        case Tool::Spline:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/spline.png"),
                                 makeName(AttrPoint1, AttrPoint4, tr("Spl_"),         tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("Angle1Spl_"),   tool_id));
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("Angle2Spl_"),   tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("C1LengthSpl_"), tool_id));
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("C2LengthSpl_"), tool_id));

            break;
        }
        case Tool::CubicBezier:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/cubic_bezier.png"),
                                 makeName(AttrPoint1, AttrPoint4, tr("Spl_"),         tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("Angle1Spl_"),   tool_id));
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("Angle2Spl_"),   tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("C1LengthSpl_"), tool_id));
            copy_memu->addAction(makeName(AttrPoint1, AttrPoint4, tr("C2LengthSpl_"), tool_id));

            break;
        }


        case Tool::CutSpline:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/spline_cut_point.png"),
                                 makeName(AttrName, "cutSpl", tr("Spl_"),         tool_id));
            copy_memu->addAction(makeName("cutSpl", AttrName, tr("Spl_"),         tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrName, "cutSpl", tr("Angle1Spl_"),   tool_id));
            copy_memu->addAction(makeName("cutSpl", AttrName, tr("Angle2Spl_"),   tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrName, "cutSpl", tr("C1LengthSpl_"), tool_id));
            copy_memu->addAction(makeName("cutSpl", AttrName, tr("C2LengthSpl_"), tool_id));

            break;
        }

        case Tool::SplinePath:
        {
            // Get a list of all <pathPoint> children under this spline node
            QDomNodeList path_points = tool_element.elementsByTagName(QStringLiteral("pathPoint"));

            // Ensure we have at least 2 points to safely construct a "First_Last" path string
            if (path_points.count() >= 2)
            {
                // Extract the first point node and its pSpline ID
                QDomElement first_elem = path_points.at(0).toElement();
                quint32 first_point_id = first_elem.attribute(QStringLiteral("pSpline")).toUInt();

                // Extract the last point node and its pSpline ID
                QDomElement last_elem  = path_points.at(path_points.count() - 1).toElement();
                quint32 last_point_id  = last_elem.attribute(QStringLiteral("pSpline")).toUInt();

                // Safely look up the actual geometric points to fetch their UI names (e.g., "A", "A4")
                const QSharedPointer<VGObject> first_point_obj = data.GetGObject(first_point_id);
                const QSharedPointer<VGObject> last_point_obj  = data.GetGObject(last_point_id);

                if (!first_point_obj.isNull() && !last_point_obj.isNull())
                {
                    QString point1_name = first_point_obj->name();
                    QString point2_name = last_point_obj->name();

                    // Assemble the copy text payload: SplPath_A_A4
                    QString text = QString("%1%2_%3").arg(tr("SplPath_"), point1_name, point2_name);

                    // Read the duplicate counter directly as a string
                    QString duplicate_val = tool_element.attribute(QStringLiteral("duplicate"));

                    // If the counter attribute is present in the XML, append it directly
                    if (!duplicate_val.isEmpty())
                    {
                        text += QStringLiteral("_") + duplicate_val;
                    }

                    // Add the main spline name item and its 4 custom angle/length properties
                    copy_memu->addAction(QIcon(":/toolicon/32x32/splinePath.png"), text);
                    copy_memu->addAction(QStringLiteral("Angle1") + text);
                    copy_memu->addAction(QStringLiteral("Angle2") + text);
                    copy_memu->addAction(QStringLiteral("C1Length") + text);
                    copy_memu->addAction(QStringLiteral("C2Length") + text);

                    // Loop through and add the individual segment names along with their 4 properties
                    int segment_count = path_points.count() - 1;
                    if (segment_count > 0)
                    {
                        for (int i = 0; i < segment_count; ++i)
                        {
                            copy_memu->addSeparator();
                            QString segment_text = QString("%1_Seg_%2").arg(text).arg(i + 1);

                            copy_memu->addAction(segment_text);
                            copy_memu->addAction(QStringLiteral("Angle1") + segment_text);
                            copy_memu->addAction(QStringLiteral("Angle2") + segment_text);
                            copy_memu->addAction(QStringLiteral("C1Length") + segment_text);
                            copy_memu->addAction(QStringLiteral("C2Length") + segment_text);
                        }
                    }
                }
            }
            break;
        }

        case Tool::CubicBezierPath:
        {
            // Get a list of all <pathPoint> children under this spline node
            QDomNodeList path_points = tool_element.elementsByTagName(QStringLiteral("pathPoint"));

            // Ensure we have at least 2 points to safely construct a "First_Last" path string
            if (path_points.count() >= 2)
            {
                // Extract the first point node and its pSpline ID
                QDomElement first_elem = path_points.at(0).toElement();
                quint32 first_point_id = first_elem.attribute(QStringLiteral("pSpline")).toUInt();

                // Extract the last point node and its pSpline ID
                QDomElement last_elem  = path_points.at(path_points.count() - 1).toElement();
                quint32 last_point_id  = last_elem.attribute(QStringLiteral("pSpline")).toUInt();

                // Safely look up the actual geometric points to fetch their UI names (e.g., "A", "A4")
                const QSharedPointer<VGObject> first_point_obj = data.GetGObject(first_point_id);
                const QSharedPointer<VGObject> last_point_obj  = data.GetGObject(last_point_id);

                if (!first_point_obj.isNull() && !last_point_obj.isNull())
                {
                    QString point1_name = first_point_obj->name();
                    QString point2_name = last_point_obj->name();

                    // Assemble the copy text payload: SplPath_A_A4
                    QString text = QString("%1%2_%3").arg(tr("SplPath_"), point1_name, point2_name);

                    // Read the duplicate counter directly as a string
                    QString duplicate_val = tool_element.attribute(QStringLiteral("duplicate"));

                    // If the counter attribute is present in the XML, append it directly
                    if (!duplicate_val.isEmpty())
                    {
                        text += QStringLiteral("_") + duplicate_val;
                    }

                    // Add the main spline name item and its 4 custom angle/length properties
                    copy_memu->addAction(QIcon(":/toolicon/32x32/cubic_bezier_path.png"), text);

                    // Loop through and add the individual segment names along with their 4 properties
                    int segment_count = path_points.count() - 1;
                    for (int i = 0; i < segment_count; ++i)
                    {
                        QString segment_text = QString("%1_Seg_%2").arg(text).arg(i + 1);

                        copy_memu->addAction(segment_text);
                    }
                }
            }
            break;
        }


        case Tool::CutSplinePath:
        {
            QString cut_point_name = tool_element.attribute(AttrName);
            if (cut_point_name.isEmpty())
            {
                break;
            }

            quint32 spline_path_id = tool_element.attribute(QStringLiteral("splinePath")).toUInt();
            const QDomElement spline_element = doc->elementById(spline_path_id);
            if (spline_element.isNull())
            {
                break;
            }

            QDomNodeList path_points = spline_element.elementsByTagName(QStringLiteral("pathPoint"));
            if (path_points.count() >= 2)
            {
                QDomElement first_elem = path_points.at(0).toElement();
                quint32 first_point_id = first_elem.attribute(QStringLiteral("pSpline")).toUInt();

                QDomElement last_elem  = path_points.at(path_points.count() - 1).toElement();
                quint32 last_point_id  = last_elem.attribute(QStringLiteral("pSpline")).toUInt();

                const QSharedPointer<VGObject> first_point_obj = data.GetGObject(first_point_id);
                const QSharedPointer<VGObject> last_point_obj  = data.GetGObject(last_point_id);

                if (!first_point_obj.isNull() && !last_point_obj.isNull())
                {
                    QString point1_name = first_point_obj->name();
                    QString point2_name = last_point_obj->name();

                    QString text1 = QString("%1%2_%3").arg(tr("SplPath_"), point1_name, cut_point_name);
                    QString text2 = QString("%1%2_%3").arg(tr("SplPath_"), cut_point_name, point2_name);

                    QString duplicate_val = spline_element.attribute(QStringLiteral("duplicate"));
                    if (!duplicate_val.isEmpty())
                    {
                        text1 += QStringLiteral("_") + duplicate_val;
                        text2 += QStringLiteral("_") + duplicate_val;
                    }

                    copy_memu->addAction(QIcon(":/toolicon/32x32/splinePath_cut_point.png"), text1);
                    copy_memu->addAction(text2);
                }
            }
            break;
        }

        case Tool::Arc:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/arc.png"),
                                 makeName(AttrCenter, "arcId", tr("Arc_"),       tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("RadiusArc_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle1Arc_"), tool_id));
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle2Arc_"), tool_id));

            break;
        }
        case Tool::ArcWithLength:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/arc_with_length.png"),
                                 makeName(AttrCenter, "arcId", tr("Arc_"),       tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("RadiusArc_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle1Arc_"), tool_id));
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle2Arc_"), tool_id));

            break;
        }

        case Tool::EllipticalArc:
        {
            copy_memu->addAction(QIcon(":/toolicon/32x32/el_arc.png"),
                                 makeName(AttrCenter, "arcId", tr("ElArc_"),       tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Radius1ElArc_"), tool_id));
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Radius2ElArc_"), tool_id));
            copy_memu->addSeparator();
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle1ElArc_"), tool_id));
            copy_memu->addAction(makeName(AttrCenter, "arcId", tr("Angle2ElArc_"), tool_id));

            break;
        }
        case Tool::CutArc:
        {
            // Get the arc ID attribute (e.g., arc="13")
            quint32 arc_id = tool_element.attribute(AttrArc).toUInt();
            if (arc_id == 0)
            {
                break;
            }

            // Look up the <arc> element
            const QDomElement arc_element = doc->elementById(arc_id);
            if (arc_element.isNull())
            {
                break;
            }

            // Get the center point ID attribute (e.g., center="7")
            quint32 center_id = arc_element.attribute(AttrCenter).toUInt();
            if (center_id == 0)
            {
                break;
            }

            // Look up the center <point> element (e.g., id="7")
            const QDomElement center_element = doc->elementById(center_id);
            if (center_element.isNull())
            {
                break;
            }

            // Extract the name attribute (e.g., name="A5")
            QString center_name = center_element.attribute(AttrName);
            if (center_name.isEmpty())
            {
                break;
            }

            // Assemble the two resulting arc names using clean QString::number() math
            QString text1 = tr("Arc_") + QString("%1_%2").arg(center_name, QString::number(tool_id + 1));
            copy_memu->addAction(QIcon(":/toolicon/32x32/arc_cut.png"), text1);

            QString text2 = tr("Arc_") + QString("%1_%2").arg(center_name, QString::number(tool_id + 2));
            copy_memu->addAction(text2);

            break;
        }

        // Tools that produce just a point and no variables
        case Tool::BasePoint:
        case Tool::Triangle:
        case Tool::PointOfIntersectionArcs:
        case Tool::PointOfIntersectionCircles:
        case Tool::PointOfIntersectionCurves:
        case Tool::PointFromCircleAndTangent:
        case Tool::PointFromArcAndTangent:
        case Tool::Rotation:
        case Tool::Move:
        case Tool::MirrorByLine:
        case Tool::MirrorByAxis:
        {
            break;
        }
    }

    // Add delete menu item.
    QAction *delete_action = menu.addAction(QIcon::fromTheme("edit-delete"), VDrawTool::tr("Delete"));
    if (show_remove == RemoveOption::Enable)
    {
        if (ref == Referens::Follow)
        {
            if (isUsed())
            {
                qCDebug(vTool, "Delete disabled. Tool has children.");
                delete_action->setEnabled(false);
            }
            else
            {
                qCDebug(vTool, "Delete enabled. Tool has no children.");
                delete_action->setEnabled(true);
            }
        }
        else
        {
            qCDebug(vTool, "Delete enabled. Ignore referens value.");
            delete_action->setEnabled(true);
        }
    }
    else
    {
        qCDebug(vTool, "Delete disabled.");
        delete_action->setEnabled(false);
    }

    QMap<quint32,QString> groups_containing_objecct     =  doc->getGroupsContainingItem(tool_id, objecct_id, true);
    QMap<quint32,QString> groups_not_containing_object =  doc->getGroupsContainingItem(tool_id, objecct_id, false);

    // Add Move Group menu item
    QActionGroup *move_group_object_action= new QActionGroup(this);

    if (!groups_containing_objecct.empty() && !groups_not_containing_object.empty())
    {
        QMenu *move_group_menu = menu.addMenu(QIcon("://icon/svg/list-move.svg"), VDrawTool::tr("Move Group Object"));
        QStringList source_list = QStringList(groups_containing_objecct.values());
        QStringList dest_list   = QStringList(groups_not_containing_object.values());
        source_list.sort(Qt::CaseInsensitive);
        dest_list.sort(Qt::CaseInsensitive);

        for(int i=0; i < source_list.count(); ++i)
        {
            const quint32 sourec_id = groups_containing_objecct.key(source_list[i]);

            for(int j=0; j < dest_list.count(); ++j)
            {
                QAction *move_group_item_action = move_group_menu->addAction(VDrawTool::tr("From ") + source_list[i] + VDrawTool::tr(" to ") + dest_list[j]);
                move_group_object_action->addAction(move_group_item_action);
                const quint32 destId = groups_not_containing_object.key(dest_list[j]);

                QPair<quint32, quint32> data(sourec_id, destId);
                QVariant variant_data = QVariant::fromValue(data);
                move_group_item_action->setData(variant_data);
            }
        }
    }

    // Add Group Item menu item
    QActionGroup *add_group_menu_action = new QActionGroup(this);
    if (!groups_not_containing_object.empty())
    {
        QMenu *add_group_menu = menu.addMenu(QIcon("://icon/32x32/add.png"), VDrawTool::tr("Add Group Object"));
        QStringList list = QStringList(groups_not_containing_object.values());
        list.sort(Qt::CaseInsensitive);

        for(int i=0; i<list.count(); ++i)
        {
            QAction *add_group_object_action = add_group_menu->addAction(list[i]);
            add_group_menu_action->addAction(add_group_object_action);
            const quint32 group_id = groups_not_containing_object.key(list[i]);
            add_group_object_action->setData(group_id);
            groups_not_containing_object.remove(group_id);   // delete any duplicate groups
        }
    }

    // Add remove Group menu item
    QActionGroup *delete_group_action = new QActionGroup(this);

    if (!groups_containing_objecct.empty())
    {
        QMenu *remove_group_obj_menu = menu.addMenu(QIcon("://icon/32x32/remove.png"),
                                                    VDrawTool::tr("Remove Group Object"));

        QStringList list = QStringList(groups_containing_objecct.values());
        list.sort(Qt::CaseInsensitive);

        for(int i=0; i<list.count(); ++i)
        {
            QAction *remove_group_obj_action = remove_group_obj_menu->addAction(list[i]);
            delete_group_action->addAction(remove_group_obj_action);
            const quint32 group_id = groups_containing_objecct.key(list[i]);
            remove_group_obj_action->setData(group_id);
            groups_containing_objecct.remove(group_id);
        }
    }

    // Show menu and handle selcction
    QAction *selected_action = menu.exec(event->screenPos());
    if(selected_action == nullptr)
    {
        return;
    }
    if (selected_action == properties_action)
    {
        qCDebug(vTool, "Show options.");
        emit qApp->getSceneView()->itemClicked(nullptr);
        m_dialog = QSharedPointer<Dialog>(new Dialog(getData(), m_id, qApp->getMainWindow()));

        connect(m_dialog.data(), &DialogTool::DialogClosed, this, &VDrawTool::FullUpdateFromGuiOk);
        connect(m_dialog.data(), &DialogTool::DialogApplied, this, &VDrawTool::FullUpdateFromGuiApply);

        this->setDialog();

        m_dialog->show();
    }
    else if (selected_action == delete_action)
    {
        qCDebug(vTool, "Deleting Object.");
        deleteTool(); // do not catch exception here
        return;       // Leave this method immediately after call!!!
    }
    else if (selected_action == show_point_name_action)
    {
        updatePointNameVisibility(objecct_id, selected_action->isChecked());
    }
    else if (selected_action->actionGroup() == add_group_menu_action)
    {
        quint32 group_id = selected_action->data().toUInt();
        QDomElement item = doc->addGroupItem(this->getId(), objecct_id, group_id);

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        {
            AddGroupItem *command = new AddGroupItem(item, doc, group_id);
            connect(command, &AddGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
            qApp->getUndoStack()->push(command);
        }
    }
    else if (selected_action->actionGroup() == move_group_object_action)
    {
        QVariant retrieved_variant = selected_action->data();
        QPair<quint32, quint32> retrieved_pair = retrieved_variant.value<QPair<quint32, quint32>>();
        const quint32 source_group_id       = retrieved_pair.first;
        const quint32 destination_group_id  = retrieved_pair.second;
        const bool source_lock = doc->getGroupLock(source_group_id);
        const bool dest_lock   = doc->getGroupLock(destination_group_id);

        //only move if both groups are unlocked
        if ((source_lock == false) && (dest_lock == false))
        {
            QDomElement source_object = doc->removeGroupItem(this->getId(), objecct_id, source_group_id);
            QDomElement dest_object   = doc->addGroupItem(this->getId(), objecct_id, destination_group_id);

            VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
            SCASSERT(scene != nullptr)
            scene->clearSelection();

            VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
            SCASSERT(window != nullptr)
            {
                MoveGroupItem *command = new MoveGroupItem(source_object, dest_object, doc,
                                                           source_group_id, destination_group_id);
                connect(command, &MoveGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
                qApp->getUndoStack()->push(command);
            }
        }
    }
    else if (selected_action->actionGroup() == delete_group_action)
    {
        quint32 group_id = selected_action->data().toUInt();
        QDomElement item = doc->removeGroupItem(this->getId(), objecct_id, group_id);

        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(qApp->getCurrentScene());
        SCASSERT(scene != nullptr)
        scene->clearSelection();

        VAbstractMainWindow *window = qobject_cast<VAbstractMainWindow *>(qApp->getMainWindow());
        SCASSERT(window != nullptr)
        {
            RemoveGroupItem *command = new RemoveGroupItem(item, doc, group_id);
            connect(command, &RemoveGroupItem::updateGroups, window, &VAbstractMainWindow::updateGroups);
            qApp->getUndoStack()->push(command);
        }
    }

    else if (copy_memu && copy_memu->actions().contains(selected_action))
    {
        QString copy_text = selected_action->text();
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(copy_text);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief  Helper function to generate a formatted variable string for a drawing tool.
///
/// @param  attribute1 First characteristic modifier for the tool name.
/// @param  attribute2 Second characteristic modifier for the tool name.
/// @param  prefix     The base category or naming prefix (e.g., "Line_", "AngleLine_").
/// @param  tool_id    Unique numeric identifier for the tool instance.
/// @return QString    The constructed tool variable.
//---------------------------------------------------------------------------------------------------------------------
inline QString  VDrawTool::makeName(const QString &attribute1, const QString &attribute2,
                                          const QString &prefix, const quint32 &tool_id)
{
    QString line_name   = QString("");
    QString point1_name = QString("");
    QString point2_name = QString("");

    const QDomElement dom_element = doc->elementById(tool_id);
    if (dom_element.isElement())
    {

        if (attribute1 == AttrName)
        {
            point1_name = doc->GetParametrString(dom_element, attribute1, "") + "_";
        }
        else if (attribute1 == "arcId")
        {
            point1_name = doc->GetParametrString(dom_element, "id", "") + "_";
        }
        else if (attribute1 == "cutSpl")
        {
            quint32 spl_id = doc->GetParametrUInt(dom_element, "spline", "0");
            const QDomElement spl_element = doc->elementById(spl_id);
            if (spl_element.isElement())
            {
                point1_name = data.GeometricObject<VPointF>(doc->GetParametrUInt(spl_element, AttrPoint1, "0"))->name() + "_";
            }
        }
        else
        {
            point1_name = data.GeometricObject<VPointF>(doc->GetParametrUInt(dom_element, attribute1, "0"))->name() + "_";
        }
        if (attribute2 == AttrName)
        {
            point2_name = doc->GetParametrString(dom_element, attribute2, "");
        }
        else if (attribute2 == "arcId")
        {
            point2_name = doc->GetParametrString(dom_element, "id", "");
        }
        else if (attribute2 == "cutSpl")
        {
            quint32 spl_id = doc->GetParametrUInt(dom_element, "spline", "0");
            const QDomElement spl_element = doc->elementById(spl_id);
            if (spl_element.isElement())
            {
                point2_name = data.GeometricObject<VPointF>(doc->GetParametrUInt(spl_element, AttrPoint4, "0"))->name();
            }
        }
        else
        {
            point2_name = data.GeometricObject<VPointF>(doc->GetParametrUInt(dom_element, attribute2, "0"))->name();
        }
        line_name = prefix + point1_name + point2_name;
    }
    return line_name;
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ShowItem highlight tool.
/// @param item tool.
/// @param id object id in container.
/// @param enable enable or disable highlight.
//---------------------------------------------------------------------------------------------------------------------
template <typename Item>
void VDrawTool::ShowItem(Item *item, quint32 id, bool enable)
{
    SCASSERT(item != nullptr)
    if (id == item->m_id)
    {
        ShowVisualization(enable);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/// @brief ObjectName get object (point, curve, arc) name.
/// @param id object id in container.
//---------------------------------------------------------------------------------------------------------------------
template <typename T>
QString VDrawTool::ObjectName(quint32 id) const
{
    try
    {
        return data.GeometricObject<T>(id)->name();
    }
    catch (const VExceptionBadId &error)
    {
        qCWarning(vTool, "Error! Couldn't get object name by id = %s. %s %s", qUtf8Printable(QString().setNum(id)),
                qUtf8Printable(error.ErrorMessage()),
                qUtf8Printable(error.DetailedInformation()));
        return QString("");// Return empty string for property browser
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VDrawTool::InitDrawToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    QObject::connect(tool, &T::chosenTool, scene, &VMainGraphicsScene::chosenItem);
    QObject::connect(tool, &T::ChangedToolSelection, scene, &VMainGraphicsScene::SelectedItem);
    QObject::connect(scene, &VMainGraphicsScene::DisableItem, tool, &T::Disable);
    QObject::connect(scene, &VMainGraphicsScene::EnableToolMove, tool, &T::EnableToolMove);
    QObject::connect(scene, &VMainGraphicsScene::curvePiecesMode, tool, &T::piecesMode);
    QObject::connect(scene, &VMainGraphicsScene::ItemSelection, tool, &T::ToolSelectionType);
}

#endif // VDRAWTOOL_H

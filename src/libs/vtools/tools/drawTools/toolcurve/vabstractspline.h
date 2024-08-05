//  @file   vabstractspline.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2024 Seamly, LLC
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

/************************************************************************
 **
 **  @file   vabstractspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 3, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
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

#ifndef VABSTRACTSPLINE_H
#define VABSTRACTSPLINE_H

#include <qcompilerdetection.h>
#include <QDomElement>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QMetaObject>
#include <QObject>
#include <QPainterPath>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

#include "../../../visualization/line/visline.h"
#include "../vdrawtool.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/def.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"

class VControlPointSpline;
template <class T> class QSharedPointer;

class VAbstractSpline:public VDrawTool, public QGraphicsPathItem
{
    Q_OBJECT
public:
                         VAbstractSpline(VAbstractPattern *doc, VContainer *data, quint32 id,
                                        QGraphicsItem * parent = nullptr);
    virtual             ~VAbstractSpline() Q_DECL_EQ_DEFAULT;

    virtual QPainterPath shape() const override;
    virtual void         paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    virtual int          type() const  override {return Type;}

    enum { Type = UserType + static_cast<int>(Tool::AbstractSpline)};

    virtual QString      getTagName() const override;
    void                 showHandles(bool show);

    QString              getLineColor() const;
    void                 setLineColor(const QString &value);

    QString              GetPenStyle() const;
    void                 SetPenStyle(const QString &value);

    QString              getLineWeight() const;
    void                 setLineWeight(const QString &value);

    QString              name() const;

    virtual void         GroupVisibility(quint32 object, bool visible) override;

public slots:
    virtual void         FullUpdateFromFile () override;
    virtual void         Disable(bool disable, const QString &draftBlockName) override;
    virtual void         piecesMode(bool mode) override;
    virtual void         AllowHover(bool enabled) override;
    virtual void         AllowSelecting(bool enabled) override;
    virtual QString      makeToolTip() const override;

signals:
    // @brief setEnabledPoint disable control points.
    // @param enable enable or diasable points.
    void                 setEnabledPoint(bool enable);

protected:
    QVector<VControlPointSpline *> m_controlPoints; /*!< @brief m_controlPoints list pointers of control points. */
    SceneObject          m_sceneType;
    bool                 m_isHovered;
    bool                 m_piecesMode;

    virtual void         refreshGeometry();
    virtual void         ShowTool(quint32 id, bool enable) override;
    virtual void         hoverEnterEvent ( QGraphicsSceneHoverEvent * event ) override;
    virtual void         hoverLeaveEvent ( QGraphicsSceneHoverEvent * event ) override;
    virtual QVariant     itemChange ( GraphicsItemChange change, const QVariant &value ) override;
    virtual void         keyReleaseEvent(QKeyEvent * event) override;
    virtual void         mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void         mouseReleaseEvent ( QGraphicsSceneMouseEvent * event ) override;
    virtual void         ReadToolAttributes(const QDomElement &domElement) override;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void         refreshCtrlPoints();
    virtual void         contextMenuEvent (QGraphicsSceneContextMenuEvent * event ) override;
    VSpline              correctedSpline(const VSpline &spline, const SplinePointPosition &position,
                                         const QPointF &pos) const;

    template <typename T>
    void                 ShowToolVisualization(bool show);

    template <typename T>
    static void          initSplineToolConnections(VMainGraphicsScene *scene, T *tool);

    template <typename T>
    static void          initSplinePathToolConnections(VMainGraphicsScene *scene, T *tool);

    template <typename T>
    static void          initArcToolConnections(VMainGraphicsScene *scene, T *tool);

    template <typename T>
    static void          initElArcToolConnections(VMainGraphicsScene *scene, T *tool);

private:
    Q_DISABLE_COPY(VAbstractSpline)

    void                 initDefShape();
};

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void VAbstractSpline::ShowToolVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<T>();
            SetVisualization();
        }
        else
        {
            if (T *visual = qobject_cast<T *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis;
    }

    if (m_piecesMode)
    {
        showHandles(m_piecesMode);
    }
    else
    {
        showHandles(show);
    }

    if (QGraphicsScene *sc = scene())
    { // Showing/hiding control points require recalculation scene size.
        VMainGraphicsView::NewSceneRect(sc, qApp->getSceneView(), this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractSpline::initSplineToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplineItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractSpline::initSplinePathToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableSplinePathItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractSpline::initArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableArcItemSelection, tool, &T::AllowSelecting);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
void VAbstractSpline::initElArcToolConnections(VMainGraphicsScene *scene, T *tool)
{
    SCASSERT(scene != nullptr)
    SCASSERT(tool != nullptr)

    InitDrawToolConnections(scene, tool);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemHover, tool, &T::AllowHover);
    QObject::connect(scene, &VMainGraphicsScene::EnableElArcItemSelection, tool, &T::AllowSelecting);
}

#endif // VABSTRACTSPLINE_H

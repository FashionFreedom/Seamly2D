/******************************************************************************
*   @file   vtooloptionspropertybrowser.h
**  @author Douglas S Caskey
**  @date   30 Apr, 2023
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
 **  @file   vtooloptionspropertybrowser.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 8, 2014
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

#ifndef VTOOLOPTIONSPROPERTYBROWSER_H
#define VTOOLOPTIONSPROPERTYBROWSER_H

#include <QEvent>
#include <QObject>
#include <QMap>

#include "../vgeometry/vgeometrydef.h"
#include "../vpropertyexplorer/vproperty.h"
#include "../vpropertyexplorer/vpropertymodel.h"
#include "../vpropertyexplorer/vpropertyformview.h"

class QDockWidget;
class QGraphicsItem;
class QScrollArea;
class VFormula;
class VContainer;

class VToolOptionsPropertyBrowser : public QObject
{
    Q_OBJECT
public:
    explicit VToolOptionsPropertyBrowser(const VContainer *data, QDockWidget *parent);
    void clearPropertyBrowser();

public slots:
    void itemClicked(QGraphicsItem *item);
    void updateOptions();
    void refreshOptions();

private slots:
    void userChangedData(VPE::VProperty *property);

protected:
    Q_DISABLE_COPY(VToolOptionsPropertyBrowser)
    const VContainer                *m_data;
    VPE::VPropertyModel             *propertyModel;
    VPE::VPropertyFormView          *formView;

    QGraphicsItem                   *currentItem;
    QMap<VPE::VProperty *, QString>  propertyToId;
    QMap<QString, VPE::VProperty *>  idToProperty;
    QString                          m_centerPointStr;

private:
    void addProperty(VPE::VProperty *property, const QString &id);
    void showItemOptions(QGraphicsItem *item);

    template<class Tool>
    QMap<QString, quint32> getObjectList(Tool *tool, GOType objType);

    template<class Tool>
    void setPointName(const QString &name);

    template<class Tool>
    void setPointName1(const QString &name);

    template<class Tool>
    void setPointName2(const QString &name);

    template<class Tool>
    void setOperationSuffix(const QString &suffix);

    template<class Type>
    Type getCrossPoint(const QVariant &value);

    template<class Tool>
    void setCirclesCrossPoint(const QVariant &value);

    template<class Tool>
    void setCurveVCrossPoint(const QVariant &value);

    template<class Tool>
    void setCurveHCrossPoint(const QVariant &value);

    template<class Tool>
    void setAxisType(const QVariant &value);

    template<class Tool>
    void addPropertyObjectName(Tool *tool, const QString &propertyName, bool readOnly = false);

    template<class Tool>
    void addPropertyPointName1(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyPointName2(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyOperationSuffix(Tool *tool, const QString &propertyName, bool readOnly = false);

    template<class Tool>
    void addPropertyLineName(Tool *tool, const QString &propertyName, bool readOnly = false);

    template<class Tool>
    void addPropertyCurveName(Tool *tool, const QString &propertyName, const QString &prefix,
                              const QString &firstPoint, const QString &secondPoint, bool readOnly = false);

    template<class Tool>
    void addPropertyCrossPoint(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyVCrossPoint(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyHCrossPoint(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyAxisType(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyLineType(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyCurveLineType(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyLineWeight(Tool *tool, const QString &propertyName);

    template<class Tool>
    void addPropertyLineColor(Tool *tool, const QString &propertyName,  const QString &id);

    template<class Tool>
    void addObjectProperty(Tool *tool, const QString &pointName, const QString &propertyName,
                           const QString &id, GOType objType);

    void addPropertyFormula(const QString &propertyName, const VFormula &formula, const QString &attrName);
    void addPropertyParentPointName(const QString &pointName, const QString &propertyName,
                                    const QString &propertyAttribute);

    void addPropertyLabel(const QString &propertyName, const QString &propertyAttribute);

    QStringList propertiesList() const;

    void changeDataToolSinglePoint(VPE::VProperty *property);
    void changeDataToolEndLine(VPE::VProperty *property);
    void changeDataToolAlongLine(VPE::VProperty *property);
    void changeDataToolArc(VPE::VProperty *property);
    void changeDataToolArcWithLength(VPE::VProperty *property);
    void changeDataToolBisector(VPE::VProperty *property);
    void changeDataToolTrueDarts(VPE::VProperty *property);
    void changeDataToolCutArc(VPE::VProperty *property);
    void changeDataToolCutSpline(VPE::VProperty *property);
    void changeDataToolCutSplinePath(VPE::VProperty *property);
    void changeDataToolHeight(VPE::VProperty *property);
    void changeDataToolLine(VPE::VProperty *property);
    void changeDataToolLineIntersect(VPE::VProperty *property);
    void changeDataToolNormal(VPE::VProperty *property);
    void changeDataToolPointOfContact(VPE::VProperty *property);
    void changeDataToolPointOfIntersection(VPE::VProperty *property);
    void changeDataToolPointOfIntersectionArcs(VPE::VProperty *property);
    void changeDataToolPointOfIntersectionCircles(VPE::VProperty *property);
    void changeDataToolPointOfIntersectionCurves(VPE::VProperty *property);
    void changeDataToolPointFromCircleAndTangent(VPE::VProperty *property);
    void changeDataToolPointFromArcAndTangent(VPE::VProperty *property);
    void changeDataToolShoulderPoint(VPE::VProperty *property);
    void changeDataToolSpline(VPE::VProperty *property);
    void changeDataToolCubicBezier(VPE::VProperty *property);
    void changeDataToolSplinePath(VPE::VProperty *property);
    void changeDataToolCubicBezierPath(VPE::VProperty *property);
    void changeDataToolTriangle(VPE::VProperty *property);
    void changeDataToolLineIntersectAxis(VPE::VProperty *property);
    void changeDataToolCurveIntersectAxis(VPE::VProperty *property);
    void changeDataToolRotation(VPE::VProperty *property);
    void changeDataToolMove(VPE::VProperty *property);
    void changeDataToolMirrorByLine(VPE::VProperty *property);
    void changeDataToolMirrorByAxis(VPE::VProperty *property);
    void changeDataToolEllipticalArc(VPE::VProperty *property);

    void showOptionsToolSinglePoint(QGraphicsItem *item);
    void showOptionsToolEndLine(QGraphicsItem *item);
    void showOptionsToolAlongLine(QGraphicsItem *item);
    void showOptionsToolArc(QGraphicsItem *item);
    void showOptionsToolArcWithLength(QGraphicsItem *item);
    void showOptionsToolBisector(QGraphicsItem *item);
    void showOptionsToolTrueDarts(QGraphicsItem *item);
    void showOptionsToolCutArc(QGraphicsItem *item);
    void showOptionsToolCutSpline(QGraphicsItem *item);
    void showOptionsToolCutSplinePath(QGraphicsItem *item);
    void showOptionsToolHeight(QGraphicsItem *item);
    void showOptionsToolLine(QGraphicsItem *item);
    void showOptionsToolLineIntersect(QGraphicsItem *item);
    void showOptionsToolNormal(QGraphicsItem *item);
    void showOptionsToolPointOfContact(QGraphicsItem *item);
    void showOptionsToolPointOfIntersection(QGraphicsItem *item);
    void showOptionsToolPointOfIntersectionArcs(QGraphicsItem *item);
    void showOptionsToolPointOfIntersectionCircles(QGraphicsItem *item);
    void showOptionsToolPointOfIntersectionCurves(QGraphicsItem *item);
    void showOptionsToolPointFromCircleAndTangent(QGraphicsItem *item);
    void showOptionsToolPointFromArcAndTangent(QGraphicsItem *item);
    void showOptionsToolShoulderPoint(QGraphicsItem *item);
    void showOptionsToolSpline(QGraphicsItem *item);
    void showOptionsToolCubicBezier(QGraphicsItem *item);
    void showOptionsToolSplinePath(QGraphicsItem *item);
    void showOptionsToolCubicBezierPath(QGraphicsItem *item);
    void showOptionsToolTriangle(QGraphicsItem *item);
    void showOptionsToolLineIntersectAxis(QGraphicsItem *item);
    void showOptionsToolCurveIntersectAxis(QGraphicsItem *item);
    void showOptionsToolRotation(QGraphicsItem *item);
    void showOptionsToolMove(QGraphicsItem *item);
    void showOptionsToolMirrorByLine(QGraphicsItem *item);
    void showOptionsToolMirrorByAxis(QGraphicsItem *item);
    void showOptionsToolEllipticalArc(QGraphicsItem *item);

    void updateOptionsToolSinglePoint();
    void updateOptionsToolEndLine();
    void updateOptionsToolAlongLine();
    void updateOptionsToolArc();
    void updateOptionsToolArcWithLength();
    void updateOptionsToolBisector();
    void updateOptionsToolTrueDarts();
    void updateOptionsToolCutArc();
    void updateOptionsToolCutSpline();
    void updateOptionsToolCutSplinePath();
    void updateOptionsToolHeight();
    void updateOptionsToolLine();
    void updateOptionsToolLineIntersect();
    void updateOptionsToolNormal();
    void updateOptionsToolPointOfContact();
    void updateOptionsToolPointOfIntersection();
    void updateOptionsToolPointOfIntersectionArcs();
    void updateOptionsToolPointOfIntersectionCircles();
    void updateOptionsToolPointOfIntersectionCurves();
    void updateOptionsToolPointFromCircleAndTangent();
    void updateOptionsToolPointFromArcAndTangent();
    void updateOptionsToolShoulderPoint();
    void updateOptionsToolSpline();
    void updateOptionsToolCubicBezier();
    void updateOptionsToolSplinePath();
    void updateOptionsToolCubicBezierPath();
    void updateOptionsToolTriangle();
    void updateOptionsToolLineIntersectAxis();
    void updateOptionsToolCurveIntersectAxis();
    void updateOptionsToolRotation();
    void updateOptionsToolMove();
    void updateOptionsToolMirrorByLine();
    void updateOptionsToolMirrorByAxis();
    void updateOptionsToolEllipticalArc();
};

#endif // VTOOLOPTIONSPROPERTYBROWSER_H

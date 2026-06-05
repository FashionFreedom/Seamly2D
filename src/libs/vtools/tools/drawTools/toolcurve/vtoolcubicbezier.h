/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                            *
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
 **  @file   vtoolcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Seamly2D project
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

#ifndef VTOOLCUBICBEZIER_H
#define VTOOLCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vabstractspline.h"

class VCubicBezier;
class VFormula;
template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
// VToolCubicBezier — parametric cubic Bézier with four optional computation modes:
//
//  State 1  (targetLength empty,  autoSmooth false): manual c1 + c2 from formulas
//  State 2  (targetLength set,    autoSmooth false): bisection finds c2 so arc-length == target
//  State 3  (targetLength empty,  autoSmooth true ): Hobby algorithm computes both c1 and c2
//  State 4  (targetLength set,    autoSmooth true ): Hobby proportions + bisection for scale factor
//---------------------------------------------------------------------------------------------------------------------
class VToolCubicBezier : public VAbstractSpline
{
    Q_OBJECT
public:
    virtual             ~VToolCubicBezier() Q_DECL_EQ_DEFAULT;
    virtual void         setDialog() override;

    // Create from GUI dialog
    static VToolCubicBezier *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                    VAbstractPattern *doc, VContainer *data);

    // Create from XML (used by both the new and the legacy formats)
    static VToolCubicBezier *Create(const quint32 _id,
                                    const quint32 point1Id, const quint32 point4Id,
                                    QString &angle1, QString &angle2,
                                    QString &c1Length, QString &c2Length,
                                    QString &targetLength, bool autoSmooth,
                                    const QString &color, const QString &penStyle,
                                    const QString &lineWeight,
                                    VMainGraphicsScene *scene, VAbstractPattern *doc,
                                    VContainer *data, const Document &parse,
                                    const Source &typeCreation);

    static const QString ToolType;
    virtual int          type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CubicBezier)};

    // Point name accessors (used by property browser and history)
    QString              GetP1Name() const;
    QString              GetP4Name() const;

    // Formula accessors (used by property browser)
    VFormula             GetFormulaAngle1() const;
    void                 SetFormulaAngle1(const VFormula &value);

    VFormula             GetFormulaAngle2() const;
    void                 SetFormulaAngle2(const VFormula &value);

    VFormula             GetFormulaC1Length() const;
    void                 SetFormulaC1Length(const VFormula &value);

    VFormula             GetFormulaC2Length() const;
    void                 SetFormulaC2Length(const VFormula &value);

    VFormula             GetFormulaTargetLength() const;
    void                 SetFormulaTargetLength(const VFormula &value);

    bool                 GetAutoSmooth() const;
    void                 SetAutoSmooth(bool value);

    // Minimum arc length string (c2=0 degenerate) for UI hint in State 2/4
    QString              getMinLengthString() const;

    virtual void         ShowVisualization(bool show) override;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) override;

protected:
    virtual void         RemoveReferens() override;
    virtual void         SaveDialog(QDomElement &domElement) override;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void         SetVisualization() override;
    virtual void         refreshGeometry() override;

private:
    Q_DISABLE_COPY(VToolCubicBezier)

    // Formula strings — the VCubicBezier in the container holds computed geometry only.
    QString              m_angle1;
    QString              m_angle2;
    QString              m_c1Length;      // empty when autoSmooth=true
    QString              m_c2Length;      // empty unless State 1 (both checkboxes off)
    QString              m_targetLength;  // empty when "enforce target length" checkbox is off
    bool                 m_autoSmooth;    // true when "Auto-Smooth (Hobby)" checkbox is on

    VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                     const QString &angle1, const QString &angle2,
                     const QString &c1Length, const QString &c2Length,
                     const QString &targetLength, bool autoSmooth,
                     const Source &typeCreation, QGraphicsItem *parent = nullptr);

    void                 SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl);
};

#endif // VTOOLCUBICBEZIER_H

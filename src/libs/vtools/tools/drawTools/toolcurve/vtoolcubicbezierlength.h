/***************************************************************************
 **  @file   vtoolcubicbezierlength.h
 **  @author Seamly2D contributors
 **
 **  @brief  Cubic Bézier curve whose length is matched to a reference via bisection.
 **
 **  Fixed parameters : start point P1, end point P4,
 **                     start handle angle α1, start handle length c1,
 **                     end handle angle α2.
 **  Computed variable: end handle length c2 (bisection so that
 **                     curve length == targetLength ± 0.01 mm).
 **
 **  Formula strings are stored in the tool instance; the VCubicBezier
 **  in the container holds only the computed geometry.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#ifndef VTOOLCUBICBEZIERLENGTH_H
#define VTOOLCUBICBEZIERLENGTH_H

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
template <class T> class QSharedPointer;

class VToolCubicBezierLength : public VAbstractSpline
{
    Q_OBJECT
public:
    virtual             ~VToolCubicBezierLength() Q_DECL_EQ_DEFAULT;
    virtual void         setDialog() Q_DECL_OVERRIDE;

    static VToolCubicBezierLength *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                          VAbstractPattern *doc, VContainer *data);
    static VToolCubicBezierLength *Create(const quint32 _id,
                                          const quint32 point1Id, const quint32 point4Id,
                                          QString &angle1, QString &angle2,
                                          QString &c1Length, QString &targetLength,
                                          const QString &color, const QString &penStyle,
                                          const QString &lineWeight,
                                          VMainGraphicsScene *scene, VAbstractPattern *doc,
                                          VContainer *data, const Document &parse,
                                          const Source &typeCreation);

    static const QString ToolType;
    virtual int          type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CubicBezierLength)};

    VFormula             GetFormulaAngle1() const;
    void                 SetFormulaAngle1(const VFormula &value);

    VFormula             GetFormulaAngle2() const;
    void                 SetFormulaAngle2(const VFormula &value);

    VFormula             GetFormulaC1Length() const;
    void                 SetFormulaC1Length(const VFormula &value);

    VFormula             GetFormulaTargetLength() const;
    void                 SetFormulaTargetLength(const VFormula &value);

    virtual void         ShowVisualization(bool show) Q_DECL_OVERRIDE;

protected slots:
    virtual void         showContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id=NULL_ID) Q_DECL_OVERRIDE;

protected:
    virtual void         RemoveReferens() Q_DECL_OVERRIDE;
    virtual void         SaveDialog(QDomElement &domElement) Q_DECL_OVERRIDE;
    virtual void         SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) Q_DECL_OVERRIDE;
    virtual void         SetVisualization() Q_DECL_OVERRIDE;
    virtual void         refreshGeometry() Q_DECL_OVERRIDE;

private:
    Q_DISABLE_COPY(VToolCubicBezierLength)

    // Formula strings; the VCubicBezier in the container holds computed geometry.
    QString              m_angle1;
    QString              m_angle2;
    QString              m_c1Length;
    QString              m_targetLength;

    VToolCubicBezierLength(VAbstractPattern *doc, VContainer *data, quint32 id,
                            const QString &angle1, const QString &angle2,
                            const QString &c1Length, const QString &targetLength,
                            const Source &typeCreation, QGraphicsItem *parent = nullptr);

    void SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl);
};

#endif // VTOOLCUBICBEZIERLENGTH_H

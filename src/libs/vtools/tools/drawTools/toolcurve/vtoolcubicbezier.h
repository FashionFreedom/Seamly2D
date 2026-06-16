/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *   https://github.com/fashionfreedom/seamly2d                            *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 *************************************************************************/

#ifndef VTOOLCUBICBEZIER_H
#define VTOOLCUBICBEZIER_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPair>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../ifc/xml/vabstractpattern.h"
#include "vabstractspline.h"

class VCubicBezier;
class VFormula;
template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
// VToolCubicBezier — "Kurve Fixiert"
//
// The user selects 4 canvas points: P1 (start), P2 (start handle), P3 (end handle), P4 (end).
// Two optional checkboxes modify how P2/P3 are used:
//
//  State 1 (both off):    classic — VCubicBezier(P1, P2, P3, P4) directly
//  State 2 (A: target):   P2/P3 define proportions; both handles scaled to hit targetLength
//  State 3 (B: smooth):   P2/P3 define directions; Hobby algorithm sets optimal lengths
//  State 4 (both A+B):    P2/P3 define directions; Hobby proportions + bisection for scale
//---------------------------------------------------------------------------------------------------------------------
class VToolCubicBezier : public VAbstractSpline
{
    Q_OBJECT
public:
    virtual              ~VToolCubicBezier() Q_DECL_EQ_DEFAULT;
    virtual void          setDialog() override;

    // Create from GUI dialog (dialog supplies P1-P4 IDs + optional targetLength/autoSmooth)
    static VToolCubicBezier *Create(QSharedPointer<DialogTool> dialog, VMainGraphicsScene *scene,
                                    VAbstractPattern *doc, VContainer *data);

    // Create from XML — called by ParseToolCubicBezier
    static VToolCubicBezier *Create(const quint32 _id,
                                     const quint32 p1Id, const quint32 p2Id,
                                     const quint32 p3Id, const quint32 p4Id,
                                     const QString &targetLength, bool autoSmooth,
                                     const QString &color, const QString &penStyle,
                                     const QString &lineWeight,
                                     VMainGraphicsScene *scene, VAbstractPattern *doc,
                                     VContainer *data, const Document &parse,
                                     const Source &typeCreation);

    static const QString  ToolType;
    virtual int           type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::CubicBezier)};

    // Point name accessors (P1/P4 from stored spline; P2/P3 from original canvas IDs)
    QString               FirstPointName()  const;
    QString               SecondPointName() const;
    QString               ThirdPointName()  const;
    QString               ForthPointName()  const;

    // Access the computed VCubicBezier geometry
    VCubicBezier          getSpline() const;
    void                  setSpline(const VCubicBezier &spl);

    // Formula-based target length (for property browser)
    VFormula              GetFormulaTargetLength() const;
    void                  SetFormulaTargetLength(const VFormula &value);

    bool                  GetAutoSmooth() const;
    void                  SetAutoSmooth(bool value);

    // Minimum arc length hint for target length field
    QString               getMinLengthString() const;

    virtual void          ShowVisualization(bool show) override;

protected slots:
    virtual void          showContextMenu(QGraphicsSceneContextMenuEvent *event,
                                          quint32 id=NULL_ID) override;

protected:
    virtual void          RemoveReferens() override;
    virtual void          SaveDialog(QDomElement &domElement) override;
    virtual void          SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void          SetVisualization() override;
    virtual void          refreshGeometry() override;

private:
    Q_DISABLE_COPY(VToolCubicBezier)

    // Original canvas point IDs for P2 and P3 (P1 and P4 are in the stored VCubicBezier)
    quint32               m_p2Id;
    quint32               m_p3Id;

    // Optional features (empty / false = State 1 classic behavior)
    QString               m_targetLength;
    bool                  m_autoSmooth;

    VToolCubicBezier(VAbstractPattern *doc, VContainer *data, quint32 id,
                      quint32 p2Id, quint32 p3Id,
                      const QString &targetLength, bool autoSmooth,
                      const Source &typeCreation, QGraphicsItem *parent = nullptr);

    void                  SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl);
    void                  recomputeGeometry();
};

#endif // VTOOLCUBICBEZIER_H

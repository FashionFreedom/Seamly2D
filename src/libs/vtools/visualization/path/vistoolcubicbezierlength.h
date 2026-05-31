/***************************************************************************
 **  @file   vistoolcubicbezierlength.h
 **  @author Seamly2D contributors
 **
 **  @brief  Visualization for the cubic Bézier curve with matched length tool.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **************************************************************************/

#ifndef VISTOOLCUBICBEZIERLENGTH_H
#define VISTOOLCUBICBEZIERLENGTH_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCubicBezierLength : public VisPath
{
    Q_OBJECT
public:
    explicit VisToolCubicBezierLength(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual ~VisToolCubicBezierLength() Q_DECL_EQ_DEFAULT;

    virtual void RefreshGeometry() Q_DECL_OVERRIDE;

    void         setObject4Id(const quint32 &value);
    void         setAngle1(const QString &value);
    void         setAngle2(const QString &value);
    void         setC1Length(const QString &value);
    void         setTargetLength(const QString &value);

    virtual int  type() const Q_DECL_OVERRIDE {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::ToolCubicBezierLength)};

protected:
    Q_DISABLE_COPY(VisToolCubicBezierLength)

    quint32         object4Id;
    QString         m_angle1;
    QString         m_angle2;
    QString         m_c1Length;
    QString         m_targetLength;

    VScaledEllipse *point1;
    VScaledEllipse *point4;
    VScaledLine    *helpLine1;
    VScaledLine    *helpLine2;
};

#endif // VISTOOLCUBICBEZIERLENGTH_H

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
 **  @file   vtoollinepoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VTOOLLINEPOINT_H
#define VTOOLLINEPOINT_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vpatterndb/vformula.h"
#include "../vtoolsinglepoint.h"
#include "../vmisc/def.h"

template <class T> class QSharedPointer;

/**
 * @brief The VToolLinePoint class parent for all tools what create point with line.
 */
class VToolLinePoint : public VToolSinglePoint
{
    Q_OBJECT
public:
                      VToolLinePoint(VAbstractPattern *doc, VContainer *data, const quint32 &id,
                                     const QString &lineType, const QString &lineWeight,
                                     const QString &lineColor, const QString &formula,
                                     const quint32 &basePointId, const qreal &angle, QGraphicsItem * parent = nullptr);
    virtual          ~VToolLinePoint() override;

    virtual int       type() const override {return Type;}
    enum              {Type = UserType + static_cast<int>(Tool::LinePoint)};

    virtual void      paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                            QWidget *widget = nullptr) override;

    VFormula          GetFormulaLength() const;
    void              SetFormulaLength(const VFormula &value);

    QString           BasePointName() const;

    quint32           GetBasePointId() const;
    void              SetBasePointId(const quint32 &value);

    qreal             GetAngle() const;
    void              SetAngle(const qreal &value);

    QString           getLineColor() const;
    void              setLineColor(const QString &value);

public slots:
    virtual void      Disable(bool disable, const QString &draftBlockName) override;
    virtual void      FullUpdateFromFile() override;

protected:
    QString           formulaLength; /** @brief formula string with length formula. */
    qreal             angle; /** @brief angle line angle. */
    quint32           basePointId; /** @brief basePointId id base line point. */
    VScaledLine      *mainLine; /** @brief mainLine line item. */
    QString           lineColor; /** @brief lineColor color of a line. */

    virtual void      RefreshGeometry();
    virtual void      RemoveReferens() override;
    virtual void      SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    virtual void      hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    virtual void      hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    virtual QString   makeToolTip() const override;

private:
    Q_DISABLE_COPY(VToolLinePoint)
};

#endif // VTOOLLINEPOINT_H

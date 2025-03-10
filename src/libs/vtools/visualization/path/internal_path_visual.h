//---------------------------------------------------------------------------------------------------------------------
//  @file   internal_path_visual.h
//  @author Douglas S Caskey
//  @date   17 Sep, 2023
//
//  @copyright
//  Copyright (C) 2017 - 2025 Seamly, LLC
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
//  @file   vistoolinternalpath.h
//  @author Roman Telezhynskyi <dismine(at)gmail.com>
//  @date   22 11, 2016
//
//  @brief
//  @copyright
//  This source code is part of the Valentina project, a pattern making
//  program, whose allow create and modeling patterns of clothing.
//  Copyright (C) 2016 Valentina project
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

#ifndef INTERNAL_PATH_VISUAL_H
#define INTERNAL_PATH_VISUAL_H

#include <QtCore/QObject>
#include <QtGlobal>

#include "vispath.h"
#include "../vpatterndb/vpiecepath.h"

class VSimplePoint;

class InternalPathVisual : public VisPath
{
    Q_OBJECT
public:
                            InternalPathVisual(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual                ~InternalPathVisual() Q_DECL_EQ_DEFAULT;

    virtual void            RefreshGeometry() override;
    void                    setPath(const VPiecePath &path);
    void                    setCutPath(const QVector<QPointF> &path);
    virtual int             type() const override {return Type;}
    enum                    {Type = UserType + static_cast<int>(Vis::ToolInternalPath)};

protected:
    virtual void            mousePressEvent(QGraphicsSceneMouseEvent * event) override;

private:
    Q_DISABLE_COPY(InternalPathVisual)
    QVector<VSimplePoint *> m_points;
    VScaledLine            *m_line;
    VPiecePath              m_path;
    QVector<QPointF>        m_cutLinePath;

    VSimplePoint           *getPoint(quint32 i, const QColor &color);
    void                    hideAllItems();
};

#endif // INTERNAL_PATH_VISUAL_H

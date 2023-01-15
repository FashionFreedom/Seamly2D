/***************************************************************************
 **  @file   pattern_piece_visual.h
 **  @author Douglas S Caskey
 **  @date   Dec 11, 2022
 **
 **  @copyright
 **  Copyright (C) 2017 - 2022 Seamly, LLC
 **  https://github.com/fashionfreedom/seamly2d
 **
 **  @brief
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
 **  along with Seamly2D. If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

/************************************************************************
 **
 **  @file   vistoolpiece.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#ifndef PATTERN_PIECE_VISUAL_H
#define PATTERN_PIECE_VISUAL_H

#include <QtCore/QObject>
#include <QtGlobal>

#include "vispath.h"
#include "../vpatterndb/vpiece.h"

class PatternPieceVisual : public VisPath
{
    Q_OBJECT
public:
                              PatternPieceVisual(const VContainer *data, QGraphicsItem *parent = nullptr);
    virtual                  ~PatternPieceVisual() Q_DECL_EQ_DEFAULT;

    virtual void              RefreshGeometry() Q_DECL_OVERRIDE;
    void                      SetPiece(const VPiece &piece);
    virtual int               type() const Q_DECL_OVERRIDE {return Type;}
    enum                      {Type = UserType + static_cast<int>(Vis::ToolPiece)};

private:
    Q_DISABLE_COPY(PatternPieceVisual)
    QVector<VScaledEllipse *>  m_points;

    VScaledLine               *m_line1;
    VScaledLine               *m_line2;
    VPiece                     m_piece;

    VScaledEllipse            *GetPoint(quint32 i, const QColor &color);
    void                       HideAllItems();
};

#endif // PATTERN_PIECE_VISUAL_H
